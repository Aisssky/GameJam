# 项目进度统计
## 总体项目进度

```
总体            ■■■■■■■□□□  ~65%
```

---

## 层级结构

```
L1  主菜单        ■■■■■■■■■□  90%   C++ / BP 完整 / 存档 UI 就绪
L2  外层游戏      ■■■■■■■□□□  75%   玩家 + 武器 + 敌人AI + 暂停菜单 / 缺交互系统
L3  游戏机        ■■■■■■■□□□  75%   状态机 / SceneCapture+RT / 动画同步待修
L4  内层游戏      ■■■■■■□□□□  65%   角色移动+跳跃 / 固定侧视角 / 玩法空白
L5  过场          □□□□□□□□□□   0%   枚举占位
```

---

## 系统进度

### 已完成 ✅

| 系统 | 说明 |
|------|------|
| 层状态机 | 5 层 (L1~L5) + 转换验证 + 委托广播 |
| 存档系统 | 双槽 (OriginalSave / PlayerSave)、创建/读取/删除、自动保存、权限控制 |
| 叙事标记 | SetFlag / HasFlag / GetAllFlags + 自动保存 + **蓝图函数库 (HorrorNarrativeLibrary)** |
| 主菜单 UI | UMainMenuWidget / USaveSlotButtonWidget / UConfirmDialogWidget C++ 基类 + BP 绑定 |
| 暂停菜单 | **UPauseMenuWidget C++ 基类: 存档/读档/返回主菜单/返回游戏 + Controller 集成** |
| UI 管理 | UHorrorUIComponent 管理 6 个 Widget 生命周期 + PushPanel 堆栈 |
| PlayerController | 4 层 IMC 切换、子状态管理 (NORMAL/MENU/INTERACT/STUNNED)、暂停菜单输入 |
| 玩家角色 | 第一人称、移动/观察、武器切换、疲劳减速 |
| 武器系统 (GAS) | 属性集、GameplayTags、开火/瞄准/放置护身符 |
| 控制台组件 | 取出/收起/强制收起、动画进度驱动 |
| 门接口 | BPI_Door (CanPlaceTalisman / PlaceTalisman / HasActiveTalisman) |
| **敌人 AI 框架** | **HorrorEnemyPawn (ACharacter) + HorrorEnemyAIController + 9 个 BT 节点 + AI Perception (视觉+听觉) + 噪音库** |
| L4 管理器 | 关卡流送、SceneCapture2D + RenderTarget |
| L4 角色 | InnerGameCharacter: X 轴移动 / 跳跃 / SceneCapture 固定侧视角 |
| SceneCapture 渲染 | M_RenderTarget + RT_InnerGameplay、PIE 流送时序修复 |
| 自动保存触发器 | BoxComponent 重叠触发 AutoSave |

### 部分完成 ⚠️

| 系统 | 已完成 | 缺失 |
|------|--------|------|
| 武器系统 | 逻辑完成 | 无视觉特效、音效、弹丸 |
| 控制台动画 | 状态机完成 | 动画与计时器解耦，可能不同步 |
| 疲劳系统 | SetSpeedMultiplier | 无视觉/音效反馈 |
| 敌人 AI | 完整 C++ 框架 + 行为树节点 + AI 感知 | 行为树资产未配置、无 BP 实现、捕获流程缺死亡/重生联动 |
| L4 管理器 | 关卡加载、SceneCapture + RT | 生命周期回调消费者待扩展 |
| L4 玩法 | 角色基础移动+跳跃 | 无平台/障碍/收集品/胜负判定 |

### 未开始 ❌

| 系统 | 备注 |
|------|------|
| **交互系统** | IA_Interact 已就绪、BPI_Door 已声明、门模型已导入 — 零 C++ 实现 |
| **L4 玩法内容** | 平台/障碍/收集品/胜负判定均未实现 |
| **死亡/重生** | 叙事结局完成，玩家死亡/敌人捕获后的重生流程未实现 (BP 事件预留) |
| 醒来逻辑 | OnWakeUp 委托悬空、SUBSTATE_STUNNED 孤立 |
| L5 过场 | 枚举占位 |
| 物品/背包 | 设计未开始 |

---

## 文件结构

```
Source/MyProject/
├── Core/           GameInstance / 类型 / 存档 / 触发器 / 叙事库
├── GameModes/      3 个 GameMode (Base / Menu / Outer)
├── Player/         PlayerController / PlayerCharacterB
├── UI/             UIComponent / MainMenu / SaveSlot / ConfirmDialog / PauseMenu
├── Console/        ConsoleComponent
├── Weapon/         GAS 组件 / 属性集 / GameplayTags / 3 个能力
├── AI/             EnemyPawn / EnemyAIController / NoiseLibrary / 9 个 BT 节点
├── InnerGameplay/  L4 管理器 + L4 角色
└── (Interaction/)  未创建

MyProject/Docs/
├── EnemyAI_Setup_ZhenY.md       敌人 AI 搭建指南
└── README_EnemyAI_ZhenY.md      敌人 AI 系统文档 (191 行)

Content/
├── Blueprints/
│   ├── Core/       4 (GI / GameMode × 2 / InnerManager)
│   ├── Player/     3 (Controller / PlayerCharacter / InnerCharacter)
│   ├── UI/         11 (完整 UI Widget 集 + InnerHUD + PauseMenu)
│   ├── Enemy/      待配置 (行为树 + Blackboard + BP_EnemyPawn)
│   └── Interaction/空
├── Audio/
│   └── Enemy/Footsteps/  木地板走路.wav
├── Actors/
│   ├── gun/        武器模型 (材质 + 网格 + 贴图)
│   └── MicroSD/    MicroSD 卡模型
├── Maps/           L_MainMenu / L_AGameplay
├── Input/          14 IA + 5 IMC
├── Characters/     角色、动画、材质
└── Interact/       门 BP + BPI_Interactable 接口
```

---

## C++ 委托一览

| 委托 | 所在 | 广播 | 消费者 |
|------|------|:--:|--------|
| `OnLayerChanged` | GI | ✅ | UIComponent / PlayerController / Console |
| `OnConsoleToggled` | GI | ✅ | AbilitySystemComponent |
| `OnForceConsolePutAway` | GI | — | 已声明，未广播 |
| `OnSaveCreated` | GI | ✅ | MainMenu → RefreshSlots |
| `OnSaveDeleted` | GI | ✅ | MainMenu → RefreshSlots |
| `OnGameLoaded` | GI | ✅ | — |
| `OnGameSaved` | GI | ✅ | — |
| `OnSaveFailed` | GI | ✅ | — |
| `OnSaveOperationDenied` | GI | ✅ | MainMenu → 红字提示 |
| `OnNarrativeFlagSet` | GI | ✅ | **EnemyPawn → 行为树条件** |
| `OnGameCompleted` | GI | ✅ | — |
| `OnWakeUp` | GI | — | 从未广播 |
| `OnEnemyShot` | GI | ✅ (GA_FireGun) | **EnemyPawn → HandleEnemyShot** |
| `OnTalismanPlacedOnDoor` | GI | ✅ (GA_PlaceTalisman) | BP 门 |
| `OnInnerGameStarted` | GI | ✅ | InnerGameplayManager |
| `OnInnerGameEnded` | GI | ✅ | InnerGameplayManager |
| `OnL4Started` | InnerManager | ✅ | — |
| `OnL4Ended` | InnerManager | ✅ | — |
| `OnL4LevelLoaded` | InnerManager | ✅ | — |
| `OnL4LevelUnloaded` | InnerManager | ✅ | — |
| `OnAIStateChanged` | **EnemyPawn** | ✅ (SetEnemyAIState) | **蓝图 / 行为树** |
| `OnCaughtPlayer` | **EnemyPawn** | ✅ (BeginCaughtPlayerSequence) | **蓝图 (BP_OnCaughtPlayer)** |

---
