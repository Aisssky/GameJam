#include "BTTask_SetEnemyAIState.h"

#include "HorrorEnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetEnemyAIState::UBTTask_SetEnemyAIState()
{
	NodeName = TEXT("Set Enemy AI State");
}

EBTNodeResult::Type UBTTask_SetEnemyAIState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AHorrorEnemyAIController* AIController = Cast<AHorrorEnemyAIController>(OwnerComp.GetAIOwner()))
	{
		AIController->ForceSetAIState(NewState);
		return EBTNodeResult::Succeeded;
	}

	if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
	{
		Blackboard->SetValueAsEnum(CurrentAIStateKeyName, static_cast<uint8>(NewState));
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
