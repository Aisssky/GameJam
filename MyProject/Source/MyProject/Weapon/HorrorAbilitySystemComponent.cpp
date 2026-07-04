#include "HorrorAbilitySystemComponent.h"
#include "HorrorAttributeSet.h"
#include "HorrorGameplayTags.h"
#include "../Core/HorrorGameInstance.h"
#include "../Core/HorrorTypes.h"
#include "../Player/PlayerCharacterB.h"
#include "AbilitySystemGlobals.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "Engine/World.h"

#define TAGS FHorrorGameplayTags::Get()

UHorrorAbilitySystemComponent::UHorrorAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bHasSavedWeapon = false;
}

void UHorrorAbilitySystemComponent::OnRegister()
{
	Super::OnRegister();

	WeaponAttributes = NewObject<UHorrorAttributeSet>(GetOwner(), TEXT("WeaponAttributes"));
	if (WeaponAttributes)
	{
		GetSpawnedAttributes_Mutable().AddUnique(WeaponAttributes);
	}
}

void UHorrorAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<APlayerCharacterB>(GetOwner());

	CachedGameInstance = Cast<UHorrorGameInstance>(GetWorld()->GetGameInstance());
	if (CachedGameInstance)
	{
		CachedGameInstance->OnConsoleToggled.AddDynamic(this, &UHorrorAbilitySystemComponent::OnConsoleToggledCallback);
		CachedGameInstance->OnLayerChanged.AddDynamic(this, &UHorrorAbilitySystemComponent::OnLayerChangedCallback);
	}

	InitAttributeDefaults();
	GrantDefaultAbilities();

	AddLooseGameplayTag(TAGS.Weapon_Unlocked_Gun);
	AddLooseGameplayTag(TAGS.Weapon_Unlocked_Talisman);

	if (!GunMesh && OwnerCharacter)
	{
		GunMesh = OwnerCharacter->GunMeshComponent;
	}

	if (GunMesh)
	{
		GunMesh->SetVisibility(false);
	}

	GunTagChangedHandle = RegisterGameplayTagEvent(TAGS.Weapon_Gun)
		.AddUObject(this, &UHorrorAbilitySystemComponent::OnGunTagChanged);
}

void UHorrorAbilitySystemComponent::OnGunTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (GunMesh)
	{
		GunMesh->SetVisibility(NewCount > 0);
	}
}

void UHorrorAbilitySystemComponent::ShowGun()
{
	AddLooseGameplayTag(TAGS.Weapon_Gun);
}

void UHorrorAbilitySystemComponent::HideGun()
{
	RemoveLooseGameplayTag(TAGS.Weapon_Gun);
}

void UHorrorAbilitySystemComponent::GrantDefaultAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (!AbilityClass) continue;

		FGameplayAbilitySpec Spec(AbilityClass, 0, -1, GetOwner());
		GiveAbility(Spec);
	}
}

void UHorrorAbilitySystemComponent::InitAttributeDefaults()
{
	if (WeaponAttributes)
	{
		WeaponAttributes->SetGunAmmo(static_cast<float>(InitialGunAmmo));
		WeaponAttributes->SetMaxGunAmmo(6.0f);
		WeaponAttributes->SetTalismanCount(static_cast<float>(InitialTalismanCount));
		WeaponAttributes->SetMaxTalismanCount(5.0f);
	}
}

void UHorrorAbilitySystemComponent::ToggleGun()
{
	if (HasMatchingGameplayTag(TAGS.Weapon_Gun))
	{
		RemoveLooseGameplayTag(TAGS.Weapon_Gun);
	}
	else if (HasMatchingGameplayTag(TAGS.Weapon_Unlocked_Gun) && GetGunAmmo() > 0.0f)
	{
		RemoveLooseGameplayTag(TAGS.Weapon_Talisman);
		AddLooseGameplayTag(TAGS.Weapon_Gun);
	}
}

void UHorrorAbilitySystemComponent::ToggleTalisman()
{
	if (HasMatchingGameplayTag(TAGS.Weapon_Talisman))
	{
		RemoveLooseGameplayTag(TAGS.Weapon_Talisman);
	}
	else if (HasMatchingGameplayTag(TAGS.Weapon_Unlocked_Talisman) && GetTalismanCount() > 0.0f)
	{
		RemoveLooseGameplayTag(TAGS.Weapon_Gun);
		AddLooseGameplayTag(TAGS.Weapon_Talisman);
	}
}

void UHorrorAbilitySystemComponent::SetWeaponInputBlocked(bool bBlocked, FGameplayTag BlockTag)
{
	if (bBlocked)
	{
		if (HasMatchingGameplayTag(TAGS.Weapon_Gun))
		{
			RemoveLooseGameplayTag(TAGS.Weapon_Gun);
		}
		if (HasMatchingGameplayTag(TAGS.Weapon_Talisman))
		{
			RemoveLooseGameplayTag(TAGS.Weapon_Talisman);
		}
		AddLooseGameplayTag(BlockTag);
	}
	else
	{
		RemoveLooseGameplayTag(BlockTag);
	}
}

int32 UHorrorAbilitySystemComponent::AddGunAmmo(int32 Amount)
{
	if (Amount <= 0 || !WeaponAttributes) return 0;

	const float Current = WeaponAttributes->GetGunAmmo();
	const float Max = WeaponAttributes->GetMaxGunAmmo();
	const int32 Room = FMath::Max(0, static_cast<int32>(Max - Current));
	const int32 Added = FMath::Min(Amount, Room);
	WeaponAttributes->SetGunAmmo(Current + Added);
	return Added;
}

int32 UHorrorAbilitySystemComponent::AddTalismans(int32 Amount)
{
	if (Amount <= 0 || !WeaponAttributes) return 0;

	const float Current = WeaponAttributes->GetTalismanCount();
	const float Max = WeaponAttributes->GetMaxTalismanCount();
	const int32 Room = FMath::Max(0, static_cast<int32>(Max - Current));
	const int32 Added = FMath::Min(Amount, Room);
	WeaponAttributes->SetTalismanCount(Current + Added);
	return Added;
}

float UHorrorAbilitySystemComponent::GetGunAmmo() const
{
	return WeaponAttributes ? WeaponAttributes->GetGunAmmo() : 0.0f;
}

float UHorrorAbilitySystemComponent::GetMaxGunAmmo() const
{
	return WeaponAttributes ? WeaponAttributes->GetMaxGunAmmo() : 6.0f;
}

float UHorrorAbilitySystemComponent::GetTalismanCount() const
{
	return WeaponAttributes ? WeaponAttributes->GetTalismanCount() : 0.0f;
}

float UHorrorAbilitySystemComponent::GetMaxTalismanCount() const
{
	return WeaponAttributes ? WeaponAttributes->GetMaxTalismanCount() : 5.0f;
}

bool UHorrorAbilitySystemComponent::IsHoldingWeapon(FGameplayTag WeaponTag) const
{
	return HasMatchingGameplayTag(WeaponTag);
}

void UHorrorAbilitySystemComponent::SetMenuBlocked(bool bBlocked)
{
	SetWeaponInputBlocked(bBlocked, TAGS.State_InputBlocked_Menu);
}

void UHorrorAbilitySystemComponent::SetLayerBlocked(bool bBlocked)
{
	SetWeaponInputBlocked(bBlocked, TAGS.State_InputBlocked_Layer);
}

void UHorrorAbilitySystemComponent::SetupWeaponInput(UEnhancedInputComponent* EnhancedInput)
{
	if (!EnhancedInput) return;

	if (IA_FireGun)
	{
		EnhancedInput->BindAction(IA_FireGun, ETriggerEvent::Started, this, &UHorrorAbilitySystemComponent::OnFireInput);
	}
	if (IA_AimGun)
	{
		EnhancedInput->BindAction(IA_AimGun, ETriggerEvent::Started, this, &UHorrorAbilitySystemComponent::OnAimStartInput);
	}
	if (IA_PlaceTalisman)
	{
		EnhancedInput->BindAction(IA_PlaceTalisman, ETriggerEvent::Triggered, this, &UHorrorAbilitySystemComponent::OnPlaceTalismanInput);
	}
}

void UHorrorAbilitySystemComponent::BeginDestroy()
{
	if (CachedGameInstance)
	{
		CachedGameInstance->OnConsoleToggled.RemoveDynamic(this, &UHorrorAbilitySystemComponent::OnConsoleToggledCallback);
		CachedGameInstance->OnLayerChanged.RemoveDynamic(this, &UHorrorAbilitySystemComponent::OnLayerChangedCallback);
	}

	if (GunTagChangedHandle.IsValid())
	{
		RegisterGameplayTagEvent(TAGS.Weapon_Gun).Remove(GunTagChangedHandle);
		GunTagChangedHandle.Reset();
	}

	Super::BeginDestroy();
}

void UHorrorAbilitySystemComponent::OnConsoleToggledCallback(bool bConsoleActive)
{
	if (bConsoleActive)
	{
		bHasSavedWeapon = false;
		if (HasMatchingGameplayTag(TAGS.Weapon_Gun))
		{
			SavedWeaponBeforeConsole = TAGS.Weapon_Gun;
			bHasSavedWeapon = true;
		}
		else if (HasMatchingGameplayTag(TAGS.Weapon_Talisman))
		{
			SavedWeaponBeforeConsole = TAGS.Weapon_Talisman;
			bHasSavedWeapon = true;
		}

		RemoveLooseGameplayTag(TAGS.Weapon_Gun);
		RemoveLooseGameplayTag(TAGS.Weapon_Talisman);

		AddLooseGameplayTag(TAGS.State_InputBlocked_Console);
	}
	else
	{
		RemoveLooseGameplayTag(TAGS.State_InputBlocked_Console);

		if (bHasSavedWeapon && SavedWeaponBeforeConsole.IsValid())
		{
			AddLooseGameplayTag(SavedWeaponBeforeConsole);
		}
		bHasSavedWeapon = false;
	}
}

void UHorrorAbilitySystemComponent::OnLayerChangedCallback(ELayerState FromLayer, ELayerState ToLayer)
{
	if (ToLayer == ELayerState::STATE_OUTER_GAMEPLAY)
	{
		SetLayerBlocked(false);
	}
	else
	{
		SetLayerBlocked(true);
	}
}

void UHorrorAbilitySystemComponent::OnFireInput(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAGS.Weapon_Gun))
	{
		FGameplayTagContainer FireTag(TAGS.Ability_FireGun);
		TryActivateAbilitiesByTag(FireTag);
	}
}

void UHorrorAbilitySystemComponent::OnAimStartInput(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAGS.Weapon_Aiming))
	{
		FGameplayTagContainer AimAssetTag(TAGS.Ability_Aim);
		CancelAbilities(&AimAssetTag);
	}
	else if (HasMatchingGameplayTag(TAGS.Weapon_Gun))
	{
		FGameplayTagContainer AimTag(TAGS.Ability_Aim);
		TryActivateAbilitiesByTag(AimTag);
	}
}

void UHorrorAbilitySystemComponent::OnAimEndInput(const FInputActionValue& Value)
{
}

void UHorrorAbilitySystemComponent::OnPlaceTalismanInput(const FInputActionValue& Value)
{
	if (HasMatchingGameplayTag(TAGS.Weapon_Talisman))
	{
		FGameplayTagContainer PlaceTag(TAGS.Ability_PlaceTalisman);
		TryActivateAbilitiesByTag(PlaceTag);
	}
}
