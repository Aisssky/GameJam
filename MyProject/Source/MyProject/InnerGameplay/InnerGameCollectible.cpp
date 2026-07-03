#include "InnerGameCollectible.h"
#include "InnerGameCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AInnerGameCollectible::AInnerGameCollectible()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	RootComponent = TriggerSphere;
	TriggerSphere->SetSphereRadius(64.0f);
	TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	CollectibleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollectibleMesh"));
	CollectibleMesh->SetupAttachment(TriggerSphere);
	CollectibleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AInnerGameCollectible::BeginPlay()
{
	Super::BeginPlay();
	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AInnerGameCollectible::OnTriggerOverlap);
}

void AInnerGameCollectible::OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bCollected) return;
	if (!OtherActor || !OtherActor->IsA<AInnerGameCharacter>()) return;

	Collect(OtherActor);
}

void AInnerGameCollectible::Collect(AActor* Collector)
{
	if (bCollected) return;
	bCollected = true;

	BP_OnCollected(Collector);
	OnCollected.Broadcast(CollectibleID, Collector);

	if (bDestroyOnCollect)
	{
		Destroy();
	}
}
