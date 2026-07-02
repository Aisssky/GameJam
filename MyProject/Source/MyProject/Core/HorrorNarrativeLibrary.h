#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HorrorNarrativeLibrary.generated.h"

UCLASS()
class MYPROJECT_API UHorrorNarrativeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Narrative", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "bAutoSaveAfterSet"))
	static void SetNarrativeFlag(UObject* WorldContextObject, FName FlagName, bool bValue = true, bool bAutoSaveAfterSet = true);

	UFUNCTION(BlueprintPure, Category = "Narrative", meta = (WorldContext = "WorldContextObject"))
	static bool HasNarrativeFlag(UObject* WorldContextObject, FName FlagName);

	UFUNCTION(BlueprintPure, Category = "Narrative", meta = (WorldContext = "WorldContextObject"))
	static TMap<FName, bool> GetAllNarrativeFlags(UObject* WorldContextObject);
};
