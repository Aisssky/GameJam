#pragma once

#include "CoreMinimal.h"
#include "HorrorGameplayAbility.h"
#include "GA_FireGun.generated.h"

class UAnimMontage;

UCLASS()
class MYPROJECT_API UGA_FireGun : public UHorrorGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_FireGun();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun")
	float FireRange = 10000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun")
	float StunDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun|Animation")
	TObjectPtr<UAnimMontage> FireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun|Effects")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun|Effects")
	TObjectPtr<class UNiagaraSystem> MuzzleFlashEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun|Effects")
	TObjectPtr<class UNiagaraSystem> ImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun|Effects")
	TObjectPtr<class UNiagaraSystem> TrailEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun|Effects")
	float TrailDuration = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun|Effects")
	TObjectPtr<class USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun|Effects")
	TSubclassOf<class UCameraShakeBase> FireCameraShake;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
};
