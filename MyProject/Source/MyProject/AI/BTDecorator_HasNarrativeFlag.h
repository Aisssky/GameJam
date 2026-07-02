#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_HasNarrativeFlag.generated.h"

UCLASS()
class MYPROJECT_API UBTDecorator_HasNarrativeFlag : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_HasNarrativeFlag();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	FName FlagName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
	bool bExpectedValue = true;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
};
