#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MetaHorrorAutoSaveTrigger.generated.h"

class UBoxComponent;

/**
 * 关卡自动保存触发器。
 * 拖进 L_BGameplay，玩家进入 Box 后自动调用 UHorrorGameInstance::AutoSaveGame。
 */
UCLASS(Blueprintable)
class MYPROJECT_API AMetaHorrorAutoSaveTrigger : public AActor
{
	GENERATED_BODY()

public:
	AMetaHorrorAutoSaveTrigger();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Auto Save")
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Auto Save")
	bool bTriggerOnlyOnce = true;

	UPROPERTY(BlueprintReadOnly, Category = "Auto Save")
	bool bHasTriggered = false;

	UFUNCTION()
	void OnBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
