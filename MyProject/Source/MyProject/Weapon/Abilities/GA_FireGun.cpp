#include "GA_FireGun.h"
#include "../HorrorGameplayTags.h"
#include "../HorrorAbilitySystemComponent.h"
#include "../HorrorAttributeSet.h"
#include "../../Core/HorrorGameInstance.h"
#include "../../Player/PlayerCharacterB.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

#define TAGS FHorrorGameplayTags::Get()

UGA_FireGun::UGA_FireGun()
{
	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(TAGS.Ability_FireGun);
	SetAssetTags(AssetTags);
	ActivationOwnedTags.AddTag(TAGS.Ability_FireGun);
	ActivationBlockedTags.AddTag(TAGS.State_InputBlocked_Console);
	ActivationBlockedTags.AddTag(TAGS.State_InputBlocked_Menu);
	ActivationBlockedTags.AddTag(TAGS.State_InputBlocked_Layer);
}

void UGA_FireGun::ActivateAbility(
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

	UHorrorAbilitySystemComponent* ASC = GetHorrorASC();
	UHorrorAttributeSet* Attr = GetWeaponAttributes();
	UCameraComponent* Camera = GetPlayerCamera();

	if (!ASC || !Attr || !Camera)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!ASC->HasMatchingGameplayTag(TAGS.Weapon_Gun))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ConsumeGunAmmo(1);

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * FireRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Cast<AActor>(GetPlayerCharacter()));

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			if (UHorrorGameInstance* GI = Cast<UHorrorGameInstance>(GetWorld()->GetGameInstance()))
			{
				GI->OnEnemyShot.Broadcast(HitActor, StunDuration);
			}
		}
	}

	if (Attr->GetGunAmmo() <= 0.0f)
	{
		ASC->RemoveLooseGameplayTag(TAGS.Weapon_Gun);
		ASC->DestroyGunActor();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

bool UGA_FireGun::CanActivateAbility(
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

	const UHorrorAbilitySystemComponent* ASC = Cast<UHorrorAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (!ASC || !ASC->HasMatchingGameplayTag(TAGS.Weapon_Gun))
	{
		return false;
	}

	const UHorrorAttributeSet* Attr = ASC->WeaponAttributes;
	return Attr && Attr->GetGunAmmo() > 0.0f;
}
