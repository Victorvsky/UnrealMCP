#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "HAL/Runnable.h"
#include "Sockets.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnrealMCP, Log, All);

class FMCPTcpServer : public FRunnable
{
public:
	FMCPTcpServer();
	virtual ~FMCPTcpServer();

	bool Start();
	void Stop();

	using FCommandHandler = TFunction<TSharedPtr<FJsonObject>(const TSharedPtr<FJsonObject>& Params)>;
	void RegisterHandler(const FString& CommandName, FCommandHandler Handler);

	// FRunnable interface
	virtual uint32 Run() override;
	virtual void Exit() override;

private:
	void HandleClient(FSocket* ClientSocket);
	void ProcessMessage(const FString& Message, FSocket* ClientSocket);
	void SendResponse(FSocket* ClientSocket, const TSharedPtr<FJsonObject>& Response);
	void WritePortFile(int32 Port);

	void RegisterActorHandlers();
	void RegisterBlueprintHandlers();
	void RegisterLevelHandlers();
	void RegisterLandscapeHandlers();
	void RegisterPlaytestHandlers();

	// Actor command implementations
	TSharedPtr<FJsonObject> HandleListActors(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSpawnActor(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleDeleteActor(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSetActorTransform(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleGetProperty(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSetProperty(const TSharedPtr<FJsonObject>& Params);

	// Blueprint command implementations
	TSharedPtr<FJsonObject> HandleListBlueprints(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleReadBPGraph(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddBPNode(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleConnectPins(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleDisconnectPins(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleCompileBP(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddBPVariable(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleRemoveBPNode(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSetPinDefault(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddBPFunction(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleRemoveBPVariable(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddEventDispatcher(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddBPComponent(const TSharedPtr<FJsonObject>& Params);

	// Level command implementations
	TSharedPtr<FJsonObject> HandleLevelInfo(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleFindByClass(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleFindInRadius(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleScreenshot(const TSharedPtr<FJsonObject>& Params);

	// Landscape command implementations
	TSharedPtr<FJsonObject> HandleLandscapeInfo(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleCreateLandscape(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSculptLandscape(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSculptNoise(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSetHeightAt(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleGetHeightAt(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleImportHeightmap(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandlePaintLayer(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddLayer(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandlePlaceFoliage(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleClearFoliage(const TSharedPtr<FJsonObject>& Params);

	// Material command implementations
	TSharedPtr<FJsonObject> HandleGetMaterialParams(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSetMaterialParam(const TSharedPtr<FJsonObject>& Params);

	void RegisterMaterialHandlers();
	void RegisterEditorUtilityHandlers();
	void RegisterComponentHandlers();
	void RegisterWidgetHandlers();
	void RegisterAssetHandlers();
	void RegisterDataTableHandlers();
	void RegisterSequencerHandlers();
	void RegisterWidgetAnimationHandlers();
	void RegisterBlueprintMacroHandlers();

#if WITH_NIAGARA
	void RegisterNiagaraHandlers();
#endif

	// Editor utility command implementations
	TSharedPtr<FJsonObject> HandleExecuteConsoleCommand(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSaveAsset(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSaveCurrentLevel(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleUndo(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleRedo(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleCreateBlueprint(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleDuplicateAsset(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleDeleteAsset(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleRenameAsset(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleOpenLevel(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleListAssets(const TSharedPtr<FJsonObject>& Params);

	// Component command implementations
	TSharedPtr<FJsonObject> HandleListComponents(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddActorComponent(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleRemoveActorComponent(const TSharedPtr<FJsonObject>& Params);

	// Widget/UMG command implementations
	TSharedPtr<FJsonObject> HandleListWidgetChildren(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddWidgetChild(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleRemoveWidgetChild(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSetWidgetProperty(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleReadWidgetTree(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSetWidgetSlot(const TSharedPtr<FJsonObject>& Params);

	// DataTable command implementations
	TSharedPtr<FJsonObject> HandleListDataTables(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleReadDataTable(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddDataTableRow(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleRemoveDataTableRow(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleEditDataTableRow(const TSharedPtr<FJsonObject>& Params);

#if WITH_NIAGARA
	// Niagara command implementations
	TSharedPtr<FJsonObject> HandleListNiagaraSystems(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleReadNiagaraSystem(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleGetNiagaraEmitterProperties(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSetNiagaraParameter(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSetNiagaraEmitterEnabled(const TSharedPtr<FJsonObject>& Params);
#endif

	// Sequencer command implementations
	TSharedPtr<FJsonObject> HandleListSequences(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleReadSequence(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddSequenceTrack(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleRemoveSequenceTrack(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleSetSequencePlaybackRange(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddSequenceBinding(const TSharedPtr<FJsonObject>& Params);

	// Widget animation command implementations
	TSharedPtr<FJsonObject> HandleListWidgetAnimations(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleReadWidgetAnimation(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleCreateWidgetAnimation(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddWidgetAnimationTrack(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleAddWidgetAnimationKeyframe(const TSharedPtr<FJsonObject>& Params);

	// Blueprint macro command implementations
	TSharedPtr<FJsonObject> HandleListBPMacros(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleCreateBPMacro(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleReadBPMacroGraph(const TSharedPtr<FJsonObject>& Params);

	// Asset import command implementations
	TSharedPtr<FJsonObject> HandleImportAsset(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleCreateMaterialInstance(const TSharedPtr<FJsonObject>& Params);

	// Playtest command implementations
	TSharedPtr<FJsonObject> HandleStartPIE(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleStopPIE(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleGetPIEStatus(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleKeyInput(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleMouseMove(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleMouseClick(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleGetGameState(const TSharedPtr<FJsonObject>& Params);
	TSharedPtr<FJsonObject> HandleCallComponentFunction(const TSharedPtr<FJsonObject>& Params);

	FSocket* ListenerSocket = nullptr;
	FRunnableThread* Thread = nullptr;
	FThreadSafeBool bRunning = false;

	TMap<FString, FCommandHandler> CommandHandlers;
	FCriticalSection HandlersMutex;
};
