#include "BTDecorator_PlayerIsAware.h"

#include "AIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

UBTDecorator_PlayerIsAware::UBTDecorator_PlayerIsAware()
{
	NodeName = TEXT("Player Is Aware");
	TargetActorKey.SelectedKeyName = TEXT("TargetPlayer");
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_PlayerIsAware, TargetActorKey), AActor::StaticClass());
}

bool UBTDecorator_PlayerIsAware::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const APawn* EnemyPawn = AIController ? AIController->GetPawn() : nullptr;
	const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	const APawn* PlayerPawn = Blackboard ? Cast<APawn>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;

	if (!EnemyPawn || !PlayerPawn)
	{
		return false;
	}

	const FVector EnemyLocation = EnemyPawn->GetActorLocation();
	FVector ViewLocation = PlayerPawn->GetActorLocation();
	FRotator ViewRotation = PlayerPawn->GetActorRotation();

	if (const AController* PlayerController = PlayerPawn->GetController())
	{
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}

	const float DistanceToEnemy = FVector::Dist(ViewLocation, EnemyLocation);
	if (DistanceToEnemy > AwarenessDistance)
	{
		return false;
	}

	const FVector ToEnemy = (EnemyLocation - ViewLocation).GetSafeNormal();
	const float Dot = FVector::DotProduct(ViewRotation.Vector(), ToEnemy);
	const float RequiredDot = FMath::Cos(FMath::DegreesToRadians(AwarenessHalfAngleDegrees));
	if (Dot < RequiredDot)
	{
		return false;
	}

	if (!bRequireLineTrace)
	{
		return true;
	}

	UWorld* World = EnemyPawn->GetWorld();
	if (!World)
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerPawn);

	const bool bHit = World->LineTraceSingleByChannel(
		Hit,
		ViewLocation,
		EnemyLocation,
		ECC_Visibility,
		Params);

	return !bHit || Hit.GetActor() == EnemyPawn;
}
