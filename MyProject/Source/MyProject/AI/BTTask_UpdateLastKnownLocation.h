#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BlackboardKeySelector.h"
#include "BTTask_UpdateLastKnownLocation.generated.h"

UCLASS()
class MYPROJECT_API UBTTask_UpdateLastKnownLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UpdateLastKnownLocation();

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector LastKnownLocationKey;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
