#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TaskPromptWidget.generated.h"

class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API UTaskPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowTask(const FText& TaskText, float Duration = 0.0f);
	void ClearTask();
	bool IsShowing() const { return bIsActive; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Task")
	void BP_OnShowTask(const FText& TaskText);

	UFUNCTION(BlueprintImplementableEvent, Category = "Task")
	void BP_OnClearTask();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Task;

private:
	FTimerHandle AutoHideTimer;
	bool bIsActive = false;

	void OnAutoHide();
};
