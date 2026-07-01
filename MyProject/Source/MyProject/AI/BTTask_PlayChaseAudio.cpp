#include "BTTask_PlayChaseAudio.h"

#include "HorrorEnemyPawn.h"

UBTTask_PlayChaseAudio::UBTTask_PlayChaseAudio()
{
	NodeName = TEXT("Play Chase Audio");
}

EBTNodeResult::Type UBTTask_PlayChaseAudio::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AHorrorEnemyPawn* EnemyPawn = Cast<AHorrorEnemyPawn>(OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr);
	if (!EnemyPawn)
	{
		return EBTNodeResult::Failed;
	}

	EnemyPawn->RequestChaseAudio();
	return EBTNodeResult::Succeeded;
}
