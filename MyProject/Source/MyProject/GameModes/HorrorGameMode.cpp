#include "HorrorGameMode.h"
#include "../Core/HorrorGameInstance.h"
#include "../Player/HorrorPlayerController.h"
#include "../Player/PlayerCharacterB.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "Kismet/GameplayStatics.h"

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
		FLatentActionInfo LatentInfo;
		UGameplayStatics::LoadStreamLevel(GetWorld(), ConsoleStreamingLevelName, true, false, LatentInfo);
	}
}
