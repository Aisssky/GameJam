#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InnerGameCharacter.generated.h"

class UCameraComponent;
class ASceneCapture2D;

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

private:
	UPROPERTY()
	TObjectPtr<ASceneCapture2D> CachedCapture;

	void CacheSceneCapture();
	void UpdateCaptureTransform();
};
