#include "TaskPromptWidget.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UTaskPromptWidget::ShowTask(const FText& TaskText, float Duration)
{
	if (!GetWorld()) return;

	if (AutoHideTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoHideTimer);
	}

	if (Text_Task)
	{
		Text_Task->SetText(TaskText);
	}

	bIsActive = true;
	BP_OnShowTask(TaskText);

	if (Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AutoHideTimer,
			this,
			&UTaskPromptWidget::OnAutoHide,
			Duration,
			false
		);
	}
}

void UTaskPromptWidget::ClearTask()
{
	if (!GetWorld()) return;

	if (AutoHideTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoHideTimer);
	}

	if (Text_Task)
	{
		Text_Task->SetText(FText::GetEmpty());
	}

	bIsActive = false;
	BP_OnClearTask();
}

void UTaskPromptWidget::OnAutoHide()
{
	ClearTask();
}
