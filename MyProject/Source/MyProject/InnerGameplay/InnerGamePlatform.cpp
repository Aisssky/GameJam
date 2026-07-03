#include "InnerGamePlatform.h"
#include "Components/StaticMeshComponent.h"

AInnerGamePlatform::AInnerGamePlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	RootComponent = PlatformMesh;
	PlatformMesh->SetMobility(EComponentMobility::Movable);
}

void AInnerGamePlatform::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = GetActorLocation();
	bIsMoving = bStartMoving;
}

void AInnerGamePlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsMoving) return;

	const FVector Direction = MoveDirection.GetSafeNormal();
	if (Direction.IsNearlyZero()) return;

	const float Step = MoveSpeed * DeltaTime;
	CurrentDistance += bMovingForward ? Step : -Step;

	float ClampedDist = CurrentDistance;
	if (bPingPong)
	{
		if (CurrentDistance >= MoveDistance)
		{
			CurrentDistance = MoveDistance;
			bMovingForward = false;
			ClampedDist = CurrentDistance;
		}
		else if (CurrentDistance <= 0.0f)
		{
			CurrentDistance = 0.0f;
			bMovingForward = true;
			ClampedDist = CurrentDistance;
		}
	}
	else
	{
		ClampedDist = FMath::Fmod(CurrentDistance, MoveDistance);
		if (ClampedDist < 0.0f) ClampedDist += MoveDistance;
	}

	SetActorLocation(StartLocation + Direction * ClampedDist);
}

void AInnerGamePlatform::StartMoving()
{
	bIsMoving = true;
}

void AInnerGamePlatform::StopMoving()
{
	bIsMoving = false;
}
