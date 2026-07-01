#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PlayChaseAudio.generated.h"

UCLASS()
class MYPROJECT_API UBTTask_PlayChaseAudio : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PlayChaseAudio();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
