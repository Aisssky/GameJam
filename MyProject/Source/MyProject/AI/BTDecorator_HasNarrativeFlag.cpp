#include "BTDecorator_HasNarrativeFlag.h"

#include "../Core/HorrorGameInstance.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Engine/World.h"

UBTDecorator_HasNarrativeFlag::UBTDecorator_HasNarrativeFlag()
{
	NodeName = TEXT("Has Narrative Flag");
}

bool UBTDecorator_HasNarrativeFlag::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	if (FlagName.IsNone())
	{
		return false;
	}

	UWorld* World = OwnerComp.GetWorld();
	const UHorrorGameInstance* GI = World ? Cast<UHorrorGameInstance>(World->GetGameInstance()) : nullptr;
	if (!GI)
	{
		return false;
	}

	return GI->HasFlag(FlagName) == bExpectedValue;
}

FString UBTDecorator_HasNarrativeFlag::GetStaticDescription() const
{
	return FString::Printf(
		TEXT("%s\n%s == %s"),
		*Super::GetStaticDescription(),
		*FlagName.ToString(),
		bExpectedValue ? TEXT("true") : TEXT("false"));
}
