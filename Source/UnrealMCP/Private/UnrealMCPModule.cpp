// Copyright (c) 2026 victorvksy. All rights reserved.

#include "UnrealMCPModule.h"
#include "MCPTcpServer.h"

IMPLEMENT_MODULE(FUnrealMCPModule, UnrealMCP)

FUnrealMCPModule* FUnrealMCPModule::Instance = nullptr;

void FUnrealMCPModule::StartupModule()
{
	Instance = this;
	UE_LOG(LogUnrealMCP, Log, TEXT("[UnrealMCP] Module starting up"));

	TcpServer = MakeShared<FMCPTcpServer>();
	if (TcpServer->Start())
	{
		UE_LOG(LogUnrealMCP, Log, TEXT("[UnrealMCP] TCP server started successfully"));
	}
	else
	{
		UE_LOG(LogUnrealMCP, Error, TEXT("[UnrealMCP] Failed to start TCP server"));
	}
}

void FUnrealMCPModule::ShutdownModule()
{
	UE_LOG(LogUnrealMCP, Log, TEXT("[UnrealMCP] Module shutting down"));

	if (TcpServer.IsValid())
	{
		TcpServer->Stop();
		TcpServer.Reset();
	}

	Instance = nullptr;
}

FMCPTcpServer* FUnrealMCPModule::GetServer()
{
	return Instance ? Instance->TcpServer.Get() : nullptr;
}
