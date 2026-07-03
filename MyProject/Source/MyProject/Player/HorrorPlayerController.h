#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "../Core/HorrorTypes.h"
#include "HorrorPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class UHorrorGameInstance;
class APlayerCharacterB;
class UHorrorUIComponent;
class AInnerGameCharacter;

UCLASS()
class MYPROJECT_API AHorrorPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHorrorPlayerController();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void ResumeGameFromPause();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	bool SaveGameFromPause();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void ReturnToMainMenuFromPause();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void ExitMenuState();

	UFUNCTION(BlueprintPure, Category = "Pause Menu")
	bool IsPauseMenuOpen() const { return CurrentSubState == EGameSubState::SUBSTATE_MENU; }

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ShowDialogue(const FText& Text, float Duration = 4.0f, bool bLarge = false);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ClearDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ShowTaskPrompt(const FText& TaskText, float Duration = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ClearTaskPrompt();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;

	UFUNCTION()
	void OnLayerStateChanged(ELayerState From, ELayerState To);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC_Gameplay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC_Menu;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC_ConsoleUI;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC_Cinematic;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ToggleConsole;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Menu;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|InnerGame")
	TObjectPtr<UInputMappingContext> IMC_InnerGameplay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|InnerGame")
	TObjectPtr<UInputAction> IA_InnerMove;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|InnerGame")
	TObjectPtr<UInputAction> IA_InnerJump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pause Menu")
	FName MainMenuLevelName = TEXT("L_MainMenu");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pause Menu", meta = (ClampMin = "0.0"))
	float PauseSaveMessageDuration = 2.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UHorrorUIComponent> UIComponent;

	UFUNCTION(BlueprintPure, Category = "Sub State")
	EGameSubState GetSubState() const { return CurrentSubState; }

	UFUNCTION(BlueprintCallable, Category = "Sub State")
	bool SetSubState(EGameSubState NewState);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DisablePlayerMovement();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EnablePlayerMovement();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Sub State")
	EGameSubState CurrentSubState;

	UPROPERTY()
	TObjectPtr<UHorrorGameInstance> CachedGameInstance;

	void OnToggleConsole(const struct FInputActionValue& Value);
	void OnMenuPressed(const struct FInputActionValue& Value);
	void OnInnerMove(const struct FInputActionValue& Value);
	void OnInnerJump(const struct FInputActionValue& Value);

	void PushInputContext(UInputMappingContext* Context, int32 Priority);
	void ClearAllInputContexts();

	void EnterMenuState();
	void CloseMenuState(bool bRestoreGameplayInput);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pause Menu")
	void BP_OnPauseMenuSaveSucceeded(float DisplaySeconds);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pause Menu")
	void BP_OnPauseMenuSaveFailed(const FText& Reason);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pause Menu")
	void BP_OnReturnToMainMenuRequested();

private:
	UPROPERTY()
	TObjectPtr<APlayerCharacterB> CachedPlayerCharacter;

	UPROPERTY()
	TObjectPtr<AInnerGameCharacter> CachedInnerCharacter;

	bool bPendingPauseAutoClose = false;
	double PauseAutoCloseRealtimeSeconds = 0.0;
	bool bMenuMovementBlocked = false;

	void EnsureInnerCharacterCached();
	AActor* GetCurrentPlayerActor();
};
