#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MetaHorrorSaveGame.generated.h"

/**
 * 存档数据对象。
 * 对应策划案里的 BP_MetaHorrorSaveGame。
 *
 * 推荐做法：
 * - C++ 负责定义稳定数据结构。
 * - 蓝图 BP_MetaHorrorSaveGame 继承本类，UI 和关卡蓝图可直接读写这些变量。
 */
UCLASS(BlueprintType, Blueprintable)
class MYPROJECT_API UMetaHorrorSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UMetaHorrorSaveGame();

	/** 槽名称："PlayerSave" 或 "OriginalSave" */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Save Data|Meta")
	FString SlotName;

	/** UI 显示名称 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Save Data|Meta")
	FString DisplayName;

	/** 是否为旧存档 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Save Data|Meta")
	bool bIsPreExistingSave;

	/** 是否允许删除 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Save Data|Permission")
	bool bCanBeDeleted;

	/** 是否允许读取 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Save Data|Permission")
	bool bCanBeRead;

	/** 创建时间 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Save Data|Time")
	FDateTime CreatedAt;

	/** 最后保存时间 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Save Data|Time")
	FDateTime LastSavedAt;

	/** 累计游玩时间，单位：秒 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Save Data|Time")
	float TotalPlayTimeSeconds;

	/** 玩家位置/旋转/缩放 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Save Data|Player")
	FTransform PlayerTransform;

	/** 叙事标记：Name -> Bool */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Save Data|Narrative")
	TMap<FName, bool> NarrativeFlags;

	/** 通关进度：0~1 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, meta = (ClampMin = "0.0", ClampMax = "1.0"), Category = "Save Data|Progress")
	float GameCompletionPercent;

	/** 是否已通关 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Save Data|Progress")
	bool bHasCompletedGame;

	/** 灵魂分裂程度：0~1 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, meta = (ClampMin = "0.0", ClampMax = "1.0"), Category = "Save Data|Narrative")
	float SoulFragmentationLevel;

	/** 旧存档灵魂是否已释放 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Save Data|Narrative")
	bool bOriginalSoulReleased;

	/** 初始化为玩家存档默认值 */
	UFUNCTION(BlueprintCallable, Category = "Save Data")
	void InitAsPlayerSave(const FTransform& InitialPlayerTransform);

	/** 初始化为旧存档默认值；旧存档通关前不可读、不可删 */
	UFUNCTION(BlueprintCallable, Category = "Save Data")
	void InitAsOriginalSave();

	/** 根据是否通关，刷新旧存档权限 */
	UFUNCTION(BlueprintCallable, Category = "Save Data")
	void RefreshOriginalSavePermission(bool bPlayerHasCompletedGame);

	/** 写入当前保存时间 */
	UFUNCTION(BlueprintCallable, Category = "Save Data")
	void TouchLastSavedTime();
};
