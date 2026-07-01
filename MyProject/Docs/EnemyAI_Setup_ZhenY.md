# Enemy AI setup for ZhenY

这份说明对应第 6 章“敌人 AI”。代码放在 `Source/MyProject/AI`，你在 UE5 里主要做蓝图资产和行为树连接。

## 1. 创建蓝图

1. 编译项目后，创建 `BP_EnemyPawn`，父类选择 `HorrorEnemyPawn`。
2. 创建 `BP_EnemyAIController`，父类选择 `HorrorEnemyAIController`。
3. 打开 `BP_EnemyPawn`：
   - `AI Controller Class` 设为 `BP_EnemyAIController`。
   - Mesh 保持空或不可见，C++ 已经默认隐藏 Mesh。
   - 配置 `CaughtScareSound`、`AmbientEventSound`。
   - 配置 `FootstepSound`，建议使用切好的脚步 SoundCue / MetaSound，不要直接用整段长 wav。
   - 需要巡逻就给 `PatrolPoints` 填场景里的 TargetPoint。
4. 打开 `BP_EnemyAIController`：
   - `SightRadius = 1500`
   - `LoseSightRadius = 2000`
   - `PeripheralVisionAngleDegrees = 90`
   - `AutoSuccessRangeFromLastSeenLocation = 500`
   - `HearingRange = 3000`
   - `LoSHearingRange = 5000`
   - `HearingMaxAge = 10`

## 2. 黑板 key

创建 `BB_Enemy`，至少放这些 key：

| Key | Type |
| --- | --- |
| `TargetPlayer` | Object, base class Actor |
| `LastKnownLocation` | Vector |
| `CurrentAIState` | Enum, 选择 `EEnemyAIState` |
| `bHasLineOfSight` | Bool |
| `AlertLevel` | Float |
| `PatrolPoint` | Object, base class Actor |
| `StalkLocation` | Vector |

把 `BB_Enemy` 绑定到 `BT_Enemy`，再把 `BT_Enemy` 填到 `BP_EnemyAIController.BehaviorTreeAsset`。

## 2.1 它怎么知道追主角

不用在关卡蓝图里手动找主角。`BP_EnemyAIController` 会把 `GetPlayerPawn(0)` 当作主角：

- 看见主角时，自动写黑板 `TargetPlayer = 主角 Pawn`
- 自动写 `bHasLineOfSight = true`
- 自动写 `LastKnownLocation = 主角当前位置`
- 自动把 `CurrentAIState` 切到 `Chasing`
- 行为树 Chase 分支的 `Move To TargetPlayer` 会追主角

听见主角噪音时，不会立刻追死主角，而是写 `LastKnownLocation` 并进入 `Investigating`。调查途中如果重新看见主角，再切到 `Chasing`。

关卡里要确认三件事：

- L2 关卡里放了 `BP_EnemyPawn`
- `BP_EnemyPawn.AI Controller Class = BP_EnemyAIController`
- 关卡有覆盖可走区域的 `NavMeshBoundsVolume`

## 3. 行为树建议接法

Root 用 Selector，顺序如下：

1. Chase
   - Decorator: Blackboard `bHasLineOfSight == true`
   - Task: `Set Enemy AI State`, NewState = Chasing
   - Task: `Update Last Known Location`
   - Task: UE 内置 `Move To`, Blackboard Key = `TargetPlayer`
   - Task: `Play Chase Audio`
2. Investigate
   - Decorator: Blackboard `LastKnownLocation Is Set`
   - Task: `Set Enemy AI State`, NewState = Investigating
   - Task: UE 内置 `Move To`, Blackboard Key = `LastKnownLocation`
   - Task: UE 内置 `Rotate to face BB entry`, Key = `LastKnownLocation`
   - Wait: 3 到 8 秒
3. Stalk
   - Decorator: `Player Is Aware`，勾选 Inverse Condition
   - Decorator: `Distance To Target`, Comparison = Greater, Distance = 800
   - Task: `Set Enemy AI State`, NewState = Stalking
   - Task: `Find Stalk Location`
   - Task: UE 内置 `Move To`, Blackboard Key = `StalkLocation`
   - Task: `Trigger Ambient Event`
4. Patrol / Idle
   - Task: `Set Enemy AI State`, NewState = Idle
   - Task: `Get Next Patrol Point`
   - Task: UE 内置 `Move To`, Blackboard Key = `PatrolPoint`

## 4. 噪音节点怎么用

代码里提供了蓝图节点 `Report Horror Noise`。

| 场景 | Noise Type | 范围 |
| --- | --- | --- |
| 玩家走路 | Footstep Walk | 500 |
| 玩家跑步 | Footstep Run | 1200 |
| 开门 | Door Open | 2000 |
| 物体掉落 | Object Drop | 1500 |

在玩家脚步、门、掉落物蓝图里调用：

- `WorldContextObject` 接 self
- `NoiseInstigator` 接玩家或发声 Actor
- `NoiseType` 选择对应类型
- `NoiseLocation` 可以填 Actor Location；如果填 0 且有 Instigator，代码会自动用 Instigator 位置

## 4.1 敌人脚步声素材

原始素材放在：

`Content/Audio/Enemy/Footsteps/木地板走路.wav`

建议在 UE 里切成多个短脚步 SoundWave，再做一个随机播放的 `SC_Enemy_Footsteps_Wood_Random` 或 `MS_EnemyFootsteps_Wood`，最后填到 `BP_EnemyPawn.FootstepSound`。

## 5. 被追上后的复活

`HorrorEnemyPawn` 在 Chasing 状态下会检查距离，进入 `CatchDistance` 后：

1. 播放 `CaughtScareSound`
2. 调用蓝图事件 `BP_OnCaughtPlayer(PlayerPawn, ScreenDuration)`，这里接雪花屏 UI
3. 等 `StaticScreenDuration`，默认 2 秒
4. 调用 `UHorrorGameInstance::ApplyLoadedPlayerTransform`，把玩家送回存档里的 `PlayerTransform`
5. 不改 `SoulFragmentationLevel`

所以不需要再做 `SUBSTATE_STUNNED` 或 `OnWakeUp`。
