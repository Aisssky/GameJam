#include "PlayerCharacterB.h"
#include "../Console/ConsoleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

APlayerCharacterB::APlayerCharacterB()
{
	PrimaryActorTick.bCanEverTick = true;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FirstPersonCamera->bUsePawnControlRotation = true;

	ConsoleComponent = CreateDefaultSubobject<UConsoleComponent>(TEXT("ConsoleComponent"));

	bUseControllerRotationYaw = true;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
		MoveComp->MaxAcceleration = 2048.0f;
		MoveComp->bOrientRotationToMovement = false;
	}
}

void APlayerCharacterB::BeginPlay()
{
	Super::BeginPlay();

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
	}
}

void APlayerCharacterB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacterB::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IA_Move)
		{
			EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &APlayerCharacterB::OnMove);
		}
		if (IA_Look)
		{
			EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &APlayerCharacterB::OnLook);
		}
	}
}

void APlayerCharacterB::OnMove(const FInputActionValue& Value)
{
	if (MovementLockCount > 0) return;

	FVector2D MoveVector = Value.Get<FVector2D>();
	if (Controller)
	{
		AddMovementInput(GetActorForwardVector(), MoveVector.Y);
		AddMovementInput(GetActorRightVector(), MoveVector.X);
	}
}

void APlayerCharacterB::OnLook(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();
	if (Controller)
	{
		AddControllerYawInput(LookVector.X * MouseSensitivity);
		AddControllerPitchInput(LookVector.Y * MouseSensitivity);
	}
}

void APlayerCharacterB::DisableMovement()
{
	MovementLockCount++;
	bMovementEnabled = false;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_None);
	}
}

void APlayerCharacterB::EnableMovement()
{
	MovementLockCount = FMath::Max(0, MovementLockCount - 1);
	bMovementEnabled = (MovementLockCount == 0);

	if (bMovementEnabled)
	{
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}
	}
}

void APlayerCharacterB::SetSpeedMultiplier(float Multiplier)
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed * Multiplier;
	}
}

void APlayerCharacterB::ApplyFatigueEffect(float SoulFragmentationLevel)
{
	if (SoulFragmentationLevel > 0.7f)
	{
		SetSpeedMultiplier(FatigueSpeedMultiplier);
	}
	else
	{
		SetSpeedMultiplier(1.0f);
	}
}
