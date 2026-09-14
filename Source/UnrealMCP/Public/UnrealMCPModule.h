// Copyright (c) 2026 victorvksy. All rights reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FMCPTcpServer;

class FUnrealMCPModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Get the TCP server instance for registering custom handlers from game modules. */
	static FMCPTcpServer* GetServer();

private:
	TSharedPtr<FMCPTcpServer> TcpServer;
	static FUnrealMCPModule* Instance;
};
