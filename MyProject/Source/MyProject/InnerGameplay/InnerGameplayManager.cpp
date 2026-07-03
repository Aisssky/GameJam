#include "InnerGameplayManager.h"
#include "InnerGameCharacter.h"
#include "InnerGameCollectible.h"
#include "../Core/HorrorGameInstance.h"
#include "../Core/HorrorTypes.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Misc/PackageName.h"
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

	UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] StartInnerGame - level '%s' already preloaded"), *L4LevelName.ToString());

	OnL4Started.Broadcast();

	EnsureSceneCaptureCached();
	SetupSceneCapture();

	CollectedInLevel = 0;
	CacheInnerCharacter();
	BindInnerGameObjects();

	OnL4LevelLoaded.Broadcast();

	bIsL4Active = true;
}

void AInnerGameplayManager::CacheInnerCharacter()
{
	if (CachedInnerCharacter && IsValid(CachedInnerCharacter)) return;

	for (TActorIterator<AInnerGameCharacter> It(GetWorld()); It; ++It)
	{
		AInnerGameCharacter* InnerChar = *It;
		if (IsValid(InnerChar))
		{
			CachedInnerCharacter = InnerChar;

			InnerChar->OnCharacterDied.AddDynamic(this, &AInnerGameplayManager::HandleCharacterDied);

			UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] Cached InnerCharacter '%s'"), *InnerChar->GetName());
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[InnerGameplayManager] CacheInnerCharacter: No InnerGameCharacter found!"));
}

void AInnerGameplayManager::BindInnerGameObjects()
{
	for (TActorIterator<AInnerGameCollectible> It(GetWorld()); It; ++It)
	{
		AInnerGameCollectible* Coll = *It;
		if (IsValid(Coll))
		{
			Coll->OnCollected.AddDynamic(this, &AInnerGameplayManager::HandleCollectibleCollected);
		}
	}

	for (TActorIterator<AInnerGameZone> It(GetWorld()); It; ++It)
	{
		AInnerGameZone* Zone = *It;
		if (IsValid(Zone))
		{
			Zone->OnZoneTriggered.AddDynamic(this, &AInnerGameplayManager::HandleZoneTriggered);
		}
	}
}

void AInnerGameplayManager::HandleCollectibleCollected(FName ID, AActor* Collector)
{
	CollectedInLevel++;

	UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] Collectible '%s' collected. Total: %d/%d"),
		*ID.ToString(), CollectedInLevel, RequiredCollectibles);

	if (RequiredCollectibles > 0 && CollectedInLevel >= RequiredCollectibles)
	{
		TriggerWin();
	}
}

void AInnerGameplayManager::HandleZoneTriggered(EInnerGameZoneType ZoneType, AActor* Trigger)
{
	if (!Trigger) return;

	switch (ZoneType)
	{
	case EInnerGameZoneType::WinZone:
		UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] WinZone triggered by '%s'"), *Trigger->GetName());
		TriggerWin();
		break;

	case EInnerGameZoneType::DeathZone:
		UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] DeathZone triggered by '%s'"), *Trigger->GetName());
		if (AInnerGameCharacter* InnerChar = Cast<AInnerGameCharacter>(Trigger))
		{
			if (!InnerChar->IsInvincible())
			{
				InnerChar->TriggerDeath();
			}
		}
		break;

	case EInnerGameZoneType::Checkpoint:
	{
		UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] Checkpoint triggered by '%s'"), *Trigger->GetName());
		const int32 CpIndex = Cast<AInnerGameZone>(Trigger) ? Cast<AInnerGameZone>(Trigger)->CheckpointIndex : 0;
		if (CachedInnerCharacter)
		{
			CachedInnerCharacter->SetCheckpoint(Trigger->GetActorLocation(), CpIndex);
		}
		break;
	}
	}
}

void AInnerGameplayManager::HandleCharacterDied()
{
	if (!CachedInnerCharacter) return;

	if (CachedInnerCharacter->CurrentLives <= 0)
	{
		TriggerLose();
	}
}

void AInnerGameplayManager::TriggerWin()
{
	if (!bIsL4Active) return;

	if (CachedGameInstance && !L4WinNarrativeFlag.IsNone())
	{
		CachedGameInstance->SetFlag(L4WinNarrativeFlag, true, false);
	}

	OnGameWon.Broadcast();
	TransitionToL3();
}

void AInnerGameplayManager::TriggerLose()
{
	if (!bIsL4Active) return;

	if (CachedGameInstance && !L4LoseNarrativeFlag.IsNone())
	{
		CachedGameInstance->SetFlag(L4LoseNarrativeFlag, true, false);
	}

	OnGameLost.Broadcast();
	TransitionToL3();
}

void AInnerGameplayManager::EnsureSceneCaptureCached()
{
	if (CachedSceneCapture && IsValid(CachedSceneCapture))
	{
		return;
	}

	for (TActorIterator<ASceneCapture2D> It(GetWorld()); It; ++It)
	{
		ASceneCapture2D* Capture = *It;
		if (Capture && Capture->GetFName() == SceneCaptureActorName)
		{
			CachedSceneCapture = Capture;
			UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] Cached SceneCapture '%s'"), *Capture->GetName());
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[InnerGameplayManager] EnsureSceneCaptureCached: No SceneCapture2D named '%s' found!"),
		*SceneCaptureActorName.ToString());
}

void AInnerGameplayManager::SetupSceneCapture()
{
	EnsureSceneCaptureCached();

	if (!RenderTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InnerGameplayManager] RenderTarget is null"));
		return;
	}

	if (!CachedSceneCapture)
	{
		UE_LOG(LogTemp, Warning, TEXT("[InnerGameplayManager] No cached SceneCapture, can't setup"));
		return;
	}

	USceneCaptureComponent2D* CaptureComp = CachedSceneCapture->GetCaptureComponent2D();
	if (CaptureComp)
	{
		CaptureComp->TextureTarget = RenderTarget;
		CaptureComp->bCaptureEveryFrame = true;
		UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] SetupSceneCapture: bound RenderTarget '%s'"),
			*RenderTarget->GetName());
	}
}

void AInnerGameplayManager::ClearSceneCapture()
{
}

void AInnerGameplayManager::EndInnerGame()
{
	if (!bIsL4Active) return;

	UE_LOG(LogTemp, Log, TEXT("[InnerGameplayManager] EndInnerGame - keeping '%s' loaded for warm-up"), *L4LevelName.ToString());

	ClearSceneCapture();

	bIsL4Active = false;

	OnL4LevelUnloaded.Broadcast();
	OnL4Ended.Broadcast();
}
