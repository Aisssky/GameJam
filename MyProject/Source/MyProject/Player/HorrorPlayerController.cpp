#include "HorrorPlayerController.h"
#include "PlayerCharacterB.h"
#include "../Core/HorrorGameInstance.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"

AHorrorPlayerController::AHorrorPlayerController()
{
	CurrentSubState = EGameSubState::SUBSTATE_NORMAL;
}

void AHorrorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	CachedGameInstance = Cast<UHorrorGameInstance>(GetGameInstance());
	if (CachedGameInstance)
	{
		CachedGameInstance->OnLayerChanged.AddDynamic(this, &AHorrorPlayerController::OnLayerStateChanged);

		const ELayerState CurrentLayer = CachedGameInstance->CurrentLayerState;
		OnLayerStateChanged(CurrentLayer, CurrentLayer);
	}

	CachedPlayerCharacter = Cast<APlayerCharacterB>(GetPawn());
}

void AHorrorPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedGameInstance)
	{
		CachedGameInstance->OnLayerChanged.RemoveDynamic(this, &AHorrorPlayerController::OnLayerStateChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void AHorrorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (IA_ToggleConsole)
		{
			EnhancedInput->BindAction(IA_ToggleConsole, ETriggerEvent::Triggered, this, &AHorrorPlayerController::OnToggleConsole);
		}
		if (IA_Menu)
		{
			EnhancedInput->BindAction(IA_Menu, ETriggerEvent::Triggered, this, &AHorrorPlayerController::OnMenuPressed);
		}
	}
}

void AHorrorPlayerController::OnLayerStateChanged(ELayerState From, ELayerState To)
{
	ClearAllInputContexts();

	HideHUD();
	HideConsoleUI();

	switch (To)
	{
	case ELayerState::STATE_MAIN_MENU:
		PushInputContext(IMC_Menu, 1);
		SetShowMouseCursor(true);
		SetInputMode(FInputModeUIOnly());
		break;

	case ELayerState::STATE_OUTER_GAMEPLAY:
		PushInputContext(IMC_Gameplay, 3);
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
		EnablePlayerMovement();
		if (CurrentSubState == EGameSubState::SUBSTATE_NORMAL)
		{
			ShowHUD();
		}
		break;

	case ELayerState::STATE_CONSOLE_ACTIVE:
		PushInputContext(IMC_ConsoleUI, 2);
		DisablePlayerMovement();
		SetShowMouseCursor(true);
		SetInputMode(FInputModeGameAndUI());
		ShowConsoleUI();
		break;

	case ELayerState::STATE_INNER_GAMEPLAY:
		PushInputContext(IMC_ConsoleUI, 2);
		DisablePlayerMovement();
		ShowConsoleUI();
		break;

	case ELayerState::STATE_CINEMATIC:
		PushInputContext(IMC_Cinematic, 0);
		SetShowMouseCursor(false);
		break;
	}
}

void AHorrorPlayerController::PushInputContext(UInputMappingContext* Context, int32 Priority)
{
	if (!Context) return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(Context, Priority);
	}
}

void AHorrorPlayerController::ClearAllInputContexts()
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
	}
}

bool AHorrorPlayerController::SetSubState(EGameSubState NewState)
{
	if (CurrentSubState == NewState) return true;

	EGameSubState Old = CurrentSubState;

	switch (NewState)
	{
	case EGameSubState::SUBSTATE_NORMAL:
		CurrentSubState = NewState;
		break;

	case EGameSubState::SUBSTATE_MENU:
		if (Old != EGameSubState::SUBSTATE_NORMAL) return false;
		EnterMenuState();
		CurrentSubState = NewState;
		break;

	case EGameSubState::SUBSTATE_INTERACT:
		if (Old != EGameSubState::SUBSTATE_NORMAL) return false;
		DisablePlayerMovement();
		CurrentSubState = NewState;
		break;

	case EGameSubState::SUBSTATE_STUNNED:
		DisablePlayerMovement();
		CurrentSubState = NewState;
		break;
	}

	return true;
}

void AHorrorPlayerController::EnterMenuState()
{
	SetSubState(EGameSubState::SUBSTATE_MENU);
	PushInputContext(IMC_Menu, 1);
	GetWorldSettings()->SetTimeDilation(0.0f);
	SetShowMouseCursor(true);
	HideHUD();
	ShowPauseMenu();
}

void AHorrorPlayerController::ExitMenuState()
{
	HidePauseMenu();
	PopPanel();
	ClearAllInputContexts();
	PushInputContext(IMC_Gameplay, 3);
	GetWorldSettings()->SetTimeDilation(1.0f);
	SetShowMouseCursor(false);
	SetSubState(EGameSubState::SUBSTATE_NORMAL);
	ShowHUD();
}

void AHorrorPlayerController::OnToggleConsole(const FInputActionValue& Value)
{
	if (!CachedGameInstance)
	{
		CachedGameInstance = Cast<UHorrorGameInstance>(GetGameInstance());
		if (CachedGameInstance)
		{
			CachedGameInstance->OnLayerChanged.RemoveDynamic(this, &AHorrorPlayerController::OnLayerStateChanged);
			CachedGameInstance->OnLayerChanged.AddDynamic(this, &AHorrorPlayerController::OnLayerStateChanged);
		}
		if (!CachedGameInstance) return;
	}

	ELayerState Current = CachedGameInstance->CurrentLayerState;

	if (!CachedPlayerCharacter)
	{
		CachedPlayerCharacter = Cast<APlayerCharacterB>(GetPawn());
	}
	if (!CachedPlayerCharacter || !CachedPlayerCharacter->ConsoleComponent) return;

	if (Current == ELayerState::STATE_OUTER_GAMEPLAY && CurrentSubState == EGameSubState::SUBSTATE_NORMAL)
	{
		CachedPlayerCharacter->ConsoleComponent->TakeOut();
	}
	else if (Current == ELayerState::STATE_CONSOLE_ACTIVE)
	{
		CachedPlayerCharacter->ConsoleComponent->PutAway();
	}
}

void AHorrorPlayerController::OnMenuPressed(const FInputActionValue& Value)
{
	if (CurrentSubState == EGameSubState::SUBSTATE_MENU)
	{
		ExitMenuState();
	}
	else if (CurrentSubState == EGameSubState::SUBSTATE_NORMAL)
	{
		EnterMenuState();
	}
}

void AHorrorPlayerController::PushPanel(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass) return;

	UUserWidget* Widget = CreateWidget<UUserWidget>(this, WidgetClass);
	if (Widget)
	{
		Widget->AddToViewport();
		PanelStack.Add(Widget);
	}
}

void AHorrorPlayerController::PopPanel()
{
	if (PanelStack.Num() == 0) return;

	UUserWidget* Top = PanelStack.Pop();
	if (Top)
	{
		Top->RemoveFromParent();
	}
}

UUserWidget* AHorrorPlayerController::GetTopPanel() const
{
	return PanelStack.Num() > 0 ? PanelStack.Last() : nullptr;
}

void AHorrorPlayerController::DisablePlayerMovement()
{
	if (!CachedPlayerCharacter)
	{
		CachedPlayerCharacter = Cast<APlayerCharacterB>(GetPawn());
	}
	if (CachedPlayerCharacter)
	{
		CachedPlayerCharacter->DisableMovement();
	}
}

void AHorrorPlayerController::EnablePlayerMovement()
{
	if (!CachedPlayerCharacter)
	{
		CachedPlayerCharacter = Cast<APlayerCharacterB>(GetPawn());
	}
	if (CachedPlayerCharacter)
	{
		CachedPlayerCharacter->EnableMovement();
	}
}

void AHorrorPlayerController::ShowHUD()
{
	if (!WBP_HUDClass) return;
	if (!HUDInstance)
	{
		HUDInstance = CreateWidget<UUserWidget>(this, WBP_HUDClass);
	}
	if (HUDInstance && !HUDInstance->IsInViewport())
	{
		HUDInstance->AddToViewport();
	}
}

void AHorrorPlayerController::HideHUD()
{
	if (HUDInstance && HUDInstance->IsInViewport())
	{
		HUDInstance->RemoveFromParent();
	}
}

void AHorrorPlayerController::ShowPauseMenu()
{
	if (!WBP_PauseMenuClass) return;
	if (!PauseMenuInstance)
	{
		PauseMenuInstance = CreateWidget<UUserWidget>(this, WBP_PauseMenuClass);
	}
	if (PauseMenuInstance && !PauseMenuInstance->IsInViewport())
	{
		PauseMenuInstance->AddToViewport();
	}
}

void AHorrorPlayerController::HidePauseMenu()
{
	if (PauseMenuInstance && PauseMenuInstance->IsInViewport())
	{
		PauseMenuInstance->RemoveFromParent();
	}
}

void AHorrorPlayerController::ShowConsoleUI()
{
	if (!WBP_ConsoleUIClass) return;
	if (!ConsoleUIInstance)
	{
		ConsoleUIInstance = CreateWidget<UUserWidget>(this, WBP_ConsoleUIClass);
	}
	if (ConsoleUIInstance && !ConsoleUIInstance->IsInViewport())
	{
		ConsoleUIInstance->AddToViewport();
	}
}

void AHorrorPlayerController::HideConsoleUI()
{
	if (ConsoleUIInstance && ConsoleUIInstance->IsInViewport())
	{
		ConsoleUIInstance->RemoveFromParent();
	}
}
