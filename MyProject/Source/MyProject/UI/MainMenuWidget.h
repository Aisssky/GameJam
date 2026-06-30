#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Core/HorrorGameInstance.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UPanelWidget;
class USaveSlotButtonWidget;
class UConfirmDialogWidget;

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> Panel_Main;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> Panel_Slots;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> SaveSlotContainer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Archive;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Settings;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Quit;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Return;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Read;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Delete;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> OpDeniedText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Classes")
	TSubclassOf<USaveSlotButtonWidget> SaveSlotButtonClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Classes")
	TSubclassOf<UConfirmDialogWidget> CreateSaveDialogClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Classes")
	TSubclassOf<UConfirmDialogWidget> DeleteSaveDialogClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Classes")
	TSubclassOf<UUserWidget> SettingsMenuClass;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Main Menu|Visual")
	void BP_OnPanelSwitched(bool bShowingSlots);

	UFUNCTION(BlueprintImplementableEvent, Category = "Main Menu|Visual")
	void BP_OnOpDeniedVisual(const FText& Reason);

private:
	void SwitchToMainPanel();
	void SwitchToSlotsPanel();
	void LoadSaveSlots();
	void RefreshSaveSlots();
	void UpdateBottomButtons();
	void ClearSlotSelection();

	UFUNCTION() void OnArchiveClicked();
	UFUNCTION() void OnSettingsClicked();
	UFUNCTION() void OnQuitClicked();
	UFUNCTION() void OnReturnClicked();
	UFUNCTION() void OnReadClicked();
	UFUNCTION() void OnDeleteClicked();

	UFUNCTION() void OnOldSlotSelected(const FMetaHorrorSaveSlotInfo& SlotInfo);
	UFUNCTION() void OnPlayerSlotSelected(const FMetaHorrorSaveSlotInfo& SlotInfo);
	UFUNCTION() void OnEmptySaveSlotClicked();

	UFUNCTION() void OnSaveCreatedCallback(const FString& SlotName);
	UFUNCTION() void OnSaveDeletedCallback(const FString& SlotName);
	UFUNCTION() void OnOpDeniedCallback(FText Reason);

	void ShowDeniedMessage(const FText& Message);
	UFUNCTION() void HideDeniedMessage();

	UPROPERTY()
	TObjectPtr<UHorrorGameInstance> CachedGI;

	UPROPERTY()
	TObjectPtr<USaveSlotButtonWidget> OldSaveSlot;

	UPROPERTY()
	TObjectPtr<USaveSlotButtonWidget> PlayerSaveSlot;

	FMetaHorrorSaveSlotInfo SelectedSlotInfo;
	bool bHasSlotSelected = false;
	TArray<FMetaHorrorSaveSlotInfo> SlotInfos;
	FTimerHandle DeniedMessageTimerHandle;
};
