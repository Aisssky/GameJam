#include "HorrorGameInstance.h"
#include "Engine/World.h"

bool UHorrorGameInstance::IsValidTransition(ELayerState From, ELayerState To) const
{
	if (From == To)
	{
		return true;
	}

	if (To == ELayerState::STATE_CINEMATIC)
	{
		return true;
	}

	if (From == ELayerState::STATE_CINEMATIC)
	{
		return true;
	}

	switch (From)
	{
	case ELayerState::STATE_MAIN_MENU:
		return To == ELayerState::STATE_OUTER_GAMEPLAY;

	case ELayerState::STATE_OUTER_GAMEPLAY:
		return To == ELayerState::STATE_MAIN_MENU
			|| To == ELayerState::STATE_CONSOLE_ACTIVE;

	case ELayerState::STATE_CONSOLE_ACTIVE:
		return To == ELayerState::STATE_OUTER_GAMEPLAY
			|| To == ELayerState::STATE_INNER_GAMEPLAY;

	case ELayerState::STATE_INNER_GAMEPLAY:
		return To == ELayerState::STATE_CONSOLE_ACTIVE;

	default:
		return false;
	}
}

bool UHorrorGameInstance::TransitionToLayer(ELayerState TargetLayer)
{
	if (!IsValidTransition(CurrentLayerState, TargetLayer))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[HorrorGameInstance] Invalid transition: %d -> %d"),
			static_cast<int32>(CurrentLayerState), static_cast<int32>(TargetLayer));
		return false;
	}

	ELayerState OldState = CurrentLayerState;

	if (TargetLayer == ELayerState::STATE_CINEMATIC)
	{
		CinematicReturnLayer = (CurrentLayerState != ELayerState::STATE_CINEMATIC)
			? CurrentLayerState
			: CinematicReturnLayer;
	}
	else if (CurrentLayerState == ELayerState::STATE_CINEMATIC)
	{
	}

	CurrentLayerState = TargetLayer;

	UE_LOG(LogTemp, Log,
		TEXT("[HorrorGameInstance] Layer transition: %d -> %d"),
		static_cast<int32>(OldState), static_cast<int32>(TargetLayer));

	OnLayerChanged.Broadcast(OldState, TargetLayer);

	return true;
}

void UHorrorGameInstance::ForceConsolePutAway()
{
	if (CurrentLayerState == ELayerState::STATE_CONSOLE_ACTIVE
		|| CurrentLayerState == ELayerState::STATE_INNER_GAMEPLAY)
	{
		UE_LOG(LogTemp, Log, TEXT("[HorrorGameInstance] ForceConsolePutAway - script triggered"));
		OnForceConsolePutAway.Broadcast();
		TransitionToLayer(ELayerState::STATE_OUTER_GAMEPLAY);
		OnConsoleToggled.Broadcast(false);
	}
}

void UHorrorGameInstance::SetFlag(FName FlagName, bool bValue)
{
	if (FlagName.IsNone())
	{
		return;
	}

	bool* Existing = NarrativeFlags.Find(FlagName);
	bool bAlreadySet = Existing && *Existing;

	if (!bAlreadySet && bValue)
	{
		NarrativeFlags.Add(FlagName, true);
		UE_LOG(LogTemp, Log, TEXT("[HorrorGameInstance] Narrative flag set: %s"), *FlagName.ToString());
		OnNarrativeFlagSet.Broadcast(FlagName);
	}
	else if (!bValue)
	{
		NarrativeFlags.Add(FlagName, false);
	}
}

bool UHorrorGameInstance::HasFlag(FName FlagName) const
{
	const bool* Found = NarrativeFlags.Find(FlagName);
	return Found && *Found;
}
