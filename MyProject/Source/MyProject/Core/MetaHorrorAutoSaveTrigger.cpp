#include "MetaHorrorAutoSaveTrigger.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "HorrorGameInstance.h"
#include "Kismet/GameplayStatics.h"

AMetaHorrorAutoSaveTrigger::AMetaHorrorAutoSaveTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("AutoSaveBox"));
	RootComponent = Box;

	Box->SetBoxExtent(FVector(150.0f, 150.0f, 120.0f));
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AMetaHorrorAutoSaveTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (Box)
	{
		Box->OnComponentBeginOverlap.AddDynamic(this, &AMetaHorrorAutoSaveTrigger::OnBoxBeginOverlap);
	}
}

void AMetaHorrorAutoSaveTrigger::OnBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (bTriggerOnlyOnce && bHasTriggered)
	{
		return;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn || OtherActor != PlayerPawn)
	{
		return;
	}

	if (UHorrorGameInstance* HGI = Cast<UHorrorGameInstance>(GetGameInstance()))
	{
		bHasTriggered = true;
		HGI->AutoSaveGame(PlayerPawn);
	}
}
