#include "BTTask_TriggerAmbientEvent.h"

#include "HorrorEnemyPawn.h"

UBTTask_TriggerAmbientEvent::UBTTask_TriggerAmbientEvent()
{
	NodeName = TEXT("Trigger Ambient Event");
}

EBTNodeResult::Type UBTTask_TriggerAmbientEvent::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AHorrorEnemyPawn* EnemyPawn = Cast<AHorrorEnemyPawn>(OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr);
	if (!EnemyPawn)
	{
		return EBTNodeResult::Failed;
	}

	EnemyPawn->RequestAmbientEvent();
	return EBTNodeResult::Succeeded;
}
