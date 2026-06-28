#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct MYPROJECT_API FHorrorGameplayTags
{
	static const FHorrorGameplayTags& Get();

	FGameplayTag Weapon_Gun;
	FGameplayTag Weapon_Talisman;
	FGameplayTag Weapon_Gun_Unlocked;
	FGameplayTag Weapon_Talisman_Unlocked;
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
