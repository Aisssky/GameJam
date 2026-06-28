#include "HorrorGameInstance.h"

#include "MetaHorrorSaveGame.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UHorrorGameInstance::UHorrorGameInstance()
{
	CurrentLayerState = ELayerState::STATE_MAIN_MENU;
	PreviousLayerState = ELayerState::STATE_MAIN_MENU;
	CinematicReturnLayer = ELayerState::STATE_MAIN_MENU;

	SaveUserIndex = 0;
	PlayerSaveSlotName = TEXT("PlayerSave");
	OriginalSaveSlotName = TEXT("OriginalSave");
	GameplayLevelName = TEXT("L_BGameplay");
	AutoSaveIntervalSeconds = 300.0f;

	CurrentSlotName = TEXT("");
	bOriginalSaveUnlocked = false;
}

void UHorrorGameInstance::Init()
{
	Super::Init();

	// 游戏启动时保证旧存档存在，满足“第一次打开有旧存档和一个空档位”。
	EnsureOriginalSaveExists();
	StartAutoSaveTimer();
}

void UHorrorGameInstance::Shutdown()
{
	StopAutoSaveTimer();
	Super::Shutdown();
}

bool UHorrorGameInstance::IsValidTransition(ELayerState From, ELayerState To) const
{
	if (From == To)
	{
		return true;
	}

	if (To == ELayerState::STATE_CINEMATIC)
	{
		return true;
	}

	if (From == ELayerState::STATE_CINEMATIC)
	{
		return true;
	}

	switch (From)
	{
	case ELayerState::STATE_MAIN_MENU:
		return To == ELayerState::STATE_OUTER_GAMEPLAY;

	case ELayerState::STATE_OUTER_GAMEPLAY:
		return To == ELayerState::STATE_MAIN_MENU
			|| To == ELayerState::STATE_CONSOLE_ACTIVE;

	case ELayerState::STATE_CONSOLE_ACTIVE:
		return To == ELayerState::STATE_OUTER_GAMEPLAY
			|| To == ELayerState::STATE_INNER_GAMEPLAY;

	case ELayerState::STATE_INNER_GAMEPLAY:
		return To == ELayerState::STATE_CONSOLE_ACTIVE;

	default:
		return false;
	}
}

bool UHorrorGameInstance::TransitionToLayer(ELayerState TargetLayer)
{
	if (!IsValidTransition(CurrentLayerState, TargetLayer))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[HorrorGameInstance] Invalid transition: %d -> %d"),
			static_cast<int32>(CurrentLayerState), static_cast<int32>(TargetLayer));
		return false;
	}

	ELayerState OldState = CurrentLayerState;
	PreviousLayerState = OldState;

	if (TargetLayer == ELayerState::STATE_CINEMATIC)
	{
		CinematicReturnLayer = (CurrentLayerState != ELayerState::STATE_CINEMATIC)
			? CurrentLayerState
			: CinematicReturnLayer;
	}

	CurrentLayerState = TargetLayer;

	UE_LOG(LogTemp, Log,
		TEXT("[HorrorGameInstance] Layer transition: %d -> %d"),
		static_cast<int32>(OldState), static_cast<int32>(TargetLayer));

	OnLayerChanged.Broadcast(OldState, TargetLayer);

	return true;
}

void UHorrorGameInstance::ForceConsolePutAway()
{
	if (CurrentLayerState == ELayerState::STATE_CONSOLE_ACTIVE
		|| CurrentLayerState == ELayerState::STATE_INNER_GAMEPLAY)
	{
		UE_LOG(LogTemp, Log, TEXT("[HorrorGameInstance] ForceConsolePutAway - script triggered"));
		OnForceConsolePutAway.Broadcast();
		TransitionToLayer(ELayerState::STATE_OUTER_GAMEPLAY);
		OnConsoleToggled.Broadcast(false);
	}
}

void UHorrorGameInstance::SetFlag(FName FlagName, bool bValue, bool bAutoSaveAfterSet)
{
	if (FlagName.IsNone())
	{
		return;
	}

	const bool* Existing = NarrativeFlags.Find(FlagName);
	const bool bAlreadyHadSameValue = Existing && (*Existing == bValue);

	NarrativeFlags.Add(FlagName, bValue);

	if (!bAlreadyHadSameValue)
	{
		UE_LOG(LogTemp, Log, TEXT("[HorrorGameInstance] Narrative flag set: %s = %s"),
			*FlagName.ToString(), bValue ? TEXT("true") : TEXT("false"));

		OnNarrativeFlagSet.Broadcast(FlagName);
	}

	if (FlagName == TEXT("NF_Ending_GameCompleted") && bValue)
	{
		SetGameCompleted(true);
		return;
	}

	// 关键叙事标记被设置时自动保存，但不在玩家还没创建 PlayerSave 时强行创建档位。
	if (bAutoSaveAfterSet && DoesSaveExist(PlayerSaveSlotName))
	{
		AutoSaveGame(GetDefaultPlayerActor());
	}
}

bool UHorrorGameInstance::HasFlag(FName FlagName) const
{
	const bool* Found = NarrativeFlags.Find(FlagName);
	return Found && *Found;
}

TMap<FName, bool> UHorrorGameInstance::GetAllFlags() const
{
	return NarrativeFlags;
}

void UHorrorGameInstance::EnsureOriginalSaveExists()
{
	// 结局里已经删除并释放旧存档后，不要在下一次刷新 UI 时又自动造回来。
	if (HasOriginalSaveBeenReleased())
	{
		return;
	}

	if (UGameplayStatics::DoesSaveGameExist(OriginalSaveSlotName, SaveUserIndex))
	{
		UpdateOriginalSavePermissionOnDisk();
		return;
	}

	UMetaHorrorSaveGame* OriginalSave = Cast<UMetaHorrorSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UMetaHorrorSaveGame::StaticClass())
	);

	if (!OriginalSave)
	{
		OnSaveFailed.Broadcast(OriginalSaveSlotName);
		return;
	}

	OriginalSave->InitAsOriginalSave();

	const bool bSaved = UGameplayStatics::SaveGameToSlot(OriginalSave, OriginalSaveSlotName, SaveUserIndex);
	if (!bSaved)
	{
		OnSaveFailed.Broadcast(OriginalSaveSlotName);
	}
}

TArray<FMetaHorrorSaveSlotInfo> UHorrorGameInstance::GetAllSaveSlotInfos()
{
	EnsureOriginalSaveExists();

	TArray<FMetaHorrorSaveSlotInfo> Result;
	Result.Add(BuildSlotInfo(0, OriginalSaveSlotName));
	Result.Add(BuildSlotInfo(1, PlayerSaveSlotName));
	return Result;
}

bool UHorrorGameInstance::DoesSaveExist(const FString& SlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, SaveUserIndex);
}

UMetaHorrorSaveGame* UHorrorGameInstance::LoadSaveObject(const FString& SlotName) const
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, SaveUserIndex))
	{
		return nullptr;
	}

	return Cast<UMetaHorrorSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, SaveUserIndex));
}

bool UHorrorGameInstance::CreatePlayerSave(const FTransform& InitialPlayerTransform)
{
	if (UGameplayStatics::DoesSaveGameExist(PlayerSaveSlotName, SaveUserIndex))
	{
		OnSaveOperationDenied.Broadcast(FText::FromString(TEXT("玩家存档已存在，不能重复创建。")));
		return false;
	}

	UMetaHorrorSaveGame* NewSave = Cast<UMetaHorrorSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UMetaHorrorSaveGame::StaticClass())
	);

	if (!NewSave)
	{
		OnSaveFailed.Broadcast(PlayerSaveSlotName);
		return false;
	}

	NewSave->InitAsPlayerSave(InitialPlayerTransform);

	const bool bSaved = UGameplayStatics::SaveGameToSlot(NewSave, PlayerSaveSlotName, SaveUserIndex);
	if (!bSaved)
	{
		OnSaveFailed.Broadcast(PlayerSaveSlotName);
		return false;
	}

	CurrentSaveGame = NewSave;
	CurrentSlotName = PlayerSaveSlotName;
	NarrativeFlags = NewSave->NarrativeFlags;

	if (const UWorld* World = GetWorld())
	{
		LastPlayTimeAccumulationWorldSeconds = World->GetTimeSeconds();
	}

	OnSaveCreated.Broadcast(PlayerSaveSlotName);
	return true;
}

bool UHorrorGameInstance::ManualSaveGame(AActor* PlayerActor)
{
	return SaveCurrentToSlot(PlayerSaveSlotName, PlayerActor, true);
}

bool UHorrorGameInstance::AutoSaveGame(AActor* PlayerActor)
{
	return SaveCurrentToSlot(PlayerSaveSlotName, PlayerActor, true);
}

bool UHorrorGameInstance::LoadGameFromManagedSlot(const FString& SlotName)
{
	EnsureOriginalSaveExists();

	UMetaHorrorSaveGame* LoadedSave = LoadSaveObject(SlotName);
	if (!LoadedSave)
	{
		OnSaveOperationDenied.Broadcast(FText::FromString(TEXT("该槽位没有可读取的存档。")));
		return false;
	}

	if (!LoadedSave->bCanBeRead)
	{
		OnSaveOperationDenied.Broadcast(FText::FromString(TEXT("旧存档尚未解锁，无法读取。")));
		return false;
	}

	CurrentSaveGame = LoadedSave;
	CurrentSlotName = SlotName;
	NarrativeFlags = LoadedSave->NarrativeFlags;
	bOriginalSaveUnlocked = IsPlayerSaveCompleted();

	if (const UWorld* World = GetWorld())
	{
		LastPlayTimeAccumulationWorldSeconds = World->GetTimeSeconds();
	}

	TransitionToLayer(ELayerState::STATE_OUTER_GAMEPLAY);
	OnGameLoaded.Broadcast(SlotName);

	if (!GameplayLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, GameplayLevelName);
	}

	return true;
}

bool UHorrorGameInstance::DeleteManagedSave(const FString& SlotName)
{
	EnsureOriginalSaveExists();

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, SaveUserIndex))
	{
		OnSaveOperationDenied.Broadcast(FText::FromString(TEXT("该槽位没有可删除的存档。")));
		return false;
	}

	UMetaHorrorSaveGame* SaveToDelete = LoadSaveObject(SlotName);
	if (!SaveToDelete)
	{
		OnSaveOperationDenied.Broadcast(FText::FromString(TEXT("存档读取失败，删除已取消。")));
		return false;
	}

	if (!SaveToDelete->bCanBeDeleted)
	{
		OnSaveOperationDenied.Broadcast(FText::FromString(TEXT("旧存档尚未解锁，无法删除。")));
		return false;
	}

	const bool bDeleted = UGameplayStatics::DeleteGameInSlot(SlotName, SaveUserIndex);
	if (!bDeleted)
	{
		OnSaveFailed.Broadcast(SlotName);
		return false;
	}

	OnSaveDeleted.Broadcast(SlotName);

	if (SlotName == PlayerSaveSlotName)
	{
		if (CurrentSlotName == PlayerSaveSlotName)
		{
			CurrentSaveGame = nullptr;
			CurrentSlotName = TEXT("");
			NarrativeFlags.Empty();
			bOriginalSaveUnlocked = false;
		}
	}
	else if (SlotName == OriginalSaveSlotName)
	{
		// 通关后删除旧存档：写回 PlayerSave，避免 EnsureOriginalSaveExists 再把旧存档生成回来。
		NarrativeFlags.Add(TEXT("NF_Ending_OriginalSaveDeleted"), true);
		MarkOriginalSoulReleasedInPlayerSave();
		OnGameCompleted.Broadcast();
	}

	return true;
}

void UHorrorGameInstance::SetGameCompleted(bool bCompleted)
{
	NarrativeFlags.Add(TEXT("NF_Ending_GameCompleted"), bCompleted);
	bOriginalSaveUnlocked = bCompleted;

	UMetaHorrorSaveGame* PlayerSave = CurrentSaveGame;
	if (!PlayerSave || PlayerSave->SlotName != PlayerSaveSlotName)
	{
		PlayerSave = LoadSaveObject(PlayerSaveSlotName);
	}

	if (PlayerSave)
	{
		PlayerSave->bHasCompletedGame = bCompleted;
		PlayerSave->GameCompletionPercent = bCompleted ? 1.0f : PlayerSave->GameCompletionPercent;
		PlayerSave->NarrativeFlags = NarrativeFlags;
		PlayerSave->TouchLastSavedTime();
		UGameplayStatics::SaveGameToSlot(PlayerSave, PlayerSaveSlotName, SaveUserIndex);

		CurrentSaveGame = PlayerSave;
		CurrentSlotName = PlayerSaveSlotName;
	}

	UpdateOriginalSavePermissionOnDisk();

	if (bCompleted)
	{
		OnGameCompleted.Broadcast();
	}
}

bool UHorrorGameInstance::ApplyLoadedPlayerTransform(AActor* PlayerActor) const
{
	if (!PlayerActor || !CurrentSaveGame)
	{
		return false;
	}

	PlayerActor->SetActorTransform(CurrentSaveGame->PlayerTransform);
	return true;
}

void UHorrorGameInstance::StartAutoSaveTimer()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	World->GetTimerManager().SetTimer(
		AutoSaveTimerHandle,
		this,
		&UHorrorGameInstance::HandleAutoSaveTimer,
		AutoSaveIntervalSeconds,
		true
	);
}

void UHorrorGameInstance::StopAutoSaveTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}
}

void UHorrorGameInstance::HandleAutoSaveTimer()
{
	// 没有 PlayerSave 时不自动创建，避免绕过“创建存档”弹窗流程。
	if (DoesSaveExist(PlayerSaveSlotName))
	{
		AutoSaveGame(GetDefaultPlayerActor());
	}
}

AActor* UHorrorGameInstance::GetDefaultPlayerActor() const
{
	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			return PC->GetPawn();
		}
	}

	return nullptr;
}

bool UHorrorGameInstance::SaveCurrentToSlot(const FString& SlotName, AActor* PlayerActor, bool bBroadcastSuccess)
{
	if (SlotName != PlayerSaveSlotName)
	{
		OnSaveOperationDenied.Broadcast(FText::FromString(TEXT("普通保存只能写入玩家存档。")));
		return false;
	}

	UMetaHorrorSaveGame* SaveGame = LoadSaveObject(PlayerSaveSlotName);
	if (!SaveGame)
	{
		OnSaveOperationDenied.Broadcast(FText::FromString(TEXT("玩家存档不存在，请先在存档界面创建存档。")));
		return false;
	}

	CaptureRuntimeDataToSave(SaveGame, PlayerActor);

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, PlayerSaveSlotName, SaveUserIndex);
	if (!bSaved)
	{
		OnSaveFailed.Broadcast(PlayerSaveSlotName);
		return false;
	}

	CurrentSaveGame = SaveGame;
	CurrentSlotName = PlayerSaveSlotName;

	if (bBroadcastSuccess)
	{
		OnGameSaved.Broadcast(PlayerSaveSlotName);
	}

	return true;
}

void UHorrorGameInstance::CaptureRuntimeDataToSave(UMetaHorrorSaveGame* SaveGame, AActor* PlayerActor)
{
	if (!SaveGame)
	{
		return;
	}

	SaveGame->SlotName = PlayerSaveSlotName;
	SaveGame->DisplayName = TEXT("你的存档");
	SaveGame->bIsPreExistingSave = false;
	SaveGame->bCanBeRead = true;
	SaveGame->bCanBeDeleted = true;
	SaveGame->TouchLastSavedTime();
	SaveGame->NarrativeFlags = NarrativeFlags;

	if (PlayerActor)
	{
		SaveGame->PlayerTransform = PlayerActor->GetActorTransform();
	}

	// 累计真实游玩时间：每次保存时，把“上次保存到现在”的世界时间差加进去。
	if (const UWorld* World = GetWorld())
	{
		const double Now = World->GetTimeSeconds();
		if (LastPlayTimeAccumulationWorldSeconds <= 0.0)
		{
			LastPlayTimeAccumulationWorldSeconds = Now;
		}

		const float DeltaPlaySeconds = FMath::Max(0.0f, static_cast<float>(Now - LastPlayTimeAccumulationWorldSeconds));
		SaveGame->TotalPlayTimeSeconds += DeltaPlaySeconds;
		LastPlayTimeAccumulationWorldSeconds = Now;
	}

	SaveGame->bHasCompletedGame = HasFlag(TEXT("NF_Ending_GameCompleted")) || SaveGame->bHasCompletedGame;
	if (SaveGame->bHasCompletedGame)
	{
		SaveGame->GameCompletionPercent = 1.0f;
	}
	else
	{
		SaveGame->GameCompletionPercent = FMath::Clamp(SaveGame->GameCompletionPercent, 0.0f, 1.0f);
	}

	SaveGame->SoulFragmentationLevel = FMath::Clamp(SaveGame->SoulFragmentationLevel, 0.0f, 1.0f);
}

void UHorrorGameInstance::UpdateOriginalSavePermissionOnDisk()
{
	if (HasOriginalSaveBeenReleased())
	{
		return;
	}

	UMetaHorrorSaveGame* OriginalSave = LoadSaveObject(OriginalSaveSlotName);
	if (!OriginalSave)
	{
		return;
	}

	const bool bCompleted = IsPlayerSaveCompleted();
	bOriginalSaveUnlocked = bCompleted;

	OriginalSave->RefreshOriginalSavePermission(bCompleted);
	OriginalSave->TouchLastSavedTime();

	UGameplayStatics::SaveGameToSlot(OriginalSave, OriginalSaveSlotName, SaveUserIndex);
}

bool UHorrorGameInstance::IsPlayerSaveCompleted() const
{
	if (CurrentSaveGame && CurrentSaveGame->SlotName == PlayerSaveSlotName)
	{
		return CurrentSaveGame->bHasCompletedGame || HasFlag(TEXT("NF_Ending_GameCompleted"));
	}

	UMetaHorrorSaveGame* PlayerSave = LoadSaveObject(PlayerSaveSlotName);
	if (!PlayerSave)
	{
		return false;
	}

	const bool* EndingFlag = PlayerSave->NarrativeFlags.Find(TEXT("NF_Ending_GameCompleted"));
	return PlayerSave->bHasCompletedGame || (EndingFlag && *EndingFlag);
}

bool UHorrorGameInstance::HasOriginalSaveBeenReleased() const
{
	const bool* RuntimeDeletedFlag = NarrativeFlags.Find(TEXT("NF_Ending_OriginalSaveDeleted"));
	if (RuntimeDeletedFlag && *RuntimeDeletedFlag)
	{
		return true;
	}

	UMetaHorrorSaveGame* PlayerSave = LoadSaveObject(PlayerSaveSlotName);
	if (!PlayerSave)
	{
		return false;
	}

	const bool* DeletedFlag = PlayerSave->NarrativeFlags.Find(TEXT("NF_Ending_OriginalSaveDeleted"));
	return PlayerSave->bOriginalSoulReleased || (DeletedFlag && *DeletedFlag);
}

void UHorrorGameInstance::MarkOriginalSoulReleasedInPlayerSave()
{
	UMetaHorrorSaveGame* PlayerSave = LoadSaveObject(PlayerSaveSlotName);
	if (!PlayerSave)
	{
		return;
	}

	PlayerSave->bOriginalSoulReleased = true;
	PlayerSave->NarrativeFlags.Add(TEXT("NF_Ending_OriginalSaveDeleted"), true);
	PlayerSave->TouchLastSavedTime();

	UGameplayStatics::SaveGameToSlot(PlayerSave, PlayerSaveSlotName, SaveUserIndex);

	CurrentSaveGame = PlayerSave;
	CurrentSlotName = PlayerSaveSlotName;
	NarrativeFlags = PlayerSave->NarrativeFlags;
}

FMetaHorrorSaveSlotInfo UHorrorGameInstance::BuildSlotInfo(int32 SlotIndex, const FString& SlotName) const
{
	FMetaHorrorSaveSlotInfo Info;
	Info.SlotIndex = SlotIndex;
	Info.SlotName = SlotName;

	const bool bExists = UGameplayStatics::DoesSaveGameExist(SlotName, SaveUserIndex);
	Info.bExists = bExists;

	if (!bExists)
	{
		if (SlotName == PlayerSaveSlotName)
		{
			Info.DisplayName = TEXT("空档位");
			Info.bCanCreate = true;
		}
		else if (HasOriginalSaveBeenReleased())
		{
			Info.DisplayName = TEXT("旧存档已删除");
			Info.bCanCreate = false;
		}
		else
		{
			Info.DisplayName = TEXT("旧存档");
			Info.bCanCreate = false;
		}

		Info.bCanRead = false;
		Info.bCanDelete = false;
		Info.bIsPreExistingSave = (SlotName == OriginalSaveSlotName);
		return Info;
	}

	UMetaHorrorSaveGame* SaveObject = LoadSaveObject(SlotName);
	if (!SaveObject)
	{
		Info.DisplayName = TEXT("损坏的存档");
		Info.bCanCreate = false;
		Info.bCanRead = false;
		Info.bCanDelete = false;
		return Info;
	}

	Info.DisplayName = SaveObject->DisplayName;
	Info.bCanCreate = false;
	Info.bCanRead = SaveObject->bCanBeRead;
	Info.bCanDelete = SaveObject->bCanBeDeleted;
	Info.bIsPreExistingSave = SaveObject->bIsPreExistingSave;
	Info.CreatedAt = SaveObject->CreatedAt;
	Info.LastSavedAt = SaveObject->LastSavedAt;
	Info.TotalPlayTimeSeconds = SaveObject->TotalPlayTimeSeconds;
	Info.GameCompletionPercent = SaveObject->GameCompletionPercent;

	return Info;
}
