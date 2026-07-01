#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "../Core/HorrorTypes.h"
#include "HorrorEnemyAIController.generated.h"

class AHorrorEnemyPawn;
class UAIPerceptionComponent;
class UAISenseConfig_Hearing;
class UAISenseConfig_Sight;
class UBehaviorTree;
class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBlackboardData;

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API AHorrorEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AHorrorEnemyAIController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI|Components")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI|Components")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI|Components")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI|Components")
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI|Components")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy AI|Behavior")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy AI|Behavior")
	TObjectPtr<UBlackboardData> BlackboardAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Blackboard")
	FName TargetPlayerKeyName = TEXT("TargetPlayer");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Blackboard")
	FName LastKnownLocationKeyName = TEXT("LastKnownLocation");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Blackboard")
	FName CurrentAIStateKeyName = TEXT("CurrentAIState");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Blackboard")
	FName HasLineOfSightKeyName = TEXT("bHasLineOfSight");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Blackboard")
	FName AlertLevelKeyName = TEXT("AlertLevel");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Perception|Sight", meta = (ClampMin = "0.0"))
	float SightRadius = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Perception|Sight", meta = (ClampMin = "0.0"))
	float LoseSightRadius = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Perception|Sight", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float PeripheralVisionAngleDegrees = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Perception|Sight", meta = (ClampMin = "0.0"))
	float AutoSuccessRangeFromLastSeenLocation = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Perception|Hearing", meta = (ClampMin = "0.0"))
	float HearingRange = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Perception|Hearing", meta = (ClampMin = "0.0"))
	float LoSHearingRange = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Perception|Hearing", meta = (ClampMin = "0.0"))
	float HearingMaxAge = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Tick", meta = (ClampMin = "0.0"))
	float NormalTickInterval = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI|Tick", meta = (ClampMin = "0.0"))
	float ConsoleActiveTickInterval = 0.2f;

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void ForceSetAIState(EEnemyAIState NewState);

	UFUNCTION(BlueprintPure, Category = "Enemy AI")
	EEnemyAIState GetCurrentAIState() const { return CurrentAIState; }

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void SetTargetPlayer(AActor* TargetActor, bool bHasLineOfSight);

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void ClearTargetPlayer();

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void SetLastKnownLocation(FVector Location, float AlertLevel, bool bFromHearing);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy AI|Events")
	void BP_OnTargetSeen(AActor* TargetActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy AI|Events")
	void BP_OnTargetLost(AActor* TargetActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy AI|Events")
	void BP_OnNoiseHeard(FVector NoiseLocation, FName NoiseTag);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy AI|Events")
	void BP_OnNarrativeFlagSet(FName FlagName);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Enemy AI")
	EEnemyAIState CurrentAIState = EEnemyAIState::Idle;

	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void HandleNarrativeFlagSet(FName FlagName);

	UFUNCTION()
	void HandleConsoleToggled(bool bConsoleActive);

	void ConfigurePerception();
	void InitializeBehaviorTree();
	bool IsLikelyPlayer(AActor* Actor) const;
	AHorrorEnemyPawn* GetEnemyPawn() const;
	UBlackboardComponent* GetEnemyBlackboard() const;
	void SetBlackboardDefaults();
	void UpdateLineOfSight();
	void SetBlackboardState(EEnemyAIState NewState);
};
