#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "../Core/HorrorTypes.h"
#include "ConsoleComponent.generated.h"

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Console")
	float TransitionCooldown = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Console|Actor")
	TSubclassOf<AActor> ConsoleActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Console|Actor")
	FName AttachSocketName = TEXT("hand_r");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Console|Actor")
	FVector ConsoleAttachOffset = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Console|Actor")
	FRotator ConsoleAttachRotation = FRotator(0.0f, 90.0f, 0.0f);

	UPROPERTY(BlueprintReadOnly, Category = "Console|Actor")
	TObjectPtr<AActor> ConsoleActorInstance;

	UPROPERTY(BlueprintAssignable, Category = "Console|Events")
	FOnConsoleStateChanged OnConsoleStateChanged;

	UFUNCTION(BlueprintCallable, Category = "Console")
	void TakeOut();

	UFUNCTION(BlueprintCallable, Category = "Console")
	void PutAway();

	UFUNCTION(BlueprintCallable, Category = "Console")
	void ForcePutAway();

	UFUNCTION(BlueprintCallable, Category = "Console")
	void SpawnConsoleActor();

	UFUNCTION(BlueprintCallable, Category = "Console")
	void DestroyConsoleActor();

	UFUNCTION(BlueprintPure, Category = "Console")
	bool IsConsoleActorActive() const { return ConsoleActorInstance != nullptr; }

	UFUNCTION(BlueprintCallable, Category = "Console")
	void EnterInnerGameplay();

	UFUNCTION(BlueprintCallable, Category = "Console")
	void ExitInnerGameplay();

	UFUNCTION(BlueprintPure, Category = "Console")
	bool IsConsoleActive() const { return CurrentState == EConsoleState::Active; }

	UFUNCTION(BlueprintPure, Category = "Console")
	bool IsHoldingConsole() const { return ConsoleActorInstance != nullptr; }

protected:
	UFUNCTION()
	void OnGameInstanceLayerChanged(ELayerState From, ELayerState To);

	UPROPERTY()
	TObjectPtr<UHorrorGameInstance> CachedGameInstance;

	UPROPERTY()
	TObjectPtr<APlayerCharacterB> CachedOwnerCharacter;

	UFUNCTION()
	void UnlockTransition();

private:
	bool bTransitionLocked = false;
	FTimerHandle TransitionLockTimer;
};
