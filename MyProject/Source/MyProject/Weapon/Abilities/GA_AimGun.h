#pragma once

#include "CoreMinimal.h"
#include "HorrorGameplayAbility.h"
#include "GA_AimGun.generated.h"

UCLASS()
class MYPROJECT_API UGA_AimGun : public UHorrorGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_AimGun();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Aim")
	float AimFOV = 55.0f;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	float DefaultFOV = 90.0f;
};
