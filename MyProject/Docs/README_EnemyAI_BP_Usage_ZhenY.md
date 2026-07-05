# 敌人 AI 蓝图落地说明 - 震Y

我提交/完成了敌人 AI 的基础蓝图实现与关卡接入，目前可以在 `L_AGameplay` 中开始巡逻，并在看见玩家后进入追逐分支。

## 已完成内容

### 1. 黑板

已创建：

- `/Game/Blueprints/Enemy/BB_Enemy`

已配置 Key：

| Key | 类型 | 用途 |
| --- | --- | --- |
| `TargetPlayer` | Object | 当前追踪目标，通常是 PlayerPawn(0) |
| `LastKnownLocation` | Vector | 最后看到/听到玩家的位置 |
| `CurrentAIState` | Enum: `EEnemyAIState` | Idle / Investigating / Chasing / Stalking |
| `bHasLineOfSight` | Bool | 是否看见玩家 |
| `AlertLevel` | Float | 警觉度 |
| `PatrolPoint` | Object | 当前巡逻点 |
| `StalkLocation` | Vector | 尾随位置，后续 Stalk 分支使用 |

### 2. 行为树

已创建：

- `/Game/Blueprints/Enemy/BT_Enemy`

当前已完成两个核心分支：

```text
Root
└── Selector
    ├── Chase
    │   ├── Blackboard Based Condition: bHasLineOfSight Is Set
    │   ├── Set Enemy AI State = Chasing
    │   ├── Update Last Known Location
    │   ├── Move To = TargetPlayer
    │   └── Play Chase Audio
    │
    └── Patrol
        ├── Set Enemy AI State = Idle
        ├── Get Next Patrol Point
        └── Move To = PatrolPoint
```

当前版本重点保证基础闭环：巡逻、看见玩家、追逐玩家。

### 3. AIController 蓝图

已创建：

- `/Game/Blueprints/Enemy/BP_EnemyAIController`

已绑定：

- `BehaviorTreeAsset = BT_Enemy`
- `BlackboardAsset = BB_Enemy`

C++ 侧也加了兜底：即使关卡里的敌人暂时还是使用 `HorrorEnemyAIController` 基类，也会自动加载 `BT_Enemy / BB_Enemy`，避免行为树不启动。

### 4. Enemy Pawn 蓝图

已创建：

- `/Game/Blueprints/Enemy/BP_EnemyPawn`

说明：

- 父类是 `HorrorEnemyPawn`
- 支持 NavMesh 移动
- 支持 PatrolPoints 巡逻点数组
- 支持脚步声、追逐音效、被追上事件、死亡/读档流程接口
- 当前关卡里挂了敌人模型用于测试观察

### 5. 关卡接入

已在 `L_AGameplay` 中放置：

- `BP_EnemyPawn_C_1`
- `NavMeshBoundsVolume`
- 3 个巡逻点：
  - `TP_EnemyPatrol_01`
  - `TP_EnemyPatrol_02`
  - `TP_EnemyPatrol_03`

并且已把这 3 个巡逻点绑定到 `BP_EnemyPawn_C_1` 的 `Patrol Points` 数组中。

## 怎么测试

1. 打开 `L_AGameplay`
2. 按 `P` 查看 NavMesh，绿色区域需要覆盖玩家、敌人、巡逻点
3. 点击 Play
4. 敌人会先按 `Patrol Points` 巡逻
5. 玩家进入敌人视野后，AI Perception 会写入：
   - `TargetPlayer`
   - `bHasLineOfSight = true`
   - `CurrentAIState = Chasing`
6. 行为树进入 Chase 分支，`Move To = TargetPlayer`

## 队友使用方式

### 如果要在别的关卡放敌人

1. 拖入 `/Game/Blueprints/Enemy/BP_EnemyPawn`
2. 放置 `NavMeshBoundsVolume`
3. 放几个 `TargetPoint`
4. 选中敌人实例，在 Details 里找到 `Patrol Points`
5. 点 `+` 添加数组元素，把 TargetPoint 拖进去
6. 确认敌人实例：
   - `Auto Possess AI = Placed in World or Spawned`
   - `AI Controller Class` 推荐设为 `BP_EnemyAIController`

### 如果要改巡逻路线

直接移动 `TP_EnemyPatrol_01/02/03`，或者新增 TargetPoint 后加入敌人实例的 `Patrol Points` 数组。

### 如果要接死亡/重生 UI

在 `BP_EnemyPawn` 里重写/使用事件：

- `BP_OnCaughtPlayer(PlayerPawn, ScreenDuration)`

这里接雪花屏、死亡 UI、读档提示等。

### 如果要接追逐音效

在 `BP_EnemyPawn` 里重写/使用事件：

- `BP_OnChaseAudioRequested`

或者直接给 `FootstepSound / CaughtScareSound / AmbientEventSound` 填 SoundCue / MetaSound。

### 如果要让玩家脚步吸引敌人

在玩家走路、跑步、开门、丢物体逻辑里调用：

- `Report Horror Noise`

常用类型：

| 场景 | Noise Type |
| --- | --- |
| 走路 | Footstep Walk |
| 跑步 | Footstep Run |
| 开门 | Door Open |
| 物体掉落 | Object Drop |

## 常见问题

### 敌人完全不动

优先检查：

- 关卡里放的是 `BP_EnemyPawn`，不是单独的 Skeletal Mesh / Static Mesh
- 有 `NavMeshBoundsVolume`
- 按 `P` 能看到绿色 NavMesh 覆盖敌人和巡逻点
- 敌人实例的 `Patrol Points` 数组不是空的
- AI Controller 能运行 `BT_Enemy`

### 敌人巡逻但不追人

检查：

- 玩家是否是 `PlayerPawn(0)`
- 玩家是否进入 Sight Radius
- 行为树 Chase 分支条件是否是 `bHasLineOfSight`
- `Move To` 的 Blackboard Key 是否是 `TargetPlayer`

### 行为树不跑

检查：

- `BP_EnemyAIController.BehaviorTreeAsset = BT_Enemy`
- `BP_EnemyAIController.BlackboardAsset = BB_Enemy`
- `BP_EnemyPawn.AI Controller Class = BP_EnemyAIController`

目前 C++ 已有兜底加载 BT/BB，但推荐蓝图里也按上面设置好，方便队友调试。

## 当前未完成/后续可扩展

- Investigate 分支还可以继续补完整：听到声音后移动到 `LastKnownLocation`
- Stalk 分支还可以继续补完整：绕开玩家视线尾随
- 玩家脚步、开门、物体掉落的 `Report Horror Noise` 需要各系统接入
- 被追上后的雪花屏 UI 和正式读档 UI 需要 UI 同学接 `BP_OnCaughtPlayer`
