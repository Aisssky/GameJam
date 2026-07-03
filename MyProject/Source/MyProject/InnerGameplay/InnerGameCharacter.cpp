#include "InnerGameCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"

AInnerGameCharacter::AInnerGameCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(RootComponent);
	FollowCamera->SetRelativeLocation(FVector(0.0f, 500.0f, 200.0f));
	FollowCamera->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
		MoveComp->JumpZVelocity = 600.0f;
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
		MoveComp->GravityScale = 2.0f;
	}
}

void AInnerGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	CurrentLives = MaxLives;
	LastCheckpointLocation = GetActorLocation();

	CacheSceneCapture();

	UE_LOG(LogTemp, Warning, TEXT("[InnerGameCharacter] BeginPlay at %s, CachedCapture=%s, Lives=%d"),
		*GetActorLocation().ToString(),
		CachedCapture ? *CachedCapture->GetName() : TEXT("NULL"),
		CurrentLives);
}

void AInnerGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateCaptureTransform();

	if (bIsInvincible)
	{
		InvincibilityTimer -= DeltaTime;
		if (InvincibilityTimer <= 0.0f)
		{
			bIsInvincible = false;
		}
	}
}

void AInnerGameCharacter::MoveForward(float Value)
{
	if (FMath::Abs(Value) > KINDA_SMALL_NUMBER)
	{
		AddMovementInput(FVector::ForwardVector, Value);
	}
}

void AInnerGameCharacter::DoJump()
{
	Jump();
}

void AInnerGameCharacter::StopJump()
{
	StopJumping();
}

void AInnerGameCharacter::TriggerDeath()
{
	if (bIsInvincible) return;

	CurrentLives--;
	OnCharacterDied.Broadcast();

	if (CurrentLives <= 0)
	{
		CurrentLives = 0;
		RespawnAtCheckpoint();
		CurrentLives = MaxLives;
	}
	else
	{
		RespawnAtCheckpoint();
	}

	bIsInvincible = true;
	InvincibilityTimer = RespawnInvincibilityTime;
}

void AInnerGameCharacter::RespawnAtCheckpoint()
{
	const FVector RespawnLoc = bHasCheckpoint ? LastCheckpointLocation : LastCheckpointLocation;
	SetActorLocation(RespawnLoc);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->Velocity = FVector::ZeroVector;
	}
}

void AInnerGameCharacter::SetCheckpoint(FVector Location, int32 Index)
{
	LastCheckpointLocation = Location;
	bHasCheckpoint = true;
	OnCheckpointSet.Broadcast(Index);
}

int32 AInnerGameCharacter::AddCollectible(int32 Points)
{
	CollectedCount += Points;
	OnCollectibleAdded.Broadcast(CollectedCount);
	return CollectedCount;
}

void AInnerGameCharacter::CacheSceneCapture()
{
	for (TActorIterator<ASceneCapture2D> It(GetWorld()); It; ++It)
	{
		ASceneCapture2D* Capture = *It;
		if (IsValid(Capture) && Capture->GetAttachParentActor() == nullptr)
		{
			CachedCapture = Capture;
			UpdateCaptureTransform();
			UE_LOG(LogTemp, Warning, TEXT("[InnerGameCharacter] Cached SceneCapture '%s' (not attached, position synced in Tick)"),
				*Capture->GetName());
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[InnerGameCharacter] CacheSceneCapture: No unattached SceneCapture2D found!"));
}

void AInnerGameCharacter::UpdateCaptureTransform()
{
	if (!CachedCapture || !IsValid(CachedCapture))
	{
		return;
	}

	const FVector CharLocation = GetActorLocation();
	const FVector TargetLocation = CharLocation + CaptureOffset;
	CachedCapture->SetActorLocation(TargetLocation);

	CachedCapture->SetActorRotation(FRotator(0.0f, CaptureYawOffset, 0.0f));
}
