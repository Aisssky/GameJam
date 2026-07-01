#include "BTDecorator_DistanceToTarget.h"

#include "AIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

UBTDecorator_DistanceToTarget::UBTDecorator_DistanceToTarget()
{
	NodeName = TEXT("Distance To Target");
	TargetActorKey.SelectedKeyName = TEXT("TargetPlayer");
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_DistanceToTarget, TargetActorKey), AActor::StaticClass());
}

bool UBTDecorator_DistanceToTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* ControlledPawn = AIController ? AIController->GetPawn() : nullptr;
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	const AActor* TargetActor = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;

	if (!ControlledPawn || !TargetActor)
	{
		return false;
	}

	const float ActualDistance = FVector::Dist(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
	switch (Comparison)
	{
	case EEnemyDistanceComparison::Less:
		return ActualDistance < Distance;
	case EEnemyDistanceComparison::LessOrEqual:
		return ActualDistance <= Distance;
	case EEnemyDistanceComparison::Greater:
		return ActualDistance > Distance;
	case EEnemyDistanceComparison::GreaterOrEqual:
		return ActualDistance >= Distance;
	default:
		return false;
	}
}
