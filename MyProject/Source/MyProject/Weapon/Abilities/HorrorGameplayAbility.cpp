#include "HorrorGameplayAbility.h"
#include "../HorrorAbilitySystemComponent.h"
#include "../HorrorAttributeSet.h"
#include "../../Player/PlayerCharacterB.h"
#include "Camera/CameraComponent.h"

UHorrorGameplayAbility::UHorrorGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

UHorrorAbilitySystemComponent* UHorrorGameplayAbility::GetHorrorASC() const
{
	return Cast<UHorrorAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
}

UHorrorAttributeSet* UHorrorGameplayAbility::GetWeaponAttributes() const
{
	if (const UHorrorAbilitySystemComponent* ASC = GetHorrorASC())
	{
		return ASC->WeaponAttributes;
	}
	return nullptr;
}

APlayerCharacterB* UHorrorGameplayAbility::GetPlayerCharacter() const
{
	return Cast<APlayerCharacterB>(CurrentActorInfo->AvatarActor.Get());
}

UCameraComponent* UHorrorGameplayAbility::GetPlayerCamera() const
{
	if (const APlayerCharacterB* Character = GetPlayerCharacter())
	{
		return Character->FirstPersonCamera;
	}
	return nullptr;
}

void UHorrorGameplayAbility::ConsumeGunAmmo(int32 Amount)
{
	if (UHorrorAttributeSet* Attr = GetWeaponAttributes())
	{
		const float NewVal = FMath::Max(0.0f, Attr->GetGunAmmo() - static_cast<float>(Amount));
		Attr->SetGunAmmo(NewVal);
	}
}

void UHorrorGameplayAbility::ConsumeTalisman(int32 Amount)
{
	if (UHorrorAttributeSet* Attr = GetWeaponAttributes())
	{
		const float NewVal = FMath::Max(0.0f, Attr->GetTalismanCount() - static_cast<float>(Amount));
		Attr->SetTalismanCount(NewVal);
	}
}

bool UHorrorGameplayAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	return true;
}
