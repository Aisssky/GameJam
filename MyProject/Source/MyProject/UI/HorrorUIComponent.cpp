#include "HorrorUIComponent.h"
#include "../Core/HorrorGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

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
	case ELayerState::STATE_INNER_GAMEPLAY:
		ShowConsoleUI();
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
		PauseMenuInstance = CreateWidget<UUserWidget>(GetWorld(), WBP_PauseMenuClass);
	}
	if (PauseMenuInstance && !PauseMenuInstance->IsInViewport())
	{
		PauseMenuInstance->AddToViewport();
	}
}

void UHorrorUIComponent::HidePauseMenu()
{
	if (PauseMenuInstance && PauseMenuInstance->IsInViewport())
	{
		PauseMenuInstance->RemoveFromParent();
	}
}

void UHorrorUIComponent::ShowConsoleUI()
{
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
