#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Components/MeshComponent.h"
#include "HorrorAbilitySystemComponent.generated.h"

class UHorrorAttributeSet;
class UHorrorGameInstance;
class UInputAction;
struct FInputActionValue;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UHorrorAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UHorrorAbilitySystemComponent();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Gun")
	int32 InitialGunAmmo = 6;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Talisman")
	int32 InitialTalismanCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Gun")
	TObjectPtr<UMeshComponent> GunMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Gun|Sound")
	TObjectPtr<class USoundBase> GunEquipSound;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ShowGun();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void HideGun();

	// Weapon.Gun Tag 变化回调 → 同步枪模型显隐 + ABP 动画状态机 bool
	void OnGunTagChanged(const FGameplayTag Tag, int32 NewCount);

	void GrantDefaultAbilities();
	void InitAttributeDefaults();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ToggleGun();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ToggleTalisman();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWeaponInputBlocked(bool bBlocked, FGameplayTag BlockTag);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	int32 AddGunAmmo(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	int32 AddTalismans(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetGunAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetMaxGunAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetTalismanCount() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetMaxTalismanCount() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsHoldingWeapon(FGameplayTag WeaponTag) const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetMenuBlocked(bool bBlocked);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetLayerBlocked(bool bBlocked);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_FireGun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_AimGun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_PlaceTalisman;

	void SetupWeaponInput(class UEnhancedInputComponent* EnhancedInput);

	UPROPERTY()
	TObjectPtr<UHorrorAttributeSet> WeaponAttributes;

	UPROPERTY()
	TObjectPtr<class APlayerCharacterB> OwnerCharacter;

	FDelegateHandle OnAmmoChangedHandle;
	FDelegateHandle OnTalismanCountChangedHandle;

	virtual void OnRegister() override;
	virtual void BeginDestroy() override;

protected:
	UFUNCTION()
	void OnConsoleToggledCallback(bool bConsoleActive);

	UFUNCTION()
	void OnLayerChangedCallback(ELayerState FromLayer, ELayerState ToLayer);

	void OnFireInput(const FInputActionValue& Value);
	void OnAimStartInput(const FInputActionValue& Value);
	void OnAimEndInput(const FInputActionValue& Value);
	void OnPlaceTalismanInput(const FInputActionValue& Value);

private:
	UPROPERTY()
	TObjectPtr<UHorrorGameInstance> CachedGameInstance;

	FGameplayTag SavedWeaponBeforeConsole;
	bool bHasSavedWeapon;

	FDelegateHandle GunTagChangedHandle;
};
