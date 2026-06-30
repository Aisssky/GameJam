#include "HorrorGameMode.h"
#include "../Core/HorrorGameInstance.h"
#include "../InnerGameplay/InnerGameplayManager.h"
#include "../Player/HorrorPlayerController.h"
#include "../Player/PlayerCharacterB.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "EngineUtils.h"

AHorrorGameMode::AHorrorGameMode()
{
	PlayerControllerClass = AHorrorPlayerController::StaticClass();
	DefaultPawnClass = APlayerCharacterB::StaticClass();
}

void AHorrorGameMode::BeginPlay()
{
	Super::BeginPlay();
}

AMenuGameMode::AMenuGameMode()
{
	DefaultPawnClass = nullptr;
}

void AMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UHorrorGameInstance* GI = Cast<UHorrorGameInstance>(GetGameInstance()))
	{
		GI->TransitionToLayer(ELayerState::STATE_MAIN_MENU);
	}
}

AOuterGameplayGameMode::AOuterGameplayGameMode()
{
	DefaultPawnClass = APlayerCharacterB::StaticClass();
	PlayerControllerClass = AHorrorPlayerController::StaticClass();
}

void AOuterGameplayGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UHorrorGameInstance* GI = Cast<UHorrorGameInstance>(GetGameInstance()))
	{
		GI->TransitionToLayer(ELayerState::STATE_OUTER_GAMEPLAY);
	}

	if (!ConsoleStreamingLevelName.IsNone())
	{
		UGameplayStatics::LoadStreamLevel(GetWorld(), ConsoleStreamingLevelName, true, true, FLatentActionInfo());

		const FString TargetName = ConsoleStreamingLevelName.ToString();

		for (ULevelStreaming* SL : GetWorld()->GetStreamingLevels())
		{
			if (!SL) continue;

			FString PkgName = SL->GetWorldAssetPackageName();
			FString ShortName = FPackageName::GetShortName(PkgName);

			// Strip PIE prefix: "UEDPIE_0_L_AGameplay" -> "L_AGameplay"
			if (ShortName.StartsWith(TEXT("UEDPIE_")))
			{
				int32 SecondUnderscore = ShortName.Find(TEXT("_"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 7);
				if (SecondUnderscore != INDEX_NONE)
				{
					ShortName.RemoveAt(0, SecondUnderscore + 1);
				}
			}

			if (ShortName == TargetName)
			{
				CachedConsoleStreamingLevel = SL;

				if (SL->IsLevelLoaded())
				{
					HideConsoleLevelActors();
					SetupConsoleSceneCapture();
				}

				SL->OnLevelLoaded.AddDynamic(this, &AOuterGameplayGameMode::OnConsoleLevelLoaded);
				break;
			}
		}
	}
}

void AOuterGameplayGameMode::OnConsoleLevelLoaded()
{
	HideConsoleLevelActors();
	SetupConsoleSceneCapture();
}

void AOuterGameplayGameMode::HideConsoleLevelActors()
{
	if (!CachedConsoleStreamingLevel || !CachedConsoleStreamingLevel->IsLevelLoaded())
	{
		UE_LOG(LogTemp, Warning, TEXT("[OuterGameplayGameMode] HideConsoleLevelActors: level not loaded yet, skipping"));
		return;
	}

	ULevel* LoadedLevel = CachedConsoleStreamingLevel->GetLoadedLevel();
	if (!LoadedLevel) return;

	int32 HiddenCount = 0;
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	for (AActor* Actor : LoadedLevel->Actors)
	{
		if (!IsValid(Actor) || Actor->IsA<ASceneCapture2D>()) continue;

		Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
		for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
		{
			if (PrimComp)
			{
				PrimComp->bVisibleInSceneCaptureOnly = true;
				PrimComp->MarkRenderStateDirty();
			}
		}
		PrimitiveComponents.Reset();

		++HiddenCount;
	}
	UE_LOG(LogTemp, Log, TEXT("[OuterGameplayGameMode] Marked %d actors SceneCaptureOnly from '%s'"),
		HiddenCount, *ConsoleStreamingLevelName.ToString());
}

void AOuterGameplayGameMode::SetupConsoleSceneCapture()
{
	if (!CachedConsoleStreamingLevel || !CachedConsoleStreamingLevel->IsLevelLoaded())
	{
		UE_LOG(LogTemp, Warning, TEXT("[OuterGameplayGameMode] SetupConsoleSceneCapture: level not loaded yet, skipping"));
		return;
	}

	ULevel* LoadedLevel = CachedConsoleStreamingLevel->GetLoadedLevel();
	if (!LoadedLevel) return;

	// Find SceneCapture2D in the loaded streaming level
	ASceneCapture2D* FoundCapture = nullptr;
	for (AActor* Actor : LoadedLevel->Actors)
	{
		if (ASceneCapture2D* Capture = Cast<ASceneCapture2D>(Actor))
		{
			if (IsValid(Capture))
			{
				FoundCapture = Capture;
				break;
			}
		}
	}

	if (!FoundCapture)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OuterGameplayGameMode] SetupConsoleSceneCapture: No SceneCapture2D found in streaming level!"));
		return;
	}

	USceneCaptureComponent2D* CaptureComp = FoundCapture->GetCaptureComponent2D();
	if (!CaptureComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OuterGameplayGameMode] SetupConsoleSceneCapture: SceneCapture2D has no CaptureComponent2D!"));
		return;
	}

	// Configure ShowOnlyList with all actors from the streaming level (except the capture itself)
	CaptureComp->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	CaptureComp->ShowOnlyActors.Reset();

	for (AActor* Actor : LoadedLevel->Actors)
	{
		if (IsValid(Actor) && Actor != FoundCapture)
		{
			CaptureComp->ShowOnlyActors.Add(Actor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[OuterGameplayGameMode] SetupConsoleSceneCapture: ShowOnlyList=%d actors"), CaptureComp->ShowOnlyActors.Num());

	// Bind RenderTarget from InnerGameplayManager
	for (TActorIterator<AInnerGameplayManager> It(GetWorld()); It; ++It)
	{
		AInnerGameplayManager* Mgr = *It;
		if (IsValid(Mgr) && Mgr->RenderTarget)
		{
			CaptureComp->TextureTarget = Mgr->RenderTarget;
			CaptureComp->bCaptureEveryFrame = true;
			CaptureComp->bCaptureOnMovement = false;

			UE_LOG(LogTemp, Log, TEXT("[OuterGameplayGameMode] SetupConsoleSceneCapture: bound RenderTarget='%s'"),
				*Mgr->RenderTarget->GetName());
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[OuterGameplayGameMode] SetupConsoleSceneCapture: could not find InnerGameplayManager with RenderTarget"));
}
