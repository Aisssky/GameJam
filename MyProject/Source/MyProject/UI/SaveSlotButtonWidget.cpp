#include "SaveSlotButtonWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void USaveSlotButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UButton* RootButton = Cast<UButton>(GetRootWidget()))
	{
		RootButton->OnClicked.AddDynamic(this, &USaveSlotButtonWidget::OnSlotClicked);
	}

	UpdateDisplayFromSlotInfo();
}

void USaveSlotButtonWidget::SetupSlot(const FMetaHorrorSaveSlotInfo& InSlotInfo)
{
	SlotInfo = InSlotInfo;
	bSelected = false;
	UpdateDisplayFromSlotInfo();
}

void USaveSlotButtonWidget::SetSelected(bool bInSelected)
{
	bSelected = bInSelected;

	if (BorderRef)
	{
		BorderRef->SetBrushColor(bSelected ? HighlightedColor : NormalColor);
	}

	BP_OnSelectionChanged(bSelected);
}

void USaveSlotButtonWidget::RefreshDisplay(const FMetaHorrorSaveSlotInfo& NewSlotInfo)
{
	SlotInfo = NewSlotInfo;
	bSelected = false;

	if (BorderRef)
	{
		BorderRef->SetBrushColor(NormalColor);
	}

	UpdateDisplayFromSlotInfo();
	BP_OnSlotInfoUpdated(NewSlotInfo);
}

void USaveSlotButtonWidget::OnSlotClicked()
{
	if (!SlotInfo.bExists && SlotInfo.bCanCreate)
	{
		OnEmptySlotClicked.Broadcast();
		return;
	}

	if (bSelected)
	{
		bSelected = false;
		if (BorderRef)
		{
			BorderRef->SetBrushColor(NormalColor);
		}
		OnSlotDeselected.Broadcast();
		BP_OnSelectionChanged(false);
	}
	else
	{
		bSelected = true;
		if (BorderRef)
		{
			BorderRef->SetBrushColor(HighlightedColor);
		}
		OnSlotSelected.Broadcast(SlotInfo);
		BP_OnSelectionChanged(true);
	}
}

void USaveSlotButtonWidget::UpdateDisplayFromSlotInfo()
{
	if (!NameText || !BorderRef)
	{
		return;
	}

	if (!SlotInfo.bExists)
	{
		if (SlotInfo.bCanCreate)
		{
			NameText->SetText(FText::FromString(TEXT("新建存档")));
			BorderRef->SetBrushColor(NormalColor);
		}
		else
		{
			NameText->SetText(FText::FromString(TEXT("存档不可用")));
			BorderRef->SetBrushColor(LockedColor);
		}
	}
	else
	{
		if (SlotInfo.bCanRead)
		{
			NameText->SetText(FText::FromString(SlotInfo.DisplayName));
			BorderRef->SetBrushColor(NormalColor);
		}
		else
		{
			NameText->SetText(FText::FromString(SlotInfo.DisplayName + TEXT(" [Locked]")));
			BorderRef->SetBrushColor(LockedColor);
		}
	}
}
