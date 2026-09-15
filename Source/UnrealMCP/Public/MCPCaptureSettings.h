// Copyright (c) 2026 victorvksy. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MCPCaptureSettings.generated.h"

/**
 * Project Settings -> Plugins -> MCP Capture. Defaults for the visual-capture tools
 * (capture_viewport, record_pie, ...). Stored in DefaultEditor.ini.
 */
UCLASS(config = Editor, defaultconfig, meta = (DisplayName = "MCP Capture"))
class UNREALMCP_API UMCPCaptureSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UMCPCaptureSettings();

	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }

	/** Capture size when a tool call does not specify one. */
	UPROPERTY(config, EditAnywhere, Category = "Capture", meta = (ClampMin = 16, ClampMax = 4096))
	int32 DefaultWidth = 1024;

	UPROPERTY(config, EditAnywhere, Category = "Capture", meta = (ClampMin = 16, ClampMax = 4096))
	int32 DefaultHeight = 576;

	/** JPEG quality for captures returned as jpeg (1-100). */
	UPROPERTY(config, EditAnywhere, Category = "Capture", meta = (ClampMin = 1, ClampMax = 100))
	int32 JpegQuality = 80;

	/** Requests with a longer edge than this are rejected with resolution_too_large. */
	UPROPERTY(config, EditAnywhere, Category = "Capture", meta = (ClampMin = 64, ClampMax = 8192))
	int32 MaxLongEdge = 2048;

	/** Cap on the visible-actor list and on the occlusion line traces per capture: the largest on-screen boxes are traced, the rest are culled as over_limit. The returned list is nearest first. */
	UPROPERTY(config, EditAnywhere, Category = "Capture", meta = (ClampMin = 0, ClampMax = 2000))
	int32 MaxActors = 200;

	/** Recording sessions live under <Project>/Saved/<StoragePath>/<session_id>/. */
	UPROPERTY(config, EditAnywhere, Category = "Recording")
	FString StoragePath = TEXT("MCPRecordings");

	/** Sessions older than this are pruned on editor startup (0 = never). */
	UPROPERTY(config, EditAnywhere, Category = "Recording", meta = (ClampMin = 0, ClampMax = 365))
	int32 RetentionDays = 7;

	static const UMCPCaptureSettings& Get();
};
