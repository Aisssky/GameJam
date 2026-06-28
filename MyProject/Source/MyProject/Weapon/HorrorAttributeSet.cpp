#include "HorrorAttributeSet.h"

UHorrorAttributeSet::UHorrorAttributeSet()
{
	InitGunAmmo(0.0f);
	InitMaxGunAmmo(6.0f);
	InitTalismanCount(0.0f);
	InitMaxTalismanCount(5.0f);
}

void UHorrorAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetGunAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxGunAmmo());
	}
	else if (Attribute == GetTalismanCountAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxTalismanCount());
	}
	else if (Attribute == GetMaxGunAmmoAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
	else if (Attribute == GetMaxTalismanCountAttribute())
	{
		NewValue = FMath::Max(0.0f, NewValue);
	}
}
