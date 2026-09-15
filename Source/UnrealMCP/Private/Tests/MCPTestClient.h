// Copyright (c) 2026 victorvksy. All rights reserved.

// Shared helpers for tests that talk to the live in-editor server over its real socket from a
// worker thread. The server needs the game thread to run commands and the tests are ticked on
// the game thread, so every test is latent: it spawns a worker, then waits for its future.

#pragma once

#include "CoreMinimal.h"
#include "MCPTcpServer.h"
#include "UnrealMCPModule.h"
#include "Misc/AutomationTest.h"
#include "Async/Async.h"
#include "Async/Future.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "IPAddress.h"
#include "HAL/PlatformProcess.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MCPTestClient
{
	/** Open a fresh connection, send one line, read one line, close. Blocking: call off the game thread. */
	inline FString Exchange(int32 Port, const FString& Line, double TimeoutSec, bool bAppendNewline = true)
	{
		ISocketSubsystem* SS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		FSocket* Sock = SS->CreateSocket(NAME_Stream, TEXT("MCPTestClient"), false);
		if (!Sock)
		{
			return TEXT("{\"test_error\":\"no socket\"}");
		}
		TSharedRef<FInternetAddr> Addr = SS->CreateInternetAddr();
		bool bOk = false;
		Addr->SetIp(TEXT("127.0.0.1"), bOk);
		Addr->SetPort(Port);
		// The server accepts one client at a time and needs a moment to notice the previous FIN.
		const double ConnectDeadline = FPlatformTime::Seconds() + TimeoutSec;
		while (!Sock->Connect(*Addr))
		{
			if (FPlatformTime::Seconds() > ConnectDeadline)
			{
				SS->DestroySocket(Sock);
				return TEXT("{\"test_error\":\"connect timed out\"}");
			}
			FPlatformProcess::Sleep(0.02f);
		}

		FTCHARToUTF8 Utf8(*(bAppendNewline ? Line + TEXT("\n") : Line));
		int32 Offset = 0;
		while (Offset < Utf8.Length())
		{
			int32 Sent = 0;
			if (!Sock->Send(reinterpret_cast<const uint8*>(Utf8.Get()) + Offset, Utf8.Length() - Offset, Sent) || Sent <= 0)
			{
				SS->DestroySocket(Sock);
				return TEXT("{\"test_error\":\"send failed\"}");
			}
			Offset += Sent;
		}

		TArray<uint8> Bytes;
		uint8 Chunk[65536];
		const double Deadline = FPlatformTime::Seconds() + TimeoutSec;
		while (FPlatformTime::Seconds() < Deadline)
		{
			uint32 PendingSize = 0;
			if (Sock->HasPendingData(PendingSize))
			{
				int32 Read = 0;
				if (!Sock->Recv(Chunk, sizeof(Chunk), Read) || Read <= 0)
				{
					break;
				}
				Bytes.Append(Chunk, Read);
				if (Bytes.Last() == '\n')
				{
					break;
				}
			}
			else
			{
				FPlatformProcess::Sleep(0.005f);
			}
		}
		Sock->Close();
		SS->DestroySocket(Sock);
		if (Bytes.Num() == 0)
		{
			return TEXT("{\"test_error\":\"no response\"}");
		}
		FUTF8ToTCHAR Conv(reinterpret_cast<const ANSICHAR*>(Bytes.GetData()), Bytes.Num());
		return FString(Conv.Length(), Conv.Get()).TrimEnd();
	}

	inline TSharedPtr<FJsonObject> Parse(const FString& Line)
	{
		TSharedPtr<FJsonObject> Obj;
		auto Reader = TJsonReaderFactory<>::Create(Line);
		FJsonSerializer::Deserialize(Reader, Obj);
		return Obj;
	}

	inline FString ErrorCode(const TSharedPtr<FJsonObject>& Obj)
	{
		const TSharedPtr<FJsonObject>* Err = nullptr;
		if (Obj.IsValid() && Obj->TryGetObjectField(TEXT("error"), Err) && Err && Err->IsValid())
		{
			return (*Err)->GetStringField(TEXT("code"));
		}
		return FString();
	}

	inline FString Request(const FString& Command, const TSharedPtr<FJsonObject>& Params)
	{
		auto Msg = MakeShared<FJsonObject>();
		Msg->SetStringField(TEXT("command"), Command);
		Msg->SetObjectField(TEXT("params"), Params.IsValid() ? Params : MakeShared<FJsonObject>());
		FString Out;
		auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Out);
		FJsonSerializer::Serialize(Msg, Writer);
		return Out;
	}

	/** A worker-thread scenario and the errors it collected (reported on the game thread). */
	struct FScenario
	{
		TFuture<bool> Future;
		double Deadline = 0.0;
		TArray<FString> Errors; // written by the worker before the future resolves, read after
	};

	inline FMCPTcpServer* Server(FAutomationTestBase& Test)
	{
		FMCPTcpServer* S = FUnrealMCPModule::GetServer();
		if (!S || S->GetPort() <= 0)
		{
			Test.AddError(TEXT("UnrealMCP server is not running"));
			return nullptr;
		}
		return S;
	}
}

// A latent command that waits for a worker-thread future, so the game thread keeps ticking
// (the server needs it to run commands) while the socket work happens elsewhere.
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FMCPWaitForScenario, TSharedPtr<MCPTestClient::FScenario>, Scenario, FAutomationTestBase*, Test);

#endif // WITH_DEV_AUTOMATION_TESTS
