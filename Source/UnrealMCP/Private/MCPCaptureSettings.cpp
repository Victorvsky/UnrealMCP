// Copyright (c) 2026 victorvksy. All rights reserved.

#include "MCPCaptureSettings.h"

UMCPCaptureSettings::UMCPCaptureSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName = TEXT("MCP Capture");
}

const UMCPCaptureSettings& UMCPCaptureSettings::Get()
{
	return *GetDefault<UMCPCaptureSettings>();
}
