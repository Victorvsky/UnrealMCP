// Copyright (c) 2026 victorvksy. All rights reserved.

// Transport regression tests. They talk to the live in-editor server over its real socket
// from a worker thread (the server needs the game thread to run commands, and the test
// itself is ticked on the game thread), so every test is latent.
//
// Run: Session Frontend -> Automation -> UnrealMCP.Transport.*, or from the console:
//   Automation RunTests UnrealMCP.Transport

#include "Tests/MCPTestClient.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MCPTransportTest = MCPTestClient;

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

static FMCPTcpServer* MCPTestServer(FAutomationTestBase& Test) { return MCPTestClient::Server(Test); }

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

// ------------------------------------------------------------------ receive cap
// A client that streams bytes with no newline must be answered with line_too_long and
// disconnected, instead of growing the receive buffer without bound.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCPTransportLineTooLongTest, "UnrealMCP.Transport.LineTooLong", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMCPTransportLineTooLongTest::RunTest(const FString& Parameters)
{
	FMCPTcpServer* Server = MCPTestServer(*this);
	if (!Server) return false;
	const int32 Port = Server->GetPort();
	const int64 SavedCap = Server->GetMaxLineBytes();
	Server->SetMaxLineBytes(1024 * 1024);

	auto Scenario = MakeShared<MCPTransportTest::FScenario>();
	Scenario->Deadline = FPlatformTime::Seconds() + 30.0;
	Scenario->Future = Async(EAsyncExecution::Thread, [Port, Scenario, Server, SavedCap]() -> bool
	{
		// 2 MB of JSON-looking bytes and no terminator: the server must not wait for one.
		FString Line = TEXT("{\"command\":\"ping\",\"params\":{\"payload\":\"");
		const int32 Count = 2 * 1024 * 1024;
		Line.Reserve(Count + 64);
		for (int32 i = 0; i < Count; ++i) Line.AppendChar(TEXT('x'));
		const FString Reply = MCPTransportTest::Exchange(Port, Line, 20.0, /*bAppendNewline*/ false);
		const FString Code = MCPTransportTest::ErrorCode(MCPTransportTest::Parse(Reply));
		bool bOk = true;
		if (Code != TEXT("line_too_long")) { Scenario->Errors.Add(FString::Printf(TEXT("expected line_too_long, got: %s"), *Reply.Left(200))); bOk = false; }

		// the server dropped that client and serves the next one normally
		const FString Again = MCPTransportTest::Exchange(Port, MCPTransportTest::Request(TEXT("ping"), nullptr), 10.0);
		TSharedPtr<FJsonObject> Obj = MCPTransportTest::Parse(Again);
		if (!Obj.IsValid() || !Obj->GetBoolField(TEXT("success"))) { Scenario->Errors.Add(FString::Printf(TEXT("expected recovery, got: %s"), *Again.Left(200))); bOk = false; }

		Server->SetMaxLineBytes(SavedCap);
		return bOk;
	});
	ADD_LATENT_AUTOMATION_COMMAND(FMCPWaitForScenario(Scenario, this));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
