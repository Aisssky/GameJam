#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HorrorAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class MYPROJECT_API UHorrorAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UHorrorAttributeSet();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	UPROPERTY(BlueprintReadOnly, Category = "Ammo")
	FGameplayAttributeData GunAmmo;
	ATTRIBUTE_ACCESSORS(UHorrorAttributeSet, GunAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo")
	FGameplayAttributeData MaxGunAmmo;
	ATTRIBUTE_ACCESSORS(UHorrorAttributeSet, MaxGunAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Talisman")
	FGameplayAttributeData TalismanCount;
	ATTRIBUTE_ACCESSORS(UHorrorAttributeSet, TalismanCount)

	UPROPERTY(BlueprintReadOnly, Category = "Talisman")
	FGameplayAttributeData MaxTalismanCount;
	ATTRIBUTE_ACCESSORS(UHorrorAttributeSet, MaxTalismanCount)
};
