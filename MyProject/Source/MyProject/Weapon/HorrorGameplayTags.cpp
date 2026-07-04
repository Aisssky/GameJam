#include "HorrorGameplayTags.h"
#include "GameplayTagsManager.h"

const FHorrorGameplayTags& FHorrorGameplayTags::Get()
{
	static FHorrorGameplayTags Instance;
	return Instance;
}

FHorrorGameplayTags::FHorrorGameplayTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	Weapon_Gun = Manager.AddNativeGameplayTag(TEXT("Weapon.Gun"));
	Weapon_Talisman = Manager.AddNativeGameplayTag(TEXT("Weapon.Talisman"));
	Weapon_Unlocked_Gun = Manager.AddNativeGameplayTag(TEXT("Weapon.Unlocked.Gun"));
	Weapon_Unlocked_Talisman = Manager.AddNativeGameplayTag(TEXT("Weapon.Unlocked.Talisman"));
	Weapon_Aiming = Manager.AddNativeGameplayTag(TEXT("Weapon.Aiming"));

	State_InputBlocked_Console = Manager.AddNativeGameplayTag(TEXT("State.InputBlocked.Console"));
	State_InputBlocked_Menu = Manager.AddNativeGameplayTag(TEXT("State.InputBlocked.Menu"));
	State_InputBlocked_Layer = Manager.AddNativeGameplayTag(TEXT("State.InputBlocked.Layer"));

	Ability_FireGun = Manager.AddNativeGameplayTag(TEXT("Ability.FireGun"));
	Ability_Aim = Manager.AddNativeGameplayTag(TEXT("Ability.Aim"));
	Ability_PlaceTalisman = Manager.AddNativeGameplayTag(TEXT("Ability.PlaceTalisman"));
}
