# 敌人 AI 提交说明 - 震Y

我提交了：

## 框架

`AI/HorrorEnemyPawn.h / .cpp`
敌人本体基类，蓝图 `BP_EnemyPawn` 继承这个类。

- 父类使用 `ACharacter`，自带 Capsule 和 CharacterMovement，方便走 NavMesh。
- 默认隐藏 Mesh，满足“敌人不可见，只靠 3D 音频提示位置”的需求。
- 支持 `ForceSetAIState(NewState)`，剧本或关卡蓝图可以强制切 AI 状态。
- 支持被玩家追上后的标准死亡/读档流程：
  - 播放贴脸惊吓音效
  - 触发雪花屏蓝图事件
  - 2 秒后调用存档里的 PlayerTransform 复活玩家
  - 不修改 SoulFragmentationLevel
- 支持银弹命中后的短暂停顿，已接 `UHorrorGameInstance::OnEnemyShot`。
- 支持游戏机 Console 激活时降低 Tick 频率。
- 支持自动 3D 脚步声，蓝图里给 `FootstepSound` 赋值即可。

`AI/HorrorEnemyAIController.h / .cpp`
敌人 AIController 基类，蓝图 `BP_EnemyAIController` 继承这个类。

- 自带 `AIPerceptionComponent`
- 自带视觉感知：
  - SightRadius = 1500
  - LoseSightRadius = 2000
  - PeripheralVisionAngleDegrees = 90
  - AutoSuccessRangeFromLastSeenLocation = 500
- 自带听觉感知：
  - HearingRange = 3000
  - LoSHearingRange = 5000
  - HearingMaxAge = 10
- 会把感知结果同步到黑板：
  - `TargetPlayer`
  - `LastKnownLocation`
  - `CurrentAIState`
  - `bHasLineOfSight`
  - `AlertLevel`
- 监听叙事 Flag：`OnNarrativeFlagSet`

它怎么知道追主角：

- AIController 只把 `UGameplayStatics::GetPlayerPawn(this, 0)` 当作主角目标。
- 看到主角时，会自动把黑板 `TargetPlayer` 设置为主角 Pawn。
- 看到主角时，会自动把 `bHasLineOfSight` 设置为 true，并把 `CurrentAIState` 切到 `Chasing`。
- 行为树 Chase 分支里的 `Move To TargetPlayer` 就会开始追主角。
- 听到主角制造的噪音时，会记录 `LastKnownLocation` 并进入 `Investigating`；如果调查过程中重新看见主角，会自动切回 `Chasing`。
- 队友一般不需要手动给 `TargetPlayer` 赋值，只要玩家是 0 号 PlayerPawn、敌人放进关卡、AIController 配好 BehaviorTree 即可。

`AI/HorrorEnemyNoiseLibrary.h / .cpp`
蓝图噪音上报工具。

- `Report Horror Noise`
- `Report Custom Horror Noise`
- 内置噪音类型：
  - `Noise_Footstep_Walk` 范围 500
  - `Noise_Footstep_Run` 范围 1200
  - `Noise_DoorOpen` 范围 2000
  - `Noise_ObjectDrop` 范围 1500

## 行为树节点

`AI/BTTask_UpdateLastKnownLocation.h / .cpp`
把目标玩家当前位置写入 `LastKnownLocation`。

`AI/BTTask_SetEnemyAIState.h / .cpp`
切换敌人 AI 状态：Idle / Investigating / Chasing / Stalking。

`AI/BTTask_GetNextPatrolPoint.h / .cpp`
从 `BP_EnemyPawn.PatrolPoints` 获取下一个巡逻点。

`AI/BTTask_FindStalkLocation.h / .cpp`
寻找一个靠近玩家但尽量避开玩家视线的尾随点。

`AI/BTTask_PlayChaseAudio.h / .cpp`
追逐时触发蓝图音频事件。

`AI/BTTask_TriggerAmbientEvent.h / .cpp`
尾随/恐怖制造时触发环境声或挪动物体事件。

## 行为树装饰器

`AI/BTDecorator_DistanceToTarget.h / .cpp`
判断敌人与目标玩家距离，比如 Stalk 分支用 `Distance > 800`。

`AI/BTDecorator_PlayerIsAware.h / .cpp`
判断玩家是否正看向敌人。Stalk 分支可以勾选 Inverse Condition，实现 `NOT PlayerIsAware`。

## 需要在 UE 里创建的蓝图资产

蓝图：

- `AI/BP_EnemyPawn` 继承 `AHorrorEnemyPawn`
- `AI/BP_EnemyAIController` 继承 `AHorrorEnemyAIController`
- `AI/BT_Enemy` 行为树
- `AI/BB_Enemy` 黑板

关卡里需要：

- 把 `BP_EnemyPawn` 放进 L2 主游戏关卡。
- 确保关卡有 `NavMeshBoundsVolume`，并覆盖敌人和玩家可行走区域。
- `BP_EnemyPawn` 的 `AI Controller Class` 设置为 `BP_EnemyAIController`。
- `BP_EnemyAIController` 的 `BehaviorTreeAsset` 设置为 `BT_Enemy`。

黑板 Key：

- `TargetPlayer` Object
- `LastKnownLocation` Vector
- `CurrentAIState` Enum，选择 `EEnemyAIState`
- `bHasLineOfSight` Bool
- `AlertLevel` Float
- `PatrolPoint` Object
- `StalkLocation` Vector

## 行为树结构建议

Root 使用 Selector，顺序：

1. Chase 追赶
   - Decorator：`bHasLineOfSight == true`
   - Task：`Set Enemy AI State`，NewState = Chasing
   - Task：`Update Last Known Location`
   - Task：UE 内置 `Move To`，Key = `TargetPlayer`
   - Task：`Play Chase Audio`
2. Investigate 调查
   - Decorator：`LastKnownLocation Is Set`
   - Task：`Set Enemy AI State`，NewState = Investigating
   - Task：UE 内置 `Move To`，Key = `LastKnownLocation`
   - Task：UE 内置 `Rotate to face BB entry`
   - Wait：3 到 8 秒
3. Stalk 尾随
   - Decorator：`Player Is Aware`，勾选 Inverse Condition
   - Decorator：`Distance To Target`，Comparison = Greater，Distance = 800
   - Task：`Set Enemy AI State`，NewState = Stalking
   - Task：`Find Stalk Location`
   - Task：UE 内置 `Move To`，Key = `StalkLocation`
   - Task：`Trigger Ambient Event`
4. Patrol / Idle
   - Task：`Set Enemy AI State`，NewState = Idle
   - Task：`Get Next Patrol Point`
   - Task：UE 内置 `Move To`，Key = `PatrolPoint`

## 音频素材

我把敌人木地板脚步声原始素材放在：

`Content/Audio/Enemy/Footsteps/木地板走路.wav`

建议音频同学或蓝图同学在 UE 里这样处理：

- 把这个 wav 切成多个短脚步片段，命名如：
  - `SW_Enemy_Footstep_Wood_01`
  - `SW_Enemy_Footstep_Wood_02`
  - `SW_Enemy_Footstep_Wood_03`
- 再做一个随机播放资源：
  - `SC_Enemy_Footsteps_Wood_Random` 或 `MS_EnemyFootsteps_Wood`
- 打开 `BP_EnemyPawn`，把 `FootstepSound` 设置成这个 SoundCue / MetaSound。

不建议直接把整段长 wav 填到 `FootstepSound`，否则每一步都会从长音频开头播放，不像真正脚步。

## 对接点

玩家脚步、开门、物体掉落蓝图里调用：

- `Report Horror Noise`
- `NoiseInstigator` 填玩家或发声 Actor
- `NoiseLocation` 填发声位置
- `NoiseType` 选对应类型

剧本强制控制敌人状态时调用：

- `BP_EnemyPawn.ForceSetAIState(NewState)`
- 或 `BP_EnemyAIController.ForceSetAIState(NewState)`

雪花屏 UI 接：

- `BP_EnemyPawn` 的 `BP_OnCaughtPlayer(PlayerPawn, ScreenDuration)`

追逐音效接：

- `BP_EnemyPawn` 的 `BP_OnChaseAudioRequested`

尾随环境事件接：

- `BP_EnemyPawn` 的 `BP_OnAmbientEventRequested`

脚步声如需蓝图自定义随机、混响或衰减，可接：

- `BP_EnemyPawn` 的 `BP_OnFootstepAudioRequested`
