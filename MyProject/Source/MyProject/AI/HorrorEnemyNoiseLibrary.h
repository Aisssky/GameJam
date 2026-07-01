#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HorrorEnemyNoiseLibrary.generated.h"

UENUM(BlueprintType)
enum class EHorrorNoiseType : uint8
{
	FootstepWalk UMETA(DisplayName = "Footstep Walk"),
	FootstepRun UMETA(DisplayName = "Footstep Run"),
	DoorOpen UMETA(DisplayName = "Door Open"),
	ObjectDrop UMETA(DisplayName = "Object Drop")
};

UCLASS()
class MYPROJECT_API UHorrorEnemyNoiseLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Enemy AI|Noise")
	static FName GetHorrorNoiseTag(EHorrorNoiseType NoiseType);

	UFUNCTION(BlueprintPure, Category = "Enemy AI|Noise")
	static float GetHorrorNoiseRange(EHorrorNoiseType NoiseType);

	UFUNCTION(BlueprintCallable, Category = "Enemy AI|Noise", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Loudness, MaxRangeOverride"))
	static void ReportHorrorNoise(
		UObject* WorldContextObject,
		AActor* NoiseInstigator,
		EHorrorNoiseType NoiseType,
		FVector NoiseLocation,
		float Loudness = 1.0f,
		float MaxRangeOverride = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Enemy AI|Noise", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Loudness"))
	static void ReportCustomHorrorNoise(
		UObject* WorldContextObject,
		AActor* NoiseInstigator,
		FName NoiseTag,
		FVector NoiseLocation,
		float MaxRange,
		float Loudness = 1.0f);
};
