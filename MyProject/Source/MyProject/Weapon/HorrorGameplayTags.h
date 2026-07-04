#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct MYPROJECT_API FHorrorGameplayTags
{
	static const FHorrorGameplayTags& Get();

	FGameplayTag Weapon_Gun;
	FGameplayTag Weapon_Talisman;
	FGameplayTag Weapon_Unlocked_Gun;       // 平级，非 Weapon.Gun 子 Tag
	FGameplayTag Weapon_Unlocked_Talisman;  // 平级，非 Weapon.Talisman 子 Tag
	FGameplayTag Weapon_Aiming;

	FGameplayTag State_InputBlocked_Console;
	FGameplayTag State_InputBlocked_Menu;
	FGameplayTag State_InputBlocked_Layer;

	FGameplayTag Ability_FireGun;
	FGameplayTag Ability_Aim;
	FGameplayTag Ability_PlaceTalisman;

private:
	FHorrorGameplayTags();
};
