#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BlackboardKeySelector.h"
#include "BTDecorator_PlayerIsAware.generated.h"

UCLASS()
class MYPROJECT_API UBTDecorator_PlayerIsAware : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_PlayerIsAware();

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Awareness", meta = (ClampMin = "0.0"))
	float AwarenessDistance = 1400.0f;

	UPROPERTY(EditAnywhere, Category = "Awareness", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float AwarenessHalfAngleDegrees = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Awareness")
	bool bRequireLineTrace = true;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
