#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HorrorTypes.h"
#include "HorrorGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLayerChanged, ELayerState, FromLayer, ELayerState, ToLayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConsoleToggled, bool, bConsoleActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnForceConsolePutAway);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCreated, const FString&, SlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveDeleted, const FString&, SlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLoaded, const FString&, SlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeFlagSet, FName, FlagName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWakeUp);

UCLASS(BlueprintType)
class MYPROJECT_API UHorrorGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Layer State")
	ELayerState CurrentLayerState;

	UFUNCTION(BlueprintCallable, Category = "Layer State")
	bool TransitionToLayer(ELayerState TargetLayer);

	UFUNCTION(BlueprintPure, Category = "Layer State")
	bool IsValidTransition(ELayerState From, ELayerState To) const;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnLayerChanged OnLayerChanged;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnConsoleToggled OnConsoleToggled;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnForceConsolePutAway OnForceConsolePutAway;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnSaveCreated OnSaveCreated;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnSaveDeleted OnSaveDeleted;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnGameLoaded OnGameLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnNarrativeFlagSet OnNarrativeFlagSet;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnGameCompleted OnGameCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnWakeUp OnWakeUp;

	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void SetFlag(FName FlagName, bool bValue = true);

	UFUNCTION(BlueprintPure, Category = "Narrative")
	bool HasFlag(FName FlagName) const;

	UFUNCTION(BlueprintPure, Category = "Narrative")
	const TMap<FName, bool>& GetAllFlags() const { return NarrativeFlags; }

	UFUNCTION(BlueprintCallable, Category = "Console")
	void ForceConsolePutAway();

	UPROPERTY(BlueprintReadOnly, Category = "Save System")
	bool bOriginalSaveUnlocked;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Narrative")
	TMap<FName, bool> NarrativeFlags;

	UPROPERTY()
	ELayerState PreviousLayerState;

	UPROPERTY()
	ELayerState CinematicReturnLayer;
};
