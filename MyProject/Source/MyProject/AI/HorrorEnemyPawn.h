#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Core/HorrorTypes.h"
#include "HorrorEnemyPawn.generated.h"

class USoundBase;
class UHorrorGameInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyAIStateChangedSignature, EEnemyAIState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyCaughtPlayerSignature, APawn*, PlayerPawn);

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API AHorrorEnemyPawn : public ACharacter
{
	GENERATED_BODY()

public:
	AHorrorEnemyPawn();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Movement", meta = (ClampMin = "0.0"))
	float IdleWalkSpeed = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Movement", meta = (ClampMin = "0.0"))
	float InvestigateSpeed = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Movement", meta = (ClampMin = "0.0"))
	float StalkSpeed = 190.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Movement", meta = (ClampMin = "0.0"))
	float ChaseSpeed = 480.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Catch", meta = (ClampMin = "1.0"))
	float CatchDistance = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Catch", meta = (ClampMin = "0.0"))
	float StaticScreenDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Catch", meta = (ClampMin = "0.0"))
	float EnemyRespawnDistanceAfterCatch = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Tick", meta = (ClampMin = "0.0"))
	float NormalTickInterval = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Tick", meta = (ClampMin = "0.0"))
	float ConsoleActiveTickInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Patrol")
	TArray<TObjectPtr<AActor>> PatrolPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Patrol")
	bool bLoopPatrol = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Stalk", meta = (ClampMin = "0.0"))
	float MinStalkDistance = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Stalk", meta = (ClampMin = "0.0"))
	float MaxStalkDistance = 1300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Stalk", meta = (ClampMin = "1"))
	int32 StalkLocationSampleCount = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Stalk")
	FVector StalkProjectionExtent = FVector(300.0f, 300.0f, 300.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Audio")
	TObjectPtr<USoundBase> CaughtScareSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Audio")
	TObjectPtr<USoundBase> AmbientEventSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Audio")
	TObjectPtr<USoundBase> FootstepSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Audio")
	bool bPlayFootstepsAutomatically = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Audio", meta = (ClampMin = "0.0"))
	float MinSpeedForFootsteps = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Audio", meta = (ClampMin = "0.05"))
	float IdleFootstepInterval = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Audio", meta = (ClampMin = "0.05"))
	float InvestigateFootstepInterval = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Audio", meta = (ClampMin = "0.05"))
	float StalkFootstepInterval = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Audio", meta = (ClampMin = "0.05"))
	float ChaseFootstepInterval = 0.32f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Audio", meta = (ClampMin = "0.0"))
	float FootstepVolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Audio", meta = (ClampMin = "0.1"))
	float FootstepPitchMultiplier = 1.0f;

	UPROPERTY(BlueprintAssignable, Category = "Enemy AI|Events")
	FOnEnemyAIStateChangedSignature OnAIStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Enemy AI|Events")
	FOnEnemyCaughtPlayerSignature OnCaughtPlayer;

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void ForceSetAIState(EEnemyAIState NewState);

	UFUNCTION(BlueprintPure, Category = "Enemy AI")
	EEnemyAIState GetCurrentAIState() const { return CurrentAIState; }

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void SetTargetPlayer(AActor* InTargetPlayer);

	UFUNCTION(BlueprintPure, Category = "Enemy AI")
	AActor* GetTargetPlayer() const { return TargetPlayer; }

	UFUNCTION(BlueprintCallable, Category = "Enemy AI|Audio")
	void RequestChaseAudio();

	UFUNCTION(BlueprintCallable, Category = "Enemy AI|Audio")
	void RequestAmbientEvent();

	UFUNCTION(BlueprintCallable, Category = "Enemy AI|Audio")
	void RequestFootstepAudio();

	UFUNCTION(BlueprintCallable, Category = "Enemy AI|Combat")
	void HandleSilverBulletHit(float StunDuration);

	UFUNCTION(BlueprintCallable, Category = "Enemy AI|Catch")
	void BeginCaughtPlayerSequence(APawn* PlayerPawn);

	UFUNCTION(BlueprintCallable, Category = "Enemy AI|Tick")
	void ApplyConsoleTickMode(bool bConsoleActive);

	UFUNCTION(BlueprintCallable, Category = "Enemy AI|Stalk")
	bool FindStalkLocationNearTarget(FVector& OutLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Enemy AI|Patrol")
	AActor* GetNextPatrolPoint();

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy AI|Events")
	void BP_OnAIStateChanged(EEnemyAIState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy AI|Events")
	void BP_OnChaseAudioRequested();

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy AI|Events")
	void BP_OnAmbientEventRequested();

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy AI|Events")
	void BP_OnFootstepAudioRequested();

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy AI|Events")
	void BP_OnCaughtPlayer(APawn* PlayerPawn, float ScreenDuration);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy AI|Events")
	void BP_OnRespawnedPlayer(APawn* PlayerPawn);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy AI|Events")
	void BP_OnSilverBulletHit(float StunDuration);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy AI|Events")
	void BP_OnNarrativeFlagSet(FName FlagName);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Enemy AI")
	EEnemyAIState CurrentAIState = EEnemyAIState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy AI")
	TObjectPtr<AActor> TargetPlayer;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy AI|Catch")
	TObjectPtr<APawn> PendingCaughtPlayer;

	UFUNCTION()
	void HandleEnemyShot(AActor* EnemyActor, float StunDuration);

	UFUNCTION()
	void HandleNarrativeFlagSet(FName FlagName);

	void ApplyMovementSpeedForState();
	void UpdateFootstepAudio(float DeltaTime);
	float GetFootstepIntervalForCurrentState() const;
	void FinishCaughtPlayerSequence();
	void FinishStun();
	bool IsPlayerCatchable(APawn* PlayerPawn) const;
	APawn* ResolvePlayerPawn() const;
	void MoveEnemyAwayFromRespawnedPlayer(APawn* PlayerPawn);

	int32 PatrolIndex = 0;
	bool bCaughtSequenceActive = false;
	bool bStunned = false;
	float FootstepTimeAccumulator = 0.0f;

	FTimerHandle CaughtTimerHandle;
	FTimerHandle StunTimerHandle;
};
