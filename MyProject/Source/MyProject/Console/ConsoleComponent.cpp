#include "ConsoleComponent.h"
#include "../Core/HorrorGameInstance.h"
#include "../Player/PlayerCharacterB.h"
#include "GameFramework/Character.h"

UConsoleComponent::UConsoleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentState = EConsoleState::Hidden;
}

void UConsoleComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedOwnerCharacter = Cast<APlayerCharacterB>(GetOwner());
	CachedGameInstance = Cast<UHorrorGameInstance>(GetWorld()->GetGameInstance());

	if (CachedGameInstance)
	{
		CachedGameInstance->OnLayerChanged.AddDynamic(this, &UConsoleComponent::OnGameInstanceLayerChanged);
	}
}

void UConsoleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedGameInstance)
	{
		CachedGameInstance->OnLayerChanged.RemoveDynamic(this, &UConsoleComponent::OnGameInstanceLayerChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void UConsoleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UConsoleComponent::TakeOut()
{
	if (CurrentState != EConsoleState::Hidden) return;
	if (bTransitionLocked) return;

	SpawnConsoleActor();

	if (CachedOwnerCharacter)
	{
		CachedOwnerCharacter->bIsHoldingConsole = true;
	}

	CurrentState = EConsoleState::Active;
	OnConsoleStateChanged.Broadcast(CurrentState);

	bTransitionLocked = true;
	GetWorld()->GetTimerManager().SetTimer(TransitionLockTimer, this, &UConsoleComponent::UnlockTransition, TransitionCooldown, false);

	if (CachedGameInstance && CachedGameInstance->CurrentLayerState != ELayerState::STATE_CONSOLE_ACTIVE)
	{
		CachedGameInstance->TransitionToLayer(ELayerState::STATE_CONSOLE_ACTIVE);
	}

	if (CachedGameInstance)
	{
		CachedGameInstance->OnConsoleToggled.Broadcast(true);
	}
}

void UConsoleComponent::PutAway()
{
	if (CurrentState != EConsoleState::Active) return;
	if (bTransitionLocked) return;

	CurrentState = EConsoleState::Hidden;

	if (CachedOwnerCharacter)
	{
		CachedOwnerCharacter->bIsHoldingConsole = false;
	}

	DestroyConsoleActor();

	if (CachedGameInstance)
	{
		CachedGameInstance->TransitionToLayer(ELayerState::STATE_OUTER_GAMEPLAY);
	}

	OnConsoleStateChanged.Broadcast(CurrentState);

	bTransitionLocked = true;
	GetWorld()->GetTimerManager().SetTimer(TransitionLockTimer, this, &UConsoleComponent::UnlockTransition, TransitionCooldown, false);

	if (CachedGameInstance)
	{
		CachedGameInstance->OnConsoleToggled.Broadcast(false);
	}
}

void UConsoleComponent::ForcePutAway()
{
	CurrentState = EConsoleState::Hidden;

	if (CachedOwnerCharacter)
	{
		CachedOwnerCharacter->bIsHoldingConsole = false;
	}

	DestroyConsoleActor();

	if (CachedGameInstance)
	{
		CachedGameInstance->TransitionToLayer(ELayerState::STATE_OUTER_GAMEPLAY);
	}

	OnConsoleStateChanged.Broadcast(CurrentState);

	if (CachedGameInstance)
	{
		CachedGameInstance->OnConsoleToggled.Broadcast(false);
	}
}

void UConsoleComponent::EnterInnerGameplay()
{
	if (CurrentState != EConsoleState::Active) return;

	if (CachedGameInstance && CachedGameInstance->CurrentLayerState == ELayerState::STATE_CONSOLE_ACTIVE)
	{
		CachedGameInstance->TransitionToLayer(ELayerState::STATE_INNER_GAMEPLAY);
	}
}

void UConsoleComponent::ExitInnerGameplay()
{
	if (CachedGameInstance && CachedGameInstance->CurrentLayerState == ELayerState::STATE_INNER_GAMEPLAY)
	{
		CachedGameInstance->TransitionToLayer(ELayerState::STATE_CONSOLE_ACTIVE);
	}
}

void UConsoleComponent::OnGameInstanceLayerChanged(ELayerState From, ELayerState To)
{
	if (To == ELayerState::STATE_OUTER_GAMEPLAY && CurrentState == EConsoleState::Active)
	{
		ForcePutAway();
	}
}

void UConsoleComponent::SpawnConsoleActor()
{
	if (!ConsoleActorClass || !CachedOwnerCharacter) return;
	if (ConsoleActorInstance && IsValid(ConsoleActorInstance)) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CachedOwnerCharacter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ConsoleActorInstance = GetWorld()->SpawnActor<AActor>(
		ConsoleActorClass,
		CachedOwnerCharacter->GetActorLocation(),
		CachedOwnerCharacter->GetActorRotation(),
		SpawnParams);

	if (!ConsoleActorInstance) return;

	if (CachedOwnerCharacter->GetMesh())
	{
		ConsoleActorInstance->AttachToComponent(
			CachedOwnerCharacter->GetMesh(),
			FAttachmentTransformRules::SnapToTargetIncludingScale,
			AttachSocketName);
	}

	ConsoleActorInstance->SetActorRelativeLocation(ConsoleAttachOffset);
	ConsoleActorInstance->SetActorRelativeRotation(ConsoleAttachRotation);
}

void UConsoleComponent::DestroyConsoleActor()
{
	if (ConsoleActorInstance && IsValid(ConsoleActorInstance))
	{
		ConsoleActorInstance->Destroy();
	}
	ConsoleActorInstance = nullptr;
}

void UConsoleComponent::UnlockTransition()
{
	bTransitionLocked = false;
}
