#include "GA_FireGun.h"
#include "../HorrorGameplayTags.h"
#include "../HorrorAbilitySystemComponent.h"
#include "../HorrorAttributeSet.h"
#include "../../Core/HorrorGameInstance.h"
#include "../../Player/PlayerCharacterB.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

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

	APlayerCharacterB* Character = GetPlayerCharacter();

	FVector MuzzleLocation = Camera->GetComponentLocation();
	FRotator MuzzleRotation = Camera->GetComponentRotation();
	if (Character && Character->GunMeshComponent)
	{
		MuzzleLocation = Character->GunMeshComponent->GetSocketLocation(MuzzleSocketName);
		MuzzleRotation = Character->GunMeshComponent->GetSocketRotation(MuzzleSocketName);
	}

	if (ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* Bullet = GetWorld()->SpawnActor<AActor>(
			ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);

		if (Bullet)
		{
			if (UProjectileMovementComponent* ProjMove = Bullet->FindComponentByClass<UProjectileMovementComponent>())
			{
				ProjMove->SetUpdatedComponent(Bullet->GetRootComponent());
				ProjMove->InitialSpeed = MuzzleVelocity;
				ProjMove->MaxSpeed = 0.0f;
				ProjMove->bRotationFollowsVelocity = true;
				ProjMove->Velocity = MuzzleRotation.Vector() * MuzzleVelocity;
			}
		}
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, MuzzleLocation);
	}

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * FireRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Cast<AActor>(Character));

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
