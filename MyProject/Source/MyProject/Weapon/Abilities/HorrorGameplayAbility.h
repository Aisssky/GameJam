#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HorrorGameplayAbility.generated.h"

class APlayerCharacterB;
class UHorrorAbilitySystemComponent;
class UHorrorAttributeSet;
class UCameraComponent;

UCLASS(Abstract)
class MYPROJECT_API UHorrorGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHorrorGameplayAbility();

	UFUNCTION(BlueprintPure, Category = "Horror|Ability")
	UHorrorAbilitySystemComponent* GetHorrorASC() const;

	UFUNCTION(BlueprintPure, Category = "Horror|Ability")
	UHorrorAttributeSet* GetWeaponAttributes() const;

	UFUNCTION(BlueprintPure, Category = "Horror|Ability")
	APlayerCharacterB* GetPlayerCharacter() const;

	UFUNCTION(BlueprintPure, Category = "Horror|Ability")
	UCameraComponent* GetPlayerCamera() const;

	UFUNCTION(BlueprintCallable, Category = "Horror|Ability")
	void ConsumeGunAmmo(int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Horror|Ability")
	void ConsumeTalisman(int32 Amount = 1);

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
};
