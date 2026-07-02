#include "PauseMenuWidget.h"

#include "../Player/HorrorPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_SaveGame)
	{
		Button_SaveGame->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleSaveGameClicked);
	}

	if (Button_ReturnMainMenu)
	{
		Button_ReturnMainMenu->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleReturnMainMenuClicked);
	}

	if (Button_ReturnGame)
	{
		Button_ReturnGame->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleReturnGameClicked);
	}
}

void UPauseMenuWidget::NativeDestruct()
{
	if (Button_SaveGame)
	{
		Button_SaveGame->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::HandleSaveGameClicked);
	}

	if (Button_ReturnMainMenu)
	{
		Button_ReturnMainMenu->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::HandleReturnMainMenuClicked);
	}

	if (Button_ReturnGame)
	{
		Button_ReturnGame->OnClicked.RemoveDynamic(this, &UPauseMenuWidget::HandleReturnGameClicked);
	}

	Super::NativeDestruct();
}

void UPauseMenuWidget::ShowSavedMessage(float DisplaySeconds)
{
	SetStatusText(FText::FromString(TEXT("已保存")));
	BP_ShowSavedMessage(DisplaySeconds);
}

void UPauseMenuWidget::ShowSaveFailedMessage(const FText& Reason)
{
	SetStatusText(Reason);
	BP_ShowSaveFailedMessage(Reason);
}

void UPauseMenuWidget::ClearStatusMessage()
{
	SetStatusText(FText::GetEmpty());
	BP_ClearStatusMessage();
}

void UPauseMenuWidget::HandleSaveGameClicked()
{
	if (AHorrorPlayerController* PC = GetHorrorPlayerController())
	{
		PC->SaveGameFromPause();
	}
}

void UPauseMenuWidget::HandleReturnMainMenuClicked()
{
	if (AHorrorPlayerController* PC = GetHorrorPlayerController())
	{
		PC->ReturnToMainMenuFromPause();
	}
}

void UPauseMenuWidget::HandleReturnGameClicked()
{
	if (AHorrorPlayerController* PC = GetHorrorPlayerController())
	{
		PC->ResumeGameFromPause();
	}
}

AHorrorPlayerController* UPauseMenuWidget::GetHorrorPlayerController() const
{
	return Cast<AHorrorPlayerController>(GetOwningPlayer());
}

void UPauseMenuWidget::SetStatusText(const FText& Message)
{
	if (Text_StatusMessage)
	{
		Text_StatusMessage->SetText(Message);
	}

	if (Text_SaveStatus && Text_SaveStatus != Text_StatusMessage)
	{
		Text_SaveStatus->SetText(Message);
	}
}
