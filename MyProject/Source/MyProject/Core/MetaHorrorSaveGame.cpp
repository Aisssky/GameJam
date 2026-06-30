#include "MetaHorrorSaveGame.h"

UMetaHorrorSaveGame::UMetaHorrorSaveGame()
{
	SlotName = TEXT("");
	DisplayName = TEXT("");
	bIsPreExistingSave = false;
	bCanBeDeleted = true;
	bCanBeRead = true;

	CreatedAt = FDateTime::Now();
	LastSavedAt = CreatedAt;
	TotalPlayTimeSeconds = 0.0f;
	PlayerTransform = FTransform::Identity;

	GameCompletionPercent = 0.0f;
	bHasCompletedGame = false;
	SoulFragmentationLevel = 0.0f;
	bOriginalSoulReleased = false;
}

void UMetaHorrorSaveGame::InitAsPlayerSave(const FTransform& InitialPlayerTransform)
{
	SlotName = TEXT("PlayerSave");
	DisplayName = TEXT("新建存档");
	bIsPreExistingSave = false;
	bCanBeDeleted = true;
	bCanBeRead = true;

	CreatedAt = FDateTime::Now();
	LastSavedAt = CreatedAt;
	TotalPlayTimeSeconds = 0.0f;
	PlayerTransform = InitialPlayerTransform;

	NarrativeFlags.Empty();
	NarrativeFlags.Add(TEXT("NF_Intro_PlayerSaveCreated"), true);

	GameCompletionPercent = 0.0f;
	bHasCompletedGame = false;
	SoulFragmentationLevel = 0.0f;
	bOriginalSoulReleased = false;
}

void UMetaHorrorSaveGame::InitAsOriginalSave()
{
	SlotName = TEXT("OriginalSave");
	DisplayName = TEXT("旧存档");
	bIsPreExistingSave = true;

	// 核心叙事规则：旧存档初始存在，但通关前不可读、不可删。
	bCanBeDeleted = false;
	bCanBeRead = false;

	CreatedAt = FDateTime::Now();
	LastSavedAt = CreatedAt;
	TotalPlayTimeSeconds = 0.0f;
	PlayerTransform = FTransform::Identity;

	NarrativeFlags.Empty();
	NarrativeFlags.Add(TEXT("NF_OriginalSave_PreExisting"), true);

	GameCompletionPercent = 0.0f;
	bHasCompletedGame = false;
	SoulFragmentationLevel = 0.0f;
	bOriginalSoulReleased = false;
}

void UMetaHorrorSaveGame::RefreshOriginalSavePermission(bool bPlayerHasCompletedGame)
{
	if (!bIsPreExistingSave)
	{
		return;
	}

	// 通关后，旧存档读取/删除全部解锁。
	bCanBeRead = bPlayerHasCompletedGame;
	bCanBeDeleted = bPlayerHasCompletedGame;
	bHasCompletedGame = bPlayerHasCompletedGame;
	GameCompletionPercent = bPlayerHasCompletedGame ? 1.0f : GameCompletionPercent;
}

void UMetaHorrorSaveGame::TouchLastSavedTime()
{
	LastSavedAt = FDateTime::Now();
}
