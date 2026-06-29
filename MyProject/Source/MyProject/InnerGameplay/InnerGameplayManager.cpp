#include "InnerGameplayManager.h"
#include "../Core/HorrorGameInstance.h"
#include "../Core/HorrorTypes.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "EngineUtils.h"

AInnerGameplayManager::AInnerGameplayManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = false;
}

void AInnerGameplayManager::BeginPlay()
{
	Super::BeginPlay();

	CachedGameInstance = Cast<UHorrorGameInstance>(GetWorld()->GetGameInstance());
	if (CachedGameInstance)
	{
		CachedGameInstance->OnLayerChanged.AddDynamic(this, &AInnerGameplayManager::OnLayerChangedCallback);
	}
}

void AInnerGameplayManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedGameInstance)
	{
		CachedGameInstance->OnLayerChanged.RemoveDynamic(this, &AInnerGameplayManager::OnLayerChangedCallback);
	}

	if (bIsL4Active)
	{
		EndInnerGame();
	}

	Super::EndPlay(EndPlayReason);
}

void AInnerGameplayManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsL4Active)
	{
		ReceiveL4Tick(DeltaTime);
	}
}

void AInnerGameplayManager::OnLayerChangedCallback(ELayerState From, ELayerState To)
{
	if (To == ELayerState::STATE_INNER_GAMEPLAY && !bIsL4Active)
	{
		if (bAutoStartOnL4Enter)
		{
			StartInnerGame();
		}
	}
	else if (From == ELayerState::STATE_INNER_GAMEPLAY && To != ELayerState::STATE_INNER_GAMEPLAY)
	{
		if (bAutoEndOnL4Exit && bIsL4Active)
		{
			EndInnerGame();
		}
	}
}

void AInnerGameplayManager::TransitionToL4()
{
	if (CachedGameInstance
		&& CachedGameInstance->CurrentLayerState == ELayerState::STATE_CONSOLE_ACTIVE)
	{
		CachedGameInstance->TransitionToLayer(ELayerState::STATE_INNER_GAMEPLAY);
	}
}

void AInnerGameplayManager::TransitionToL3()
{
	if (CachedGameInstance
		&& CachedGameInstance->CurrentLayerState == ELayerState::STATE_INNER_GAMEPLAY)
	{
		CachedGameInstance->TransitionToLayer(ELayerState::STATE_CONSOLE_ACTIVE);
	}
}

void AInnerGameplayManager::StartInnerGame()
{
	if (bIsL4Active) return;

	UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] StartInnerGame - loading '%s'"), *L4LevelName.ToString());

	OnL4Started.Broadcast();

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = TEXT("OnL4LevelLoadComplete");
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = FMath::Rand();

	UGameplayStatics::LoadStreamLevel(this, L4LevelName, true, false, LatentInfo);

	bIsL4Active = true;
}

void AInnerGameplayManager::OnL4LevelLoadComplete()
{
	UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] L4 level loaded - looking for SceneCapture named '%s'"), *SceneCaptureActorName.ToString());

	SetupSceneCapture();

	OnL4LevelLoaded.Broadcast();
}

void AInnerGameplayManager::SetupSceneCapture()
{
	ClearSceneCapture();

	if (!RenderTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InnerGameplayManager] RenderTarget is null - SceneCapture won't render anywhere. Assign a RenderTarget2D asset in the editor."));
		return;
	}

	for (TActorIterator<ASceneCapture2D> It(GetWorld()); It; ++It)
	{
		ASceneCapture2D* Capture = *It;
		if (Capture && Capture->GetFName() == SceneCaptureActorName)
		{
			USceneCaptureComponent2D* CaptureComp = Capture->GetCaptureComponent2D();
			if (CaptureComp)
			{
				CaptureComp->TextureTarget = RenderTarget;
				CaptureComp->bCaptureEveryFrame = true;
				CaptureComp->bCaptureOnMovement = false;

				if (bShowOnlyL4Actors)
				{
					CaptureComp->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
					CaptureComp->ShowOnlyActors.Reset();

					for (ULevelStreaming* StreamingLevel : GetWorld()->GetStreamingLevels())
					{
						if (StreamingLevel && StreamingLevel->GetWorldAssetPackageFName() == L4LevelName)
						{
							if (ULevel* LoadedLevel = StreamingLevel->GetLoadedLevel())
							{
								for (AActor* Actor : LoadedLevel->Actors)
								{
									if (IsValid(Actor) && Actor != Capture)
									{
										CaptureComp->ShowOnlyActors.Add(Actor);
									}
								}
								UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] ShowOnlyList: %d actors from level '%s'"),
									CaptureComp->ShowOnlyActors.Num(), *L4LevelName.ToString());
							}
							break;
						}
					}
				}

				CachedSceneCapture = Capture;

				UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] SceneCapture '%s' bound to RenderTarget '%s'"),
					*Capture->GetName(), *RenderTarget->GetName());
				return;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[InnerGameplayManager] No ASceneCapture2D named '%s' found in loaded levels. Did you place one in %s?"),
		*SceneCaptureActorName.ToString(), *L4LevelName.ToString());
}

void AInnerGameplayManager::ClearSceneCapture()
{
	if (CachedSceneCapture)
	{
		if (USceneCaptureComponent2D* Comp = CachedSceneCapture->GetCaptureComponent2D())
		{
			Comp->TextureTarget = nullptr;
		}
		CachedSceneCapture = nullptr;
	}
}

void AInnerGameplayManager::EndInnerGame()
{
	if (!bIsL4Active) return;

	UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] EndInnerGame - unloading '%s'"), *L4LevelName.ToString());

	ClearSceneCapture();

	FLatentActionInfo LatentInfo;
	UGameplayStatics::UnloadStreamLevel(this, L4LevelName, LatentInfo, false);

	bIsL4Active = false;

	OnL4LevelUnloaded.Broadcast();
	OnL4Ended.Broadcast();
}
