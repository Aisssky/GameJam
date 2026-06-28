#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "HorrorTypes.h"
#include "HorrorGameInstance.generated.h"

class AActor;
class UMetaHorrorSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLayerChanged, ELayerState, FromLayer, ELayerState, ToLayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConsoleToggled, bool, bConsoleActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnForceConsolePutAway);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCreated, const FString&, SlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveDeleted, const FString&, SlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLoaded, const FString&, SlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeFlagSet, FName, FlagName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWakeUp);

// 新增：存档成功/失败/拒绝，用于自动保存提示和按钮红亮+禁止音效。
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameSaved, const FString&, SlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveFailed, const FString&, SlotName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveOperationDenied, FText, Reason);

/**
 * 存档槽 UI 状态。
 * WBP_SaveSlotButton / WBP_MainMenu 刷新档位时直接读取这个结构体。
 */
USTRUCT(BlueprintType)
struct FMetaHorrorSaveSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Save Slot")
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Save Slot")
	FString SlotName;

	UPROPERTY(BlueprintReadOnly, Category = "Save Slot")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Save Slot")
	bool bExists = false;

	UPROPERTY(BlueprintReadOnly, Category = "Save Slot")
	bool bCanCreate = false;

	UPROPERTY(BlueprintReadOnly, Category = "Save Slot")
	bool bCanRead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Save Slot")
	bool bCanDelete = false;

	UPROPERTY(BlueprintReadOnly, Category = "Save Slot")
	bool bIsPreExistingSave = false;

	UPROPERTY(BlueprintReadOnly, Category = "Save Slot")
	FDateTime CreatedAt;

	UPROPERTY(BlueprintReadOnly, Category = "Save Slot")
	FDateTime LastSavedAt;

	UPROPERTY(BlueprintReadOnly, Category = "Save Slot")
	float TotalPlayTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Save Slot")
	float GameCompletionPercent = 0.0f;
};

/**
 * 项目全局 GameInstance。
 * 保留队友已有的层级状态机/游戏机事件逻辑，并合并震Y负责的存档系统与叙事标记保存。
 */
UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API UHorrorGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UHorrorGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	// =========================
	// 层级状态机：队友原有逻辑，保留
	// =========================

	UPROPERTY(BlueprintReadOnly, Category = "Layer State")
	ELayerState CurrentLayerState;

	UFUNCTION(BlueprintCallable, Category = "Layer State")
	bool TransitionToLayer(ELayerState TargetLayer);

	UFUNCTION(BlueprintPure, Category = "Layer State")
	bool IsValidTransition(ELayerState From, ELayerState To) const;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnLayerChanged OnLayerChanged;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnConsoleToggled OnConsoleToggled;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnForceConsolePutAway OnForceConsolePutAway;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnSaveCreated OnSaveCreated;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnSaveDeleted OnSaveDeleted;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnGameLoaded OnGameLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnNarrativeFlagSet OnNarrativeFlagSet;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnGameCompleted OnGameCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Event Dispatchers")
	FOnWakeUp OnWakeUp;

	// =========================
	// 存档系统新增事件
	// =========================

	/** 手动/自动保存成功。UI 可绑定后显示 2 秒淡出提示。 */
	UPROPERTY(BlueprintAssignable, Category = "Save System|Events")
	FOnGameSaved OnGameSaved;

	/** 保存失败。 */
	UPROPERTY(BlueprintAssignable, Category = "Save System|Events")
	FOnSaveFailed OnSaveFailed;

	/** 存档操作被拒绝。UI 可绑定后做按钮红亮 + 禁止音效。 */
	UPROPERTY(BlueprintAssignable, Category = "Save System|Events")
	FOnSaveOperationDenied OnSaveOperationDenied;

	// =========================
	// 叙事系统：保留原接口，增加自动保存参数
	// =========================

	/** 设置叙事标记；默认会在 PlayerSave 存在时自动保存。 */
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void SetFlag(FName FlagName, bool bValue = true, bool bAutoSaveAfterSet = true);

	UFUNCTION(BlueprintPure, Category = "Narrative")
	bool HasFlag(FName FlagName) const;

	/** 返回全部叙事标记，用于 UI / 存档 / 调试。 */
	UFUNCTION(BlueprintPure, Category = "Narrative")
	TMap<FName, bool> GetAllFlags() const;

	UFUNCTION(BlueprintCallable, Category = "Console")
	void ForceConsolePutAway();

	// =========================
	// 存档配置与运行时缓存
	// =========================

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Save System|Config")
	int32 SaveUserIndex = 0;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Save System|Config")
	FString PlayerSaveSlotName = TEXT("PlayerSave");

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Save System|Config")
	FString OriginalSaveSlotName = TEXT("OriginalSave");

	/** 读档后进入的 L2 主游戏关卡。 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Save System|Config")
	FName GameplayLevelName = TEXT("L_BGameplay");

	/** 定时自动保存间隔，默认 5 分钟。 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Save System|Config", meta = (ClampMin = "10.0"))
	float AutoSaveIntervalSeconds = 300.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Save System|Runtime")
	TObjectPtr<UMetaHorrorSaveGame> CurrentSaveGame;

	UPROPERTY(BlueprintReadOnly, Category = "Save System|Runtime")
	FString CurrentSlotName;

	/** 兼容队友原变量：通关后 OriginalSave 解锁。 */
	UPROPERTY(BlueprintReadOnly, Category = "Save System")
	bool bOriginalSaveUnlocked;

	/** 游戏启动时保证旧存档存在；如果旧存档已被结局删除，则不会重新生成。 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void EnsureOriginalSaveExists();

	/** 获取 OriginalSave / PlayerSave 两个固定槽的 UI 状态。 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	TArray<FMetaHorrorSaveSlotInfo> GetAllSaveSlotInfos();

	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool DoesSaveExist(const FString& SlotName) const;

	UFUNCTION(BlueprintCallable, Category = "Save System")
	UMetaHorrorSaveGame* LoadSaveObject(const FString& SlotName) const;

	/** L1 空档点击“是”后调用。 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool CreatePlayerSave(const FTransform& InitialPlayerTransform);

	/** ESC 菜单点击“保存游戏”时调用。 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool ManualSaveGame(AActor* PlayerActor);

	/** Trigger Volume / 叙事标记 / 定时器调用。 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool AutoSaveGame(AActor* PlayerActor);

	/** 读取 PlayerSave；OriginalSave 通关前会被拒绝。 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool LoadGameFromManagedSlot(const FString& SlotName);

	/** 删除存档；OriginalSave 未通关前会被拒绝。 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool DeleteManagedSave(const FString& SlotName);

	/** 设置通关状态，并解锁 OriginalSave 的读取/删除。 */
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void SetGameCompleted(bool bCompleted = true);

	/** L2 BeginPlay 时可调用，用当前读档恢复玩家位置。 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool ApplyLoadedPlayerTransform(AActor* PlayerActor) const;

	/** 启动/重启定时自动保存。建议 AOuterGameplayGameMode::BeginPlay 里也调用一次。 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void StartAutoSaveTimer();

	/** 停止定时自动保存。 */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void StopAutoSaveTimer();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Narrative")
	TMap<FName, bool> NarrativeFlags;

	UPROPERTY()
	ELayerState PreviousLayerState;

	UPROPERTY()
	ELayerState CinematicReturnLayer;

	FTimerHandle AutoSaveTimerHandle;

	/** 上次累计游玩时间时的世界时间，用来避免 TotalPlayTimeSeconds 只加一帧。 */
	double LastPlayTimeAccumulationWorldSeconds = 0.0;

	UFUNCTION()
	void HandleAutoSaveTimer();

	AActor* GetDefaultPlayerActor() const;

	bool SaveCurrentToSlot(const FString& SlotName, AActor* PlayerActor, bool bBroadcastSuccess);
	void CaptureRuntimeDataToSave(UMetaHorrorSaveGame* SaveGame, AActor* PlayerActor);
	void UpdateOriginalSavePermissionOnDisk();
	bool IsPlayerSaveCompleted() const;
	bool HasOriginalSaveBeenReleased() const;
	void MarkOriginalSoulReleasedInPlayerSave();
	FMetaHorrorSaveSlotInfo BuildSlotInfo(int32 SlotIndex, const FString& SlotName) const;
};
