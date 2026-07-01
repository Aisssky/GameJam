#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BlackboardKeySelector.h"
#include "BTDecorator_DistanceToTarget.generated.h"

UENUM(BlueprintType)
enum class EEnemyDistanceComparison : uint8
{
	Less UMETA(DisplayName = "Less"),
	LessOrEqual UMETA(DisplayName = "Less Or Equal"),
	Greater UMETA(DisplayName = "Greater"),
	GreaterOrEqual UMETA(DisplayName = "Greater Or Equal")
};

UCLASS()
class MYPROJECT_API UBTDecorator_DistanceToTarget : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_DistanceToTarget();

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0"))
	float Distance = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Condition")
	EEnemyDistanceComparison Comparison = EEnemyDistanceComparison::Greater;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
