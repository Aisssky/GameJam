#include "BTTask_UpdateLastKnownLocation.h"

#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UBTTask_UpdateLastKnownLocation::UBTTask_UpdateLastKnownLocation()
{
	NodeName = TEXT("Update Last Known Location");
	TargetActorKey.SelectedKeyName = TEXT("TargetPlayer");
	LastKnownLocationKey.SelectedKeyName = TEXT("LastKnownLocation");
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_UpdateLastKnownLocation, TargetActorKey), AActor::StaticClass());
	LastKnownLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_UpdateLastKnownLocation, LastKnownLocationKey));
}

EBTNodeResult::Type UBTTask_UpdateLastKnownLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	Blackboard->SetValueAsVector(LastKnownLocationKey.SelectedKeyName, TargetActor->GetActorLocation());
	return EBTNodeResult::Succeeded;
}
