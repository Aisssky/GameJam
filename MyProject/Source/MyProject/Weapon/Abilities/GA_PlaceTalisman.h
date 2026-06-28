#pragma once

#include "CoreMinimal.h"
#include "HorrorGameplayAbility.h"
#include "GA_PlaceTalisman.generated.h"

UCLASS()
class MYPROJECT_API UGA_PlaceTalisman : public UHorrorGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PlaceTalisman();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talisman")
	float TraceRange = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Talisman")
	float BlockDuration = 30.0f;

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
