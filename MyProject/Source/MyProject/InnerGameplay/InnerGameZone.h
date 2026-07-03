#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InnerGameZone.generated.h"

class UBoxComponent;

UENUM(BlueprintType)
enum class EInnerGameZoneType : uint8
{
	WinZone    UMETA(DisplayName = "Win Zone"),
	DeathZone  UMETA(DisplayName = "Death Zone"),
	Checkpoint UMETA(DisplayName = "Checkpoint"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneTriggeredSig, EInnerGameZoneType, ZoneType, AActor*, TriggeringActor);

UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API AInnerGameZone : public AActor
{
	GENERATED_BODY()

public:
	AInnerGameZone();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	EInnerGameZoneType ZoneType = EInnerGameZoneType::DeathZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone|Checkpoint",
		meta = (EditCondition = "ZoneType == EInnerGameZoneType::Checkpoint"))
	int32 CheckpointIndex = 0;

	UPROPERTY(BlueprintAssignable, Category = "Zone|Events")
	FOnZoneTriggeredSig OnZoneTriggered;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone|Components")
	TObjectPtr<UBoxComponent> TriggerBox;

protected:
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);
};
