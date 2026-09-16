// Copyright (c) 2026 victorvksy. All rights reserved.

#include "Capture/MCPRecording.h"
#include "Capture/MCPCapture.h"
#include "MCPTcpServer.h"
#include "MCPCaptureSettings.h"

#include <atomic>
#include "Containers/Queue.h"
#include "Containers/StringConv.h"
#include "Containers/Ticker.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Info.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Kismet/GameplayStatics.h"
#include "LevelEditor.h"
#include "Math/Float16Color.h"
#include "Misc/App.h"
#include "Misc/Base64.h"
#include "Misc/CoreDelegates.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "Modules/ModuleManager.h"
#include "PixelFormat.h"
#include "RHI.h"
#include "RHICommandList.h"
#include "RHIGPUReadback.h"
#include "RHIStats.h"
#include "RenderingThread.h"
#include "SLevelViewport.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Slate/SceneViewport.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogMCPRecording, Log, All);

// ---------------------------------------------------------------------------------------------
// Storage helpers (namespace MCPRecording)
// ---------------------------------------------------------------------------------------------

FString MCPRecording::StorageRoot()
{
	const FString& Configured = UMCPCaptureSettings::Get().StoragePath;
	const FString Path = Configured.IsEmpty() ? TEXT("MCPRecordings") : Configured;
	return FPaths::IsRelative(Path) ? FPaths::Combine(FPaths::ProjectSavedDir(), Path) : Path;
}

FString MCPRecording::SessionDir(const FString& SessionId)
{
	return FPaths::Combine(StorageRoot(), SessionId);
}

bool MCPRecording::IsValidSessionId(const FString& SessionId)
{
	if (SessionId.IsEmpty() || SessionId.Len() > 80)
	{
		return false;
	}
	for (const TCHAR C : SessionId)
	{
		if (!FChar::IsAlnum(C) && C != TEXT('-') && C != TEXT('_'))
		{
			return false;
		}
	}
	return true;
}

FString MCPRecording::ToJsonLine(const TSharedPtr<FJsonObject>& Obj)
{
	FString Out;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Out);
	FJsonSerializer::Serialize(Obj.ToSharedRef(), Writer);
	return Out;
}

void MCPRecording::ForEachJsonLine(const FString& Path, TFunctionRef<bool(const TSharedPtr<FJsonObject>&)> Fn)
{
	TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(*Path, FILEREAD_AllowWrite));
	if (!Reader)
	{
		return;
	}
	TArray<uint8> Chunk;
	Chunk.SetNumUninitialized(256 * 1024);
	TArray<uint8> Pending; // bytes of a line that continues in the next chunk
	const int64 Total = Reader->TotalSize();
	int64 Pos = 0;
	while (Pos < Total)
	{
		const int64 N = FMath::Min<int64>(Chunk.Num(), Total - Pos);
		Reader->Serialize(Chunk.GetData(), N);
		Pos += N;
		int64 LineStart = 0;
		for (int64 i = 0; i < N; ++i)
		{
			if (Chunk[i] != '\n')
			{
				continue;
			}
			Pending.Append(Chunk.GetData() + LineStart, static_cast<int32>(i - LineStart));
			LineStart = i + 1;
			if (Pending.Num() > 0)
			{
				FUTF8ToTCHAR Conv(reinterpret_cast<const ANSICHAR*>(Pending.GetData()), Pending.Num());
				const FString Line(Conv.Length(), Conv.Get());
				TSharedPtr<FJsonObject> Obj;
				auto JsonReader = TJsonReaderFactory<>::Create(Line);
				if (FJsonSerializer::Deserialize(JsonReader, Obj) && Obj.IsValid())
				{
					if (!Fn(Obj))
					{
						return;
					}
				}
			}
			Pending.Reset();
		}
		Pending.Append(Chunk.GetData() + LineStart, static_cast<int32>(N - LineStart));
	}
	// An unterminated tail is a line still being written: ignored on purpose.
}

namespace
{
	TSharedPtr<FJsonObject> RecError(const FString& Code, const FString& Message, const FString& Hint = FString())
	{
		return MCPCapture::MakeError(Code, Message, Hint);
	}

	FString NewSessionId()
	{
		return FDateTime::UtcNow().ToString(TEXT("%Y%m%d-%H%M%S")) + TEXT("-") + FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(6).ToLower();
	}

	FString ManifestPath(const FString& SessionId) { return FPaths::Combine(MCPRecording::SessionDir(SessionId), TEXT("manifest.json")); }
	FString FramesIndexPath(const FString& Dir) { return FPaths::Combine(Dir, TEXT("frames.jsonl")); }
	FString TimelinePath(const FString& Dir) { return FPaths::Combine(Dir, TEXT("timeline.jsonl")); }

	FString ManifestUri(const FString& SessionId)
	{
		FString Full = FPaths::ConvertRelativePathToFull(ManifestPath(SessionId));
		FPaths::NormalizeFilename(Full);
		return TEXT("file:///") + Full;
	}

	TSharedPtr<FJsonObject> ReadJsonFile(const FString& Path)
	{
		FString Text;
		if (!FFileHelper::LoadFileToString(Text, *Path))
		{
			return nullptr;
		}
		TSharedPtr<FJsonObject> Obj;
		auto Reader = TJsonReaderFactory<>::Create(Text);
		return (FJsonSerializer::Deserialize(Reader, Obj) && Obj.IsValid()) ? Obj : nullptr;
	}

	/** Writes Text to Path through a temp file + move, so readers never see a half-written file. */
	bool WriteFileAtomic(const FString& Path, const FString& Text)
	{
		const FString Tmp = Path + TEXT(".tmp");
		if (!FFileHelper::SaveStringToFile(Text, *Tmp, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
		{
			return false;
		}
		return IFileManager::Get().Move(*Path, *Tmp, /*Replace*/ true, /*EvenIfReadOnly*/ true, /*Attributes*/ false, /*DoNotRetryOrError*/ true);
	}

	void WriteUtf8Line(FArchive& Ar, const FString& Line)
	{
		FTCHARToUTF8 Utf8(*Line);
		Ar.Serialize(const_cast<ANSICHAR*>(Utf8.Get()), Utf8.Length());
		Ar.Serialize(const_cast<char*>("\n"), 1);
	}

	const TCHAR* VerbosityName(ELogVerbosity::Type V)
	{
		switch (V & ELogVerbosity::VerbosityMask)
		{
		case ELogVerbosity::Fatal: return TEXT("Fatal");
		case ELogVerbosity::Error: return TEXT("Error");
		case ELogVerbosity::Warning: return TEXT("Warning");
		case ELogVerbosity::Display: return TEXT("Display");
		case ELogVerbosity::Log: return TEXT("Log");
		case ELogVerbosity::Verbose: return TEXT("Verbose");
		default: return TEXT("VeryVerbose");
		}
	}

	TSharedPtr<FJsonObject> VecJson(const FVector& V)
	{
		auto O = MakeShared<FJsonObject>();
		O->SetNumberField(TEXT("x"), V.X);
		O->SetNumberField(TEXT("y"), V.Y);
		O->SetNumberField(TEXT("z"), V.Z);
		return O;
	}

	/** Raw readback bytes in the viewport's pixel format -> opaque BGRA. Worker thread. */
	bool ConvertToBGRA(const TArray<uint8>& Raw, EPixelFormat Format, int32 W, int32 H, TArray<FColor>& Out)
	{
		const int64 N = static_cast<int64>(W) * H;
		if (N <= 0)
		{
			return false;
		}
		Out.SetNumUninitialized(N);
		switch (Format)
		{
		case PF_B8G8R8A8:
			if (Raw.Num() < N * 4) return false;
			FMemory::Memcpy(Out.GetData(), Raw.GetData(), N * 4);
			break;
		case PF_R8G8B8A8:
		{
			if (Raw.Num() < N * 4) return false;
			const uint8* S = Raw.GetData();
			for (int64 i = 0; i < N; ++i, S += 4)
			{
				Out[i] = FColor(S[0], S[1], S[2], 255);
			}
			break;
		}
		case PF_A2B10G10R10:
		{
			if (Raw.Num() < N * 4) return false;
			const uint32* S = reinterpret_cast<const uint32*>(Raw.GetData());
			for (int64 i = 0; i < N; ++i)
			{
				const uint32 V = S[i];
				Out[i] = FColor((V & 0x3FF) >> 2, ((V >> 10) & 0x3FF) >> 2, ((V >> 20) & 0x3FF) >> 2, 255);
			}
			break;
		}
		case PF_FloatRGBA:
		{
			if (Raw.Num() < N * 8) return false;
			const FFloat16Color* S = reinterpret_cast<const FFloat16Color*>(Raw.GetData());
			for (int64 i = 0; i < N; ++i)
			{
				Out[i] = FColor(
					FMath::Clamp(FMath::RoundToInt(S[i].R.GetFloat() * 255.f), 0, 255),
					FMath::Clamp(FMath::RoundToInt(S[i].G.GetFloat() * 255.f), 0, 255),
					FMath::Clamp(FMath::RoundToInt(S[i].B.GetFloat() * 255.f), 0, 255), 255);
			}
			break;
		}
		default:
			return false;
		}
		for (FColor& C : Out)
		{
			C.A = 255;
		}
		return true;
	}

	// -----------------------------------------------------------------------------------------
	// One recording session
	// -----------------------------------------------------------------------------------------

	class FMCPRecordingSession final : public FRunnable, public FOutputDevice
	{
	public:
		struct FConfig
		{
			FString SessionId;
			double DurationS = 30.0;
			double Fps = 2.0;
			int32 Width = 1024;
			int32 Height = 576;
			int32 Quality = 80;
			int32 MaxActors = 200;
			TArray<FString> ActorFilter;
			bool bStartedPIE = false; // the tool requested the play session: fixed step + seed apply
		};

		explicit FMCPRecordingSession(const FConfig& InConfig)
			: Config(InConfig)
			, Dir(MCPRecording::SessionDir(InConfig.SessionId))
		{
			Stats.State = TEXT("starting");
			Stats.StartedUtc = FDateTime::UtcNow().ToIso8601();
			WorkerEvent = FPlatformProcess::GetSynchEventFromPool(false);
		}

		virtual ~FMCPRecordingSession() override
		{
			Finish(TEXT("shutdown"));
			FPlatformProcess::ReturnSynchEventToPool(WorkerEvent);
			WorkerEvent = nullptr;
		}

		const FString& Id() const { return Config.SessionId; }
		bool IsDone() const { return bFinished; }

		/** Game thread. Starts the worker, hooks the ticker, the log and the PIE-end delegate. */
		void Begin()
		{
			check(IsInGameThread());
			WaitingSince = FPlatformTime::Seconds();
			Thread = FRunnableThread::Create(this, *FString::Printf(TEXT("MCPRecording %s"), *Config.SessionId), 0, TPri_BelowNormal);
			TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FMCPRecordingSession::Tick), 0.0f);
			EndPIEHandle = FEditorDelegates::EndPIE.AddRaw(this, &FMCPRecordingSession::OnEndPIE);
			PreExitHandle = FCoreDelegates::OnPreExit.AddRaw(this, &FMCPRecordingSession::OnPreExit);
			if (GLog)
			{
				GLog->AddOutputDevice(this);
			}
			UE_LOG(LogMCPRecording, Log, TEXT("[UnrealMCP] Recording %s: %.1f s at %.1f fps, %dx%d -> %s"),
				*Config.SessionId, Config.DurationS, Config.Fps, Config.Width, Config.Height, *Dir);
		}

		/** Game thread, idempotent. Unhooks everything, drains the GPU and the worker, writes the
		 *  final manifest. After this returns no render command or worker references the session. */
		void Finish(const FString& Reason)
		{
			check(IsInGameThread());
			if (bFinished)
			{
				return;
			}
			bFinished = true;
			FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
			FEditorDelegates::EndPIE.Remove(EndPIEHandle);
			FCoreDelegates::OnPreExit.Remove(PreExitHandle);
			if (GLog)
			{
				GLog->RemoveOutputDevice(this);
			}
			RestoreTimeStep();

			// Copy out whatever the GPU has finished; drop what it has not.
			EnqueuePoll(/*bFinal*/ true);
			FlushRenderingCommands();

			// The worker drains the queue and exits.
			bStopWorker = true;
			if (WorkerEvent)
			{
				WorkerEvent->Trigger();
			}
			if (Thread)
			{
				Thread->WaitForCompletion();
				delete Thread;
				Thread = nullptr;
			}
			{
				FScopeLock Lock(&StatsLock);
				Stats.State = TEXT("finished");
				Stats.EndReason = Reason;
			}
			WriteManifest();
			UE_LOG(LogMCPRecording, Log, TEXT("[UnrealMCP] Recording %s finished (%s): %d frames, %d dropped, %d warnings, %d errors"),
				*Config.SessionId, *Reason, Stats.FramesWritten, Stats.FramesDropped, Stats.Warnings, Stats.Errors);
		}

		/** The manifest as JSON (any thread). This is also what get_recording_status returns live. */
		TSharedPtr<FJsonObject> BuildManifest() const
		{
			FScopeLock Lock(&StatsLock);
			auto M = MakeShared<FJsonObject>();
			M->SetStringField(TEXT("session_id"), Config.SessionId);
			M->SetStringField(TEXT("state"), Stats.State);
			if (!Stats.EndReason.IsEmpty())
			{
				M->SetStringField(TEXT("end_reason"), Stats.EndReason);
			}
			M->SetStringField(TEXT("level"), Stats.Level);
			M->SetStringField(TEXT("started_utc"), Stats.StartedUtc);
			M->SetNumberField(TEXT("fps"), Config.Fps);
			M->SetNumberField(TEXT("duration_s"), Config.DurationS);
			auto Res = MakeShared<FJsonObject>();
			Res->SetNumberField(TEXT("w"), Config.Width);
			Res->SetNumberField(TEXT("h"), Config.Height);
			M->SetObjectField(TEXT("resolution"), Res);
			M->SetNumberField(TEXT("frames"), Stats.FramesWritten);
			M->SetNumberField(TEXT("dropped_frames"), Stats.FramesDropped);
			M->SetNumberField(TEXT("warnings"), Stats.Warnings);
			M->SetNumberField(TEXT("errors"), Stats.Errors);
			M->SetNumberField(TEXT("log_lines"), Stats.LogLines);
			M->SetNumberField(TEXT("actor_events"), Stats.ActorEvents);
			M->SetNumberField(TEXT("world_time_start"), Stats.WorldTimeStart);
			M->SetNumberField(TEXT("world_time_end"), Stats.WorldTimeEnd);
			M->SetNumberField(TEXT("recorded_s"), Stats.WorldTimeStart >= 0 ? Stats.WorldTimeEnd - Stats.WorldTimeStart : 0.0);
			M->SetStringField(TEXT("source_format"), Stats.SourceFormat);
			M->SetBoolField(TEXT("started_pie"), Config.bStartedPIE);
			M->SetBoolField(TEXT("fixed_timestep"), Config.bStartedPIE);
			TArray<TSharedPtr<FJsonValue>> Filter;
			for (const FString& F : Config.ActorFilter)
			{
				Filter.Add(MakeShared<FJsonValueString>(F));
			}
			M->SetArrayField(TEXT("actor_filter"), Filter);
			auto Timings = MakeShared<FJsonObject>();
			Timings->SetNumberField(TEXT("game_thread_ms_avg"), Stats.GTFrames > 0 ? Stats.GTSumMs / Stats.GTFrames : 0.0);
			Timings->SetNumberField(TEXT("game_thread_ms_max"), Stats.GTMaxMs);
			Timings->SetNumberField(TEXT("frames_measured"), Stats.GTFrames);
			Timings->SetNumberField(TEXT("worker_encode_ms_avg"), Stats.EncodeFrames > 0 ? Stats.EncodeSumMs / Stats.EncodeFrames : 0.0);
			M->SetObjectField(TEXT("timings"), Timings);
			auto Files = MakeShared<FJsonObject>();
			Files->SetStringField(TEXT("dir"), Dir);
			Files->SetStringField(TEXT("frames_index"), TEXT("frames.jsonl"));
			Files->SetStringField(TEXT("timeline"), TEXT("timeline.jsonl"));
			M->SetObjectField(TEXT("files"), Files);
			M->SetStringField(TEXT("manifest_uri"), ManifestUri(Config.SessionId));
			TArray<TSharedPtr<FJsonValue>> Notes;
			for (const FString& N : Stats.Notes)
			{
				Notes.Add(MakeShared<FJsonValueString>(N));
			}
			M->SetArrayField(TEXT("notes"), Notes);
			return M;
		}

		// ---- FOutputDevice (any thread) ----
		virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
		{
			const ELogVerbosity::Type Level = static_cast<ELogVerbosity::Type>(Verbosity & ELogVerbosity::VerbosityMask);
			if (Level > ELogVerbosity::Log || Category == TEXT("LogUnrealMCP") || Category == TEXT("LogMCPRecording"))
			{
				return; // Verbose/VeryVerbose and the transport's own chatter stay out
			}
			if (State.load() != EState::Recording)
			{
				return;
			}
			int32 Count = 0;
			{
				FScopeLock Lock(&StatsLock);
				if (Level == ELogVerbosity::Warning) ++Stats.Warnings;
				else if (Level <= ELogVerbosity::Error) ++Stats.Errors;
				Count = ++Stats.LogLines;
			}
			if (Count > MaxLogLines)
			{
				if (Count == MaxLogLines + 1)
				{
					auto O = MakeShared<FJsonObject>();
					O->SetStringField(TEXT("verbosity"), TEXT("Warning"));
					O->SetStringField(TEXT("category"), TEXT("LogMCPRecording"));
					O->SetStringField(TEXT("message"), FString::Printf(TEXT("log truncated after %d lines"), MaxLogLines));
					PushRecord(TEXT("log"), O);
				}
				return;
			}
			auto O = MakeShared<FJsonObject>();
			O->SetStringField(TEXT("verbosity"), VerbosityName(Level));
			O->SetStringField(TEXT("category"), Category.ToString());
			FString Message(V);
			if (Message.Len() > 1000)
			{
				Message = Message.Left(1000) + TEXT("...");
			}
			O->SetStringField(TEXT("message"), Message);
			PushRecord(TEXT("log"), O);
		}
		virtual bool CanBeUsedOnAnyThread() const override { return true; }
		virtual bool CanBeUsedOnMultipleThreads() const override { return true; }

		// ---- FRunnable (worker thread) ----
		virtual uint32 Run() override
		{
			IFileManager::Get().MakeDirectory(*FPaths::Combine(Dir, TEXT("frames")), true);
			FramesWriter.Reset(IFileManager::Get().CreateFileWriter(*FramesIndexPath(Dir), FILEWRITE_Append | FILEWRITE_AllowRead));
			TimelineWriter.Reset(IFileManager::Get().CreateFileWriter(*TimelinePath(Dir), FILEWRITE_Append | FILEWRITE_AllowRead));
			WriteManifest();
			while (!bStopWorker)
			{
				WorkerEvent->Wait();
				Drain();
			}
			Drain();
			FramesWriter.Reset();
			TimelineWriter.Reset();
			return 0;
		}
		virtual void Stop() override
		{
			bStopWorker = true;
			if (WorkerEvent)
			{
				WorkerEvent->Trigger();
			}
		}

	private:
		enum class EState : int32 { WaitingForPIE, Recording, Finished };

		struct FSlot
		{
			TAtomic<int32> Pending{0};                 // 1 while a copy is enqueued/in flight
			TUniquePtr<FRHIGPUTextureReadback> Readback;
			int32 Index = 0;
			double T = 0.0;                            // seconds since the recording started
			double WorldTime = 0.0;
			FIntPoint Size = FIntPoint::ZeroValue;
			EPixelFormat Format = PF_Unknown;
			TArray<MCPCapture::FVisibleActor> Actors;
		};

		struct FJob
		{
			enum class EKind { Frame, Record } Kind = EKind::Record;
			// Frame
			int32 Index = 0;
			double T = 0.0;
			double WorldTime = 0.0;
			FIntPoint Size = FIntPoint::ZeroValue;
			EPixelFormat Format = PF_Unknown;
			TArray<uint8> Raw;                         // pitch-stripped rows in Format
			TArray<FColor> Pixels;                     // set instead of Raw by the synchronous fallback
			TArray<MCPCapture::FVisibleActor> Actors;
			// Record
			FString Line;
		};

		struct FTracked
		{
			TWeakObjectPtr<AActor> Actor;
			FString Name;
			FVector Location = FVector::ZeroVector;
			FRotator Rotation = FRotator::ZeroRotator;
			TMap<FName, double> Numbers;
			TMap<FName, bool> Bools;
		};

		struct FStats
		{
			FString State;
			FString EndReason;
			FString Level;
			FString StartedUtc;
			FString SourceFormat = TEXT("none");
			int32 FramesWritten = 0;
			int32 FramesDropped = 0;
			int32 Warnings = 0;
			int32 Errors = 0;
			int32 LogLines = 0;
			int32 ActorEvents = 0;
			double WorldTimeStart = -1.0;
			double WorldTimeEnd = -1.0;
			double GTSumMs = 0.0;
			double GTMaxMs = 0.0;
			int32 GTFrames = 0;
			double EncodeSumMs = 0.0;
			int32 EncodeFrames = 0;
			TArray<FString> Notes;
		};

		static constexpr int32 MaxLogLines = 20000;

		bool Tick(float DeltaTime)
		{
			if (bFinished)
			{
				return false;
			}
			UWorld* World = GEditor ? GEditor->PlayWorld.Get() : nullptr;
			if (State.load() == EState::WaitingForPIE)
			{
				APlayerController* PC = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
				if (!World || !PC || !GEngine || !GEngine->GameViewport)
				{
					if (FPlatformTime::Seconds() - WaitingSince > 60.0)
					{
						Note(TEXT("PIE did not produce a player within 60 s"));
						Finish(TEXT("failed"));
						return false;
					}
					return true;
				}
				WorldTimeStart = World->GetTimeSeconds();
				NextCaptureTime = WorldTimeStart;
				{
					FScopeLock Lock(&StatsLock);
					Stats.WorldTimeStart = WorldTimeStart;
					Stats.WorldTimeEnd = WorldTimeStart;
					Stats.Level = UWorld::RemovePIEPrefix(World->GetMapName());
					Stats.State = TEXT("recording");
				}
				if (Config.bStartedPIE)
				{
					ApplyFixedTimeStep();
				}
				State.store(EState::Recording);
			}
			if (!World)
			{
				Finish(TEXT("pie_ended"));
				return false;
			}
			const double WorldTime = World->GetTimeSeconds();
			CurrentT.store(WorldTime - WorldTimeStart);
			{
				FScopeLock Lock(&StatsLock);
				Stats.WorldTimeEnd = WorldTime;
			}
			if (WorldTime - WorldTimeStart >= Config.DurationS - KINDA_SMALL_NUMBER)
			{
				Finish(TEXT("duration"));
				return false;
			}
			if (WorldTime + KINDA_SMALL_NUMBER >= NextCaptureTime)
			{
				const double T0 = FPlatformTime::Seconds();
				CaptureFrame(World, WorldTime);
				SampleActors(World, WorldTime - WorldTimeStart);
				PushStatsRecord(WorldTime - WorldTimeStart, DeltaTime);
				const double Ms = (FPlatformTime::Seconds() - T0) * 1000.0;
				{
					FScopeLock Lock(&StatsLock);
					Stats.GTSumMs += Ms;
					Stats.GTMaxMs = FMath::Max(Stats.GTMaxMs, Ms);
					++Stats.GTFrames;
				}
				// Stay on the world-time grid; skip intervals the editor could not reach.
				const double Interval = 1.0 / Config.Fps;
				NextCaptureTime = WorldTimeStart + Interval * (FMath::Floor((WorldTime - WorldTimeStart) / Interval) + 1.0);
			}
			EnqueuePoll(/*bFinal*/ false);
			return true;
		}

		/** Game thread: enqueue a GPU copy of the PIE viewport's last frame and note the actors. */
		void CaptureFrame(UWorld* World, double WorldTime)
		{
			const int32 Index = NextIndex++;
			const double T = WorldTime - WorldTimeStart;
			MCPCapture::FView View;
			if (MCPCapture::ResolvePIEView(View).IsValid())
			{
				Dropped(TEXT("no player view"));
				return;
			}
			FSceneViewport* SceneViewport = (GEngine && GEngine->GameViewport) ? GEngine->GameViewport->GetGameViewport() : nullptr;
			FTextureRHIRef Texture = SceneViewport ? SceneViewport->GetRenderTargetTexture() : nullptr;
			if (!Texture.IsValid() || Texture->GetSizeXY().X <= 0 || GPixelFormats[Texture->GetFormat()].BlockBytes == 0)
			{
				// No separate render target (a viewport drawing straight into the window):
				// synchronous readback so the session still records, with a note in the manifest.
				if (!View.Viewport)
				{
					Dropped(TEXT("no viewport"));
					return;
				}
				NoteOnce(bSyncFallbackNoted, TEXT("PIE viewport has no render target texture; frames use a synchronous readback (game-thread stall per frame)"));
				const FIntPoint Size = View.Viewport->GetSizeXY();
				TUniquePtr<FJob> Job = MakeUnique<FJob>();
				Job->Kind = FJob::EKind::Frame;
				Job->Index = Index;
				Job->T = T;
				Job->WorldTime = WorldTime;
				Job->Size = Size;
				Job->Format = PF_B8G8R8A8;
				if (!View.Viewport->ReadPixels(Job->Pixels, FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX), FIntRect(0, 0, Size.X, Size.Y)))
				{
					Dropped(TEXT("readback failed"));
					return;
				}
				Job->Actors = MCPCapture::FindVisibleActors(View, Size.X, Size.Y, Config.MaxActors);
				SetSourceFormat(TEXT("sync:B8G8R8A8"));
				Queue.Enqueue(MoveTemp(Job));
				WorkerEvent->Trigger();
				return;
			}

			FSlot* Slot = nullptr;
			for (FSlot& S : Slots)
			{
				if (S.Pending.Load() == 0)
				{
					Slot = &S;
					break;
				}
			}
			if (!Slot)
			{
				Dropped(TEXT("both readbacks still in flight")); // the GPU is more than two frames behind
				return;
			}
			if (!Slot->Readback)
			{
				Slot->Readback = MakeUnique<FRHIGPUTextureReadback>(TEXT("MCPRecordingReadback"));
			}
			Slot->Index = Index;
			Slot->T = T;
			Slot->WorldTime = WorldTime;
			Slot->Size = Texture->GetSizeXY();
			Slot->Format = Texture->GetFormat();
			Slot->Actors = MCPCapture::FindVisibleActors(View, Slot->Size.X, Slot->Size.Y, Config.MaxActors);
			SetSourceFormat(GPixelFormats[Slot->Format].Name);
			Slot->Pending.Store(1);

			FRHIGPUTextureReadback* Readback = Slot->Readback.Get();
			ENQUEUE_RENDER_COMMAND(MCPRecordingEnqueueCopy)([Readback, Texture](FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.Transition(FRHITransitionInfo(Texture, ERHIAccess::Unknown, ERHIAccess::CopySrc));
				Readback->EnqueueCopy(RHICmdList, Texture, FResolveRect());
				RHICmdList.Transition(FRHITransitionInfo(Texture, ERHIAccess::CopySrc, ERHIAccess::SRVMask));
			});
		}

		/** Game thread: one render command that copies out every readback the GPU has finished.
		 *  Nothing here waits, except the final poll at Finish(), which blocks until the GPU is
		 *  idle so the last enqueued frame is not lost. The session outlives the command: Finish()
		 *  flushes rendering commands before the object can be destroyed. */
		void EnqueuePoll(bool bFinal)
		{
			bool bAny = false;
			for (FSlot& S : Slots)
			{
				bAny |= S.Pending.Load() != 0;
			}
			if (!bAny)
			{
				return;
			}
			ENQUEUE_RENDER_COMMAND(MCPRecordingPoll)([this, bFinal](FRHICommandListImmediate& RHICmdList)
			{
				if (bFinal)
				{
					RHICmdList.BlockUntilGPUIdle();
				}
				for (FSlot& S : Slots)
				{
					if (S.Pending.Load() != 1 || !S.Readback)
					{
						continue;
					}
					if (!S.Readback->IsReady())
					{
						if (bFinal)
						{
							Dropped(TEXT("GPU copy still pending at finish"));
							S.Pending.Store(0);
						}
						continue;
					}
					int32 PitchPixels = 0;
					int32 BufferHeight = 0;
					const uint8* Data = static_cast<const uint8*>(S.Readback->Lock(PitchPixels, &BufferHeight));
					if (Data && PitchPixels >= S.Size.X)
					{
						const uint32 Bpp = GPixelFormats[S.Format].BlockBytes;
						TUniquePtr<FJob> Job = MakeUnique<FJob>();
						Job->Kind = FJob::EKind::Frame;
						Job->Index = S.Index;
						Job->T = S.T;
						Job->WorldTime = S.WorldTime;
						Job->Size = S.Size;
						Job->Format = S.Format;
						Job->Actors = MoveTemp(S.Actors);
						Job->Raw.SetNumUninitialized(static_cast<int64>(S.Size.X) * S.Size.Y * Bpp);
						for (int32 y = 0; y < S.Size.Y; ++y)
						{
							FMemory::Memcpy(Job->Raw.GetData() + static_cast<int64>(y) * S.Size.X * Bpp, Data + static_cast<int64>(y) * PitchPixels * Bpp, static_cast<int64>(S.Size.X) * Bpp);
						}
						S.Readback->Unlock();
						Queue.Enqueue(MoveTemp(Job));
						WorkerEvent->Trigger();
					}
					else
					{
						if (Data)
						{
							S.Readback->Unlock();
						}
						Dropped(TEXT("readback lock failed"));
					}
					S.Pending.Store(0);
				}
			});
		}

		bool IsTracked(AActor* A) const
		{
			if (!A || A->IsA<AInfo>())
			{
				return false;
			}
			if (Config.ActorFilter.Num() == 0)
			{
				return A->IsA<APawn>() || A->ActorHasTag(TEXT("MCPTrack"));
			}
			for (const FString& F : Config.ActorFilter)
			{
				if (A->ActorHasTag(FName(*F)))
				{
					return true;
				}
				for (UClass* C = A->GetClass(); C; C = C->GetSuperClass())
				{
					if (C->GetName() == F)
					{
						return true;
					}
				}
			}
			return false;
		}

		/** Blueprint-visible bools and numbers, generic through reflection (no game-specific names). */
		static void SnapshotProperties(AActor* A, TMap<FName, double>& Numbers, TMap<FName, bool>& Bools)
		{
			int32 Count = 0;
			for (TFieldIterator<FProperty> It(A->GetClass()); It && Count < 32; ++It)
			{
				FProperty* P = *It;
				if (!P->HasAnyPropertyFlags(CPF_BlueprintVisible) || P->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated) || P->ArrayDim != 1)
				{
					continue;
				}
				const void* V = P->ContainerPtrToValuePtr<void>(A);
				if (const FBoolProperty* B = CastField<FBoolProperty>(P))
				{
					Bools.Add(P->GetFName(), B->GetPropertyValue(V));
					++Count;
				}
				else if (const FNumericProperty* N = CastField<FNumericProperty>(P))
				{
					if (N->IsEnum())
					{
						continue;
					}
					Numbers.Add(P->GetFName(), N->IsFloatingPoint() ? N->GetFloatingPointPropertyValue(V) : static_cast<double>(N->GetSignedIntPropertyValue(V)));
					++Count;
				}
			}
		}

		void SampleActors(UWorld* World, double T)
		{
			TSet<AActor*> Seen;
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				AActor* A = *It;
				if (!IsTracked(A))
				{
					continue;
				}
				Seen.Add(A);
				FTracked* Tracked = TrackedActors.Find(A);
				if (!Tracked)
				{
					FTracked& New = TrackedActors.Add(A);
					New.Actor = A;
					New.Name = A->GetActorNameOrLabel();
					New.Location = A->GetActorLocation();
					New.Rotation = A->GetActorRotation();
					SnapshotProperties(A, New.Numbers, New.Bools);
					auto Detail = MakeShared<FJsonObject>();
					Detail->SetStringField(TEXT("class"), A->GetClass()->GetName());
					Detail->SetObjectField(TEXT("location"), VecJson(New.Location));
					ActorEvent(T, New.Name, bFirstSample ? TEXT("tracked") : TEXT("spawned"), Detail);
					continue;
				}
				const FVector Loc = A->GetActorLocation();
				const FRotator Rot = A->GetActorRotation();
				const double Moved = FVector::Dist(Loc, Tracked->Location);
				const double Turned = FMath::Abs((Rot - Tracked->Rotation).GetNormalized().Yaw) + FMath::Abs((Rot - Tracked->Rotation).GetNormalized().Pitch);
				if (Moved > 10.0 || Turned > 2.0)
				{
					auto Detail = MakeShared<FJsonObject>();
					Detail->SetObjectField(TEXT("from"), VecJson(Tracked->Location));
					Detail->SetObjectField(TEXT("to"), VecJson(Loc));
					Detail->SetNumberField(TEXT("distance"), Moved);
					Detail->SetNumberField(TEXT("yaw"), Rot.Yaw);
					ActorEvent(T, Tracked->Name, TEXT("moved"), Detail);
					Tracked->Location = Loc;
					Tracked->Rotation = Rot;
				}
				TMap<FName, double> Numbers;
				TMap<FName, bool> Bools;
				SnapshotProperties(A, Numbers, Bools);
				int32 Changes = 0;
				for (const auto& Pair : Numbers)
				{
					const double* Old = Tracked->Numbers.Find(Pair.Key);
					if (Old && !FMath::IsNearlyEqual(*Old, Pair.Value, 1e-4) && Changes++ < 8)
					{
						auto Detail = MakeShared<FJsonObject>();
						Detail->SetStringField(TEXT("property"), Pair.Key.ToString());
						Detail->SetNumberField(TEXT("from"), *Old);
						Detail->SetNumberField(TEXT("to"), Pair.Value);
						ActorEvent(T, Tracked->Name, TEXT("state_changed"), Detail);
					}
				}
				for (const auto& Pair : Bools)
				{
					const bool* Old = Tracked->Bools.Find(Pair.Key);
					if (Old && *Old != Pair.Value && Changes++ < 8)
					{
						auto Detail = MakeShared<FJsonObject>();
						Detail->SetStringField(TEXT("property"), Pair.Key.ToString());
						Detail->SetBoolField(TEXT("from"), *Old);
						Detail->SetBoolField(TEXT("to"), Pair.Value);
						ActorEvent(T, Tracked->Name, TEXT("state_changed"), Detail);
					}
				}
				Tracked->Numbers = MoveTemp(Numbers);
				Tracked->Bools = MoveTemp(Bools);
			}
			for (auto It = TrackedActors.CreateIterator(); It; ++It)
			{
				AActor* A = It->Value.Actor.Get();
				if (!A || !Seen.Contains(A))
				{
					auto Detail = MakeShared<FJsonObject>();
					Detail->SetObjectField(TEXT("last_location"), VecJson(It->Value.Location));
					ActorEvent(T, It->Value.Name, TEXT("destroyed"), Detail);
					It.RemoveCurrent();
				}
			}
			bFirstSample = false;
		}

		void ActorEvent(double T, const FString& Name, const TCHAR* Event, const TSharedPtr<FJsonObject>& Detail)
		{
			auto O = MakeShared<FJsonObject>();
			O->SetStringField(TEXT("actor"), Name);
			O->SetStringField(TEXT("event"), Event);
			O->SetObjectField(TEXT("detail"), Detail);
			{
				FScopeLock Lock(&StatsLock);
				++Stats.ActorEvents;
			}
			PushRecord(TEXT("actor"), O, T);
		}

		void PushStatsRecord(double T, float DeltaTime)
		{
			auto O = MakeShared<FJsonObject>();
			O->SetNumberField(TEXT("fps"), DeltaTime > 0.f ? 1.0 / DeltaTime : 0.0);
			O->SetNumberField(TEXT("frame_ms"), DeltaTime * 1000.0);
			O->SetNumberField(TEXT("draw_calls"), GNumDrawCallsRHI[0]);
			PushRecord(TEXT("stats"), O, T);
		}

		/** Any thread. T defaults to the recording clock as last seen by the ticker. */
		void PushRecord(const TCHAR* Kind, const TSharedPtr<FJsonObject>& Obj, double T = -1.0)
		{
			Obj->SetNumberField(TEXT("t"), T >= 0.0 ? T : CurrentT.load());
			Obj->SetStringField(TEXT("kind"), Kind);
			TUniquePtr<FJob> Job = MakeUnique<FJob>();
			Job->Kind = FJob::EKind::Record;
			Job->Line = MCPRecording::ToJsonLine(Obj);
			Queue.Enqueue(MoveTemp(Job));
			if (WorkerEvent)
			{
				WorkerEvent->Trigger();
			}
		}

		void Dropped(const TCHAR* Why)
		{
			FScopeLock Lock(&StatsLock);
			++Stats.FramesDropped;
			if (Stats.Notes.Num() < 20)
			{
				Stats.Notes.AddUnique(FString::Printf(TEXT("dropped frame: %s"), Why));
			}
		}

		void Note(const FString& Text)
		{
			FScopeLock Lock(&StatsLock);
			if (Stats.Notes.Num() < 20)
			{
				Stats.Notes.AddUnique(Text);
			}
		}

		void NoteOnce(bool& bFlag, const TCHAR* Text)
		{
			if (!bFlag)
			{
				bFlag = true;
				Note(Text);
			}
		}

		void SetSourceFormat(const FString& Name)
		{
			FScopeLock Lock(&StatsLock);
			Stats.SourceFormat = Name;
		}

		void ApplyFixedTimeStep()
		{
			bPrevUseFixed = FApp::UseFixedTimeStep();
			PrevFixedDelta = FApp::GetFixedDeltaTime();
			FApp::SetUseFixedTimeStep(true);
			FApp::SetFixedDeltaTime(1.0 / 30.0);
			FMath::RandInit(20260915);
			FMath::SRandInit(20260915);
			bFixedApplied = true;
		}

		void RestoreTimeStep()
		{
			if (bFixedApplied)
			{
				FApp::SetUseFixedTimeStep(bPrevUseFixed);
				FApp::SetFixedDeltaTime(PrevFixedDelta);
				bFixedApplied = false;
			}
		}

		void OnEndPIE(const bool /*bIsSimulating*/)
		{
			Finish(TEXT("pie_ended"));
		}

		void OnPreExit()
		{
			Finish(TEXT("shutdown"));
		}

		// ---- worker thread ----
		void Drain()
		{
			TUniquePtr<FJob> Job;
			bool bWroteFrame = false;
			while (Queue.Dequeue(Job))
			{
				if (Job->Kind == FJob::EKind::Frame)
				{
					ProcessFrame(*Job);
					bWroteFrame = true;
				}
				else if (TimelineWriter)
				{
					WriteUtf8Line(*TimelineWriter, Job->Line);
				}
			}
			if (TimelineWriter)
			{
				TimelineWriter->Flush();
			}
			if (bWroteFrame)
			{
				if (FramesWriter)
				{
					FramesWriter->Flush();
				}
				WriteManifest();
			}
		}

		void ProcessFrame(FJob& Job)
		{
			const double T0 = FPlatformTime::Seconds();
			TArray<FColor> Pixels;
			if (Job.Pixels.Num() > 0)
			{
				Pixels = MoveTemp(Job.Pixels);
			}
			else if (!ConvertToBGRA(Job.Raw, Job.Format, Job.Size.X, Job.Size.Y, Pixels))
			{
				Dropped(*FString::Printf(TEXT("unsupported viewport format %s"), GPixelFormats[Job.Format].Name));
				return;
			}
			int32 W = Config.Width, H = Config.Height;
			MCPCapture::FitSize(Job.Size.X, Job.Size.Y, W, H);
			MCPCapture::BoxResize(Pixels, Job.Size.X, Job.Size.Y, W, H);
			TArray64<uint8> Bytes;
			if (!MCPCapture::EncodeImage(Pixels, W, H, TEXT("jpeg"), Config.Quality, Bytes))
			{
				Dropped(TEXT("jpeg encode failed"));
				return;
			}
			const FString File = FString::Printf(TEXT("frames/%06d.jpg"), Job.Index);
			if (!FFileHelper::SaveArrayToFile(TArrayView<const uint8>(Bytes.GetData(), static_cast<int32>(Bytes.Num())), *FPaths::Combine(Dir, File)))
			{
				Dropped(TEXT("frame write failed"));
				return;
			}
			auto Rec = MakeShared<FJsonObject>();
			Rec->SetNumberField(TEXT("index"), Job.Index);
			Rec->SetNumberField(TEXT("t"), Job.T);
			Rec->SetNumberField(TEXT("world_time"), Job.WorldTime);
			Rec->SetStringField(TEXT("file"), File);
			Rec->SetNumberField(TEXT("width"), W);
			Rec->SetNumberField(TEXT("height"), H);
			Rec->SetNumberField(TEXT("bytes"), Bytes.Num());
			TArray<TSharedPtr<FJsonValue>> Actors;
			const double SX = static_cast<double>(W) / Job.Size.X, SY = static_cast<double>(H) / Job.Size.Y;
			for (const MCPCapture::FVisibleActor& V : Job.Actors)
			{
				auto O = MakeShared<FJsonObject>();
				O->SetStringField(TEXT("name"), V.Name);
				O->SetStringField(TEXT("class"), V.Class);
				TArray<TSharedPtr<FJsonValue>> Box;
				Box.Add(MakeShared<FJsonValueNumber>(FMath::RoundToInt(V.ScreenBox.Min.X * SX)));
				Box.Add(MakeShared<FJsonValueNumber>(FMath::RoundToInt(V.ScreenBox.Min.Y * SY)));
				Box.Add(MakeShared<FJsonValueNumber>(FMath::RoundToInt(V.ScreenBox.Max.X * SX)));
				Box.Add(MakeShared<FJsonValueNumber>(FMath::RoundToInt(V.ScreenBox.Max.Y * SY)));
				O->SetArrayField(TEXT("screen_bbox"), Box);
				O->SetObjectField(TEXT("world_location"), VecJson(V.WorldLocation));
				O->SetNumberField(TEXT("distance"), V.Distance);
				Actors.Add(MakeShared<FJsonValueObject>(O));
			}
			Rec->SetArrayField(TEXT("actors"), Actors);
			if (FramesWriter)
			{
				WriteUtf8Line(*FramesWriter, MCPRecording::ToJsonLine(Rec));
			}
			const double Ms = (FPlatformTime::Seconds() - T0) * 1000.0;
			FScopeLock Lock(&StatsLock);
			++Stats.FramesWritten;
			Stats.EncodeSumMs += Ms;
			++Stats.EncodeFrames;
		}

		void WriteManifest()
		{
			WriteFileAtomic(FPaths::Combine(Dir, TEXT("manifest.json")), MCPRecording::ToJsonLine(BuildManifest()));
		}

		// ---- state ----
		FConfig Config;
		FString Dir;
		std::atomic<EState> State{EState::WaitingForPIE};
		bool bFinished = false;                   // game thread
		double WaitingSince = 0.0;
		double WorldTimeStart = -1.0;
		double NextCaptureTime = 0.0;
		int32 NextIndex = 0;
		std::atomic<double> CurrentT{0.0};
		FSlot Slots[2];
		FTSTicker::FDelegateHandle TickerHandle;
		FDelegateHandle EndPIEHandle;
		FDelegateHandle PreExitHandle;
		bool bFixedApplied = false;
		bool bPrevUseFixed = false;
		double PrevFixedDelta = 0.0;
		bool bSyncFallbackNoted = false;
		TMap<TWeakObjectPtr<AActor>, FTracked> TrackedActors;
		bool bFirstSample = true;
		// worker
		TQueue<TUniquePtr<FJob>, EQueueMode::Mpsc> Queue;
		FEvent* WorkerEvent = nullptr;
		FRunnableThread* Thread = nullptr;
		TAtomic<bool> bStopWorker{false};
		TUniquePtr<FArchive> FramesWriter;
		TUniquePtr<FArchive> TimelineWriter;
		// stats
		mutable FCriticalSection StatsLock;
		FStats Stats;
	};

	// -----------------------------------------------------------------------------------------
	// Recorder: at most one session at a time
	// -----------------------------------------------------------------------------------------

	struct FRecorder
	{
		TSharedPtr<FMCPRecordingSession> Active;
		FString LastSessionId;
	};

	FRecorder& Recorder()
	{
		static FRecorder R;
		return R;
	}

	/** The running session, or null. Releases a session that has finished on its own. */
	TSharedPtr<FMCPRecordingSession> ActiveSession()
	{
		FRecorder& R = Recorder();
		if (R.Active && R.Active->IsDone())
		{
			R.LastSessionId = R.Active->Id();
			R.Active.Reset();
		}
		return R.Active;
	}

	bool ReadNumber(const TSharedPtr<FJsonObject>& Params, const TCHAR* Field, double& Out)
	{
		if (Params->HasTypedField<EJson::Number>(Field))
		{
			Out = Params->GetNumberField(Field);
			return true;
		}
		return false;
	}

	// ---- record_pie ----
	TSharedPtr<FJsonObject> HandleRecordPIE(const TSharedPtr<FJsonObject>& Params)
	{
		check(IsInGameThread());
		if (!GEditor)
		{
			return RecError(TEXT("no_editor"), TEXT("No editor available"));
		}
		if (TSharedPtr<FMCPRecordingSession> Active = ActiveSession())
		{
			auto Err = RecError(TEXT("recording_in_progress"), FString::Printf(TEXT("Session %s is still recording"), *Active->Id()),
				TEXT("Call stop_recording, or wait until get_recording_status reports state \"finished\""));
			Err->SetStringField(TEXT("session_id"), Active->Id());
			return Err;
		}
		const UMCPCaptureSettings& Settings = UMCPCaptureSettings::Get();
		FMCPRecordingSession::FConfig Config;
		Config.SessionId = NewSessionId();
		double D = 30.0, Fps = 2.0, Q = Settings.JpegQuality;
		ReadNumber(Params, TEXT("duration_s"), D);
		ReadNumber(Params, TEXT("fps"), Fps);
		ReadNumber(Params, TEXT("quality"), Q);
		if (D <= 0.0 || D > 120.0)
		{
			return RecError(TEXT("invalid_duration"), TEXT("duration_s must be between 0 and 120 seconds"));
		}
		if (Fps <= 0.0 || Fps > 10.0)
		{
			return RecError(TEXT("invalid_fps"), TEXT("fps must be between 0 and 10"));
		}
		Config.DurationS = D;
		Config.Fps = Fps;
		Config.Quality = FMath::Clamp(static_cast<int32>(Q), 1, 100);
		Config.MaxActors = Settings.MaxActors;
		Config.Width = Settings.DefaultWidth;
		Config.Height = Settings.DefaultHeight;
		const TSharedPtr<FJsonObject>* Res = nullptr;
		if (Params->TryGetObjectField(TEXT("resolution"), Res) && Res && Res->IsValid())
		{
			Config.Width = static_cast<int32>((*Res)->GetNumberField(TEXT("w")));
			Config.Height = static_cast<int32>((*Res)->GetNumberField(TEXT("h")));
		}
		if (Config.Width < 16 || Config.Height < 16)
		{
			return RecError(TEXT("invalid_resolution"), TEXT("resolution needs w and h of at least 16"));
		}
		if (FMath::Max(Config.Width, Config.Height) > Settings.MaxLongEdge)
		{
			return RecError(TEXT("resolution_too_large"), FString::Printf(TEXT("Longest edge above the cap of %d"), Settings.MaxLongEdge),
				TEXT("Raise MaxLongEdge in Project Settings > Plugins > MCP Capture"));
		}
		const TArray<TSharedPtr<FJsonValue>>* Filter = nullptr;
		if (Params->TryGetArrayField(TEXT("actor_filter"), Filter) && Filter)
		{
			for (const TSharedPtr<FJsonValue>& V : *Filter)
			{
				FString S;
				if (V.IsValid() && V->TryGetString(S) && !S.IsEmpty())
				{
					Config.ActorFilter.Add(S);
				}
			}
		}
		const bool bStartPIE = !Params->HasTypedField<EJson::Boolean>(TEXT("start_pie")) || Params->GetBoolField(TEXT("start_pie"));

		if (!GEditor->PlayWorld)
		{
			if (!bStartPIE)
			{
				return RecError(TEXT("pie_not_running"), TEXT("No Play-In-Editor session is running"),
					TEXT("Call start_pie first, or pass start_pie: true"));
			}
			FRequestPlaySessionParams PlayParams;
			PlayParams.WorldType = EPlaySessionWorldType::PlayInEditor;
			if (FLevelEditorModule* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor")))
			{
				TSharedPtr<SLevelViewport> ActiveViewport = LevelEditor->GetFirstActiveLevelViewport();
				if (ActiveViewport.IsValid())
				{
					PlayParams.DestinationSlateViewport = ActiveViewport;
				}
			}
			GEditor->RequestPlaySession(PlayParams);
			Config.bStartedPIE = true;
		}

		const FString Dir = MCPRecording::SessionDir(Config.SessionId);
		if (!IFileManager::Get().MakeDirectory(*FPaths::Combine(Dir, TEXT("frames")), true))
		{
			return RecError(TEXT("storage_failed"), FString::Printf(TEXT("Could not create %s"), *Dir),
				TEXT("Check StoragePath in Project Settings > Plugins > MCP Capture"));
		}
		TSharedPtr<FMCPRecordingSession> Session = MakeShared<FMCPRecordingSession>(Config);
		Recorder().Active = Session;
		Session->Begin();

		auto Result = MakeShared<FJsonObject>();
		Result->SetBoolField(TEXT("success"), true);
		Result->SetStringField(TEXT("session_id"), Config.SessionId);
		Result->SetStringField(TEXT("manifest_uri"), ManifestUri(Config.SessionId));
		Result->SetStringField(TEXT("state"), Config.bStartedPIE ? TEXT("starting") : TEXT("recording"));
		Result->SetBoolField(TEXT("started_pie"), Config.bStartedPIE);
		Result->SetNumberField(TEXT("duration_s"), Config.DurationS);
		Result->SetNumberField(TEXT("fps"), Config.Fps);
		Result->SetStringField(TEXT("dir"), Dir);
		return Result;
	}

	/** Resolves the session a query refers to: explicit id, else the active one, else the last. */
	TSharedPtr<FJsonObject> ResolveSessionId(const TSharedPtr<FJsonObject>& Params, FString& OutId, TSharedPtr<FMCPRecordingSession>& OutActive)
	{
		Params->TryGetStringField(TEXT("session_id"), OutId);
		OutActive = ActiveSession();
		if (OutId.IsEmpty())
		{
			OutId = OutActive ? OutActive->Id() : Recorder().LastSessionId;
		}
		if (OutId.IsEmpty())
		{
			return RecError(TEXT("session_not_found"), TEXT("No recording session in this editor run"), TEXT("Call record_pie first, or pass a session_id"));
		}
		if (!MCPRecording::IsValidSessionId(OutId))
		{
			return RecError(TEXT("invalid_session"), TEXT("session_id may contain letters, digits, '-' and '_' only"));
		}
		if (OutActive && OutActive->Id() != OutId)
		{
			OutActive.Reset(); // the query targets a finished session, not the running one
		}
		if (!OutActive && !FPaths::FileExists(ManifestPath(OutId)))
		{
			return RecError(TEXT("session_not_found"), FString::Printf(TEXT("No session %s under %s"), *OutId, *MCPRecording::StorageRoot()));
		}
		return nullptr;
	}

	void MergeInto(const TSharedPtr<FJsonObject>& Target, const TSharedPtr<FJsonObject>& Source)
	{
		for (const auto& Pair : Source->Values)
		{
			Target->SetField(Pair.Key, Pair.Value);
		}
	}

	// ---- get_recording_status ----
	TSharedPtr<FJsonObject> HandleGetRecordingStatus(const TSharedPtr<FJsonObject>& Params)
	{
		FString Id;
		TSharedPtr<FMCPRecordingSession> Active;
		if (auto Err = ResolveSessionId(Params, Id, Active))
		{
			return Err;
		}
		auto Result = MakeShared<FJsonObject>();
		Result->SetBoolField(TEXT("success"), true);
		if (Active)
		{
			MergeInto(Result, Active->BuildManifest());
			Result->SetBoolField(TEXT("active"), true);
			return Result;
		}
		Result->SetBoolField(TEXT("active"), false);
		const FString Path = ManifestPath(Id);
		FMCPTcpServer::QueuePostProcess([Path](TSharedPtr<FJsonObject>& Out)
		{
			TSharedPtr<FJsonObject> Manifest = ReadJsonFile(Path);
			if (!Manifest.IsValid())
			{
				Out = RecError(TEXT("storage_failed"), FString::Printf(TEXT("Could not read %s"), *Path));
				return;
			}
			MergeInto(Out, Manifest);
		});
		return Result;
	}

	// ---- stop_recording ----
	TSharedPtr<FJsonObject> HandleStopRecording(const TSharedPtr<FJsonObject>& Params)
	{
		check(IsInGameThread());
		FString Id;
		TSharedPtr<FMCPRecordingSession> Active;
		if (auto Err = ResolveSessionId(Params, Id, Active))
		{
			return Err;
		}
		if (!Active)
		{
			return RecError(TEXT("not_recording"), FString::Printf(TEXT("Session %s is not recording"), *Id), TEXT("get_recording_status returns its final manifest"));
		}
		Active->Finish(TEXT("stopped"));
		auto Result = MakeShared<FJsonObject>();
		Result->SetBoolField(TEXT("success"), true);
		MergeInto(Result, Active->BuildManifest());
		Result->SetBoolField(TEXT("active"), false);
		Recorder().LastSessionId = Active->Id();
		Recorder().Active.Reset();
		return Result;
	}

	/** The manifest of a session, live from memory or from disk (small file; game thread is fine). */
	TSharedPtr<FJsonObject> LoadManifest(const FString& Id, const TSharedPtr<FMCPRecordingSession>& Active)
	{
		return Active ? Active->BuildManifest() : ReadJsonFile(ManifestPath(Id));
	}

	// ---- get_recording_frames ----
	TSharedPtr<FJsonObject> HandleGetRecordingFrames(const TSharedPtr<FJsonObject>& Params)
	{
		FString Id;
		TSharedPtr<FMCPRecordingSession> Active;
		if (auto Err = ResolveSessionId(Params, Id, Active))
		{
			return Err;
		}
		TSharedPtr<FJsonObject> Manifest = LoadManifest(Id, Active);
		if (!Manifest.IsValid())
		{
			return RecError(TEXT("storage_failed"), TEXT("Could not read the session manifest"));
		}
		const double RecordedFps = FMath::Max(0.01, Manifest->GetNumberField(TEXT("fps")));
		double StartS = 0.0, EndS = Manifest->HasField(TEXT("recorded_s")) ? Manifest->GetNumberField(TEXT("recorded_s")) : Manifest->GetNumberField(TEXT("duration_s"));
		double Fps = RecordedFps, MaxFrames = 8.0;
		ReadNumber(Params, TEXT("start_s"), StartS);
		ReadNumber(Params, TEXT("end_s"), EndS);
		ReadNumber(Params, TEXT("fps"), Fps);
		ReadNumber(Params, TEXT("max_frames"), MaxFrames);
		if (StartS < 0.0 || EndS < StartS)
		{
			return RecError(TEXT("invalid_range"), TEXT("start_s must be >= 0 and end_s >= start_s"));
		}
		if (Fps <= 0.0)
		{
			return RecError(TEXT("invalid_fps"), TEXT("fps must be positive"));
		}
		const int32 Cap = FMath::Clamp(static_cast<int32>(MaxFrames), 1, 32);
		const FString Index = FramesIndexPath(MCPRecording::SessionDir(Id));
		const FString Dir = MCPRecording::SessionDir(Id);

		auto Result = MakeShared<FJsonObject>();
		Result->SetBoolField(TEXT("success"), true);
		Result->SetStringField(TEXT("session_id"), Id);
		Result->SetNumberField(TEXT("recorded_fps"), RecordedFps);
		Result->SetNumberField(TEXT("requested_fps"), Fps);
		Result->SetNumberField(TEXT("start_s"), StartS);
		Result->SetNumberField(TEXT("end_s"), EndS);
		Result->SetStringField(TEXT("state"), Manifest->GetStringField(TEXT("state")));

		// Everything below streams from disk on the socket thread; the game thread is done.
		FMCPTcpServer::QueuePostProcess([Index, Dir, StartS, EndS, Fps, RecordedFps, Cap](TSharedPtr<FJsonObject>& Out)
		{
			check(!IsInGameThread());
			// Pass 1: the (t, index) of every recorded frame in range. Small.
			struct FEntry { double T; int32 Index; };
			TArray<FEntry> Entries;
			const double Tol = FMath::Min(0.5 / Fps, 0.5 / RecordedFps) + 0.01;
			MCPRecording::ForEachJsonLine(Index, [&](const TSharedPtr<FJsonObject>& Rec)
			{
				const double T = Rec->GetNumberField(TEXT("t"));
				if (T >= StartS - Tol && T <= EndS + Tol)
				{
					Entries.Add({ T, static_cast<int32>(Rec->GetNumberField(TEXT("index"))) });
				}
				return T <= EndS + Tol; // the index is time-ordered: stop once past the range
			});
			// Targets on the requested grid, matched to the nearest unused recorded frame.
			struct FTarget { double T; int32 FrameIndex; };
			TArray<FTarget> Targets;
			TSet<int32> Used;
			bool bTruncated = false;
			for (int32 k = 0; ; ++k)
			{
				const double Tk = StartS + k / Fps;
				if (Tk > EndS + KINDA_SMALL_NUMBER)
				{
					break;
				}
				if (Targets.Num() >= Cap)
				{
					bTruncated = true;
					break;
				}
				int32 Best = -1;
				double BestDist = Tol;
				for (const FEntry& E : Entries)
				{
					const double Dist = FMath::Abs(E.T - Tk);
					if (Dist <= BestDist && !Used.Contains(E.Index))
					{
						Best = E.Index;
						BestDist = Dist;
					}
				}
				if (Best >= 0)
				{
					Used.Add(Best);
				}
				Targets.Add({ Tk, Best });
			}
			// Pass 2: the full records of the selected frames only.
			TMap<int32, TSharedPtr<FJsonObject>> Selected;
			if (Used.Num() > 0)
			{
				MCPRecording::ForEachJsonLine(Index, [&](const TSharedPtr<FJsonObject>& Rec)
				{
					const int32 I = static_cast<int32>(Rec->GetNumberField(TEXT("index")));
					if (Used.Contains(I))
					{
						Selected.Add(I, Rec);
					}
					return Selected.Num() < Used.Num();
				});
			}
			TArray<TSharedPtr<FJsonValue>> Frames;
			int32 Available = 0;
			for (const FTarget& Target : Targets)
			{
				auto F = MakeShared<FJsonObject>();
				F->SetNumberField(TEXT("t"), Target.T);
				const TSharedPtr<FJsonObject>* Rec = Target.FrameIndex >= 0 ? Selected.Find(Target.FrameIndex) : nullptr;
				TArray<uint8> Bytes;
				if (Rec && FFileHelper::LoadFileToArray(Bytes, *FPaths::Combine(Dir, (*Rec)->GetStringField(TEXT("file")))))
				{
					F->SetBoolField(TEXT("available"), true);
					F->SetNumberField(TEXT("index"), Target.FrameIndex);
					F->SetNumberField(TEXT("recorded_t"), (*Rec)->GetNumberField(TEXT("t")));
					F->SetNumberField(TEXT("world_time"), (*Rec)->GetNumberField(TEXT("world_time")));
					auto Image = MakeShared<FJsonObject>();
					Image->SetStringField(TEXT("format"), TEXT("jpeg"));
					Image->SetStringField(TEXT("mime_type"), TEXT("image/jpeg"));
					Image->SetNumberField(TEXT("width"), (*Rec)->GetNumberField(TEXT("width")));
					Image->SetNumberField(TEXT("height"), (*Rec)->GetNumberField(TEXT("height")));
					Image->SetNumberField(TEXT("bytes"), Bytes.Num());
					Image->SetStringField(TEXT("data"), FBase64::Encode(Bytes));
					F->SetObjectField(TEXT("image"), Image);
					const TArray<TSharedPtr<FJsonValue>>* Actors = nullptr;
					F->SetArrayField(TEXT("actors"), (*Rec)->TryGetArrayField(TEXT("actors"), Actors) && Actors ? *Actors : TArray<TSharedPtr<FJsonValue>>());
					++Available;
				}
				else
				{
					F->SetBoolField(TEXT("available"), false);
					F->SetStringField(TEXT("reason"), Rec ? TEXT("frame file missing") : TEXT("not available: no recorded frame near this time"));
				}
				Frames.Add(MakeShared<FJsonValueObject>(F));
			}
			Out->SetArrayField(TEXT("frames"), Frames);
			Out->SetNumberField(TEXT("available"), Available);
			Out->SetBoolField(TEXT("truncated"), bTruncated);
		});
		return Result;
	}

	// ---- get_recording_timeline ----
	TSharedPtr<FJsonObject> HandleGetRecordingTimeline(const TSharedPtr<FJsonObject>& Params)
	{
		FString Id;
		TSharedPtr<FMCPRecordingSession> Active;
		if (auto Err = ResolveSessionId(Params, Id, Active))
		{
			return Err;
		}
		double StartS = 0.0, EndS = TNumericLimits<double>::Max();
		ReadNumber(Params, TEXT("start_s"), StartS);
		ReadNumber(Params, TEXT("end_s"), EndS);
		if (StartS < 0.0 || EndS < StartS)
		{
			return RecError(TEXT("invalid_range"), TEXT("start_s must be >= 0 and end_s >= start_s"));
		}
		const FString Path = TimelinePath(MCPRecording::SessionDir(Id));
		auto Result = MakeShared<FJsonObject>();
		Result->SetBoolField(TEXT("success"), true);
		Result->SetStringField(TEXT("session_id"), Id);
		Result->SetNumberField(TEXT("start_s"), StartS);
		if (EndS < TNumericLimits<double>::Max())
		{
			Result->SetNumberField(TEXT("end_s"), EndS);
		}
		FMCPTcpServer::QueuePostProcess([Path, StartS, EndS](TSharedPtr<FJsonObject>& Out)
		{
			check(!IsInGameThread());
			const int32 LogCap = 2000, ActorCap = 2000, StatsCap = 4000;
			TArray<TSharedPtr<FJsonValue>> Log, ActorEvents, Stats;
			bool bLogTrunc = false, bActorTrunc = false, bStatsTrunc = false;
			MCPRecording::ForEachJsonLine(Path, [&](const TSharedPtr<FJsonObject>& Rec)
			{
				const double T = Rec->GetNumberField(TEXT("t"));
				if (T < StartS || T > EndS)
				{
					return true; // records are appended as they happen, but log lines from other threads may lag: keep scanning
				}
				const FString Kind = Rec->GetStringField(TEXT("kind"));
				Rec->RemoveField(TEXT("kind"));
				auto Value = MakeShared<FJsonValueObject>(Rec);
				if (Kind == TEXT("log")) { if (Log.Num() < LogCap) Log.Add(Value); else bLogTrunc = true; }
				else if (Kind == TEXT("actor")) { if (ActorEvents.Num() < ActorCap) ActorEvents.Add(Value); else bActorTrunc = true; }
				else if (Kind == TEXT("stats")) { if (Stats.Num() < StatsCap) Stats.Add(Value); else bStatsTrunc = true; }
				return true;
			});
			Out->SetArrayField(TEXT("log"), Log);
			Out->SetArrayField(TEXT("actor_events"), ActorEvents);
			Out->SetArrayField(TEXT("stats"), Stats);
			auto Trunc = MakeShared<FJsonObject>();
			Trunc->SetBoolField(TEXT("log"), bLogTrunc);
			Trunc->SetBoolField(TEXT("actor_events"), bActorTrunc);
			Trunc->SetBoolField(TEXT("stats"), bStatsTrunc);
			Out->SetObjectField(TEXT("truncated"), Trunc);
		});
		return Result;
	}
}

int32 MCPRecording::PruneOldSessions()
{
	const int32 Days = UMCPCaptureSettings::Get().RetentionDays;
	if (Days <= 0)
	{
		return 0;
	}
	const FString Root = StorageRoot();
	TArray<FString> Dirs;
	IFileManager::Get().FindFiles(Dirs, *FPaths::Combine(Root, TEXT("*")), /*Files*/ false, /*Directories*/ true);
	const FDateTime Cutoff = FDateTime::UtcNow() - FTimespan::FromDays(Days);
	int32 Removed = 0;
	for (const FString& Name : Dirs)
	{
		const FString Dir = FPaths::Combine(Root, Name);
		const FString Manifest = FPaths::Combine(Dir, TEXT("manifest.json"));
		if (!FPaths::FileExists(Manifest))
		{
			continue; // not one of ours
		}
		if (IFileManager::Get().GetTimeStamp(*Manifest) < Cutoff && IFileManager::Get().DeleteDirectory(*Dir, false, true))
		{
			++Removed;
		}
	}
	if (Removed > 0)
	{
		UE_LOG(LogMCPRecording, Log, TEXT("[UnrealMCP] Pruned %d recording session(s) older than %d days"), Removed, Days);
	}
	return Removed;
}

void MCPRecording::Shutdown()
{
	FRecorder& R = Recorder();
	if (R.Active)
	{
		if (IsInGameThread())
		{
			R.Active->Finish(TEXT("shutdown"));
		}
		R.LastSessionId = R.Active->Id();
		R.Active.Reset();
	}
}

void MCPRecording::RegisterHandlers(FMCPTcpServer& Server)
{
	Server.RegisterHandler(TEXT("record_pie"), [](const TSharedPtr<FJsonObject>& Params) { return HandleRecordPIE(Params); });
	Server.RegisterHandler(TEXT("get_recording_status"), [](const TSharedPtr<FJsonObject>& Params) { return HandleGetRecordingStatus(Params); });
	Server.RegisterHandler(TEXT("stop_recording"), [](const TSharedPtr<FJsonObject>& Params) { return HandleStopRecording(Params); });
	Server.RegisterHandler(TEXT("get_recording_frames"), [](const TSharedPtr<FJsonObject>& Params) { return HandleGetRecordingFrames(Params); });
	Server.RegisterHandler(TEXT("get_recording_timeline"), [](const TSharedPtr<FJsonObject>& Params) { return HandleGetRecordingTimeline(Params); });
	PruneOldSessions();
}
