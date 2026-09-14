// Copyright (c) 2026 victorvksy. All rights reserved.

// Transport regression tests. They talk to the live in-editor server over its real socket
// from a worker thread (the server needs the game thread to run commands, and the test
// itself is ticked on the game thread), so every test is latent.
//
// Run: Session Frontend -> Automation -> UnrealMCP.Transport.*, or from the console:
//   Automation RunTests UnrealMCP.Transport

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

namespace MCPTransportTest
{
	/** Open a fresh connection, send one line, read one line, close. Blocking: call off the game thread. */
	static FString Exchange(int32 Port, const FString& Line, double TimeoutSec)
	{
		ISocketSubsystem* SS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		FSocket* Sock = SS->CreateSocket(NAME_Stream, TEXT("MCPTransportTest"), false);
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

		FTCHARToUTF8 Utf8(*(Line + TEXT("\n")));
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

	static TSharedPtr<FJsonObject> Parse(const FString& Line)
	{
		TSharedPtr<FJsonObject> Obj;
		auto Reader = TJsonReaderFactory<>::Create(Line);
		FJsonSerializer::Deserialize(Reader, Obj);
		return Obj;
	}

	static FString ErrorCode(const TSharedPtr<FJsonObject>& Obj)
	{
		const TSharedPtr<FJsonObject>* Err = nullptr;
		if (Obj.IsValid() && Obj->TryGetObjectField(TEXT("error"), Err) && Err && Err->IsValid())
		{
			return (*Err)->GetStringField(TEXT("code"));
		}
		return FString();
	}

	static FString Request(const FString& Command, const TSharedPtr<FJsonObject>& Params)
	{
		auto Msg = MakeShared<FJsonObject>();
		Msg->SetStringField(TEXT("command"), Command);
		Msg->SetObjectField(TEXT("params"), Params.IsValid() ? Params : MakeShared<FJsonObject>());
		FString Out;
		auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Out);
		FJsonSerializer::Serialize(Msg, Writer);
		return Out;
	}

	/** Runs a blocking scenario on a worker thread and finishes the latent test when it is done. */
	struct FScenario
	{
		TFuture<bool> Future;
		double Deadline = 0.0;
		TArray<FString> Errors; // written by the worker before the future resolves, read after
	};
}

// A latent command that waits for a worker-thread future, so the game thread keeps ticking
// (the server needs it to run commands) while the socket work happens elsewhere.
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FMCPWaitForScenario, TSharedPtr<MCPTransportTest::FScenario>, Scenario, FAutomationTestBase*, Test);
bool FMCPWaitForScenario::Update()
{
	if (!Scenario->Future.IsReady())
	{
		if (FPlatformTime::Seconds() > Scenario->Deadline)
		{
			Test->AddError(TEXT("scenario did not finish in time"));
			return true;
		}
		return false;
	}
	for (const FString& E : Scenario->Errors)
	{
		Test->AddError(E);
	}
	if (!Scenario->Future.Get() && Scenario->Errors.Num() == 0)
	{
		Test->AddError(TEXT("scenario reported failure"));
	}
	return true;
}

static FMCPTcpServer* MCPTestServer(FAutomationTestBase& Test)
{
	FMCPTcpServer* Server = FUnrealMCPModule::GetServer();
	if (!Server || Server->GetPort() <= 0)
	{
		Test.AddError(TEXT("UnrealMCP server is not running"));
		return nullptr;
	}
	return Server;
}

// ------------------------------------------------------------------ smoke: the harness runs
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCPTransportSmokeTest, "UnrealMCP.Transport.Smoke", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMCPTransportSmokeTest::RunTest(const FString& Parameters)
{
	FMCPTcpServer* Server = MCPTestServer(*this);
	if (!Server) return false;
	const int32 Port = Server->GetPort();
	auto Scenario = MakeShared<MCPTransportTest::FScenario>();
	Scenario->Deadline = FPlatformTime::Seconds() + 20.0;
	Scenario->Future = Async(EAsyncExecution::Thread, [Port, Scenario]() -> bool
	{
		const FString Reply = MCPTransportTest::Exchange(Port, MCPTransportTest::Request(TEXT("ping"), nullptr), 10.0);
		TSharedPtr<FJsonObject> Obj = MCPTransportTest::Parse(Reply);
		const bool bOk = Obj.IsValid() && Obj->GetBoolField(TEXT("success")) && Obj->GetStringField(TEXT("reply")) == TEXT("pong");
		if (!bOk) Scenario->Errors.Add(FString::Printf(TEXT("ping reply: %s"), *Reply.Left(200)));
		return bOk;
	});
	ADD_LATENT_AUTOMATION_COMMAND(FMCPWaitForScenario(Scenario, this));
	return true;
}

// ------------------------------------------------------------------ 5 MB line with multi-byte UTF-8
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCPTransportLargeLineTest, "UnrealMCP.Transport.LargeLine", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMCPTransportLargeLineTest::RunTest(const FString& Parameters)
{
	FMCPTcpServer* Server = MCPTestServer(*this);
	if (!Server) return false;
	const int32 Port = Server->GetPort();
	auto Scenario = MakeShared<MCPTransportTest::FScenario>();
	Scenario->Deadline = FPlatformTime::Seconds() + 60.0;
	Scenario->Future = Async(EAsyncExecution::Thread, [Port, Scenario]() -> bool
	{
		// 5 MB of ASCII, then a 2-byte and a 3-byte character; the regression this guards
		// against truncated the message at the first multi-byte sequence.
		const int32 Count = 5 * 1024 * 1024;
		FString Payload;
		Payload.Reserve(Count + 2);
		for (int32 i = 0; i < Count; ++i) Payload.AppendChar(TEXT('a') + (i % 26));
		Payload.AppendChar(static_cast<TCHAR>(0x00E9));   // e-acute, 2 bytes in UTF-8
		Payload.AppendChar(static_cast<TCHAR>(0x2192));   // right arrow, 3 bytes in UTF-8
		auto Params = MakeShared<FJsonObject>();
		Params->SetStringField(TEXT("payload"), Payload);
		const FString Reply = MCPTransportTest::Exchange(Port, MCPTransportTest::Request(TEXT("ping"), Params), 45.0);
		TSharedPtr<FJsonObject> Obj = MCPTransportTest::Parse(Reply);
		if (!Obj.IsValid() || !Obj->GetBoolField(TEXT("success")))
		{
			Scenario->Errors.Add(FString::Printf(TEXT("large ping failed: %s"), *Reply.Left(200)));
			return false;
		}
		const int32 Len = static_cast<int32>(Obj->GetNumberField(TEXT("payload_length")));
		const FString Tail = Obj->GetStringField(TEXT("payload_tail"));
		if (Len != Count + 2) Scenario->Errors.Add(FString::Printf(TEXT("payload_length %d, expected %d"), Len, Count + 2));
		const FString Arrow = FString::Chr(static_cast<TCHAR>(0x2192));
		if (Tail != Arrow) Scenario->Errors.Add(FString::Printf(TEXT("payload_tail '%s', expected the arrow"), *Tail));
		return Len == Count + 2 && Tail == Arrow;
	});
	ADD_LATENT_AUTOMATION_COMMAND(FMCPWaitForScenario(Scenario, this));
	return true;
}

// ------------------------------------------------------------------ timeout isolation
// A command that outlives the wait must (a) be answered with a structured timeout error,
// (b) make the next command fail fast with "busy" rather than run on top of it, and
// (c) release the server once it finishes.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCPTransportTimeoutIsolationTest, "UnrealMCP.Transport.TimeoutIsolation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMCPTransportTimeoutIsolationTest::RunTest(const FString& Parameters)
{
	FMCPTcpServer* Server = MCPTestServer(*this);
	if (!Server) return false;
	const int32 Port = Server->GetPort();
	const int32 SavedTimeout = Server->GetCommandTimeoutMs();
	Server->SetCommandTimeoutMs(200);
	Server->RegisterHandler(TEXT("__test_block"), [](const TSharedPtr<FJsonObject>& Params)
	{
		FPlatformProcess::Sleep(1.2f); // deliberately stalls the game thread
		auto R = MakeShared<FJsonObject>();
		R->SetBoolField(TEXT("success"), true);
		return R;
	});

	auto Scenario = MakeShared<MCPTransportTest::FScenario>();
	Scenario->Deadline = FPlatformTime::Seconds() + 30.0;
	Scenario->Future = Async(EAsyncExecution::Thread, [Port, Scenario, Server, SavedTimeout]() -> bool
	{
		bool bOk = true;
		// (a) the blocking command times out with a structured error
		FString Reply = MCPTransportTest::Exchange(Port, MCPTransportTest::Request(TEXT("__test_block"), nullptr), 10.0);
		FString Code = MCPTransportTest::ErrorCode(MCPTransportTest::Parse(Reply));
		if (Code != TEXT("timeout")) { Scenario->Errors.Add(FString::Printf(TEXT("expected timeout, got: %s"), *Reply.Left(200))); bOk = false; }

		// (b) while it is still running, a new command is refused with "busy" (answered by the
		//     socket thread, so it cannot be waiting on the blocked game thread)
		Reply = MCPTransportTest::Exchange(Port, MCPTransportTest::Request(TEXT("ping"), nullptr), 5.0);
		Code = MCPTransportTest::ErrorCode(MCPTransportTest::Parse(Reply));
		if (Code != TEXT("busy")) { Scenario->Errors.Add(FString::Printf(TEXT("expected busy, got: %s"), *Reply.Left(200))); bOk = false; }

		// (c) once the stale command finishes the server is usable again
		FPlatformProcess::Sleep(1.5f);
		Reply = MCPTransportTest::Exchange(Port, MCPTransportTest::Request(TEXT("ping"), nullptr), 10.0);
		TSharedPtr<FJsonObject> Obj = MCPTransportTest::Parse(Reply);
		if (!Obj.IsValid() || !Obj->GetBoolField(TEXT("success"))) { Scenario->Errors.Add(FString::Printf(TEXT("expected recovery, got: %s"), *Reply.Left(200))); bOk = false; }
		if (Server->IsStaleCommandRunning()) { Scenario->Errors.Add(TEXT("stale flag still set after the command finished")); bOk = false; }

		Server->SetCommandTimeoutMs(SavedTimeout);
		Server->UnregisterHandler(TEXT("__test_block"));
		return bOk;
	});
	ADD_LATENT_AUTOMATION_COMMAND(FMCPWaitForScenario(Scenario, this));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
