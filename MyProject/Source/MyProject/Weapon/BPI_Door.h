#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BPI_Door.generated.h"

UINTERFACE(BlueprintType)
class MYPROJECT_API UBPI_Door : public UInterface
{
	GENERATED_BODY()
};

class MYPROJECT_API IBPI_Door
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Door")
	bool CanPlaceTalisman() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Door")
	void PlaceTalisman(float BlockDuration);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Door")
	bool HasActiveTalisman() const;
};
