#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "../Core/HorrorTypes.h"
#include "BTTask_SetEnemyAIState.generated.h"

UCLASS()
class MYPROJECT_API UBTTask_SetEnemyAIState : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetEnemyAIState();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI")
	EEnemyAIState NewState = EEnemyAIState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FName CurrentAIStateKeyName = TEXT("CurrentAIState");

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
