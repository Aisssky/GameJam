#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueLineWidget.generated.h"

class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API UDialogueLineWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowText(const FText& Text, float Duration, bool bLarge);
	void ClearText();
	bool IsShowing() const { return bIsActive; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void BP_OnShowDialogue(const FText& Text, bool bLarge);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void BP_OnClearDialogue();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Large;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Small;

private:
	FTimerHandle AutoHideTimer;
	bool bIsActive = false;

	void OnAutoHide();
};
