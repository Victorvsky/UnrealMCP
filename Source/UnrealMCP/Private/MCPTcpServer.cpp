// Copyright (c) 2026 victorvksy. All rights reserved.

#include "MCPTcpServer.h"
#include "Common/TcpSocketBuilder.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_BreakStruct.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_GetArrayItem.h"
#include "K2Node_MakeArray.h"
#include "K2Node_SpawnActorFromClass.h"
#include "K2Node_Select.h"
#include "K2Node_SwitchEnum.h"
#include "K2Node_SwitchInteger.h"
#include "K2Node_SwitchString.h"
#include "K2Node_SwitchName.h"
#include "K2Node_ForEachElementInEnum.h"
#include "K2Node_MacroInstance.h"
#include "K2Node_Composite.h"
#include "K2Node_TemporaryVariable.h"
#include "K2Node_Timeline.h"
#include "K2Node_AddDelegate.h"
#include "K2Node_CallDelegate.h"
#include "K2Node_ClearDelegate.h"
#include "K2Node_RemoveDelegate.h"
#include "K2Node_CreateDelegate.h"
#include "K2Node_AssignDelegate.h"
#include "K2Node_ComponentBoundEvent.h"
#include "K2Node_AddComponent.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Components/Widget.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "WidgetBlueprint.h"
#include "Components/PanelWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/SizeBox.h"
#include "ObjectTools.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Factories/BlueprintFactory.h"
#include "FileHelpers.h"
#include "EditorAssetLibrary.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "WidgetBlueprintFactory.h"
#include "Engine/DataTable.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/OverlaySlot.h"
#include "Components/BorderSlot.h"
#include "Components/SizeBoxSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Animation/WidgetAnimation.h"
#include "K2Node_Tunnel.h"
#include "UObject/SavePackage.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#if WITH_NIAGARA
#include "NiagaraSystem.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraScript.h"
#include "NiagaraTypes.h"
#include "NiagaraCommon.h"
#include "NiagaraEmitter.h"
#include "NiagaraScriptSource.h"
#include "NiagaraGraph.h"
#include "NiagaraNodeFunctionCall.h"
#include "NiagaraNodeOutput.h"
#endif
#include "LevelSequence.h"
#include "MovieScene.h"
#include "MovieSceneTrack.h"
#include "MovieSceneSection.h"
#include "Tracks/MovieSceneFloatTrack.h"
#include "Tracks/MovieSceneBoolTrack.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Tracks/MovieSceneVisibilityTrack.h"
#include "Tracks/MovieSceneEventTrack.h"
#include "Tracks/MovieSceneAudioTrack.h"
#include "Tracks/MovieSceneSkeletalAnimationTrack.h"
#include "Tracks/MovieSceneSubTrack.h"
#include "Tracks/MovieSceneCameraCutTrack.h"
#include "Tracks/MovieSceneFadeTrack.h"
#include "Sections/MovieSceneFloatSection.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectIterator.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeComponent.h"
#include "LandscapeEdit.h"
#include "LandscapeEditorUtils.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEditLayer.h"
#include "InstancedFoliageActor.h"
#include "FoliageType_InstancedStaticMesh.h"
#include "Engine/StaticMesh.h"
#include "LevelEditorViewport.h"
#include "SLevelViewport.h"
#include "LevelEditor.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/Base64.h"
#include "Templates/Atomic.h"
#include "HAL/ThreadSafeCounter.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/RenderingCommon.h"
#include "Materials/MaterialInstanceConstant.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/Character.h"

DEFINE_LOG_CATEGORY(LogUnrealMCP);

// Helper: create a success response
static TSharedPtr<FJsonObject> MCPSuccess()
{
	auto R = MakeShared<FJsonObject>();
	R->SetBoolField(TEXT("success"), true);
	return R;
}

// Helper: create an error response
static TSharedPtr<FJsonObject> MCPError(const FString& Message)
{
	auto R = MakeShared<FJsonObject>();
	R->SetStringField(TEXT("error"), Message);
	return R;
}

// Helper: structured error response {"error": {"code", "message", "hint"}} - the shape every
// transport-level error and every new tool uses. The Python bridge flattens it for the
// legacy handlers, which only look at resp["error"].
static TSharedPtr<FJsonObject> MCPErrorEx(const FString& Code, const FString& Message, const FString& Hint = FString())
{
	auto Err = MakeShared<FJsonObject>();
	Err->SetStringField(TEXT("code"), Code);
	Err->SetStringField(TEXT("message"), Message);
	if (!Hint.IsEmpty())
	{
		Err->SetStringField(TEXT("hint"), Hint);
	}
	auto R = MakeShared<FJsonObject>();
	R->SetBoolField(TEXT("success"), false);
	R->SetObjectField(TEXT("error"), Err);
	return R;
}

namespace
{
	/** One dispatched command. Owns the completion event so that whichever side finishes last
	 *  (the socket thread giving up, or the game-thread task completing late) can still use it. */
	struct FMCPPendingCommand
	{
		enum EState : int32 { Pending = 0, Running = 1, Cancelled = 2, Finished = 3, StaleRunning = 4 };

		FEvent* Event = FPlatformProcess::GetSynchEventFromPool(true);
		TAtomic<int32> State{ Pending };
		TSharedPtr<FJsonObject> Result;

		~FMCPPendingCommand() { FPlatformProcess::ReturnSynchEventToPool(Event); }
	};
}

// Helper: get editor world
static UWorld* GetEditorWorld()
{
	if (GEditor)
	{
		return GEditor->GetEditorWorldContext().World();
	}
	return nullptr;
}

// Helper: find actor by name in editor world
static AActor* FindActorByName(const FString& Name)
{
	UWorld* World = GetEditorWorld();
	if (!World) return nullptr;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->GetActorLabel() == Name || It->GetName() == Name)
		{
			return *It;
		}
	}
	return nullptr;
}

// Helper: find UClass by name (replaces deprecated ANY_PACKAGE usage)
static UClass* FindClassByName(const FString& ClassName)
{
	// Try FindFirstObjectSafe which replaces FindObject with ANY_PACKAGE in UE5.1+
	UClass* Found = FindFirstObject<UClass>(*ClassName, EFindFirstObjectOptions::ExactClass, ELogVerbosity::NoLogging);
	if (Found) return Found;

	// Brute force search through all UClasses
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->GetName() == ClassName)
		{
			return *It;
		}
	}
	return nullptr;
}

// Helper: serialize JSON object to string
static FString JsonToString(const TSharedPtr<FJsonObject>& Obj)
{
	FString Output;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Output);
	FJsonSerializer::Serialize(Obj.ToSharedRef(), Writer);
	return Output;
}

FMCPTcpServer::FMCPTcpServer()
{
}

FMCPTcpServer::~FMCPTcpServer()
{
	Stop();
}

bool FMCPTcpServer::Start()
{
	// Register all command handlers
	StaleState = MakeShared<FStaleState>();
	RegisterTransportHandlers();
	RegisterActorHandlers();
	RegisterBlueprintHandlers();
	RegisterLevelHandlers();
	RegisterLandscapeHandlers();
	RegisterMaterialHandlers();
	RegisterPlaytestHandlers();
	RegisterEditorUtilityHandlers();
	RegisterComponentHandlers();
	RegisterWidgetHandlers();
	RegisterAssetHandlers();
	RegisterDataTableHandlers();
#if WITH_NIAGARA
	RegisterNiagaraHandlers();
#endif
	RegisterSequencerHandlers();
	RegisterWidgetAnimationHandlers();
	RegisterBlueprintMacroHandlers();

	// Create TCP listener on port 0 (OS assigns port)
	FIPv4Endpoint Endpoint(FIPv4Address(127, 0, 0, 1), 0);
	ListenerSocket = FTcpSocketBuilder(TEXT("UnrealMCPListener"))
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.Listening(8);

	if (!ListenerSocket)
	{
		UE_LOG(LogUnrealMCP, Error, TEXT("[UnrealMCP] Failed to create listener socket"));
		return false;
	}

	// Get the assigned port
	TSharedRef<FInternetAddr> LocalAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	ListenerSocket->GetAddress(*LocalAddr);
	int32 AssignedPort = LocalAddr->GetPort();

	UE_LOG(LogUnrealMCP, Log, TEXT("[UnrealMCP] Listening on 127.0.0.1:%d"), AssignedPort);

	// Write port file
	ListenPort = AssignedPort;
	WritePortFile(AssignedPort);

	// Start listener thread
	bRunning = true;
	Thread = FRunnableThread::Create(this, TEXT("UnrealMCPServerThread"), 0, TPri_Normal);

	return true;
}

void FMCPTcpServer::Stop()
{
	bRunning = false;

	if (ListenerSocket)
	{
		ListenerSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenerSocket);
		ListenerSocket = nullptr;
	}

	if (Thread)
	{
		FRunnableThread* ThreadToDelete = Thread;
		Thread = nullptr;
		ThreadToDelete->WaitForCompletion();
		delete ThreadToDelete;
	}

	// Clean up port file
	FString PortFilePath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("UnrealMCP"), TEXT("port.txt"));
	IFileManager::Get().Delete(*PortFilePath);
}

void FMCPTcpServer::RegisterHandler(const FString& CommandName, FCommandHandler Handler)
{
	FScopeLock Lock(&HandlersMutex);
	CommandHandlers.Add(CommandName, MoveTemp(Handler));
}

bool FMCPTcpServer::UnregisterHandler(const FString& CommandName)
{
	FScopeLock Lock(&HandlersMutex);
	return CommandHandlers.Remove(CommandName) > 0;
}

bool FMCPTcpServer::IsStaleCommandRunning() const
{
	return StaleState.IsValid() && StaleState->Running.GetValue() > 0;
}

uint32 FMCPTcpServer::Run()
{
	while (bRunning)
	{
		bool bHasPendingConnection = false;
		if (ListenerSocket->HasPendingConnection(bHasPendingConnection) && bHasPendingConnection)
		{
			TSharedRef<FInternetAddr> RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
			FSocket* ClientSocket = ListenerSocket->Accept(*RemoteAddr, TEXT("UnrealMCPClient"));
			if (ClientSocket)
			{
				UE_LOG(LogUnrealMCP, Log, TEXT("[UnrealMCP] Client connected"));
				HandleClient(ClientSocket);
			}
		}
		else
		{
			FPlatformProcess::Sleep(0.01f);
		}
	}
	return 0;
}

void FMCPTcpServer::Exit()
{
}

void FMCPTcpServer::HandleClient(FSocket* ClientSocket)
{
	// Accumulate BYTES and decode a line only once its '\n' has arrived: a UTF-8 sequence can
	// straddle two recv() calls, the old byte-count-as-char-count conversion truncated any
	// non-ASCII message, and a single JSON line may be several MB (base64 images).
	TArray<uint8> Pending;
	TArray<uint8> RecvBuffer;
	RecvBuffer.SetNumUninitialized(65536);

	while (bRunning)
	{
		uint32 PendingDataSize = 0;
		if (ClientSocket->HasPendingData(PendingDataSize))
		{
			int32 BytesRead = 0;
			if (ClientSocket->Recv(RecvBuffer.GetData(), RecvBuffer.Num(), BytesRead))
			{
				if (BytesRead > 0)
				{
					const int32 ScanFrom = Pending.Num();
					Pending.Append(RecvBuffer.GetData(), BytesRead);
					int32 LineStart = 0;
					for (int32 i = ScanFrom; i < Pending.Num(); ++i)
					{
						if (Pending[i] != '\n')
						{
							continue;
						}
						const int32 Len = i - LineStart;
						if (Len > 0)
						{
							FUTF8ToTCHAR Converter(reinterpret_cast<const ANSICHAR*>(Pending.GetData() + LineStart), Len);
							FString Line(Converter.Length(), Converter.Get());
							Line.TrimStartAndEndInline();
							if (!Line.IsEmpty())
							{
								ProcessMessage(Line, ClientSocket);
							}
						}
						LineStart = i + 1;
					}
					if (LineStart > 0)
					{
						Pending.RemoveAt(0, LineStart, EAllowShrinking::No);
					}
				}
			}
			else
			{
				break;
			}
		}
		else
		{
			ESocketConnectionState State = ClientSocket->GetConnectionState();
			if (State != SCS_Connected)
			{
				break;
			}

			// GetConnectionState cannot see a remote FIN (graceful close) — the
			// socket stays "connected" forever and this loop wedges, blocking all
			// future clients. A readable socket with zero pending data IS a FIN.
			if (ClientSocket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromMilliseconds(1)))
			{
				uint32 ProbeDataSize = 0;
				if (!ClientSocket->HasPendingData(ProbeDataSize) || ProbeDataSize == 0)
				{
					break;
				}
			}
			else
			{
				FPlatformProcess::Sleep(0.001f);
			}
		}
	}

	UE_LOG(LogUnrealMCP, Log, TEXT("[UnrealMCP] Client disconnected"));
	ClientSocket->Close();
	ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
}

void FMCPTcpServer::ProcessMessage(const FString& Message, FSocket* ClientSocket)
{
	// Messages can be several MB (base64 images); never put more than a preview in the log.
	UE_LOG(LogUnrealMCP, Log, TEXT("[UnrealMCP] Received (%d chars): %s"), Message.Len(), *Message.Left(300));

	TSharedPtr<FJsonObject> JsonMsg;
	auto Reader = TJsonReaderFactory<>::Create(Message);
	if (!FJsonSerializer::Deserialize(Reader, JsonMsg) || !JsonMsg.IsValid())
	{
		SendResponse(ClientSocket, MCPErrorEx(TEXT("invalid_json"), TEXT("Message is not a JSON object"),
			TEXT("Send one {\"command\", \"params\"} object per line")));
		return;
	}

	// A command that timed out is still executing on the game thread: refuse to stack another
	// on top of it (the two would interleave and corrupt any stateful tool).
	if (IsStaleCommandRunning())
	{
		FString StaleName;
		{
			FScopeLock Lock(&StaleState->Mutex);
			StaleName = StaleState->CommandName;
		}
		SendResponse(ClientSocket, MCPErrorEx(TEXT("busy"),
			FString::Printf(TEXT("Previous command '%s' timed out and is still running on the game thread"), *StaleName),
			TEXT("Wait and retry; if it never clears the editor is probably blocked by a modal dialog")));
		return;
	}

	FString Command = JsonMsg->GetStringField(TEXT("command"));
	TSharedPtr<FJsonObject> Params = JsonMsg->GetObjectField(TEXT("params"));
	if (!Params.IsValid())
	{
		Params = MakeShared<FJsonObject>();
	}

	FCommandHandler Handler;
	{
		FScopeLock Lock(&HandlersMutex);
		auto* Found = CommandHandlers.Find(Command);
		if (!Found)
		{
			SendResponse(ClientSocket, MCPErrorEx(TEXT("unknown_command"), FString::Printf(TEXT("Unknown command: %s"), *Command)));
			return;
		}
		Handler = *Found;
	}

	// Execute on the game thread and wait. The pending-command object is shared with the task,
	// so nothing here is touched after this function returns; the state machine decides who
	// owns the outcome when the wait expires:
	//   Pending -> Cancelled  : the task had not started; it will return without running.
	//   Running -> StaleRunning: it is executing; we answer "timeout", mark it stale, and the
	//                            task itself clears the stale flag when it finishes.
	TSharedPtr<FMCPPendingCommand> Cmd = MakeShared<FMCPPendingCommand>();
	TSharedPtr<FStaleState> Stale = StaleState;

	AsyncTask(ENamedThreads::GameThread, [Handler, Params, Cmd, Stale]()
	{
		int32 Expected = FMCPPendingCommand::Pending;
		if (!Cmd->State.CompareExchange(Expected, FMCPPendingCommand::Running))
		{
			return; // cancelled before it ran
		}
		Cmd->Result = Handler(Params);
		Expected = FMCPPendingCommand::Running;
		if (!Cmd->State.CompareExchange(Expected, FMCPPendingCommand::Finished))
		{
			// the socket thread gave up on us: we were the stale command, and it is over now
			Stale->Running.Decrement();
		}
		Cmd->Event->Trigger();
	});

	const bool bCompleted = Cmd->Event->Wait(CommandTimeoutMs);
	TSharedPtr<FJsonObject> Result;
	if (bCompleted && Cmd->Result.IsValid())
	{
		Result = Cmd->Result;
	}
	else if (bCompleted)
	{
		Result = MCPErrorEx(TEXT("handler_failed"), FString::Printf(TEXT("Command '%s' returned no result"), *Command));
	}
	else
	{
		int32 Expected = FMCPPendingCommand::Pending;
		if (Cmd->State.CompareExchange(Expected, FMCPPendingCommand::Cancelled))
		{
			Result = MCPErrorEx(TEXT("timeout"),
				FString::Printf(TEXT("Command '%s' was not started within %d ms (game thread busy); it was cancelled"), *Command, CommandTimeoutMs),
				TEXT("The editor is stalled or blocked by a modal dialog; retry once it responds"));
		}
		else
		{
			Stale->Running.Increment();
			{
				FScopeLock Lock(&Stale->Mutex);
				Stale->CommandName = Command;
			}
			Expected = FMCPPendingCommand::Running;
			if (!Cmd->State.CompareExchange(Expected, FMCPPendingCommand::StaleRunning))
			{
				Stale->Running.Decrement(); // it finished in the meantime
			}
			Result = MCPErrorEx(TEXT("timeout"),
				FString::Printf(TEXT("Command '%s' is still running after %d ms"), *Command, CommandTimeoutMs),
				TEXT("Further commands are refused with 'busy' until it finishes"));
		}
	}

	SendResponse(ClientSocket, Result);
}

// ====================================================================================
// TRANSPORT COMMAND HANDLERS
// ====================================================================================

void FMCPTcpServer::RegisterTransportHandlers()
{
	RegisterHandler(TEXT("ping"), [this](const TSharedPtr<FJsonObject>& Params) { return HandlePing(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandlePing(const TSharedPtr<FJsonObject>& Params)
{
	// Connectivity check and transport regression probe: echoes the length of an optional
	// payload plus its last character, so a client can prove multi-MB and multi-byte UTF-8
	// lines survive the trip.
	auto R = MCPSuccess();
	R->SetStringField(TEXT("reply"), TEXT("pong"));
	FString Payload;
	if (Params->TryGetStringField(TEXT("payload"), Payload))
	{
		R->SetNumberField(TEXT("payload_length"), Payload.Len());
		R->SetStringField(TEXT("payload_tail"), Payload.IsEmpty() ? FString() : Payload.Right(1));
	}
	return R;
}

void FMCPTcpServer::SendResponse(FSocket* ClientSocket, const TSharedPtr<FJsonObject>& Response)
{
	FString JsonStr = JsonToString(Response) + TEXT("\n");
	FTCHARToUTF8 Converter(*JsonStr);
	int32 BytesSent = 0;
	ClientSocket->Send((const uint8*)Converter.Get(), Converter.Length(), BytesSent);

	UE_LOG(LogUnrealMCP, Log, TEXT("[UnrealMCP] Sent (%d chars): %s"), JsonStr.Len(), *JsonStr.Left(300).TrimEnd());
}

void FMCPTcpServer::WritePortFile(int32 Port)
{
	FString Dir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("UnrealMCP"));
	IFileManager::Get().MakeDirectory(*Dir, true);

	FString PortFilePath = FPaths::Combine(Dir, TEXT("port.txt"));
	FString PortStr = FString::Printf(TEXT("%d"), Port);
	FFileHelper::SaveStringToFile(PortStr, *PortFilePath);

	UE_LOG(LogUnrealMCP, Log, TEXT("[UnrealMCP] Port file written to: %s"), *PortFilePath);
}

// ====================================================================================
// ACTOR COMMAND HANDLERS
// ====================================================================================

void FMCPTcpServer::RegisterActorHandlers()
{
	RegisterHandler(TEXT("list_actors"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleListActors(Params); });
	RegisterHandler(TEXT("spawn_actor"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSpawnActor(Params); });
	RegisterHandler(TEXT("delete_actor"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleDeleteActor(Params); });
	RegisterHandler(TEXT("set_actor_transform"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSetActorTransform(Params); });
	RegisterHandler(TEXT("get_property"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleGetProperty(Params); });
	RegisterHandler(TEXT("set_property"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSetProperty(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleListActors(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World) return MCPError(TEXT("No editor world available"));

	FString ClassFilter;
	Params->TryGetStringField(TEXT("class_filter"), ClassFilter);

	TArray<TSharedPtr<FJsonValue>> ActorArray;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		if (!ClassFilter.IsEmpty())
		{
			FString ClassName = Actor->GetClass()->GetName();
			if (!ClassName.Contains(ClassFilter))
			{
				continue;
			}
		}

		auto ActorObj = MakeShared<FJsonObject>();
		ActorObj->SetStringField(TEXT("name"), Actor->GetActorLabel());
		ActorObj->SetStringField(TEXT("internal_name"), Actor->GetName());
		ActorObj->SetStringField(TEXT("class"), Actor->GetClass()->GetName());

		FVector Loc = Actor->GetActorLocation();
		ActorObj->SetNumberField(TEXT("x"), Loc.X);
		ActorObj->SetNumberField(TEXT("y"), Loc.Y);
		ActorObj->SetNumberField(TEXT("z"), Loc.Z);

		ActorArray.Add(MakeShared<FJsonValueObject>(ActorObj));
	}

	auto Result = MCPSuccess();
	Result->SetArrayField(TEXT("actors"), ActorArray);
	Result->SetNumberField(TEXT("count"), ActorArray.Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSpawnActor(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World) return MCPError(TEXT("No editor world available"));

	FString ClassName;
	if (!Params->TryGetStringField(TEXT("class_name"), ClassName))
	{
		return MCPError(TEXT("Missing required param: class_name"));
	}

	double X = Params->GetNumberField(TEXT("x"));
	double Y = Params->GetNumberField(TEXT("y"));
	double Z = Params->GetNumberField(TEXT("z"));

	UClass* ActorClass = nullptr;

	// Try finding as a native class
	ActorClass = FindClassByName(ClassName);

	// Try with 'A' prefix for native actor classes
	if (!ActorClass)
	{
		ActorClass = FindClassByName(TEXT("A") + ClassName);
	}

	// Try loading as a Blueprint class
	if (!ActorClass)
	{
		FString BlueprintPath = ClassName;
		if (!BlueprintPath.StartsWith(TEXT("/")))
		{
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			TArray<FAssetData> AssetData;
			AssetRegistryModule.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), AssetData);

			for (const FAssetData& Asset : AssetData)
			{
				if (Asset.AssetName.ToString() == ClassName)
				{
					UBlueprint* BP = Cast<UBlueprint>(Asset.GetAsset());
					if (BP && BP->GeneratedClass)
					{
						ActorClass = BP->GeneratedClass;
						break;
					}
				}
			}
		}
		else
		{
			UBlueprint* BP = LoadObject<UBlueprint>(nullptr, *BlueprintPath);
			if (BP && BP->GeneratedClass)
			{
				ActorClass = BP->GeneratedClass;
			}
		}
	}

	if (!ActorClass)
	{
		return MCPError(FString::Printf(TEXT("Could not find class: %s"), *ClassName));
	}

	if (!ActorClass->IsChildOf(AActor::StaticClass()))
	{
		return MCPError(FString::Printf(TEXT("Class %s is not an Actor class"), *ClassName));
	}

	FVector Location(X, Y, Z);
	FRotator Rotation = FRotator::ZeroRotator;

	double Pitch = 0, Yaw = 0, Roll = 0;
	if (Params->TryGetNumberField(TEXT("pitch"), Pitch) ||
		Params->TryGetNumberField(TEXT("yaw"), Yaw) ||
		Params->TryGetNumberField(TEXT("roll"), Roll))
	{
		Rotation = FRotator(Pitch, Yaw, Roll);
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* NewActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
	if (!NewActor)
	{
		return MCPError(TEXT("Failed to spawn actor"));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("name"), NewActor->GetActorLabel());
	Result->SetStringField(TEXT("internal_name"), NewActor->GetName());
	Result->SetStringField(TEXT("class"), NewActor->GetClass()->GetName());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleDeleteActor(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return MCPError(TEXT("Missing required param: name"));
	}

	AActor* Actor = FindActorByName(Name);
	if (!Actor)
	{
		return MCPError(FString::Printf(TEXT("Actor not found: %s"), *Name));
	}

	UWorld* World = GetEditorWorld();
	if (!World) return MCPError(TEXT("No editor world available"));

	bool bDestroyed = World->EditorDestroyActor(Actor, true);
	if (!bDestroyed)
	{
		return MCPError(FString::Printf(TEXT("Failed to destroy actor: %s"), *Name));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("deleted"), Name);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSetActorTransform(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return MCPError(TEXT("Missing required param: name"));
	}

	AActor* Actor = FindActorByName(Name);
	if (!Actor)
	{
		return MCPError(FString::Printf(TEXT("Actor not found: %s"), *Name));
	}

	const TSharedPtr<FJsonObject>* PosObj;
	if (Params->TryGetObjectField(TEXT("position"), PosObj))
	{
		double PX = (*PosObj)->GetNumberField(TEXT("x"));
		double PY = (*PosObj)->GetNumberField(TEXT("y"));
		double PZ = (*PosObj)->GetNumberField(TEXT("z"));
		Actor->SetActorLocation(FVector(PX, PY, PZ));
	}

	const TSharedPtr<FJsonObject>* RotObj;
	if (Params->TryGetObjectField(TEXT("rotation"), RotObj))
	{
		double RPitch = (*RotObj)->GetNumberField(TEXT("pitch"));
		double RYaw = (*RotObj)->GetNumberField(TEXT("yaw"));
		double RRoll = (*RotObj)->GetNumberField(TEXT("roll"));
		Actor->SetActorRotation(FRotator(RPitch, RYaw, RRoll));
	}

	const TSharedPtr<FJsonObject>* ScaleObj;
	if (Params->TryGetObjectField(TEXT("scale"), ScaleObj))
	{
		double SX = (*ScaleObj)->GetNumberField(TEXT("x"));
		double SY = (*ScaleObj)->GetNumberField(TEXT("y"));
		double SZ = (*ScaleObj)->GetNumberField(TEXT("z"));
		Actor->SetActorScale3D(FVector(SX, SY, SZ));
	}

	FVector Loc = Actor->GetActorLocation();
	FRotator Rot = Actor->GetActorRotation();
	FVector Scale = Actor->GetActorScale3D();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("name"), Name);

	auto PosResult = MakeShared<FJsonObject>();
	PosResult->SetNumberField(TEXT("x"), Loc.X);
	PosResult->SetNumberField(TEXT("y"), Loc.Y);
	PosResult->SetNumberField(TEXT("z"), Loc.Z);
	Result->SetObjectField(TEXT("position"), PosResult);

	auto RotResult = MakeShared<FJsonObject>();
	RotResult->SetNumberField(TEXT("pitch"), Rot.Pitch);
	RotResult->SetNumberField(TEXT("yaw"), Rot.Yaw);
	RotResult->SetNumberField(TEXT("roll"), Rot.Roll);
	Result->SetObjectField(TEXT("rotation"), RotResult);

	auto ScaleResult = MakeShared<FJsonObject>();
	ScaleResult->SetNumberField(TEXT("x"), Scale.X);
	ScaleResult->SetNumberField(TEXT("y"), Scale.Y);
	ScaleResult->SetNumberField(TEXT("z"), Scale.Z);
	Result->SetObjectField(TEXT("scale"), ScaleResult);

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleGetProperty(const TSharedPtr<FJsonObject>& Params)
{
	FString ActorName;
	if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
	{
		return MCPError(TEXT("Missing required param: actor_name"));
	}

	FString PropertyName;
	if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
	{
		return MCPError(TEXT("Missing required param: property_name"));
	}

	AActor* Actor = FindActorByName(ActorName);
	if (!Actor)
	{
		return MCPError(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
	}

	UObject* TargetObject = Actor;
	FString ComponentName;
	FString ActualPropertyName = PropertyName;

	// Support "ComponentName.PropertyName" syntax
	if (PropertyName.Contains(TEXT(".")))
	{
		PropertyName.Split(TEXT("."), &ComponentName, &ActualPropertyName);

		TArray<UActorComponent*> Components;
		Actor->GetComponents(Components);
		for (UActorComponent* C : Components)
		{
			if (C->GetName() == ComponentName || C->GetName().Contains(ComponentName))
			{
				TargetObject = C;
				break;
			}
		}
	}

	FProperty* Prop = TargetObject->GetClass()->FindPropertyByName(FName(*ActualPropertyName));
	if (!Prop)
	{
		return MCPError(FString::Printf(TEXT("Property not found: %s on %s"), *ActualPropertyName, *TargetObject->GetName()));
	}

	FString ValueStr;
	const void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(TargetObject);
	Prop->ExportTextItem_Direct(ValueStr, ValuePtr, nullptr, TargetObject, PPF_None);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("actor"), ActorName);
	Result->SetStringField(TEXT("property"), PropertyName);
	Result->SetStringField(TEXT("value"), ValueStr);
	Result->SetStringField(TEXT("type"), Prop->GetCPPType());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSetProperty(const TSharedPtr<FJsonObject>& Params)
{
	FString ActorName;
	if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
	{
		return MCPError(TEXT("Missing required param: actor_name"));
	}

	FString PropertyName;
	if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
	{
		return MCPError(TEXT("Missing required param: property_name"));
	}

	FString Value;
	if (!Params->TryGetStringField(TEXT("value"), Value))
	{
		return MCPError(TEXT("Missing required param: value"));
	}

	AActor* Actor = FindActorByName(ActorName);
	if (!Actor)
	{
		return MCPError(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
	}

	UObject* TargetObject = Actor;
	FString ComponentName;
	FString ActualPropertyName = PropertyName;

	if (PropertyName.Contains(TEXT(".")))
	{
		PropertyName.Split(TEXT("."), &ComponentName, &ActualPropertyName);
		TArray<UActorComponent*> Components;
		Actor->GetComponents(Components);
		for (UActorComponent* C : Components)
		{
			if (C->GetName() == ComponentName || C->GetName().Contains(ComponentName))
			{
				TargetObject = C;
				break;
			}
		}
	}

	FProperty* Prop = TargetObject->GetClass()->FindPropertyByName(FName(*ActualPropertyName));
	if (!Prop)
	{
		return MCPError(FString::Printf(TEXT("Property not found: %s on %s"), *ActualPropertyName, *TargetObject->GetName()));
	}

	TargetObject->PreEditChange(Prop);

	void* ValPtr = Prop->ContainerPtrToValuePtr<void>(TargetObject);
	const TCHAR* ValueStream = *Value;
	Prop->ImportText_Direct(ValueStream, ValPtr, TargetObject, PPF_None);

	FPropertyChangedEvent ChangeEvent(Prop);
	TargetObject->PostEditChangeProperty(ChangeEvent);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("actor"), ActorName);
	Result->SetStringField(TEXT("property"), PropertyName);
	Result->SetStringField(TEXT("new_value"), Value);
	return Result;
}

// ====================================================================================
// BLUEPRINT COMMAND HANDLERS
// ====================================================================================

void FMCPTcpServer::RegisterBlueprintHandlers()
{
	RegisterHandler(TEXT("list_blueprints"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleListBlueprints(Params); });
	RegisterHandler(TEXT("read_bp_graph"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleReadBPGraph(Params); });
	RegisterHandler(TEXT("add_bp_node"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddBPNode(Params); });
	RegisterHandler(TEXT("connect_pins"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleConnectPins(Params); });
	RegisterHandler(TEXT("disconnect_pins"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleDisconnectPins(Params); });
	RegisterHandler(TEXT("compile_bp"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleCompileBP(Params); });
	RegisterHandler(TEXT("add_bp_variable"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddBPVariable(Params); });
	RegisterHandler(TEXT("remove_bp_node"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleRemoveBPNode(Params); });
	RegisterHandler(TEXT("set_pin_default"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSetPinDefault(Params); });
	RegisterHandler(TEXT("add_bp_function"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddBPFunction(Params); });
	RegisterHandler(TEXT("remove_bp_variable"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleRemoveBPVariable(Params); });
	RegisterHandler(TEXT("add_event_dispatcher"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddEventDispatcher(Params); });
	RegisterHandler(TEXT("add_bp_component"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddBPComponent(Params); });
}

// Helper: find Blueprint by path or name
static UBlueprint* FindBlueprintByPath(const FString& Path)
{
	FString AssetPath = Path;

	if (!AssetPath.StartsWith(TEXT("/")))
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> AssetData;
		AssetRegistryModule.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), AssetData);

		for (const FAssetData& Asset : AssetData)
		{
			if (Asset.AssetName.ToString() == Path || Asset.GetObjectPathString().Contains(Path))
			{
				return Cast<UBlueprint>(Asset.GetAsset());
			}
		}
		return nullptr;
	}

	return LoadObject<UBlueprint>(nullptr, *AssetPath);
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleListBlueprints(const TSharedPtr<FJsonObject>& Params)
{
	FString PathFilter;
	Params->TryGetStringField(TEXT("path_filter"), PathFilter);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), AssetData);

	TArray<TSharedPtr<FJsonValue>> BPArray;

	for (const FAssetData& Asset : AssetData)
	{
		FString ObjectPath = Asset.GetObjectPathString();

		if (!PathFilter.IsEmpty() && !ObjectPath.Contains(PathFilter))
		{
			continue;
		}

		auto BPObj = MakeShared<FJsonObject>();
		BPObj->SetStringField(TEXT("name"), Asset.AssetName.ToString());
		BPObj->SetStringField(TEXT("path"), ObjectPath);

		FString ParentClass;
		Asset.GetTagValue(FBlueprintTags::ParentClassPath, ParentClass);
		if (!ParentClass.IsEmpty())
		{
			BPObj->SetStringField(TEXT("parent_class"), ParentClass);
		}

		BPArray.Add(MakeShared<FJsonValueObject>(BPObj));
	}

	auto Result = MCPSuccess();
	Result->SetArrayField(TEXT("blueprints"), BPArray);
	Result->SetNumberField(TEXT("count"), BPArray.Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleReadBPGraph(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	FString GraphName;
	Params->TryGetStringField(TEXT("graph_name"), GraphName);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("blueprint"), BP->GetPathName());

	// Variables
	TArray<TSharedPtr<FJsonValue>> VarArray;
	for (const FBPVariableDescription& Var : BP->NewVariables)
	{
		auto VarObj = MakeShared<FJsonObject>();
		VarObj->SetStringField(TEXT("name"), Var.VarName.ToString());
		VarObj->SetStringField(TEXT("type"), Var.VarType.PinCategory.ToString());
		if (Var.VarType.PinSubCategoryObject.IsValid())
		{
			VarObj->SetStringField(TEXT("sub_type"), Var.VarType.PinSubCategoryObject->GetName());
		}
		VarObj->SetBoolField(TEXT("is_instance_editable"), (Var.PropertyFlags & CPF_Edit) != 0);
		VarArray.Add(MakeShared<FJsonValueObject>(VarObj));
	}
	Result->SetArrayField(TEXT("variables"), VarArray);

	// Helper lambda to serialize a graph's nodes
	auto SerializeGraph = [](UEdGraph* Graph) -> TSharedPtr<FJsonObject>
	{
		auto GraphObj = MakeShared<FJsonObject>();
		GraphObj->SetStringField(TEXT("name"), Graph->GetName());

		TArray<TSharedPtr<FJsonValue>> NodeArray;
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			auto NodeObj = MakeShared<FJsonObject>();
			NodeObj->SetStringField(TEXT("id"), Node->NodeGuid.ToString());
			NodeObj->SetStringField(TEXT("class"), Node->GetClass()->GetName());
			NodeObj->SetStringField(TEXT("title"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
			NodeObj->SetNumberField(TEXT("x"), Node->NodePosX);
			NodeObj->SetNumberField(TEXT("y"), Node->NodePosY);

			TArray<TSharedPtr<FJsonValue>> PinArray;
			for (UEdGraphPin* Pin : Node->Pins)
			{
				auto PinObj = MakeShared<FJsonObject>();
				PinObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
				PinObj->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
				PinObj->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());

				if (!Pin->DefaultValue.IsEmpty())
				{
					PinObj->SetStringField(TEXT("default_value"), Pin->DefaultValue);
				}

				TArray<TSharedPtr<FJsonValue>> ConnArray;
				for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
				{
					auto ConnObj = MakeShared<FJsonObject>();
					ConnObj->SetStringField(TEXT("node_id"), LinkedPin->GetOwningNode()->NodeGuid.ToString());
					ConnObj->SetStringField(TEXT("pin_name"), LinkedPin->PinName.ToString());
					ConnArray.Add(MakeShared<FJsonValueObject>(ConnObj));
				}
				if (ConnArray.Num() > 0)
				{
					PinObj->SetArrayField(TEXT("connections"), ConnArray);
				}

				PinArray.Add(MakeShared<FJsonValueObject>(PinObj));
			}
			NodeObj->SetArrayField(TEXT("pins"), PinArray);
			NodeArray.Add(MakeShared<FJsonValueObject>(NodeObj));
		}
		GraphObj->SetArrayField(TEXT("nodes"), NodeArray);
		return GraphObj;
	};

	TArray<TSharedPtr<FJsonValue>> GraphArray;

	for (UEdGraph* Graph : BP->UbergraphPages)
	{
		if (!GraphName.IsEmpty() && Graph->GetName() != GraphName) continue;
		GraphArray.Add(MakeShared<FJsonValueObject>(SerializeGraph(Graph)));
	}

	for (UEdGraph* Graph : BP->FunctionGraphs)
	{
		if (!GraphName.IsEmpty() && Graph->GetName() != GraphName) continue;
		auto GraphObj = SerializeGraph(Graph);
		GraphObj->SetStringField(TEXT("type"), TEXT("function"));
		GraphArray.Add(MakeShared<FJsonValueObject>(GraphObj));
	}

	Result->SetArrayField(TEXT("graphs"), GraphArray);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddBPNode(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	FString NodeType;
	if (!Params->TryGetStringField(TEXT("node_type"), NodeType))
	{
		return MCPError(TEXT("Missing required param: node_type"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	FString GraphName;
	Params->TryGetStringField(TEXT("graph_name"), GraphName);

	UEdGraph* TargetGraph = nullptr;
	if (GraphName.IsEmpty())
	{
		if (BP->UbergraphPages.Num() > 0)
		{
			TargetGraph = BP->UbergraphPages[0];
		}
	}
	else
	{
		for (UEdGraph* Graph : BP->UbergraphPages)
		{
			if (Graph->GetName() == GraphName) { TargetGraph = Graph; break; }
		}
		if (!TargetGraph)
		{
			for (UEdGraph* Graph : BP->FunctionGraphs)
			{
				if (Graph->GetName() == GraphName) { TargetGraph = Graph; break; }
			}
		}
	}

	if (!TargetGraph)
	{
		return MCPError(TEXT("Could not find target graph"));
	}

	int32 PosX = (int32)Params->GetNumberField(TEXT("x"));
	int32 PosY = (int32)Params->GetNumberField(TEXT("y"));

	UEdGraphNode* NewNode = nullptr;

	if (NodeType == TEXT("CallFunction"))
	{
		FString FunctionName;
		if (!Params->TryGetStringField(TEXT("function_name"), FunctionName))
		{
			return MCPError(TEXT("CallFunction requires function_name param"));
		}

		FString TargetClassName;
		Params->TryGetStringField(TEXT("target_class"), TargetClassName);

		UFunction* Function = nullptr;
		if (!TargetClassName.IsEmpty())
		{
			UClass* TargetClass = FindClassByName(TargetClassName);
			if (!TargetClass) TargetClass = FindClassByName(TEXT("U") + TargetClassName);
			if (!TargetClass) TargetClass = FindClassByName(TEXT("A") + TargetClassName);
			if (TargetClass)
			{
				Function = TargetClass->FindFunctionByName(FName(*FunctionName));
			}
		}

		if (!Function)
		{
			TArray<UClass*> SearchClasses = {
				AActor::StaticClass(),
				UActorComponent::StaticClass(),
				UKismetSystemLibrary::StaticClass(),
				UKismetMathLibrary::StaticClass(),
				UGameplayStatics::StaticClass()
			};

			for (UClass* SearchClass : SearchClasses)
			{
				Function = SearchClass->FindFunctionByName(FName(*FunctionName));
				if (Function) break;
			}
		}

		if (!Function)
		{
			return MCPError(FString::Printf(TEXT("Function not found: %s"), *FunctionName));
		}

		UK2Node_CallFunction* CallNode = NewObject<UK2Node_CallFunction>(TargetGraph);
		CallNode->SetFromFunction(Function);
		CallNode->NodePosX = PosX;
		CallNode->NodePosY = PosY;
		CallNode->AllocateDefaultPins();
		TargetGraph->AddNode(CallNode, false, false);
		NewNode = CallNode;
	}
	else if (NodeType == TEXT("Event"))
	{
		FString EventName;
		if (!Params->TryGetStringField(TEXT("event_name"), EventName))
		{
			return MCPError(TEXT("Event requires event_name param"));
		}

		UFunction* EventFunc = AActor::StaticClass()->FindFunctionByName(FName(*EventName));
		if (EventFunc)
		{
			UK2Node_Event* EventNode = NewObject<UK2Node_Event>(TargetGraph);
			EventNode->EventReference.SetExternalMember(FName(*EventName), AActor::StaticClass());
			EventNode->NodePosX = PosX;
			EventNode->NodePosY = PosY;
			EventNode->AllocateDefaultPins();
			TargetGraph->AddNode(EventNode, false, false);
			NewNode = EventNode;
		}
		else
		{
			UK2Node_CustomEvent* CustomEventNode = NewObject<UK2Node_CustomEvent>(TargetGraph);
			CustomEventNode->CustomFunctionName = FName(*EventName);
			CustomEventNode->NodePosX = PosX;
			CustomEventNode->NodePosY = PosY;
			CustomEventNode->CreateNewGuid();
			CustomEventNode->AllocateDefaultPins();
			TargetGraph->AddNode(CustomEventNode, false, false);
			NewNode = CustomEventNode;
		}
	}
	else if (NodeType == TEXT("ComponentBoundEvent"))
	{
		FString ComponentName;
		if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
		{
			return MCPError(TEXT("ComponentBoundEvent requires component_name param (e.g. 'Btn_Options')"));
		}

		FString DelegateName;
		if (!Params->TryGetStringField(TEXT("delegate_name"), DelegateName))
		{
			return MCPError(TEXT("ComponentBoundEvent requires delegate_name param (e.g. 'OnClicked', 'OnButtonClicked')"));
		}

		UClass* BPClass = BP->SkeletonGeneratedClass ? BP->SkeletonGeneratedClass : BP->GeneratedClass;
		if (!BPClass)
		{
			return MCPError(TEXT("Blueprint has no generated class — compile it first"));
		}

		FObjectPropertyBase* ComponentProp = nullptr;
		for (TFieldIterator<FObjectPropertyBase> It(BPClass); It; ++It)
		{
			if (It->GetName() == ComponentName)
			{
				ComponentProp = *It;
				break;
			}
		}

		if (!ComponentProp)
		{
			return MCPError(FString::Printf(TEXT("Component property '%s' not found on blueprint class. Make sure it is marked as a variable."), *ComponentName));
		}

		UClass* ComponentClass = ComponentProp->PropertyClass;
		if (!ComponentClass)
		{
			return MCPError(FString::Printf(TEXT("Could not determine class for component '%s'"), *ComponentName));
		}

		FMulticastDelegateProperty* DelegateProp = nullptr;
		for (TFieldIterator<FMulticastDelegateProperty> It(ComponentClass); It; ++It)
		{
			if (It->GetName() == DelegateName)
			{
				DelegateProp = *It;
				break;
			}
		}

		if (!DelegateProp)
		{
			FString Available;
			for (TFieldIterator<FMulticastDelegateProperty> It(ComponentClass); It; ++It)
			{
				if (!Available.IsEmpty()) Available += TEXT(", ");
				Available += It->GetName();
			}
			return MCPError(FString::Printf(TEXT("Delegate '%s' not found on %s. Available: %s"), *DelegateName, *ComponentClass->GetName(), *Available));
		}

		// Find the class that actually declares the delegate (may be a parent class)
		UClass* DelegateOwner = ComponentClass;
		for (UClass* TestClass = ComponentClass; TestClass; TestClass = TestClass->GetSuperClass())
		{
			if (FindFProperty<FMulticastDelegateProperty>(TestClass, DelegateProp->GetFName()))
			{
				DelegateOwner = TestClass;
			}
			else
			{
				break;
			}
		}

		UE_LOG(LogUnrealMCP, Log, TEXT("Creating ComponentBoundEvent: Component='%s' Delegate='%s' OwnerClass='%s'"),
			*ComponentName, *DelegateProp->GetName(), *DelegateOwner->GetName());

		// Cast to FObjectProperty for InitializeComponentBoundEventParams
		FObjectProperty* ObjProp = CastField<FObjectProperty>(ComponentProp);
		if (!ObjProp)
		{
			return MCPError(FString::Printf(TEXT("Component property '%s' is not an FObjectProperty"), *ComponentName));
		}

		UK2Node_ComponentBoundEvent* EventNode = NewObject<UK2Node_ComponentBoundEvent>(TargetGraph);
		EventNode->NodePosX = PosX;
		EventNode->NodePosY = PosY;
		TargetGraph->AddNode(EventNode, false, false);
		// Use the proper initialization method — sets EventReference, CustomFunctionName, etc.
		EventNode->InitializeComponentBoundEventParams(ObjProp, DelegateProp);
		EventNode->AllocateDefaultPins();
		NewNode = EventNode;
	}
	else if (NodeType == TEXT("VariableGet"))
	{
		FString VarName;
		if (!Params->TryGetStringField(TEXT("variable_name"), VarName))
		{
			return MCPError(TEXT("VariableGet requires variable_name param"));
		}

		UK2Node_VariableGet* GetNode = NewObject<UK2Node_VariableGet>(TargetGraph);
		GetNode->VariableReference.SetSelfMember(FName(*VarName));
		GetNode->NodePosX = PosX;
		GetNode->NodePosY = PosY;
		GetNode->AllocateDefaultPins();
		TargetGraph->AddNode(GetNode, false, false);
		NewNode = GetNode;
	}
	else if (NodeType == TEXT("VariableSet"))
	{
		FString VarName;
		if (!Params->TryGetStringField(TEXT("variable_name"), VarName))
		{
			return MCPError(TEXT("VariableSet requires variable_name param"));
		}

		UK2Node_VariableSet* SetNode = NewObject<UK2Node_VariableSet>(TargetGraph);
		SetNode->VariableReference.SetSelfMember(FName(*VarName));
		SetNode->NodePosX = PosX;
		SetNode->NodePosY = PosY;
		SetNode->AllocateDefaultPins();
		TargetGraph->AddNode(SetNode, false, false);
		NewNode = SetNode;
	}
	else if (NodeType == TEXT("IfThenElse") || NodeType == TEXT("Branch"))
	{
		UK2Node_IfThenElse* BranchNode = NewObject<UK2Node_IfThenElse>(TargetGraph);
		BranchNode->NodePosX = PosX;
		BranchNode->NodePosY = PosY;
		BranchNode->AllocateDefaultPins();
		TargetGraph->AddNode(BranchNode, false, false);
		NewNode = BranchNode;
	}
	else if (NodeType == TEXT("Sequence"))
	{
		UK2Node_ExecutionSequence* SeqNode = NewObject<UK2Node_ExecutionSequence>(TargetGraph);
		SeqNode->NodePosX = PosX;
		SeqNode->NodePosY = PosY;
		SeqNode->AllocateDefaultPins();
		TargetGraph->AddNode(SeqNode, false, false);
		NewNode = SeqNode;
	}
	else if (NodeType == TEXT("MakeStruct"))
	{
		FString StructName;
		if (!Params->TryGetStringField(TEXT("struct_name"), StructName))
		{
			return MCPError(TEXT("MakeStruct requires struct_name param (e.g. 'Vector', 'Rotator', 'Transform', 'LinearColor')"));
		}

		UScriptStruct* Struct = nullptr;
		// Try common structs first
		if (StructName == TEXT("Vector") || StructName == TEXT("FVector"))
			Struct = TBaseStructure<FVector>::Get();
		else if (StructName == TEXT("Rotator") || StructName == TEXT("FRotator"))
			Struct = TBaseStructure<FRotator>::Get();
		else if (StructName == TEXT("Transform") || StructName == TEXT("FTransform"))
			Struct = TBaseStructure<FTransform>::Get();
		else if (StructName == TEXT("LinearColor") || StructName == TEXT("FLinearColor"))
			Struct = TBaseStructure<FLinearColor>::Get();
		else if (StructName == TEXT("Vector2D") || StructName == TEXT("FVector2D"))
			Struct = TBaseStructure<FVector2D>::Get();
		else
		{
			Struct = FindFirstObject<UScriptStruct>(*StructName, EFindFirstObjectOptions::ExactClass, ELogVerbosity::NoLogging);
			if (!Struct)
				Struct = FindFirstObject<UScriptStruct>(*(TEXT("F") + StructName), EFindFirstObjectOptions::ExactClass, ELogVerbosity::NoLogging);
		}

		if (!Struct)
		{
			return MCPError(FString::Printf(TEXT("Struct not found: %s"), *StructName));
		}

		UK2Node_MakeStruct* MakeNode = NewObject<UK2Node_MakeStruct>(TargetGraph);
		MakeNode->StructType = Struct;
		MakeNode->NodePosX = PosX;
		MakeNode->NodePosY = PosY;
		MakeNode->AllocateDefaultPins();
		TargetGraph->AddNode(MakeNode, false, false);
		NewNode = MakeNode;
	}
	else if (NodeType == TEXT("BreakStruct"))
	{
		FString StructName;
		if (!Params->TryGetStringField(TEXT("struct_name"), StructName))
		{
			return MCPError(TEXT("BreakStruct requires struct_name param (e.g. 'Vector', 'Rotator', 'Transform', 'LinearColor')"));
		}

		UScriptStruct* Struct = nullptr;
		if (StructName == TEXT("Vector") || StructName == TEXT("FVector"))
			Struct = TBaseStructure<FVector>::Get();
		else if (StructName == TEXT("Rotator") || StructName == TEXT("FRotator"))
			Struct = TBaseStructure<FRotator>::Get();
		else if (StructName == TEXT("Transform") || StructName == TEXT("FTransform"))
			Struct = TBaseStructure<FTransform>::Get();
		else if (StructName == TEXT("LinearColor") || StructName == TEXT("FLinearColor"))
			Struct = TBaseStructure<FLinearColor>::Get();
		else if (StructName == TEXT("Vector2D") || StructName == TEXT("FVector2D"))
			Struct = TBaseStructure<FVector2D>::Get();
		else
		{
			Struct = FindFirstObject<UScriptStruct>(*StructName, EFindFirstObjectOptions::ExactClass, ELogVerbosity::NoLogging);
			if (!Struct)
				Struct = FindFirstObject<UScriptStruct>(*(TEXT("F") + StructName), EFindFirstObjectOptions::ExactClass, ELogVerbosity::NoLogging);
		}

		if (!Struct)
		{
			return MCPError(FString::Printf(TEXT("Struct not found: %s"), *StructName));
		}

		UK2Node_BreakStruct* BreakNode = NewObject<UK2Node_BreakStruct>(TargetGraph);
		BreakNode->StructType = Struct;
		BreakNode->NodePosX = PosX;
		BreakNode->NodePosY = PosY;
		BreakNode->AllocateDefaultPins();
		TargetGraph->AddNode(BreakNode, false, false);
		NewNode = BreakNode;
	}
	else if (NodeType == TEXT("Cast"))
	{
		FString TargetClassName;
		if (!Params->TryGetStringField(TEXT("target_class"), TargetClassName))
		{
			return MCPError(TEXT("Cast requires target_class param"));
		}

		UClass* CastClass = FindClassByName(TargetClassName);
		if (!CastClass) CastClass = FindClassByName(TEXT("A") + TargetClassName);
		if (!CastClass) CastClass = FindClassByName(TEXT("U") + TargetClassName);
		if (!CastClass)
		{
			return MCPError(FString::Printf(TEXT("Class not found for cast: %s"), *TargetClassName));
		}

		UK2Node_DynamicCast* CastNode = NewObject<UK2Node_DynamicCast>(TargetGraph);
		CastNode->TargetType = CastClass;
		CastNode->SetPurity(false);
		CastNode->NodePosX = PosX;
		CastNode->NodePosY = PosY;
		CastNode->AllocateDefaultPins();
		TargetGraph->AddNode(CastNode, false, false);
		NewNode = CastNode;
	}
	else if (NodeType == TEXT("GetArrayItem") || NodeType == TEXT("ArrayGet"))
	{
		UK2Node_GetArrayItem* ArrayGetNode = NewObject<UK2Node_GetArrayItem>(TargetGraph);
		ArrayGetNode->NodePosX = PosX;
		ArrayGetNode->NodePosY = PosY;
		ArrayGetNode->AllocateDefaultPins();
		TargetGraph->AddNode(ArrayGetNode, false, false);
		NewNode = ArrayGetNode;
	}
	else if (NodeType == TEXT("MakeArray"))
	{
		UK2Node_MakeArray* MakeArrayNode = NewObject<UK2Node_MakeArray>(TargetGraph);
		MakeArrayNode->NodePosX = PosX;
		MakeArrayNode->NodePosY = PosY;
		MakeArrayNode->AllocateDefaultPins();
		TargetGraph->AddNode(MakeArrayNode, false, false);
		NewNode = MakeArrayNode;
	}
	else if (NodeType == TEXT("SpawnActorFromClass"))
	{
		UK2Node_SpawnActorFromClass* SpawnNode = NewObject<UK2Node_SpawnActorFromClass>(TargetGraph);
		SpawnNode->NodePosX = PosX;
		SpawnNode->NodePosY = PosY;
		SpawnNode->AllocateDefaultPins();
		TargetGraph->AddNode(SpawnNode, false, false);
		NewNode = SpawnNode;
	}
	else if (NodeType == TEXT("Select"))
	{
		UK2Node_Select* SelectNode = NewObject<UK2Node_Select>(TargetGraph);
		SelectNode->NodePosX = PosX;
		SelectNode->NodePosY = PosY;
		SelectNode->AllocateDefaultPins();
		TargetGraph->AddNode(SelectNode, false, false);
		NewNode = SelectNode;
	}
	else if (NodeType == TEXT("SwitchOnInt") || NodeType == TEXT("SwitchInteger"))
	{
		UK2Node_SwitchInteger* SwitchNode = NewObject<UK2Node_SwitchInteger>(TargetGraph);
		SwitchNode->NodePosX = PosX;
		SwitchNode->NodePosY = PosY;
		SwitchNode->AllocateDefaultPins();
		TargetGraph->AddNode(SwitchNode, false, false);
		NewNode = SwitchNode;
	}
	else if (NodeType == TEXT("SwitchOnString") || NodeType == TEXT("SwitchString"))
	{
		UK2Node_SwitchString* SwitchNode = NewObject<UK2Node_SwitchString>(TargetGraph);
		SwitchNode->NodePosX = PosX;
		SwitchNode->NodePosY = PosY;
		SwitchNode->AllocateDefaultPins();
		TargetGraph->AddNode(SwitchNode, false, false);
		NewNode = SwitchNode;
	}
	else if (NodeType == TEXT("SwitchOnName") || NodeType == TEXT("SwitchName"))
	{
		UK2Node_SwitchName* SwitchNode = NewObject<UK2Node_SwitchName>(TargetGraph);
		SwitchNode->NodePosX = PosX;
		SwitchNode->NodePosY = PosY;
		SwitchNode->AllocateDefaultPins();
		TargetGraph->AddNode(SwitchNode, false, false);
		NewNode = SwitchNode;
	}
	else if (NodeType == TEXT("CallDelegate"))
	{
		FString DelegateName;
		if (!Params->TryGetStringField(TEXT("delegate_name"), DelegateName))
		{
			return MCPError(TEXT("CallDelegate requires delegate_name param"));
		}

		UK2Node_CallDelegate* DelegateNode = NewObject<UK2Node_CallDelegate>(TargetGraph);
		FMemberReference DelegateRef;
		DelegateRef.SetSelfMember(FName(*DelegateName));
		DelegateNode->SetFromProperty(FindFProperty<FMulticastDelegateProperty>(BP->SkeletonGeneratedClass, FName(*DelegateName)), false, BP->SkeletonGeneratedClass);
		DelegateNode->NodePosX = PosX;
		DelegateNode->NodePosY = PosY;
		DelegateNode->AllocateDefaultPins();
		TargetGraph->AddNode(DelegateNode, false, false);
		NewNode = DelegateNode;
	}
	else if (NodeType == TEXT("BindDelegate") || NodeType == TEXT("AddDelegate"))
	{
		FString DelegateName;
		if (!Params->TryGetStringField(TEXT("delegate_name"), DelegateName))
		{
			return MCPError(TEXT("BindDelegate requires delegate_name param"));
		}

		UK2Node_AddDelegate* AddDelegateNode = NewObject<UK2Node_AddDelegate>(TargetGraph);
		FMemberReference DelegateRef;
		DelegateRef.SetSelfMember(FName(*DelegateName));
		AddDelegateNode->SetFromProperty(FindFProperty<FMulticastDelegateProperty>(BP->SkeletonGeneratedClass, FName(*DelegateName)), false, BP->SkeletonGeneratedClass);
		AddDelegateNode->NodePosX = PosX;
		AddDelegateNode->NodePosY = PosY;
		AddDelegateNode->AllocateDefaultPins();
		TargetGraph->AddNode(AddDelegateNode, false, false);
		NewNode = AddDelegateNode;
	}
	else if (NodeType == TEXT("RemoveDelegate") || NodeType == TEXT("UnbindDelegate"))
	{
		FString DelegateName;
		if (!Params->TryGetStringField(TEXT("delegate_name"), DelegateName))
		{
			return MCPError(TEXT("RemoveDelegate requires delegate_name param"));
		}

		UK2Node_RemoveDelegate* RemoveDelegateNode = NewObject<UK2Node_RemoveDelegate>(TargetGraph);
		FMemberReference DelegateRef;
		DelegateRef.SetSelfMember(FName(*DelegateName));
		RemoveDelegateNode->SetFromProperty(FindFProperty<FMulticastDelegateProperty>(BP->SkeletonGeneratedClass, FName(*DelegateName)), false, BP->SkeletonGeneratedClass);
		RemoveDelegateNode->NodePosX = PosX;
		RemoveDelegateNode->NodePosY = PosY;
		RemoveDelegateNode->AllocateDefaultPins();
		TargetGraph->AddNode(RemoveDelegateNode, false, false);
		NewNode = RemoveDelegateNode;
	}
	else if (NodeType == TEXT("ClearDelegate"))
	{
		FString DelegateName;
		if (!Params->TryGetStringField(TEXT("delegate_name"), DelegateName))
		{
			return MCPError(TEXT("ClearDelegate requires delegate_name param"));
		}

		UK2Node_ClearDelegate* ClearDelegateNode = NewObject<UK2Node_ClearDelegate>(TargetGraph);
		ClearDelegateNode->SetFromProperty(FindFProperty<FMulticastDelegateProperty>(BP->SkeletonGeneratedClass, FName(*DelegateName)), false, BP->SkeletonGeneratedClass);
		ClearDelegateNode->NodePosX = PosX;
		ClearDelegateNode->NodePosY = PosY;
		ClearDelegateNode->AllocateDefaultPins();
		TargetGraph->AddNode(ClearDelegateNode, false, false);
		NewNode = ClearDelegateNode;
	}
	else if (NodeType == TEXT("CreateDelegate") || NodeType == TEXT("AssignDelegate"))
	{
		UK2Node_CreateDelegate* CreateDelegateNode = NewObject<UK2Node_CreateDelegate>(TargetGraph);
		CreateDelegateNode->NodePosX = PosX;
		CreateDelegateNode->NodePosY = PosY;
		CreateDelegateNode->AllocateDefaultPins();
		TargetGraph->AddNode(CreateDelegateNode, false, false);
		NewNode = CreateDelegateNode;
	}
	else if (NodeType == TEXT("ForEachLoop"))
	{
		// ForEachLoop is a macro — find and instantiate the macro graph
		UEdGraph* MacroGraph = nullptr;

		// Search engine-level macro library for ForEachLoop
		TArray<FAssetData> MacroLibraries;
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetReg.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), MacroLibraries);

		for (const FAssetData& Asset : MacroLibraries)
		{
			UBlueprint* MacroBP = Cast<UBlueprint>(Asset.GetAsset());
			if (!MacroBP || MacroBP->BlueprintType != BPTYPE_MacroLibrary) continue;

			for (UEdGraph* Graph : MacroBP->MacroGraphs)
			{
				if (Graph->GetName() == TEXT("ForEachLoop"))
				{
					MacroGraph = Graph;
					break;
				}
			}
			if (MacroGraph) break;
		}

		if (!MacroGraph)
		{
			return MCPError(TEXT("Could not find ForEachLoop macro. Use CallFunction with Array utility functions instead."));
		}

		UK2Node_MacroInstance* MacroNode = NewObject<UK2Node_MacroInstance>(TargetGraph);
		MacroNode->SetMacroGraph(MacroGraph);
		MacroNode->NodePosX = PosX;
		MacroNode->NodePosY = PosY;
		MacroNode->AllocateDefaultPins();
		TargetGraph->AddNode(MacroNode, false, false);
		NewNode = MacroNode;
	}
	else if (NodeType == TEXT("MacroInstance"))
	{
		FString MacroName;
		if (!Params->TryGetStringField(TEXT("macro_name"), MacroName))
		{
			return MCPError(TEXT("MacroInstance requires macro_name param (e.g. 'ForEachLoop', 'WhileLoop', 'IsValid')"));
		}

		UEdGraph* MacroGraph = nullptr;
		TArray<FAssetData> MacroLibraries;
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetReg.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), MacroLibraries);

		for (const FAssetData& Asset : MacroLibraries)
		{
			UBlueprint* MacroBP = Cast<UBlueprint>(Asset.GetAsset());
			if (!MacroBP || MacroBP->BlueprintType != BPTYPE_MacroLibrary) continue;

			for (UEdGraph* Graph : MacroBP->MacroGraphs)
			{
				if (Graph->GetName() == MacroName)
				{
					MacroGraph = Graph;
					break;
				}
			}
			if (MacroGraph) break;
		}

		if (!MacroGraph)
		{
			return MCPError(FString::Printf(TEXT("Macro not found: %s"), *MacroName));
		}

		UK2Node_MacroInstance* MacroNode = NewObject<UK2Node_MacroInstance>(TargetGraph);
		MacroNode->SetMacroGraph(MacroGraph);
		MacroNode->NodePosX = PosX;
		MacroNode->NodePosY = PosY;
		MacroNode->AllocateDefaultPins();
		TargetGraph->AddNode(MacroNode, false, false);
		NewNode = MacroNode;
	}
	else
	{
		return MCPError(FString::Printf(TEXT("Unknown node_type: %s. Supported: CallFunction, Event, ComponentBoundEvent, VariableGet, VariableSet, Branch, Sequence, MakeStruct, BreakStruct, Cast, GetArrayItem, MakeArray, SpawnActorFromClass, Select, SwitchOnInt, SwitchOnString, SwitchOnName, CallDelegate, BindDelegate, RemoveDelegate, ClearDelegate, CreateDelegate, ForEachLoop, MacroInstance"), *NodeType));
	}

	if (!NewNode)
	{
		return MCPError(TEXT("Failed to create node"));
	}

	// Ensure the node has a valid GUID (some node types don't auto-assign one)
	if (!NewNode->NodeGuid.IsValid())
	{
		NewNode->CreateNewGuid();
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("node_id"), NewNode->NodeGuid.ToString());
	Result->SetStringField(TEXT("node_class"), NewNode->GetClass()->GetName());
	Result->SetStringField(TEXT("title"), NewNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString());

	TArray<TSharedPtr<FJsonValue>> PinArray;
	for (UEdGraphPin* Pin : NewNode->Pins)
	{
		auto PinObj = MakeShared<FJsonObject>();
		PinObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
		PinObj->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
		PinObj->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
		PinArray.Add(MakeShared<FJsonValueObject>(PinObj));
	}
	Result->SetArrayField(TEXT("pins"), PinArray);

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleConnectPins(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	FString SourceNodeId, SourcePinName, TargetNodeId, TargetPinName;
	if (!Params->TryGetStringField(TEXT("source_node_id"), SourceNodeId) ||
		!Params->TryGetStringField(TEXT("source_pin"), SourcePinName) ||
		!Params->TryGetStringField(TEXT("target_node_id"), TargetNodeId) ||
		!Params->TryGetStringField(TEXT("target_pin"), TargetPinName))
	{
		return MCPError(TEXT("Missing required params: source_node_id, source_pin, target_node_id, target_pin"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	FGuid SourceGuid, TargetGuid;
	FGuid::Parse(SourceNodeId, SourceGuid);
	FGuid::Parse(TargetNodeId, TargetGuid);

	UEdGraphNode* SourceNode = nullptr;
	UEdGraphNode* TargetNode = nullptr;

	auto SearchGraphs = [&](const TArray<TObjectPtr<UEdGraph>>& Graphs)
	{
		for (UEdGraph* Graph : Graphs)
		{
			for (UEdGraphNode* Node : Graph->Nodes)
			{
				if (Node->NodeGuid == SourceGuid) SourceNode = Node;
				if (Node->NodeGuid == TargetGuid) TargetNode = Node;
			}
		}
	};

	SearchGraphs(BP->UbergraphPages);
	SearchGraphs(BP->FunctionGraphs);

	if (!SourceNode)
	{
		return MCPError(FString::Printf(TEXT("Source node not found: %s"), *SourceNodeId));
	}
	if (!TargetNode)
	{
		return MCPError(FString::Printf(TEXT("Target node not found: %s"), *TargetNodeId));
	}

	UEdGraphPin* SourcePin = nullptr;
	UEdGraphPin* TargetPin = nullptr;

	for (UEdGraphPin* Pin : SourceNode->Pins)
	{
		if (Pin->PinName.ToString() == SourcePinName) { SourcePin = Pin; break; }
	}
	for (UEdGraphPin* Pin : TargetNode->Pins)
	{
		if (Pin->PinName.ToString() == TargetPinName) { TargetPin = Pin; break; }
	}

	if (!SourcePin)
	{
		return MCPError(FString::Printf(TEXT("Source pin not found: %s on node %s"), *SourcePinName, *SourceNodeId));
	}
	if (!TargetPin)
	{
		return MCPError(FString::Printf(TEXT("Target pin not found: %s on node %s"), *TargetPinName, *TargetNodeId));
	}

	bool bConnected = SourcePin->GetSchema()->TryCreateConnection(SourcePin, TargetPin);
	if (!bConnected)
	{
		return MCPError(TEXT("Failed to create connection. Pins may be incompatible."));
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);
	FKismetEditorUtilities::CompileBlueprint(BP, EBlueprintCompileOptions::SkipGarbageCollection);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("source"), FString::Printf(TEXT("%s.%s"), *SourceNodeId, *SourcePinName));
	Result->SetStringField(TEXT("target"), FString::Printf(TEXT("%s.%s"), *TargetNodeId, *TargetPinName));
	Result->SetBoolField(TEXT("compiled"), true);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleCompileBP(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	FKismetEditorUtilities::CompileBlueprint(BP, EBlueprintCompileOptions::SkipGarbageCollection);

	bool bHasErrors = (BP->Status == BS_Error);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("blueprint"), BP->GetPathName());
	Result->SetBoolField(TEXT("has_errors"), bHasErrors);
	Result->SetStringField(TEXT("status"), bHasErrors ? TEXT("error") : TEXT("ok"));

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddBPVariable(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	FString VarName;
	if (!Params->TryGetStringField(TEXT("variable_name"), VarName))
	{
		return MCPError(TEXT("Missing required param: variable_name"));
	}

	FString VarType;
	if (!Params->TryGetStringField(TEXT("variable_type"), VarType))
	{
		return MCPError(TEXT("Missing required param: variable_type"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	FEdGraphPinType PinType;

	if (VarType == TEXT("bool") || VarType == TEXT("Boolean"))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
	}
	else if (VarType == TEXT("int") || VarType == TEXT("Integer") || VarType == TEXT("int32"))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
	}
	else if (VarType == TEXT("float") || VarType == TEXT("Float") || VarType == TEXT("double"))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Real;
		PinType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
	}
	else if (VarType == TEXT("string") || VarType == TEXT("String") || VarType == TEXT("FString"))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_String;
	}
	else if (VarType == TEXT("text") || VarType == TEXT("Text") || VarType == TEXT("FText"))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Text;
	}
	else if (VarType == TEXT("name") || VarType == TEXT("Name") || VarType == TEXT("FName"))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Name;
	}
	else if (VarType == TEXT("Vector") || VarType == TEXT("FVector"))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
	}
	else if (VarType == TEXT("Rotator") || VarType == TEXT("FRotator"))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
	}
	else if (VarType == TEXT("Transform") || VarType == TEXT("FTransform"))
	{
		PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		PinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
	}
	else
	{
		// Try as object type
		UClass* ObjClass = FindClassByName(VarType);
		if (!ObjClass) ObjClass = FindClassByName(TEXT("A") + VarType);
		if (!ObjClass) ObjClass = FindClassByName(TEXT("U") + VarType);

		if (ObjClass)
		{
			PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
			PinType.PinSubCategoryObject = ObjClass;
		}
		else
		{
			return MCPError(FString::Printf(TEXT("Unknown variable type: %s. Supported: bool, int, float, string, text, name, Vector, Rotator, Transform, or any UClass name"), *VarType));
		}
	}

	bool bIsArray = false;
	Params->TryGetBoolField(TEXT("is_array"), bIsArray);
	if (bIsArray)
	{
		PinType.ContainerType = EPinContainerType::Array;
	}

	bool bSuccess = FBlueprintEditorUtils::AddMemberVariable(BP, FName(*VarName), PinType);
	if (!bSuccess)
	{
		return MCPError(FString::Printf(TEXT("Failed to add variable: %s (may already exist)"), *VarName));
	}

	bool bInstanceEditable = false;
	if (Params->TryGetBoolField(TEXT("instance_editable"), bInstanceEditable) && bInstanceEditable)
	{
		FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(BP, FName(*VarName), false);
	}

	FString DefaultValue;
	if (Params->TryGetStringField(TEXT("default_value"), DefaultValue))
	{
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(BP, FName(*VarName), nullptr, TEXT("DefaultValue"), DefaultValue);
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("blueprint"), BP->GetPathName());
	Result->SetStringField(TEXT("variable_name"), VarName);
	Result->SetStringField(TEXT("variable_type"), VarType);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleRemoveBPNode(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	FString NodeId;
	if (!Params->TryGetStringField(TEXT("node_id"), NodeId))
	{
		return MCPError(TEXT("Missing required param: node_id"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	FGuid TargetGuid;
	FGuid::Parse(NodeId, TargetGuid);

	UEdGraphNode* TargetNode = nullptr;
	UEdGraph* ContainingGraph = nullptr;

	auto SearchGraphs = [&](const TArray<TObjectPtr<UEdGraph>>& Graphs)
	{
		for (UEdGraph* Graph : Graphs)
		{
			for (UEdGraphNode* Node : Graph->Nodes)
			{
				if (Node->NodeGuid == TargetGuid)
				{
					TargetNode = Node;
					ContainingGraph = Graph;
					return;
				}
			}
		}
	};

	SearchGraphs(BP->UbergraphPages);
	if (!TargetNode) SearchGraphs(BP->FunctionGraphs);

	if (!TargetNode)
	{
		return MCPError(FString::Printf(TEXT("Node not found: %s"), *NodeId));
	}

	FString NodeTitle = TargetNode->GetNodeTitle(ENodeTitleType::FullTitle).ToString();

	// Break all pin connections first
	for (UEdGraphPin* Pin : TargetNode->Pins)
	{
		Pin->BreakAllPinLinks();
	}

	ContainingGraph->RemoveNode(TargetNode);
	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("removed_node"), NodeTitle);
	Result->SetStringField(TEXT("node_id"), NodeId);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleDisconnectPins(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	FString NodeId, PinName;
	bool bDisconnectAll = false;

	// Mode 1: disconnect a specific pin from all connections
	// Mode 2: disconnect two specific pins from each other
	if (!Params->TryGetStringField(TEXT("node_id"), NodeId))
	{
		return MCPError(TEXT("Missing required param: node_id"));
	}
	if (!Params->TryGetStringField(TEXT("pin_name"), PinName))
	{
		return MCPError(TEXT("Missing required param: pin_name"));
	}

	FGuid NodeGuid;
	FGuid::Parse(NodeId, NodeGuid);

	UEdGraphNode* Node = nullptr;
	auto SearchGraphs = [&](const TArray<TObjectPtr<UEdGraph>>& Graphs)
	{
		for (UEdGraph* Graph : Graphs)
		{
			for (UEdGraphNode* N : Graph->Nodes)
			{
				if (N->NodeGuid == NodeGuid) { Node = N; return; }
			}
		}
	};
	SearchGraphs(BP->UbergraphPages);
	if (!Node) SearchGraphs(BP->FunctionGraphs);

	if (!Node)
	{
		return MCPError(FString::Printf(TEXT("Node not found: %s"), *NodeId));
	}

	UEdGraphPin* Pin = nullptr;
	for (UEdGraphPin* P : Node->Pins)
	{
		if (P->PinName.ToString() == PinName) { Pin = P; break; }
	}
	if (!Pin)
	{
		return MCPError(FString::Printf(TEXT("Pin not found: %s on node %s"), *PinName, *NodeId));
	}

	// Check if we should disconnect from a specific target
	FString TargetNodeId, TargetPinName;
	if (Params->TryGetStringField(TEXT("target_node_id"), TargetNodeId) &&
		Params->TryGetStringField(TEXT("target_pin"), TargetPinName))
	{
		FGuid TargetGuid;
		FGuid::Parse(TargetNodeId, TargetGuid);

		UEdGraphNode* TargetNode = nullptr;
		auto SearchGraphs2 = [&](const TArray<TObjectPtr<UEdGraph>>& Graphs)
		{
			for (UEdGraph* Graph : Graphs)
			{
				for (UEdGraphNode* N : Graph->Nodes)
				{
					if (N->NodeGuid == TargetGuid) { TargetNode = N; return; }
				}
			}
		};
		SearchGraphs2(BP->UbergraphPages);
		if (!TargetNode) SearchGraphs2(BP->FunctionGraphs);

		if (!TargetNode)
		{
			return MCPError(FString::Printf(TEXT("Target node not found: %s"), *TargetNodeId));
		}

		UEdGraphPin* TargetPin = nullptr;
		for (UEdGraphPin* P : TargetNode->Pins)
		{
			if (P->PinName.ToString() == TargetPinName) { TargetPin = P; break; }
		}
		if (!TargetPin)
		{
			return MCPError(FString::Printf(TEXT("Target pin not found: %s on node %s"), *TargetPinName, *TargetNodeId));
		}

		Pin->BreakLinkTo(TargetPin);
	}
	else
	{
		Pin->BreakAllPinLinks();
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("disconnected_pin"), FString::Printf(TEXT("%s.%s"), *NodeId, *PinName));
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSetPinDefault(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	FString NodeId;
	if (!Params->TryGetStringField(TEXT("node_id"), NodeId))
	{
		return MCPError(TEXT("Missing required param: node_id"));
	}

	FString PinName;
	if (!Params->TryGetStringField(TEXT("pin_name"), PinName))
	{
		return MCPError(TEXT("Missing required param: pin_name"));
	}

	FString DefaultValue;
	if (!Params->TryGetStringField(TEXT("value"), DefaultValue))
	{
		return MCPError(TEXT("Missing required param: value"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	FGuid NodeGuid;
	FGuid::Parse(NodeId, NodeGuid);

	UEdGraphNode* Node = nullptr;
	auto SearchGraphs = [&](const TArray<TObjectPtr<UEdGraph>>& Graphs)
	{
		for (UEdGraph* Graph : Graphs)
		{
			for (UEdGraphNode* N : Graph->Nodes)
			{
				if (N->NodeGuid == NodeGuid) { Node = N; return; }
			}
		}
	};
	SearchGraphs(BP->UbergraphPages);
	if (!Node) SearchGraphs(BP->FunctionGraphs);

	if (!Node)
	{
		return MCPError(FString::Printf(TEXT("Node not found: %s"), *NodeId));
	}

	UEdGraphPin* Pin = nullptr;
	for (UEdGraphPin* P : Node->Pins)
	{
		if (P->PinName.ToString() == PinName) { Pin = P; break; }
	}
	if (!Pin)
	{
		return MCPError(FString::Printf(TEXT("Pin not found: %s on node %s"), *PinName, *NodeId));
	}

	const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();

	// Class/object pins store their default in DefaultObject and text pins in
	// DefaultTextValue; TrySetDefaultValue only writes the string DefaultValue,
	// so those categories used to fail silently. Route by pin category and
	// verify the write actually landed.
	const FName PinCategory = Pin->PinType.PinCategory;
	if (PinCategory == UEdGraphSchema_K2::PC_Class || PinCategory == UEdGraphSchema_K2::PC_SoftClass)
	{
		UClass* Cls = DefaultValue.StartsWith(TEXT("/")) ? LoadObject<UClass>(nullptr, *DefaultValue) : nullptr;
		if (!Cls)
		{
			Cls = FindClassByName(DefaultValue);
		}
		if (!Cls)
		{
			return MCPError(FString::Printf(TEXT("Class not found for class pin '%s': '%s' (short name or /Script/Module.ClassName path)"), *PinName, *DefaultValue));
		}
		Schema->TrySetDefaultObject(*Pin, Cls);
		if (Pin->DefaultObject != Cls)
		{
			return MCPError(FString::Printf(TEXT("Schema rejected class '%s' for pin '%s' (class may not match the pin's allowed base class)"), *Cls->GetName(), *PinName));
		}
	}
	else if (PinCategory == UEdGraphSchema_K2::PC_Object || PinCategory == UEdGraphSchema_K2::PC_SoftObject)
	{
		UObject* Obj = LoadObject<UObject>(nullptr, *DefaultValue);
		if (!Obj)
		{
			return MCPError(FString::Printf(TEXT("Object not found for object pin '%s': '%s' (use a full object path)"), *PinName, *DefaultValue));
		}
		Schema->TrySetDefaultObject(*Pin, Obj);
		if (Pin->DefaultObject != Obj)
		{
			return MCPError(FString::Printf(TEXT("Schema rejected object '%s' for pin '%s'"), *Obj->GetName(), *PinName));
		}
	}
	else if (PinCategory == UEdGraphSchema_K2::PC_Text)
	{
		Schema->TrySetDefaultText(*Pin, FText::FromString(DefaultValue));
		if (!Pin->DefaultTextValue.ToString().Equals(DefaultValue))
		{
			return MCPError(FString::Printf(TEXT("Failed to set text default on pin '%s'"), *PinName));
		}
	}
	else
	{
		Schema->TrySetDefaultValue(*Pin, DefaultValue);
		if (Pin->DefaultValue.IsEmpty() && !DefaultValue.IsEmpty())
		{
			return MCPError(FString::Printf(TEXT("Schema rejected value '%s' for pin '%s' (pin category: %s)"), *DefaultValue, *PinName, *PinCategory.ToString()));
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("node_id"), NodeId);
	Result->SetStringField(TEXT("pin_name"), PinName);
	Result->SetStringField(TEXT("new_value"), Pin->GetDefaultAsString());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddBPFunction(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	FString FuncName;
	if (!Params->TryGetStringField(TEXT("function_name"), FuncName))
	{
		return MCPError(TEXT("Missing required param: function_name"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	// Check if function already exists
	for (UEdGraph* Graph : BP->FunctionGraphs)
	{
		if (Graph->GetName() == FuncName)
		{
			return MCPError(FString::Printf(TEXT("Function already exists: %s"), *FuncName));
		}
	}

	UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(BP, FName(*FuncName), UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	FBlueprintEditorUtils::AddFunctionGraph<UFunction>(BP, NewGraph, true, nullptr);

	bool bIsPure = false;
	Params->TryGetBoolField(TEXT("is_pure"), bIsPure);

	// Add input/output parameters if specified
	const TArray<TSharedPtr<FJsonValue>>* InputsArray = nullptr;
	if (Params->TryGetArrayField(TEXT("inputs"), InputsArray))
	{
		for (const auto& InputVal : *InputsArray)
		{
			auto InputObj = InputVal->AsObject();
			if (!InputObj) continue;

			FString ParamName, ParamType;
			if (!InputObj->TryGetStringField(TEXT("name"), ParamName) ||
				!InputObj->TryGetStringField(TEXT("type"), ParamType))
				continue;

			FEdGraphPinType PinType;
			if (ParamType == TEXT("bool")) PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
			else if (ParamType == TEXT("int")) PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
			else if (ParamType == TEXT("float") || ParamType == TEXT("double"))
			{
				PinType.PinCategory = UEdGraphSchema_K2::PC_Real;
				PinType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
			}
			else if (ParamType == TEXT("string")) PinType.PinCategory = UEdGraphSchema_K2::PC_String;
			else if (ParamType == TEXT("Vector"))
			{
				PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
				PinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
			}
			else if (ParamType == TEXT("Rotator"))
			{
				PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
				PinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
			}
			else if (ParamType == TEXT("Transform"))
			{
				PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
				PinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
			}
			else
			{
				UClass* ObjClass = FindClassByName(ParamType);
				if (!ObjClass) ObjClass = FindClassByName(TEXT("A") + ParamType);
				if (!ObjClass) ObjClass = FindClassByName(TEXT("U") + ParamType);
				if (ObjClass)
				{
					PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
					PinType.PinSubCategoryObject = ObjClass;
				}
				else
				{
					PinType.PinCategory = UEdGraphSchema_K2::PC_String; // fallback
				}
			}

			// Find the function entry node and add the parameter
			for (UEdGraphNode* Node : NewGraph->Nodes)
			{
				if (UK2Node_FunctionEntry* EntryNode = Cast<UK2Node_FunctionEntry>(Node))
				{
					TSharedPtr<FUserPinInfo> NewPin = MakeShareable(new FUserPinInfo());
					NewPin->PinName = FName(*ParamName);
					NewPin->PinType = PinType;
					EntryNode->UserDefinedPins.Add(NewPin);
					EntryNode->ReconstructNode();
					break;
				}
			}
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* OutputsArray = nullptr;
	if (Params->TryGetArrayField(TEXT("outputs"), OutputsArray))
	{
		for (const auto& OutputVal : *OutputsArray)
		{
			auto OutputObj = OutputVal->AsObject();
			if (!OutputObj) continue;

			FString ParamName, ParamType;
			if (!OutputObj->TryGetStringField(TEXT("name"), ParamName) ||
				!OutputObj->TryGetStringField(TEXT("type"), ParamType))
				continue;

			FEdGraphPinType PinType;
			if (ParamType == TEXT("bool")) PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
			else if (ParamType == TEXT("int")) PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
			else if (ParamType == TEXT("float") || ParamType == TEXT("double"))
			{
				PinType.PinCategory = UEdGraphSchema_K2::PC_Real;
				PinType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
			}
			else if (ParamType == TEXT("string")) PinType.PinCategory = UEdGraphSchema_K2::PC_String;
			else if (ParamType == TEXT("Vector"))
			{
				PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
				PinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
			}
			else
			{
				UClass* ObjClass = FindClassByName(ParamType);
				if (!ObjClass) ObjClass = FindClassByName(TEXT("A") + ParamType);
				if (!ObjClass) ObjClass = FindClassByName(TEXT("U") + ParamType);
				if (ObjClass)
				{
					PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
					PinType.PinSubCategoryObject = ObjClass;
				}
				else
				{
					PinType.PinCategory = UEdGraphSchema_K2::PC_String;
				}
			}

			// Find or create the function result node
			UK2Node_FunctionResult* ResultNode = nullptr;
			for (UEdGraphNode* Node : NewGraph->Nodes)
			{
				ResultNode = Cast<UK2Node_FunctionResult>(Node);
				if (ResultNode) break;
			}
			if (!ResultNode)
			{
				ResultNode = FBlueprintEditorUtils::FindOrCreateFunctionResultNode(Cast<UK2Node_FunctionEntry>(NewGraph->Nodes[0]));
			}
			if (ResultNode)
			{
				TSharedPtr<FUserPinInfo> NewPin = MakeShareable(new FUserPinInfo());
				NewPin->PinName = FName(*ParamName);
				NewPin->PinType = PinType;
				ResultNode->UserDefinedPins.Add(NewPin);
				ResultNode->ReconstructNode();
			}
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("blueprint"), BP->GetPathName());
	Result->SetStringField(TEXT("function_name"), FuncName);
	Result->SetStringField(TEXT("graph_name"), NewGraph->GetName());

	// Return entry/result node IDs
	for (UEdGraphNode* Node : NewGraph->Nodes)
	{
		if (UK2Node_FunctionEntry* EntryNode = Cast<UK2Node_FunctionEntry>(Node))
		{
			Result->SetStringField(TEXT("entry_node_id"), EntryNode->NodeGuid.ToString());
		}
		else if (UK2Node_FunctionResult* ResultNode = Cast<UK2Node_FunctionResult>(Node))
		{
			Result->SetStringField(TEXT("result_node_id"), ResultNode->NodeGuid.ToString());
		}
	}

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleRemoveBPVariable(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	FString VarName;
	if (!Params->TryGetStringField(TEXT("variable_name"), VarName))
	{
		return MCPError(TEXT("Missing required param: variable_name"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	FBlueprintEditorUtils::RemoveMemberVariable(BP, FName(*VarName));
	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("blueprint"), BP->GetPathName());
	Result->SetStringField(TEXT("removed_variable"), VarName);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddEventDispatcher(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	FString DispatcherName;
	if (!Params->TryGetStringField(TEXT("name"), DispatcherName))
	{
		return MCPError(TEXT("Missing required param: name"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	// Create a multicast delegate variable
	FEdGraphPinType PinType;
	PinType.PinCategory = UEdGraphSchema_K2::PC_MCDelegate;

	bool bSuccess = FBlueprintEditorUtils::AddMemberVariable(BP, FName(*DispatcherName), PinType);
	if (!bSuccess)
	{
		return MCPError(FString::Printf(TEXT("Failed to add event dispatcher: %s (may already exist)"), *DispatcherName));
	}

	// Add parameters to the dispatcher signature if specified
	const TArray<TSharedPtr<FJsonValue>>* ParamsArray = nullptr;
	TArray<FString> ParamNames;
	if (Params->TryGetArrayField(TEXT("params"), ParamsArray) && ParamsArray->Num() > 0)
	{
		// First compile so the delegate signature graph is created
		FBlueprintEditorUtils::MarkBlueprintAsModified(BP);
		FKismetEditorUtilities::CompileBlueprint(BP, EBlueprintCompileOptions::SkipGarbageCollection);

		// Find the delegate signature function graph
		UEdGraph* SigGraph = nullptr;
		FName DispFName(*DispatcherName);
		for (UEdGraph* Graph : BP->DelegateSignatureGraphs)
		{
			if (Graph->GetFName() == DispFName || Graph->GetName().Contains(DispatcherName))
			{
				SigGraph = Graph;
				break;
			}
		}

		if (SigGraph)
		{
			// Find the function entry node
			UK2Node_FunctionEntry* EntryNode = nullptr;
			for (UEdGraphNode* Node : SigGraph->Nodes)
			{
				EntryNode = Cast<UK2Node_FunctionEntry>(Node);
				if (EntryNode) break;
			}

			if (EntryNode)
			{
				for (const auto& ParamVal : *ParamsArray)
				{
					auto ParamObj = ParamVal->AsObject();
					if (!ParamObj) continue;

					FString ParamName, ParamType;
					if (!ParamObj->TryGetStringField(TEXT("name"), ParamName) ||
						!ParamObj->TryGetStringField(TEXT("type"), ParamType))
						continue;

					FEdGraphPinType ParamPinType;
					if (ParamType == TEXT("bool")) ParamPinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
					else if (ParamType == TEXT("int")) ParamPinType.PinCategory = UEdGraphSchema_K2::PC_Int;
					else if (ParamType == TEXT("float") || ParamType == TEXT("double"))
					{
						ParamPinType.PinCategory = UEdGraphSchema_K2::PC_Real;
						ParamPinType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
					}
					else if (ParamType == TEXT("string")) ParamPinType.PinCategory = UEdGraphSchema_K2::PC_String;
					else if (ParamType == TEXT("Vector"))
					{
						ParamPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
						ParamPinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
					}
					else if (ParamType == TEXT("Rotator"))
					{
						ParamPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
						ParamPinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
					}
					else if (ParamType == TEXT("Transform"))
					{
						ParamPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
						ParamPinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
					}
					else
					{
						UClass* ObjClass = FindClassByName(ParamType);
						if (!ObjClass) ObjClass = FindClassByName(TEXT("A") + ParamType);
						if (!ObjClass) ObjClass = FindClassByName(TEXT("U") + ParamType);
						if (ObjClass)
						{
							ParamPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
							ParamPinType.PinSubCategoryObject = ObjClass;
						}
						else
						{
							ParamPinType.PinCategory = UEdGraphSchema_K2::PC_String; // fallback
						}
					}

					TSharedPtr<FUserPinInfo> NewPin = MakeShareable(new FUserPinInfo());
					NewPin->PinName = FName(*ParamName);
					NewPin->PinType = ParamPinType;
					EntryNode->UserDefinedPins.Add(NewPin);
					ParamNames.Add(ParamName);
				}
				EntryNode->ReconstructNode();
			}
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);
	FKismetEditorUtilities::CompileBlueprint(BP, EBlueprintCompileOptions::SkipGarbageCollection);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("blueprint"), BP->GetPathName());
	Result->SetStringField(TEXT("dispatcher_name"), DispatcherName);
	if (ParamNames.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> ParamNamesJson;
		for (const FString& Name : ParamNames)
		{
			ParamNamesJson.Add(MakeShared<FJsonValueString>(Name));
		}
		Result->SetArrayField(TEXT("parameters"), ParamNamesJson);
	}
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddBPComponent(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	FString ComponentClass;
	if (!Params->TryGetStringField(TEXT("component_class"), ComponentClass))
	{
		return MCPError(TEXT("Missing required param: component_class"));
	}

	FString ComponentName;
	Params->TryGetStringField(TEXT("component_name"), ComponentName);

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	UClass* CompClass = FindClassByName(ComponentClass);
	if (!CompClass) CompClass = FindClassByName(TEXT("U") + ComponentClass);
	if (!CompClass)
	{
		return MCPError(FString::Printf(TEXT("Component class not found: %s"), *ComponentClass));
	}

	if (!CompClass->IsChildOf(UActorComponent::StaticClass()))
	{
		return MCPError(FString::Printf(TEXT("%s is not an ActorComponent subclass"), *ComponentClass));
	}

	FName CompFName = ComponentName.IsEmpty() ? CompClass->GetFName() : FName(*ComponentName);

	USCS_Node* NewNode = BP->SimpleConstructionScript->CreateNode(CompClass, CompFName);
	if (!NewNode)
	{
		return MCPError(TEXT("Failed to create SCS node for component"));
	}

	BP->SimpleConstructionScript->AddNode(NewNode);
	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("blueprint"), BP->GetPathName());
	Result->SetStringField(TEXT("component_name"), NewNode->GetVariableName().ToString());
	Result->SetStringField(TEXT("component_class"), CompClass->GetName());
	return Result;
}

// ====================================================================================
// LEVEL COMMAND HANDLERS
// ====================================================================================

void FMCPTcpServer::RegisterLevelHandlers()
{
	RegisterHandler(TEXT("level_info"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleLevelInfo(Params); });
	RegisterHandler(TEXT("find_by_class"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleFindByClass(Params); });
	RegisterHandler(TEXT("find_in_radius"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleFindInRadius(Params); });
	RegisterHandler(TEXT("screenshot"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleScreenshot(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleLevelInfo(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World) return MCPError(TEXT("No editor world available"));

	int32 ActorCount = 0;
	FBox WorldBounds(ForceInit);
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		ActorCount++;
		FVector Origin, BoxExtent;
		It->GetActorBounds(false, Origin, BoxExtent);
		WorldBounds += FBox(Origin - BoxExtent, Origin + BoxExtent);
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("level_name"), World->GetMapName());
	Result->SetStringField(TEXT("level_path"), World->GetOutermost()->GetName());
	Result->SetNumberField(TEXT("actor_count"), ActorCount);

	if (WorldBounds.IsValid)
	{
		auto BoundsObj = MakeShared<FJsonObject>();
		FVector Min = WorldBounds.Min;
		FVector Max = WorldBounds.Max;
		BoundsObj->SetNumberField(TEXT("min_x"), Min.X);
		BoundsObj->SetNumberField(TEXT("min_y"), Min.Y);
		BoundsObj->SetNumberField(TEXT("min_z"), Min.Z);
		BoundsObj->SetNumberField(TEXT("max_x"), Max.X);
		BoundsObj->SetNumberField(TEXT("max_y"), Max.Y);
		BoundsObj->SetNumberField(TEXT("max_z"), Max.Z);
		Result->SetObjectField(TEXT("world_bounds"), BoundsObj);
	}

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleFindByClass(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World) return MCPError(TEXT("No editor world available"));

	FString ClassName;
	if (!Params->TryGetStringField(TEXT("class_name"), ClassName))
	{
		return MCPError(TEXT("Missing required param: class_name"));
	}

	TArray<TSharedPtr<FJsonValue>> ActorArray;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		FString ActorClassName = Actor->GetClass()->GetName();

		if (ActorClassName.Contains(ClassName) || Actor->GetClass()->GetFName().ToString().Contains(ClassName))
		{
			auto ActorObj = MakeShared<FJsonObject>();
			ActorObj->SetStringField(TEXT("name"), Actor->GetActorLabel());
			ActorObj->SetStringField(TEXT("internal_name"), Actor->GetName());
			ActorObj->SetStringField(TEXT("class"), ActorClassName);

			FVector Loc = Actor->GetActorLocation();
			ActorObj->SetNumberField(TEXT("x"), Loc.X);
			ActorObj->SetNumberField(TEXT("y"), Loc.Y);
			ActorObj->SetNumberField(TEXT("z"), Loc.Z);

			ActorArray.Add(MakeShared<FJsonValueObject>(ActorObj));
		}
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("class_filter"), ClassName);
	Result->SetArrayField(TEXT("actors"), ActorArray);
	Result->SetNumberField(TEXT("count"), ActorArray.Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleFindInRadius(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World) return MCPError(TEXT("No editor world available"));

	double CenterX, CenterY, CenterZ, Radius;
	if (!Params->TryGetNumberField(TEXT("x"), CenterX) ||
		!Params->TryGetNumberField(TEXT("y"), CenterY) ||
		!Params->TryGetNumberField(TEXT("z"), CenterZ) ||
		!Params->TryGetNumberField(TEXT("radius"), Radius))
	{
		return MCPError(TEXT("Missing required params: x, y, z, radius"));
	}

	FVector Center(CenterX, CenterY, CenterZ);
	double RadiusSq = Radius * Radius;

	FString ClassFilter;
	Params->TryGetStringField(TEXT("class_filter"), ClassFilter);

	TArray<TSharedPtr<FJsonValue>> ActorArray;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		double DistSq = FVector::DistSquared(Actor->GetActorLocation(), Center);
		if (DistSq <= RadiusSq)
		{
			if (!ClassFilter.IsEmpty() && !Actor->GetClass()->GetName().Contains(ClassFilter))
			{
				continue;
			}

			auto ActorObj = MakeShared<FJsonObject>();
			ActorObj->SetStringField(TEXT("name"), Actor->GetActorLabel());
			ActorObj->SetStringField(TEXT("internal_name"), Actor->GetName());
			ActorObj->SetStringField(TEXT("class"), Actor->GetClass()->GetName());

			FVector Loc = Actor->GetActorLocation();
			ActorObj->SetNumberField(TEXT("x"), Loc.X);
			ActorObj->SetNumberField(TEXT("y"), Loc.Y);
			ActorObj->SetNumberField(TEXT("z"), Loc.Z);
			ActorObj->SetNumberField(TEXT("distance"), FMath::Sqrt(DistSq));

			ActorArray.Add(MakeShared<FJsonValueObject>(ActorObj));
		}
	}

	auto Result = MCPSuccess();
	Result->SetNumberField(TEXT("center_x"), CenterX);
	Result->SetNumberField(TEXT("center_y"), CenterY);
	Result->SetNumberField(TEXT("center_z"), CenterZ);
	Result->SetNumberField(TEXT("radius"), Radius);
	Result->SetArrayField(TEXT("actors"), ActorArray);
	Result->SetNumberField(TEXT("count"), ActorArray.Num());
	return Result;
}

// ====================================================================================
// LANDSCAPE COMMAND HANDLERS
// ====================================================================================

// Helper: find the first landscape proxy in the editor world
static ALandscapeProxy* FindLandscapeProxy()
{
	UWorld* World = GetEditorWorld();
	if (!World) return nullptr;

	for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

void FMCPTcpServer::RegisterLandscapeHandlers()
{
	RegisterHandler(TEXT("landscape_info"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleLandscapeInfo(Params); });
	RegisterHandler(TEXT("create_landscape"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleCreateLandscape(Params); });
	RegisterHandler(TEXT("sculpt_landscape"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSculptLandscape(Params); });
	RegisterHandler(TEXT("sculpt_noise"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSculptNoise(Params); });
	RegisterHandler(TEXT("set_height_at"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSetHeightAt(Params); });
	RegisterHandler(TEXT("get_height_at"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleGetHeightAt(Params); });
	RegisterHandler(TEXT("import_heightmap"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleImportHeightmap(Params); });
	RegisterHandler(TEXT("paint_layer"), [this](const TSharedPtr<FJsonObject>& Params) { return HandlePaintLayer(Params); });
	RegisterHandler(TEXT("add_layer"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddLayer(Params); });
	RegisterHandler(TEXT("place_foliage"), [this](const TSharedPtr<FJsonObject>& Params) { return HandlePlaceFoliage(Params); });
	RegisterHandler(TEXT("clear_foliage"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleClearFoliage(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleLandscapeInfo(const TSharedPtr<FJsonObject>& Params)
{
	ALandscapeProxy* Landscape = FindLandscapeProxy();
	if (!Landscape)
	{
		return MCPError(TEXT("No landscape found in the current level. Use create_landscape to create one."));
	}

	ULandscapeInfo* LandInfo = Landscape->GetLandscapeInfo();
	if (!LandInfo)
	{
		return MCPError(TEXT("Landscape found but has no LandscapeInfo."));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("name"), Landscape->GetActorLabel());

	// Get bounds
	FVector Origin, BoxExtent;
	Landscape->GetActorBounds(false, Origin, BoxExtent);
	auto BoundsObj = MakeShared<FJsonObject>();
	BoundsObj->SetNumberField(TEXT("min_x"), Origin.X - BoxExtent.X);
	BoundsObj->SetNumberField(TEXT("min_y"), Origin.Y - BoxExtent.Y);
	BoundsObj->SetNumberField(TEXT("min_z"), Origin.Z - BoxExtent.Z);
	BoundsObj->SetNumberField(TEXT("max_x"), Origin.X + BoxExtent.X);
	BoundsObj->SetNumberField(TEXT("max_y"), Origin.Y + BoxExtent.Y);
	BoundsObj->SetNumberField(TEXT("max_z"), Origin.Z + BoxExtent.Z);
	Result->SetObjectField(TEXT("bounds"), BoundsObj);

	// Component count
	TArray<ULandscapeComponent*> LandComponents;
	Landscape->GetComponents<ULandscapeComponent>(LandComponents);
	Result->SetNumberField(TEXT("component_count"), LandComponents.Num());

	// Component size info
	if (LandComponents.Num() > 0)
	{
		ULandscapeComponent* FirstComp = LandComponents[0];
		Result->SetNumberField(TEXT("component_size_quads"), FirstComp->ComponentSizeQuads);
		Result->SetNumberField(TEXT("subsection_size_quads"), FirstComp->SubsectionSizeQuads);
		Result->SetNumberField(TEXT("num_subsections"), FirstComp->NumSubsections);
	}

	// Landscape material
	UMaterialInterface* Material = Landscape->GetLandscapeMaterial();
	if (Material)
	{
		Result->SetStringField(TEXT("material"), Material->GetPathName());
	}

	// Paint layers
	TArray<TSharedPtr<FJsonValue>> LayerArray;
	for (const FLandscapeInfoLayerSettings& LayerSettings : LandInfo->Layers)
	{
		auto LayerObj = MakeShared<FJsonObject>();
		LayerObj->SetStringField(TEXT("name"), LayerSettings.LayerName.ToString());
		if (LayerSettings.LayerInfoObj)
		{
			LayerObj->SetStringField(TEXT("info_path"), LayerSettings.LayerInfoObj->GetPathName());
		}
		LayerArray.Add(MakeShared<FJsonValueObject>(LayerObj));
	}
	Result->SetArrayField(TEXT("layers"), LayerArray);

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleCreateLandscape(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World) return MCPError(TEXT("No editor world available"));

	int32 SizeX = (int32)Params->GetNumberField(TEXT("size_x"));
	int32 SizeY = (int32)Params->GetNumberField(TEXT("size_y"));
	int32 SectionsPerComponent = 1;
	int32 QuadsPerSection = 63;

	double TempVal;
	if (Params->TryGetNumberField(TEXT("sections_per_component"), TempVal))
		SectionsPerComponent = (int32)TempVal;
	if (Params->TryGetNumberField(TEXT("quads_per_section"), TempVal))
		QuadsPerSection = (int32)TempVal;

	if (SizeX < 1 || SizeY < 1)
	{
		return MCPError(TEXT("size_x and size_y must be >= 1 (number of components)"));
	}

	// Total quads per component
	int32 ComponentSizeQuads = SectionsPerComponent * QuadsPerSection;
	// Total resolution in vertices
	int32 TotalVertsX = SizeX * ComponentSizeQuads + 1;
	int32 TotalVertsY = SizeY * ComponentSizeQuads + 1;

	// Create flat heightmap (32768 = sea level)
	TArray<uint16> HeightData;
	HeightData.SetNumUninitialized(TotalVertsX * TotalVertsY);
	for (int32 i = 0; i < HeightData.Num(); i++)
	{
		HeightData[i] = 32768;
	}

	// Create empty weight data layers
	TArray<FLandscapeImportLayerInfo> ImportLayers;

	// Spawn landscape
	ALandscape* NewLandscape = World->SpawnActor<ALandscape>(ALandscape::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	if (!NewLandscape)
	{
		return MCPError(TEXT("Failed to spawn landscape actor"));
	}

	// Set scale (100 units per quad is standard)
	NewLandscape->SetActorScale3D(FVector(100.0f, 100.0f, 100.0f));

	FGuid LandscapeGuid = FGuid::NewGuid();
	NewLandscape->SetLandscapeGuid(LandscapeGuid);

	// Import the heightmap data - engine expects default FGuid() as key
	TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
	HeightDataPerLayers.Add(FGuid(), MoveTemp(HeightData));
	MaterialLayerDataPerLayers.Add(FGuid(), MoveTemp(ImportLayers));

	NewLandscape->Import(
		LandscapeGuid,
		0, 0, TotalVertsX - 1, TotalVertsY - 1,
		SectionsPerComponent, QuadsPerSection,
		HeightDataPerLayers, TEXT(""),
		MaterialLayerDataPerLayers,
		ELandscapeImportAlphamapType::Additive,
		TArrayView<const FLandscapeLayer>()
	);

	// Apply material if specified
	FString MaterialPath;
	if (Params->TryGetStringField(TEXT("material_path"), MaterialPath))
	{
		UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
		if (Mat)
		{
			NewLandscape->LandscapeMaterial = Mat;
		}
	}

	// Register with editor
	NewLandscape->RegisterAllComponents();

	// Create a default edit layer so programmatic heightmap writes work properly in UE 5.7
	if (!NewLandscape->HasLayersContent())
	{
		NewLandscape->CreateLayer(FName(TEXT("Default")));
	}

	ULandscapeInfo* LandInfo = NewLandscape->GetLandscapeInfo();
	if (LandInfo)
	{
		LandInfo->UpdateLayerInfoMap(NewLandscape);
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("name"), NewLandscape->GetActorLabel());
	Result->SetNumberField(TEXT("resolution_x"), TotalVertsX);
	Result->SetNumberField(TEXT("resolution_y"), TotalVertsY);
	Result->SetNumberField(TEXT("components_x"), SizeX);
	Result->SetNumberField(TEXT("components_y"), SizeY);
	return Result;
}

// Helper: writes heightmap data to base (non-layer) heightmap directly, bypassing edit layers
static void WriteBaseHeightData(ALandscapeProxy* Landscape, ULandscapeInfo* LandInfo,
	int32 MinX, int32 MinY, int32 MaxX, int32 MaxY, uint16* Data)
{
	FLandscapeEditDataInterface LandscapeEdit(LandInfo);
	TSet<ULandscapeComponent*> ChangedComponents;
	LandscapeEdit.GetComponentsInRegion(MinX, MinY, MaxX, MaxY, &ChangedComponents);
	for (ULandscapeComponent* Component : ChangedComponents)
	{
		Component->RequestHeightmapUpdate();
	}
	LandscapeEdit.SetHeightData(MinX, MinY, MaxX, MaxY, Data, 0, false);
	LandscapeEdit.Flush();

	for (ULandscapeComponent* Component : ChangedComponents)
	{
		Component->UpdateCachedBounds();
		Component->MarkRenderStateDirty();
	}
	Landscape->MarkPackageDirty();
}

// Helper: writes heightmap data to landscape, handling edit layers properly
static void WriteLandscapeHeightData(ALandscapeProxy* Landscape, ULandscapeInfo* LandInfo,
	int32 MinX, int32 MinY, int32 MaxX, int32 MaxY, uint16* Data)
{
	ALandscape* LandscapeActor = Landscape->GetLandscapeActor();

	// Get edit layer GUID if available
	FGuid LayerGuid;
	bool bHasEditLayers = LandscapeActor && LandscapeActor->HasLayersContent();
	if (bHasEditLayers)
	{
		const TArray<ULandscapeEditLayerBase*> EditLayers = LandscapeActor->GetEditLayers();
		if (EditLayers.Num() > 0 && EditLayers[0])
		{
			LayerGuid = EditLayers[0]->GetGuid();
		}
	}

	// Always use the scoped editing layer + FHeightmapAccessor pattern
	// In UE 5.7, even non-layer landscapes may need this pipeline
	if (LandscapeActor)
	{
		{
			FScopedSetLandscapeEditingLayer Scope(LandscapeActor, LayerGuid, [LandscapeActor]() {
				LandscapeActor->RequestLayersContentUpdate(ELandscapeLayerUpdateMode::Update_Heightmap_All);
			});

			FHeightmapAccessor<false> HeightmapAccessor(LandInfo);
			HeightmapAccessor.SetData(MinX, MinY, MaxX, MaxY, Data);
		}
		LandscapeActor->ForceUpdateLayersContent();
	}
	else
	{
		// Fallback: direct write for landscape proxies without a landscape actor
		FLandscapeEditDataInterface LandscapeEdit(LandInfo);
		TSet<ULandscapeComponent*> ChangedComponents;
		LandscapeEdit.GetComponentsInRegion(MinX, MinY, MaxX, MaxY, &ChangedComponents);
		for (ULandscapeComponent* Component : ChangedComponents)
		{
			Component->RequestHeightmapUpdate();
		}
		LandscapeEdit.SetHeightData(MinX, MinY, MaxX, MaxY, Data, 0, false);
		LandscapeEdit.Flush();
	}

	Landscape->MarkPackageDirty();
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSculptLandscape(const TSharedPtr<FJsonObject>& Params)
{
	ALandscapeProxy* Landscape = FindLandscapeProxy();
	if (!Landscape)
	{
		return MCPError(TEXT("No landscape found. Use create_landscape first."));
	}

	double CenterX, CenterY, Radius, Strength;
	if (!Params->TryGetNumberField(TEXT("center_x"), CenterX) ||
		!Params->TryGetNumberField(TEXT("center_y"), CenterY) ||
		!Params->TryGetNumberField(TEXT("radius"), Radius) ||
		!Params->TryGetNumberField(TEXT("strength"), Strength))
	{
		return MCPError(TEXT("Missing required params: center_x, center_y, radius, strength"));
	}

	FString Operation = TEXT("raise");
	Params->TryGetStringField(TEXT("operation"), Operation);

	FString Shape = TEXT("circle");
	Params->TryGetStringField(TEXT("shape"), Shape);

	ULandscapeInfo* LandInfo = Landscape->GetLandscapeInfo();
	if (!LandInfo)
	{
		return MCPError(TEXT("Landscape has no LandscapeInfo"));
	}

	// Convert world coordinates to landscape local coordinates
	FVector LandscapeScale = Landscape->GetActorScale3D();
	FVector LandscapeLocation = Landscape->GetActorLocation();

	// Determine the affected region in landscape space (vertex coordinates)
	int32 MinX = FMath::FloorToInt32((CenterX - Radius - LandscapeLocation.X) / LandscapeScale.X);
	int32 MaxX = FMath::CeilToInt32((CenterX + Radius - LandscapeLocation.X) / LandscapeScale.X);
	int32 MinY = FMath::FloorToInt32((CenterY - Radius - LandscapeLocation.Y) / LandscapeScale.Y);
	int32 MaxY = FMath::CeilToInt32((CenterY + Radius - LandscapeLocation.Y) / LandscapeScale.Y);

	// Read existing heightmap data
	FLandscapeEditDataInterface LandscapeEdit(LandInfo);
	TArray<uint16> HeightData;
	int32 ReadMinX = MinX, ReadMinY = MinY, ReadMaxX = MaxX, ReadMaxY = MaxY;
	HeightData.SetNumUninitialized((ReadMaxX - ReadMinX + 1) * (ReadMaxY - ReadMinY + 1));
	LandscapeEdit.GetHeightDataFast(ReadMinX, ReadMinY, ReadMaxX, ReadMaxY, HeightData.GetData(), 0);

	int32 DataWidth = ReadMaxX - ReadMinX + 1;
	int32 DataHeight = ReadMaxY - ReadMinY + 1;

	// Flatten target: use midpoint (height 0) as default, which resets terrain to flat
	double FlattenTarget = 32768.0;

	// Apply operation
	int32 ModifiedPixels = 0;
	for (int32 Y = 0; Y < DataHeight; Y++)
	{
		for (int32 X = 0; X < DataWidth; X++)
		{
			double WorldX = (ReadMinX + X) * LandscapeScale.X + LandscapeLocation.X;
			double WorldY = (ReadMinY + Y) * LandscapeScale.Y + LandscapeLocation.Y;

			double Distance;
			if (Shape == TEXT("square"))
			{
				Distance = FMath::Max(FMath::Abs(WorldX - CenterX), FMath::Abs(WorldY - CenterY));
			}
			else
			{
				Distance = FMath::Sqrt(FMath::Square(WorldX - CenterX) + FMath::Square(WorldY - CenterY));
			}

			if (Distance <= Radius)
			{
				float Falloff = 1.0f - (float)(Distance / Radius);
				Falloff = FMath::SmoothStep(0.0f, 1.0f, Falloff);

				int32 Idx = Y * DataWidth + X;
				uint16 OldHeight = HeightData[Idx];
				int32 NewHeight = (int32)OldHeight;

				if (Operation == TEXT("raise"))
				{
					NewHeight = OldHeight + (int32)(Strength * Falloff);
				}
				else if (Operation == TEXT("lower"))
				{
					NewHeight = OldHeight - (int32)(Strength * Falloff);
				}
				else if (Operation == TEXT("flatten"))
				{
					int32 Target = (int32)FlattenTarget;
					NewHeight = FMath::Lerp((int32)OldHeight, Target, Falloff);
				}
				else if (Operation == TEXT("smooth"))
				{
					// Average of neighbors
					int32 Sum = 0;
					int32 Cnt = 0;
					for (int32 DY = -1; DY <= 1; DY++)
					{
						for (int32 DX = -1; DX <= 1; DX++)
						{
							int32 NX = X + DX;
							int32 NY = Y + DY;
							if (NX >= 0 && NX < DataWidth && NY >= 0 && NY < DataHeight)
							{
								Sum += HeightData[NY * DataWidth + NX];
								Cnt++;
							}
						}
					}
					int32 Avg = Sum / Cnt;
					NewHeight = FMath::Lerp((int32)OldHeight, Avg, Falloff * (float)Strength / 1000.0f);
				}

				HeightData[Idx] = (uint16)FMath::Clamp(NewHeight, 0, 65535);
				ModifiedPixels++;
			}
		}
	}

	// Write back using edit-layer-aware helper
	WriteLandscapeHeightData(Landscape, LandInfo, ReadMinX, ReadMinY, ReadMaxX, ReadMaxY, HeightData.GetData());

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("operation"), Operation);
	Result->SetNumberField(TEXT("modified_pixels"), ModifiedPixels);
	Result->SetNumberField(TEXT("region_width"), DataWidth);
	Result->SetNumberField(TEXT("region_height"), DataHeight);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSculptNoise(const TSharedPtr<FJsonObject>& Params)
{
	ALandscapeProxy* Landscape = FindLandscapeProxy();
	if (!Landscape)
	{
		return MCPError(TEXT("No landscape found. Use create_landscape first."));
	}

	double CenterX, CenterY, Radius, Amplitude;
	if (!Params->TryGetNumberField(TEXT("center_x"), CenterX) ||
		!Params->TryGetNumberField(TEXT("center_y"), CenterY) ||
		!Params->TryGetNumberField(TEXT("radius"), Radius) ||
		!Params->TryGetNumberField(TEXT("amplitude"), Amplitude))
	{
		return MCPError(TEXT("Missing required params: center_x, center_y, radius, amplitude"));
	}

	double Frequency = 0.01;
	double Seed = 0.0;
	int32 Octaves = 4;
	double TempVal;
	if (Params->TryGetNumberField(TEXT("frequency"), TempVal)) Frequency = TempVal;
	if (Params->TryGetNumberField(TEXT("seed"), TempVal)) Seed = TempVal;
	if (Params->TryGetNumberField(TEXT("octaves"), TempVal)) Octaves = FMath::Clamp((int32)TempVal, 1, 8);

	ULandscapeInfo* LandInfo = Landscape->GetLandscapeInfo();
	if (!LandInfo)
	{
		return MCPError(TEXT("Landscape has no LandscapeInfo"));
	}

	FVector LandscapeScale = Landscape->GetActorScale3D();
	FVector LandscapeLocation = Landscape->GetActorLocation();

	int32 MinX = FMath::FloorToInt32((CenterX - Radius - LandscapeLocation.X) / LandscapeScale.X);
	int32 MaxX = FMath::CeilToInt32((CenterX + Radius - LandscapeLocation.X) / LandscapeScale.X);
	int32 MinY = FMath::FloorToInt32((CenterY - Radius - LandscapeLocation.Y) / LandscapeScale.Y);
	int32 MaxY = FMath::CeilToInt32((CenterY + Radius - LandscapeLocation.Y) / LandscapeScale.Y);

	FLandscapeEditDataInterface LandscapeEdit(LandInfo);
	TArray<uint16> HeightData;
	int32 ReadMinX = MinX, ReadMinY = MinY, ReadMaxX = MaxX, ReadMaxY = MaxY;
	HeightData.SetNumUninitialized((ReadMaxX - ReadMinX + 1) * (ReadMaxY - ReadMinY + 1));
	LandscapeEdit.GetHeightDataFast(ReadMinX, ReadMinY, ReadMaxX, ReadMaxY, HeightData.GetData(), 0);

	int32 DataWidth = ReadMaxX - ReadMinX + 1;
	int32 DataHeight = ReadMaxY - ReadMinY + 1;

	int32 ModifiedPixels = 0;
	for (int32 Y = 0; Y < DataHeight; Y++)
	{
		for (int32 X = 0; X < DataWidth; X++)
		{
			double WorldX = (ReadMinX + X) * LandscapeScale.X + LandscapeLocation.X;
			double WorldY = (ReadMinY + Y) * LandscapeScale.Y + LandscapeLocation.Y;
			double Distance = FMath::Sqrt(FMath::Square(WorldX - CenterX) + FMath::Square(WorldY - CenterY));

			if (Distance <= Radius)
			{
				float Falloff = 1.0f - (float)(Distance / Radius);
				Falloff = FMath::SmoothStep(0.0f, 1.0f, Falloff);

				// Layered noise
				float NoiseValue = 0.0f;
				float Amp = (float)Amplitude;
				float Freq = (float)Frequency;
				for (int32 Oct = 0; Oct < Octaves; Oct++)
				{
					NoiseValue += Amp * FMath::PerlinNoise2D(FVector2D(
						(WorldX + Seed) * Freq,
						(WorldY + Seed) * Freq
					));
					Amp *= 0.5f;
					Freq *= 2.0f;
				}

				int32 Idx = Y * DataWidth + X;
				int32 NewHeight = (int32)HeightData[Idx] + (int32)(NoiseValue * Falloff);
				HeightData[Idx] = (uint16)FMath::Clamp(NewHeight, 0, 65535);
				ModifiedPixels++;
			}
		}
	}

	// Write back using edit-layer-aware helper
	WriteLandscapeHeightData(Landscape, LandInfo, ReadMinX, ReadMinY, ReadMaxX, ReadMaxY, HeightData.GetData());

	auto Result = MCPSuccess();
	Result->SetNumberField(TEXT("modified_pixels"), ModifiedPixels);
	Result->SetNumberField(TEXT("octaves"), Octaves);
	Result->SetNumberField(TEXT("amplitude"), Amplitude);
	Result->SetNumberField(TEXT("frequency"), Frequency);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSetHeightAt(const TSharedPtr<FJsonObject>& Params)
{
	ALandscapeProxy* Landscape = FindLandscapeProxy();
	if (!Landscape)
	{
		return MCPError(TEXT("No landscape found. Use create_landscape first."));
	}

	double X, Y, Height;
	if (!Params->TryGetNumberField(TEXT("x"), X) ||
		!Params->TryGetNumberField(TEXT("y"), Y) ||
		!Params->TryGetNumberField(TEXT("height"), Height))
	{
		return MCPError(TEXT("Missing required params: x, y, height"));
	}

	ULandscapeInfo* LandInfo = Landscape->GetLandscapeInfo();
	if (!LandInfo)
	{
		return MCPError(TEXT("Landscape has no LandscapeInfo"));
	}

	FVector LandscapeScale = Landscape->GetActorScale3D();
	FVector LandscapeLocation = Landscape->GetActorLocation();

	int32 LX = FMath::RoundToInt32((X - LandscapeLocation.X) / LandscapeScale.X);
	int32 LY = FMath::RoundToInt32((Y - LandscapeLocation.Y) / LandscapeScale.Y);

	// Convert height to uint16 (height is in world units, divide by Z scale, add 32768)
	// UE uses LANDSCAPE_ZSCALE = 1/128 but with edit layers the effective factor is 256
	uint16 HeightVal = (uint16)FMath::Clamp((int32)(Height / LandscapeScale.Z * 256.0f + 32768.0f), 0, 65535);

	WriteLandscapeHeightData(Landscape, LandInfo, LX, LY, LX, LY, &HeightVal);

	auto Result = MCPSuccess();
	Result->SetNumberField(TEXT("x"), X);
	Result->SetNumberField(TEXT("y"), Y);
	Result->SetNumberField(TEXT("height"), Height);
	Result->SetNumberField(TEXT("raw_value"), HeightVal);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleGetHeightAt(const TSharedPtr<FJsonObject>& Params)
{
	ALandscapeProxy* Landscape = FindLandscapeProxy();
	if (!Landscape)
	{
		return MCPError(TEXT("No landscape found. Use create_landscape first."));
	}

	double X, Y;
	if (!Params->TryGetNumberField(TEXT("x"), X) ||
		!Params->TryGetNumberField(TEXT("y"), Y))
	{
		return MCPError(TEXT("Missing required params: x, y"));
	}

	UWorld* World = GetEditorWorld();
	if (!World) return MCPError(TEXT("No editor world available"));

	// Use line trace to get accurate height
	FVector TraceStart(X, Y, 100000.0);
	FVector TraceEnd(X, Y, -100000.0);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;

	bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams);

	if (!bHit)
	{
		return MCPError(FString::Printf(TEXT("No landscape surface found at position (%f, %f)"), X, Y));
	}

	auto Result = MCPSuccess();
	Result->SetNumberField(TEXT("x"), X);
	Result->SetNumberField(TEXT("y"), Y);
	Result->SetNumberField(TEXT("height"), HitResult.ImpactPoint.Z);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleImportHeightmap(const TSharedPtr<FJsonObject>& Params)
{
	ALandscapeProxy* Landscape = FindLandscapeProxy();
	if (!Landscape)
	{
		return MCPError(TEXT("No landscape found. Use create_landscape first."));
	}

	FString FilePath;
	if (!Params->TryGetStringField(TEXT("file_path"), FilePath))
	{
		return MCPError(TEXT("Missing required param: file_path"));
	}

	double Width, Height;
	if (!Params->TryGetNumberField(TEXT("width"), Width) ||
		!Params->TryGetNumberField(TEXT("height"), Height))
	{
		return MCPError(TEXT("Missing required params: width, height"));
	}

	int32 DataWidth = (int32)Width;
	int32 DataHeight = (int32)Height;

	ULandscapeInfo* LandInfo = Landscape->GetLandscapeInfo();
	if (!LandInfo)
	{
		return MCPError(TEXT("Landscape has no LandscapeInfo"));
	}

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		return MCPError(FString::Printf(TEXT("Failed to load file: %s"), *FilePath));
	}

	TArray<uint16> HeightData;

	// Detect format by extension and size
	if (FilePath.EndsWith(TEXT(".r16")) || FilePath.EndsWith(TEXT(".raw")))
	{
		// Raw uint16 data
		if (FileData.Num() != DataWidth * DataHeight * 2)
		{
			return MCPError(FString::Printf(TEXT("File size mismatch. Expected %d bytes for %dx%d r16, got %d"),
				DataWidth * DataHeight * 2, DataWidth, DataHeight, FileData.Num()));
		}
		HeightData.SetNumUninitialized(DataWidth * DataHeight);
		FMemory::Memcpy(HeightData.GetData(), FileData.GetData(), FileData.Num());
	}
	else
	{
		return MCPError(TEXT("Unsupported heightmap format. Use .r16 or .raw (uint16 raw data)."));
	}

	// Write to landscape
	FLandscapeEditDataInterface LandscapeEdit(LandInfo);
	int32 MinX = 0, MinY = 0;
	int32 MaxX = DataWidth - 1;
	int32 MaxY = DataHeight - 1;
	LandscapeEdit.SetHeightData(MinX, MinY, MaxX, MaxY, HeightData.GetData(), 0, true);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("file"), FilePath);
	Result->SetNumberField(TEXT("width"), DataWidth);
	Result->SetNumberField(TEXT("height"), DataHeight);
	Result->SetNumberField(TEXT("pixels_imported"), DataWidth * DataHeight);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandlePaintLayer(const TSharedPtr<FJsonObject>& Params)
{
	ALandscapeProxy* Landscape = FindLandscapeProxy();
	if (!Landscape)
	{
		return MCPError(TEXT("No landscape found. Use create_landscape first."));
	}

	FString LayerName;
	double CenterX, CenterY, Radius, StrengthVal;
	if (!Params->TryGetStringField(TEXT("layer_name"), LayerName) ||
		!Params->TryGetNumberField(TEXT("center_x"), CenterX) ||
		!Params->TryGetNumberField(TEXT("center_y"), CenterY) ||
		!Params->TryGetNumberField(TEXT("radius"), Radius) ||
		!Params->TryGetNumberField(TEXT("strength"), StrengthVal))
	{
		return MCPError(TEXT("Missing required params: layer_name, center_x, center_y, radius, strength"));
	}

	double Falloff = 0.5;
	Params->TryGetNumberField(TEXT("falloff"), Falloff);

	ULandscapeInfo* LandInfo = Landscape->GetLandscapeInfo();
	if (!LandInfo)
	{
		return MCPError(TEXT("Landscape has no LandscapeInfo"));
	}

	// Find the layer
	ULandscapeLayerInfoObject* LayerInfo = nullptr;
	for (const FLandscapeInfoLayerSettings& LayerSettings : LandInfo->Layers)
	{
		if (LayerSettings.LayerName.ToString() == LayerName)
		{
			LayerInfo = LayerSettings.LayerInfoObj;
			break;
		}
	}

	if (!LayerInfo)
	{
		return MCPError(FString::Printf(TEXT("Layer '%s' not found. Use add_layer to create it first. Existing layers: %s"),
			*LayerName, *FString::JoinBy(LandInfo->Layers, TEXT(", "), [](const FLandscapeInfoLayerSettings& L) { return L.LayerName.ToString(); })));
	}

	FVector LandscapeScale = Landscape->GetActorScale3D();
	FVector LandscapeLocation = Landscape->GetActorLocation();

	int32 MinX = FMath::FloorToInt32((CenterX - Radius - LandscapeLocation.X) / LandscapeScale.X);
	int32 MaxX = FMath::CeilToInt32((CenterX + Radius - LandscapeLocation.X) / LandscapeScale.X);
	int32 MinY = FMath::FloorToInt32((CenterY - Radius - LandscapeLocation.Y) / LandscapeScale.Y);
	int32 MaxY = FMath::CeilToInt32((CenterY + Radius - LandscapeLocation.Y) / LandscapeScale.Y);

	int32 DataWidth = MaxX - MinX + 1;
	int32 DataHeight = MaxY - MinY + 1;

	// Read existing weight data
	FLandscapeEditDataInterface LandscapeEdit(LandInfo);
	TArray<uint8> WeightData;
	WeightData.SetNumZeroed(DataWidth * DataHeight);

	int32 ReadMinX = MinX, ReadMinY = MinY, ReadMaxX = MaxX, ReadMaxY = MaxY;
	LandscapeEdit.GetWeightDataFast(LayerInfo, ReadMinX, ReadMinY, ReadMaxX, ReadMaxY, WeightData.GetData(), 0);

	// Apply paint
	int32 ModifiedPixels = 0;
	for (int32 Y = 0; Y < DataHeight; Y++)
	{
		for (int32 X = 0; X < DataWidth; X++)
		{
			double WorldX = (MinX + X) * LandscapeScale.X + LandscapeLocation.X;
			double WorldY = (MinY + Y) * LandscapeScale.Y + LandscapeLocation.Y;
			double Distance = FMath::Sqrt(FMath::Square(WorldX - CenterX) + FMath::Square(WorldY - CenterY));

			if (Distance <= Radius)
			{
				float FalloffFactor = 1.0f;
				float FalloffStart = (float)(Radius * (1.0 - Falloff));
				if (Distance > FalloffStart)
				{
					FalloffFactor = 1.0f - (float)((Distance - FalloffStart) / (Radius - FalloffStart));
					FalloffFactor = FMath::SmoothStep(0.0f, 1.0f, FalloffFactor);
				}

				int32 Idx = Y * DataWidth + X;
				int32 NewWeight = (int32)WeightData[Idx] + (int32)(StrengthVal * 255.0 * FalloffFactor);
				WeightData[Idx] = (uint8)FMath::Clamp(NewWeight, 0, 255);
				ModifiedPixels++;
			}
		}
	}

	LandscapeEdit.SetAlphaData(LayerInfo, MinX, MinY, MaxX, MaxY, WeightData.GetData(), 0);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("layer"), LayerName);
	Result->SetNumberField(TEXT("modified_pixels"), ModifiedPixels);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddLayer(const TSharedPtr<FJsonObject>& Params)
{
	ALandscapeProxy* Landscape = FindLandscapeProxy();
	if (!Landscape)
	{
		return MCPError(TEXT("No landscape found. Use create_landscape first."));
	}

	FString LayerName;
	if (!Params->TryGetStringField(TEXT("layer_name"), LayerName))
	{
		return MCPError(TEXT("Missing required param: layer_name"));
	}

	ULandscapeInfo* LandInfo = Landscape->GetLandscapeInfo();
	if (!LandInfo)
	{
		return MCPError(TEXT("Landscape has no LandscapeInfo"));
	}

	// Check if layer already exists
	int32 ExistingIndex = INDEX_NONE;
	for (int32 i = 0; i < LandInfo->Layers.Num(); i++)
	{
		if (LandInfo->Layers[i].LayerName.ToString() == LayerName)
		{
			if (LandInfo->Layers[i].LayerInfoObj)
			{
				return MCPError(FString::Printf(TEXT("Layer '%s' already exists"), *LayerName));
			}
			// Layer exists by name but has no LayerInfoObj - we'll create one
			ExistingIndex = i;
			break;
		}
	}

	// Try to load existing layer info or create new one
	ULandscapeLayerInfoObject* LayerInfo = nullptr;
	FString LayerInfoPath;
	if (Params->TryGetStringField(TEXT("layer_info_path"), LayerInfoPath))
	{
		LayerInfo = LoadObject<ULandscapeLayerInfoObject>(nullptr, *LayerInfoPath);
		if (!LayerInfo)
		{
			return MCPError(FString::Printf(TEXT("Could not load layer info: %s"), *LayerInfoPath));
		}
	}
	else
	{
		// Create a new layer info object
		FString PackageName = FString::Printf(TEXT("/Game/Landscape/LayerInfo_%s"), *LayerName);
		UPackage* Package = CreatePackage(*PackageName);
		LayerInfo = NewObject<ULandscapeLayerInfoObject>(Package, FName(*LayerName), RF_Public | RF_Standalone);
		LayerInfo->SetLayerName(FName(*LayerName), true);
		LayerInfo->MarkPackageDirty();
	}

	// Add or update layer in landscape info
	int32 LayerIndex;
	if (ExistingIndex != INDEX_NONE)
	{
		LandInfo->Layers[ExistingIndex].LayerInfoObj = LayerInfo;
		LayerIndex = ExistingIndex;
	}
	else
	{
		LayerIndex = LandInfo->Layers.Num();
		LandInfo->Layers.Add(FLandscapeInfoLayerSettings(LayerInfo, Landscape));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("layer_name"), LayerName);
	Result->SetNumberField(TEXT("layer_index"), LayerIndex);
	if (LayerInfo)
	{
		Result->SetStringField(TEXT("layer_info_path"), LayerInfo->GetPathName());
	}
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandlePlaceFoliage(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World) return MCPError(TEXT("No editor world available"));

	FString MeshPath;
	double CenterX, CenterY, Radius;
	double Count;
	if (!Params->TryGetStringField(TEXT("mesh_path"), MeshPath) ||
		!Params->TryGetNumberField(TEXT("center_x"), CenterX) ||
		!Params->TryGetNumberField(TEXT("center_y"), CenterY) ||
		!Params->TryGetNumberField(TEXT("radius"), Radius) ||
		!Params->TryGetNumberField(TEXT("count"), Count))
	{
		return MCPError(TEXT("Missing required params: mesh_path, center_x, center_y, radius, count"));
	}

	double MinScale = 1.0, MaxScale = 1.0;
	bool bAlignToSurface = true;
	Params->TryGetNumberField(TEXT("min_scale"), MinScale);
	Params->TryGetNumberField(TEXT("max_scale"), MaxScale);
	Params->TryGetBoolField(TEXT("align_to_surface"), bAlignToSurface);

	// Load the static mesh
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
	if (!Mesh)
	{
		return MCPError(FString::Printf(TEXT("Could not load static mesh: %s"), *MeshPath));
	}

	// Get or create foliage actor
	AInstancedFoliageActor* FoliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(World);
	if (!FoliageActor)
	{
		return MCPError(TEXT("Could not get or create foliage actor for current level"));
	}

	// Create foliage type from mesh
	UFoliageType_InstancedStaticMesh* FoliageType = NewObject<UFoliageType_InstancedStaticMesh>(FoliageActor);
	FoliageType->SetStaticMesh(Mesh);

	// Add foliage type to foliage actor - find existing or add new
	FFoliageInfo* FoliageInfo = nullptr;

	// Check if this mesh type already exists
	FoliageActor->ForEachFoliageInfo([&](UFoliageType* Type, FFoliageInfo& Info) -> bool
	{
		UFoliageType_InstancedStaticMesh* FTISM = Cast<UFoliageType_InstancedStaticMesh>(Type);
		if (FTISM && FTISM->GetStaticMesh() == Mesh)
		{
			FoliageInfo = &Info;
			return false; // stop iteration
		}
		return true; // continue
	});

	if (!FoliageInfo)
	{
		FFoliageInfo* OutInfo = nullptr;
		FoliageActor->AddFoliageType(FoliageType, &OutInfo);
		FoliageInfo = OutInfo;
		if (!FoliageInfo)
		{
			return MCPError(TEXT("Failed to add foliage type to foliage actor"));
		}
	}

	// Place instances
	int32 PlacedCount = 0;
	FRandomStream RandStream(FMath::Rand());

	for (int32 i = 0; i < (int32)Count; i++)
	{
		// Random position within radius
		float Angle = RandStream.FRand() * 2.0f * PI;
		float Dist = FMath::Sqrt(RandStream.FRand()) * (float)Radius;
		float PosX = (float)CenterX + FMath::Cos(Angle) * Dist;
		float PosY = (float)CenterY + FMath::Sin(Angle) * Dist;

		// Line trace to find surface height
		FVector TraceStart(PosX, PosY, 100000.0f);
		FVector TraceEnd(PosX, PosY, -100000.0f);
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = true;

		if (!World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
		{
			continue;
		}

		float Scale = FMath::Lerp((float)MinScale, (float)MaxScale, RandStream.FRand());
		float RandomYaw = RandStream.FRand() * 360.0f;

		FFoliageInstance Instance;
		Instance.Location = HitResult.ImpactPoint;
		Instance.Rotation = FRotator(0.0f, RandomYaw, 0.0f);

		if (bAlignToSurface)
		{
			FVector Normal = HitResult.ImpactNormal;
			FRotator AlignRotation = Normal.Rotation();
			Instance.Rotation = FRotator(AlignRotation.Pitch - 90.0f, RandomYaw, 0.0f);
		}

		Instance.DrawScale3D = FVector3f(Scale, Scale, Scale);

		FoliageInfo->AddInstance(FoliageType, Instance);
		PlacedCount++;
	}

	FoliageInfo->Refresh(true, false);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("mesh"), MeshPath);
	Result->SetNumberField(TEXT("placed_count"), PlacedCount);
	Result->SetNumberField(TEXT("requested_count"), (int32)Count);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleClearFoliage(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World) return MCPError(TEXT("No editor world available"));

	double CenterX, CenterY, Radius;
	if (!Params->TryGetNumberField(TEXT("center_x"), CenterX) ||
		!Params->TryGetNumberField(TEXT("center_y"), CenterY) ||
		!Params->TryGetNumberField(TEXT("radius"), Radius))
	{
		return MCPError(TEXT("Missing required params: center_x, center_y, radius"));
	}

	FString MeshPath;
	Params->TryGetStringField(TEXT("mesh_path"), MeshPath);

	UStaticMesh* FilterMesh = nullptr;
	if (!MeshPath.IsEmpty())
	{
		FilterMesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
	}

	AInstancedFoliageActor* FoliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForCurrentLevel(World);
	if (!FoliageActor)
	{
		return MCPError(TEXT("No foliage actor found in current level"));
	}

	double RadiusSq = Radius * Radius;
	FVector Center(CenterX, CenterY, 0.0);
	int32 RemovedCount = 0;

	FoliageActor->ForEachFoliageInfo([&](UFoliageType* Type, FFoliageInfo& Info) -> bool
	{
		UFoliageType_InstancedStaticMesh* FTISM = Cast<UFoliageType_InstancedStaticMesh>(Type);
		if (FilterMesh && FTISM && FTISM->GetStaticMesh() != FilterMesh)
		{
			return true; // continue
		}

		TArray<int32> IndicesToRemove;

		for (int32 Idx = 0; Idx < Info.Instances.Num(); Idx++)
		{
			const FFoliageInstance& Instance = Info.Instances[Idx];
			FVector InstancePos2D(Instance.Location.X, Instance.Location.Y, 0.0);
			FVector Center2D(CenterX, CenterY, 0.0);

			if (FVector::DistSquared(InstancePos2D, Center2D) <= RadiusSq)
			{
				IndicesToRemove.Add(Idx);
			}
		}

		// Remove in reverse order to preserve indices
		for (int32 i = IndicesToRemove.Num() - 1; i >= 0; i--)
		{
			Info.RemoveInstances(TArrayView<const int32>(&IndicesToRemove[i], 1), true);
			RemovedCount++;
		}

		if (IndicesToRemove.Num() > 0)
		{
			Info.Refresh(true, false);
		}

		return true; // continue
	});

	auto Result = MCPSuccess();
	Result->SetNumberField(TEXT("removed_count"), RemovedCount);
	if (!MeshPath.IsEmpty())
	{
		Result->SetStringField(TEXT("mesh_filter"), MeshPath);
	}
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleScreenshot(const TSharedPtr<FJsonObject>& Params)
{
	// Get the active level editor viewport
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	TSharedPtr<SLevelViewport> ActiveViewport = LevelEditorModule.GetFirstActiveLevelViewport();

	if (!ActiveViewport.IsValid())
	{
		return MCPError(TEXT("No active level editor viewport found"));
	}

	// Use Slate's TakeScreenshot to capture the already-rendered viewport widget
	TSharedPtr<SWidget> ViewportWidget = ActiveViewport->GetViewportWidget().Pin();
	if (!ViewportWidget.IsValid())
	{
		return MCPError(TEXT("Could not get viewport widget"));
	}

	TArray<FColor> Bitmap;
	FIntVector OutSize;
	bool bSuccess = FSlateApplication::Get().TakeScreenshot(ViewportWidget.ToSharedRef(), Bitmap, OutSize);

	if (!bSuccess || Bitmap.Num() == 0)
	{
		return MCPError(TEXT("Failed to capture viewport screenshot via Slate"));
	}

	int32 Width = OutSize.X;
	int32 Height = OutSize.Y;

	// Determine output filename
	FString Filename = Params->GetStringField(TEXT("filename"));
	if (Filename.IsEmpty())
	{
		Filename = FString::Printf(TEXT("mcp_screenshot_%s.png"),
			*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	}

	if (!Filename.EndsWith(TEXT(".png"), ESearchCase::IgnoreCase))
	{
		Filename += TEXT(".png");
	}

	// Save to Saved/UnrealMCP/Screenshots/
	FString OutputDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("UnrealMCP"), TEXT("Screenshots"));
	IFileManager::Get().MakeDirectory(*OutputDir, true);
	FString FullPath = FPaths::Combine(OutputDir, Filename);

	// Compress to PNG
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (!ImageWrapper.IsValid())
	{
		return MCPError(TEXT("Failed to create image wrapper"));
	}

	if (!ImageWrapper->SetRaw(Bitmap.GetData(), Bitmap.Num() * sizeof(FColor),
		Width, Height, ERGBFormat::BGRA, 8))
	{
		return MCPError(TEXT("Failed to set raw image data"));
	}

	const TArray64<uint8>& PNGData = ImageWrapper->GetCompressed();
	if (!FFileHelper::SaveArrayToFile(PNGData, *FullPath))
	{
		return MCPError(TEXT("Failed to save screenshot to disk"));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("path"), FullPath);
	Result->SetNumberField(TEXT("width"), Width);
	Result->SetNumberField(TEXT("height"), Height);
	return Result;
}

// ============================================================================
// Playtest Handlers
// ============================================================================

static UWorld* GetPIEWorld()
{
	if (GEditor && GEditor->PlayWorld)
	{
		return GEditor->PlayWorld;
	}
	return nullptr;
}

void FMCPTcpServer::RegisterPlaytestHandlers()
{
	RegisterHandler(TEXT("start_pie"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleStartPIE(Params); });
	RegisterHandler(TEXT("stop_pie"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleStopPIE(Params); });
	RegisterHandler(TEXT("get_pie_status"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleGetPIEStatus(Params); });
	RegisterHandler(TEXT("key_input"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleKeyInput(Params); });
	RegisterHandler(TEXT("mouse_move"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleMouseMove(Params); });
	RegisterHandler(TEXT("mouse_click"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleMouseClick(Params); });
	RegisterHandler(TEXT("get_game_state"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleGetGameState(Params); });
	RegisterHandler(TEXT("call_component_function"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleCallComponentFunction(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleStartPIE(const TSharedPtr<FJsonObject>& Params)
{
	if (!GEditor)
	{
		return MCPError(TEXT("No editor available"));
	}

	if (GEditor->PlayWorld)
	{
		return MCPError(TEXT("PIE is already running"));
	}

	FRequestPlaySessionParams PlayParams;
	PlayParams.WorldType = EPlaySessionWorldType::PlayInEditor;

	// Use the first active viewport
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	TSharedPtr<SLevelViewport> ActiveViewport = LevelEditorModule.GetFirstActiveLevelViewport();
	if (ActiveViewport.IsValid())
	{
		PlayParams.DestinationSlateViewport = ActiveViewport;
	}

	GEditor->RequestPlaySession(PlayParams);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("status"), TEXT("play_session_requested"));
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleStopPIE(const TSharedPtr<FJsonObject>& Params)
{
	if (!GEditor || !GEditor->PlayWorld)
	{
		return MCPError(TEXT("PIE is not running"));
	}

	GEditor->RequestEndPlayMap();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("status"), TEXT("stop_requested"));
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleGetPIEStatus(const TSharedPtr<FJsonObject>& Params)
{
	auto Result = MCPSuccess();
	bool bIsPlaying = GEditor && GEditor->PlayWorld != nullptr;
	Result->SetBoolField(TEXT("is_playing"), bIsPlaying);

	if (bIsPlaying)
	{
		Result->SetNumberField(TEXT("play_time"), GEditor->PlayWorld->GetTimeSeconds());

		APlayerController* PC = GEditor->PlayWorld->GetFirstPlayerController();
		if (PC && PC->GetCharacter())
		{
			FVector Loc = PC->GetCharacter()->GetActorLocation();
			Result->SetNumberField(TEXT("player_x"), Loc.X);
			Result->SetNumberField(TEXT("player_y"), Loc.Y);
			Result->SetNumberField(TEXT("player_z"), Loc.Z);
		}
	}

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleKeyInput(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* PIEWorld = GetPIEWorld();
	if (!PIEWorld)
	{
		return MCPError(TEXT("PIE is not running"));
	}

	FString KeyName = Params->GetStringField(TEXT("key"));
	FString Action = Params->GetStringField(TEXT("action"));
	double HoldDurationMs = Params->GetNumberField(TEXT("hold_duration_ms"));

	if (KeyName.IsEmpty())
	{
		return MCPError(TEXT("Missing 'key' parameter"));
	}

	if (Action.IsEmpty())
	{
		Action = TEXT("tap");
	}

	// Map friendly key names to FKey
	FKey Key = FKey(*KeyName);
	if (!Key.IsValid())
	{
		return MCPError(FString::Printf(TEXT("Invalid key: %s"), *KeyName));
	}

	APlayerController* PC = PIEWorld->GetFirstPlayerController();
	if (!PC)
	{
		return MCPError(TEXT("No player controller in PIE"));
	}

	if (Action == TEXT("press") || Action == TEXT("tap"))
	{
		PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Pressed, 1.0f));
	}

	if (Action == TEXT("release"))
	{
		PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0.0f));
	}

	// For "tap", schedule the release after a short delay
	if (Action == TEXT("tap"))
	{
		float Delay = HoldDurationMs > 0 ? HoldDurationMs / 1000.0f : 0.1f;
		FTimerHandle Handle;
		PIEWorld->GetTimerManager().SetTimer(Handle, [PC, Key]()
		{
			if (PC && PC->IsValidLowLevel())
			{
				PC->InputKey(FInputKeyEventArgs::CreateSimulated(Key, IE_Released, 0.0f));
			}
		}, Delay, false);
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("key"), KeyName);
	Result->SetStringField(TEXT("action"), Action);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleMouseMove(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* PIEWorld = GetPIEWorld();
	if (!PIEWorld)
	{
		return MCPError(TEXT("PIE is not running"));
	}

	double DeltaX = Params->GetNumberField(TEXT("delta_x"));
	double DeltaY = Params->GetNumberField(TEXT("delta_y"));

	APlayerController* PC = PIEWorld->GetFirstPlayerController();
	if (!PC)
	{
		return MCPError(TEXT("No player controller in PIE"));
	}

	PC->AddYawInput(DeltaX);
	PC->AddPitchInput(DeltaY);

	auto Result = MCPSuccess();
	Result->SetNumberField(TEXT("delta_x"), DeltaX);
	Result->SetNumberField(TEXT("delta_y"), DeltaY);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleMouseClick(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* PIEWorld = GetPIEWorld();
	if (!PIEWorld)
	{
		return MCPError(TEXT("PIE is not running"));
	}

	FString Button = Params->GetStringField(TEXT("button"));
	FString Action = Params->GetStringField(TEXT("action"));

	if (Button.IsEmpty()) Button = TEXT("left");
	if (Action.IsEmpty()) Action = TEXT("click");

	FKey MouseKey;
	if (Button == TEXT("left")) MouseKey = EKeys::LeftMouseButton;
	else if (Button == TEXT("right")) MouseKey = EKeys::RightMouseButton;
	else if (Button == TEXT("middle")) MouseKey = EKeys::MiddleMouseButton;
	else return MCPError(FString::Printf(TEXT("Invalid button: %s"), *Button));

	APlayerController* PC = PIEWorld->GetFirstPlayerController();
	if (!PC)
	{
		return MCPError(TEXT("No player controller in PIE"));
	}

	if (Action == TEXT("press") || Action == TEXT("click"))
	{
		PC->InputKey(FInputKeyEventArgs::CreateSimulated(MouseKey, IE_Pressed, 1.0f));
	}
	if (Action == TEXT("release") || Action == TEXT("click"))
	{
		if (Action == TEXT("click"))
		{
			// Small delay before release for a click
			FTimerHandle Handle;
			PIEWorld->GetTimerManager().SetTimer(Handle, [PC, MouseKey]()
			{
				if (PC && PC->IsValidLowLevel())
				{
					PC->InputKey(FInputKeyEventArgs::CreateSimulated(MouseKey, IE_Released, 0.0f));
				}
			}, 0.05f, false);
		}
		else
		{
			PC->InputKey(FInputKeyEventArgs::CreateSimulated(MouseKey, IE_Released, 0.0f));
		}
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("button"), Button);
	Result->SetStringField(TEXT("action"), Action);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleGetGameState(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* PIEWorld = GetPIEWorld();
	if (!PIEWorld)
	{
		return MCPError(TEXT("PIE is not running"));
	}

	auto Result = MCPSuccess();
	Result->SetNumberField(TEXT("play_time"), PIEWorld->GetTimeSeconds());

	// Player state
	APlayerController* PC = PIEWorld->GetFirstPlayerController();
	if (PC)
	{
		auto PlayerObj = MakeShared<FJsonObject>();

		ACharacter* PlayerChar = PC->GetCharacter();
		if (PlayerChar)
		{
			FVector Loc = PlayerChar->GetActorLocation();
			FRotator Rot = PlayerChar->GetActorRotation();
			FVector Vel = PlayerChar->GetVelocity();

			PlayerObj->SetNumberField(TEXT("x"), Loc.X);
			PlayerObj->SetNumberField(TEXT("y"), Loc.Y);
			PlayerObj->SetNumberField(TEXT("z"), Loc.Z);
			PlayerObj->SetNumberField(TEXT("yaw"), Rot.Yaw);
			PlayerObj->SetNumberField(TEXT("pitch"), Rot.Pitch);
			PlayerObj->SetNumberField(TEXT("speed"), Vel.Size());
			PlayerObj->SetBoolField(TEXT("is_moving"), Vel.Size() > 1.0);
		}

		Result->SetObjectField(TEXT("player"), PlayerObj);
	}

	// Nearby actors within observation radius
	ACharacter* PlayerChar = PC ? PC->GetCharacter() : nullptr;
	if (PlayerChar)
	{
		FVector PlayerLoc = PlayerChar->GetActorLocation();
		float Radius = 3000.0f; // Default observation radius

		TArray<TSharedPtr<FJsonValue>> ActorArray;
		for (TActorIterator<AActor> It(PIEWorld); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor == PlayerChar) continue;
			if (Actor->IsHidden()) continue;

			float Dist = FVector::Dist(PlayerLoc, Actor->GetActorLocation());
			if (Dist > Radius) continue;

			// Skip uninteresting actors (brushes, volumes, etc.)
			FString ClassName = Actor->GetClass()->GetName();
			if (ClassName.Contains(TEXT("Brush")) || ClassName.Contains(TEXT("Volume")))
			{
				continue;
			}

			auto ActorObj = MakeShared<FJsonObject>();
			ActorObj->SetStringField(TEXT("name"), Actor->GetActorLabel().IsEmpty() ? Actor->GetName() : Actor->GetActorLabel());
			ActorObj->SetStringField(TEXT("class"), ClassName);
			ActorObj->SetNumberField(TEXT("distance"), Dist);

			FVector ActorLoc = Actor->GetActorLocation();
			ActorObj->SetNumberField(TEXT("x"), ActorLoc.X);
			ActorObj->SetNumberField(TEXT("y"), ActorLoc.Y);
			ActorObj->SetNumberField(TEXT("z"), ActorLoc.Z);

			ActorArray.Add(MakeShared<FJsonValueObject>(ActorObj));

			if (ActorArray.Num() >= 30) break; // Cap results
		}

		Result->SetArrayField(TEXT("nearby_actors"), ActorArray);
		Result->SetNumberField(TEXT("nearby_actor_count"), ActorArray.Num());
	}

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleCallComponentFunction(const TSharedPtr<FJsonObject>& Params)
{
	FString ActorName;
	if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
	{
		return MCPError(TEXT("Missing required param: actor_name"));
	}

	FString ComponentClass;
	if (!Params->TryGetStringField(TEXT("component_class"), ComponentClass))
	{
		return MCPError(TEXT("Missing required param: component_class"));
	}

	FString FunctionName;
	if (!Params->TryGetStringField(TEXT("function_name"), FunctionName))
	{
		return MCPError(TEXT("Missing required param: function_name"));
	}

	// Check both editor world and PIE world
	UWorld* World = GetPIEWorld();
	if (!World)
	{
		World = GetEditorWorld();
	}
	if (!World)
	{
		return MCPError(TEXT("No world available"));
	}

	// Find the actor
	AActor* TargetActor = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->GetActorLabel() == ActorName || It->GetName() == ActorName)
		{
			TargetActor = *It;
			break;
		}
	}

	if (!TargetActor)
	{
		return MCPError(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
	}

	// Find the component by class name
	UActorComponent* FoundComponent = nullptr;
	for (UActorComponent* Comp : TargetActor->GetComponents())
	{
		if (Comp->GetClass()->GetName() == ComponentClass || Comp->GetClass()->GetName().Contains(ComponentClass))
		{
			FoundComponent = Comp;
			break;
		}
	}

	if (!FoundComponent)
	{
		return MCPError(FString::Printf(TEXT("Component '%s' not found on actor '%s'"), *ComponentClass, *ActorName));
	}

	// Find the function
	UFunction* Func = FoundComponent->GetClass()->FindFunctionByName(*FunctionName);
	if (!Func)
	{
		return MCPError(FString::Printf(TEXT("Function '%s' not found on component '%s'"), *FunctionName, *ComponentClass));
	}

	// Allocate parameter buffer and fill from JSON args
	uint8* FuncParamBuffer = (uint8*)FMemory_Alloca(Func->ParmsSize);
	FMemory::Memzero(FuncParamBuffer, Func->ParmsSize);

	const TSharedPtr<FJsonObject>* ArgsObj = nullptr;
	Params->TryGetObjectField(TEXT("args"), ArgsObj);

	// Fill in parameters from the args object
	for (TFieldIterator<FProperty> PropIt(Func); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
	{
		FProperty* Prop = *PropIt;
		if (Prop->HasAnyPropertyFlags(CPF_ReturnParm))
		{
			continue;
		}

		if (ArgsObj && (*ArgsObj).IsValid())
		{
			const TSharedPtr<FJsonValue>* JsonVal = (*ArgsObj)->Values.Find(Prop->GetName());
			if (JsonVal && JsonVal->IsValid())
			{
				if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
				{
					StrProp->SetPropertyValue_InContainer(FuncParamBuffer, (*JsonVal)->AsString());
				}
				else if (FIntProperty* IntProp = CastField<FIntProperty>(Prop))
				{
					IntProp->SetPropertyValue_InContainer(FuncParamBuffer, (int32)(*JsonVal)->AsNumber());
				}
				else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
				{
					FloatProp->SetPropertyValue_InContainer(FuncParamBuffer, (float)(*JsonVal)->AsNumber());
				}
				else if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
				{
					DoubleProp->SetPropertyValue_InContainer(FuncParamBuffer, (*JsonVal)->AsNumber());
				}
				else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
				{
					BoolProp->SetPropertyValue_InContainer(FuncParamBuffer, (*JsonVal)->AsBool());
				}
				else if (FNameProperty* NameProp = CastField<FNameProperty>(Prop))
				{
					NameProp->SetPropertyValue_InContainer(FuncParamBuffer, FName(*(*JsonVal)->AsString()));
				}
			}
		}
	}

	// Call the function
	FoundComponent->ProcessEvent(Func, FuncParamBuffer);

	// Extract return value if any
	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("actor"), ActorName);
	Result->SetStringField(TEXT("component"), FoundComponent->GetClass()->GetName());
	Result->SetStringField(TEXT("function"), FunctionName);

	for (TFieldIterator<FProperty> PropIt(Func); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
	{
		FProperty* Prop = *PropIt;
		if (Prop->HasAnyPropertyFlags(CPF_ReturnParm) || Prop->HasAnyPropertyFlags(CPF_OutParm))
		{
			FString ValueStr;
			Prop->ExportTextItem_Direct(ValueStr, Prop->ContainerPtrToValuePtr<void>(FuncParamBuffer), nullptr, nullptr, PPF_None);
			Result->SetStringField(FString::Printf(TEXT("out_%s"), *Prop->GetName()), ValueStr);
		}
	}

	// Destroy parameter buffer properties
	for (TFieldIterator<FProperty> PropIt(Func); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
	{
		(*PropIt)->DestroyValue_InContainer(FuncParamBuffer);
	}

	return Result;
}

// ============================================================
// Material Instance handlers
// ============================================================

void FMCPTcpServer::RegisterMaterialHandlers()
{
	RegisterHandler(TEXT("get_material_params"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleGetMaterialParams(Params); });
	RegisterHandler(TEXT("set_material_param"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSetMaterialParam(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleGetMaterialParams(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return MCPError(TEXT("Missing required param: asset_path"));
	}

	UMaterialInstanceConstant* MI = LoadObject<UMaterialInstanceConstant>(nullptr, *AssetPath);
	if (!MI)
	{
		return MCPError(FString::Printf(TEXT("Material instance not found: %s"), *AssetPath));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("name"), MI->GetName());
	Result->SetStringField(TEXT("path"), MI->GetPathName());

	// Parent material
	if (MI->Parent)
	{
		Result->SetStringField(TEXT("parent"), MI->Parent->GetPathName());
	}

	// Scalar parameters
	TArray<TSharedPtr<FJsonValue>> ScalarArray;
	TArray<FMaterialParameterInfo> ScalarInfos;
	TArray<FGuid> ScalarGuids;
	MI->GetAllScalarParameterInfo(ScalarInfos, ScalarGuids);
	for (const FMaterialParameterInfo& Info : ScalarInfos)
	{
		float Value = 0.f;
		MI->GetScalarParameterValue(Info, Value);

		// Check if overridden in this instance
		bool bOverridden = false;
		for (const auto& Param : MI->ScalarParameterValues)
		{
			if (Param.ParameterInfo.Name == Info.Name)
			{
				bOverridden = true;
				break;
			}
		}

		auto ParamObj = MakeShared<FJsonObject>();
		ParamObj->SetStringField(TEXT("name"), Info.Name.ToString());
		ParamObj->SetNumberField(TEXT("value"), Value);
		ParamObj->SetBoolField(TEXT("overridden"), bOverridden);
		ScalarArray.Add(MakeShared<FJsonValueObject>(ParamObj));
	}
	Result->SetArrayField(TEXT("scalar_params"), ScalarArray);

	// Vector parameters
	TArray<TSharedPtr<FJsonValue>> VectorArray;
	TArray<FMaterialParameterInfo> VectorInfos;
	TArray<FGuid> VectorGuids;
	MI->GetAllVectorParameterInfo(VectorInfos, VectorGuids);
	for (const FMaterialParameterInfo& Info : VectorInfos)
	{
		FLinearColor Value = FLinearColor::Black;
		MI->GetVectorParameterValue(Info, Value);

		bool bOverridden = false;
		for (const auto& Param : MI->VectorParameterValues)
		{
			if (Param.ParameterInfo.Name == Info.Name)
			{
				bOverridden = true;
				break;
			}
		}

		auto ParamObj = MakeShared<FJsonObject>();
		ParamObj->SetStringField(TEXT("name"), Info.Name.ToString());
		ParamObj->SetStringField(TEXT("value"), FString::Printf(TEXT("(R=%.6f,G=%.6f,B=%.6f,A=%.6f)"), Value.R, Value.G, Value.B, Value.A));

		// Also provide sRGB hex for convenience
		FColor SRGB = Value.ToFColor(true);
		ParamObj->SetStringField(TEXT("hex"), FString::Printf(TEXT("#%02X%02X%02X"), SRGB.R, SRGB.G, SRGB.B));

		ParamObj->SetBoolField(TEXT("overridden"), bOverridden);
		VectorArray.Add(MakeShared<FJsonValueObject>(ParamObj));
	}
	Result->SetArrayField(TEXT("vector_params"), VectorArray);

	// Texture parameters
	TArray<TSharedPtr<FJsonValue>> TextureArray;
	TArray<FMaterialParameterInfo> TextureInfos;
	TArray<FGuid> TextureGuids;
	MI->GetAllTextureParameterInfo(TextureInfos, TextureGuids);
	for (const FMaterialParameterInfo& Info : TextureInfos)
	{
		UTexture* Value = nullptr;
		MI->GetTextureParameterValue(Info, Value);

		bool bOverridden = false;
		for (const auto& Param : MI->TextureParameterValues)
		{
			if (Param.ParameterInfo.Name == Info.Name)
			{
				bOverridden = true;
				break;
			}
		}

		auto ParamObj = MakeShared<FJsonObject>();
		ParamObj->SetStringField(TEXT("name"), Info.Name.ToString());
		ParamObj->SetStringField(TEXT("value"), Value ? Value->GetPathName() : TEXT("None"));
		ParamObj->SetBoolField(TEXT("overridden"), bOverridden);
		TextureArray.Add(MakeShared<FJsonValueObject>(ParamObj));
	}
	Result->SetArrayField(TEXT("texture_params"), TextureArray);

	// Static switch parameters
	TArray<TSharedPtr<FJsonValue>> SwitchArray;
	TArray<FMaterialParameterInfo> SwitchInfos;
	TArray<FGuid> SwitchGuids;
	MI->GetAllStaticSwitchParameterInfo(SwitchInfos, SwitchGuids);
	for (const FMaterialParameterInfo& Info : SwitchInfos)
	{
		bool Value = false;
		FGuid OutGuid;
		MI->GetStaticSwitchParameterValue(Info.Name, Value, OutGuid);

		bool bOverridden = false;
		FStaticParameterSet StaticParams;
		MI->GetStaticParameterValues(StaticParams);
		for (const auto& Param : StaticParams.StaticSwitchParameters)
		{
			if (Param.ParameterInfo.Name == Info.Name && Param.bOverride)
			{
				bOverridden = true;
				break;
			}
		}

		auto ParamObj = MakeShared<FJsonObject>();
		ParamObj->SetStringField(TEXT("name"), Info.Name.ToString());
		ParamObj->SetBoolField(TEXT("value"), Value);
		ParamObj->SetBoolField(TEXT("overridden"), bOverridden);
		SwitchArray.Add(MakeShared<FJsonValueObject>(ParamObj));
	}
	Result->SetArrayField(TEXT("static_switch_params"), SwitchArray);

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSetMaterialParam(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return MCPError(TEXT("Missing required param: asset_path"));
	}

	FString ParamName;
	if (!Params->TryGetStringField(TEXT("param_name"), ParamName))
	{
		return MCPError(TEXT("Missing required param: param_name"));
	}

	FString ParamType;
	if (!Params->TryGetStringField(TEXT("param_type"), ParamType))
	{
		return MCPError(TEXT("Missing required param: param_type (scalar, vector, or texture)"));
	}

	UMaterialInstanceConstant* MI = LoadObject<UMaterialInstanceConstant>(nullptr, *AssetPath);
	if (!MI)
	{
		return MCPError(FString::Printf(TEXT("Material instance not found: %s"), *AssetPath));
	}

	if (ParamType == TEXT("scalar"))
	{
		double Value = 0.0;
		if (!Params->TryGetNumberField(TEXT("value"), Value))
		{
			return MCPError(TEXT("Missing required param: value (number)"));
		}
		MI->SetScalarParameterValueEditorOnly(FMaterialParameterInfo(FName(*ParamName)), (float)Value);
	}
	else if (ParamType == TEXT("vector"))
	{
		FString Value;
		if (!Params->TryGetStringField(TEXT("value"), Value))
		{
			return MCPError(TEXT("Missing required param: value (e.g. '(R=1.0,G=0.5,B=0.0,A=1.0)' or '#FF8800')"));
		}

		FLinearColor Color;

		// Support hex format
		if (Value.StartsWith(TEXT("#")))
		{
			FColor SRGB = FColor::FromHex(Value);
			Color = FLinearColor(SRGB);
		}
		else
		{
			// Parse UE format: (R=x,G=x,B=x,A=x)
			Color.InitFromString(Value);
		}

		MI->SetVectorParameterValueEditorOnly(FMaterialParameterInfo(FName(*ParamName)), Color);
	}
	else if (ParamType == TEXT("texture"))
	{
		FString Value;
		if (!Params->TryGetStringField(TEXT("value"), Value))
		{
			return MCPError(TEXT("Missing required param: value (texture asset path)"));
		}
		UTexture* Tex = LoadObject<UTexture>(nullptr, *Value);
		if (!Tex)
		{
			return MCPError(FString::Printf(TEXT("Texture not found: %s"), *Value));
		}
		MI->SetTextureParameterValueEditorOnly(FMaterialParameterInfo(FName(*ParamName)), Tex);
	}
	else
	{
		return MCPError(FString::Printf(TEXT("Unknown param_type: %s (use scalar, vector, or texture)"), *ParamType));
	}

	// The SetXxxParameterValueEditorOnly calls above already update the internal
	// parameter arrays. We just need to mark the package dirty and trigger a
	// deferred update so the render thread picks up changes safely on the next frame.
	MI->MarkPackageDirty();
	MI->RecacheUniformExpressions(true);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("material"), MI->GetName());
	Result->SetStringField(TEXT("param"), ParamName);
	Result->SetStringField(TEXT("type"), ParamType);
	return Result;
}

// ====================================================================================
// EDITOR UTILITY COMMAND HANDLERS
// ====================================================================================

void FMCPTcpServer::RegisterEditorUtilityHandlers()
{
	RegisterHandler(TEXT("execute_console_command"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleExecuteConsoleCommand(Params); });
	RegisterHandler(TEXT("save_asset"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSaveAsset(Params); });
	RegisterHandler(TEXT("save_current_level"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSaveCurrentLevel(Params); });
	RegisterHandler(TEXT("undo"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleUndo(Params); });
	RegisterHandler(TEXT("redo"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleRedo(Params); });
	RegisterHandler(TEXT("create_blueprint"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleCreateBlueprint(Params); });
	RegisterHandler(TEXT("duplicate_asset"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleDuplicateAsset(Params); });
	RegisterHandler(TEXT("delete_asset"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleDeleteAsset(Params); });
	RegisterHandler(TEXT("rename_asset"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleRenameAsset(Params); });
	RegisterHandler(TEXT("open_level"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleOpenLevel(Params); });
	RegisterHandler(TEXT("list_assets"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleListAssets(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleExecuteConsoleCommand(const TSharedPtr<FJsonObject>& Params)
{
	FString Command;
	if (!Params->TryGetStringField(TEXT("command"), Command))
	{
		return MCPError(TEXT("Missing required param: command"));
	}

	// Prefer the PIE world while a session is running so gameplay commands
	// (ke, BugItGo, cheats) reach the game instead of the editor world.
	// Pass world:"editor" to force the editor world during PIE.
	UWorld* World = (GEditor && GEditor->PlayWorld) ? GEditor->PlayWorld.Get() : GetEditorWorld();
	FString WorldParam;
	if (Params->TryGetStringField(TEXT("world"), WorldParam) && WorldParam == TEXT("editor"))
	{
		World = GetEditorWorld();
	}
	if (!World)
	{
		return MCPError(TEXT("No world available"));
	}

	GEngine->Exec(World, *Command);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("executed"), Command);
	Result->SetStringField(TEXT("world"), (GEditor && World == GEditor->PlayWorld) ? TEXT("pie") : TEXT("editor"));
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSaveAsset(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return MCPError(TEXT("Missing required param: asset_path"));
	}

	UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
	if (!Asset)
	{
		// Try finding by name in asset registry
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByPackageName(FName(*AssetPath), Assets);
		if (Assets.Num() > 0)
		{
			Asset = Assets[0].GetAsset();
		}
	}

	if (!Asset)
	{
		return MCPError(FString::Printf(TEXT("Asset not found: %s"), *AssetPath));
	}

	UPackage* Package = Asset->GetOutermost();
	FString PackageFilename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Standalone;
	bool bSaved = UPackage::SavePackage(Package, Asset, *PackageFilename, SaveArgs);

	if (!bSaved)
	{
		return MCPError(TEXT("Failed to save asset"));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("saved"), AssetPath);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSaveCurrentLevel(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return MCPError(TEXT("No editor world available"));
	}

	bool bSaved = FEditorFileUtils::SaveCurrentLevel();

	auto Result = MCPSuccess();
	Result->SetBoolField(TEXT("saved"), bSaved);
	Result->SetStringField(TEXT("level"), World->GetMapName());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleUndo(const TSharedPtr<FJsonObject>& Params)
{
	if (GEditor)
	{
		bool bUndone = GEditor->UndoTransaction();
		auto Result = MCPSuccess();
		Result->SetBoolField(TEXT("undone"), bUndone);
		return Result;
	}
	return MCPError(TEXT("Editor not available"));
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleRedo(const TSharedPtr<FJsonObject>& Params)
{
	if (GEditor)
	{
		bool bRedone = GEditor->RedoTransaction();
		auto Result = MCPSuccess();
		Result->SetBoolField(TEXT("redone"), bRedone);
		return Result;
	}
	return MCPError(TEXT("Editor not available"));
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleCreateBlueprint(const TSharedPtr<FJsonObject>& Params)
{
	FString BPName;
	if (!Params->TryGetStringField(TEXT("name"), BPName))
	{
		return MCPError(TEXT("Missing required param: name"));
	}

	FString PackagePath = TEXT("/Game/Blueprints/");
	Params->TryGetStringField(TEXT("path"), PackagePath);

	FString ParentClassName = TEXT("Actor");
	Params->TryGetStringField(TEXT("parent_class"), ParentClassName);

	UClass* ParentClass = FindClassByName(ParentClassName);
	if (!ParentClass) ParentClass = FindClassByName(TEXT("A") + ParentClassName);
	if (!ParentClass) ParentClass = FindClassByName(TEXT("U") + ParentClassName);
	if (!ParentClass)
	{
		return MCPError(FString::Printf(TEXT("Parent class not found: %s"), *ParentClassName));
	}

	FString FullPath = PackagePath / BPName;
	UPackage* Package = CreatePackage(*FullPath);

	UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
	Factory->ParentClass = ParentClass;

	UObject* NewAsset = Factory->FactoryCreateNew(UBlueprint::StaticClass(), Package, FName(*BPName), RF_Standalone | RF_Public, nullptr, GWarn);
	UBlueprint* NewBP = Cast<UBlueprint>(NewAsset);

	if (!NewBP)
	{
		return MCPError(TEXT("Failed to create Blueprint"));
	}

	FAssetRegistryModule::AssetCreated(NewBP);
	Package->MarkPackageDirty();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("name"), BPName);
	Result->SetStringField(TEXT("path"), NewBP->GetPathName());
	Result->SetStringField(TEXT("parent_class"), ParentClass->GetName());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleDuplicateAsset(const TSharedPtr<FJsonObject>& Params)
{
	FString SourcePath;
	if (!Params->TryGetStringField(TEXT("source_path"), SourcePath))
	{
		return MCPError(TEXT("Missing required param: source_path"));
	}

	FString DestPath;
	if (!Params->TryGetStringField(TEXT("dest_path"), DestPath))
	{
		return MCPError(TEXT("Missing required param: dest_path"));
	}

	FString DestName;
	if (!Params->TryGetStringField(TEXT("dest_name"), DestName))
	{
		return MCPError(TEXT("Missing required param: dest_name"));
	}

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	UObject* SourceAsset = LoadObject<UObject>(nullptr, *SourcePath);
	if (!SourceAsset)
	{
		return MCPError(FString::Printf(TEXT("Source asset not found: %s"), *SourcePath));
	}

	UObject* DuplicatedAsset = AssetTools.DuplicateAsset(DestName, DestPath, SourceAsset);
	if (!DuplicatedAsset)
	{
		return MCPError(TEXT("Failed to duplicate asset"));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("source"), SourcePath);
	Result->SetStringField(TEXT("new_path"), DuplicatedAsset->GetPathName());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleDeleteAsset(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return MCPError(TEXT("Missing required param: asset_path"));
	}

	UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
	if (!Asset)
	{
		return MCPError(FString::Printf(TEXT("Asset not found: %s"), *AssetPath));
	}

	TArray<UObject*> AssetsToDelete;
	AssetsToDelete.Add(Asset);

	int32 DeletedCount = ObjectTools::DeleteObjects(AssetsToDelete, false);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("deleted"), AssetPath);
	Result->SetNumberField(TEXT("count"), DeletedCount);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleRenameAsset(const TSharedPtr<FJsonObject>& Params)
{
	FString SourcePath;
	if (!Params->TryGetStringField(TEXT("source_path"), SourcePath))
	{
		return MCPError(TEXT("Missing required param: source_path"));
	}

	FString NewName;
	if (!Params->TryGetStringField(TEXT("new_name"), NewName))
	{
		return MCPError(TEXT("Missing required param: new_name"));
	}

	FString NewPath;
	Params->TryGetStringField(TEXT("new_path"), NewPath);

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	UObject* Asset = LoadObject<UObject>(nullptr, *SourcePath);
	if (!Asset)
	{
		return MCPError(FString::Printf(TEXT("Asset not found: %s"), *SourcePath));
	}

	FString DestPath = NewPath.IsEmpty() ? FPackageName::GetLongPackagePath(Asset->GetOutermost()->GetName()) : NewPath;

	TArray<FAssetRenameData> RenameData;
	RenameData.Add(FAssetRenameData(Asset, DestPath, NewName));
	bool bSuccess = AssetTools.RenameAssets(RenameData);

	auto Result = MCPSuccess();
	Result->SetBoolField(TEXT("renamed"), bSuccess);
	Result->SetStringField(TEXT("old_path"), SourcePath);
	Result->SetStringField(TEXT("new_name"), NewName);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleOpenLevel(const TSharedPtr<FJsonObject>& Params)
{
	FString LevelPath;
	if (!Params->TryGetStringField(TEXT("level_path"), LevelPath))
	{
		return MCPError(TEXT("Missing required param: level_path"));
	}

	// Search for the level by name if not a full path
	if (!LevelPath.StartsWith(TEXT("/")))
	{
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByClass(UWorld::StaticClass()->GetClassPathName(), Assets);

		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString().Contains(LevelPath))
			{
				LevelPath = Asset.PackageName.ToString();
				break;
			}
		}
	}

	FString Filename;
	if (!FPackageName::TryConvertLongPackageNameToFilename(LevelPath, Filename, FPackageName::GetMapPackageExtension()))
	{
		return MCPError(FString::Printf(TEXT("Level not found: %s"), *LevelPath));
	}

	if (!FPaths::FileExists(Filename))
	{
		return MCPError(FString::Printf(TEXT("Level file not found: %s"), *Filename));
	}

	FEditorFileUtils::LoadMap(LevelPath);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("opened"), LevelPath);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleListAssets(const TSharedPtr<FJsonObject>& Params)
{
	FString PathFilter;
	Params->TryGetStringField(TEXT("path_filter"), PathFilter);

	FString ClassFilter;
	Params->TryGetStringField(TEXT("class_filter"), ClassFilter);

	FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	if (!PathFilter.IsEmpty())
	{
		Filter.PackagePaths.Add(FName(*PathFilter));
		Filter.bRecursivePaths = true;
	}
	if (!ClassFilter.IsEmpty())
	{
		UClass* FilterClass = FindClassByName(ClassFilter);
		if (FilterClass)
		{
			Filter.ClassPaths.Add(FilterClass->GetClassPathName());
		}
	}

	TArray<FAssetData> Assets;
	AssetReg.Get().GetAssets(Filter, Assets);

	int32 MaxResults = 100;
	double TempMaxResults;
	if (Params->TryGetNumberField(TEXT("max_results"), TempMaxResults))
	{
		MaxResults = (int32)TempMaxResults;
	}

	TArray<TSharedPtr<FJsonValue>> AssetArray;
	for (int32 i = 0; i < FMath::Min(Assets.Num(), MaxResults); ++i)
	{
		const FAssetData& Asset = Assets[i];
		auto AssetObj = MakeShared<FJsonObject>();
		AssetObj->SetStringField(TEXT("name"), Asset.AssetName.ToString());
		AssetObj->SetStringField(TEXT("path"), Asset.GetObjectPathString());
		AssetObj->SetStringField(TEXT("class"), Asset.AssetClassPath.GetAssetName().ToString());
		AssetObj->SetStringField(TEXT("package"), Asset.PackageName.ToString());
		AssetArray.Add(MakeShared<FJsonValueObject>(AssetObj));
	}

	auto Result = MCPSuccess();
	Result->SetArrayField(TEXT("assets"), AssetArray);
	Result->SetNumberField(TEXT("count"), AssetArray.Num());
	Result->SetNumberField(TEXT("total"), Assets.Num());
	return Result;
}

// ====================================================================================
// COMPONENT COMMAND HANDLERS
// ====================================================================================

void FMCPTcpServer::RegisterComponentHandlers()
{
	RegisterHandler(TEXT("list_components"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleListComponents(Params); });
	RegisterHandler(TEXT("add_actor_component"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddActorComponent(Params); });
	RegisterHandler(TEXT("remove_actor_component"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleRemoveActorComponent(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleListComponents(const TSharedPtr<FJsonObject>& Params)
{
	FString ActorName;
	if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
	{
		return MCPError(TEXT("Missing required param: actor_name"));
	}

	AActor* Actor = FindActorByName(ActorName);
	if (!Actor)
	{
		return MCPError(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
	}

	TArray<TSharedPtr<FJsonValue>> CompArray;
	TInlineComponentArray<UActorComponent*> Components;
	Actor->GetComponents(Components);

	for (UActorComponent* Comp : Components)
	{
		auto CompObj = MakeShared<FJsonObject>();
		CompObj->SetStringField(TEXT("name"), Comp->GetName());
		CompObj->SetStringField(TEXT("class"), Comp->GetClass()->GetName());

		if (USceneComponent* SceneComp = Cast<USceneComponent>(Comp))
		{
			FVector Loc = SceneComp->GetRelativeLocation();
			CompObj->SetNumberField(TEXT("x"), Loc.X);
			CompObj->SetNumberField(TEXT("y"), Loc.Y);
			CompObj->SetNumberField(TEXT("z"), Loc.Z);
			CompObj->SetBoolField(TEXT("is_root"), SceneComp == Actor->GetRootComponent());
		}

		CompObj->SetBoolField(TEXT("is_active"), Comp->IsActive());
		CompArray.Add(MakeShared<FJsonValueObject>(CompObj));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("actor"), ActorName);
	Result->SetArrayField(TEXT("components"), CompArray);
	Result->SetNumberField(TEXT("count"), CompArray.Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddActorComponent(const TSharedPtr<FJsonObject>& Params)
{
	FString ActorName;
	if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
	{
		return MCPError(TEXT("Missing required param: actor_name"));
	}

	FString ComponentClass;
	if (!Params->TryGetStringField(TEXT("component_class"), ComponentClass))
	{
		return MCPError(TEXT("Missing required param: component_class"));
	}

	AActor* Actor = FindActorByName(ActorName);
	if (!Actor)
	{
		return MCPError(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
	}

	UClass* CompClass = FindClassByName(ComponentClass);
	if (!CompClass) CompClass = FindClassByName(TEXT("U") + ComponentClass);
	if (!CompClass || !CompClass->IsChildOf(UActorComponent::StaticClass()))
	{
		return MCPError(FString::Printf(TEXT("Component class not found or invalid: %s"), *ComponentClass));
	}

	FString CompName;
	Params->TryGetStringField(TEXT("component_name"), CompName);
	FName CompFName = CompName.IsEmpty() ? CompClass->GetFName() : FName(*CompName);

	UActorComponent* NewComp = NewObject<UActorComponent>(Actor, CompClass, CompFName);
	if (!NewComp)
	{
		return MCPError(TEXT("Failed to create component"));
	}

	if (USceneComponent* SceneComp = Cast<USceneComponent>(NewComp))
	{
		SceneComp->SetupAttachment(Actor->GetRootComponent());
	}

	Actor->AddInstanceComponent(NewComp);
	NewComp->RegisterComponent();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("actor"), ActorName);
	Result->SetStringField(TEXT("component_name"), NewComp->GetName());
	Result->SetStringField(TEXT("component_class"), CompClass->GetName());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleRemoveActorComponent(const TSharedPtr<FJsonObject>& Params)
{
	FString ActorName;
	if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
	{
		return MCPError(TEXT("Missing required param: actor_name"));
	}

	FString ComponentName;
	if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
	{
		return MCPError(TEXT("Missing required param: component_name"));
	}

	AActor* Actor = FindActorByName(ActorName);
	if (!Actor)
	{
		return MCPError(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
	}

	UActorComponent* TargetComp = nullptr;
	TInlineComponentArray<UActorComponent*> Components;
	Actor->GetComponents(Components);
	for (UActorComponent* Comp : Components)
	{
		if (Comp->GetName() == ComponentName)
		{
			TargetComp = Comp;
			break;
		}
	}

	if (!TargetComp)
	{
		return MCPError(FString::Printf(TEXT("Component not found: %s on actor %s"), *ComponentName, *ActorName));
	}

	TargetComp->DestroyComponent();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("actor"), ActorName);
	Result->SetStringField(TEXT("removed_component"), ComponentName);
	return Result;
}

// ====================================================================================
// WIDGET/UMG COMMAND HANDLERS
// ====================================================================================

void FMCPTcpServer::RegisterWidgetHandlers()
{
	RegisterHandler(TEXT("read_widget_tree"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleReadWidgetTree(Params); });
	RegisterHandler(TEXT("list_widget_children"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleListWidgetChildren(Params); });
	RegisterHandler(TEXT("add_widget_child"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddWidgetChild(Params); });
	RegisterHandler(TEXT("remove_widget_child"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleRemoveWidgetChild(Params); });
	RegisterHandler(TEXT("set_widget_property"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSetWidgetProperty(Params); });
	RegisterHandler(TEXT("get_widget_property"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleGetWidgetProperty(Params); });
	RegisterHandler(TEXT("set_widget_slot"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSetWidgetSlot(Params); });
}

// Helper: find a WidgetBlueprint by path or name
static UWidgetBlueprint* FindWidgetBlueprintByPath(const FString& Path)
{
	FString AssetPath = Path;
	if (!AssetPath.StartsWith(TEXT("/")))
	{
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByClass(UWidgetBlueprint::StaticClass()->GetClassPathName(), Assets);
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == Path || Asset.GetObjectPathString().Contains(Path))
			{
				return Cast<UWidgetBlueprint>(Asset.GetAsset());
			}
		}
		return nullptr;
	}
	return LoadObject<UWidgetBlueprint>(nullptr, *AssetPath);
}

// Helper: recursively serialize widget tree
static TSharedPtr<FJsonObject> SerializeWidget(UWidget* Widget)
{
	if (!Widget) return nullptr;

	auto Obj = MakeShared<FJsonObject>();
	Obj->SetStringField(TEXT("name"), Widget->GetName());
	Obj->SetStringField(TEXT("class"), Widget->GetClass()->GetName());
	Obj->SetBoolField(TEXT("is_visible"), Widget->IsVisible());

	if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
	{
		Obj->SetStringField(TEXT("text"), TextBlock->GetText().ToString());
	}

	if (UPanelWidget* Panel = Cast<UPanelWidget>(Widget))
	{
		TArray<TSharedPtr<FJsonValue>> Children;
		for (int32 i = 0; i < Panel->GetChildrenCount(); i++)
		{
			UWidget* Child = Panel->GetChildAt(i);
			auto ChildObj = SerializeWidget(Child);
			if (ChildObj)
			{
				Children.Add(MakeShared<FJsonValueObject>(ChildObj));
			}
		}
		Obj->SetArrayField(TEXT("children"), Children);
	}

	return Obj;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleReadWidgetTree(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetBPPath;
	if (!Params->TryGetStringField(TEXT("widget_blueprint"), WidgetBPPath))
	{
		return MCPError(TEXT("Missing required param: widget_blueprint"));
	}

	UWidgetBlueprint* WBP = FindWidgetBlueprintByPath(WidgetBPPath);
	if (!WBP)
	{
		return MCPError(FString::Printf(TEXT("Widget Blueprint not found: %s"), *WidgetBPPath));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("widget_blueprint"), WBP->GetPathName());

	if (WBP->WidgetTree)
	{
		UWidget* Root = WBP->WidgetTree->RootWidget;
		if (Root)
		{
			Result->SetObjectField(TEXT("tree"), SerializeWidget(Root));
		}

		// List all widgets
		TArray<TSharedPtr<FJsonValue>> AllWidgets;
		WBP->WidgetTree->ForEachWidget([&AllWidgets](UWidget* Widget)
		{
			auto WObj = MakeShared<FJsonObject>();
			WObj->SetStringField(TEXT("name"), Widget->GetName());
			WObj->SetStringField(TEXT("class"), Widget->GetClass()->GetName());
			AllWidgets.Add(MakeShared<FJsonValueObject>(WObj));
		});
		Result->SetArrayField(TEXT("all_widgets"), AllWidgets);
		Result->SetNumberField(TEXT("widget_count"), AllWidgets.Num());
	}

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleListWidgetChildren(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetBPPath;
	if (!Params->TryGetStringField(TEXT("widget_blueprint"), WidgetBPPath))
	{
		return MCPError(TEXT("Missing required param: widget_blueprint"));
	}

	UWidgetBlueprint* WBP = FindWidgetBlueprintByPath(WidgetBPPath);
	if (!WBP)
	{
		return MCPError(FString::Printf(TEXT("Widget Blueprint not found: %s"), *WidgetBPPath));
	}

	FString ParentName;
	Params->TryGetStringField(TEXT("parent_name"), ParentName);

	if (!WBP->WidgetTree)
	{
		return MCPError(TEXT("Widget tree is null"));
	}

	UPanelWidget* Parent = nullptr;
	if (ParentName.IsEmpty())
	{
		Parent = Cast<UPanelWidget>(WBP->WidgetTree->RootWidget);
	}
	else
	{
		WBP->WidgetTree->ForEachWidget([&Parent, &ParentName](UWidget* Widget)
		{
			if (Widget->GetName() == ParentName)
			{
				Parent = Cast<UPanelWidget>(Widget);
			}
		});
	}

	if (!Parent)
	{
		return MCPError(FString::Printf(TEXT("Parent panel widget not found: %s"), *ParentName));
	}

	TArray<TSharedPtr<FJsonValue>> Children;
	for (int32 i = 0; i < Parent->GetChildrenCount(); i++)
	{
		UWidget* Child = Parent->GetChildAt(i);
		auto ChildObj = MakeShared<FJsonObject>();
		ChildObj->SetStringField(TEXT("name"), Child->GetName());
		ChildObj->SetStringField(TEXT("class"), Child->GetClass()->GetName());
		ChildObj->SetNumberField(TEXT("index"), i);
		Children.Add(MakeShared<FJsonValueObject>(ChildObj));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("parent"), Parent->GetName());
	Result->SetArrayField(TEXT("children"), Children);
	Result->SetNumberField(TEXT("count"), Children.Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddWidgetChild(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetBPPath;
	if (!Params->TryGetStringField(TEXT("widget_blueprint"), WidgetBPPath))
	{
		return MCPError(TEXT("Missing required param: widget_blueprint"));
	}

	FString WidgetClass;
	if (!Params->TryGetStringField(TEXT("widget_class"), WidgetClass))
	{
		return MCPError(TEXT("Missing required param: widget_class (e.g. 'TextBlock', 'Image', 'Button', 'CanvasPanel', 'VerticalBox', 'HorizontalBox', 'ProgressBar', 'Border', 'Overlay', 'SizeBox')"));
	}

	UWidgetBlueprint* WBP = FindWidgetBlueprintByPath(WidgetBPPath);
	if (!WBP)
	{
		return MCPError(FString::Printf(TEXT("Widget Blueprint not found: %s"), *WidgetBPPath));
	}

	if (!WBP->WidgetTree)
	{
		return MCPError(TEXT("Widget tree is null"));
	}

	// Find the widget class
	UClass* WClass = FindClassByName(WidgetClass);
	if (!WClass) WClass = FindClassByName(TEXT("U") + WidgetClass);
	if (!WClass || !WClass->IsChildOf(UWidget::StaticClass()))
	{
		return MCPError(FString::Printf(TEXT("Widget class not found: %s"), *WidgetClass));
	}

	// Find parent first (before constructing, so we can validate)
	FString ParentName;
	Params->TryGetStringField(TEXT("parent_name"), ParentName);

	UPanelWidget* Parent = nullptr;
	bool bSetAsRoot = false;

	if (ParentName.IsEmpty())
	{
		Parent = Cast<UPanelWidget>(WBP->WidgetTree->RootWidget);
		if (!Parent)
		{
			// Will set as root — validated after construction
			bSetAsRoot = true;
		}
	}
	else
	{
		// Search for parent widget by name
		WBP->WidgetTree->ForEachWidget([&Parent, &ParentName](UWidget* Widget)
		{
			if (!Parent && Widget->GetName() == ParentName)
			{
				Parent = Cast<UPanelWidget>(Widget);
			}
		});

		if (!Parent)
		{
			return MCPError(FString::Printf(TEXT("Parent panel widget not found: %s"), *ParentName));
		}

		// Check if parent is a single-child widget (Border, SizeBox, etc.) that already has a child
		if (UContentWidget* ContentParent = Cast<UContentWidget>(Parent))
		{
			if (ContentParent->GetChildrenCount() > 0)
			{
				return MCPError(FString::Printf(TEXT("Parent '%s' is a single-child widget (%s) and already has a child. Remove the existing child first."),
					*ParentName, *Parent->GetClass()->GetName()));
			}
		}
	}

	// Generate a unique name to avoid FName conflicts
	FString WidgetName;
	Params->TryGetStringField(TEXT("name"), WidgetName);
	FName WFName;
	if (WidgetName.IsEmpty())
	{
		WFName = MakeUniqueObjectName(WBP->WidgetTree, WClass, FName(*WidgetClass));
	}
	else
	{
		// Check if the name is already taken
		WFName = FName(*WidgetName);
		UObject* Existing = StaticFindObjectFast(nullptr, WBP->WidgetTree, WFName);
		if (Existing)
		{
			WFName = MakeUniqueObjectName(WBP->WidgetTree, WClass, WFName);
			UE_LOG(LogUnrealMCP, Warning, TEXT("[UnrealMCP] Widget name '%s' already taken, using '%s'"), *WidgetName, *WFName.ToString());
		}
	}

	UWidget* NewWidget = WBP->WidgetTree->ConstructWidget<UWidget>(WClass, WFName);
	if (!NewWidget)
	{
		return MCPError(FString::Printf(TEXT("Failed to construct widget of class: %s"), *WidgetClass));
	}

	if (bSetAsRoot)
	{
		// Set as root widget
		if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(NewWidget))
		{
			WBP->WidgetTree->RootWidget = PanelWidget;

			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WBP);

			auto Result = MCPSuccess();
			Result->SetStringField(TEXT("name"), NewWidget->GetName());
			Result->SetStringField(TEXT("class"), NewWidget->GetClass()->GetName());
			Result->SetBoolField(TEXT("set_as_root"), true);
			return Result;
		}
		return MCPError(TEXT("No root widget and new widget is not a panel — cannot set as root"));
	}

	// Add to parent
	UPanelSlot* Slot = Parent->AddChild(NewWidget);
	if (!Slot)
	{
		// AddChild failed — clean up the orphaned widget
		WBP->WidgetTree->RemoveWidget(NewWidget);
		return MCPError(FString::Printf(TEXT("Failed to add widget to parent '%s'. Parent may not accept children of this type."), *Parent->GetName()));
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WBP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("name"), NewWidget->GetName());
	Result->SetStringField(TEXT("class"), NewWidget->GetClass()->GetName());
	Result->SetStringField(TEXT("parent"), Parent->GetName());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleRemoveWidgetChild(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetBPPath;
	if (!Params->TryGetStringField(TEXT("widget_blueprint"), WidgetBPPath))
	{
		return MCPError(TEXT("Missing required param: widget_blueprint"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return MCPError(TEXT("Missing required param: widget_name"));
	}

	UWidgetBlueprint* WBP = FindWidgetBlueprintByPath(WidgetBPPath);
	if (!WBP)
	{
		return MCPError(FString::Printf(TEXT("Widget Blueprint not found: %s"), *WidgetBPPath));
	}

	if (!WBP->WidgetTree)
	{
		return MCPError(TEXT("Widget tree is null"));
	}

	UWidget* TargetWidget = nullptr;
	WBP->WidgetTree->ForEachWidget([&TargetWidget, &WidgetName](UWidget* Widget)
	{
		if (Widget->GetName() == WidgetName)
		{
			TargetWidget = Widget;
		}
	});

	if (!TargetWidget)
	{
		return MCPError(FString::Printf(TEXT("Widget not found: %s"), *WidgetName));
	}

	// If removing a panel widget, also remove all its children first
	if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(TargetWidget))
	{
		while (PanelWidget->GetChildrenCount() > 0)
		{
			UWidget* Child = PanelWidget->GetChildAt(0);
			WBP->WidgetTree->RemoveWidget(Child);
		}
	}

	// If it's the root, clear the root reference
	if (WBP->WidgetTree->RootWidget == TargetWidget)
	{
		WBP->WidgetTree->RootWidget = nullptr;
	}

	WBP->WidgetTree->RemoveWidget(TargetWidget);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WBP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("removed"), WidgetName);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSetWidgetProperty(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetBPPath;
	if (!Params->TryGetStringField(TEXT("widget_blueprint"), WidgetBPPath))
	{
		return MCPError(TEXT("Missing required param: widget_blueprint"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return MCPError(TEXT("Missing required param: widget_name"));
	}

	FString PropertyName;
	if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
	{
		return MCPError(TEXT("Missing required param: property_name"));
	}

	FString Value;
	if (!Params->TryGetStringField(TEXT("value"), Value))
	{
		return MCPError(TEXT("Missing required param: value"));
	}

	UWidgetBlueprint* WBP = FindWidgetBlueprintByPath(WidgetBPPath);
	if (!WBP)
	{
		return MCPError(FString::Printf(TEXT("Widget Blueprint not found: %s"), *WidgetBPPath));
	}

	if (!WBP->WidgetTree)
	{
		return MCPError(TEXT("Widget tree is null"));
	}

	UWidget* TargetWidget = nullptr;
	WBP->WidgetTree->ForEachWidget([&TargetWidget, &WidgetName](UWidget* Widget)
	{
		if (Widget->GetName() == WidgetName)
		{
			TargetWidget = Widget;
		}
	});

	if (!TargetWidget)
	{
		return MCPError(FString::Printf(TEXT("Widget not found: %s"), *WidgetName));
	}

	// Special handling for common widget properties
	bool bHandled = false;

	// TextBlock.Text — FText needs special handling
	if (PropertyName == TEXT("Text"))
	{
		if (UTextBlock* TextBlock = Cast<UTextBlock>(TargetWidget))
		{
			TextBlock->SetText(FText::FromString(Value));
			bHandled = true;
		}
	}
	// Visibility — ESlateVisibility enum
	else if (PropertyName == TEXT("Visibility"))
	{
		ESlateVisibility NewVisibility = ESlateVisibility::Visible;
		if (Value == TEXT("Collapsed")) NewVisibility = ESlateVisibility::Collapsed;
		else if (Value == TEXT("Hidden")) NewVisibility = ESlateVisibility::Hidden;
		else if (Value == TEXT("HitTestInvisible")) NewVisibility = ESlateVisibility::HitTestInvisible;
		else if (Value == TEXT("SelfHitTestInvisible")) NewVisibility = ESlateVisibility::SelfHitTestInvisible;
		TargetWidget->SetVisibility(NewVisibility);
		bHandled = true;
	}

	if (!bHandled)
	{
		// Use reflection to set the property
		FProperty* Prop = TargetWidget->GetClass()->FindPropertyByName(FName(*PropertyName));
		if (!Prop)
		{
			return MCPError(FString::Printf(TEXT("Property not found: %s on widget %s"), *PropertyName, *WidgetName));
		}

		void* PropAddr = Prop->ContainerPtrToValuePtr<void>(TargetWidget);
		bool bSuccess = Prop->ImportText_Direct(*Value, PropAddr, TargetWidget, PPF_None) != nullptr;
		if (!bSuccess)
		{
			return MCPError(FString::Printf(TEXT("Failed to set property %s to value: %s"), *PropertyName, *Value));
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WBP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("widget"), WidgetName);
	Result->SetStringField(TEXT("property"), PropertyName);
	Result->SetStringField(TEXT("value"), Value);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleGetWidgetProperty(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetBPPath;
	if (!Params->TryGetStringField(TEXT("widget_blueprint"), WidgetBPPath))
	{
		return MCPError(TEXT("Missing required param: widget_blueprint"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return MCPError(TEXT("Missing required param: widget_name"));
	}

	FString PropertyName;
	if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
	{
		return MCPError(TEXT("Missing required param: property_name"));
	}

	UWidgetBlueprint* WBP = FindWidgetBlueprintByPath(WidgetBPPath);
	if (!WBP)
	{
		return MCPError(FString::Printf(TEXT("Widget Blueprint not found: %s"), *WidgetBPPath));
	}

	if (!WBP->WidgetTree)
	{
		return MCPError(TEXT("Widget tree is null"));
	}

	UWidget* TargetWidget = nullptr;
	WBP->WidgetTree->ForEachWidget([&TargetWidget, &WidgetName](UWidget* Widget)
	{
		if (Widget->GetName() == WidgetName)
		{
			TargetWidget = Widget;
		}
	});

	if (!TargetWidget)
	{
		return MCPError(FString::Printf(TEXT("Widget not found: %s"), *WidgetName));
	}

	// If property_name is "*", dump all properties with non-default values
	if (PropertyName == TEXT("*"))
	{
		auto Result = MCPSuccess();
		Result->SetStringField(TEXT("widget"), WidgetName);
		Result->SetStringField(TEXT("class"), TargetWidget->GetClass()->GetName());

		TSharedPtr<FJsonObject> PropsObj = MakeShareable(new FJsonObject());
		UObject* DefaultObj = TargetWidget->GetClass()->GetDefaultObject();

		for (TFieldIterator<FProperty> It(TargetWidget->GetClass()); It; ++It)
		{
			FProperty* Prop = *It;
			if (Prop->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_EditorOnly))
				continue;

			void* ValueAddr = Prop->ContainerPtrToValuePtr<void>(TargetWidget);
			void* DefaultAddr = Prop->ContainerPtrToValuePtr<void>(DefaultObj);

			if (!Prop->Identical(ValueAddr, DefaultAddr))
			{
				FString ValueStr;
				Prop->ExportTextItem_Direct(ValueStr, ValueAddr, DefaultAddr, TargetWidget, PPF_None);
				PropsObj->SetStringField(Prop->GetName(), ValueStr);
			}
		}

		Result->SetObjectField(TEXT("properties"), PropsObj);
		return Result;
	}

	// Single property lookup
	FProperty* Prop = TargetWidget->GetClass()->FindPropertyByName(FName(*PropertyName));
	if (!Prop)
	{
		return MCPError(FString::Printf(TEXT("Property not found: %s on widget %s (%s)"), *PropertyName, *WidgetName, *TargetWidget->GetClass()->GetName()));
	}

	void* ValueAddr = Prop->ContainerPtrToValuePtr<void>(TargetWidget);
	FString ValueStr;
	Prop->ExportTextItem_Direct(ValueStr, ValueAddr, nullptr, TargetWidget, PPF_None);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("widget"), WidgetName);
	Result->SetStringField(TEXT("property"), PropertyName);
	Result->SetStringField(TEXT("type"), Prop->GetCPPType());
	Result->SetStringField(TEXT("value"), ValueStr);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSetWidgetSlot(const TSharedPtr<FJsonObject>& Params)
{
	FString WidgetBPPath;
	if (!Params->TryGetStringField(TEXT("widget_blueprint"), WidgetBPPath))
	{
		return MCPError(TEXT("Missing required param: widget_blueprint"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return MCPError(TEXT("Missing required param: widget_name"));
	}

	UWidgetBlueprint* WBP = FindWidgetBlueprintByPath(WidgetBPPath);
	if (!WBP)
	{
		return MCPError(FString::Printf(TEXT("Widget Blueprint not found: %s"), *WidgetBPPath));
	}

	if (!WBP->WidgetTree)
	{
		return MCPError(TEXT("Widget tree is null"));
	}

	UWidget* TargetWidget = nullptr;
	WBP->WidgetTree->ForEachWidget([&TargetWidget, &WidgetName](UWidget* Widget)
	{
		if (Widget->GetName() == WidgetName)
		{
			TargetWidget = Widget;
		}
	});

	if (!TargetWidget)
	{
		return MCPError(FString::Printf(TEXT("Widget not found: %s"), *WidgetName));
	}

	UPanelSlot* Slot = TargetWidget->Slot;
	if (!Slot)
	{
		return MCPError(FString::Printf(TEXT("Widget '%s' has no slot (not a child of a panel)"), *WidgetName));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("widget"), WidgetName);
	Result->SetStringField(TEXT("slot_class"), Slot->GetClass()->GetName());

	// Canvas Panel Slot
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		// Anchors
		const TSharedPtr<FJsonObject>* AnchorsObj;
		if (Params->TryGetObjectField(TEXT("anchors"), AnchorsObj))
		{
			FAnchors Anchors = CanvasSlot->GetAnchors();
			double Val;
			if ((*AnchorsObj)->TryGetNumberField(TEXT("min_x"), Val)) Anchors.Minimum.X = Val;
			if ((*AnchorsObj)->TryGetNumberField(TEXT("min_y"), Val)) Anchors.Minimum.Y = Val;
			if ((*AnchorsObj)->TryGetNumberField(TEXT("max_x"), Val)) Anchors.Maximum.X = Val;
			if ((*AnchorsObj)->TryGetNumberField(TEXT("max_y"), Val)) Anchors.Maximum.Y = Val;
			CanvasSlot->SetAnchors(Anchors);
		}

		// Offsets (position and size when not stretching)
		const TSharedPtr<FJsonObject>* OffsetsObj;
		if (Params->TryGetObjectField(TEXT("offsets"), OffsetsObj))
		{
			FMargin Offsets = CanvasSlot->GetOffsets();
			double Val;
			if ((*OffsetsObj)->TryGetNumberField(TEXT("left"), Val)) Offsets.Left = Val;
			if ((*OffsetsObj)->TryGetNumberField(TEXT("top"), Val)) Offsets.Top = Val;
			if ((*OffsetsObj)->TryGetNumberField(TEXT("right"), Val)) Offsets.Right = Val;
			if ((*OffsetsObj)->TryGetNumberField(TEXT("bottom"), Val)) Offsets.Bottom = Val;
			CanvasSlot->SetOffsets(Offsets);
		}

		// Position (convenience — sets left/top offsets)
		const TSharedPtr<FJsonObject>* PosObj;
		if (Params->TryGetObjectField(TEXT("position"), PosObj))
		{
			double X = 0, Y = 0;
			(*PosObj)->TryGetNumberField(TEXT("x"), X);
			(*PosObj)->TryGetNumberField(TEXT("y"), Y);
			CanvasSlot->SetPosition(FVector2D(X, Y));
		}

		// Size
		const TSharedPtr<FJsonObject>* SizeObj;
		if (Params->TryGetObjectField(TEXT("size"), SizeObj))
		{
			double W = 0, H = 0;
			(*SizeObj)->TryGetNumberField(TEXT("width"), W);
			(*SizeObj)->TryGetNumberField(TEXT("height"), H);
			CanvasSlot->SetSize(FVector2D(W, H));
		}

		// Alignment
		const TSharedPtr<FJsonObject>* AlignObj;
		if (Params->TryGetObjectField(TEXT("alignment"), AlignObj))
		{
			double X = 0, Y = 0;
			(*AlignObj)->TryGetNumberField(TEXT("x"), X);
			(*AlignObj)->TryGetNumberField(TEXT("y"), Y);
			CanvasSlot->SetAlignment(FVector2D(X, Y));
		}

		// Auto-size
		bool bAutoSize = false;
		if (Params->TryGetBoolField(TEXT("auto_size"), bAutoSize))
		{
			CanvasSlot->SetAutoSize(bAutoSize);
		}

		// Z-order
		double ZOrder;
		if (Params->TryGetNumberField(TEXT("z_order"), ZOrder))
		{
			CanvasSlot->SetZOrder((int32)ZOrder);
		}

		Result->SetStringField(TEXT("slot_type"), TEXT("CanvasPanel"));
	}
	// Vertical Box Slot
	else if (UVerticalBoxSlot* VBoxSlot = Cast<UVerticalBoxSlot>(Slot))
	{
		// Padding
		const TSharedPtr<FJsonObject>* PadObj;
		if (Params->TryGetObjectField(TEXT("padding"), PadObj))
		{
			FMargin Padding;
			double Val;
			if ((*PadObj)->TryGetNumberField(TEXT("left"), Val)) Padding.Left = Val;
			if ((*PadObj)->TryGetNumberField(TEXT("top"), Val)) Padding.Top = Val;
			if ((*PadObj)->TryGetNumberField(TEXT("right"), Val)) Padding.Right = Val;
			if ((*PadObj)->TryGetNumberField(TEXT("bottom"), Val)) Padding.Bottom = Val;
			// Check for uniform padding
			if ((*PadObj)->TryGetNumberField(TEXT("all"), Val))
			{
				Padding.Left = Padding.Top = Padding.Right = Padding.Bottom = Val;
			}
			VBoxSlot->SetPadding(Padding);
		}

		// Size rule
		FString SizeRule;
		if (Params->TryGetStringField(TEXT("size_rule"), SizeRule))
		{
			FSlateChildSize Size;
			if (SizeRule == TEXT("Auto")) Size.SizeRule = ESlateSizeRule::Automatic;
			else if (SizeRule == TEXT("Fill")) Size.SizeRule = ESlateSizeRule::Fill;
			double FillVal;
			if (Params->TryGetNumberField(TEXT("fill_value"), FillVal)) Size.Value = FillVal;
			VBoxSlot->SetSize(Size);
		}

		// Horizontal alignment
		FString HAlign;
		if (Params->TryGetStringField(TEXT("h_align"), HAlign))
		{
			if (HAlign == TEXT("Fill")) VBoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			else if (HAlign == TEXT("Left")) VBoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
			else if (HAlign == TEXT("Center")) VBoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			else if (HAlign == TEXT("Right")) VBoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		}

		// Vertical alignment
		FString VAlign;
		if (Params->TryGetStringField(TEXT("v_align"), VAlign))
		{
			if (VAlign == TEXT("Fill")) VBoxSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
			else if (VAlign == TEXT("Top")) VBoxSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
			else if (VAlign == TEXT("Center")) VBoxSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
			else if (VAlign == TEXT("Bottom")) VBoxSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		}

		Result->SetStringField(TEXT("slot_type"), TEXT("VerticalBox"));
	}
	// Horizontal Box Slot
	else if (UHorizontalBoxSlot* HBoxSlot = Cast<UHorizontalBoxSlot>(Slot))
	{
		const TSharedPtr<FJsonObject>* PadObj;
		if (Params->TryGetObjectField(TEXT("padding"), PadObj))
		{
			FMargin Padding;
			double Val;
			if ((*PadObj)->TryGetNumberField(TEXT("left"), Val)) Padding.Left = Val;
			if ((*PadObj)->TryGetNumberField(TEXT("top"), Val)) Padding.Top = Val;
			if ((*PadObj)->TryGetNumberField(TEXT("right"), Val)) Padding.Right = Val;
			if ((*PadObj)->TryGetNumberField(TEXT("bottom"), Val)) Padding.Bottom = Val;
			if ((*PadObj)->TryGetNumberField(TEXT("all"), Val))
			{
				Padding.Left = Padding.Top = Padding.Right = Padding.Bottom = Val;
			}
			HBoxSlot->SetPadding(Padding);
		}

		FString SizeRule;
		if (Params->TryGetStringField(TEXT("size_rule"), SizeRule))
		{
			FSlateChildSize Size;
			if (SizeRule == TEXT("Auto")) Size.SizeRule = ESlateSizeRule::Automatic;
			else if (SizeRule == TEXT("Fill")) Size.SizeRule = ESlateSizeRule::Fill;
			double FillVal;
			if (Params->TryGetNumberField(TEXT("fill_value"), FillVal)) Size.Value = FillVal;
			HBoxSlot->SetSize(Size);
		}

		FString HAlign;
		if (Params->TryGetStringField(TEXT("h_align"), HAlign))
		{
			if (HAlign == TEXT("Fill")) HBoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			else if (HAlign == TEXT("Left")) HBoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
			else if (HAlign == TEXT("Center")) HBoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			else if (HAlign == TEXT("Right")) HBoxSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		}

		FString VAlign;
		if (Params->TryGetStringField(TEXT("v_align"), VAlign))
		{
			if (VAlign == TEXT("Fill")) HBoxSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
			else if (VAlign == TEXT("Top")) HBoxSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
			else if (VAlign == TEXT("Center")) HBoxSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
			else if (VAlign == TEXT("Bottom")) HBoxSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		}

		Result->SetStringField(TEXT("slot_type"), TEXT("HorizontalBox"));
	}
	// Overlay Slot
	else if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Slot))
	{
		const TSharedPtr<FJsonObject>* PadObj;
		if (Params->TryGetObjectField(TEXT("padding"), PadObj))
		{
			FMargin Padding;
			double Val;
			if ((*PadObj)->TryGetNumberField(TEXT("left"), Val)) Padding.Left = Val;
			if ((*PadObj)->TryGetNumberField(TEXT("top"), Val)) Padding.Top = Val;
			if ((*PadObj)->TryGetNumberField(TEXT("right"), Val)) Padding.Right = Val;
			if ((*PadObj)->TryGetNumberField(TEXT("bottom"), Val)) Padding.Bottom = Val;
			if ((*PadObj)->TryGetNumberField(TEXT("all"), Val))
			{
				Padding.Left = Padding.Top = Padding.Right = Padding.Bottom = Val;
			}
			OverlaySlot->SetPadding(Padding);
		}

		FString HAlign;
		if (Params->TryGetStringField(TEXT("h_align"), HAlign))
		{
			if (HAlign == TEXT("Fill")) OverlaySlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			else if (HAlign == TEXT("Left")) OverlaySlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
			else if (HAlign == TEXT("Center")) OverlaySlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			else if (HAlign == TEXT("Right")) OverlaySlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		}

		FString VAlign;
		if (Params->TryGetStringField(TEXT("v_align"), VAlign))
		{
			if (VAlign == TEXT("Fill")) OverlaySlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
			else if (VAlign == TEXT("Top")) OverlaySlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
			else if (VAlign == TEXT("Center")) OverlaySlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
			else if (VAlign == TEXT("Bottom")) OverlaySlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		}

		Result->SetStringField(TEXT("slot_type"), TEXT("Overlay"));
	}
	else
	{
		// Generic fallback — try setting slot properties via reflection
		FProperty* Prop = nullptr;
		FString PropertyName;
		FString Value;

		if (Params->TryGetStringField(TEXT("property_name"), PropertyName) &&
			Params->TryGetStringField(TEXT("value"), Value))
		{
			Prop = Slot->GetClass()->FindPropertyByName(FName(*PropertyName));
			if (Prop)
			{
				void* PropAddr = Prop->ContainerPtrToValuePtr<void>(Slot);
				Prop->ImportText_Direct(*Value, PropAddr, Slot, PPF_None);
				Result->SetStringField(TEXT("slot_type"), TEXT("Generic"));
				Result->SetStringField(TEXT("set_property"), PropertyName);
			}
			else
			{
				return MCPError(FString::Printf(TEXT("Slot property not found: %s on slot class %s"), *PropertyName, *Slot->GetClass()->GetName()));
			}
		}
		else
		{
			return MCPError(FString::Printf(TEXT("Unsupported slot type: %s. Use property_name and value params for generic slot property access."), *Slot->GetClass()->GetName()));
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(WBP);
	return Result;
}

// ====================================================================================
// DATATABLE COMMAND HANDLERS
// ====================================================================================

// Helper: find a DataTable by path or name
static UDataTable* FindDataTableByPath(const FString& Path)
{
	if (Path.StartsWith(TEXT("/")))
	{
		return LoadObject<UDataTable>(nullptr, *Path);
	}

	FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> Assets;
	AssetReg.Get().GetAssetsByClass(UDataTable::StaticClass()->GetClassPathName(), Assets);

	for (const FAssetData& Asset : Assets)
	{
		if (Asset.AssetName.ToString() == Path || Asset.GetObjectPathString().Contains(Path))
		{
			return Cast<UDataTable>(Asset.GetAsset());
		}
	}
	return nullptr;
}

void FMCPTcpServer::RegisterDataTableHandlers()
{
	RegisterHandler(TEXT("list_data_tables"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleListDataTables(Params); });
	RegisterHandler(TEXT("read_data_table"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleReadDataTable(Params); });
	RegisterHandler(TEXT("add_data_table_row"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddDataTableRow(Params); });
	RegisterHandler(TEXT("remove_data_table_row"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleRemoveDataTableRow(Params); });
	RegisterHandler(TEXT("edit_data_table_row"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleEditDataTableRow(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleListDataTables(const TSharedPtr<FJsonObject>& Params)
{
	FString PathFilter;
	Params->TryGetStringField(TEXT("path_filter"), PathFilter);

	FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> Assets;
	AssetReg.Get().GetAssetsByClass(UDataTable::StaticClass()->GetClassPathName(), Assets);

	TArray<TSharedPtr<FJsonValue>> DTArray;
	for (const FAssetData& Asset : Assets)
	{
		FString ObjectPath = Asset.GetObjectPathString();
		if (!PathFilter.IsEmpty() && !ObjectPath.Contains(PathFilter))
		{
			continue;
		}

		auto DTObj = MakeShared<FJsonObject>();
		DTObj->SetStringField(TEXT("name"), Asset.AssetName.ToString());
		DTObj->SetStringField(TEXT("path"), ObjectPath);

		// Try to get the row struct name
		UDataTable* DT = Cast<UDataTable>(Asset.GetAsset());
		if (DT && DT->GetRowStruct())
		{
			DTObj->SetStringField(TEXT("row_struct"), DT->GetRowStruct()->GetName());
			DTObj->SetNumberField(TEXT("row_count"), DT->GetRowMap().Num());
		}

		DTArray.Add(MakeShared<FJsonValueObject>(DTObj));
	}

	auto Result = MCPSuccess();
	Result->SetArrayField(TEXT("data_tables"), DTArray);
	Result->SetNumberField(TEXT("count"), DTArray.Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleReadDataTable(const TSharedPtr<FJsonObject>& Params)
{
	FString DTPath;
	if (!Params->TryGetStringField(TEXT("data_table"), DTPath))
	{
		return MCPError(TEXT("Missing required param: data_table"));
	}

	UDataTable* DT = FindDataTableByPath(DTPath);
	if (!DT)
	{
		return MCPError(FString::Printf(TEXT("DataTable not found: %s"), *DTPath));
	}

	const UScriptStruct* RowStruct = DT->GetRowStruct();
	if (!RowStruct)
	{
		return MCPError(TEXT("DataTable has no row struct"));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("data_table"), DT->GetPathName());
	Result->SetStringField(TEXT("row_struct"), RowStruct->GetName());

	// Serialize column info
	TArray<TSharedPtr<FJsonValue>> Columns;
	for (TFieldIterator<FProperty> PropIt(RowStruct); PropIt; ++PropIt)
	{
		FProperty* Prop = *PropIt;
		auto ColObj = MakeShared<FJsonObject>();
		ColObj->SetStringField(TEXT("name"), Prop->GetName());
		ColObj->SetStringField(TEXT("type"), Prop->GetCPPType());
		Columns.Add(MakeShared<FJsonValueObject>(ColObj));
	}
	Result->SetArrayField(TEXT("columns"), Columns);

	// Serialize rows
	TArray<TSharedPtr<FJsonValue>> Rows;
	const TMap<FName, uint8*>& RowMap = DT->GetRowMap();

	FString RowFilter;
	Params->TryGetStringField(TEXT("row_filter"), RowFilter);

	for (auto& Pair : RowMap)
	{
		if (!RowFilter.IsEmpty() && !Pair.Key.ToString().Contains(RowFilter))
		{
			continue;
		}

		auto RowObj = MakeShared<FJsonObject>();
		RowObj->SetStringField(TEXT("row_name"), Pair.Key.ToString());

		const uint8* RowData = Pair.Value;
		for (TFieldIterator<FProperty> PropIt(RowStruct); PropIt; ++PropIt)
		{
			FProperty* Prop = *PropIt;
			FString ValueStr;
			Prop->ExportTextItem_Direct(ValueStr, Prop->ContainerPtrToValuePtr<void>(RowData), nullptr, nullptr, PPF_None);
			RowObj->SetStringField(Prop->GetName(), ValueStr);
		}

		Rows.Add(MakeShared<FJsonValueObject>(RowObj));
	}

	Result->SetArrayField(TEXT("rows"), Rows);
	Result->SetNumberField(TEXT("row_count"), Rows.Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddDataTableRow(const TSharedPtr<FJsonObject>& Params)
{
	FString DTPath;
	if (!Params->TryGetStringField(TEXT("data_table"), DTPath))
	{
		return MCPError(TEXT("Missing required param: data_table"));
	}

	FString RowName;
	if (!Params->TryGetStringField(TEXT("row_name"), RowName))
	{
		return MCPError(TEXT("Missing required param: row_name"));
	}

	UDataTable* DT = FindDataTableByPath(DTPath);
	if (!DT)
	{
		return MCPError(FString::Printf(TEXT("DataTable not found: %s"), *DTPath));
	}

	const UScriptStruct* RowStruct = DT->GetRowStruct();
	if (!RowStruct)
	{
		return MCPError(TEXT("DataTable has no row struct"));
	}

	// Check if row already exists
	if (DT->GetRowMap().Contains(FName(*RowName)))
	{
		return MCPError(FString::Printf(TEXT("Row already exists: %s"), *RowName));
	}

	// Create a new row with default values
	uint8* NewRowData = (uint8*)FMemory::Malloc(RowStruct->GetStructureSize());
	RowStruct->InitializeStruct(NewRowData);

	// Set values from params
	const TSharedPtr<FJsonObject>* ValuesObj;
	if (Params->TryGetObjectField(TEXT("values"), ValuesObj))
	{
		for (TFieldIterator<FProperty> PropIt(RowStruct); PropIt; ++PropIt)
		{
			FProperty* Prop = *PropIt;
			FString Value;
			if ((*ValuesObj)->TryGetStringField(Prop->GetName(), Value))
			{
				void* PropAddr = Prop->ContainerPtrToValuePtr<void>(NewRowData);
				Prop->ImportText_Direct(*Value, PropAddr, nullptr, PPF_None);
			}
		}
	}

	DT->AddRow(FName(*RowName), *reinterpret_cast<FTableRowBase*>(NewRowData));

	RowStruct->DestroyStruct(NewRowData);
	FMemory::Free(NewRowData);

	DT->MarkPackageDirty();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("data_table"), DT->GetPathName());
	Result->SetStringField(TEXT("row_name"), RowName);
	Result->SetNumberField(TEXT("row_count"), DT->GetRowMap().Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleRemoveDataTableRow(const TSharedPtr<FJsonObject>& Params)
{
	FString DTPath;
	if (!Params->TryGetStringField(TEXT("data_table"), DTPath))
	{
		return MCPError(TEXT("Missing required param: data_table"));
	}

	FString RowName;
	if (!Params->TryGetStringField(TEXT("row_name"), RowName))
	{
		return MCPError(TEXT("Missing required param: row_name"));
	}

	UDataTable* DT = FindDataTableByPath(DTPath);
	if (!DT)
	{
		return MCPError(FString::Printf(TEXT("DataTable not found: %s"), *DTPath));
	}

	if (!DT->GetRowMap().Contains(FName(*RowName)))
	{
		return MCPError(FString::Printf(TEXT("Row not found: %s"), *RowName));
	}

	DT->RemoveRow(FName(*RowName));
	DT->MarkPackageDirty();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("data_table"), DT->GetPathName());
	Result->SetStringField(TEXT("removed_row"), RowName);
	Result->SetNumberField(TEXT("row_count"), DT->GetRowMap().Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleEditDataTableRow(const TSharedPtr<FJsonObject>& Params)
{
	FString DTPath;
	if (!Params->TryGetStringField(TEXT("data_table"), DTPath))
	{
		return MCPError(TEXT("Missing required param: data_table"));
	}

	FString RowName;
	if (!Params->TryGetStringField(TEXT("row_name"), RowName))
	{
		return MCPError(TEXT("Missing required param: row_name"));
	}

	UDataTable* DT = FindDataTableByPath(DTPath);
	if (!DT)
	{
		return MCPError(FString::Printf(TEXT("DataTable not found: %s"), *DTPath));
	}

	const UScriptStruct* RowStruct = DT->GetRowStruct();
	if (!RowStruct)
	{
		return MCPError(TEXT("DataTable has no row struct"));
	}

	uint8* RowData = DT->FindRowUnchecked(FName(*RowName));
	if (!RowData)
	{
		return MCPError(FString::Printf(TEXT("Row not found: %s"), *RowName));
	}

	const TSharedPtr<FJsonObject>* ValuesObj;
	if (!Params->TryGetObjectField(TEXT("values"), ValuesObj))
	{
		return MCPError(TEXT("Missing required param: values (object with column name -> value pairs)"));
	}

	TArray<FString> UpdatedFields;
	for (TFieldIterator<FProperty> PropIt(RowStruct); PropIt; ++PropIt)
	{
		FProperty* Prop = *PropIt;
		FString Value;
		if ((*ValuesObj)->TryGetStringField(Prop->GetName(), Value))
		{
			void* PropAddr = Prop->ContainerPtrToValuePtr<void>(RowData);
			Prop->ImportText_Direct(*Value, PropAddr, nullptr, PPF_None);
			UpdatedFields.Add(Prop->GetName());
		}
	}

	DT->MarkPackageDirty();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("data_table"), DT->GetPathName());
	Result->SetStringField(TEXT("row_name"), RowName);
	Result->SetNumberField(TEXT("fields_updated"), UpdatedFields.Num());

	TArray<TSharedPtr<FJsonValue>> FieldsArray;
	for (const FString& Field : UpdatedFields)
	{
		FieldsArray.Add(MakeShared<FJsonValueString>(Field));
	}
	Result->SetArrayField(TEXT("updated_fields"), FieldsArray);

	return Result;
}

// ====================================================================================
// ASSET IMPORT COMMAND HANDLERS
// ====================================================================================

void FMCPTcpServer::RegisterAssetHandlers()
{
	RegisterHandler(TEXT("import_asset"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleImportAsset(Params); });
	RegisterHandler(TEXT("create_material_instance"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleCreateMaterialInstance(Params); });
	RegisterHandler(TEXT("get_asset_property"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleGetAssetProperty(Params); });
	RegisterHandler(TEXT("set_asset_property"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSetAssetProperty(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleImportAsset(const TSharedPtr<FJsonObject>& Params)
{
	FString FilePath;
	if (!Params->TryGetStringField(TEXT("file_path"), FilePath))
	{
		return MCPError(TEXT("Missing required param: file_path (absolute path to file on disk)"));
	}

	FString DestPath = TEXT("/Game/Imported/");
	Params->TryGetStringField(TEXT("dest_path"), DestPath);

	if (!FPaths::FileExists(FilePath))
	{
		return MCPError(FString::Printf(TEXT("File not found: %s"), *FilePath));
	}

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	TArray<FString> Files;
	Files.Add(FilePath);

	TArray<UObject*> ImportedAssets = AssetTools.ImportAssets(Files, DestPath);

	if (ImportedAssets.Num() == 0)
	{
		return MCPError(TEXT("Import failed — no assets were created. Check that the file type is supported."));
	}

	TArray<TSharedPtr<FJsonValue>> AssetArray;
	for (UObject* Asset : ImportedAssets)
	{
		auto AssetObj = MakeShared<FJsonObject>();
		AssetObj->SetStringField(TEXT("name"), Asset->GetName());
		AssetObj->SetStringField(TEXT("path"), Asset->GetPathName());
		AssetObj->SetStringField(TEXT("class"), Asset->GetClass()->GetName());
		AssetArray.Add(MakeShared<FJsonValueObject>(AssetObj));
	}

	auto Result = MCPSuccess();
	Result->SetArrayField(TEXT("imported"), AssetArray);
	Result->SetNumberField(TEXT("count"), AssetArray.Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleCreateMaterialInstance(const TSharedPtr<FJsonObject>& Params)
{
	FString ParentPath;
	if (!Params->TryGetStringField(TEXT("parent_material"), ParentPath))
	{
		return MCPError(TEXT("Missing required param: parent_material (path to parent material or material instance)"));
	}

	FString InstanceName;
	if (!Params->TryGetStringField(TEXT("name"), InstanceName))
	{
		return MCPError(TEXT("Missing required param: name"));
	}

	FString DestPath = TEXT("/Game/Materials/");
	Params->TryGetStringField(TEXT("path"), DestPath);

	UMaterialInterface* ParentMat = LoadObject<UMaterialInterface>(nullptr, *ParentPath);
	if (!ParentMat)
	{
		return MCPError(FString::Printf(TEXT("Parent material not found: %s"), *ParentPath));
	}

	FString FullPath = DestPath / InstanceName;
	UPackage* Package = CreatePackage(*FullPath);

	UMaterialInstanceConstant* NewMI = NewObject<UMaterialInstanceConstant>(Package, FName(*InstanceName), RF_Standalone | RF_Public);
	NewMI->Parent = ParentMat;

	FAssetRegistryModule::AssetCreated(NewMI);
	Package->MarkPackageDirty();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("name"), InstanceName);
	Result->SetStringField(TEXT("path"), NewMI->GetPathName());
	Result->SetStringField(TEXT("parent"), ParentMat->GetPathName());
	return Result;
}

// ============================================================================
// Generic Asset Property Handlers
// ============================================================================

TSharedPtr<FJsonObject> FMCPTcpServer::HandleGetAssetProperty(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return MCPError(TEXT("Missing required param: asset_path (e.g. '/Game/Audio/SA_Wisp3D.SA_Wisp3D')"));
	}

	FString PropertyName;
	if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
	{
		return MCPError(TEXT("Missing required param: property_name"));
	}

	UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
	if (!Asset)
	{
		return MCPError(FString::Printf(TEXT("Asset not found: %s"), *AssetPath));
	}

	// Support nested property access with "." syntax (e.g. "Attenuation.FalloffDistance")
	UObject* TargetObject = Asset;
	FString ActualPropertyName = PropertyName;
	void* StructContainer = nullptr;
	UStruct* StructClass = nullptr;

	// Walk dot-separated path for nested struct properties
	TArray<FString> PathParts;
	PropertyName.ParseIntoArray(PathParts, TEXT("."));

	if (PathParts.Num() > 1)
	{
		ActualPropertyName = PathParts.Last();
		StructClass = Asset->GetClass();
		void* CurrentContainer = Asset;

		for (int32 i = 0; i < PathParts.Num() - 1; ++i)
		{
			FProperty* Prop = StructClass->FindPropertyByName(FName(*PathParts[i]));
			if (!Prop)
			{
				return MCPError(FString::Printf(TEXT("Property '%s' not found on %s"), *PathParts[i], *StructClass->GetName()));
			}

			if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
			{
				CurrentContainer = Prop->ContainerPtrToValuePtr<void>(CurrentContainer);
				StructClass = StructProp->Struct;
			}
			else
			{
				return MCPError(FString::Printf(TEXT("Property '%s' is not a struct, cannot traverse further"), *PathParts[i]));
			}
		}

		StructContainer = CurrentContainer;
	}

	// Find the final property
	FProperty* Prop;
	const void* ValuePtr;
	UObject* ExportObj = Asset;

	if (StructContainer)
	{
		Prop = StructClass->FindPropertyByName(FName(*ActualPropertyName));
		if (!Prop)
		{
			return MCPError(FString::Printf(TEXT("Property '%s' not found in struct %s"), *ActualPropertyName, *StructClass->GetName()));
		}
		ValuePtr = Prop->ContainerPtrToValuePtr<void>(StructContainer);
	}
	else
	{
		Prop = Asset->GetClass()->FindPropertyByName(FName(*ActualPropertyName));
		if (!Prop)
		{
			// Try listing available properties to help
			FString AvailableProps;
			int32 Count = 0;
			for (TFieldIterator<FProperty> It(Asset->GetClass()); It; ++It)
			{
				if (Count > 0) AvailableProps += TEXT(", ");
				AvailableProps += It->GetName();
				if (++Count >= 30) { AvailableProps += TEXT("..."); break; }
			}
			return MCPError(FString::Printf(TEXT("Property '%s' not found on %s. Available: %s"),
				*ActualPropertyName, *Asset->GetClass()->GetName(), *AvailableProps));
		}
		ValuePtr = Prop->ContainerPtrToValuePtr<void>(Asset);
	}

	FString ValueStr;
	Prop->ExportTextItem_Direct(ValueStr, ValuePtr, nullptr, ExportObj, PPF_None);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("asset"), AssetPath);
	Result->SetStringField(TEXT("property"), PropertyName);
	Result->SetStringField(TEXT("value"), ValueStr);
	Result->SetStringField(TEXT("type"), Prop->GetCPPType());

	// If property is a struct, also list its sub-properties
	if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
	{
		TArray<TSharedPtr<FJsonValue>> SubProps;
		for (TFieldIterator<FProperty> It(StructProp->Struct); It; ++It)
		{
			auto SubPropObj = MakeShared<FJsonObject>();
			SubPropObj->SetStringField(TEXT("name"), It->GetName());
			SubPropObj->SetStringField(TEXT("type"), It->GetCPPType());
			SubProps.Add(MakeShared<FJsonValueObject>(SubPropObj));
		}
		Result->SetArrayField(TEXT("sub_properties"), SubProps);
	}

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSetAssetProperty(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return MCPError(TEXT("Missing required param: asset_path"));
	}

	FString PropertyName;
	if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
	{
		return MCPError(TEXT("Missing required param: property_name"));
	}

	FString Value;
	if (!Params->TryGetStringField(TEXT("value"), Value))
	{
		return MCPError(TEXT("Missing required param: value (UE5 text format)"));
	}

	UObject* Asset = LoadObject<UObject>(nullptr, *AssetPath);
	if (!Asset)
	{
		return MCPError(FString::Printf(TEXT("Asset not found: %s"), *AssetPath));
	}

	// Walk dot-separated path for nested struct properties
	FString ActualPropertyName = PropertyName;
	void* StructContainer = nullptr;
	UStruct* StructClass = nullptr;

	TArray<FString> PathParts;
	PropertyName.ParseIntoArray(PathParts, TEXT("."));

	if (PathParts.Num() > 1)
	{
		ActualPropertyName = PathParts.Last();
		StructClass = Asset->GetClass();
		void* CurrentContainer = Asset;

		for (int32 i = 0; i < PathParts.Num() - 1; ++i)
		{
			FProperty* Prop = StructClass->FindPropertyByName(FName(*PathParts[i]));
			if (!Prop)
			{
				return MCPError(FString::Printf(TEXT("Property '%s' not found on %s"), *PathParts[i], *StructClass->GetName()));
			}

			if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
			{
				CurrentContainer = Prop->ContainerPtrToValuePtr<void>(CurrentContainer);
				StructClass = StructProp->Struct;
			}
			else
			{
				return MCPError(FString::Printf(TEXT("Property '%s' is not a struct, cannot traverse further"), *PathParts[i]));
			}
		}

		StructContainer = CurrentContainer;
	}

	// Find the final property
	FProperty* Prop;
	void* ValPtr;

	if (StructContainer)
	{
		Prop = StructClass->FindPropertyByName(FName(*ActualPropertyName));
		if (!Prop)
		{
			return MCPError(FString::Printf(TEXT("Property '%s' not found in struct %s"), *ActualPropertyName, *StructClass->GetName()));
		}
		ValPtr = Prop->ContainerPtrToValuePtr<void>(StructContainer);
	}
	else
	{
		Prop = Asset->GetClass()->FindPropertyByName(FName(*ActualPropertyName));
		if (!Prop)
		{
			return MCPError(FString::Printf(TEXT("Property '%s' not found on %s"), *ActualPropertyName, *Asset->GetClass()->GetName()));
		}
		ValPtr = Prop->ContainerPtrToValuePtr<void>(Asset);
	}

	Asset->PreEditChange(Prop);

	const TCHAR* ValueStream = *Value;
	Prop->ImportText_Direct(ValueStream, ValPtr, Asset, PPF_None);

	FPropertyChangedEvent ChangeEvent(Prop);
	Asset->PostEditChangeProperty(ChangeEvent);
	Asset->MarkPackageDirty();

	// Read back the value to confirm
	FString NewValueStr;
	const void* ReadPtr = Prop->ContainerPtrToValuePtr<void>(StructContainer ? StructContainer : (void*)Asset);
	Prop->ExportTextItem_Direct(NewValueStr, ReadPtr, nullptr, Asset, PPF_None);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("asset"), AssetPath);
	Result->SetStringField(TEXT("property"), PropertyName);
	Result->SetStringField(TEXT("new_value"), NewValueStr);
	return Result;
}

#if WITH_NIAGARA
// ============================================================================
// Niagara Handlers
// ============================================================================

void FMCPTcpServer::RegisterNiagaraHandlers()
{
	RegisterHandler(TEXT("list_niagara_systems"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleListNiagaraSystems(Params); });
	RegisterHandler(TEXT("read_niagara_system"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleReadNiagaraSystem(Params); });
	RegisterHandler(TEXT("get_niagara_emitter_properties"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleGetNiagaraEmitterProperties(Params); });
	RegisterHandler(TEXT("set_niagara_parameter"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSetNiagaraParameter(Params); });
	RegisterHandler(TEXT("set_niagara_emitter_enabled"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSetNiagaraEmitterEnabled(Params); });
	RegisterHandler(TEXT("get_niagara_emitter_modules"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleGetNiagaraEmitterModules(Params); });
	RegisterHandler(TEXT("set_niagara_module_input"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSetNiagaraModuleInput(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleListNiagaraSystems(const TSharedPtr<FJsonObject>& Params)
{
	FString PathFilter;
	Params->TryGetStringField(TEXT("path_filter"), PathFilter);

	FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> Assets;
	AssetReg.Get().GetAssetsByClass(UNiagaraSystem::StaticClass()->GetClassPathName(), Assets);

	TArray<TSharedPtr<FJsonValue>> SystemArray;
	for (const FAssetData& Asset : Assets)
	{
		FString AssetPath = Asset.GetObjectPathString();
		if (!PathFilter.IsEmpty() && !AssetPath.Contains(PathFilter))
		{
			continue;
		}

		auto SysObj = MakeShared<FJsonObject>();
		SysObj->SetStringField(TEXT("name"), Asset.AssetName.ToString());
		SysObj->SetStringField(TEXT("path"), AssetPath);

		// Load to get emitter count
		UNiagaraSystem* System = Cast<UNiagaraSystem>(Asset.GetAsset());
		if (System)
		{
			SysObj->SetNumberField(TEXT("emitter_count"), System->GetEmitterHandles().Num());
		}

		SystemArray.Add(MakeShared<FJsonValueObject>(SysObj));
	}

	auto Result = MCPSuccess();
	Result->SetArrayField(TEXT("systems"), SystemArray);
	Result->SetNumberField(TEXT("count"), SystemArray.Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleReadNiagaraSystem(const TSharedPtr<FJsonObject>& Params)
{
	FString SystemPath;
	if (!Params->TryGetStringField(TEXT("system"), SystemPath))
	{
		return MCPError(TEXT("Missing required param: system (name or path)"));
	}

	// Try loading by path first, then search by name
	UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
	if (!System)
	{
		// Search by name in asset registry
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByClass(UNiagaraSystem::StaticClass()->GetClassPathName(), Assets);
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == SystemPath)
			{
				System = Cast<UNiagaraSystem>(Asset.GetAsset());
				break;
			}
		}
	}

	if (!System)
	{
		return MCPError(FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("name"), System->GetName());
	Result->SetStringField(TEXT("path"), System->GetPathName());

	// Emitter handles
	TArray<TSharedPtr<FJsonValue>> EmitterArray;
	const TArray<FNiagaraEmitterHandle>& Handles = System->GetEmitterHandles();
	for (int32 i = 0; i < Handles.Num(); i++)
	{
		const FNiagaraEmitterHandle& Handle = Handles[i];
		auto EmitterObj = MakeShared<FJsonObject>();
		EmitterObj->SetNumberField(TEXT("index"), i);
		EmitterObj->SetStringField(TEXT("name"), Handle.GetName().ToString());
		EmitterObj->SetStringField(TEXT("unique_name"), Handle.GetUniqueInstanceName());
		EmitterObj->SetBoolField(TEXT("enabled"), Handle.GetIsEnabled());

		FVersionedNiagaraEmitterData* EmitterData = Handle.GetEmitterData();
		if (EmitterData)
		{
			EmitterObj->SetStringField(TEXT("sim_target"),
				EmitterData->SimTarget == ENiagaraSimTarget::CPUSim ? TEXT("CPU") : TEXT("GPU"));
		}

		EmitterArray.Add(MakeShared<FJsonValueObject>(EmitterObj));
	}
	Result->SetArrayField(TEXT("emitters"), EmitterArray);

	// System exposed parameters (user parameters)
	TArray<TSharedPtr<FJsonValue>> ParamArray;
	const FNiagaraUserRedirectionParameterStore& UserParams = System->GetExposedParameters();
	TArray<FNiagaraVariable> SortedParams;
	UserParams.GetParameters(SortedParams);
	for (const FNiagaraVariable& Param : SortedParams)
	{
		auto ParamObj = MakeShared<FJsonObject>();
		ParamObj->SetStringField(TEXT("name"), Param.GetName().ToString());
		ParamObj->SetStringField(TEXT("type"), Param.GetType().GetName());
		ParamArray.Add(MakeShared<FJsonValueObject>(ParamObj));
	}
	Result->SetArrayField(TEXT("user_parameters"), ParamArray);

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleGetNiagaraEmitterProperties(const TSharedPtr<FJsonObject>& Params)
{
	FString SystemPath;
	if (!Params->TryGetStringField(TEXT("system"), SystemPath))
	{
		return MCPError(TEXT("Missing required param: system"));
	}

	FString EmitterName;
	int32 EmitterIndex = -1;
	Params->TryGetStringField(TEXT("emitter_name"), EmitterName);
	if (Params->HasField(TEXT("emitter_index")))
	{
		EmitterIndex = (int32)Params->GetNumberField(TEXT("emitter_index"));
	}

	UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
	if (!System)
	{
		// Search by name
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByClass(UNiagaraSystem::StaticClass()->GetClassPathName(), Assets);
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == SystemPath)
			{
				System = Cast<UNiagaraSystem>(Asset.GetAsset());
				break;
			}
		}
	}
	if (!System)
	{
		return MCPError(FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath));
	}

	const TArray<FNiagaraEmitterHandle>& Handles = System->GetEmitterHandles();

	// Find the emitter by name or index
	int32 FoundIndex = -1;
	if (EmitterIndex >= 0 && EmitterIndex < Handles.Num())
	{
		FoundIndex = EmitterIndex;
	}
	else if (!EmitterName.IsEmpty())
	{
		for (int32 i = 0; i < Handles.Num(); i++)
		{
			if (Handles[i].GetName().ToString() == EmitterName)
			{
				FoundIndex = i;
				break;
			}
		}
	}

	if (FoundIndex < 0)
	{
		return MCPError(TEXT("Emitter not found. Provide emitter_name or emitter_index."));
	}

	const FNiagaraEmitterHandle& Handle = Handles[FoundIndex];
	FVersionedNiagaraEmitterData* EmitterData = Handle.GetEmitterData();
	if (!EmitterData)
	{
		return MCPError(TEXT("Failed to get emitter data."));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("name"), Handle.GetName().ToString());
	Result->SetNumberField(TEXT("index"), FoundIndex);
	Result->SetBoolField(TEXT("enabled"), Handle.GetIsEnabled());
	Result->SetStringField(TEXT("sim_target"),
		EmitterData->SimTarget == ENiagaraSimTarget::CPUSim ? TEXT("CPU") : TEXT("GPU"));

	// Spawn rate / properties from scripts
	TArray<TSharedPtr<FJsonValue>> ScriptArray;
	auto AddScript = [&](const FString& Purpose, UNiagaraScript* Script)
	{
		if (!Script) return;
		auto ScriptObj = MakeShared<FJsonObject>();
		ScriptObj->SetStringField(TEXT("purpose"), Purpose);
		ScriptObj->SetStringField(TEXT("name"), Script->GetName());
		ScriptArray.Add(MakeShared<FJsonValueObject>(ScriptObj));
	};

	AddScript(TEXT("SpawnScript"), EmitterData->SpawnScriptProps.Script);
	AddScript(TEXT("UpdateScript"), EmitterData->UpdateScriptProps.Script);

	Result->SetArrayField(TEXT("scripts"), ScriptArray);

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSetNiagaraParameter(const TSharedPtr<FJsonObject>& Params)
{
	FString SystemPath;
	if (!Params->TryGetStringField(TEXT("system"), SystemPath))
	{
		return MCPError(TEXT("Missing required param: system"));
	}

	FString ParamName;
	if (!Params->TryGetStringField(TEXT("parameter_name"), ParamName))
	{
		return MCPError(TEXT("Missing required param: parameter_name"));
	}

	UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
	if (!System)
	{
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByClass(UNiagaraSystem::StaticClass()->GetClassPathName(), Assets);
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == SystemPath)
			{
				System = Cast<UNiagaraSystem>(Asset.GetAsset());
				break;
			}
		}
	}
	if (!System)
	{
		return MCPError(FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath));
	}

	FNiagaraUserRedirectionParameterStore& UserParams = System->GetExposedParameters();

	// Find the parameter
	TArray<FNiagaraVariable> AllParams;
	UserParams.GetParameters(AllParams);

	FNiagaraVariable FoundVar;
	bool bFound = false;
	for (const FNiagaraVariable& P : AllParams)
	{
		if (P.GetName().ToString() == ParamName)
		{
			FoundVar = P;
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		return MCPError(FString::Printf(TEXT("Parameter '%s' not found in system user parameters."), *ParamName));
	}

	// Set value based on type
	FString TypeName = FoundVar.GetType().GetName();
	System->Modify();

	if (TypeName == TEXT("float") || TypeName == TEXT("Float"))
	{
		double Value = Params->GetNumberField(TEXT("value"));
		float FloatVal = (float)Value;
		UserParams.SetParameterValue(FloatVal, FoundVar);
	}
	else if (TypeName == TEXT("int32") || TypeName == TEXT("Int32"))
	{
		int32 Value = (int32)Params->GetNumberField(TEXT("value"));
		UserParams.SetParameterValue(Value, FoundVar);
	}
	else if (TypeName == TEXT("bool") || TypeName == TEXT("Bool"))
	{
		// Niagara uses FNiagaraBool which stores as int32
		int32 Value = Params->GetBoolField(TEXT("value")) ? INDEX_NONE : 0;
		FNiagaraBool NiagaraBool(Params->GetBoolField(TEXT("value")));
		UserParams.SetParameterValue(NiagaraBool, FoundVar);
	}
	else if (TypeName.Contains(TEXT("Vector")))
	{
		const TSharedPtr<FJsonObject>* ValueObj;
		if (Params->TryGetObjectField(TEXT("value"), ValueObj))
		{
			FVector Vec;
			Vec.X = (*ValueObj)->GetNumberField(TEXT("x"));
			Vec.Y = (*ValueObj)->GetNumberField(TEXT("y"));
			Vec.Z = (*ValueObj)->GetNumberField(TEXT("z"));
			UserParams.SetParameterValue(Vec, FoundVar);
		}
		else
		{
			return MCPError(TEXT("Vector parameter requires value as {x, y, z} object."));
		}
	}
	else if (TypeName.Contains(TEXT("Color")) || TypeName.Contains(TEXT("LinearColor")))
	{
		const TSharedPtr<FJsonObject>* ValueObj;
		if (Params->TryGetObjectField(TEXT("value"), ValueObj))
		{
			FLinearColor Color;
			Color.R = (float)(*ValueObj)->GetNumberField(TEXT("r"));
			Color.G = (float)(*ValueObj)->GetNumberField(TEXT("g"));
			Color.B = (float)(*ValueObj)->GetNumberField(TEXT("b"));
			Color.A = (*ValueObj)->HasField(TEXT("a")) ? (float)(*ValueObj)->GetNumberField(TEXT("a")) : 1.0f;
			UserParams.SetParameterValue(Color, FoundVar);
		}
		else
		{
			return MCPError(TEXT("Color parameter requires value as {r, g, b, a} object."));
		}
	}
	else
	{
		return MCPError(FString::Printf(TEXT("Unsupported parameter type: %s. Supported: float, int32, bool, Vector, LinearColor"), *TypeName));
	}

	System->MarkPackageDirty();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("parameter"), ParamName);
	Result->SetStringField(TEXT("type"), TypeName);
	Result->SetStringField(TEXT("system"), System->GetName());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSetNiagaraEmitterEnabled(const TSharedPtr<FJsonObject>& Params)
{
	FString SystemPath;
	if (!Params->TryGetStringField(TEXT("system"), SystemPath))
	{
		return MCPError(TEXT("Missing required param: system"));
	}

	bool bEnabled = true;
	if (Params->HasField(TEXT("enabled")))
	{
		bEnabled = Params->GetBoolField(TEXT("enabled"));
	}

	FString EmitterName;
	int32 EmitterIndex = -1;
	Params->TryGetStringField(TEXT("emitter_name"), EmitterName);
	if (Params->HasField(TEXT("emitter_index")))
	{
		EmitterIndex = (int32)Params->GetNumberField(TEXT("emitter_index"));
	}

	UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
	if (!System)
	{
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByClass(UNiagaraSystem::StaticClass()->GetClassPathName(), Assets);
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == SystemPath)
			{
				System = Cast<UNiagaraSystem>(Asset.GetAsset());
				break;
			}
		}
	}
	if (!System)
	{
		return MCPError(FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath));
	}

	TArray<FNiagaraEmitterHandle>& Handles = System->GetEmitterHandles();

	int32 FoundIndex = -1;
	if (EmitterIndex >= 0 && EmitterIndex < Handles.Num())
	{
		FoundIndex = EmitterIndex;
	}
	else if (!EmitterName.IsEmpty())
	{
		for (int32 i = 0; i < Handles.Num(); i++)
		{
			if (Handles[i].GetName().ToString() == EmitterName)
			{
				FoundIndex = i;
				break;
			}
		}
	}

	if (FoundIndex < 0)
	{
		return MCPError(TEXT("Emitter not found. Provide emitter_name or emitter_index."));
	}

	System->Modify();
	Handles[FoundIndex].SetIsEnabled(bEnabled, *System, false);
	System->MarkPackageDirty();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("emitter"), Handles[FoundIndex].GetName().ToString());
	Result->SetBoolField(TEXT("enabled"), bEnabled);
	Result->SetStringField(TEXT("system"), System->GetName());
	return Result;
}

// Helper: find a Niagara system by name or path
static UNiagaraSystem* FindNiagaraSystem(const FString& SystemPath)
{
	UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, *SystemPath);
	if (!System)
	{
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByClass(UNiagaraSystem::StaticClass()->GetClassPathName(), Assets);
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == SystemPath)
			{
				System = Cast<UNiagaraSystem>(Asset.GetAsset());
				break;
			}
		}
	}
	return System;
}

// Helper: find an emitter handle by name or index
static int32 FindEmitterIndex(const TArray<FNiagaraEmitterHandle>& Handles, const TSharedPtr<FJsonObject>& Params)
{
	FString EmitterName;
	Params->TryGetStringField(TEXT("emitter_name"), EmitterName);
	int32 EmitterIndex = -1;
	if (Params->HasField(TEXT("emitter_index")))
	{
		EmitterIndex = (int32)Params->GetNumberField(TEXT("emitter_index"));
	}

	if (EmitterIndex >= 0 && EmitterIndex < Handles.Num())
	{
		return EmitterIndex;
	}
	if (!EmitterName.IsEmpty())
	{
		for (int32 i = 0; i < Handles.Num(); i++)
		{
			if (Handles[i].GetName().ToString() == EmitterName)
			{
				return i;
			}
		}
	}
	return -1;
}

// Helper: serialize an RI parameter value to JSON
static void SerializeRIParamValue(const FNiagaraParameterStore& Store, const FNiagaraVariable& Param, TSharedRef<FJsonObject> OutObj)
{
	FString TypeName = Param.GetType().GetName();
	const uint8* Data = Store.GetParameterData(Param);
	if (!Data) return;

	if (TypeName == TEXT("float") || TypeName == TEXT("Float"))
	{
		OutObj->SetNumberField(TEXT("value"), *(const float*)Data);
	}
	else if (TypeName == TEXT("int32") || TypeName == TEXT("Int32"))
	{
		OutObj->SetNumberField(TEXT("value"), *(const int32*)Data);
	}
	else if (TypeName == TEXT("bool") || TypeName == TEXT("Bool") || TypeName == TEXT("NiagaraBool"))
	{
		FNiagaraBool BoolVal = *(const FNiagaraBool*)Data;
		OutObj->SetBoolField(TEXT("value"), BoolVal.GetValue());
	}
	else if (TypeName.Contains(TEXT("Vector")))
	{
		const FVector3f* Vec = (const FVector3f*)Data;
		auto VecObj = MakeShared<FJsonObject>();
		VecObj->SetNumberField(TEXT("x"), Vec->X);
		VecObj->SetNumberField(TEXT("y"), Vec->Y);
		VecObj->SetNumberField(TEXT("z"), Vec->Z);
		OutObj->SetObjectField(TEXT("value"), VecObj);
	}
	else if (TypeName.Contains(TEXT("Color")) || TypeName.Contains(TEXT("LinearColor")))
	{
		const FLinearColor* Color = (const FLinearColor*)Data;
		auto ColObj = MakeShared<FJsonObject>();
		ColObj->SetNumberField(TEXT("r"), Color->R);
		ColObj->SetNumberField(TEXT("g"), Color->G);
		ColObj->SetNumberField(TEXT("b"), Color->B);
		ColObj->SetNumberField(TEXT("a"), Color->A);
		OutObj->SetObjectField(TEXT("value"), ColObj);
	}
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleGetNiagaraEmitterModules(const TSharedPtr<FJsonObject>& Params)
{
	FString SystemPath;
	if (!Params->TryGetStringField(TEXT("system"), SystemPath))
	{
		return MCPError(TEXT("Missing required param: system"));
	}

	UNiagaraSystem* System = FindNiagaraSystem(SystemPath);
	if (!System)
	{
		return MCPError(FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath));
	}

	const TArray<FNiagaraEmitterHandle>& Handles = System->GetEmitterHandles();
	int32 FoundIndex = FindEmitterIndex(Handles, Params);
	if (FoundIndex < 0)
	{
		return MCPError(TEXT("Emitter not found. Provide emitter_name or emitter_index."));
	}

	const FNiagaraEmitterHandle& Handle = Handles[FoundIndex];
	FVersionedNiagaraEmitterData* EmitterData = Handle.GetEmitterData();
	if (!EmitterData)
	{
		return MCPError(TEXT("Failed to get emitter data."));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("emitter"), Handle.GetName().ToString());
	Result->SetNumberField(TEXT("index"), FoundIndex);

	// Get modules from the graph
	TArray<TSharedPtr<FJsonValue>> ModuleArray;
	UNiagaraScriptSource* Source = Cast<UNiagaraScriptSource>(EmitterData->GraphSource);
	if (Source && Source->NodeGraph)
	{
		TArray<UNiagaraNodeFunctionCall*> FuncNodes;
		Source->NodeGraph->GetNodesOfClass(FuncNodes);

		for (UNiagaraNodeFunctionCall* Node : FuncNodes)
		{
			auto ModObj = MakeShared<FJsonObject>();
			ModObj->SetStringField(TEXT("name"), Node->GetFunctionName());
			ModObj->SetStringField(TEXT("node_name"), Node->GetName());

			// Determine script usage from connected output node
			FString Usage = TEXT("Unknown");
			for (UEdGraphPin* Pin : Node->Pins)
			{
				if (Pin->Direction == EGPD_Output)
				{
					for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
					{
						if (UNiagaraNodeOutput* OutputNode = Cast<UNiagaraNodeOutput>(LinkedPin->GetOwningNode()))
						{
							switch (OutputNode->GetUsage())
							{
							case ENiagaraScriptUsage::EmitterSpawnScript: Usage = TEXT("EmitterSpawn"); break;
							case ENiagaraScriptUsage::EmitterUpdateScript: Usage = TEXT("EmitterUpdate"); break;
							case ENiagaraScriptUsage::ParticleSpawnScript: Usage = TEXT("ParticleSpawn"); break;
							case ENiagaraScriptUsage::ParticleUpdateScript: Usage = TEXT("ParticleUpdate"); break;
							default: break;
							}
						}
						// Also check if linked to another function call (chained modules)
						if (UNiagaraNodeFunctionCall* ChainedNode = Cast<UNiagaraNodeFunctionCall>(LinkedPin->GetOwningNode()))
						{
							// Inherit usage from the chain
						}
					}
				}
			}
			ModObj->SetStringField(TEXT("usage"), Usage);

			// Get input pins
			TArray<TSharedPtr<FJsonValue>> InputArray;
			for (UEdGraphPin* Pin : Node->Pins)
			{
				if (Pin->Direction == EGPD_Input && !Pin->bHidden)
				{
					auto InObj = MakeShared<FJsonObject>();
					InObj->SetStringField(TEXT("name"), Pin->GetName());
					InObj->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
					if (!Pin->DefaultValue.IsEmpty())
					{
						InObj->SetStringField(TEXT("default"), Pin->DefaultValue);
					}
					InputArray.Add(MakeShared<FJsonValueObject>(InObj));
				}
			}
			if (InputArray.Num() > 0)
			{
				ModObj->SetArrayField(TEXT("inputs"), InputArray);
			}

			ModuleArray.Add(MakeShared<FJsonValueObject>(ModObj));
		}
	}
	Result->SetArrayField(TEXT("modules"), ModuleArray);

	// Read rapid iteration parameters from all scripts
	TArray<TSharedPtr<FJsonValue>> RIParamArray;

	auto ReadRIParams = [&](UNiagaraScript* Script, const FString& ScriptLabel)
	{
		if (!Script) return;
		const FNiagaraParameterStore& RIStore = Script->RapidIterationParameters;
		TArray<FNiagaraVariable> RIVars;
		RIStore.GetParameters(RIVars);

		for (const FNiagaraVariable& Var : RIVars)
		{
			auto PObj = MakeShared<FJsonObject>();
			PObj->SetStringField(TEXT("name"), Var.GetName().ToString());
			PObj->SetStringField(TEXT("type"), Var.GetType().GetName());
			PObj->SetStringField(TEXT("script"), ScriptLabel);
			SerializeRIParamValue(RIStore, Var, PObj);
			RIParamArray.Add(MakeShared<FJsonValueObject>(PObj));
		}
	};

	ReadRIParams(EmitterData->SpawnScriptProps.Script, TEXT("Spawn"));
	ReadRIParams(EmitterData->UpdateScriptProps.Script, TEXT("Update"));

	// Also check emitter spawn/update scripts if available
	if (EmitterData->EmitterSpawnScriptProps.Script)
	{
		ReadRIParams(EmitterData->EmitterSpawnScriptProps.Script, TEXT("EmitterSpawn"));
	}
	if (EmitterData->EmitterUpdateScriptProps.Script)
	{
		ReadRIParams(EmitterData->EmitterUpdateScriptProps.Script, TEXT("EmitterUpdate"));
	}

	Result->SetArrayField(TEXT("rapid_iteration_parameters"), RIParamArray);

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSetNiagaraModuleInput(const TSharedPtr<FJsonObject>& Params)
{
	FString SystemPath;
	if (!Params->TryGetStringField(TEXT("system"), SystemPath))
	{
		return MCPError(TEXT("Missing required param: system"));
	}

	FString ParamName;
	if (!Params->TryGetStringField(TEXT("parameter_name"), ParamName))
	{
		return MCPError(TEXT("Missing required param: parameter_name"));
	}

	if (!Params->HasField(TEXT("value")))
	{
		return MCPError(TEXT("Missing required param: value"));
	}

	UNiagaraSystem* System = FindNiagaraSystem(SystemPath);
	if (!System)
	{
		return MCPError(FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath));
	}

	const TArray<FNiagaraEmitterHandle>& Handles = System->GetEmitterHandles();
	int32 FoundIndex = FindEmitterIndex(Handles, Params);
	if (FoundIndex < 0)
	{
		return MCPError(TEXT("Emitter not found. Provide emitter_name or emitter_index."));
	}

	const FNiagaraEmitterHandle& Handle = Handles[FoundIndex];
	FVersionedNiagaraEmitterData* EmitterData = Handle.GetEmitterData();
	if (!EmitterData)
	{
		return MCPError(TEXT("Failed to get emitter data."));
	}

	// Search for the RI parameter across all scripts
	struct FScriptEntry
	{
		UNiagaraScript* Script;
		FString Label;
	};
	TArray<FScriptEntry> Scripts;
	Scripts.Add({EmitterData->SpawnScriptProps.Script, TEXT("Spawn")});
	Scripts.Add({EmitterData->UpdateScriptProps.Script, TEXT("Update")});
	if (EmitterData->EmitterSpawnScriptProps.Script)
		Scripts.Add({EmitterData->EmitterSpawnScriptProps.Script, TEXT("EmitterSpawn")});
	if (EmitterData->EmitterUpdateScriptProps.Script)
		Scripts.Add({EmitterData->EmitterUpdateScriptProps.Script, TEXT("EmitterUpdate")});

	// Optional: restrict to a specific script
	FString ScriptFilter;
	Params->TryGetStringField(TEXT("script"), ScriptFilter);

	UNiagaraScript* TargetScript = nullptr;
	FNiagaraVariable FoundVar;
	FString FoundScriptLabel;

	for (const FScriptEntry& Entry : Scripts)
	{
		if (!Entry.Script) continue;
		if (!ScriptFilter.IsEmpty() && !Entry.Label.Contains(ScriptFilter)) continue;

		FNiagaraParameterStore& RIStore = Entry.Script->RapidIterationParameters;
		TArray<FNiagaraVariable> RIVars;
		RIStore.GetParameters(RIVars);

		for (const FNiagaraVariable& Var : RIVars)
		{
			if (Var.GetName().ToString().Contains(ParamName))
			{
				FoundVar = Var;
				TargetScript = Entry.Script;
				FoundScriptLabel = Entry.Label;
				break;
			}
		}
		if (TargetScript) break;
	}

	if (!TargetScript)
	{
		// List available params for helpful error
		TArray<FString> Available;
		for (const FScriptEntry& Entry : Scripts)
		{
			if (!Entry.Script) continue;
			TArray<FNiagaraVariable> RIVars;
			Entry.Script->RapidIterationParameters.GetParameters(RIVars);
			for (const FNiagaraVariable& Var : RIVars)
			{
				Available.Add(FString::Printf(TEXT("[%s] %s (%s)"), *Entry.Label, *Var.GetName().ToString(), *Var.GetType().GetName()));
			}
		}
		FString AvailList = FString::Join(Available, TEXT("\n  "));
		return MCPError(FString::Printf(TEXT("RI parameter '%s' not found. Available:\n  %s"), *ParamName, *AvailList));
	}

	System->Modify();

	FNiagaraParameterStore& RIStore = TargetScript->RapidIterationParameters;
	FString TypeName = FoundVar.GetType().GetName();

	if (TypeName == TEXT("float") || TypeName == TEXT("Float"))
	{
		float Value = (float)Params->GetNumberField(TEXT("value"));
		RIStore.SetParameterValue(Value, FoundVar);
	}
	else if (TypeName == TEXT("int32") || TypeName == TEXT("Int32"))
	{
		int32 Value = (int32)Params->GetNumberField(TEXT("value"));
		RIStore.SetParameterValue(Value, FoundVar);
	}
	else if (TypeName == TEXT("bool") || TypeName == TEXT("Bool") || TypeName == TEXT("NiagaraBool"))
	{
		FNiagaraBool Value(Params->GetBoolField(TEXT("value")));
		RIStore.SetParameterValue(Value, FoundVar);
	}
	else if (TypeName.Contains(TEXT("Vector")))
	{
		const TSharedPtr<FJsonObject>* ValueObj;
		if (Params->TryGetObjectField(TEXT("value"), ValueObj))
		{
			FVector3f Vec;
			Vec.X = (float)(*ValueObj)->GetNumberField(TEXT("x"));
			Vec.Y = (float)(*ValueObj)->GetNumberField(TEXT("y"));
			Vec.Z = (float)(*ValueObj)->GetNumberField(TEXT("z"));
			RIStore.SetParameterValue(Vec, FoundVar);
		}
		else
		{
			return MCPError(TEXT("Vector value requires {x, y, z} object."));
		}
	}
	else if (TypeName.Contains(TEXT("Color")) || TypeName.Contains(TEXT("LinearColor")))
	{
		const TSharedPtr<FJsonObject>* ValueObj;
		if (Params->TryGetObjectField(TEXT("value"), ValueObj))
		{
			FLinearColor Color;
			Color.R = (float)(*ValueObj)->GetNumberField(TEXT("r"));
			Color.G = (float)(*ValueObj)->GetNumberField(TEXT("g"));
			Color.B = (float)(*ValueObj)->GetNumberField(TEXT("b"));
			Color.A = (*ValueObj)->HasField(TEXT("a")) ? (float)(*ValueObj)->GetNumberField(TEXT("a")) : 1.0f;
			RIStore.SetParameterValue(Color, FoundVar);
		}
		else
		{
			return MCPError(TEXT("Color value requires {r, g, b, a} object."));
		}
	}
	else
	{
		return MCPError(FString::Printf(TEXT("Unsupported RI parameter type: %s"), *TypeName));
	}

	System->MarkPackageDirty();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("parameter"), FoundVar.GetName().ToString());
	Result->SetStringField(TEXT("type"), TypeName);
	Result->SetStringField(TEXT("script"), FoundScriptLabel);
	Result->SetStringField(TEXT("emitter"), Handle.GetName().ToString());
	Result->SetStringField(TEXT("system"), System->GetName());
	return Result;
}

#endif // WITH_NIAGARA

// ============================================================================
// Sequencer Handlers
// ============================================================================

void FMCPTcpServer::RegisterSequencerHandlers()
{
	RegisterHandler(TEXT("list_sequences"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleListSequences(Params); });
	RegisterHandler(TEXT("read_sequence"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleReadSequence(Params); });
	RegisterHandler(TEXT("add_sequence_track"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddSequenceTrack(Params); });
	RegisterHandler(TEXT("remove_sequence_track"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleRemoveSequenceTrack(Params); });
	RegisterHandler(TEXT("set_sequence_playback_range"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleSetSequencePlaybackRange(Params); });
	RegisterHandler(TEXT("add_sequence_binding"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddSequenceBinding(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleListSequences(const TSharedPtr<FJsonObject>& Params)
{
	FString PathFilter;
	Params->TryGetStringField(TEXT("path_filter"), PathFilter);

	FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> Assets;
	AssetReg.Get().GetAssetsByClass(ULevelSequence::StaticClass()->GetClassPathName(), Assets);

	TArray<TSharedPtr<FJsonValue>> SeqArray;
	for (const FAssetData& Asset : Assets)
	{
		FString AssetPath = Asset.GetObjectPathString();
		if (!PathFilter.IsEmpty() && !AssetPath.Contains(PathFilter))
		{
			continue;
		}

		auto SeqObj = MakeShared<FJsonObject>();
		SeqObj->SetStringField(TEXT("name"), Asset.AssetName.ToString());
		SeqObj->SetStringField(TEXT("path"), AssetPath);

		ULevelSequence* Seq = Cast<ULevelSequence>(Asset.GetAsset());
		if (Seq && Seq->GetMovieScene())
		{
			UMovieScene* MovieScene = Seq->GetMovieScene();
			SeqObj->SetNumberField(TEXT("track_count"), MovieScene->GetTracks().Num());

			// Playback range
			FFrameRate TickRes = MovieScene->GetTickResolution();
			TRange<FFrameNumber> PlayRange = MovieScene->GetPlaybackRange();
			if (PlayRange.HasLowerBound() && PlayRange.HasUpperBound())
			{
				double StartSec = TickRes.AsSeconds(PlayRange.GetLowerBoundValue());
				double EndSec = TickRes.AsSeconds(PlayRange.GetUpperBoundValue());
				SeqObj->SetNumberField(TEXT("start_seconds"), StartSec);
				SeqObj->SetNumberField(TEXT("end_seconds"), EndSec);
				SeqObj->SetNumberField(TEXT("duration_seconds"), EndSec - StartSec);
			}
		}

		SeqArray.Add(MakeShared<FJsonValueObject>(SeqObj));
	}

	auto Result = MCPSuccess();
	Result->SetArrayField(TEXT("sequences"), SeqArray);
	Result->SetNumberField(TEXT("count"), SeqArray.Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleReadSequence(const TSharedPtr<FJsonObject>& Params)
{
	FString SeqPath;
	if (!Params->TryGetStringField(TEXT("sequence"), SeqPath))
	{
		return MCPError(TEXT("Missing required param: sequence (name or path)"));
	}

	ULevelSequence* Seq = LoadObject<ULevelSequence>(nullptr, *SeqPath);
	if (!Seq)
	{
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByClass(ULevelSequence::StaticClass()->GetClassPathName(), Assets);
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == SeqPath)
			{
				Seq = Cast<ULevelSequence>(Asset.GetAsset());
				break;
			}
		}
	}
	if (!Seq)
	{
		return MCPError(FString::Printf(TEXT("Level sequence not found: %s"), *SeqPath));
	}

	UMovieScene* MovieScene = Seq->GetMovieScene();
	if (!MovieScene)
	{
		return MCPError(TEXT("Sequence has no MovieScene."));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("name"), Seq->GetName());
	Result->SetStringField(TEXT("path"), Seq->GetPathName());

	FFrameRate TickRes = MovieScene->GetTickResolution();
	FFrameRate DisplayRate = MovieScene->GetDisplayRate();
	Result->SetStringField(TEXT("display_rate"), FString::Printf(TEXT("%d fps"), DisplayRate.Numerator));

	// Playback range
	TRange<FFrameNumber> PlayRange = MovieScene->GetPlaybackRange();
	if (PlayRange.HasLowerBound() && PlayRange.HasUpperBound())
	{
		double StartSec = TickRes.AsSeconds(PlayRange.GetLowerBoundValue());
		double EndSec = TickRes.AsSeconds(PlayRange.GetUpperBoundValue());
		Result->SetNumberField(TEXT("start_seconds"), StartSec);
		Result->SetNumberField(TEXT("end_seconds"), EndSec);
		Result->SetNumberField(TEXT("duration_seconds"), EndSec - StartSec);
	}

	// Master tracks
	TArray<TSharedPtr<FJsonValue>> TrackArray;
	for (UMovieSceneTrack* Track : MovieScene->GetTracks())
	{
		auto TrackObj = MakeShared<FJsonObject>();
		TrackObj->SetStringField(TEXT("name"), Track->GetDisplayName().ToString());
		TrackObj->SetStringField(TEXT("type"), Track->GetClass()->GetName());
		TrackObj->SetStringField(TEXT("scope"), TEXT("master"));
		TrackObj->SetNumberField(TEXT("section_count"), Track->GetAllSections().Num());
		TrackArray.Add(MakeShared<FJsonValueObject>(TrackObj));
	}

	// Bindings (possessables + spawnables)
	TArray<TSharedPtr<FJsonValue>> BindingArray;
	for (int32 i = 0; i < MovieScene->GetPossessableCount(); i++)
	{
		const FMovieScenePossessable& Poss = MovieScene->GetPossessable(i);
		auto BindObj = MakeShared<FJsonObject>();
		BindObj->SetStringField(TEXT("name"), Poss.GetName());
		BindObj->SetStringField(TEXT("guid"), Poss.GetGuid().ToString());
		BindObj->SetStringField(TEXT("type"), TEXT("possessable"));
		BindObj->SetStringField(TEXT("class"), Poss.GetPossessedObjectClass() ? Poss.GetPossessedObjectClass()->GetName() : TEXT("Unknown"));

		// Tracks on this binding
		FMovieSceneBinding* Binding = MovieScene->FindBinding(Poss.GetGuid());
		if (Binding)
		{
			TArray<TSharedPtr<FJsonValue>> BoundTrackArray;
			for (UMovieSceneTrack* Track : Binding->GetTracks())
			{
				auto TrackObj = MakeShared<FJsonObject>();
				TrackObj->SetStringField(TEXT("name"), Track->GetDisplayName().ToString());
				TrackObj->SetStringField(TEXT("type"), Track->GetClass()->GetName());
				TrackObj->SetNumberField(TEXT("section_count"), Track->GetAllSections().Num());
				BoundTrackArray.Add(MakeShared<FJsonValueObject>(TrackObj));
				// Also add to global track list
				auto GlobalTrackObj = MakeShared<FJsonObject>();
				GlobalTrackObj->SetStringField(TEXT("name"), Track->GetDisplayName().ToString());
				GlobalTrackObj->SetStringField(TEXT("type"), Track->GetClass()->GetName());
				GlobalTrackObj->SetStringField(TEXT("scope"), TEXT("binding"));
				GlobalTrackObj->SetStringField(TEXT("binding"), Poss.GetName());
				GlobalTrackObj->SetNumberField(TEXT("section_count"), Track->GetAllSections().Num());
				TrackArray.Add(MakeShared<FJsonValueObject>(GlobalTrackObj));
			}
			BindObj->SetArrayField(TEXT("tracks"), BoundTrackArray);
		}

		BindingArray.Add(MakeShared<FJsonValueObject>(BindObj));
	}

	for (int32 i = 0; i < MovieScene->GetSpawnableCount(); i++)
	{
		const FMovieSceneSpawnable& Spawn = MovieScene->GetSpawnable(i);
		auto BindObj = MakeShared<FJsonObject>();
		BindObj->SetStringField(TEXT("name"), Spawn.GetName());
		BindObj->SetStringField(TEXT("guid"), Spawn.GetGuid().ToString());
		BindObj->SetStringField(TEXT("type"), TEXT("spawnable"));

		FMovieSceneBinding* Binding = MovieScene->FindBinding(Spawn.GetGuid());
		if (Binding)
		{
			TArray<TSharedPtr<FJsonValue>> BoundTrackArray;
			for (UMovieSceneTrack* Track : Binding->GetTracks())
			{
				auto TrackObj = MakeShared<FJsonObject>();
				TrackObj->SetStringField(TEXT("name"), Track->GetDisplayName().ToString());
				TrackObj->SetStringField(TEXT("type"), Track->GetClass()->GetName());
				TrackObj->SetNumberField(TEXT("section_count"), Track->GetAllSections().Num());
				BoundTrackArray.Add(MakeShared<FJsonValueObject>(TrackObj));
			}
			BindObj->SetArrayField(TEXT("tracks"), BoundTrackArray);
		}

		BindingArray.Add(MakeShared<FJsonValueObject>(BindObj));
	}

	Result->SetArrayField(TEXT("tracks"), TrackArray);
	Result->SetNumberField(TEXT("track_count"), TrackArray.Num());
	Result->SetArrayField(TEXT("bindings"), BindingArray);
	Result->SetNumberField(TEXT("binding_count"), BindingArray.Num());

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddSequenceTrack(const TSharedPtr<FJsonObject>& Params)
{
	FString SeqPath;
	if (!Params->TryGetStringField(TEXT("sequence"), SeqPath))
	{
		return MCPError(TEXT("Missing required param: sequence"));
	}

	FString TrackType;
	if (!Params->TryGetStringField(TEXT("track_type"), TrackType))
	{
		return MCPError(TEXT("Missing required param: track_type (e.g. Float, Bool, Transform, Visibility, Event, Audio, SkeletalAnimation, Sub, CameraCut, Fade)"));
	}

	// Optional: binding guid to add track to a specific binding instead of master
	FString BindingGuid;
	Params->TryGetStringField(TEXT("binding_guid"), BindingGuid);

	ULevelSequence* Seq = LoadObject<ULevelSequence>(nullptr, *SeqPath);
	if (!Seq)
	{
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByClass(ULevelSequence::StaticClass()->GetClassPathName(), Assets);
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == SeqPath)
			{
				Seq = Cast<ULevelSequence>(Asset.GetAsset());
				break;
			}
		}
	}
	if (!Seq)
	{
		return MCPError(FString::Printf(TEXT("Level sequence not found: %s"), *SeqPath));
	}

	UMovieScene* MovieScene = Seq->GetMovieScene();
	if (!MovieScene)
	{
		return MCPError(TEXT("Sequence has no MovieScene."));
	}

	// Map track type string to UClass
	UClass* TrackClass = nullptr;
	if (TrackType == TEXT("Float")) TrackClass = UMovieSceneFloatTrack::StaticClass();
	else if (TrackType == TEXT("Bool")) TrackClass = UMovieSceneBoolTrack::StaticClass();
	else if (TrackType == TEXT("Transform")) TrackClass = UMovieScene3DTransformTrack::StaticClass();
	else if (TrackType == TEXT("Visibility")) TrackClass = UMovieSceneVisibilityTrack::StaticClass();
	else if (TrackType == TEXT("Event")) TrackClass = UMovieSceneEventTrack::StaticClass();
	else if (TrackType == TEXT("Audio")) TrackClass = UMovieSceneAudioTrack::StaticClass();
	else if (TrackType == TEXT("SkeletalAnimation")) TrackClass = UMovieSceneSkeletalAnimationTrack::StaticClass();
	else if (TrackType == TEXT("Sub")) TrackClass = UMovieSceneSubTrack::StaticClass();
	else if (TrackType == TEXT("CameraCut")) TrackClass = UMovieSceneCameraCutTrack::StaticClass();
	else if (TrackType == TEXT("Fade")) TrackClass = UMovieSceneFadeTrack::StaticClass();
	else
	{
		return MCPError(FString::Printf(TEXT("Unknown track type: %s. Supported: Float, Bool, Transform, Visibility, Event, Audio, SkeletalAnimation, Sub, CameraCut, Fade"), *TrackType));
	}

	MovieScene->Modify();

	UMovieSceneTrack* NewTrack = nullptr;

	if (!BindingGuid.IsEmpty())
	{
		FGuid Guid;
		if (!FGuid::Parse(BindingGuid, Guid))
		{
			return MCPError(TEXT("Invalid binding_guid format."));
		}

		FMovieSceneBinding* Binding = MovieScene->FindBinding(Guid);
		if (!Binding)
		{
			return MCPError(FString::Printf(TEXT("Binding not found: %s"), *BindingGuid));
		}

		NewTrack = MovieScene->AddTrack(TrackClass, Guid);
	}
	else
	{
		NewTrack = MovieScene->AddTrack(TrackClass);
	}

	if (!NewTrack)
	{
		return MCPError(TEXT("Failed to create track."));
	}

	// Add a default section to the track
	UMovieSceneSection* Section = NewTrack->CreateNewSection();
	if (Section)
	{
		NewTrack->AddSection(*Section);
		// Set section range to match playback range
		TRange<FFrameNumber> PlayRange = MovieScene->GetPlaybackRange();
		Section->SetRange(PlayRange);
	}

	Seq->MarkPackageDirty();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("track_type"), TrackType);
	Result->SetStringField(TEXT("track_name"), NewTrack->GetDisplayName().ToString());
	Result->SetStringField(TEXT("track_class"), NewTrack->GetClass()->GetName());
	Result->SetStringField(TEXT("sequence"), Seq->GetName());
	if (!BindingGuid.IsEmpty())
	{
		Result->SetStringField(TEXT("binding_guid"), BindingGuid);
	}
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleRemoveSequenceTrack(const TSharedPtr<FJsonObject>& Params)
{
	FString SeqPath;
	if (!Params->TryGetStringField(TEXT("sequence"), SeqPath))
	{
		return MCPError(TEXT("Missing required param: sequence"));
	}

	FString TrackName;
	if (!Params->TryGetStringField(TEXT("track_name"), TrackName))
	{
		return MCPError(TEXT("Missing required param: track_name"));
	}

	FString BindingGuid;
	Params->TryGetStringField(TEXT("binding_guid"), BindingGuid);

	ULevelSequence* Seq = LoadObject<ULevelSequence>(nullptr, *SeqPath);
	if (!Seq)
	{
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByClass(ULevelSequence::StaticClass()->GetClassPathName(), Assets);
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == SeqPath)
			{
				Seq = Cast<ULevelSequence>(Asset.GetAsset());
				break;
			}
		}
	}
	if (!Seq)
	{
		return MCPError(FString::Printf(TEXT("Level sequence not found: %s"), *SeqPath));
	}

	UMovieScene* MovieScene = Seq->GetMovieScene();
	if (!MovieScene)
	{
		return MCPError(TEXT("Sequence has no MovieScene."));
	}

	// Find and remove the track
	UMovieSceneTrack* FoundTrack = nullptr;

	if (!BindingGuid.IsEmpty())
	{
		FGuid Guid;
		if (!FGuid::Parse(BindingGuid, Guid))
		{
			return MCPError(TEXT("Invalid binding_guid."));
		}
		FMovieSceneBinding* Binding = MovieScene->FindBinding(Guid);
		if (Binding)
		{
			for (UMovieSceneTrack* Track : Binding->GetTracks())
			{
				if (Track->GetDisplayName().ToString() == TrackName)
				{
					FoundTrack = Track;
					break;
				}
			}
		}
	}
	else
	{
		for (UMovieSceneTrack* Track : MovieScene->GetTracks())
		{
			if (Track->GetDisplayName().ToString() == TrackName)
			{
				FoundTrack = Track;
				break;
			}
		}
	}

	if (!FoundTrack)
	{
		return MCPError(FString::Printf(TEXT("Track not found: %s"), *TrackName));
	}

	MovieScene->Modify();

	if (!BindingGuid.IsEmpty())
	{
		FGuid Guid;
		FGuid::Parse(BindingGuid, Guid);
		MovieScene->RemoveTrack(*FoundTrack);
	}
	else
	{
		MovieScene->RemoveTrack(*FoundTrack);
	}

	Seq->MarkPackageDirty();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("removed_track"), TrackName);
	Result->SetStringField(TEXT("sequence"), Seq->GetName());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleSetSequencePlaybackRange(const TSharedPtr<FJsonObject>& Params)
{
	FString SeqPath;
	if (!Params->TryGetStringField(TEXT("sequence"), SeqPath))
	{
		return MCPError(TEXT("Missing required param: sequence"));
	}

	if (!Params->HasField(TEXT("start_seconds")) || !Params->HasField(TEXT("end_seconds")))
	{
		return MCPError(TEXT("Missing required params: start_seconds and end_seconds"));
	}

	double StartSec = Params->GetNumberField(TEXT("start_seconds"));
	double EndSec = Params->GetNumberField(TEXT("end_seconds"));

	ULevelSequence* Seq = LoadObject<ULevelSequence>(nullptr, *SeqPath);
	if (!Seq)
	{
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByClass(ULevelSequence::StaticClass()->GetClassPathName(), Assets);
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == SeqPath)
			{
				Seq = Cast<ULevelSequence>(Asset.GetAsset());
				break;
			}
		}
	}
	if (!Seq)
	{
		return MCPError(FString::Printf(TEXT("Level sequence not found: %s"), *SeqPath));
	}

	UMovieScene* MovieScene = Seq->GetMovieScene();
	if (!MovieScene)
	{
		return MCPError(TEXT("Sequence has no MovieScene."));
	}

	MovieScene->Modify();

	FFrameRate TickRes = MovieScene->GetTickResolution();
	FFrameNumber StartFrame = TickRes.AsFrameNumber(StartSec);
	FFrameNumber EndFrame = TickRes.AsFrameNumber(EndSec);

	MovieScene->SetPlaybackRange(TRange<FFrameNumber>(StartFrame, EndFrame));

	Seq->MarkPackageDirty();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("sequence"), Seq->GetName());
	Result->SetNumberField(TEXT("start_seconds"), StartSec);
	Result->SetNumberField(TEXT("end_seconds"), EndSec);
	Result->SetNumberField(TEXT("duration_seconds"), EndSec - StartSec);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddSequenceBinding(const TSharedPtr<FJsonObject>& Params)
{
	FString SeqPath;
	if (!Params->TryGetStringField(TEXT("sequence"), SeqPath))
	{
		return MCPError(TEXT("Missing required param: sequence"));
	}

	FString ActorName;
	if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
	{
		return MCPError(TEXT("Missing required param: actor_name (name of actor in level to bind)"));
	}

	ULevelSequence* Seq = LoadObject<ULevelSequence>(nullptr, *SeqPath);
	if (!Seq)
	{
		FAssetRegistryModule& AssetReg = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> Assets;
		AssetReg.Get().GetAssetsByClass(ULevelSequence::StaticClass()->GetClassPathName(), Assets);
		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == SeqPath)
			{
				Seq = Cast<ULevelSequence>(Asset.GetAsset());
				break;
			}
		}
	}
	if (!Seq)
	{
		return MCPError(FString::Printf(TEXT("Level sequence not found: %s"), *SeqPath));
	}

	UMovieScene* MovieScene = Seq->GetMovieScene();
	if (!MovieScene)
	{
		return MCPError(TEXT("Sequence has no MovieScene."));
	}

	// Find actor in the world
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		return MCPError(TEXT("No editor world available."));
	}

	AActor* FoundActor = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->GetActorLabel() == ActorName || It->GetName() == ActorName)
		{
			FoundActor = *It;
			break;
		}
	}

	if (!FoundActor)
	{
		return MCPError(FString::Printf(TEXT("Actor not found in level: %s"), *ActorName));
	}

	MovieScene->Modify();

	// Create a possessable for this actor
	FGuid NewGuid = MovieScene->AddPossessable(FoundActor->GetActorLabel(), FoundActor->GetClass());

	// Bind the possessable to the actor
	Seq->BindPossessableObject(NewGuid, *FoundActor, World);

	Seq->MarkPackageDirty();

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("sequence"), Seq->GetName());
	Result->SetStringField(TEXT("actor"), FoundActor->GetActorLabel());
	Result->SetStringField(TEXT("binding_guid"), NewGuid.ToString());
	Result->SetStringField(TEXT("type"), TEXT("possessable"));
	return Result;
}

// ============================================================================
// Widget Animation Handlers
// ============================================================================

void FMCPTcpServer::RegisterWidgetAnimationHandlers()
{
	RegisterHandler(TEXT("list_widget_animations"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleListWidgetAnimations(Params); });
	RegisterHandler(TEXT("read_widget_animation"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleReadWidgetAnimation(Params); });
	RegisterHandler(TEXT("create_widget_animation"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleCreateWidgetAnimation(Params); });
	RegisterHandler(TEXT("add_widget_animation_track"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddWidgetAnimationTrack(Params); });
	RegisterHandler(TEXT("add_widget_animation_keyframe"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleAddWidgetAnimationKeyframe(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleListWidgetAnimations(const TSharedPtr<FJsonObject>& Params)
{
	FString WBPPath;
	if (!Params->TryGetStringField(TEXT("widget_blueprint"), WBPPath))
	{
		return MCPError(TEXT("Missing required param: widget_blueprint"));
	}

	UWidgetBlueprint* WBP = FindWidgetBlueprintByPath(WBPPath);
	if (!WBP)
	{
		return MCPError(FString::Printf(TEXT("Widget Blueprint not found: %s"), *WBPPath));
	}

	TArray<TSharedPtr<FJsonValue>> AnimArray;
	for (UWidgetAnimation* Anim : WBP->Animations)
	{
		if (!Anim) continue;
		auto AnimObj = MakeShared<FJsonObject>();
		AnimObj->SetStringField(TEXT("name"), Anim->GetDisplayName().ToString());
		AnimObj->SetNumberField(TEXT("start_time"), Anim->GetStartTime());
		AnimObj->SetNumberField(TEXT("end_time"), Anim->GetEndTime());
		AnimObj->SetNumberField(TEXT("duration"), Anim->GetEndTime() - Anim->GetStartTime());

		UMovieScene* MovieScene = Anim->GetMovieScene();
		if (MovieScene)
		{
			AnimObj->SetNumberField(TEXT("track_count"), MovieScene->GetTracks().Num());

			// Count bindings
			int32 BindCount = MovieScene->GetPossessableCount();
			AnimObj->SetNumberField(TEXT("binding_count"), BindCount);
		}

		// Bound widget names
		TArray<TSharedPtr<FJsonValue>> BindingNames;
		for (const FWidgetAnimationBinding& Binding : Anim->GetBindings())
		{
			BindingNames.Add(MakeShared<FJsonValueString>(Binding.WidgetName.ToString()));
		}
		AnimObj->SetArrayField(TEXT("bound_widgets"), BindingNames);

		AnimArray.Add(MakeShared<FJsonValueObject>(AnimObj));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("widget_blueprint"), WBP->GetPathName());
	Result->SetArrayField(TEXT("animations"), AnimArray);
	Result->SetNumberField(TEXT("count"), AnimArray.Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleReadWidgetAnimation(const TSharedPtr<FJsonObject>& Params)
{
	FString WBPPath;
	if (!Params->TryGetStringField(TEXT("widget_blueprint"), WBPPath))
	{
		return MCPError(TEXT("Missing required param: widget_blueprint"));
	}

	FString AnimName;
	if (!Params->TryGetStringField(TEXT("animation_name"), AnimName))
	{
		return MCPError(TEXT("Missing required param: animation_name"));
	}

	UWidgetBlueprint* WBP = FindWidgetBlueprintByPath(WBPPath);
	if (!WBP)
	{
		return MCPError(FString::Printf(TEXT("Widget Blueprint not found: %s"), *WBPPath));
	}

	UWidgetAnimation* FoundAnim = nullptr;
	for (UWidgetAnimation* Anim : WBP->Animations)
	{
		if (Anim && Anim->GetDisplayName().ToString() == AnimName)
		{
			FoundAnim = Anim;
			break;
		}
	}

	if (!FoundAnim)
	{
		return MCPError(FString::Printf(TEXT("Animation not found: %s"), *AnimName));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("name"), FoundAnim->GetDisplayName().ToString());
	Result->SetNumberField(TEXT("start_time"), FoundAnim->GetStartTime());
	Result->SetNumberField(TEXT("end_time"), FoundAnim->GetEndTime());

	UMovieScene* MovieScene = FoundAnim->GetMovieScene();
	if (!MovieScene)
	{
		return Result;
	}

	FFrameRate TickRes = MovieScene->GetTickResolution();
	FFrameRate DisplayRate = MovieScene->GetDisplayRate();
	Result->SetStringField(TEXT("display_rate"), FString::Printf(TEXT("%d fps"), DisplayRate.Numerator));

	// Bindings (widgets being animated)
	TArray<TSharedPtr<FJsonValue>> BindingArray;
	for (int32 i = 0; i < MovieScene->GetPossessableCount(); i++)
	{
		const FMovieScenePossessable& Poss = MovieScene->GetPossessable(i);
		auto BindObj = MakeShared<FJsonObject>();
		BindObj->SetStringField(TEXT("name"), Poss.GetName());
		BindObj->SetStringField(TEXT("guid"), Poss.GetGuid().ToString());

		FMovieSceneBinding* Binding = MovieScene->FindBinding(Poss.GetGuid());
		if (Binding)
		{
			TArray<TSharedPtr<FJsonValue>> TrackArr;
			for (UMovieSceneTrack* Track : Binding->GetTracks())
			{
				auto TrackObj = MakeShared<FJsonObject>();
				TrackObj->SetStringField(TEXT("name"), Track->GetDisplayName().ToString());
				TrackObj->SetStringField(TEXT("type"), Track->GetClass()->GetName());
				TrackObj->SetNumberField(TEXT("section_count"), Track->GetAllSections().Num());
				TrackArr.Add(MakeShared<FJsonValueObject>(TrackObj));
			}
			BindObj->SetArrayField(TEXT("tracks"), TrackArr);
		}

		BindingArray.Add(MakeShared<FJsonValueObject>(BindObj));
	}
	Result->SetArrayField(TEXT("bindings"), BindingArray);

	// Master tracks
	TArray<TSharedPtr<FJsonValue>> MasterTrackArray;
	for (UMovieSceneTrack* Track : MovieScene->GetTracks())
	{
		auto TrackObj = MakeShared<FJsonObject>();
		TrackObj->SetStringField(TEXT("name"), Track->GetDisplayName().ToString());
		TrackObj->SetStringField(TEXT("type"), Track->GetClass()->GetName());
		MasterTrackArray.Add(MakeShared<FJsonValueObject>(TrackObj));
	}
	Result->SetArrayField(TEXT("master_tracks"), MasterTrackArray);

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleCreateWidgetAnimation(const TSharedPtr<FJsonObject>& Params)
{
	FString WBPPath;
	if (!Params->TryGetStringField(TEXT("widget_blueprint"), WBPPath))
	{
		return MCPError(TEXT("Missing required param: widget_blueprint"));
	}

	FString AnimName;
	if (!Params->TryGetStringField(TEXT("animation_name"), AnimName))
	{
		return MCPError(TEXT("Missing required param: animation_name"));
	}

	double Duration = 1.0;
	if (Params->HasField(TEXT("duration")))
	{
		Duration = Params->GetNumberField(TEXT("duration"));
	}

	UWidgetBlueprint* WBP = FindWidgetBlueprintByPath(WBPPath);
	if (!WBP)
	{
		return MCPError(FString::Printf(TEXT("Widget Blueprint not found: %s"), *WBPPath));
	}

	// Check for duplicate
	for (UWidgetAnimation* Anim : WBP->Animations)
	{
		if (Anim && Anim->GetDisplayName().ToString() == AnimName)
		{
			return MCPError(FString::Printf(TEXT("Animation already exists: %s"), *AnimName));
		}
	}

	// Create the animation object
	UWidgetAnimation* NewAnim = NewObject<UWidgetAnimation>(WBP, FName(*AnimName), RF_Transactional);
	NewAnim->SetDisplayLabel(AnimName);

	// Create its MovieScene
	UMovieScene* MovieScene = NewObject<UMovieScene>(NewAnim, FName(*(AnimName + TEXT("_MovieScene"))), RF_Transactional);
	NewAnim->MovieScene = MovieScene;

	// Set playback range
	FFrameRate TickRes = MovieScene->GetTickResolution();
	FFrameNumber StartFrame = TickRes.AsFrameNumber(0.0);
	FFrameNumber EndFrame = TickRes.AsFrameNumber(Duration);
	MovieScene->SetPlaybackRange(TRange<FFrameNumber>(StartFrame, EndFrame));

	// Add to the widget blueprint
	WBP->Animations.Add(NewAnim);
	FBlueprintEditorUtils::MarkBlueprintAsModified(WBP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("widget_blueprint"), WBP->GetPathName());
	Result->SetStringField(TEXT("animation_name"), AnimName);
	Result->SetNumberField(TEXT("duration"), Duration);
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddWidgetAnimationTrack(const TSharedPtr<FJsonObject>& Params)
{
	FString WBPPath;
	if (!Params->TryGetStringField(TEXT("widget_blueprint"), WBPPath))
	{
		return MCPError(TEXT("Missing required param: widget_blueprint"));
	}

	FString AnimName;
	if (!Params->TryGetStringField(TEXT("animation_name"), AnimName))
	{
		return MCPError(TEXT("Missing required param: animation_name"));
	}

	FString WidgetName;
	if (!Params->TryGetStringField(TEXT("widget_name"), WidgetName))
	{
		return MCPError(TEXT("Missing required param: widget_name (name of widget to animate)"));
	}

	FString TrackType;
	if (!Params->TryGetStringField(TEXT("track_type"), TrackType))
	{
		return MCPError(TEXT("Missing required param: track_type (Float, Bool, Visibility, Transform)"));
	}

	UWidgetBlueprint* WBP = FindWidgetBlueprintByPath(WBPPath);
	if (!WBP)
	{
		return MCPError(FString::Printf(TEXT("Widget Blueprint not found: %s"), *WBPPath));
	}

	UWidgetAnimation* FoundAnim = nullptr;
	for (UWidgetAnimation* Anim : WBP->Animations)
	{
		if (Anim && Anim->GetDisplayName().ToString() == AnimName)
		{
			FoundAnim = Anim;
			break;
		}
	}
	if (!FoundAnim)
	{
		return MCPError(FString::Printf(TEXT("Animation not found: %s"), *AnimName));
	}

	UMovieScene* MovieScene = FoundAnim->GetMovieScene();
	if (!MovieScene)
	{
		return MCPError(TEXT("Animation has no MovieScene."));
	}

	// Find the widget in the tree
	UWidget* TargetWidget = nullptr;
	bool bIsRootWidget = false;
	if (WBP->WidgetTree)
	{
		if (WBP->WidgetTree->RootWidget && WBP->WidgetTree->RootWidget->GetName() == WidgetName)
		{
			TargetWidget = WBP->WidgetTree->RootWidget;
			bIsRootWidget = true;
		}
		else
		{
			WBP->WidgetTree->ForEachWidget([&TargetWidget, &WidgetName](UWidget* Widget)
			{
				if (Widget->GetName() == WidgetName)
				{
					TargetWidget = Widget;
				}
			});
		}
	}

	if (!TargetWidget)
	{
		return MCPError(FString::Printf(TEXT("Widget not found in tree: %s"), *WidgetName));
	}

	// Check if a possessable already exists for this widget, or create one
	FGuid WidgetGuid;
	bool bFoundExisting = false;
	for (const FWidgetAnimationBinding& Binding : FoundAnim->AnimationBindings)
	{
		if (Binding.WidgetName == FName(*WidgetName))
		{
			WidgetGuid = Binding.AnimationGuid;
			bFoundExisting = true;
			break;
		}
	}

	if (!bFoundExisting)
	{
		WidgetGuid = MovieScene->AddPossessable(WidgetName, TargetWidget->GetClass());

		// Add the binding
		FWidgetAnimationBinding NewBinding;
		NewBinding.WidgetName = FName(*WidgetName);
		NewBinding.AnimationGuid = WidgetGuid;
		NewBinding.bIsRootWidget = bIsRootWidget;

		// SlotWidgetName is the parent panel widget name (for slot property animation)
		if (TargetWidget->Slot)
		{
			UPanelWidget* ParentPanel = TargetWidget->Slot->Parent;
			if (ParentPanel)
			{
				NewBinding.SlotWidgetName = FName(*ParentPanel->GetName());
			}
		}

		FoundAnim->AnimationBindings.Add(NewBinding);
	}

	// Map track type to UClass
	UClass* TrackClass = nullptr;
	if (TrackType == TEXT("Float")) TrackClass = UMovieSceneFloatTrack::StaticClass();
	else if (TrackType == TEXT("Bool")) TrackClass = UMovieSceneBoolTrack::StaticClass();
	else if (TrackType == TEXT("Visibility")) TrackClass = UMovieSceneVisibilityTrack::StaticClass();
	else if (TrackType == TEXT("Transform")) TrackClass = UMovieScene3DTransformTrack::StaticClass();
	else
	{
		return MCPError(FString::Printf(TEXT("Unsupported track type: %s. Supported: Float, Bool, Visibility, Transform"), *TrackType));
	}

	MovieScene->Modify();
	UMovieSceneTrack* NewTrack = MovieScene->AddTrack(TrackClass, WidgetGuid);
	if (!NewTrack)
	{
		return MCPError(TEXT("Failed to create track."));
	}

	// Add default section spanning the animation range
	UMovieSceneSection* Section = NewTrack->CreateNewSection();
	if (Section)
	{
		NewTrack->AddSection(*Section);
		TRange<FFrameNumber> PlayRange = MovieScene->GetPlaybackRange();
		Section->SetRange(PlayRange);
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(WBP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("animation"), AnimName);
	Result->SetStringField(TEXT("widget"), WidgetName);
	Result->SetStringField(TEXT("track_type"), TrackType);
	Result->SetStringField(TEXT("binding_guid"), WidgetGuid.ToString());
	Result->SetStringField(TEXT("track_name"), NewTrack->GetDisplayName().ToString());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleAddWidgetAnimationKeyframe(const TSharedPtr<FJsonObject>& Params)
{
	FString WBPPath;
	if (!Params->TryGetStringField(TEXT("widget_blueprint"), WBPPath))
	{
		return MCPError(TEXT("Missing required param: widget_blueprint"));
	}

	FString AnimName;
	if (!Params->TryGetStringField(TEXT("animation_name"), AnimName))
	{
		return MCPError(TEXT("Missing required param: animation_name"));
	}

	FString BindingGuid;
	if (!Params->TryGetStringField(TEXT("binding_guid"), BindingGuid))
	{
		return MCPError(TEXT("Missing required param: binding_guid (from add_widget_animation_track)"));
	}

	if (!Params->HasField(TEXT("time")))
	{
		return MCPError(TEXT("Missing required param: time (in seconds)"));
	}
	double Time = Params->GetNumberField(TEXT("time"));

	if (!Params->HasField(TEXT("value")))
	{
		return MCPError(TEXT("Missing required param: value"));
	}

	UWidgetBlueprint* WBP = FindWidgetBlueprintByPath(WBPPath);
	if (!WBP)
	{
		return MCPError(FString::Printf(TEXT("Widget Blueprint not found: %s"), *WBPPath));
	}

	UWidgetAnimation* FoundAnim = nullptr;
	for (UWidgetAnimation* Anim : WBP->Animations)
	{
		if (Anim && Anim->GetDisplayName().ToString() == AnimName)
		{
			FoundAnim = Anim;
			break;
		}
	}
	if (!FoundAnim)
	{
		return MCPError(FString::Printf(TEXT("Animation not found: %s"), *AnimName));
	}

	UMovieScene* MovieScene = FoundAnim->GetMovieScene();
	if (!MovieScene)
	{
		return MCPError(TEXT("Animation has no MovieScene."));
	}

	FGuid Guid;
	if (!FGuid::Parse(BindingGuid, Guid))
	{
		return MCPError(TEXT("Invalid binding_guid."));
	}

	FMovieSceneBinding* Binding = MovieScene->FindBinding(Guid);
	if (!Binding)
	{
		return MCPError(TEXT("Binding not found for the given GUID."));
	}

	// Find the first track on this binding (or use track_index)
	int32 TrackIndex = 0;
	if (Params->HasField(TEXT("track_index")))
	{
		TrackIndex = (int32)Params->GetNumberField(TEXT("track_index"));
	}

	const TArray<UMovieSceneTrack*>& Tracks = Binding->GetTracks();
	if (TrackIndex < 0 || TrackIndex >= Tracks.Num())
	{
		return MCPError(FString::Printf(TEXT("Track index %d out of range (0-%d)"), TrackIndex, Tracks.Num() - 1));
	}

	UMovieSceneTrack* Track = Tracks[TrackIndex];
	TArrayView<UMovieSceneSection* const> Sections = Track->GetAllSections();
	if (Sections.Num() == 0)
	{
		return MCPError(TEXT("Track has no sections."));
	}

	UMovieSceneSection* Section = Sections[0];
	FFrameRate TickRes = MovieScene->GetTickResolution();
	FFrameNumber Frame = TickRes.AsFrameNumber(Time);

	// Try to add keyframe to float channel
	FMovieSceneChannelProxy& ChannelProxy = Section->GetChannelProxy();
	TArrayView<FMovieSceneFloatChannel*> FloatChannels = ChannelProxy.GetChannels<FMovieSceneFloatChannel>();

	if (FloatChannels.Num() > 0)
	{
		int32 ChannelIdx = 0;
		if (Params->HasField(TEXT("channel_index")))
		{
			ChannelIdx = (int32)Params->GetNumberField(TEXT("channel_index"));
		}
		if (ChannelIdx >= 0 && ChannelIdx < FloatChannels.Num())
		{
			float Value = (float)Params->GetNumberField(TEXT("value"));

			FString InterpMode = TEXT("Cubic");
			Params->TryGetStringField(TEXT("interpolation"), InterpMode);

			ERichCurveInterpMode Interp = RCIM_Cubic;
			if (InterpMode == TEXT("Linear")) Interp = RCIM_Linear;
			else if (InterpMode == TEXT("Constant")) Interp = RCIM_Constant;

			FMovieSceneFloatValue KeyValue(Value);
			KeyValue.InterpMode = Interp;

			FloatChannels[ChannelIdx]->AddLinearKey(Frame, Value);

			FBlueprintEditorUtils::MarkBlueprintAsModified(WBP);

			auto Result = MCPSuccess();
			Result->SetStringField(TEXT("animation"), AnimName);
			Result->SetNumberField(TEXT("time"), Time);
			Result->SetNumberField(TEXT("value"), Value);
			Result->SetStringField(TEXT("interpolation"), InterpMode);
			Result->SetNumberField(TEXT("channel_index"), ChannelIdx);
			return Result;
		}
	}

	// Try double channels
	TArrayView<FMovieSceneDoubleChannel*> DoubleChannels = ChannelProxy.GetChannels<FMovieSceneDoubleChannel>();
	if (DoubleChannels.Num() > 0)
	{
		int32 ChannelIdx = 0;
		if (Params->HasField(TEXT("channel_index")))
		{
			ChannelIdx = (int32)Params->GetNumberField(TEXT("channel_index"));
		}
		if (ChannelIdx >= 0 && ChannelIdx < DoubleChannels.Num())
		{
			double Value = Params->GetNumberField(TEXT("value"));
			DoubleChannels[ChannelIdx]->AddLinearKey(Frame, Value);

			FBlueprintEditorUtils::MarkBlueprintAsModified(WBP);

			auto Result = MCPSuccess();
			Result->SetStringField(TEXT("animation"), AnimName);
			Result->SetNumberField(TEXT("time"), Time);
			Result->SetNumberField(TEXT("value"), Value);
			Result->SetNumberField(TEXT("channel_index"), ChannelIdx);
			return Result;
		}
	}

	return MCPError(TEXT("No compatible channels found on the track section. Ensure the track has Float or Double channels."));
}

// ============================================================================
// Blueprint Macro Handlers
// ============================================================================

void FMCPTcpServer::RegisterBlueprintMacroHandlers()
{
	RegisterHandler(TEXT("list_blueprint_macros"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleListBPMacros(Params); });
	RegisterHandler(TEXT("create_blueprint_macro"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleCreateBPMacro(Params); });
	RegisterHandler(TEXT("read_blueprint_macro_graph"), [this](const TSharedPtr<FJsonObject>& Params) { return HandleReadBPMacroGraph(Params); });
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleListBPMacros(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	TArray<TSharedPtr<FJsonValue>> MacroArray;
	for (UEdGraph* Graph : BP->MacroGraphs)
	{
		if (!Graph) continue;
		auto MacroObj = MakeShared<FJsonObject>();
		MacroObj->SetStringField(TEXT("name"), Graph->GetName());
		MacroObj->SetNumberField(TEXT("node_count"), Graph->Nodes.Num());

		// Find tunnel entry/exit nodes to report inputs/outputs
		TArray<TSharedPtr<FJsonValue>> InputPins;
		TArray<TSharedPtr<FJsonValue>> OutputPins;
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			UK2Node_Tunnel* TunnelNode = Cast<UK2Node_Tunnel>(Node);
			if (!TunnelNode) continue;

			if (TunnelNode->bCanHaveOutputs && !TunnelNode->bCanHaveInputs)
			{
				// This is the entry node (has outputs = macro inputs)
				for (const TSharedPtr<FUserPinInfo>& Pin : TunnelNode->UserDefinedPins)
				{
					auto PinObj = MakeShared<FJsonObject>();
					PinObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
					PinObj->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
					InputPins.Add(MakeShared<FJsonValueObject>(PinObj));
				}
			}
			else if (TunnelNode->bCanHaveInputs && !TunnelNode->bCanHaveOutputs)
			{
				// This is the exit node (has inputs = macro outputs)
				for (const TSharedPtr<FUserPinInfo>& Pin : TunnelNode->UserDefinedPins)
				{
					auto PinObj = MakeShared<FJsonObject>();
					PinObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
					PinObj->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
					OutputPins.Add(MakeShared<FJsonValueObject>(PinObj));
				}
			}
		}

		MacroObj->SetArrayField(TEXT("inputs"), InputPins);
		MacroObj->SetArrayField(TEXT("outputs"), OutputPins);
		MacroArray.Add(MakeShared<FJsonValueObject>(MacroObj));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("blueprint"), BP->GetPathName());
	Result->SetArrayField(TEXT("macros"), MacroArray);
	Result->SetNumberField(TEXT("count"), MacroArray.Num());
	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleCreateBPMacro(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	FString MacroName;
	if (!Params->TryGetStringField(TEXT("macro_name"), MacroName))
	{
		return MCPError(TEXT("Missing required param: macro_name"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	// Check for duplicate
	for (UEdGraph* Graph : BP->MacroGraphs)
	{
		if (Graph && Graph->GetName() == MacroName)
		{
			return MCPError(FString::Printf(TEXT("Macro already exists: %s"), *MacroName));
		}
	}

	// Create the macro graph
	UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(BP, FName(*MacroName), UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	FBlueprintEditorUtils::AddMacroGraph(BP, NewGraph, true, nullptr);

	// The AddMacroGraph creates the tunnel entry and exit nodes automatically.
	// Now add inputs/outputs if specified.

	// Helper lambda to resolve pin type
	auto ResolvePinType = [](const FString& TypeStr) -> FEdGraphPinType
	{
		FEdGraphPinType PinType;
		if (TypeStr == TEXT("exec"))
		{
			PinType.PinCategory = UEdGraphSchema_K2::PC_Exec;
		}
		else if (TypeStr == TEXT("bool"))
		{
			PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
		}
		else if (TypeStr == TEXT("int"))
		{
			PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
		}
		else if (TypeStr == TEXT("float") || TypeStr == TEXT("double"))
		{
			PinType.PinCategory = UEdGraphSchema_K2::PC_Real;
			PinType.PinSubCategory = UEdGraphSchema_K2::PC_Double;
		}
		else if (TypeStr == TEXT("string"))
		{
			PinType.PinCategory = UEdGraphSchema_K2::PC_String;
		}
		else if (TypeStr == TEXT("Vector"))
		{
			PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
			PinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
		}
		else if (TypeStr == TEXT("Rotator"))
		{
			PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
			PinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
		}
		else if (TypeStr == TEXT("Transform"))
		{
			PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
			PinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
		}
		else
		{
			// Try as object class
			UClass* ObjClass = FindClassByName(TypeStr);
			if (!ObjClass) ObjClass = FindClassByName(TEXT("A") + TypeStr);
			if (!ObjClass) ObjClass = FindClassByName(TEXT("U") + TypeStr);
			if (ObjClass)
			{
				PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
				PinType.PinSubCategoryObject = ObjClass;
			}
			else
			{
				PinType.PinCategory = UEdGraphSchema_K2::PC_String; // fallback
			}
		}
		return PinType;
	};

	// Find tunnel entry and exit nodes
	UK2Node_Tunnel* EntryNode = nullptr;
	UK2Node_Tunnel* ExitNode = nullptr;
	for (UEdGraphNode* Node : NewGraph->Nodes)
	{
		UK2Node_Tunnel* TunnelNode = Cast<UK2Node_Tunnel>(Node);
		if (!TunnelNode) continue;

		if (TunnelNode->bCanHaveOutputs && !TunnelNode->bCanHaveInputs)
		{
			EntryNode = TunnelNode;
		}
		else if (TunnelNode->bCanHaveInputs && !TunnelNode->bCanHaveOutputs)
		{
			ExitNode = TunnelNode;
		}
	}

	// Add inputs
	const TArray<TSharedPtr<FJsonValue>>* InputsArray = nullptr;
	if (Params->TryGetArrayField(TEXT("inputs"), InputsArray) && EntryNode)
	{
		for (const auto& InputVal : *InputsArray)
		{
			auto InputObj = InputVal->AsObject();
			if (!InputObj) continue;

			FString ParamName, ParamType;
			if (!InputObj->TryGetStringField(TEXT("name"), ParamName) ||
				!InputObj->TryGetStringField(TEXT("type"), ParamType))
				continue;

			FEdGraphPinType PinType = ResolvePinType(ParamType);

			TSharedPtr<FUserPinInfo> NewPin = MakeShareable(new FUserPinInfo());
			NewPin->PinName = FName(*ParamName);
			NewPin->PinType = PinType;
			NewPin->DesiredPinDirection = EGPD_Output; // Entry node outputs = macro inputs
			EntryNode->UserDefinedPins.Add(NewPin);
		}
		EntryNode->ReconstructNode();
	}

	// Add outputs
	const TArray<TSharedPtr<FJsonValue>>* OutputsArray = nullptr;
	if (Params->TryGetArrayField(TEXT("outputs"), OutputsArray) && ExitNode)
	{
		for (const auto& OutputVal : *OutputsArray)
		{
			auto OutputObj = OutputVal->AsObject();
			if (!OutputObj) continue;

			FString ParamName, ParamType;
			if (!OutputObj->TryGetStringField(TEXT("name"), ParamName) ||
				!OutputObj->TryGetStringField(TEXT("type"), ParamType))
				continue;

			FEdGraphPinType PinType = ResolvePinType(ParamType);

			TSharedPtr<FUserPinInfo> NewPin = MakeShareable(new FUserPinInfo());
			NewPin->PinName = FName(*ParamName);
			NewPin->PinType = PinType;
			NewPin->DesiredPinDirection = EGPD_Input; // Exit node inputs = macro outputs
			ExitNode->UserDefinedPins.Add(NewPin);
		}
		ExitNode->ReconstructNode();
	}

	FBlueprintEditorUtils::MarkBlueprintAsModified(BP);

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("blueprint"), BP->GetPathName());
	Result->SetStringField(TEXT("macro_name"), MacroName);
	Result->SetStringField(TEXT("graph_name"), NewGraph->GetName());

	if (EntryNode)
	{
		Result->SetStringField(TEXT("entry_node_id"), EntryNode->NodeGuid.ToString());
	}
	if (ExitNode)
	{
		Result->SetStringField(TEXT("exit_node_id"), ExitNode->NodeGuid.ToString());
	}

	return Result;
}

TSharedPtr<FJsonObject> FMCPTcpServer::HandleReadBPMacroGraph(const TSharedPtr<FJsonObject>& Params)
{
	FString BPPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BPPath))
	{
		return MCPError(TEXT("Missing required param: blueprint"));
	}

	FString MacroName;
	if (!Params->TryGetStringField(TEXT("macro_name"), MacroName))
	{
		return MCPError(TEXT("Missing required param: macro_name"));
	}

	UBlueprint* BP = FindBlueprintByPath(BPPath);
	if (!BP)
	{
		return MCPError(FString::Printf(TEXT("Blueprint not found: %s"), *BPPath));
	}

	UEdGraph* MacroGraph = nullptr;
	for (UEdGraph* Graph : BP->MacroGraphs)
	{
		if (Graph && Graph->GetName() == MacroName)
		{
			MacroGraph = Graph;
			break;
		}
	}

	if (!MacroGraph)
	{
		return MCPError(FString::Printf(TEXT("Macro not found: %s"), *MacroName));
	}

	auto Result = MCPSuccess();
	Result->SetStringField(TEXT("blueprint"), BP->GetPathName());
	Result->SetStringField(TEXT("macro_name"), MacroName);

	// Serialize all nodes (same pattern as read_blueprint_graph)
	TArray<TSharedPtr<FJsonValue>> NodeArray;
	for (UEdGraphNode* Node : MacroGraph->Nodes)
	{
		auto NodeObj = MakeShared<FJsonObject>();
		NodeObj->SetStringField(TEXT("id"), Node->NodeGuid.ToString());
		NodeObj->SetStringField(TEXT("class"), Node->GetClass()->GetName());
		NodeObj->SetStringField(TEXT("title"), Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
		NodeObj->SetNumberField(TEXT("x"), Node->NodePosX);
		NodeObj->SetNumberField(TEXT("y"), Node->NodePosY);
		NodeObj->SetStringField(TEXT("comment"), Node->NodeComment);

		// Pins
		TArray<TSharedPtr<FJsonValue>> PinArray;
		for (UEdGraphPin* Pin : Node->Pins)
		{
			auto PinObj = MakeShared<FJsonObject>();
			PinObj->SetStringField(TEXT("name"), Pin->PinName.ToString());
			PinObj->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
			PinObj->SetStringField(TEXT("type"), Pin->PinType.PinCategory.ToString());
			PinObj->SetStringField(TEXT("default"), Pin->DefaultValue);

			// Connections
			TArray<TSharedPtr<FJsonValue>> ConnArray;
			for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
			{
				auto ConnObj = MakeShared<FJsonObject>();
				ConnObj->SetStringField(TEXT("node_id"), LinkedPin->GetOwningNode()->NodeGuid.ToString());
				ConnObj->SetStringField(TEXT("pin_name"), LinkedPin->PinName.ToString());
				ConnArray.Add(MakeShared<FJsonValueObject>(ConnObj));
			}
			PinObj->SetArrayField(TEXT("connections"), ConnArray);

			PinArray.Add(MakeShared<FJsonValueObject>(PinObj));
		}
		NodeObj->SetArrayField(TEXT("pins"), PinArray);

		NodeArray.Add(MakeShared<FJsonValueObject>(NodeObj));
	}
	Result->SetArrayField(TEXT("nodes"), NodeArray);
	Result->SetNumberField(TEXT("node_count"), NodeArray.Num());

	return Result;
}
