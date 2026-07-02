#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class AHorrorPlayerController;
class UButton;
class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void ShowSavedMessage(float DisplaySeconds);

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void ShowSaveFailedMessage(const FText& Reason);

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void ClearStatusMessage();

	UFUNCTION(BlueprintImplementableEvent, Category = "Pause Menu")
	void BP_ShowSavedMessage(float DisplaySeconds);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pause Menu")
	void BP_ShowSaveFailedMessage(const FText& Reason);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pause Menu")
	void BP_ClearStatusMessage();

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_SaveGame;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_ReturnMainMenu;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_ReturnGame;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_StatusMessage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_SaveStatus;

	UFUNCTION()
	void HandleSaveGameClicked();

	UFUNCTION()
	void HandleReturnMainMenuClicked();

	UFUNCTION()
	void HandleReturnGameClicked();

	AHorrorPlayerController* GetHorrorPlayerController() const;
	void SetStatusText(const FText& Message);
};
