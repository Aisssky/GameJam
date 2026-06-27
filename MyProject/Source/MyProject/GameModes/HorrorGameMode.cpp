#include "HorrorGameMode.h"
#include "../Core/HorrorGameInstance.h"
#include "../Player/HorrorPlayerController.h"
#include "../Player/PlayerCharacterB.h"
#include "Engine/World.h"

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
}
