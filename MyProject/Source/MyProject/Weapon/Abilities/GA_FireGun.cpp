#include "GA_FireGun.h"
#include "../HorrorGameplayTags.h"
#include "../HorrorAbilitySystemComponent.h"
#include "../HorrorAttributeSet.h"
#include "../../Core/HorrorGameInstance.h"
#include "../../Player/PlayerCharacterB.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"

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

	if (FireMontage)
	{
		if (UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance())
		{
			AnimInstance->Montage_Play(FireMontage);
		}
	}

	FVector MuzzleLocation = Camera->GetComponentLocation();
	APlayerCharacterB* Character = GetPlayerCharacter();
	if (Character && Character->GunMeshComponent)
	{
		MuzzleLocation = Character->GunMeshComponent->GetSocketLocation(MuzzleSocketName);
	}

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * FireRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Cast<AActor>(Character));

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	const FVector ImpactPoint = bHit ? Hit.ImpactPoint : End;

	if (MuzzleFlashEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), MuzzleFlashEffect, MuzzleLocation,
			Camera->GetComponentRotation());
	}

	if (TrailEffect)
	{
		UNiagaraComponent* TrailComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), TrailEffect, MuzzleLocation);
		if (TrailComp)
		{
			TrailComp->SetVectorParameter(TEXT("BeamEnd"), ImpactPoint);
			TrailComp->SetFloatParameter(TEXT("BeamLifeTime"), TrailDuration);
		}
	}
	if (bHit)
	{
		if (ImpactEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), ImpactEffect, ImpactPoint,
				Hit.ImpactNormal.Rotation());
		}

		if (AActor* HitActor = Hit.GetActor())
		{
			if (UHorrorGameInstance* GI = Cast<UHorrorGameInstance>(GetWorld()->GetGameInstance()))
			{
				GI->OnEnemyShot.Broadcast(HitActor, StunDuration);
			}
		}
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, MuzzleLocation);
	}

	if (FireCameraShake && Character)
	{
		if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			PC->ClientStartCameraShake(FireCameraShake);
		}
	}

	if (Attr->GetGunAmmo() <= 0.0f)
	{
		ASC->RemoveLooseGameplayTag(TAGS.Weapon_Gun);
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
