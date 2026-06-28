#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "../Console/ConsoleComponent.h"
#include "PlayerCharacterB.generated.h"

class UCameraComponent;
class UCharacterMovementComponent;
class UHorrorAbilitySystemComponent;
class UInputAction;

UCLASS()
class MYPROJECT_API APlayerCharacterB : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APlayerCharacterB();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MouseSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float FatigueSpeedMultiplier = 0.6f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	int32 MovementLockCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bMovementEnabled = true;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DisableMovement();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EnableMovement();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetSpeedMultiplier(float Multiplier);

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsMovementEnabled() const { return MovementLockCount == 0; }

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ApplyFatigueEffect(float SoulFragmentationLevel);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Console")
	TObjectPtr<UConsoleComponent> ConsoleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UHorrorAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ToggleGun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ToggleTalisman;

protected:
	void OnMove(const struct FInputActionValue& Value);
	void OnLook(const struct FInputActionValue& Value);
	void OnToggleGunInput(const struct FInputActionValue& Value);
	void OnToggleTalismanInput(const struct FInputActionValue& Value);
};
