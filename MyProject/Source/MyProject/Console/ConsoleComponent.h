#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "../Core/HorrorTypes.h"
#include "ConsoleComponent.generated.h"

class UAnimMontage;
class UHorrorGameInstance;
class APlayerCharacterB;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConsoleStateChanged, EConsoleState, NewState);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UConsoleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UConsoleComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Console")
	EConsoleState CurrentState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Console|Animation")
	TObjectPtr<UAnimMontage> TakeOutAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Console|Animation")
	TObjectPtr<UAnimMontage> PutAwayAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Console|Animation")
	float AnimationDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Console|Animation")
	float InputSwitchAtProgress = 0.5f;

	UPROPERTY(BlueprintAssignable, Category = "Console|Events")
	FOnConsoleStateChanged OnConsoleStateChanged;

	UFUNCTION(BlueprintCallable, Category = "Console")
	void TakeOut();

	UFUNCTION(BlueprintCallable, Category = "Console")
	void PutAway();

	UFUNCTION(BlueprintCallable, Category = "Console")
	void ForcePutAway();

	UFUNCTION(BlueprintCallable, Category = "Console")
	void EnterInnerGameplay();

	UFUNCTION(BlueprintCallable, Category = "Console")
	void ExitInnerGameplay();

	UFUNCTION(BlueprintPure, Category = "Console")
	bool IsConsoleActive() const { return CurrentState == EConsoleState::Active; }

protected:
	UFUNCTION()
	void OnTakeOutAnimProgress(float Progress);

	UFUNCTION()
	void OnTakeOutAnimComplete();

	UFUNCTION()
	void OnPutAwayAnimComplete();

	UFUNCTION()
	void OnGameInstanceLayerChanged(ELayerState From, ELayerState To);

	void TickAnimProgress(float DeltaTime, bool bIsTakingOut);

	UPROPERTY()
	TObjectPtr<UHorrorGameInstance> CachedGameInstance;

	UPROPERTY()
	TObjectPtr<APlayerCharacterB> CachedOwnerCharacter;

private:
	float AnimElapsed = 0.0f;
	bool bAnimPlaying = false;
	bool bIsTakeOut = false;
};
