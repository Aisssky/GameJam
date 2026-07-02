#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Core/HorrorTypes.h"
#include "HorrorUIComponent.generated.h"

class UUserWidget;
class UHorrorGameInstance;
class UPauseMenuWidget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UHorrorUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHorrorUIComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Classes")
	TSubclassOf<UUserWidget> WBP_HUDClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Classes")
	TSubclassOf<UUserWidget> WBP_PauseMenuClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Classes")
	TSubclassOf<UUserWidget> WBP_ConsoleUIClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Classes")
	TSubclassOf<UUserWidget> WBP_InnerGameplayHUDClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Classes")
	TSubclassOf<UUserWidget> WBP_MainMenuClass;

	UFUNCTION(BlueprintCallable, Category = "UI|Panel")
	void PushPanel(TSubclassOf<UUserWidget> WidgetClass);

	UFUNCTION(BlueprintCallable, Category = "UI|Panel")
	void PopPanel();

	UFUNCTION(BlueprintPure, Category = "UI|Panel")
	UUserWidget* GetTopPanel() const;

	UFUNCTION(BlueprintCallable, Category = "UI|State")
	void SetSubStateBlocked(bool bBlocked);

	void ShowPauseMenu();
	void HidePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI|Pause Menu")
	void ShowPauseMenuSavedMessage(float DisplaySeconds);

	UFUNCTION(BlueprintCallable, Category = "UI|Pause Menu")
	void ShowPauseMenuSaveFailedMessage(const FText& Reason);

protected:
	UFUNCTION()
	void OnLayerChangedCallback(ELayerState FromLayer, ELayerState ToLayer);

	UFUNCTION()
	void HandlePauseMenuSaveClicked();

	UFUNCTION()
	void HandlePauseMenuReturnMainMenuClicked();

	UFUNCTION()
	void HandlePauseMenuReturnGameClicked();

	void ShowHUD();
	void HideHUD();
	void ShowConsoleUI();
	void HideConsoleUI();
	void ShowInnerGameplayHUD();
	void HideInnerGameplayHUD();
	void ShowMainMenu();
	void HideMainMenu();
	void HideAll();

	void RefreshUIForCurrentState();
	void BindPauseMenuButtons();
	void SetPauseMenuStatusText(const FText& Message);

	UPROPERTY()
	TObjectPtr<UHorrorGameInstance> CachedGameInstance;

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDInstance;

	UPROPERTY()
	TObjectPtr<UUserWidget> PauseMenuInstance;

	UPROPERTY()
	TObjectPtr<UUserWidget> ConsoleUIInstance;

	UPROPERTY()
	TObjectPtr<UUserWidget> InnerGameplayHUDInstance;

	UPROPERTY()
	TObjectPtr<UUserWidget> MainMenuInstance;

	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> PanelStack;

	ELayerState CurrentLayer;
	bool bSubStateBlocked;
};
