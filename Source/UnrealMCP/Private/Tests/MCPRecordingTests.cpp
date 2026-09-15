// Copyright (c) 2026 victorvksy. All rights reserved.

// PIE recording through the real socket:
//  - Contract: the structured errors (no PIE, bad ranges, unknown/invalid session ids).
//  - QueryPartialSession: a session written by hand in the "recording" state is queried for
//    frames (grid, gaps, cap) and timeline (range filter) - the partial-session guarantee.
//  - PIESession: starts PIE, records 4 s at 2 fps, waits for the session to finish on its own,
//    checks frames and timeline, and logs the game-thread cost per captured frame.
//
//   Automation RunTests UnrealMCP.Recording

#include "Tests/MCPTestClient.h"
#include "Capture/MCPRecording.h"
#include "Capture/MCPCapture.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	TSharedPtr<FJsonObject> Call(int32 Port, const FString& Command, const TSharedPtr<FJsonObject>& Params, double TimeoutSec = 30.0)
	{
		return MCPTestClient::Parse(MCPTestClient::Exchange(Port, MCPTestClient::Request(Command, Params), TimeoutSec));
	}

	TSharedPtr<FJsonObject> Obj() { return MakeShared<FJsonObject>(); }

	TSharedPtr<FJsonObject> WithSession(const FString& Id)
	{
		auto P = Obj();
		P->SetStringField(TEXT("session_id"), Id);
		return P;
	}

	bool IsJpeg(const FString& Base64)
	{
		TArray<uint8> Bytes;
		return FBase64::Decode(Base64, Bytes) && Bytes.Num() > 3 && Bytes[0] == 0xFF && Bytes[1] == 0xD8;
	}
}

// ---------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCPRecordingContractTest, "UnrealMCP.Recording.Contract", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMCPRecordingContractTest::RunTest(const FString& Parameters)
{
	FMCPTcpServer* Server = MCPTestClient::Server(*this);
	if (!Server) return false;
	const int32 Port = Server->GetPort();
	const bool bPIERunning = GEditor && GEditor->PlayWorld;
	auto Scenario = MakeShared<MCPTestClient::FScenario>();
	Scenario->Deadline = FPlatformTime::Seconds() + 60.0;
	Scenario->Future = Async(EAsyncExecution::Thread, [Port, Scenario, bPIERunning]() -> bool
	{
		auto Expect = [&](const TCHAR* What, const TSharedPtr<FJsonObject>& Resp, const TCHAR* Code)
		{
			const FString Got = MCPTestClient::ErrorCode(Resp);
			if (Got != Code)
			{
				Scenario->Errors.Add(FString::Printf(TEXT("%s: expected %s, got '%s'"), What, Code, *Got));
			}
		};
		{
			auto P = Obj();
			P->SetNumberField(TEXT("fps"), 50);
			P->SetBoolField(TEXT("start_pie"), false);
			Expect(TEXT("fps 50"), Call(Port, TEXT("record_pie"), P), TEXT("invalid_fps"));
		}
		{
			auto P = Obj();
			P->SetNumberField(TEXT("duration_s"), 500);
			P->SetBoolField(TEXT("start_pie"), false);
			Expect(TEXT("duration 500"), Call(Port, TEXT("record_pie"), P), TEXT("invalid_duration"));
		}
		if (!bPIERunning)
		{
			auto P = Obj();
			P->SetBoolField(TEXT("start_pie"), false);
			Expect(TEXT("record without PIE"), Call(Port, TEXT("record_pie"), P), TEXT("pie_not_running"));
		}
		Expect(TEXT("unknown session"), Call(Port, TEXT("get_recording_status"), WithSession(TEXT("nope-000000"))), TEXT("session_not_found"));
		Expect(TEXT("path in session id"), Call(Port, TEXT("get_recording_frames"), WithSession(TEXT("../etc"))), TEXT("invalid_session"));
		Expect(TEXT("unknown session frames"), Call(Port, TEXT("get_recording_frames"), WithSession(TEXT("nope-000000"))), TEXT("session_not_found"));
		Expect(TEXT("unknown session stop"), Call(Port, TEXT("stop_recording"), WithSession(TEXT("nope-000000"))), TEXT("session_not_found"));
		return Scenario->Errors.Num() == 0;
	});
	ADD_LATENT_AUTOMATION_COMMAND(FMCPWaitForScenario(Scenario, this));
	return true;
}

// ---------------------------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCPRecordingPartialQueryTest, "UnrealMCP.Recording.QueryPartialSession", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMCPRecordingPartialQueryTest::RunTest(const FString& Parameters)
{
	FMCPTcpServer* Server = MCPTestClient::Server(*this);
	if (!Server) return false;
	const int32 Port = Server->GetPort();

	// A session in the "recording" state, as the worker would leave it mid-way: three frames at
	// 2 fps, a manifest, and a timeline with one log line, one actor event and three stats.
	const FString Id = TEXT("test-partial-") + FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(6).ToLower();
	const FString Dir = MCPRecording::SessionDir(Id);
	if (!IFileManager::Get().MakeDirectory(*FPaths::Combine(Dir, TEXT("frames")), true))
	{
		AddError(FString::Printf(TEXT("could not create %s"), *Dir));
		return false;
	}
	TArray<FColor> Pixels;
	Pixels.Init(FColor::Red, 16 * 16);
	TArray64<uint8> Jpeg;
	if (!MCPCapture::EncodeImage(Pixels, 16, 16, TEXT("jpeg"), 80, Jpeg))
	{
		AddError(TEXT("jpeg encode failed"));
		return false;
	}
	FString FramesIndex, Timeline;
	for (int32 i = 0; i < 3; ++i)
	{
		const FString File = FString::Printf(TEXT("frames/%06d.jpg"), i);
		FFileHelper::SaveArrayToFile(TArrayView<const uint8>(Jpeg.GetData(), static_cast<int32>(Jpeg.Num())), *FPaths::Combine(Dir, File));
		auto Rec = Obj();
		Rec->SetNumberField(TEXT("index"), i);
		Rec->SetNumberField(TEXT("t"), i * 0.5);
		Rec->SetNumberField(TEXT("world_time"), 100.0 + i * 0.5);
		Rec->SetStringField(TEXT("file"), File);
		Rec->SetNumberField(TEXT("width"), 16);
		Rec->SetNumberField(TEXT("height"), 16);
		Rec->SetNumberField(TEXT("bytes"), Jpeg.Num());
		TArray<TSharedPtr<FJsonValue>> Actors;
		auto A = Obj();
		A->SetStringField(TEXT("name"), TEXT("TestPawn"));
		Actors.Add(MakeShared<FJsonValueObject>(A));
		Rec->SetArrayField(TEXT("actors"), Actors);
		FramesIndex += MCPRecording::ToJsonLine(Rec) + TEXT("\n");

		auto St = Obj();
		St->SetNumberField(TEXT("t"), i * 0.5);
		St->SetStringField(TEXT("kind"), TEXT("stats"));
		St->SetNumberField(TEXT("fps"), 60);
		St->SetNumberField(TEXT("frame_ms"), 16.6);
		St->SetNumberField(TEXT("draw_calls"), 100);
		Timeline += MCPRecording::ToJsonLine(St) + TEXT("\n");
	}
	{
		auto L = Obj();
		L->SetNumberField(TEXT("t"), 0.2);
		L->SetStringField(TEXT("kind"), TEXT("log"));
		L->SetStringField(TEXT("verbosity"), TEXT("Warning"));
		L->SetStringField(TEXT("category"), TEXT("LogTemp"));
		L->SetStringField(TEXT("message"), TEXT("hello"));
		Timeline += MCPRecording::ToJsonLine(L) + TEXT("\n");
		auto E = Obj();
		E->SetNumberField(TEXT("t"), 0.6);
		E->SetStringField(TEXT("kind"), TEXT("actor"));
		E->SetStringField(TEXT("actor"), TEXT("TestPawn"));
		E->SetStringField(TEXT("event"), TEXT("spawned"));
		Timeline += MCPRecording::ToJsonLine(E) + TEXT("\n");
		Timeline += TEXT("{\"t\": 0.9, \"kind\": \"stats\", \"fps\": 6"); // an unterminated line still being written
	}
	FFileHelper::SaveStringToFile(FramesIndex, *FPaths::Combine(Dir, TEXT("frames.jsonl")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	FFileHelper::SaveStringToFile(Timeline, *FPaths::Combine(Dir, TEXT("timeline.jsonl")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	auto Manifest = Obj();
	Manifest->SetStringField(TEXT("session_id"), Id);
	Manifest->SetStringField(TEXT("state"), TEXT("recording"));
	Manifest->SetNumberField(TEXT("fps"), 2);
	Manifest->SetNumberField(TEXT("duration_s"), 10);
	Manifest->SetNumberField(TEXT("recorded_s"), 1.0);
	Manifest->SetNumberField(TEXT("world_time_start"), 100.0);
	Manifest->SetNumberField(TEXT("frames"), 3);
	FFileHelper::SaveStringToFile(MCPRecording::ToJsonLine(Manifest), *FPaths::Combine(Dir, TEXT("manifest.json")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	auto Scenario = MakeShared<MCPTestClient::FScenario>();
	Scenario->Deadline = FPlatformTime::Seconds() + 60.0;
	Scenario->Future = Async(EAsyncExecution::Thread, [Port, Scenario, Id, Dir]() -> bool
	{
		auto Fail = [&](const FString& Msg) { Scenario->Errors.Add(Msg); };
		// Frames on a 4 fps grid over a 2 fps recording: 0, 0.5 and 1.0 exist; 0.25 and 0.75 do not.
		{
			auto P = WithSession(Id);
			P->SetNumberField(TEXT("start_s"), 0);
			P->SetNumberField(TEXT("end_s"), 1);
			P->SetNumberField(TEXT("fps"), 4);
			P->SetNumberField(TEXT("max_frames"), 8);
			auto R = Call(Port, TEXT("get_recording_frames"), P);
			const TArray<TSharedPtr<FJsonValue>>* Frames = nullptr;
			if (!R.IsValid() || !R->GetBoolField(TEXT("success")) || !R->TryGetArrayField(TEXT("frames"), Frames) || !Frames)
			{
				Fail(TEXT("frames query failed: ") + (R.IsValid() ? MCPTestClient::ErrorCode(R) : TEXT("no response")));
			}
			else
			{
				if (Frames->Num() != 5) Fail(FString::Printf(TEXT("expected 5 grid targets, got %d"), Frames->Num()));
				const bool ExpectAvail[5] = { true, false, true, false, true };
				for (int32 i = 0; i < FMath::Min(5, Frames->Num()); ++i)
				{
					const TSharedPtr<FJsonObject> F = (*Frames)[i]->AsObject();
					const bool bAvail = F->GetBoolField(TEXT("available"));
					if (bAvail != ExpectAvail[i]) Fail(FString::Printf(TEXT("frame target %d: available=%d, expected %d"), i, bAvail, ExpectAvail[i]));
					if (bAvail)
					{
						const TSharedPtr<FJsonObject>* Image = nullptr;
						if (!F->TryGetObjectField(TEXT("image"), Image) || !IsJpeg((*Image)->GetStringField(TEXT("data")))) Fail(FString::Printf(TEXT("frame target %d: image is not a JPEG"), i));
						const TArray<TSharedPtr<FJsonValue>>* Actors = nullptr;
						if (!F->TryGetArrayField(TEXT("actors"), Actors) || Actors->Num() != 1) Fail(FString::Printf(TEXT("frame target %d: actors missing"), i));
					}
				}
				if (static_cast<int32>(R->GetNumberField(TEXT("available"))) != 3) Fail(TEXT("available count should be 3"));
				if (R->GetBoolField(TEXT("truncated"))) Fail(TEXT("should not be truncated"));
			}
		}
		// max_frames caps the grid and reports it.
		{
			auto P = WithSession(Id);
			P->SetNumberField(TEXT("start_s"), 0);
			P->SetNumberField(TEXT("end_s"), 1);
			P->SetNumberField(TEXT("max_frames"), 2);
			auto R = Call(Port, TEXT("get_recording_frames"), P);
			const TArray<TSharedPtr<FJsonValue>>* Frames = nullptr;
			if (!R.IsValid() || !R->TryGetArrayField(TEXT("frames"), Frames) || Frames->Num() != 2 || !R->GetBoolField(TEXT("truncated")))
			{
				Fail(TEXT("max_frames 2 should return 2 frames and truncated=true"));
			}
		}
		// Timeline range filter; the unterminated tail line is ignored.
		{
			auto P = WithSession(Id);
			P->SetNumberField(TEXT("start_s"), 0.5);
			P->SetNumberField(TEXT("end_s"), 1.0);
			auto R = Call(Port, TEXT("get_recording_timeline"), P);
			const TArray<TSharedPtr<FJsonValue>>* Log = nullptr; const TArray<TSharedPtr<FJsonValue>>* Events = nullptr; const TArray<TSharedPtr<FJsonValue>>* Stats = nullptr;
			if (!R.IsValid() || !R->GetBoolField(TEXT("success")) || !R->TryGetArrayField(TEXT("log"), Log) || !R->TryGetArrayField(TEXT("actor_events"), Events) || !R->TryGetArrayField(TEXT("stats"), Stats))
			{
				Fail(TEXT("timeline query failed: ") + (R.IsValid() ? MCPTestClient::ErrorCode(R) : TEXT("no response")));
			}
			else
			{
				if (Log->Num() != 0) Fail(FString::Printf(TEXT("log at 0.2 s should be outside [0.5, 1.0]; got %d"), Log->Num()));
				if (Events->Num() != 1) Fail(FString::Printf(TEXT("expected 1 actor event, got %d"), Events->Num()));
				if (Stats->Num() != 2) Fail(FString::Printf(TEXT("expected 2 stats records, got %d"), Stats->Num()));
			}
		}
		// Status reads the manifest from disk; stop refuses a session that is not recording.
		{
			auto R = Call(Port, TEXT("get_recording_status"), WithSession(Id));
			if (!R.IsValid() || R->GetStringField(TEXT("state")) != TEXT("recording") || R->GetBoolField(TEXT("active"))) Fail(TEXT("status should report state=recording, active=false"));
			if (MCPTestClient::ErrorCode(Call(Port, TEXT("stop_recording"), WithSession(Id))) != TEXT("not_recording")) Fail(TEXT("stop_recording on a finished session should be not_recording"));
		}
		IFileManager::Get().DeleteDirectory(*Dir, false, true);
		return Scenario->Errors.Num() == 0;
	});
	ADD_LATENT_AUTOMATION_COMMAND(FMCPWaitForScenario(Scenario, this));
	return true;
}

// ---------------------------------------------------------------------------------------------

namespace
{
	struct FPIEScenarioState
	{
		int32 Port = 0;
		bool bStarted = false;
		double Deadline = 0.0;
		TFuture<bool> Future;
		TArray<FString> Errors;
		TArray<FString> Infos;
	};

	bool RunPIEScenario(int32 Port, TArray<FString>& Errors, TArray<FString>& Infos)
	{
		auto Fail = [&](const FString& Msg) { Errors.Add(Msg); };
		auto P = Obj();
		P->SetBoolField(TEXT("start_pie"), false);
		P->SetNumberField(TEXT("duration_s"), 4);
		P->SetNumberField(TEXT("fps"), 2);
		auto Res = Obj();
		Res->SetNumberField(TEXT("w"), 320);
		Res->SetNumberField(TEXT("h"), 180);
		P->SetObjectField(TEXT("resolution"), Res);
		auto Started = Call(Port, TEXT("record_pie"), P);
		if (!Started.IsValid() || !Started->GetBoolField(TEXT("success")))
		{
			Fail(TEXT("record_pie failed: ") + (Started.IsValid() ? MCPTestClient::ErrorCode(Started) : TEXT("no response")));
			return false;
		}
		const FString Id = Started->GetStringField(TEXT("session_id"));
		Infos.Add(FString::Printf(TEXT("session %s started"), *Id));

		// The session ends on its own after 4 s of world time; poll until it says so.
		TSharedPtr<FJsonObject> Status;
		const double PollDeadline = FPlatformTime::Seconds() + 90.0;
		while (FPlatformTime::Seconds() < PollDeadline)
		{
			FPlatformProcess::Sleep(0.5f);
			Status = Call(Port, TEXT("get_recording_status"), WithSession(Id));
			if (Status.IsValid() && Status->GetStringField(TEXT("state")) == TEXT("finished"))
			{
				break;
			}
		}
		if (!Status.IsValid() || Status->GetStringField(TEXT("state")) != TEXT("finished"))
		{
			Fail(TEXT("session did not finish within 90 s"));
			Call(Port, TEXT("stop_recording"), WithSession(Id));
			return false;
		}
		const int32 Frames = static_cast<int32>(Status->GetNumberField(TEXT("frames")));
		const TSharedPtr<FJsonObject>* Timings = nullptr;
		if (Status->TryGetObjectField(TEXT("timings"), Timings) && Timings)
		{
			Infos.Add(FString::Printf(TEXT("[UnrealMCP.Recording] %d frames (%d dropped), game thread per captured frame avg %.2f ms max %.2f ms, worker encode avg %.1f ms, source %s, end_reason %s"),
				Frames, static_cast<int32>(Status->GetNumberField(TEXT("dropped_frames"))),
				(*Timings)->GetNumberField(TEXT("game_thread_ms_avg")), (*Timings)->GetNumberField(TEXT("game_thread_ms_max")),
				(*Timings)->GetNumberField(TEXT("worker_encode_ms_avg")), *Status->GetStringField(TEXT("source_format")), *Status->GetStringField(TEXT("end_reason"))));
		}
		if (Status->GetStringField(TEXT("end_reason")) != TEXT("duration")) Fail(TEXT("end_reason should be duration, got ") + Status->GetStringField(TEXT("end_reason")));
		if (Frames < 3) Fail(FString::Printf(TEXT("expected at least 3 frames in 4 s at 2 fps, got %d"), Frames));
		if (Status->GetBoolField(TEXT("active"))) Fail(TEXT("finished session reported active"));

		auto FP = WithSession(Id);
		FP->SetNumberField(TEXT("start_s"), 0);
		FP->SetNumberField(TEXT("end_s"), 4);
		FP->SetNumberField(TEXT("max_frames"), 16);
		auto FrameResp = Call(Port, TEXT("get_recording_frames"), FP, 60.0);
		const TArray<TSharedPtr<FJsonValue>>* FrameList = nullptr;
		if (!FrameResp.IsValid() || !FrameResp->GetBoolField(TEXT("success")) || !FrameResp->TryGetArrayField(TEXT("frames"), FrameList))
		{
			Fail(TEXT("get_recording_frames failed: ") + (FrameResp.IsValid() ? MCPTestClient::ErrorCode(FrameResp) : TEXT("no response")));
		}
		else
		{
			int32 Decodable = 0;
			for (const TSharedPtr<FJsonValue>& V : *FrameList)
			{
				const TSharedPtr<FJsonObject> F = V->AsObject();
				const TSharedPtr<FJsonObject>* Image = nullptr;
				if (F->GetBoolField(TEXT("available")) && F->TryGetObjectField(TEXT("image"), Image) && IsJpeg((*Image)->GetStringField(TEXT("data"))))
				{
					++Decodable;
				}
			}
			if (Decodable < 3) Fail(FString::Printf(TEXT("expected at least 3 decodable frames, got %d"), Decodable));
			Infos.Add(FString::Printf(TEXT("%d of %d grid frames available"), static_cast<int32>(FrameResp->GetNumberField(TEXT("available"))), FrameList->Num()));
		}
		auto TL = Call(Port, TEXT("get_recording_timeline"), WithSession(Id), 60.0);
		const TArray<TSharedPtr<FJsonValue>>* Stats = nullptr;
		if (!TL.IsValid() || !TL->GetBoolField(TEXT("success")) || !TL->TryGetArrayField(TEXT("stats"), Stats) || Stats->Num() < 3)
		{
			Fail(TEXT("timeline should carry at least 3 stats records"));
		}
		return Errors.Num() == 0;
	}
}

// Waits for the PIE player, runs the socket scenario on a worker, then reports.
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FMCPRecordingPIEScenario, TSharedPtr<FPIEScenarioState>, State, FAutomationTestBase*, Test);
bool FMCPRecordingPIEScenario::Update()
{
	if (!State->bStarted)
	{
		UWorld* World = GEditor ? GEditor->PlayWorld.Get() : nullptr;
		const bool bReady = World && UGameplayStatics::GetPlayerController(World, 0) && GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport;
		if (!bReady)
		{
			if (FPlatformTime::Seconds() > State->Deadline)
			{
				Test->AddError(TEXT("PIE did not produce a player in time"));
				return true;
			}
			return false;
		}
		State->bStarted = true;
		State->Deadline = FPlatformTime::Seconds() + 150.0;
		TSharedPtr<FPIEScenarioState> S = State;
		State->Future = Async(EAsyncExecution::Thread, [S]() -> bool { return RunPIEScenario(S->Port, S->Errors, S->Infos); });
		return false;
	}
	if (!State->Future.IsReady())
	{
		if (FPlatformTime::Seconds() > State->Deadline)
		{
			Test->AddError(TEXT("recording scenario timed out"));
			return true;
		}
		return false;
	}
	for (const FString& Info : State->Infos)
	{
		UE_LOG(LogTemp, Display, TEXT("%s"), *Info);
		Test->AddInfo(Info);
	}
	for (const FString& Error : State->Errors)
	{
		Test->AddError(Error);
	}
	return true;
}

// The game under PIE logs whatever it logs (a project's own errors are not this plugin's
// failures), so this test does not fail on log errors or warnings; it asserts on the session.
class FMCPRecordingPIETestBase : public FAutomationTestBase
{
public:
	FMCPRecordingPIETestBase(const FString& InName, const bool bInComplexTask) : FAutomationTestBase(InName, bInComplexTask) {}
	virtual bool SuppressLogErrors() override { return true; }
	virtual bool SuppressLogWarnings() override { return true; }
};

IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FMCPRecordingPIESessionTest, FMCPRecordingPIETestBase, "UnrealMCP.Recording.PIESession", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMCPRecordingPIESessionTest::RunTest(const FString& Parameters)
{
	FMCPTcpServer* Server = MCPTestClient::Server(*this);
	if (!Server) return false;
	if (GEditor && GEditor->PlayWorld)
	{
		AddInfo(TEXT("PIE is already running; skipping the recording session test"));
		return true;
	}
	auto State = MakeShared<FPIEScenarioState>();
	State->Port = Server->GetPort();
	State->Deadline = FPlatformTime::Seconds() + 180.0;
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FMCPRecordingPIEScenario(State, this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
