#include "PlayerCharacterB.h"
#include "../Console/ConsoleComponent.h"
#include "../Weapon/HorrorAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Components/StaticMeshComponent.h"

APlayerCharacterB::APlayerCharacterB()
{
	PrimaryActorTick.bCanEverTick = true;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("head"));
	FirstPersonCamera->bUsePawnControlRotation = true;

	ConsoleComponent = CreateDefaultSubobject<UConsoleComponent>(TEXT("ConsoleComponent"));
	AbilitySystemComponent = CreateDefaultSubobject<UHorrorAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	GunMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMesh"));
	GunMeshComponent->SetupAttachment(GetMesh(), TEXT("hand_r"));
	GunMeshComponent->SetVisibility(false);  // 编辑器可见，游戏里默认隐藏

	bUseControllerRotationYaw = true;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
		MoveComp->MaxAcceleration = 2048.0f;
		MoveComp->bOrientRotationToMovement = false;
	}
}

UAbilitySystemComponent* APlayerCharacterB::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void APlayerCharacterB::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
	}
}

void APlayerCharacterB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Controller && !bMovementEnabled)
	{
		const FRotator CtrlRot = Controller->GetControlRotation();
		SetActorRotation(FRotator(0.0f, CtrlRot.Yaw, 0.0f));

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->Velocity = FVector::ZeroVector;
		}
	}

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
		if (IA_ToggleGun)
		{
			EnhancedInput->BindAction(IA_ToggleGun, ETriggerEvent::Started, this, &APlayerCharacterB::OnToggleGunInput);
		}
		if (IA_ToggleTalisman)
		{
			EnhancedInput->BindAction(IA_ToggleTalisman, ETriggerEvent::Started, this, &APlayerCharacterB::OnToggleTalismanInput);
		}

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->SetupWeaponInput(EnhancedInput);
		}
	}
}

void APlayerCharacterB::OnMove(const FInputActionValue& Value)
{
	if (MovementLockCount > 0) return;

	FVector2D MoveVector = Value.Get<FVector2D>();
	if (Controller)
	{
		FRotator CamRot = Controller->GetControlRotation();
		CamRot.Pitch = 0.0f;
		CamRot.Roll = 0.0f;
		const FVector CamForward = FRotationMatrix(CamRot).GetScaledAxis(EAxis::X);
		const FVector CamRight   = FRotationMatrix(CamRot).GetScaledAxis(EAxis::Y);

		AddMovementInput(CamForward, MoveVector.Y);
		AddMovementInput(CamRight,   MoveVector.X);
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

void APlayerCharacterB::OnToggleGunInput(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ToggleGun();
	}
}

void APlayerCharacterB::OnToggleTalismanInput(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ToggleTalisman();
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
			MoveComp->Velocity = FVector::ZeroVector;
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
