#include "ConsoleComponent.h"
#include "../Core/HorrorGameInstance.h"
#include "../Player/PlayerCharacterB.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"

UConsoleComponent::UConsoleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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

	if (bAnimPlaying)
	{
		TickAnimProgress(DeltaTime, bIsTakeOut);
	}
}

void UConsoleComponent::TakeOut()
{
	if (CurrentState != EConsoleState::Hidden) return;

	CurrentState = EConsoleState::AnimatingOut;
	OnConsoleStateChanged.Broadcast(CurrentState);

	bIsTakeOut = true;
	AnimElapsed = 0.0f;
	bAnimPlaying = true;

	if (CachedOwnerCharacter && TakeOutAnimation)
	{
		CachedOwnerCharacter->PlayAnimMontage(TakeOutAnimation);
	}
}

void UConsoleComponent::OnTakeOutAnimProgress(float Progress)
{
	if (Progress >= InputSwitchAtProgress && !CachedGameInstance)
	{
		CachedGameInstance = Cast<UHorrorGameInstance>(GetWorld()->GetGameInstance());
	}

	if (Progress >= InputSwitchAtProgress && CachedGameInstance
		&& CachedGameInstance->CurrentLayerState != ELayerState::STATE_CONSOLE_ACTIVE)
	{
		CachedGameInstance->TransitionToLayer(ELayerState::STATE_CONSOLE_ACTIVE);
	}
}

void UConsoleComponent::OnTakeOutAnimComplete()
{
	bAnimPlaying = false;
	AnimElapsed = 0.0f;
	CurrentState = EConsoleState::Active;
	OnConsoleStateChanged.Broadcast(CurrentState);

	if (CachedGameInstance)
	{
		CachedGameInstance->OnConsoleToggled.Broadcast(true);
	}
}

void UConsoleComponent::PutAway()
{
	if (CurrentState != EConsoleState::Active) return;

	CurrentState = EConsoleState::AnimatingAway;
	OnConsoleStateChanged.Broadcast(CurrentState);

	bIsTakeOut = false;
	AnimElapsed = 0.0f;
	bAnimPlaying = true;

	if (CachedOwnerCharacter && PutAwayAnimation)
	{
		CachedOwnerCharacter->PlayAnimMontage(PutAwayAnimation);
	}
}

void UConsoleComponent::OnPutAwayAnimComplete()
{
	bAnimPlaying = false;
	AnimElapsed = 0.0f;

	if (CachedGameInstance)
	{
		CachedGameInstance->TransitionToLayer(ELayerState::STATE_OUTER_GAMEPLAY);
	}

	CurrentState = EConsoleState::Hidden;
	OnConsoleStateChanged.Broadcast(CurrentState);

	if (CachedGameInstance)
	{
		CachedGameInstance->OnConsoleToggled.Broadcast(false);
	}
}

void UConsoleComponent::ForcePutAway()
{
	bAnimPlaying = false;
	AnimElapsed = 0.0f;

	CurrentState = EConsoleState::Hidden;
	OnConsoleStateChanged.Broadcast(CurrentState);
}

void UConsoleComponent::TickAnimProgress(float DeltaTime, bool bIsTakingOut)
{
	AnimElapsed += DeltaTime;
	float Progress = (AnimationDuration > 0.0f) ? FMath::Clamp(AnimElapsed / AnimationDuration, 0.0f, 1.0f) : 1.0f;

	if (bIsTakingOut)
	{
		OnTakeOutAnimProgress(Progress);
	}

	if (Progress >= 1.0f)
	{
		if (bIsTakingOut)
		{
			OnTakeOutAnimComplete();
		}
		else
		{
			OnPutAwayAnimComplete();
		}
	}
}

void UConsoleComponent::OnGameInstanceLayerChanged(ELayerState From, ELayerState To)
{
	if (To == ELayerState::STATE_OUTER_GAMEPLAY && CurrentState == EConsoleState::Active)
	{
		ForcePutAway();
	}
}
