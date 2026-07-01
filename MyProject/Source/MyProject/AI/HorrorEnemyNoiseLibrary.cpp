#include "HorrorEnemyNoiseLibrary.h"

#include "GameFramework/Actor.h"
#include "Perception/AISense_Hearing.h"

FName UHorrorEnemyNoiseLibrary::GetHorrorNoiseTag(EHorrorNoiseType NoiseType)
{
	switch (NoiseType)
	{
	case EHorrorNoiseType::FootstepWalk:
		return TEXT("Noise_Footstep_Walk");
	case EHorrorNoiseType::FootstepRun:
		return TEXT("Noise_Footstep_Run");
	case EHorrorNoiseType::DoorOpen:
		return TEXT("Noise_DoorOpen");
	case EHorrorNoiseType::ObjectDrop:
		return TEXT("Noise_ObjectDrop");
	default:
		return NAME_None;
	}
}

float UHorrorEnemyNoiseLibrary::GetHorrorNoiseRange(EHorrorNoiseType NoiseType)
{
	switch (NoiseType)
	{
	case EHorrorNoiseType::FootstepWalk:
		return 500.0f;
	case EHorrorNoiseType::FootstepRun:
		return 1200.0f;
	case EHorrorNoiseType::DoorOpen:
		return 2000.0f;
	case EHorrorNoiseType::ObjectDrop:
		return 1500.0f;
	default:
		return 0.0f;
	}
}

void UHorrorEnemyNoiseLibrary::ReportHorrorNoise(
	UObject* WorldContextObject,
	AActor* NoiseInstigator,
	EHorrorNoiseType NoiseType,
	FVector NoiseLocation,
	float Loudness,
	float MaxRangeOverride)
{
	const float MaxRange = MaxRangeOverride > 0.0f
		? MaxRangeOverride
		: GetHorrorNoiseRange(NoiseType);

	ReportCustomHorrorNoise(
		WorldContextObject,
		NoiseInstigator,
		GetHorrorNoiseTag(NoiseType),
		NoiseLocation,
		MaxRange,
		Loudness);
}

void UHorrorEnemyNoiseLibrary::ReportCustomHorrorNoise(
	UObject* WorldContextObject,
	AActor* NoiseInstigator,
	FName NoiseTag,
	FVector NoiseLocation,
	float MaxRange,
	float Loudness)
{
	if (!WorldContextObject || MaxRange <= 0.0f)
	{
		return;
	}

	if (NoiseLocation.IsNearlyZero() && NoiseInstigator)
	{
		NoiseLocation = NoiseInstigator->GetActorLocation();
	}

	UAISense_Hearing::ReportNoiseEvent(
		WorldContextObject,
		NoiseLocation,
		FMath::Max(0.0f, Loudness),
		NoiseInstigator,
		MaxRange,
		NoiseTag);
}
