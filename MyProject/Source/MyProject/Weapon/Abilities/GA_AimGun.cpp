#include "GA_AimGun.h"
#include "../HorrorGameplayTags.h"
#include "../HorrorAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"

#define TAGS FHorrorGameplayTags::Get()

UGA_AimGun::UGA_AimGun()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAGS.Ability_Aim);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(TAGS.Weapon_Aiming);
	ActivationBlockedTags.AddTag(TAGS.State_InputBlocked_Console);
	ActivationBlockedTags.AddTag(TAGS.State_InputBlocked_Menu);
	ActivationBlockedTags.AddTag(TAGS.State_InputBlocked_Layer);

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_AimGun::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UCameraComponent* Camera = GetPlayerCamera())
	{
		DefaultFOV = Camera->FieldOfView;
		Camera->SetFieldOfView(AimFOV);
	}
}

void UGA_AimGun::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UCameraComponent* Camera = GetPlayerCamera())
	{
		Camera->SetFieldOfView(DefaultFOV);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
