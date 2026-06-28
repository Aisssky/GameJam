#include "GA_PlaceTalisman.h"
#include "../HorrorGameplayTags.h"
#include "../HorrorAbilitySystemComponent.h"
#include "../HorrorAttributeSet.h"
#include "../BPI_Door.h"
#include "../../Core/HorrorGameInstance.h"
#include "../../Player/PlayerCharacterB.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

#define TAGS FHorrorGameplayTags::Get()

UGA_PlaceTalisman::UGA_PlaceTalisman()
{
	AbilityTags.AddTag(TAGS.Ability_PlaceTalisman);
	ActivationOwnedTags.AddTag(TAGS.Ability_PlaceTalisman);
	ActivationBlockedTags.AddTag(TAGS.State_InputBlocked_Console);
	ActivationBlockedTags.AddTag(TAGS.State_InputBlocked_Menu);
	ActivationBlockedTags.AddTag(TAGS.State_InputBlocked_Layer);
}

void UGA_PlaceTalisman::ActivateAbility(
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

	if (!ASC->HasMatchingGameplayTag(TAGS.Weapon_Talisman))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * TraceRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Cast<AActor>(GetPlayerCharacter()));

	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AActor* HitActor = Hit.GetActor();
	if (!HitActor || !HitActor->Implements<UBPI_Door>())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!IBPI_Door::Execute_CanPlaceTalisman(HitActor))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	ConsumeTalisman(1);
	IBPI_Door::Execute_PlaceTalisman(HitActor, BlockDuration);

	if (UHorrorGameInstance* GI = Cast<UHorrorGameInstance>(GetWorld()->GetGameInstance()))
	{
		GI->OnTalismanPlacedOnDoor.Broadcast(HitActor);
	}

	if (Attr->GetTalismanCount() <= 0.0f)
	{
		ASC->RemoveLooseGameplayTag(TAGS.Weapon_Talisman);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

bool UGA_PlaceTalisman::CanActivateAbility(
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
	if (!ASC || !ASC->HasMatchingGameplayTag(TAGS.Weapon_Talisman))
	{
		return false;
	}

	const UHorrorAttributeSet* Attr = ASC->WeaponAttributes;
	return Attr && Attr->GetTalismanCount() > 0.0f;
}
