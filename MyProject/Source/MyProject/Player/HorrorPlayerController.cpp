#include "HorrorPlayerController.h"
#include "PlayerCharacterB.h"
#include "../InnerGameplay/InnerGameCharacter.h"
#include "../Core/HorrorGameInstance.h"
#include "../UI/HorrorUIComponent.h"
#include "../Weapon/HorrorAbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "EngineUtils.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"

AHorrorPlayerController::AHorrorPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	CurrentSubState = EGameSubState::SUBSTATE_NORMAL;

	UIComponent = CreateDefaultSubobject<UHorrorUIComponent>(TEXT("UIComponent"));
}

void AHorrorPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bPendingPauseAutoClose && FPlatformTime::Seconds() >= PauseAutoCloseRealtimeSeconds)
	{
		bPendingPauseAutoClose = false;
		ResumeGameFromPause();
	}
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
		if (IA_InnerMove)
		{
			EnhancedInput->BindAction(IA_InnerMove, ETriggerEvent::Triggered, this, &AHorrorPlayerController::OnInnerMove);
		}
		if (IA_InnerJump)
		{
			EnhancedInput->BindAction(IA_InnerJump, ETriggerEvent::Started, this, &AHorrorPlayerController::OnInnerJump);
			EnhancedInput->BindAction(IA_InnerJump, ETriggerEvent::Completed, this, &AHorrorPlayerController::OnInnerJump);
		}
	}
}

void AHorrorPlayerController::OnLayerStateChanged(ELayerState From, ELayerState To)
{
	ClearAllInputContexts();

	const bool bFromMovementBlocked = (From == ELayerState::STATE_CONSOLE_ACTIVE || From == ELayerState::STATE_INNER_GAMEPLAY);
	const bool bToMovementBlocked   = (To   == ELayerState::STATE_CONSOLE_ACTIVE || To   == ELayerState::STATE_INNER_GAMEPLAY);

	if (!bFromMovementBlocked && bToMovementBlocked)
	{
		DisablePlayerMovement();
	}
	else if (bFromMovementBlocked && !bToMovementBlocked)
	{
		EnablePlayerMovement();
	}

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
		break;

	case ELayerState::STATE_CONSOLE_ACTIVE:
		PushInputContext(IMC_Gameplay, 1);
		PushInputContext(IMC_ConsoleUI, 2);
		PushInputContext(IMC_InnerGameplay, 4);
		SetShowMouseCursor(true);
		SetInputMode(FInputModeGameAndUI());
		break;

	case ELayerState::STATE_INNER_GAMEPLAY:
		PushInputContext(IMC_ConsoleUI, 2);
		PushInputContext(IMC_InnerGameplay, 4);
		EnsureInnerCharacterCached();
		SetShowMouseCursor(false);
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
		if (IMC_Gameplay)     Subsystem->RemoveMappingContext(IMC_Gameplay);
		if (IMC_Menu)         Subsystem->RemoveMappingContext(IMC_Menu);
		if (IMC_ConsoleUI)    Subsystem->RemoveMappingContext(IMC_ConsoleUI);
		if (IMC_InnerGameplay)Subsystem->RemoveMappingContext(IMC_InnerGameplay);
		if (IMC_Cinematic)    Subsystem->RemoveMappingContext(IMC_Cinematic);
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
		CurrentSubState = NewState;
		DisablePlayerMovement();
		bMenuMovementBlocked = (CachedPlayerCharacter != nullptr);
		if (UIComponent)
		{
			UIComponent->SetSubStateBlocked(true);
		}
		if (CachedPlayerCharacter && CachedPlayerCharacter->AbilitySystemComponent)
		{
			CachedPlayerCharacter->AbilitySystemComponent->SetMenuBlocked(true);
		}
		EnterMenuState();
		return true;

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

	if (UIComponent)
	{
		UIComponent->SetSubStateBlocked(CurrentSubState != EGameSubState::SUBSTATE_NORMAL);
	}

	if (CachedPlayerCharacter && CachedPlayerCharacter->AbilitySystemComponent)
	{
		const bool bMenuBlocked = (CurrentSubState != EGameSubState::SUBSTATE_NORMAL);
		CachedPlayerCharacter->AbilitySystemComponent->SetMenuBlocked(bMenuBlocked);
	}

	return true;
}

void AHorrorPlayerController::EnterMenuState()
{
	PushInputContext(IMC_Menu, 1);
	UGameplayStatics::SetGlobalTimeDilation(this, 0.0f);
	SetShowMouseCursor(true);
	SetInputMode(FInputModeGameAndUI());

	if (UIComponent)
	{
		UIComponent->ShowPauseMenu();
	}
}

void AHorrorPlayerController::ExitMenuState()
{
	CloseMenuState(true);
}

void AHorrorPlayerController::CloseMenuState(bool bRestoreGameplayInput)
{
	if (UIComponent)
	{
		UIComponent->HidePauseMenu();
		UIComponent->PopPanel();
	}

	ClearAllInputContexts();
	if (bRestoreGameplayInput)
	{
		PushInputContext(IMC_Gameplay, 3);
		SetInputMode(FInputModeGameOnly());
	}

	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
	SetShowMouseCursor(false);

	bPendingPauseAutoClose = false;
	if (bMenuMovementBlocked)
	{
		EnablePlayerMovement();
		bMenuMovementBlocked = false;
	}
	SetSubState(EGameSubState::SUBSTATE_NORMAL);
}

void AHorrorPlayerController::ResumeGameFromPause()
{
	ExitMenuState();
}

bool AHorrorPlayerController::SaveGameFromPause()
{
	if (!CachedGameInstance)
	{
		CachedGameInstance = Cast<UHorrorGameInstance>(GetGameInstance());
	}

	if (!CachedGameInstance)
	{
		BP_OnPauseMenuSaveFailed(FText::FromString(TEXT("GameInstance 不存在，无法保存。")));
		return false;
	}

	const bool bSaved = CachedGameInstance->ManualSaveGame(GetCurrentPlayerActor());
	if (!bSaved)
	{
		BP_OnPauseMenuSaveFailed(FText::FromString(TEXT("保存失败，请确认玩家存档已创建。")));
		if (UIComponent)
		{
			UIComponent->ShowPauseMenuSaveFailedMessage(FText::FromString(TEXT("保存失败")));
		}
		return false;
	}

	if (UIComponent)
	{
		UIComponent->ShowPauseMenuSavedMessage(PauseSaveMessageDuration);
	}
	BP_OnPauseMenuSaveSucceeded(PauseSaveMessageDuration);

	bPendingPauseAutoClose = true;
	PauseAutoCloseRealtimeSeconds = FPlatformTime::Seconds() + PauseSaveMessageDuration;
	return true;
}

void AHorrorPlayerController::ReturnToMainMenuFromPause()
{
	if (!CachedGameInstance)
	{
		CachedGameInstance = Cast<UHorrorGameInstance>(GetGameInstance());
	}

	if (CachedGameInstance)
	{
		CachedGameInstance->AutoSaveGame(GetCurrentPlayerActor());
	}

	BP_OnReturnToMainMenuRequested();
	CloseMenuState(false);

	if (CachedGameInstance)
	{
		CachedGameInstance->TransitionToLayer(ELayerState::STATE_MAIN_MENU);
	}

	if (!MainMenuLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, MainMenuLevelName);
	}
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
	else if (Current == ELayerState::STATE_INNER_GAMEPLAY)
	{
		CachedGameInstance->TransitionToLayer(ELayerState::STATE_CONSOLE_ACTIVE);
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
		SetSubState(EGameSubState::SUBSTATE_MENU);
	}
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

void AHorrorPlayerController::EnsureInnerCharacterCached()
{
	if (CachedInnerCharacter && IsValid(CachedInnerCharacter))
	{
		return;
	}

	for (TActorIterator<AInnerGameCharacter> It(GetWorld()); It; ++It)
	{
		AInnerGameCharacter* InnerChar = *It;
		if (IsValid(InnerChar))
		{
			CachedInnerCharacter = InnerChar;
			return;
		}
	}
}

AActor* AHorrorPlayerController::GetCurrentPlayerActor()
{
	if (!CachedPlayerCharacter)
	{
		CachedPlayerCharacter = Cast<APlayerCharacterB>(GetPawn());
	}

	return CachedPlayerCharacter ? Cast<AActor>(CachedPlayerCharacter.Get()) : Cast<AActor>(GetPawn());
}

void AHorrorPlayerController::ShowDialogue(const FText& Text, float Duration, bool bLarge)
{
	if (UIComponent)
	{
		UIComponent->ShowDialogueLine(Text, Duration, bLarge);
	}
}

void AHorrorPlayerController::ClearDialogue()
{
	if (UIComponent)
	{
		UIComponent->ClearDialogueLine();
	}
}

void AHorrorPlayerController::ShowTaskPrompt(const FText& TaskText, float Duration)
{
	if (UIComponent)
	{
		UIComponent->ShowTaskPrompt(TaskText, Duration);
	}
}

void AHorrorPlayerController::ClearTaskPrompt()
{
	if (UIComponent)
	{
		UIComponent->ClearTaskPrompt();
	}
}

void AHorrorPlayerController::OnInnerMove(const FInputActionValue& Value)
{
	EnsureInnerCharacterCached();

	if (!CachedInnerCharacter || !CachedGameInstance)
	{
		return;
	}

	if (CachedGameInstance->CurrentLayerState != ELayerState::STATE_INNER_GAMEPLAY)
	{
		return;
	}

	const float MoveValue = Value.Get<float>();
	CachedInnerCharacter->MoveForward(MoveValue);
}

void AHorrorPlayerController::OnInnerJump(const FInputActionValue& Value)
{
	if (!CachedGameInstance)
	{
		return;
	}

	const ELayerState Current = CachedGameInstance->CurrentLayerState;

	if (Current == ELayerState::STATE_CONSOLE_ACTIVE)
	{
		if (Value.Get<bool>() && CachedPlayerCharacter && CachedPlayerCharacter->ConsoleComponent)
		{
			CachedPlayerCharacter->ConsoleComponent->EnterInnerGameplay();
		}
		return;
	}

	// L4: Space makes A jump
	if (Current != ELayerState::STATE_INNER_GAMEPLAY)
	{
		return;
	}

	EnsureInnerCharacterCached();
	if (!CachedInnerCharacter)
	{
		return;
	}

	if (Value.Get<bool>())
	{
		CachedInnerCharacter->DoJump();
	}
	else
	{
		CachedInnerCharacter->StopJump();
	}
}
