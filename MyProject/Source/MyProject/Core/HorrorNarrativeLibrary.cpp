#include "HorrorNarrativeLibrary.h"

#include "HorrorGameInstance.h"
#include "Engine/World.h"

namespace
{
	UHorrorGameInstance* GetHorrorGameInstanceFromContext(UObject* WorldContextObject)
	{
		UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
		return World ? Cast<UHorrorGameInstance>(World->GetGameInstance()) : nullptr;
	}
}

void UHorrorNarrativeLibrary::SetNarrativeFlag(UObject* WorldContextObject, FName FlagName, bool bValue, bool bAutoSaveAfterSet)
{
	if (UHorrorGameInstance* GI = GetHorrorGameInstanceFromContext(WorldContextObject))
	{
		GI->SetFlag(FlagName, bValue, bAutoSaveAfterSet);
	}
}

bool UHorrorNarrativeLibrary::HasNarrativeFlag(UObject* WorldContextObject, FName FlagName)
{
	if (const UHorrorGameInstance* GI = GetHorrorGameInstanceFromContext(WorldContextObject))
	{
		return GI->HasFlag(FlagName);
	}

	return false;
}

TMap<FName, bool> UHorrorNarrativeLibrary::GetAllNarrativeFlags(UObject* WorldContextObject)
{
	if (const UHorrorGameInstance* GI = GetHorrorGameInstanceFromContext(WorldContextObject))
	{
		return GI->GetAllFlags();
	}

	return TMap<FName, bool>();
}
