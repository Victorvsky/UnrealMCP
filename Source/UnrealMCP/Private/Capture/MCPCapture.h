// Copyright (c) 2026 victorvksy. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class FMCPTcpServer;
class UWorld;
class FViewport;

/**
 * Visual capture: a frame of the editor viewport, the PIE camera, or an arbitrary camera,
 * returned with the engine state that explains it (camera, visible actors). See
 * docs/visual-capture/ARCHITECTURE.md for the threading contract: everything in here that
 * touches the world runs on the game thread; encoding runs on the socket thread through
 * FMCPTcpServer::QueuePostProcess.
 */
namespace MCPCapture
{
	/** A resolved camera for one capture. */
	struct FView
	{
		FVector Location = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		float FOV = 90.f;      // horizontal, degrees
		UWorld* World = nullptr;
		bool bIsPIE = false;
		/** For "editor"/"pie": the live viewport whose last frame is read back (every feature the
		 *  user sees - Lumen, post process, exposure). Null for explicit cameras, which render
		 *  through a scene capture. */
		FViewport* Viewport = nullptr;
	};

	/** One visible actor as reported to the client. */
	struct FVisibleActor
	{
		FString Name;
		FString Class;
		FVector WorldLocation;
		double Distance = 0.0;
		FIntRect ScreenBox;   // pixels, clamped to the image
	};

	/** Registers capture_viewport with the server. Called from FMCPTcpServer::Start(). */
	void RegisterHandlers(FMCPTcpServer& Server);

	/** Why an actor was left out (returned when the caller asks for debug output). */
	struct FCulledActor
	{
		FString Name;
		FString Reason; // no_rendered_mesh | behind_camera | off_screen | occluded_by:<actor>
	};

	/** Actors whose bounds project into a Width x Height image seen from View, nearest first.
	 *  Off-screen actors are culled; a line trace to the bounds centre culls the fully occluded.
	 *  OutCulled, when given, receives every skipped actor with the reason. */
	TArray<FVisibleActor> FindVisibleActors(const FView& View, int32 Width, int32 Height, int32 MaxActors, TArray<FCulledActor>* OutCulled = nullptr);
}
