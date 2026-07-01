#include "BTTask_FindStalkLocation.h"

#include "HorrorEnemyPawn.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UBTTask_FindStalkLocation::UBTTask_FindStalkLocation()
{
	NodeName = TEXT("Find Stalk Location");
	TargetActorKey.SelectedKeyName = TEXT("TargetPlayer");
	StalkLocationKey.SelectedKeyName = TEXT("StalkLocation");
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindStalkLocation, TargetActorKey), AActor::StaticClass());
	StalkLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindStalkLocation, StalkLocationKey));
}

EBTNodeResult::Type UBTTask_FindStalkLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AHorrorEnemyPawn* EnemyPawn = Cast<AHorrorEnemyPawn>(OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr);
	if (!Blackboard || !EnemyPawn)
	{
		return EBTNodeResult::Failed;
	}

	if (AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)))
	{
		EnemyPawn->SetTargetPlayer(TargetActor);
	}

	FVector StalkLocation = FVector::ZeroVector;
	if (!EnemyPawn->FindStalkLocationNearTarget(StalkLocation))
	{
		return EBTNodeResult::Failed;
	}

	Blackboard->SetValueAsVector(StalkLocationKey.SelectedKeyName, StalkLocation);
	return EBTNodeResult::Succeeded;
}
