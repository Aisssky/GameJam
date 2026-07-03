#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InnerGameCollectible.generated.h"

class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollectibleCollectedSig, FName, CollectibleID, AActor*, Collector);

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API AInnerGameCollectible : public AActor
{
	GENERATED_BODY()

public:
	AInnerGameCollectible();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible")
	FName CollectibleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible")
	int32 PointsValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible")
	bool bDestroyOnCollect = true;

	UFUNCTION(BlueprintCallable, Category = "Collectible")
	void Collect(AActor* Collector);

	UFUNCTION(BlueprintImplementableEvent, Category = "Collectible")
	void BP_OnCollected(AActor* Collector);

	UPROPERTY(BlueprintAssignable, Category = "Collectible|Events")
	FOnCollectibleCollectedSig OnCollected;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collectible|Components")
	TObjectPtr<USphereComponent> TriggerSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collectible|Components")
	TObjectPtr<UStaticMeshComponent> CollectibleMesh;

protected:
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

private:
	bool bCollected = false;
};
