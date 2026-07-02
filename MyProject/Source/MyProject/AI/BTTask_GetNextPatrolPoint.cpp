#include "BTTask_GetNextPatrolPoint.h"

#include "HorrorEnemyPawn.h"
#include "AIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UBTTask_GetNextPatrolPoint::UBTTask_GetNextPatrolPoint()
{
	NodeName = TEXT("Get Next Patrol Point");
	PatrolPointKey.SelectedKeyName = TEXT("PatrolPoint");
	PatrolPointKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_GetNextPatrolPoint, PatrolPointKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_GetNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AHorrorEnemyPawn* EnemyPawn = Cast<AHorrorEnemyPawn>(OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr);
	if (!EnemyPawn)
	{
		return EBTNodeResult::Failed;
	}

	AActor* PatrolPoint = EnemyPawn->GetNextPatrolPoint();
	if (!PatrolPoint)
	{
		return EBTNodeResult::Failed;
	}

	if (UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent())
	{
		Blackboard->SetValueAsObject(PatrolPointKey.SelectedKeyName, PatrolPoint);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
