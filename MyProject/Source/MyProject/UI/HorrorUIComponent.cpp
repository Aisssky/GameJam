#include "HorrorUIComponent.h"
#include "../Core/HorrorGameInstance.h"
#include "../Player/HorrorPlayerController.h"
#include "../Player/PlayerCharacterB.h"
#include "../Console/ConsoleComponent.h"
#include "PauseMenuWidget.h"
#include "DialogueLineWidget.h"
#include "DialogueData.h"
#include "TaskPromptWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UHorrorUIComponent::UHorrorUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentLayer = ELayerState::STATE_MAIN_MENU;
	bSubStateBlocked = false;
}

void UHorrorUIComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedGameInstance = Cast<UHorrorGameInstance>(GetWorld()->GetGameInstance());
	if (CachedGameInstance)
	{
		CachedGameInstance->OnLayerChanged.AddDynamic(this, &UHorrorUIComponent::OnLayerChangedCallback);

		CurrentLayer = CachedGameInstance->CurrentLayerState;
		RefreshUIForCurrentState();
	}
}

void UHorrorUIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedGameInstance)
	{
		CachedGameInstance->OnLayerChanged.RemoveDynamic(this, &UHorrorUIComponent::OnLayerChangedCallback);
	}

	Super::EndPlay(EndPlayReason);
}

void UHorrorUIComponent::OnLayerChangedCallback(ELayerState FromLayer, ELayerState ToLayer)
{
	CurrentLayer = ToLayer;
	RefreshUIForCurrentState();
}

void UHorrorUIComponent::SetSubStateBlocked(bool bBlocked)
{
	bSubStateBlocked = bBlocked;
	RefreshUIForCurrentState();
}

void UHorrorUIComponent::RefreshUIForCurrentState()
{
	HideAll();

	switch (CurrentLayer)
	{
	case ELayerState::STATE_OUTER_GAMEPLAY:
		if (!bSubStateBlocked)
		{
			ShowHUD();
		}
		break;

	case ELayerState::STATE_CONSOLE_ACTIVE:
		ShowConsoleUI();
		break;

	case ELayerState::STATE_INNER_GAMEPLAY:
		ShowConsoleUI();
		ShowInnerGameplayHUD();
		break;

	case ELayerState::STATE_MAIN_MENU:
		ShowMainMenu();
		break;

	default:
		break;
	}
}

void UHorrorUIComponent::HideAll()
{
	HideHUD();
	HidePauseMenu();
	HideConsoleUI();
	HideInnerGameplayHUD();
	HideMainMenu();
	ClearDialogueLine();
	ClearTaskPrompt();
}

void UHorrorUIComponent::ShowHUD()
{
	if (!WBP_HUDClass) return;
	if (!HUDInstance)
	{
		HUDInstance = CreateWidget<UUserWidget>(GetWorld(), WBP_HUDClass);
	}
	if (HUDInstance && !HUDInstance->IsInViewport())
	{
		HUDInstance->AddToViewport();
	}
}

void UHorrorUIComponent::HideHUD()
{
	if (HUDInstance && HUDInstance->IsInViewport())
	{
		HUDInstance->RemoveFromParent();
	}
}

void UHorrorUIComponent::ShowPauseMenu()
{
	if (!WBP_PauseMenuClass) return;
	if (!PauseMenuInstance)
	{
		if (APlayerController* OwnerPC = Cast<APlayerController>(GetOwner()))
		{
			PauseMenuInstance = CreateWidget<UUserWidget>(OwnerPC, WBP_PauseMenuClass);
		}
		else
		{
			PauseMenuInstance = CreateWidget<UUserWidget>(GetWorld(), WBP_PauseMenuClass);
		}
	}
	if (PauseMenuInstance && !PauseMenuInstance->IsInViewport())
	{
		BindPauseMenuButtons();
		PauseMenuInstance->AddToViewport();
		PanelStack.AddUnique(PauseMenuInstance);
	}
}

void UHorrorUIComponent::HidePauseMenu()
{
	if (PauseMenuInstance && PauseMenuInstance->IsInViewport())
	{
		PauseMenuInstance->RemoveFromParent();
	}
}

void UHorrorUIComponent::ShowPauseMenuSavedMessage(float DisplaySeconds)
{
	SetPauseMenuStatusText(FText::FromString(TEXT("已保存")));

	if (UPauseMenuWidget* PauseMenu = Cast<UPauseMenuWidget>(PauseMenuInstance))
	{
		PauseMenu->ShowSavedMessage(DisplaySeconds);
	}
}

void UHorrorUIComponent::ShowPauseMenuSaveFailedMessage(const FText& Reason)
{
	SetPauseMenuStatusText(Reason);

	if (UPauseMenuWidget* PauseMenu = Cast<UPauseMenuWidget>(PauseMenuInstance))
	{
		PauseMenu->ShowSaveFailedMessage(Reason);
	}
}

void UHorrorUIComponent::HandlePauseMenuSaveClicked()
{
	if (AHorrorPlayerController* PC = Cast<AHorrorPlayerController>(GetOwner()))
	{
		PC->SaveGameFromPause();
	}
}

void UHorrorUIComponent::HandlePauseMenuReturnMainMenuClicked()
{
	if (AHorrorPlayerController* PC = Cast<AHorrorPlayerController>(GetOwner()))
	{
		PC->ReturnToMainMenuFromPause();
	}
}

void UHorrorUIComponent::HandlePauseMenuReturnGameClicked()
{
	if (AHorrorPlayerController* PC = Cast<AHorrorPlayerController>(GetOwner()))
	{
		PC->ResumeGameFromPause();
	}
}

void UHorrorUIComponent::BindPauseMenuButtons()
{
	if (!PauseMenuInstance)
	{
		return;
	}

	if (PauseMenuInstance->IsA<UPauseMenuWidget>())
	{
		return;
	}

	if (UButton* SaveButton = Cast<UButton>(PauseMenuInstance->GetWidgetFromName(TEXT("Button_SaveGame"))))
	{
		SaveButton->OnClicked.AddUniqueDynamic(this, &UHorrorUIComponent::HandlePauseMenuSaveClicked);
	}

	if (UButton* MainMenuButton = Cast<UButton>(PauseMenuInstance->GetWidgetFromName(TEXT("Button_ReturnMainMenu"))))
	{
		MainMenuButton->OnClicked.AddUniqueDynamic(this, &UHorrorUIComponent::HandlePauseMenuReturnMainMenuClicked);
	}

	if (UButton* ReturnGameButton = Cast<UButton>(PauseMenuInstance->GetWidgetFromName(TEXT("Button_ReturnGame"))))
	{
		ReturnGameButton->OnClicked.AddUniqueDynamic(this, &UHorrorUIComponent::HandlePauseMenuReturnGameClicked);
	}
}

void UHorrorUIComponent::SetPauseMenuStatusText(const FText& Message)
{
	if (!PauseMenuInstance)
	{
		return;
	}

	if (UTextBlock* StatusText = Cast<UTextBlock>(PauseMenuInstance->GetWidgetFromName(TEXT("Text_StatusMessage"))))
	{
		StatusText->SetText(Message);
	}

	if (UTextBlock* SaveStatusText = Cast<UTextBlock>(PauseMenuInstance->GetWidgetFromName(TEXT("Text_SaveStatus"))))
	{
		SaveStatusText->SetText(Message);
	}
}

void UHorrorUIComponent::ShowConsoleUI()
{
	if (AHorrorPlayerController* PC = Cast<AHorrorPlayerController>(GetOwner()))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			if (APlayerCharacterB* Char = Cast<APlayerCharacterB>(Pawn))
			{
				if (Char->ConsoleComponent && Char->ConsoleComponent->IsConsoleActorActive())
				{
					return;
				}
			}
		}
	}

	if (!WBP_ConsoleUIClass) return;
	if (!ConsoleUIInstance)
	{
		ConsoleUIInstance = CreateWidget<UUserWidget>(GetWorld(), WBP_ConsoleUIClass);
	}
	if (ConsoleUIInstance && !ConsoleUIInstance->IsInViewport())
	{
		ConsoleUIInstance->AddToViewport();
	}
}

void UHorrorUIComponent::HideConsoleUI()
{
	if (ConsoleUIInstance && ConsoleUIInstance->IsInViewport())
	{
		ConsoleUIInstance->RemoveFromParent();
	}
}

void UHorrorUIComponent::ShowInnerGameplayHUD()
{
	if (!WBP_InnerGameplayHUDClass) return;
	if (!InnerGameplayHUDInstance)
	{
		InnerGameplayHUDInstance = CreateWidget<UUserWidget>(GetWorld(), WBP_InnerGameplayHUDClass);
	}
	if (InnerGameplayHUDInstance && !InnerGameplayHUDInstance->IsInViewport())
	{
		InnerGameplayHUDInstance->AddToViewport();
	}
}

void UHorrorUIComponent::HideInnerGameplayHUD()
{
	if (InnerGameplayHUDInstance && InnerGameplayHUDInstance->IsInViewport())
	{
		InnerGameplayHUDInstance->RemoveFromParent();
	}
}

void UHorrorUIComponent::ShowMainMenu()
{
	if (!WBP_MainMenuClass) return;
	if (!MainMenuInstance)
	{
		MainMenuInstance = CreateWidget<UUserWidget>(GetWorld(), WBP_MainMenuClass);
	}
	if (MainMenuInstance && !MainMenuInstance->IsInViewport())
	{
		MainMenuInstance->AddToViewport();
	}
}

void UHorrorUIComponent::HideMainMenu()
{
	if (MainMenuInstance && MainMenuInstance->IsInViewport())
	{
		MainMenuInstance->RemoveFromParent();
	}
}

void UHorrorUIComponent::PushPanel(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass) return;

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
	if (Widget)
	{
		Widget->AddToViewport();
		PanelStack.Add(Widget);
	}
}

void UHorrorUIComponent::PopPanel()
{
	if (PanelStack.Num() == 0) return;

	UUserWidget* Top = PanelStack.Pop();
	if (Top)
	{
		Top->RemoveFromParent();
	}
}

UUserWidget* UHorrorUIComponent::GetTopPanel() const
{
	return PanelStack.Num() > 0 ? PanelStack.Last() : nullptr;
}

void UHorrorUIComponent::ShowDialogueLine(const FText& Text, float Duration, bool bLarge)
{
	if (!WBP_DialogueLineClass) return;
	if (!DialogueLineInstance)
	{
		DialogueLineInstance = CreateWidget<UUserWidget>(GetWorld(), WBP_DialogueLineClass);
	}
	if (DialogueLineInstance && !DialogueLineInstance->IsInViewport())
	{
		DialogueLineInstance->AddToViewport();
	}
	if (UDialogueLineWidget* DLW = Cast<UDialogueLineWidget>(DialogueLineInstance))
	{
		DLW->ShowText(Text, Duration, bLarge);
	}
}

void UHorrorUIComponent::ShowDialogueByID(FName RowID)
{
	if (!DialogueDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HorrorUIComponent] ShowDialogueByID: DialogueDataTable is null"));
		return;
	}

	const FDialogueRow* Row = DialogueDataTable->FindRow<FDialogueRow>(RowID, TEXT("ShowDialogueByID"));
	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HorrorUIComponent] ShowDialogueByID: Row '%s' not found"), *RowID.ToString());
		return;
	}

	ShowDialogueLine(Row->Text, Row->Duration, Row->bLargeFont);
}

void UHorrorUIComponent::ClearDialogueLine()
{
	if (UDialogueLineWidget* DLW = Cast<UDialogueLineWidget>(DialogueLineInstance))
	{
		DLW->ClearText();
	}
	if (DialogueLineInstance && DialogueLineInstance->IsInViewport())
	{
		DialogueLineInstance->RemoveFromParent();
	}
}

void UHorrorUIComponent::ShowTaskPrompt(const FText& TaskText, float Duration)
{
	if (!WBP_TaskPromptClass) return;
	if (!TaskPromptInstance)
	{
		TaskPromptInstance = CreateWidget<UUserWidget>(GetWorld(), WBP_TaskPromptClass);
	}
	if (TaskPromptInstance && !TaskPromptInstance->IsInViewport())
	{
		TaskPromptInstance->AddToViewport();
	}
	if (UTaskPromptWidget* TPW = Cast<UTaskPromptWidget>(TaskPromptInstance))
	{
		TPW->ShowTask(TaskText, Duration);
	}
}

void UHorrorUIComponent::ShowTaskPromptByID(FName RowID)
{
	if (!TaskPromptDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HorrorUIComponent] ShowTaskPromptByID: TaskPromptDataTable is null"));
		return;
	}

	const FTaskPromptRow* Row = TaskPromptDataTable->FindRow<FTaskPromptRow>(RowID, TEXT("ShowTaskPromptByID"));
	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("[HorrorUIComponent] ShowTaskPromptByID: Row '%s' not found"), *RowID.ToString());
		return;
	}

	ShowTaskPrompt(Row->Text, Row->Duration);
}

void UHorrorUIComponent::ClearTaskPrompt()
{
	if (UTaskPromptWidget* TPW = Cast<UTaskPromptWidget>(TaskPromptInstance))
	{
		TPW->ClearTask();
	}
	if (TaskPromptInstance && TaskPromptInstance->IsInViewport())
	{
		TaskPromptInstance->RemoveFromParent();
	}
}

