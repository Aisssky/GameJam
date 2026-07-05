# 项目进度统计
## 总体项目进度

```
总体            ■■■■■■■■■□  ~89%
```

---

## 层级结构

```
L1  主菜单        ■■■■■■■■■□  90%   C++ / BP 完整 / 存档 UI 就绪
L2  外层游戏      ■■■■■■■■■□  92%   弹丸生成 / 音效 / 左手IK / Tag驱动显隐 / 第一人称移动修复 / 武器状态机 Blend Mask / 瞄准切换
L3  游戏机        ■■■■■■■■■□  90%   3D Actor 挂载 / NoCollision 修复 / 视角同步 / WidgetComponent
L4  内层游戏      ■■■■■■■■■□  90%   完整玩法循环: 收集/平台/区域/胜负/重生
L5  过场          □□□□□□□□□□   0%   枚举占位
```

---

## 系统进度

### 已完成 ✅

| 系统 | 说明 |
|------|------|
| 层状态机 | 5 层 (L1~L5) + 转换验证 + 委托广播 |
| 存档系统 | 双槽 (OriginalSave / PlayerSave)、创建/读取/删除、自动保存、权限控制 |
| 叙事标记 | SetFlag / HasFlag / GetAllFlags + 自动保存 + 蓝图函数库 (HorrorNarrativeLibrary) |
| 主菜单 UI | UMainMenuWidget / SaveSlotButtonWidget / ConfirmDialogWidget C++ 基类 + BP 绑定 |
| 暂停菜单 | UPauseMenuWidget C++ 基类: 存档/读档/返回主菜单/返回游戏 + Controller 集成 |
| UI 管理 | UHorrorUIComponent 管理 Widget 生命周期 + PushPanel 堆栈 |
| PlayerController | 4 层 IMC 切换、子状态管理 (NORMAL/MENU/INTERACT/STUNNED)、暂停菜单输入 |
| 玩家角色 | 第一人称、移动/观察、武器切换、疲劳减速 |
| 武器系统 (GAS) | 属性集、GameplayTags、开火/瞄准/放置护身符、GunMesh 组件挂 hand_r + Tag 回调自动显隐、GA 蓝图配置、IA 映射、FireMontage 播放、弹丸生成 (ProjectileMovementComponent)、开枪音效 (FireSound)、装备音效 (GunEquipSound)、左手IK (GetLeftHandIKTarget) |
| 控制台组件 | 取出/收起/强制收起、3D Actor 生成/销毁、骨骼挂载、NoCollision 修复、自由视角 |
| ABP_CharacterB | WeaponState 状态机: Unarmed → HoldGun → Aiming，Blend Mask 上身混合，Tag 驱动 |
| 游戏机 BP | WidgetComponent 渲染 UI、AnimBP Blend by bIsHoldingConsole |
| 门交互 | BPI_Door C++ 接口 + Door_Father BP (护身符放置/检测) |
| 敌人 AI 框架 | HorrorEnemyPawn + AIController + 9 个 BT 节点 + AI Perception + 噪音库 |
| L4 管理器 | 关卡流送、SceneCapture2D + RenderTarget、胜负判定、叙事 Flag 集成 |
| L4 角色 | X 轴移动/跳跃、SceneCapture 固定侧视角、生命/死亡/无敌、检查点、收集物 |
| L4 玩法系统 | 移动平台 (往返/循环) + 收集物 + 胜利区/死亡区/检查点 + 胜负条件 |
| 对话/任务 UI | DialogueLineWidget + TaskPromptWidget + Controller API + DataTable 配表 (ShowDialogueByID / ShowTaskPromptByID) |
| SceneCapture 渲染 | M_RenderTarget + RT_InnerGameplay、PIE 流送时序修复 |
| 自动保存触发器 | BoxComponent 重叠触发 AutoSave |

### 部分完成 ⚠️

| 系统 | 已完成 | 缺失 |
|------|--------|------|
| 武器系统 | 逻辑+弹丸+音效+IK 完成 | 命中反馈特效待完善 |
| 敌人生成/管理 | C++ 框架 + AI 感知 | BP 待实现 |
| 疲劳系统 | SetSpeedMultiplier | 无视觉/音效反馈 |
| 敌人 AI | 完整 C++ 框架 + 行为树节点 + AI 感知 | 行为树资产未配置、无 BP 实现、捕获流程缺死亡/重生联动 |
| 对话/任务 UI | C++ Widget 基类 + Controller API + DataTable 配表 | BP Widget 布局待完善 |
| 交互系统 | 门 (Door_Father BP + BPI_Door C++ 接口: 护身符放置/检测) | 拾取/检查等其他交互对象待扩展 (纯蓝图) |

### 未开始 ❌

| 系统 | 备注 |
|------|------|
| 醒来逻辑 | OnWakeUp 委托悬空、SUBSTATE_STUNNED 孤立 |
| L5 过场 | 枚举占位 |

---

## 文件结构

```
Source/MyProject/
├── Core/           1343 行  GameInstance / 类型 / 存档 / 触发器 / 叙事库
├── GameModes/       259 行  3 个 GameMode (Base / Menu / Outer)
├── Player/          965 行  PlayerController / PlayerCharacterB / 相机移动 / 左手IK
├── UI/             1634 行  UIComponent / MainMenu / SaveSlot / ConfirmDialog / PauseMenu / Dialogue / TaskPrompt / DialogueData
├── Console/         218 行  ConsoleComponent (3D Actor 生成/销毁/骨骼挂载/碰撞修复)
├── Weapon/         1161 行  GAS 组件 / 属性集 / GameplayTags / 3 个能力 / 弹丸生成 / 音效
├── AI/             1822 行  EnemyPawn / EnemyAIController / NoiseLibrary / 9 个 BT 节点
└── InnerGameplay/   986 行  Manager + Character + Collectible + Platform + Zone (5 类)

总计: ~8400 行 C++ / 79 个源文件

Content/Blueprints/
├── Core/         4 (GI / GameMode x2 / InnerManager)
├── Player/       3 (Controller / PlayerCharacter / InnerCharacter)
├── UI/          12 (MenuFolder x10 + Dialogue x2)
├── Console/      3 (BP_GameConsole + SK_Sequence + SK_Montage)
├── ConsoleGun/   1 (BP_Bullet)
├── Interaction/  6 (CheckPoint / Collectible / DeathZone / Game / Platform / WinZone)
└── Enemy/        待配置 (行为树 + Blackboard + BP_EnemyPawn)

Content/ 其他资产:
├── Audio/        木地板脚步声
├── Actors/       gun / MicroSD / game_console (→ Blueprints/Console/)
├── Maps/         L_MainMenu / L_AGameplay (含外部 Actor)
├── Input/        14 IA + 5 IMC
├── Characters/   骨骼网格 / 动画 / 材质 (Unarmed + Rifle + Pistol)
└── Interact/     门 BP + 跳板 + 目标靶
```

---

## C++ 委托一览

| 委托 | 所在 | 广播 | 消费者 |
|------|------|:--:|--------|
| `OnLayerChanged` | GI | ✅ | UIComponent / PlayerController / Console |
| `OnConsoleToggled` | GI | ✅ | AbilitySystemComponent |
| `OnForceConsolePutAway` | GI | — | 已声明，未广播 |
| `OnSaveCreated` | GI | ✅ | MainMenu -> RefreshSlots |
| `OnSaveDeleted` | GI | ✅ | MainMenu -> RefreshSlots |
| `OnGameLoaded` | GI | ✅ | — |
| `OnGameSaved` | GI | ✅ | — |
| `OnSaveFailed` | GI | ✅ | — |
| `OnSaveOperationDenied` | GI | ✅ | MainMenu -> 红字提示 |
| `OnNarrativeFlagSet` | GI | ✅ | EnemyPawn -> 行为树条件 |
| `OnGameCompleted` | GI | ✅ | — |
| `OnWakeUp` | GI | — | 从未广播 |
| `OnEnemyShot` | GI | ✅ (GA_FireGun) | EnemyPawn -> HandleEnemyShot |
| `OnTalismanPlacedOnDoor` | GI | ✅ (GA_PlaceTalisman) | BP 门 |
| `OnInnerGameStarted` | GI | ✅ | InnerGameplayManager |
| `OnInnerGameEnded` | GI | ✅ | InnerGameplayManager |
| `OnL4Started` | InnerManager | ✅ | — |
| `OnL4Ended` | InnerManager | ✅ | — |
| `OnL4LevelLoaded` | InnerManager | ✅ | — |
| `OnL4LevelUnloaded` | InnerManager | ✅ | — |
| `OnGameWon` | InnerManager | ✅ | — (蓝图层可绑定 UI 表现) |
| `OnGameLost` | InnerManager | ✅ | — (蓝图层可绑定 UI 表现) |
| `OnAIStateChanged` | EnemyPawn | ✅ (SetEnemyAIState) | 蓝图 / 行为树 |
| `OnCaughtPlayer` | EnemyPawn | ✅ (BeginCaughtPlayerSequence) | 蓝图 (BP_OnCaughtPlayer) |
| `OnCharacterDied` | InnerGameCharacter | ✅ | InnerGameplayManager -> HandleCharacterDied |
| `OnCollectibleAdded` | InnerGameCharacter | ✅ | — (蓝图层可绑定 UI/音效) |
| `OnCheckpointSet` | InnerGameCharacter | ✅ | — (蓝图层可绑定 UI/音效) |
