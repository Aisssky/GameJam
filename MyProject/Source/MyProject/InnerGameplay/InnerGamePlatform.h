#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InnerGamePlatform.generated.h"

class UStaticMeshComponent;

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API AInnerGamePlatform : public AActor
{
	GENERATED_BODY()

public:
	AInnerGamePlatform();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform|Components")
	TObjectPtr<UStaticMeshComponent> PlatformMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform|Motion")
	FVector MoveDirection = FVector(0.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform|Motion")
	float MoveDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform|Motion")
	float MoveSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform|Motion")
	bool bPingPong = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Platform|Motion")
	bool bStartMoving = true;

	UFUNCTION(BlueprintCallable, Category = "Platform")
	void StartMoving();

	UFUNCTION(BlueprintCallable, Category = "Platform")
	void StopMoving();

private:
	FVector StartLocation;
	float CurrentDistance = 0.0f;
	bool bMovingForward = true;
	bool bIsMoving = true;
};
