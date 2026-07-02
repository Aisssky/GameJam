#include "HorrorEnemyAIController.h"

#include "HorrorEnemyPawn.h"
#include "../Core/HorrorGameInstance.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

AHorrorEnemyAIController::AHorrorEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AI Sight Config"));
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("AI Hearing Config"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

	BrainComponent = BehaviorTreeComponent;
	SetPerceptionComponent(*AIPerceptionComponent);
	ConfigurePerception();
	SetActorTickInterval(NormalTickInterval);
}

void AHorrorEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	ConfigurePerception();

	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(
			this,
			&AHorrorEnemyAIController::HandleTargetPerceptionUpdated);
	}

	if (UHorrorGameInstance* GI = Cast<UHorrorGameInstance>(GetGameInstance()))
	{
		GI->OnNarrativeFlagSet.AddDynamic(this, &AHorrorEnemyAIController::HandleNarrativeFlagSet);
		GI->OnConsoleToggled.AddDynamic(this, &AHorrorEnemyAIController::HandleConsoleToggled);
	}
}

void AHorrorEnemyAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UHorrorGameInstance* GI = Cast<UHorrorGameInstance>(GetGameInstance()))
	{
		GI->OnNarrativeFlagSet.RemoveDynamic(this, &AHorrorEnemyAIController::HandleNarrativeFlagSet);
		GI->OnConsoleToggled.RemoveDynamic(this, &AHorrorEnemyAIController::HandleConsoleToggled);
	}

	Super::EndPlay(EndPlayReason);
}

void AHorrorEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	InitializeBehaviorTree();
	SetBlackboardDefaults();

	if (AHorrorEnemyPawn* EnemyPawn = Cast<AHorrorEnemyPawn>(InPawn))
	{
		EnemyPawn->ForceSetAIState(CurrentAIState);
	}
}

void AHorrorEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateLineOfSight();
}

void AHorrorEnemyAIController::ForceSetAIState(EEnemyAIState NewState)
{
	CurrentAIState = NewState;
	SetBlackboardState(NewState);

	if (AHorrorEnemyPawn* EnemyPawn = GetEnemyPawn())
	{
		EnemyPawn->ForceSetAIState(NewState);
	}
}

void AHorrorEnemyAIController::SetTargetPlayer(AActor* TargetActor, bool bHasLineOfSight)
{
	if (!TargetActor)
	{
		ClearTargetPlayer();
		return;
	}

	if (UBlackboardComponent* BB = GetEnemyBlackboard())
	{
		BB->SetValueAsObject(TargetPlayerKeyName, TargetActor);
		BB->SetValueAsVector(LastKnownLocationKeyName, TargetActor->GetActorLocation());
		BB->SetValueAsBool(HasLineOfSightKeyName, bHasLineOfSight);
		BB->SetValueAsFloat(AlertLevelKeyName, bHasLineOfSight ? 1.0f : 0.65f);
	}

	if (AHorrorEnemyPawn* EnemyPawn = GetEnemyPawn())
	{
		EnemyPawn->SetTargetPlayer(TargetActor);
	}

	if (bHasLineOfSight)
	{
		ForceSetAIState(EEnemyAIState::Chasing);
		BP_OnTargetSeen(TargetActor);
	}
}

void AHorrorEnemyAIController::ClearTargetPlayer()
{
	if (UBlackboardComponent* BB = GetEnemyBlackboard())
	{
		BB->ClearValue(TargetPlayerKeyName);
		BB->SetValueAsBool(HasLineOfSightKeyName, false);
		BB->SetValueAsFloat(AlertLevelKeyName, 0.0f);
	}

	if (AHorrorEnemyPawn* EnemyPawn = GetEnemyPawn())
	{
		EnemyPawn->SetTargetPlayer(nullptr);
	}

	ForceSetAIState(EEnemyAIState::Idle);
}

void AHorrorEnemyAIController::SetLastKnownLocation(FVector Location, float AlertLevel, bool bFromHearing)
{
	if (UBlackboardComponent* BB = GetEnemyBlackboard())
	{
		BB->SetValueAsVector(LastKnownLocationKeyName, Location);
		BB->SetValueAsFloat(AlertLevelKeyName, FMath::Clamp(AlertLevel, 0.0f, 1.0f));
		if (bFromHearing)
		{
			BB->SetValueAsBool(HasLineOfSightKeyName, false);
		}
	}

	if (CurrentAIState != EEnemyAIState::Chasing)
	{
		ForceSetAIState(EEnemyAIState::Investigating);
	}
}

void AHorrorEnemyAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !IsLikelyPlayer(Actor))
	{
		return;
	}

	const bool bIsNoiseStimulus = !Stimulus.Tag.IsNone();

	if (Stimulus.WasSuccessfullySensed())
	{
		if (bIsNoiseStimulus)
		{
			const FVector NoiseLocation = Stimulus.StimulusLocation.IsNearlyZero()
				? Actor->GetActorLocation()
				: Stimulus.StimulusLocation;

			SetTargetPlayer(Actor, false);
			SetLastKnownLocation(NoiseLocation, 0.55f, true);
			BP_OnNoiseHeard(NoiseLocation, Stimulus.Tag);
			return;
		}

		SetTargetPlayer(Actor, true);
		return;
	}

	if (!bIsNoiseStimulus)
	{
		if (UBlackboardComponent* BB = GetEnemyBlackboard())
		{
			BB->SetValueAsBool(HasLineOfSightKeyName, false);
			BB->SetValueAsVector(LastKnownLocationKeyName, Actor->GetActorLocation());
			BB->SetValueAsFloat(AlertLevelKeyName, 0.7f);
		}

		if (CurrentAIState == EEnemyAIState::Chasing)
		{
			ForceSetAIState(EEnemyAIState::Investigating);
		}

		BP_OnTargetLost(Actor);
	}
}

void AHorrorEnemyAIController::HandleNarrativeFlagSet(FName FlagName)
{
	BP_OnNarrativeFlagSet(FlagName);
}

void AHorrorEnemyAIController::HandleConsoleToggled(bool bConsoleActive)
{
	SetActorTickInterval(bConsoleActive ? ConsoleActiveTickInterval : NormalTickInterval);

	if (AHorrorEnemyPawn* EnemyPawn = GetEnemyPawn())
	{
		EnemyPawn->ApplyConsoleTickMode(bConsoleActive);
	}
}

void AHorrorEnemyAIController::ConfigurePerception()
{
	if (SightConfig)
	{
		SightConfig->SightRadius = SightRadius;
		SightConfig->LoseSightRadius = LoseSightRadius;
		SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
		SightConfig->AutoSuccessRangeFromLastSeenLocation = AutoSuccessRangeFromLastSeenLocation;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	}

	if (HearingConfig)
	{
		HearingConfig->HearingRange = HearingRange;
		HearingConfig->LoSHearingRange = LoSHearingRange;
		HearingConfig->bUseLoSHearing = true;
		HearingConfig->SetMaxAge(HearingMaxAge);
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	}

	if (AIPerceptionComponent)
	{
		if (SightConfig)
		{
			AIPerceptionComponent->ConfigureSense(*SightConfig);
			AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
		}

		if (HearingConfig)
		{
			AIPerceptionComponent->ConfigureSense(*HearingConfig);
		}
	}
}

void AHorrorEnemyAIController::InitializeBehaviorTree()
{
	UBlackboardData* BlackboardToUse = BlackboardAsset;
	if (!BlackboardToUse && BehaviorTreeAsset)
	{
		BlackboardToUse = BehaviorTreeAsset->BlackboardAsset;
	}

	if (BlackboardToUse)
	{
		UBlackboardComponent* BlackboardRaw = BlackboardComponent.Get();
		UseBlackboard(BlackboardToUse, BlackboardRaw);
		BlackboardComponent = BlackboardRaw;
	}

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

bool AHorrorEnemyAIController::IsLikelyPlayer(AActor* Actor) const
{
	return Actor && Actor == UGameplayStatics::GetPlayerPawn(this, 0);
}

AHorrorEnemyPawn* AHorrorEnemyAIController::GetEnemyPawn() const
{
	return Cast<AHorrorEnemyPawn>(GetPawn());
}

UBlackboardComponent* AHorrorEnemyAIController::GetEnemyBlackboard()
{
	if (BlackboardComponent)
	{
		return BlackboardComponent;
	}

	return GetBlackboardComponent();
}

void AHorrorEnemyAIController::SetBlackboardDefaults()
{
	if (UBlackboardComponent* BB = GetEnemyBlackboard())
	{
		BB->SetValueAsBool(HasLineOfSightKeyName, false);
		BB->SetValueAsFloat(AlertLevelKeyName, 0.0f);
		SetBlackboardState(CurrentAIState);
	}
}

void AHorrorEnemyAIController::UpdateLineOfSight()
{
	UBlackboardComponent* BB = GetEnemyBlackboard();
	if (!BB)
	{
		return;
	}

	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetPlayerKeyName));
	if (!TargetActor)
	{
		return;
	}

	const bool bHadLineOfSight = BB->GetValueAsBool(HasLineOfSightKeyName);
	const bool bHasLineOfSight = LineOfSightTo(TargetActor);
	BB->SetValueAsBool(HasLineOfSightKeyName, bHasLineOfSight);

	if (bHasLineOfSight)
	{
		BB->SetValueAsVector(LastKnownLocationKeyName, TargetActor->GetActorLocation());
		BB->SetValueAsFloat(AlertLevelKeyName, 1.0f);

		if (CurrentAIState != EEnemyAIState::Chasing)
		{
			ForceSetAIState(EEnemyAIState::Chasing);
		}
	}
	else if (bHadLineOfSight && CurrentAIState == EEnemyAIState::Chasing)
	{
		BB->SetValueAsVector(LastKnownLocationKeyName, TargetActor->GetActorLocation());
		BB->SetValueAsFloat(AlertLevelKeyName, 0.7f);
		ForceSetAIState(EEnemyAIState::Investigating);
		BP_OnTargetLost(TargetActor);
	}
}

void AHorrorEnemyAIController::SetBlackboardState(EEnemyAIState NewState)
{
	if (UBlackboardComponent* BB = GetEnemyBlackboard())
	{
		BB->SetValueAsEnum(CurrentAIStateKeyName, static_cast<uint8>(NewState));
	}
}
