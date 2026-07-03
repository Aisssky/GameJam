#include "DialogueLineWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UDialogueLineWidget::ShowText(const FText& Text, float Duration, bool bLarge)
{
	if (!GetWorld()) return;

	// Clear existing timer so repeated calls don't stack
	if (AutoHideTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoHideTimer);
	}

	if (Text_Large)
	{
		Text_Large->SetText(bLarge ? Text : FText::GetEmpty());
		Text_Large->SetVisibility(bLarge ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (Text_Small)
	{
		Text_Small->SetText(bLarge ? FText::GetEmpty() : Text);
		Text_Small->SetVisibility(bLarge ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	bIsActive = true;
	BP_OnShowDialogue(Text, bLarge);

	if (Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AutoHideTimer,
			this,
			&UDialogueLineWidget::OnAutoHide,
			Duration,
			false
		);
	}
}

void UDialogueLineWidget::ClearText()
{
	if (!GetWorld()) return;

	if (AutoHideTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoHideTimer);
	}

	if (Text_Large)
	{
		Text_Large->SetText(FText::GetEmpty());
	}
	if (Text_Small)
	{
		Text_Small->SetText(FText::GetEmpty());
	}

	bIsActive = false;
	BP_OnClearDialogue();
}

void UDialogueLineWidget::OnAutoHide()
{
	ClearText();
}
