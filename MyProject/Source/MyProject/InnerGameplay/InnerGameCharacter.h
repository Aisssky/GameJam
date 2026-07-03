#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InnerGameCharacter.generated.h"

class UCameraComponent;
class ASceneCapture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInnerCharDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInnerCharCollectibleAdded, int32, TotalCollected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInnerCharCheckpointSet, int32, CheckpointIndex);

UCLASS()
class MYPROJECT_API AInnerGameCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AInnerGameCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 400.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector CaptureOffset = FVector(0.0f, 500.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CaptureYawOffset = -90.0f;

	void MoveForward(float Value);
	void DoJump();
	void StopJump();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "1"))
	int32 MaxLives = 3;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 CurrentLives = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float RespawnInvincibilityTime = 1.5f;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	FVector LastCheckpointLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	bool bHasCheckpoint = false;

	UPROPERTY(BlueprintReadOnly, Category = "Gameplay")
	int32 CollectedCount = 0;

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void TriggerDeath();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void RespawnAtCheckpoint();

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void SetCheckpoint(FVector Location, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	int32 AddCollectible(int32 Points = 1);

	UFUNCTION(BlueprintPure, Category = "Gameplay")
	bool IsInvincible() const { return bIsInvincible; }

	UPROPERTY(BlueprintAssignable, Category = "Gameplay|Events")
	FOnInnerCharDied OnCharacterDied;

	UPROPERTY(BlueprintAssignable, Category = "Gameplay|Events")
	FOnInnerCharCollectibleAdded OnCollectibleAdded;

	UPROPERTY(BlueprintAssignable, Category = "Gameplay|Events")
	FOnInnerCharCheckpointSet OnCheckpointSet;

private:
	UPROPERTY()
	TObjectPtr<ASceneCapture2D> CachedCapture;

	bool bIsInvincible = false;
	float InvincibilityTimer = 0.0f;

	void CacheSceneCapture();
	void UpdateCaptureTransform();
};
