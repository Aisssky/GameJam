#include "HorrorEnemyPawn.h"

#include "HorrorEnemyAIController.h"
#include "../Core/HorrorGameInstance.h"
#include "../Player/PlayerCharacterB.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

AHorrorEnemyPawn::AHorrorEnemyPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AHorrorEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetVisibility(false, true);
		MeshComponent->SetHiddenInGame(true, true);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = IdleWalkSpeed;
		MoveComp->MaxAcceleration = 1800.0f;
		MoveComp->BrakingDecelerationWalking = 1600.0f;
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	}

	bUseControllerRotationYaw = false;
	SetActorTickInterval(NormalTickInterval);
}

void AHorrorEnemyPawn::BeginPlay()
{
	Super::BeginPlay();

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetVisibility(false, true);
		MeshComponent->SetHiddenInGame(true, true);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	ApplyMovementSpeedForState();

	if (UHorrorGameInstance* GI = Cast<UHorrorGameInstance>(GetGameInstance()))
	{
		GI->OnEnemyShot.AddDynamic(this, &AHorrorEnemyPawn::HandleEnemyShot);
		GI->OnNarrativeFlagSet.AddDynamic(this, &AHorrorEnemyPawn::HandleNarrativeFlagSet);
		GI->OnConsoleToggled.AddDynamic(this, &AHorrorEnemyPawn::ApplyConsoleTickMode);
	}
}

void AHorrorEnemyPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UHorrorGameInstance* GI = Cast<UHorrorGameInstance>(GetGameInstance()))
	{
		GI->OnEnemyShot.RemoveDynamic(this, &AHorrorEnemyPawn::HandleEnemyShot);
		GI->OnNarrativeFlagSet.RemoveDynamic(this, &AHorrorEnemyPawn::HandleNarrativeFlagSet);
		GI->OnConsoleToggled.RemoveDynamic(this, &AHorrorEnemyPawn::ApplyConsoleTickMode);
	}

	Super::EndPlay(EndPlayReason);
}

void AHorrorEnemyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCaughtSequenceActive || bStunned || CurrentAIState != EEnemyAIState::Chasing)
	{
		UpdateFootstepAudio(DeltaTime);
		return;
	}

	UpdateFootstepAudio(DeltaTime);

	if (APawn* PlayerPawn = ResolvePlayerPawn())
	{
		if (IsPlayerCatchable(PlayerPawn))
		{
			BeginCaughtPlayerSequence(PlayerPawn);
		}
	}
}

void AHorrorEnemyPawn::ForceSetAIState(EEnemyAIState NewState)
{
	if (CurrentAIState == NewState)
	{
		return;
	}

	CurrentAIState = NewState;
	FootstepTimeAccumulator = 0.0f;
	ApplyMovementSpeedForState();
	OnAIStateChanged.Broadcast(NewState);
	BP_OnAIStateChanged(NewState);
}

void AHorrorEnemyPawn::SetTargetPlayer(AActor* InTargetPlayer)
{
	TargetPlayer = InTargetPlayer;
}

void AHorrorEnemyPawn::RequestChaseAudio()
{
	BP_OnChaseAudioRequested();
}

void AHorrorEnemyPawn::RequestAmbientEvent()
{
	if (AmbientEventSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AmbientEventSound, GetActorLocation());
	}

	BP_OnAmbientEventRequested();
}

void AHorrorEnemyPawn::RequestFootstepAudio()
{
	if (FootstepSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FootstepSound,
			GetActorLocation(),
			FootstepVolumeMultiplier,
			FootstepPitchMultiplier);
	}

	BP_OnFootstepAudioRequested();
}

void AHorrorEnemyPawn::HandleSilverBulletHit(float StunDuration)
{
	if (StunDuration <= 0.0f)
	{
		return;
	}

	bStunned = true;

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	BP_OnSilverBulletHit(StunDuration);

	GetWorldTimerManager().ClearTimer(StunTimerHandle);
	GetWorldTimerManager().SetTimer(
		StunTimerHandle,
		this,
		&AHorrorEnemyPawn::FinishStun,
		StunDuration,
		false);
}

void AHorrorEnemyPawn::BeginCaughtPlayerSequence(APawn* PlayerPawn)
{
	if (!PlayerPawn || bCaughtSequenceActive)
	{
		return;
	}

	bCaughtSequenceActive = true;
	PendingCaughtPlayer = PlayerPawn;

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}

	if (CaughtScareSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CaughtScareSound, GetActorLocation());
	}

	if (APlayerCharacterB* PlayerB = Cast<APlayerCharacterB>(PlayerPawn))
	{
		PlayerB->DisableMovement();
	}

	OnCaughtPlayer.Broadcast(PlayerPawn);
	BP_OnCaughtPlayer(PlayerPawn, StaticScreenDuration);

	GetWorldTimerManager().ClearTimer(CaughtTimerHandle);
	GetWorldTimerManager().SetTimer(
		CaughtTimerHandle,
		this,
		&AHorrorEnemyPawn::FinishCaughtPlayerSequence,
		StaticScreenDuration,
		false);
}

void AHorrorEnemyPawn::ApplyConsoleTickMode(bool bConsoleActive)
{
	SetActorTickInterval(bConsoleActive ? ConsoleActiveTickInterval : NormalTickInterval);
}

bool AHorrorEnemyPawn::FindStalkLocationNearTarget(FVector& OutLocation) const
{
	const AActor* Target = TargetPlayer ? TargetPlayer.Get() : ResolvePlayerPawn();
	if (!Target)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
	if (!NavSystem)
	{
		return false;
	}

	const FVector TargetLocation = Target->GetActorLocation();
	const FVector TargetForward = Target->GetActorForwardVector().GetSafeNormal2D();
	const int32 SampleCount = FMath::Max(1, StalkLocationSampleCount);

	for (int32 Index = 0; Index < SampleCount; ++Index)
	{
		const float AngleRadians = FMath::FRandRange(0.0f, TWO_PI);
		const float Distance = FMath::FRandRange(MinStalkDistance, FMath::Max(MinStalkDistance, MaxStalkDistance));
		const FVector Direction(FMath::Cos(AngleRadians), FMath::Sin(AngleRadians), 0.0f);
		const FVector Candidate = TargetLocation + Direction * Distance;

		const FVector ToCandidate = (Candidate - TargetLocation).GetSafeNormal2D();
		const bool bMostlyOutsidePlayerView = FVector::DotProduct(TargetForward, ToCandidate) < 0.15f;
		if (!bMostlyOutsidePlayerView)
		{
			continue;
		}

		FNavLocation ProjectedLocation;
		if (NavSystem->ProjectPointToNavigation(Candidate, ProjectedLocation, StalkProjectionExtent))
		{
			OutLocation = ProjectedLocation.Location;
			return true;
		}
	}

	FNavLocation FallbackLocation;
	if (NavSystem->GetRandomReachablePointInRadius(TargetLocation, MaxStalkDistance, FallbackLocation))
	{
		OutLocation = FallbackLocation.Location;
		return true;
	}

	return false;
}

AActor* AHorrorEnemyPawn::GetNextPatrolPoint()
{
	if (PatrolPoints.Num() == 0)
	{
		return nullptr;
	}

	if (!PatrolPoints.IsValidIndex(PatrolIndex))
	{
		PatrolIndex = 0;
	}

	AActor* Result = PatrolPoints[PatrolIndex];

	if (bLoopPatrol)
	{
		PatrolIndex = (PatrolIndex + 1) % PatrolPoints.Num();
	}
	else
	{
		PatrolIndex = FMath::Min(PatrolIndex + 1, PatrolPoints.Num() - 1);
	}

	return Result;
}

void AHorrorEnemyPawn::HandleEnemyShot(AActor* EnemyActor, float StunDuration)
{
	if (EnemyActor == this || (EnemyActor && EnemyActor->GetOwner() == this))
	{
		HandleSilverBulletHit(StunDuration);
	}
}

void AHorrorEnemyPawn::HandleNarrativeFlagSet(FName FlagName)
{
	BP_OnNarrativeFlagSet(FlagName);
}

void AHorrorEnemyPawn::ApplyMovementSpeedForState()
{
	float DesiredSpeed = IdleWalkSpeed;

	switch (CurrentAIState)
	{
	case EEnemyAIState::Investigating:
		DesiredSpeed = InvestigateSpeed;
		break;
	case EEnemyAIState::Chasing:
		DesiredSpeed = ChaseSpeed;
		break;
	case EEnemyAIState::Stalking:
		DesiredSpeed = StalkSpeed;
		break;
	case EEnemyAIState::Idle:
	default:
		DesiredSpeed = IdleWalkSpeed;
		break;
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = DesiredSpeed;
	}
}

void AHorrorEnemyPawn::UpdateFootstepAudio(float DeltaTime)
{
	if (!bPlayFootstepsAutomatically || bCaughtSequenceActive || bStunned)
	{
		FootstepTimeAccumulator = 0.0f;
		return;
	}

	const float Speed2D = GetVelocity().Size2D();
	if (Speed2D < MinSpeedForFootsteps)
	{
		FootstepTimeAccumulator = 0.0f;
		return;
	}

	FootstepTimeAccumulator += DeltaTime;

	const float Interval = GetFootstepIntervalForCurrentState();
	if (FootstepTimeAccumulator >= Interval)
	{
		FootstepTimeAccumulator = 0.0f;
		RequestFootstepAudio();
	}
}

float AHorrorEnemyPawn::GetFootstepIntervalForCurrentState() const
{
	switch (CurrentAIState)
	{
	case EEnemyAIState::Investigating:
		return InvestigateFootstepInterval;
	case EEnemyAIState::Chasing:
		return ChaseFootstepInterval;
	case EEnemyAIState::Stalking:
		return StalkFootstepInterval;
	case EEnemyAIState::Idle:
	default:
		return IdleFootstepInterval;
	}
}

void AHorrorEnemyPawn::FinishCaughtPlayerSequence()
{
	APawn* PlayerPawn = PendingCaughtPlayer;

	if (UHorrorGameInstance* GI = Cast<UHorrorGameInstance>(GetGameInstance()))
	{
		GI->ApplyLoadedPlayerTransform(PlayerPawn);
	}

	if (APlayerCharacterB* PlayerB = Cast<APlayerCharacterB>(PlayerPawn))
	{
		PlayerB->EnableMovement();
	}

	if (PlayerPawn)
	{
		MoveEnemyAwayFromRespawnedPlayer(PlayerPawn);
		BP_OnRespawnedPlayer(PlayerPawn);
	}

	PendingCaughtPlayer = nullptr;
	bCaughtSequenceActive = false;
	ForceSetAIState(EEnemyAIState::Investigating);
}

void AHorrorEnemyPawn::FinishStun()
{
	bStunned = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
		ApplyMovementSpeedForState();
	}
}

bool AHorrorEnemyPawn::IsPlayerCatchable(APawn* PlayerPawn) const
{
	if (!PlayerPawn)
	{
		return false;
	}

	return FVector::DistSquared(GetActorLocation(), PlayerPawn->GetActorLocation()) <= FMath::Square(CatchDistance);
}

APawn* AHorrorEnemyPawn::ResolvePlayerPawn() const
{
	if (APawn* TargetPawn = Cast<APawn>(TargetPlayer))
	{
		return TargetPawn;
	}

	return UGameplayStatics::GetPlayerPawn(this, 0);
}

void AHorrorEnemyPawn::MoveEnemyAwayFromRespawnedPlayer(APawn* PlayerPawn)
{
	if (!PlayerPawn || EnemyRespawnDistanceAfterCatch <= 0.0f)
	{
		return;
	}

	const FVector DesiredLocation = PlayerPawn->GetActorLocation()
		- PlayerPawn->GetActorForwardVector().GetSafeNormal2D() * EnemyRespawnDistanceAfterCatch;

	if (UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld()))
	{
		FNavLocation ProjectedLocation;
		if (NavSystem->ProjectPointToNavigation(DesiredLocation, ProjectedLocation, FVector(500.0f, 500.0f, 500.0f)))
		{
			SetActorLocation(ProjectedLocation.Location);
			return;
		}
	}

	SetActorLocation(DesiredLocation);
}
