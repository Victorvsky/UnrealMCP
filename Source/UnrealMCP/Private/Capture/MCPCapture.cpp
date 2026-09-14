// Copyright (c) 2026 victorvksy. All rights reserved.

#include "Capture/MCPCapture.h"
#include "MCPTcpServer.h"
#include "MCPCaptureSettings.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/PrimitiveComponent.h"
#include "Components/MeshComponent.h"
#include "Components/BrushComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Info.h"
#include "GameFramework/Volume.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "LevelEditorViewport.h"
#include "SLevelViewport.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/Base64.h"
#include "Misc/DateTime.h"
#include "Modules/ModuleManager.h"
#include "TextureResource.h"
#include "UnrealClient.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

DEFINE_LOG_CATEGORY_STATIC(LogMCPCapture, Log, All);

namespace
{
	TSharedPtr<FJsonObject> CaptureError(const FString& Code, const FString& Message, const FString& Hint = FString())
	{
		auto Err = MakeShared<FJsonObject>();
		Err->SetStringField(TEXT("code"), Code);
		Err->SetStringField(TEXT("message"), Message);
		if (!Hint.IsEmpty())
		{
			Err->SetStringField(TEXT("hint"), Hint);
		}
		auto R = MakeShared<FJsonObject>();
		R->SetBoolField(TEXT("success"), false);
		R->SetObjectField(TEXT("error"), Err);
		return R;
	}

	TSharedPtr<FJsonObject> VectorJson(const FVector& V)
	{
		auto O = MakeShared<FJsonObject>();
		O->SetNumberField(TEXT("x"), V.X);
		O->SetNumberField(TEXT("y"), V.Y);
		O->SetNumberField(TEXT("z"), V.Z);
		return O;
	}

	TSharedPtr<FJsonObject> RotatorJson(const FRotator& R)
	{
		auto O = MakeShared<FJsonObject>();
		O->SetNumberField(TEXT("pitch"), R.Pitch);
		O->SetNumberField(TEXT("yaw"), R.Yaw);
		O->SetNumberField(TEXT("roll"), R.Roll);
		return O;
	}

	bool ReadVector(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field, FVector& Out)
	{
		const TSharedPtr<FJsonObject>* Sub = nullptr;
		if (!Obj->TryGetObjectField(Field, Sub) || !Sub || !Sub->IsValid())
		{
			return false;
		}
		Out.X = (*Sub)->GetNumberField(TEXT("x"));
		Out.Y = (*Sub)->GetNumberField(TEXT("y"));
		Out.Z = (*Sub)->GetNumberField(TEXT("z"));
		return true;
	}

	bool ReadRotator(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Field, FRotator& Out)
	{
		const TSharedPtr<FJsonObject>* Sub = nullptr;
		if (!Obj->TryGetObjectField(Field, Sub) || !Sub || !Sub->IsValid())
		{
			return false;
		}
		Out.Pitch = (*Sub)->GetNumberField(TEXT("pitch"));
		Out.Yaw = (*Sub)->GetNumberField(TEXT("yaw"));
		Out.Roll = (*Sub)->GetNumberField(TEXT("roll"));
		return true;
	}

	UWorld* EditorWorld()
	{
		return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	}

	UWorld* PIEWorld()
	{
		return GEditor ? GEditor->PlayWorld.Get() : nullptr;
	}

	/** Resolve the camera the caller asked for. Returns an error object on failure. */
	TSharedPtr<FJsonObject> ResolveView(const TSharedPtr<FJsonObject>& Params, MCPCapture::FView& Out)
	{
		FString Mode = TEXT("editor");
		const TSharedPtr<FJsonObject>* CameraObj = nullptr;
		if (Params->HasTypedField<EJson::String>(TEXT("camera")))
		{
			Mode = Params->GetStringField(TEXT("camera")).ToLower();
		}
		else if (Params->TryGetObjectField(TEXT("camera"), CameraObj) && CameraObj && CameraObj->IsValid())
		{
			Mode = TEXT("explicit");
		}
		else if (PIEWorld())
		{
			Mode = TEXT("pie"); // default: what the player sees if a game is running
		}

		if (Mode == TEXT("pie"))
		{
			UWorld* World = PIEWorld();
			if (!World)
			{
				return CaptureError(TEXT("pie_not_running"), TEXT("No Play-In-Editor session is running"),
					TEXT("Call start_pie first, or use camera \"editor\""));
			}
			APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
			APlayerCameraManager* Cam = PC ? PC->PlayerCameraManager : nullptr;
			if (Cam)
			{
				Out.Location = Cam->GetCameraLocation();
				Out.Rotation = Cam->GetCameraRotation();
				Out.FOV = Cam->GetFOVAngle();
			}
			else if (PC)
			{
				PC->GetPlayerViewPoint(Out.Location, Out.Rotation);
				Out.FOV = 90.f;
			}
			else
			{
				return CaptureError(TEXT("no_player"), TEXT("PIE is running but there is no player controller yet"),
					TEXT("Wait a frame or two after start_pie"));
			}
			Out.World = World;
			Out.bIsPIE = true;
			Out.Viewport = (GEngine && GEngine->GameViewport) ? GEngine->GameViewport->Viewport : nullptr;
			return nullptr;
		}

		if (Mode == TEXT("editor"))
		{
			FLevelEditorModule* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>(TEXT("LevelEditor"));
			TSharedPtr<SLevelViewport> Viewport = LevelEditor ? LevelEditor->GetFirstActiveLevelViewport() : nullptr;
			FLevelEditorViewportClient* Client = Viewport.IsValid() ? &Viewport->GetLevelViewportClient() : nullptr;
			UWorld* World = EditorWorld();
			if (!Client || !World)
			{
				return CaptureError(TEXT("no_editor_viewport"), TEXT("No active level editor viewport"),
					TEXT("Open a level in the editor, or pass an explicit camera"));
			}
			Out.Location = Client->GetViewLocation();
			Out.Rotation = Client->GetViewRotation();
			Out.FOV = Client->ViewFOV;
			Out.World = World;
			Out.bIsPIE = false;
			Out.Viewport = Client->Viewport;
			return nullptr;
		}

		if (Mode == TEXT("explicit"))
		{
			const TSharedPtr<FJsonObject>& Cam = *CameraObj;
			if (!ReadVector(Cam, TEXT("location"), Out.Location) || !ReadRotator(Cam, TEXT("rotation"), Out.Rotation))
			{
				return CaptureError(TEXT("invalid_camera"), TEXT("camera needs {location:{x,y,z}, rotation:{pitch,yaw,roll}, fov?}"));
			}
			Out.FOV = Cam->HasField(TEXT("fov")) ? static_cast<float>(Cam->GetNumberField(TEXT("fov"))) : 90.f;
			Out.FOV = FMath::Clamp(Out.FOV, 5.f, 170.f);
			FString WorldName;
			const bool bWantEditor = Params->TryGetStringField(TEXT("world"), WorldName) && WorldName.ToLower() == TEXT("editor");
			Out.World = (!bWantEditor && PIEWorld()) ? PIEWorld() : EditorWorld();
			Out.bIsPIE = Out.World && Out.World == PIEWorld();
			if (!Out.World)
			{
				return CaptureError(TEXT("no_world"), TEXT("No editor or PIE world is available"));
			}
			return nullptr;
		}

		return CaptureError(TEXT("invalid_camera"), FString::Printf(TEXT("Unknown camera mode '%s'"), *Mode),
			TEXT("Use \"editor\", \"pie\", or {location, rotation, fov}"));
	}

	/** View-projection for a horizontal FOV camera, matching what the scene capture renders. */
	FMatrix ViewProjection(const MCPCapture::FView& View, int32 Width, int32 Height)
	{
		const FMatrix ViewMatrix = FTranslationMatrix(-View.Location) * FInverseRotationMatrix(View.Rotation) *
			FMatrix(FPlane(0, 0, 1, 0), FPlane(1, 0, 0, 0), FPlane(0, 1, 0, 0), FPlane(0, 0, 0, 1));
		const float HalfFOV = FMath::DegreesToRadians(FMath::Max(View.FOV, 5.f)) * 0.5f;
		const FMatrix Projection = FReversedZPerspectiveMatrix(HalfFOV, static_cast<float>(Width), static_cast<float>(Height), GNearClippingPlane);
		return ViewMatrix * Projection;
	}

	/** Bounds of what would actually render: visible mesh/landscape primitives only, so a
	 *  camera boom, a trigger sphere or an effect with fixed bounds does not inflate the box.
	 *  Returns false if nothing renders. */
	bool RenderedBounds(const AActor* Actor, bool bGameWorld, FBox& OutBox)
	{
		OutBox.Init();
		Actor->ForEachComponent<UPrimitiveComponent>(false, [&](const UPrimitiveComponent* Prim)
		{
			if (!Prim || Prim->IsA<UBrushComponent>())
			{
				return;
			}
			const bool bMesh = Prim->IsA<UMeshComponent>() || Prim->GetClass()->GetName().Contains(TEXT("Landscape"));
			if (!bMesh)
			{
				return;
			}
			const bool bVisible = bGameWorld ? (Prim->IsVisible() && !Prim->bHiddenInGame) : Prim->IsVisibleInEditor();
			if (bVisible && Prim->GetNumMaterials() > 0)
			{
				OutBox += Prim->Bounds.GetBox();
			}
		});
		return OutBox.IsValid && !OutBox.GetExtent().IsNearlyZero(1.f);
	}
}

TArray<MCPCapture::FVisibleActor> MCPCapture::FindVisibleActors(const FView& View, int32 Width, int32 Height, int32 MaxActors, TArray<FCulledActor>* OutCulled)
{
	auto Cull = [OutCulled](const AActor* A, const FString& Why) { if (OutCulled) { OutCulled->Add({ A->GetActorNameOrLabel(), Why }); } };
	check(IsInGameThread());
	TArray<FVisibleActor> Out;
	if (!View.World || MaxActors <= 0)
	{
		return Out;
	}
	const bool bGameWorld = View.World->IsGameWorld();
	const FMatrix VP = ViewProjection(View, Width, Height);

	for (TActorIterator<AActor> It(View.World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor->IsA<AInfo>() || Actor->IsA<AVolume>() || Actor->IsA<ABrush>())
		{
			continue;
		}
		if (bGameWorld ? Actor->IsHidden() : Actor->IsHiddenEd())
		{
			continue;
		}
		if (Actor->GetClass()->GetName().Contains(TEXT("InstancedFoliageActor")))
		{
			continue; // one actor for every foliage instance on the map: its bounds mean nothing
		}
		FBox Bounds;
		if (!RenderedBounds(Actor, bGameWorld, Bounds))
		{
			Cull(Actor, TEXT("no_rendered_mesh"));
			continue;
		}
		const FVector Origin = Bounds.GetCenter();
		const FVector Extent = Bounds.GetExtent();

		// Project the 8 bounds corners; keep the actor if any corner is in front and the box hits the image
		FIntRect Box(INT_MAX, INT_MAX, INT_MIN, INT_MIN);
		bool bAnyInFront = false;
		for (int32 i = 0; i < 8; ++i)
		{
			const FVector Corner(Origin.X + ((i & 1) ? Extent.X : -Extent.X),
				Origin.Y + ((i & 2) ? Extent.Y : -Extent.Y),
				Origin.Z + ((i & 4) ? Extent.Z : -Extent.Z));
			const FVector4 Clip = VP.TransformFVector4(FVector4(Corner, 1.0));
			if (Clip.W <= KINDA_SMALL_NUMBER)
			{
				continue; // behind the camera; the box is extended by the corners in front
			}
			bAnyInFront = true;
			const double NdcX = Clip.X / Clip.W;
			const double NdcY = Clip.Y / Clip.W;
			const int32 Px = FMath::RoundToInt((NdcX * 0.5 + 0.5) * Width);
			const int32 Py = FMath::RoundToInt((1.0 - (NdcY * 0.5 + 0.5)) * Height);
			Box.Min.X = FMath::Min(Box.Min.X, Px); Box.Min.Y = FMath::Min(Box.Min.Y, Py);
			Box.Max.X = FMath::Max(Box.Max.X, Px); Box.Max.Y = FMath::Max(Box.Max.Y, Py);
		}
		if (!bAnyInFront)
		{
			Cull(Actor, TEXT("behind_camera"));
			continue;
		}
		Box.Min.X = FMath::Clamp(Box.Min.X, 0, Width); Box.Max.X = FMath::Clamp(Box.Max.X, 0, Width);
		Box.Min.Y = FMath::Clamp(Box.Min.Y, 0, Height); Box.Max.Y = FMath::Clamp(Box.Max.Y, 0, Height);
		if (Box.Width() <= 0 || Box.Height() <= 0)
		{
			Cull(Actor, TEXT("off_screen"));
			continue;
		}

		// Cheap occlusion: a single trace to the bounds centre. Occluded only if something else
		// is hit before the ray reaches the actor's bounds (a hit inside the slightly expanded
		// box counts as reaching it: small props sit on or in the ground, and the ground is
		// what the ray touches first).
		const double Distance = FVector::Dist(View.Location, Origin);
		FHitResult Hit;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MCPCaptureVisibility), true);
		if (View.World->LineTraceSingleByChannel(Hit, View.Location, Origin, ECC_Visibility, QueryParams))
		{
			const bool bReachedBounds = Bounds.ExpandBy(FMath::Max(10.0, Extent.Size() * 0.25)).IsInside(Hit.ImpactPoint);
			if (Hit.GetActor() != Actor && !bReachedBounds && Hit.Distance < Distance - Extent.Size())
			{
				Cull(Actor, FString::Printf(TEXT("occluded_by:%s"), Hit.GetActor() ? *Hit.GetActor()->GetActorNameOrLabel() : TEXT("?")));
				continue;
			}
		}

		FVisibleActor V;
		V.Name = Actor->GetActorNameOrLabel();
		V.Class = Actor->GetClass()->GetName();
		V.WorldLocation = Actor->GetActorLocation();
		V.Distance = Distance;
		V.ScreenBox = Box;
		Out.Add(MoveTemp(V));
	}
	Out.Sort([](const FVisibleActor& A, const FVisibleActor& B) { return A.Distance < B.Distance; });
	if (Out.Num() > MaxActors)
	{
		Out.SetNum(MaxActors);
	}
	return Out;
}

namespace
{
	TSharedPtr<FJsonObject> HandleCaptureViewport(const TSharedPtr<FJsonObject>& Params)
	{
		check(IsInGameThread());
		const double T0 = FPlatformTime::Seconds();
		const UMCPCaptureSettings& Settings = UMCPCaptureSettings::Get();

		// --- parameters
		int32 Width = Settings.DefaultWidth, Height = Settings.DefaultHeight;
		const TSharedPtr<FJsonObject>* Res = nullptr;
		if (Params->TryGetObjectField(TEXT("resolution"), Res) && Res && Res->IsValid())
		{
			Width = static_cast<int32>((*Res)->GetNumberField(TEXT("w")));
			Height = static_cast<int32>((*Res)->GetNumberField(TEXT("h")));
		}
		if (Width < 16 || Height < 16)
		{
			return CaptureError(TEXT("invalid_resolution"), TEXT("resolution.w and resolution.h must be at least 16"));
		}
		if (FMath::Max(Width, Height) > Settings.MaxLongEdge)
		{
			return CaptureError(TEXT("resolution_too_large"),
				FString::Printf(TEXT("Longest edge %d exceeds the cap of %d"), FMath::Max(Width, Height), Settings.MaxLongEdge),
				TEXT("Lower the resolution or raise MaxLongEdge in Project Settings > Plugins > MCP Capture"));
		}
		FString Format = TEXT("jpeg");
		Params->TryGetStringField(TEXT("format"), Format);
		Format = Format.ToLower();
		if (Format == TEXT("jpg")) Format = TEXT("jpeg");
		if (Format != TEXT("jpeg") && Format != TEXT("png"))
		{
			return CaptureError(TEXT("invalid_format"), FString::Printf(TEXT("Unknown format '%s'"), *Format), TEXT("Use \"jpeg\" or \"png\""));
		}
		int32 Quality = Settings.JpegQuality;
		if (Params->HasField(TEXT("quality")))
		{
			Quality = FMath::Clamp(static_cast<int32>(Params->GetNumberField(TEXT("quality"))), 1, 100);
		}
		int32 MaxActors = Settings.MaxActors;
		if (Params->HasField(TEXT("max_actors")))
		{
			MaxActors = FMath::Clamp(static_cast<int32>(Params->GetNumberField(TEXT("max_actors"))), 0, 2000);
		}

		MCPCapture::FView View;
		if (TSharedPtr<FJsonObject> Err = ResolveView(Params, View))
		{
			return Err;
		}

		// --- capture
		// "editor"/"pie": read the live viewport's last frame, which carries everything the user
		// sees (Lumen, post process, converged exposure). A scene capture of the same camera
		// measured 13x darker on a night scene. Explicit cameras have no viewport and render
		// through a transient scene capture; the frame is then resized to the request.
		TArray<FColor> Pixels;
		int32 SrcW = Width, SrcH = Height;
		bool bRead = false;
		double TRead = 0.0;
		if (View.Viewport)
		{
			const FIntPoint Size = View.Viewport->GetSizeXY();
			if (Size.X <= 0 || Size.Y <= 0)
			{
				return CaptureError(TEXT("capture_failed"), TEXT("The viewport has no size (minimised?)"));
			}
			if (!View.bIsPIE)
			{
				View.Viewport->Draw(false); // editor viewports are not realtime: render a fresh frame
			}
			SrcW = Size.X; SrcH = Size.Y;
			// The viewport has its own aspect; fit the requested size around it rather than
			// squash the frame (a 2.9:1 editor viewport into 16:9 would distort everything).
			const double Fit = FMath::Min(static_cast<double>(Width) / SrcW, static_cast<double>(Height) / SrcH);
			Width = FMath::Max(16, FMath::RoundToInt(SrcW * Fit));
			Height = FMath::Max(16, FMath::RoundToInt(SrcH * Fit));
			bRead = View.Viewport->ReadPixels(Pixels, FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX), FIntRect(0, 0, SrcW, SrcH));
			TRead = FPlatformTime::Seconds();
			if (!bRead || Pixels.Num() != SrcW * SrcH)
			{
				return CaptureError(TEXT("capture_failed"), TEXT("Viewport readback failed"),
					TEXT("The viewport may not have rendered yet; retry"));
			}
		}
		else
		{
		UTextureRenderTarget2D* Target = NewObject<UTextureRenderTarget2D>(GetTransientPackage(), NAME_None, RF_Transient);
		Target->RenderTargetFormat = RTF_RGBA8;
		Target->ClearColor = FLinearColor::Black;
		Target->InitAutoFormat(Width, Height);
		Target->UpdateResourceImmediate(true);

		USceneCaptureComponent2D* Capture = NewObject<USceneCaptureComponent2D>(GetTransientPackage(), NAME_None, RF_Transient);
		Capture->bCaptureEveryFrame = false;
		Capture->bCaptureOnMovement = false;
		// A one-shot capture has no eye-adaptation history, so auto-exposed scenes (any night
		// scene) come out nearly black. Keep the view state between renders, make adaptation
		// instant, and render twice: the first pass measures the exposure, the second uses it.
		Capture->bAlwaysPersistRenderingState = true;
		Capture->PostProcessSettings.bOverride_AutoExposureSpeedUp = true;
		Capture->PostProcessSettings.AutoExposureSpeedUp = 1000.f;
		Capture->PostProcessSettings.bOverride_AutoExposureSpeedDown = true;
		Capture->PostProcessSettings.AutoExposureSpeedDown = 1000.f;
		Capture->PostProcessBlendWeight = 1.f;
		Capture->ShowFlags.SetLumenGlobalIllumination(true);
		Capture->ShowFlags.SetLumenReflections(true);
		Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		Capture->FOVAngle = View.FOV;
		Capture->TextureTarget = Target;
		Capture->RegisterComponentWithWorld(View.World);
		Capture->SetWorldLocationAndRotation(View.Location, View.Rotation);
		Capture->CaptureScene();
		Capture->CaptureScene();

		FTextureRenderTargetResource* Resource = Target->GameThread_GetRenderTargetResource();
		bRead = Resource && Resource->ReadPixels(Pixels);
		TRead = FPlatformTime::Seconds();

		Capture->UnregisterComponent();
		Capture->MarkAsGarbage();
		Target->MarkAsGarbage();

		if (!bRead || Pixels.Num() != Width * Height)
		{
			return CaptureError(TEXT("capture_failed"), TEXT("Render target readback failed"),
				TEXT("The renderer may not be ready; retry once the editor has drawn a frame"));
		}
		}
		for (FColor& C : Pixels)
		{
			C.A = 255; // alpha is scene-dependent in both paths; the image is opaque by definition
		}

		// --- state that explains the frame (game thread, cheap)
		const bool bDebug = Params->HasField(TEXT("debug")) && Params->GetBoolField(TEXT("debug"));
		TArray<MCPCapture::FCulledActor> Culled;
		TArray<MCPCapture::FVisibleActor> Visible = MCPCapture::FindVisibleActors(View, SrcW, SrcH, MaxActors, bDebug ? &Culled : nullptr);
		if (SrcW != Width || SrcH != Height)
		{
			const double SX = static_cast<double>(Width) / SrcW, SY = static_cast<double>(Height) / SrcH;
			for (MCPCapture::FVisibleActor& V : Visible)
			{
				V.ScreenBox.Min.X = FMath::RoundToInt(V.ScreenBox.Min.X * SX); V.ScreenBox.Max.X = FMath::RoundToInt(V.ScreenBox.Max.X * SX);
				V.ScreenBox.Min.Y = FMath::RoundToInt(V.ScreenBox.Min.Y * SY); V.ScreenBox.Max.Y = FMath::RoundToInt(V.ScreenBox.Max.Y * SY);
			}
		}
		const double TActors = FPlatformTime::Seconds();

		auto Result = MakeShared<FJsonObject>();
		Result->SetBoolField(TEXT("success"), true);
		Result->SetNumberField(TEXT("timestamp"), View.bIsPIE ? View.World->GetTimeSeconds() : FDateTime::UtcNow().ToUnixTimestampDecimal());
		Result->SetStringField(TEXT("timestamp_kind"), View.bIsPIE ? TEXT("world_time") : TEXT("unix_time"));
		auto Camera = MakeShared<FJsonObject>();
		Camera->SetObjectField(TEXT("location"), VectorJson(View.Location));
		Camera->SetObjectField(TEXT("rotation"), RotatorJson(View.Rotation));
		Camera->SetNumberField(TEXT("fov"), View.FOV);
		Camera->SetStringField(TEXT("world"), View.bIsPIE ? TEXT("pie") : TEXT("editor"));
		Camera->SetStringField(TEXT("source"), View.Viewport ? TEXT("viewport") : TEXT("scene_capture"));
		Result->SetObjectField(TEXT("camera"), Camera);

		TArray<TSharedPtr<FJsonValue>> Actors;
		for (const MCPCapture::FVisibleActor& V : Visible)
		{
			auto A = MakeShared<FJsonObject>();
			A->SetStringField(TEXT("name"), V.Name);
			A->SetStringField(TEXT("class"), V.Class);
			TArray<TSharedPtr<FJsonValue>> Box;
			Box.Add(MakeShared<FJsonValueNumber>(V.ScreenBox.Min.X));
			Box.Add(MakeShared<FJsonValueNumber>(V.ScreenBox.Min.Y));
			Box.Add(MakeShared<FJsonValueNumber>(V.ScreenBox.Max.X));
			Box.Add(MakeShared<FJsonValueNumber>(V.ScreenBox.Max.Y));
			A->SetArrayField(TEXT("screen_bbox"), Box);
			A->SetObjectField(TEXT("world_location"), VectorJson(V.WorldLocation));
			A->SetNumberField(TEXT("distance"), V.Distance);
			Actors.Add(MakeShared<FJsonValueObject>(A));
		}
		Result->SetArrayField(TEXT("actors"), Actors);
		if (bDebug)
		{
			TArray<TSharedPtr<FJsonValue>> CulledJson;
			for (const MCPCapture::FCulledActor& Cu : Culled)
			{
				auto O = MakeShared<FJsonObject>();
				O->SetStringField(TEXT("name"), Cu.Name);
				O->SetStringField(TEXT("reason"), Cu.Reason);
				CulledJson.Add(MakeShared<FJsonValueObject>(O));
			}
			Result->SetArrayField(TEXT("culled"), CulledJson);
		}

		auto Image = MakeShared<FJsonObject>();
		Image->SetStringField(TEXT("format"), Format);
		Image->SetNumberField(TEXT("width"), Width);
		Image->SetNumberField(TEXT("height"), Height);
		Result->SetObjectField(TEXT("image"), Image);

		auto Timings = MakeShared<FJsonObject>();
		Timings->SetNumberField(TEXT("game_thread_capture_ms"), (TRead - T0) * 1000.0);
		Timings->SetNumberField(TEXT("game_thread_actors_ms"), (TActors - TRead) * 1000.0);
		Result->SetObjectField(TEXT("timings"), Timings);

		// --- encoding happens on the socket thread, after this handler has returned
		FMCPTcpServer::QueuePostProcess([Pixels = MoveTemp(Pixels), SrcW, SrcH, Width, Height, Format, Quality](TSharedPtr<FJsonObject>& Out) mutable
		{
			check(!IsInGameThread());
			const double TEnc0 = FPlatformTime::Seconds();
			if (SrcW != Width || SrcH != Height)
			{
				// Box-filter resize (viewport size -> requested size); off the game thread.
				TArray<FColor> Resized;
				Resized.SetNumUninitialized(Width * Height);
				for (int32 y = 0; y < Height; ++y)
				{
					const int32 Y0 = y * SrcH / Height, Y1 = FMath::Max(Y0 + 1, (y + 1) * SrcH / Height);
					for (int32 x = 0; x < Width; ++x)
					{
						const int32 X0 = x * SrcW / Width, X1 = FMath::Max(X0 + 1, (x + 1) * SrcW / Width);
						uint32 R = 0, G = 0, B = 0, N = 0;
						for (int32 sy = Y0; sy < Y1; ++sy)
							for (int32 sx = X0; sx < X1; ++sx)
							{
								const FColor& S = Pixels[sy * SrcW + sx];
								R += S.R; G += S.G; B += S.B; ++N;
							}
						Resized[y * Width + x] = FColor(R / N, G / N, B / N, 255);
					}
				}
				Pixels = MoveTemp(Resized);
			}
			IImageWrapperModule& Module = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
			TSharedPtr<IImageWrapper> Wrapper = Module.CreateImageWrapper(Format == TEXT("png") ? EImageFormat::PNG : EImageFormat::JPEG);
			TArray64<uint8> Bytes;
			if (Wrapper.IsValid() && Wrapper->SetRaw(Pixels.GetData(), Pixels.Num() * sizeof(FColor), Width, Height, ERGBFormat::BGRA, 8))
			{
				Bytes = Wrapper->GetCompressed(Format == TEXT("png") ? 0 : Quality);
			}
			if (Bytes.Num() == 0)
			{
				Out = CaptureError(TEXT("encode_failed"), TEXT("Image encoding failed"));
				return;
			}
			const TSharedPtr<FJsonObject>* ImageObj = nullptr;
			if (Out->TryGetObjectField(TEXT("image"), ImageObj) && ImageObj && ImageObj->IsValid())
			{
				(*ImageObj)->SetStringField(TEXT("mime_type"), Format == TEXT("png") ? TEXT("image/png") : TEXT("image/jpeg"));
				(*ImageObj)->SetNumberField(TEXT("bytes"), Bytes.Num());
				(*ImageObj)->SetStringField(TEXT("data"), FBase64::Encode(Bytes.GetData(), Bytes.Num()));
			}
			const TSharedPtr<FJsonObject>* TimingsObj = nullptr;
			if (Out->TryGetObjectField(TEXT("timings"), TimingsObj) && TimingsObj && TimingsObj->IsValid())
			{
				(*TimingsObj)->SetNumberField(TEXT("encode_ms"), (FPlatformTime::Seconds() - TEnc0) * 1000.0);
			}
		});
		return Result;
	}
}

void MCPCapture::RegisterHandlers(FMCPTcpServer& Server)
{
	Server.RegisterHandler(TEXT("capture_viewport"), [](const TSharedPtr<FJsonObject>& Params) { return HandleCaptureViewport(Params); });
}
