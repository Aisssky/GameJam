#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "InnerGameZone.h"
#include "InnerGameplayManager.generated.h"

class UHorrorGameInstance;
class AInnerGameCharacter;
class ASceneCapture2D;
class USceneCaptureComponent2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnL4LifecycleEvent);

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API AInnerGameplayManager : public AActor
{
	GENERATED_BODY()

public:
	AInnerGameplayManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InnerGameplay|Config")
	FName L4LevelName = TEXT("L_AGameplay");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InnerGameplay|Config")
	FName SceneCaptureActorName = TEXT("BP_InnerCapture");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InnerGameplay|Config")
	bool bShowOnlyL4Actors = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InnerGameplay|Config")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InnerGameplay|Config")
	bool bAutoStartOnL4Enter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InnerGameplay|Config")
	bool bAutoEndOnL4Exit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InnerGameplay|Config")
	int32 RequiredCollectibles = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InnerGameplay|Config")
	FName L4WinNarrativeFlag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InnerGameplay|Config")
	FName L4LoseNarrativeFlag;

	UPROPERTY(BlueprintReadOnly, Category = "InnerGameplay|Runtime")
	bool bIsL4Active = false;

	UPROPERTY(BlueprintAssignable, Category = "InnerGameplay|Events")
	FOnL4LifecycleEvent OnL4LevelLoaded;

	UPROPERTY(BlueprintAssignable, Category = "InnerGameplay|Events")
	FOnL4LifecycleEvent OnL4LevelUnloaded;

	UPROPERTY(BlueprintAssignable, Category = "InnerGameplay|Events")
	FOnL4LifecycleEvent OnL4Started;

	UPROPERTY(BlueprintAssignable, Category = "InnerGameplay|Events")
	FOnL4LifecycleEvent OnL4Ended;

	UPROPERTY(BlueprintAssignable, Category = "InnerGameplay|Events")
	FOnL4LifecycleEvent OnGameWon;

	UPROPERTY(BlueprintAssignable, Category = "InnerGameplay|Events")
	FOnL4LifecycleEvent OnGameLost;

	UFUNCTION(BlueprintImplementableEvent, Category = "InnerGameplay|Events", meta = (DisplayName = "On L4 Tick"))
	void ReceiveL4Tick(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "InnerGameplay")
	void StartInnerGame();

	UFUNCTION(BlueprintCallable, Category = "InnerGameplay")
	void EndInnerGame();

	UFUNCTION(BlueprintPure, Category = "InnerGameplay")
	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

	UFUNCTION(BlueprintPure, Category = "InnerGameplay")
	bool IsInnerGameActive() const { return bIsL4Active; }

	UFUNCTION(BlueprintCallable, Category = "InnerGameplay")
	void TransitionToL4();

	UFUNCTION(BlueprintCallable, Category = "InnerGameplay")
	void TransitionToL3();

	UFUNCTION(BlueprintCallable, Category = "InnerGameplay")
	void TriggerWin();

	UFUNCTION(BlueprintCallable, Category = "InnerGameplay")
	void TriggerLose();

protected:
	UFUNCTION()
	void OnLayerChangedCallback(ELayerState From, ELayerState To);

private:
	UPROPERTY()
	TObjectPtr<UHorrorGameInstance> CachedGameInstance;

	UPROPERTY()
	TObjectPtr<ASceneCapture2D> CachedSceneCapture;

	UPROPERTY()
	TObjectPtr<AInnerGameCharacter> CachedInnerCharacter;

	int32 CollectedInLevel = 0;

	void SetupSceneCapture();
	void ClearSceneCapture();
	void EnsureSceneCaptureCached();
	void CacheInnerCharacter();
	void BindInnerGameObjects();

	UFUNCTION()
	void HandleCollectibleCollected(FName ID, AActor* Collector);

	UFUNCTION()
	void HandleZoneTriggered(EInnerGameZoneType ZoneType, AActor* Trigger);

	UFUNCTION()
	void HandleCharacterDied();
};
