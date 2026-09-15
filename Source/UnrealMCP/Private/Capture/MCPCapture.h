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
		FString Reason; // no_rendered_mesh | behind_camera | off_screen | over_limit | occluded_by:<actor>
	};

	/** Structured error object: {"success": false, "error": {code, message, hint}}. */
	TSharedPtr<FJsonObject> MakeError(const FString& Code, const FString& Message, const FString& Hint = FString());

	/** The PIE player's view (camera, world, viewport). Returns an error object when there is no
	 *  PIE session or player yet, null on success. Game thread. */
	TSharedPtr<FJsonObject> ResolvePIEView(FView& Out);

	/** Shrinks a requested size to the source's aspect ratio (both edges stay >= 16). */
	void FitSize(int32 SrcW, int32 SrcH, int32& InOutW, int32& InOutH);

	/** Box-filter resize in place (a no-op when the sizes match) that also makes the buffer
	 *  opaque. Any thread. */
	void BoxResize(TArray<FColor>& Pixels, int32 SrcW, int32 SrcH, int32 DstW, int32 DstH);

	/** JPEG ("jpeg", Quality 1-100) or PNG ("png") encode of a BGRA buffer. Any thread. */
	bool EncodeImage(const TArray<FColor>& Pixels, int32 Width, int32 Height, const FString& Format, int32 Quality, TArray64<uint8>& OutBytes);

	/** Actors whose bounds project into a Width x Height image seen from View, nearest first.
	 *  Off-screen actors are culled; a line trace to the bounds centre culls the fully occluded.
	 *  At most MaxActors traces run (the largest on-screen boxes first); further candidates are
	 *  culled as over_limit without a trace, so the trace count is bounded by the request, not
	 *  the level (the projection pass still visits every actor). OutCulled, when given, receives
	 *  every skipped actor with the reason; with MaxActors 0 that is every candidate. */
	TArray<FVisibleActor> FindVisibleActors(const FView& View, int32 Width, int32 Height, int32 MaxActors, TArray<FCulledActor>* OutCulled = nullptr);
}
