#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Core/HorrorGameInstance.h"
#include "SaveSlotButtonWidget.generated.h"

class UButton;
class UBorder;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotSelectedSig, const FMetaHorrorSaveSlotInfo&, SlotInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSlotDeselectedSig);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmptySlotClickedSig);

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API USaveSlotButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Save Slot")
	void SetupSlot(const FMetaHorrorSaveSlotInfo& InSlotInfo);

	UFUNCTION(BlueprintCallable, Category = "Save Slot")
	void SetSelected(bool bInSelected);

	UFUNCTION(BlueprintCallable, Category = "Save Slot")
	void RefreshDisplay(const FMetaHorrorSaveSlotInfo& NewSlotInfo);

	UFUNCTION(BlueprintPure, Category = "Save Slot")
	bool IsSelected() const { return bSelected; }

	UFUNCTION(BlueprintPure, Category = "Save Slot")
	const FMetaHorrorSaveSlotInfo& GetSlotInfo() const { return SlotInfo; }

	UPROPERTY(BlueprintAssignable, Category = "Save Slot|Events")
	FOnSlotSelectedSig OnSlotSelected;

	UPROPERTY(BlueprintAssignable, Category = "Save Slot|Events")
	FOnSlotDeselectedSig OnSlotDeselected;

	UPROPERTY(BlueprintAssignable, Category = "Save Slot|Events")
	FOnEmptySlotClickedSig OnEmptySlotClicked;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UBorder> BorderRef;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(EditDefaultsOnly, Category = "Save Slot|Appearance")
	FLinearColor NormalColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Save Slot|Appearance")
	FLinearColor HighlightedColor = FLinearColor(0.1f, 0.15f, 0.4f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Save Slot|Appearance")
	FLinearColor LockedColor = FLinearColor(0.08f, 0.08f, 0.08f, 1.0f);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Save Slot")
	void BP_OnSelectionChanged(bool bIsSelected);

	UFUNCTION(BlueprintImplementableEvent, Category = "Save Slot")
	void BP_OnSlotInfoUpdated(const FMetaHorrorSaveSlotInfo& NewInfo);

private:
	UFUNCTION()
	void OnSlotClicked();

	void UpdateDisplayFromSlotInfo();

	bool bSelected = false;
	FMetaHorrorSaveSlotInfo SlotInfo;
};
