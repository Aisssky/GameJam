#include "MainMenuWidget.h"
#include "SaveSlotButtonWidget.h"
#include "ConfirmDialogWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CachedGI = Cast<UHorrorGameInstance>(GetGameInstance());
	if (!CachedGI)
	{
		return;
	}

	CachedGI->EnsureOriginalSaveExists();

	CachedGI->OnSaveCreated.AddDynamic(this, &UMainMenuWidget::OnSaveCreatedCallback);
	CachedGI->OnSaveDeleted.AddDynamic(this, &UMainMenuWidget::OnSaveDeletedCallback);
	CachedGI->OnSaveOperationDenied.AddDynamic(this, &UMainMenuWidget::OnOpDeniedCallback);

	if (Button_Archive)  Button_Archive->OnClicked.AddDynamic(this, &UMainMenuWidget::OnArchiveClicked);
	if (Button_Settings) Button_Settings->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
	if (Button_Quit)     Button_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	if (Button_Return)   Button_Return->OnClicked.AddDynamic(this, &UMainMenuWidget::OnReturnClicked);
	if (Button_Read)     Button_Read->OnClicked.AddDynamic(this, &UMainMenuWidget::OnReadClicked);
	if (Button_Delete)   Button_Delete->OnClicked.AddDynamic(this, &UMainMenuWidget::OnDeleteClicked);

	if (Panel_Main)        Panel_Main->SetVisibility(ESlateVisibility::Visible);
	if (Panel_Slots)       Panel_Slots->SetVisibility(ESlateVisibility::Collapsed);
	if (SaveSlotContainer) SaveSlotContainer->SetVisibility(ESlateVisibility::Collapsed);

	if (Button_Read)   Button_Read->SetVisibility(ESlateVisibility::Hidden);
	if (Button_Delete) Button_Delete->SetVisibility(ESlateVisibility::Hidden);
	if (OpDeniedText)  OpDeniedText->SetVisibility(ESlateVisibility::Hidden);

	LoadSaveSlots();
}

void UMainMenuWidget::NativeDestruct()
{
	if (CachedGI)
	{
		CachedGI->OnSaveCreated.RemoveDynamic(this, &UMainMenuWidget::OnSaveCreatedCallback);
		CachedGI->OnSaveDeleted.RemoveDynamic(this, &UMainMenuWidget::OnSaveDeletedCallback);
		CachedGI->OnSaveOperationDenied.RemoveDynamic(this, &UMainMenuWidget::OnOpDeniedCallback);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeniedMessageTimerHandle);
	}

	Super::NativeDestruct();
}

void UMainMenuWidget::SwitchToMainPanel()
{
	if (Panel_Main)        Panel_Main->SetVisibility(ESlateVisibility::Visible);
	if (Panel_Slots)       Panel_Slots->SetVisibility(ESlateVisibility::Collapsed);
	if (SaveSlotContainer) SaveSlotContainer->SetVisibility(ESlateVisibility::Collapsed);
	ClearSlotSelection();
	BP_OnPanelSwitched(false);
}

void UMainMenuWidget::SwitchToSlotsPanel()
{
	if (Panel_Main)        Panel_Main->SetVisibility(ESlateVisibility::Collapsed);
	if (Panel_Slots)       Panel_Slots->SetVisibility(ESlateVisibility::Visible);
	if (SaveSlotContainer) SaveSlotContainer->SetVisibility(ESlateVisibility::Visible);
	LoadSaveSlots();
	BP_OnPanelSwitched(true);
}

void UMainMenuWidget::LoadSaveSlots()
{
	if (!CachedGI || !SaveSlotContainer)
	{
		return;
	}

	SlotInfos = CachedGI->GetAllSaveSlotInfos();
	if (SlotInfos.Num() < 2)
	{
		return;
	}

	if (!OldSaveSlot && SaveSlotButtonClass)
	{
		OldSaveSlot = CreateWidget<USaveSlotButtonWidget>(GetWorld(), SaveSlotButtonClass);
		if (OldSaveSlot)
		{
			OldSaveSlot->SetupSlot(SlotInfos[0]);
			OldSaveSlot->OnSlotSelected.AddDynamic(this, &UMainMenuWidget::OnOldSlotSelected);
			OldSaveSlot->OnSlotDeselected.AddDynamic(this, &UMainMenuWidget::ClearSlotSelection);
			OldSaveSlot->OnEmptySlotClicked.AddDynamic(this, &UMainMenuWidget::OnEmptySaveSlotClicked);
			SaveSlotContainer->AddChild(OldSaveSlot);
			if (UVerticalBoxSlot* VBoxSlot = Cast<UVerticalBoxSlot>(OldSaveSlot->Slot))
			{
				VBoxSlot->SetPadding(FMargin(0.0f, 20.0f));
				VBoxSlot->SetHorizontalAlignment(HAlign_Fill);
			}
		}
	}

	if (!PlayerSaveSlot && SaveSlotButtonClass)
	{
		PlayerSaveSlot = CreateWidget<USaveSlotButtonWidget>(GetWorld(), SaveSlotButtonClass);
		if (PlayerSaveSlot)
		{
			PlayerSaveSlot->SetupSlot(SlotInfos[1]);
			PlayerSaveSlot->OnSlotSelected.AddDynamic(this, &UMainMenuWidget::OnPlayerSlotSelected);
			PlayerSaveSlot->OnSlotDeselected.AddDynamic(this, &UMainMenuWidget::ClearSlotSelection);
			PlayerSaveSlot->OnEmptySlotClicked.AddDynamic(this, &UMainMenuWidget::OnEmptySaveSlotClicked);
			SaveSlotContainer->AddChild(PlayerSaveSlot);
			if (UVerticalBoxSlot* VBoxSlot = Cast<UVerticalBoxSlot>(PlayerSaveSlot->Slot))
			{
				VBoxSlot->SetPadding(FMargin(0.0f, 20.0f));
				VBoxSlot->SetHorizontalAlignment(HAlign_Fill);
			}
		}
	}
}

void UMainMenuWidget::RefreshSaveSlots()
{
	if (!CachedGI)
	{
		return;
	}

	SlotInfos = CachedGI->GetAllSaveSlotInfos();
	if (SlotInfos.Num() < 2)
	{
		return;
	}

	if (OldSaveSlot)
	{
		OldSaveSlot->RefreshDisplay(SlotInfos[0]);
	}
	if (PlayerSaveSlot)
	{
		PlayerSaveSlot->RefreshDisplay(SlotInfos[1]);
	}

	ClearSlotSelection();
}

void UMainMenuWidget::UpdateBottomButtons()
{
	if (!bHasSlotSelected)
	{
		if (Button_Read)  Button_Read->SetVisibility(ESlateVisibility::Hidden);
		if (Button_Delete) Button_Delete->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	if (Button_Read)
	{
		Button_Read->SetVisibility(ESlateVisibility::Visible);
		Button_Read->SetIsEnabled(SelectedSlotInfo.bCanRead);
	}

	if (Button_Delete)
	{
		Button_Delete->SetVisibility(ESlateVisibility::Visible);
		Button_Delete->SetIsEnabled(SelectedSlotInfo.bCanDelete);
	}
}

void UMainMenuWidget::ClearSlotSelection()
{
	if (OldSaveSlot)    OldSaveSlot->SetSelected(false);
	if (PlayerSaveSlot) PlayerSaveSlot->SetSelected(false);

	bHasSlotSelected = false;

	if (Button_Read)  Button_Read->SetVisibility(ESlateVisibility::Hidden);
	if (Button_Delete) Button_Delete->SetVisibility(ESlateVisibility::Hidden);
}

void UMainMenuWidget::OnArchiveClicked()
{
	SwitchToSlotsPanel();
}

void UMainMenuWidget::OnSettingsClicked()
{
	if (SettingsMenuClass)
	{
		UUserWidget* SettingsMenu = CreateWidget<UUserWidget>(GetWorld(), SettingsMenuClass);
		if (SettingsMenu)
		{
			SettingsMenu->AddToViewport();
		}
	}
}

void UMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}

void UMainMenuWidget::OnReturnClicked()
{
	SwitchToMainPanel();
}

void UMainMenuWidget::OnReadClicked()
{
	if (!CachedGI) return;

	if (bHasSlotSelected && SelectedSlotInfo.bCanRead)
	{
		CachedGI->LoadGameFromManagedSlot(SelectedSlotInfo.SlotName);
	}
	else
	{
		ShowDeniedMessage(FText::FromString(TEXT("Cannot load this save")));
	}
}

void UMainMenuWidget::OnDeleteClicked()
{
	if (!CachedGI) return;

	if (bHasSlotSelected && SelectedSlotInfo.bCanDelete)
	{
		UConfirmDialogWidget* Dialog = CreateWidget<UConfirmDialogWidget>(GetWorld(), DeleteSaveDialogClass);
		if (Dialog)
		{
			TMap<FString, FString> Params;
			Params.Add(TEXT("TargetSlotName"), SelectedSlotInfo.SlotName);
			Dialog->SetupDialog(Params);
			Dialog->AddToViewport();
		}
	}
	else
	{
		ShowDeniedMessage(FText::FromString(TEXT("Cannot delete this save")));
	}
}

void UMainMenuWidget::OnOldSlotSelected(const FMetaHorrorSaveSlotInfo& SlotInfo)
{
	if (PlayerSaveSlot)
	{
		PlayerSaveSlot->SetSelected(false);
	}

	SelectedSlotInfo = SlotInfo;
	bHasSlotSelected = true;
	UpdateBottomButtons();
}

void UMainMenuWidget::OnPlayerSlotSelected(const FMetaHorrorSaveSlotInfo& SlotInfo)
{
	if (OldSaveSlot)
	{
		OldSaveSlot->SetSelected(false);
	}

	SelectedSlotInfo = SlotInfo;
	bHasSlotSelected = true;
	UpdateBottomButtons();
}

void UMainMenuWidget::OnEmptySaveSlotClicked()
{
	if (CreateSaveDialogClass)
	{
		UConfirmDialogWidget* Dialog = CreateWidget<UConfirmDialogWidget>(GetWorld(), CreateSaveDialogClass);
		if (Dialog)
		{
			TMap<FString, FString> Params;
			Dialog->SetupDialog(Params);
			Dialog->AddToViewport();
		}
	}
}

void UMainMenuWidget::OnSaveCreatedCallback(const FString& SlotName)
{
	RefreshSaveSlots();
}

void UMainMenuWidget::OnSaveDeletedCallback(const FString& SlotName)
{
	RefreshSaveSlots();
}

void UMainMenuWidget::OnOpDeniedCallback(FText Reason)
{
	ShowDeniedMessage(Reason);
	BP_OnOpDeniedVisual(Reason);
}

void UMainMenuWidget::ShowDeniedMessage(const FText& Message)
{
	if (!OpDeniedText) return;

	OpDeniedText->SetText(Message);
	OpDeniedText->SetVisibility(ESlateVisibility::Visible);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeniedMessageTimerHandle);
		World->GetTimerManager().SetTimer(
			DeniedMessageTimerHandle,
			this,
			&UMainMenuWidget::HideDeniedMessage,
			2.0f,
			false
		);
	}
}

void UMainMenuWidget::HideDeniedMessage()
{
	if (OpDeniedText)
	{
		OpDeniedText->SetVisibility(ESlateVisibility::Hidden);
	}
}
