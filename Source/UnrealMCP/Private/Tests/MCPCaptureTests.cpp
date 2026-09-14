// Copyright (c) 2026 victorvksy. All rights reserved.

// capture_viewport through the real socket: an editor frame with the requested size and a
// decodable image, the structured errors for "pie" without PIE and for an oversized request,
// and the visible-actor list being populated by the projection/occlusion pass.
//
//   Automation RunTests UnrealMCP.Capture

#include "Tests/MCPTestClient.h"
#include "Capture/MCPCapture.h"
#include "Misc/Base64.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Editor.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	TSharedPtr<FJsonObject> Resolution(int32 W, int32 H)
	{
		auto R = MakeShared<FJsonObject>();
		R->SetNumberField(TEXT("w"), W);
		R->SetNumberField(TEXT("h"), H);
		return R;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCPCaptureEditorFrameTest, "UnrealMCP.Capture.EditorFrame", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMCPCaptureEditorFrameTest::RunTest(const FString& Parameters)
{
	FMCPTcpServer* Server = MCPTestClient::Server(*this);
	if (!Server) return false;
	const int32 Port = Server->GetPort();
	auto Scenario = MakeShared<MCPTestClient::FScenario>();
	Scenario->Deadline = FPlatformTime::Seconds() + 60.0;
	Scenario->Future = Async(EAsyncExecution::Thread, [Port, Scenario]() -> bool
	{
		auto Params = MakeShared<FJsonObject>();
		Params->SetStringField(TEXT("camera"), TEXT("editor"));
		Params->SetObjectField(TEXT("resolution"), Resolution(640, 360));
		const FString Reply = MCPTestClient::Exchange(Port, MCPTestClient::Request(TEXT("capture_viewport"), Params), 30.0);
		TSharedPtr<FJsonObject> Obj = MCPTestClient::Parse(Reply);
		if (!Obj.IsValid() || !Obj->GetBoolField(TEXT("success")))
		{
			Scenario->Errors.Add(FString::Printf(TEXT("capture failed: %s"), *Reply.Left(300)));
			return false;
		}
		bool bOk = true;
		const TSharedPtr<FJsonObject>* Image = nullptr;
		if (!Obj->TryGetObjectField(TEXT("image"), Image) || !Image || !Image->IsValid())
		{
			Scenario->Errors.Add(TEXT("no image object")); return false;
		}
		// The viewport keeps its own aspect ratio: the request is a bounding box, one edge is hit.
		const int32 W = (int32)(*Image)->GetNumberField(TEXT("width")), H = (int32)(*Image)->GetNumberField(TEXT("height"));
		if (W < 16 || H < 16 || W > 640 || H > 360 || (W != 640 && H != 360))
		{
			Scenario->Errors.Add(FString::Printf(TEXT("image size %dx%d does not fit the 640x360 request"), W, H)); bOk = false;
		}
		TArray<uint8> Bytes;
		if (!FBase64::Decode((*Image)->GetStringField(TEXT("data")), Bytes) || Bytes.Num() < 4 || Bytes[0] != 0xFF || Bytes[1] != 0xD8)
		{
			Scenario->Errors.Add(TEXT("image data is not a JPEG")); bOk = false;
		}
		if (!Obj->HasTypedField<EJson::Array>(TEXT("actors")) || !Obj->HasTypedField<EJson::Object>(TEXT("camera")))
		{
			Scenario->Errors.Add(TEXT("missing actors[] or camera")); bOk = false;
		}
		const TSharedPtr<FJsonObject>* Timings = nullptr;
		if (Obj->TryGetObjectField(TEXT("timings"), Timings) && Timings && Timings->IsValid())
		{
			UE_LOG(LogTemp, Display, TEXT("[UnrealMCP.Capture] %dx%d jpeg (640x360 requested): game thread capture %.1f ms, actors %.1f ms, encode %.1f ms, %d bytes"),
				W, H, (*Timings)->GetNumberField(TEXT("game_thread_capture_ms")), (*Timings)->GetNumberField(TEXT("game_thread_actors_ms")),
				(*Timings)->GetNumberField(TEXT("encode_ms")), Bytes.Num());
		}
		return bOk;
	});
	ADD_LATENT_AUTOMATION_COMMAND(FMCPWaitForScenario(Scenario, this));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCPCapturePieNotRunningTest, "UnrealMCP.Capture.PieNotRunning", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMCPCapturePieNotRunningTest::RunTest(const FString& Parameters)
{
	if (GEditor && GEditor->PlayWorld)
	{
		AddInfo(TEXT("PIE is running; skipping the no-PIE error check"));
		return true;
	}
	FMCPTcpServer* Server = MCPTestClient::Server(*this);
	if (!Server) return false;
	const int32 Port = Server->GetPort();
	auto Scenario = MakeShared<MCPTestClient::FScenario>();
	Scenario->Deadline = FPlatformTime::Seconds() + 30.0;
	Scenario->Future = Async(EAsyncExecution::Thread, [Port, Scenario]() -> bool
	{
		auto Params = MakeShared<FJsonObject>();
		Params->SetStringField(TEXT("camera"), TEXT("pie"));
		const FString Reply = MCPTestClient::Exchange(Port, MCPTestClient::Request(TEXT("capture_viewport"), Params), 20.0);
		const FString Code = MCPTestClient::ErrorCode(MCPTestClient::Parse(Reply));
		if (Code != TEXT("pie_not_running"))
		{
			Scenario->Errors.Add(FString::Printf(TEXT("expected pie_not_running, got: %s"), *Reply.Left(300)));
			return false;
		}
		return true;
	});
	ADD_LATENT_AUTOMATION_COMMAND(FMCPWaitForScenario(Scenario, this));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCPCaptureResolutionCapTest, "UnrealMCP.Capture.ResolutionCap", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMCPCaptureResolutionCapTest::RunTest(const FString& Parameters)
{
	FMCPTcpServer* Server = MCPTestClient::Server(*this);
	if (!Server) return false;
	const int32 Port = Server->GetPort();
	auto Scenario = MakeShared<MCPTestClient::FScenario>();
	Scenario->Deadline = FPlatformTime::Seconds() + 30.0;
	Scenario->Future = Async(EAsyncExecution::Thread, [Port, Scenario]() -> bool
	{
		auto Params = MakeShared<FJsonObject>();
		Params->SetStringField(TEXT("camera"), TEXT("editor"));
		Params->SetObjectField(TEXT("resolution"), Resolution(4096, 4096));
		const FString Reply = MCPTestClient::Exchange(Port, MCPTestClient::Request(TEXT("capture_viewport"), Params), 20.0);
		const FString Code = MCPTestClient::ErrorCode(MCPTestClient::Parse(Reply));
		if (Code != TEXT("resolution_too_large"))
		{
			Scenario->Errors.Add(FString::Printf(TEXT("expected resolution_too_large, got: %s"), *Reply.Left(300)));
			return false;
		}
		return true;
	});
	ADD_LATENT_AUTOMATION_COMMAND(FMCPWaitForScenario(Scenario, this));
	return true;
}

// The projection/occlusion pass on its own (no socket): an actor placed in front of a camera
// is listed with a sane box, and the same actor behind the camera is not.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMCPCaptureVisibleActorsTest, "UnrealMCP.Capture.VisibleActors", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FMCPCaptureVisibleActorsTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		AddError(TEXT("no editor world"));
		return false;
	}
	// Any actor with rendered bounds will do: pick the first one the pass accepts from far away
	MCPCapture::FView View;
	View.World = World;
	View.FOV = 90.f;
	// ...as long as it renders: a static mesh actor with mid-sized bounds (a light or a volume
	// has bounds but nothing the pass would list)
	AActor* Subject = nullptr;
	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		FVector O, E;
		It->GetActorBounds(false, O, E);
		const UStaticMeshComponent* Mesh = It->GetStaticMeshComponent();
		if (!It->IsHiddenEd() && Mesh && Mesh->GetStaticMesh() && Mesh->IsVisibleInEditor() && E.Size() > 50.f && E.Size() < 2000.f)
		{
			Subject = *It;
			break;
		}
	}
	if (!Subject)
	{
		AddInfo(TEXT("no suitable actor in the level; skipping"));
		return true;
	}
	FVector Origin, Extent;
	Subject->GetActorBounds(false, Origin, Extent);
	const double Back = Extent.Size() * 3.0 + 200.0;

	View.Location = Origin - FVector(Back, 0, 0);
	View.Rotation = FRotator::ZeroRotator; // looking down +X at the subject
	TArray<MCPCapture::FVisibleActor> InFront = MCPCapture::FindVisibleActors(View, 640, 360, 500);
	const MCPCapture::FVisibleActor* Found = InFront.FindByPredicate([&](const MCPCapture::FVisibleActor& A) { return A.Name == Subject->GetActorNameOrLabel(); });
	if (!Found)
	{
		AddError(FString::Printf(TEXT("%s in front of the camera was not listed"), *Subject->GetActorNameOrLabel()));
		return false;
	}
	TestTrue(TEXT("box has area"), Found->ScreenBox.Width() > 0 && Found->ScreenBox.Height() > 0);
	TestTrue(TEXT("box is inside the image"), Found->ScreenBox.Min.X >= 0 && Found->ScreenBox.Max.X <= 640 && Found->ScreenBox.Min.Y >= 0 && Found->ScreenBox.Max.Y <= 360);
	TestTrue(TEXT("nearest first"), InFront[0].Distance <= Found->Distance);

	View.Rotation = FRotator(0, 180, 0); // looking away from it
	TArray<MCPCapture::FVisibleActor> Behind = MCPCapture::FindVisibleActors(View, 640, 360, 500);
	TestNull(TEXT("actor behind the camera is culled"), Behind.FindByPredicate([&](const MCPCapture::FVisibleActor& A) { return A.Name == Subject->GetActorNameOrLabel(); }));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
