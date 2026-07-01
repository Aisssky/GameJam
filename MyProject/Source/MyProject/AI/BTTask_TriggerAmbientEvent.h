#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TriggerAmbientEvent.generated.h"

UCLASS()
class MYPROJECT_API UBTTask_TriggerAmbientEvent : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_TriggerAmbientEvent();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
