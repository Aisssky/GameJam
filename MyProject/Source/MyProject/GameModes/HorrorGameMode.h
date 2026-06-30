#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HorrorGameMode.generated.h"

class ULevelStreaming;

UCLASS()
class MYPROJECT_API AHorrorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHorrorGameMode();

	virtual void BeginPlay() override;
};

UCLASS()
class MYPROJECT_API AMenuGameMode : public AHorrorGameMode
{
	GENERATED_BODY()

public:
	AMenuGameMode();

	virtual void BeginPlay() override;
};

UCLASS()
class MYPROJECT_API AOuterGameplayGameMode : public AHorrorGameMode
{
	GENERATED_BODY()

public:
	AOuterGameplayGameMode();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Streaming")
	FName ConsoleStreamingLevelName = TEXT("L_AGameplay");

protected:
	void HideConsoleLevelActors();
	void SetupConsoleSceneCapture();

	UFUNCTION()
	void OnConsoleLevelLoaded();

	UPROPERTY()
	TObjectPtr<ULevelStreaming> CachedConsoleStreamingLevel;
};
