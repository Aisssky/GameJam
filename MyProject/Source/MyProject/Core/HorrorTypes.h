#pragma once

#include "CoreMinimal.h"
#include "HorrorTypes.generated.h"

UENUM(BlueprintType)
enum class ELayerState : uint8
{
	STATE_MAIN_MENU        UMETA(DisplayName = "Main Menu (L1)"),
	STATE_OUTER_GAMEPLAY   UMETA(DisplayName = "Outer Gameplay (L2)"),
	STATE_CONSOLE_ACTIVE   UMETA(DisplayName = "Console Active (L3)"),
	STATE_INNER_GAMEPLAY   UMETA(DisplayName = "Inner Gameplay (L4)"),
	STATE_CINEMATIC        UMETA(DisplayName = "Cinematic (Block All Input)"),
};

UENUM(BlueprintType)
enum class EGameSubState : uint8
{
	SUBSTATE_NORMAL     UMETA(DisplayName = "Normal - Free movement & interaction"),
	SUBSTATE_MENU       UMETA(DisplayName = "Menu - ESC menu open (world paused)"),
	SUBSTATE_INTERACT   UMETA(DisplayName = "Interact - Deep interaction (puzzle, note, etc)"),
	SUBSTATE_STUNNED    UMETA(DisplayName = "Stunned - Caught by enemy, waking up"),
};

UENUM(BlueprintType)
enum class EConsoleState : uint8
{
	Hidden           UMETA(DisplayName = "Hidden"),
	AnimatingOut     UMETA(DisplayName = "Animating Out (taking out)"),
	Active           UMETA(DisplayName = "Active"),
	AnimatingAway    UMETA(DisplayName = "Animating Away (putting away)"),
};

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
	Idle           UMETA(DisplayName = "Idle"),
	Investigating  UMETA(DisplayName = "Investigating"),
	Chasing        UMETA(DisplayName = "Chasing"),
	Stalking       UMETA(DisplayName = "Stalking"),
};
