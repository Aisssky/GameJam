# MyProject

UE 5.6 第一人称恐怖游戏。

## 技术栈

| 类别 | 技术 |
|------|------|
| 引擎 | UE 5.6.1 |
| 图形 | DX12 / Lumen / 光线追踪 / SM6 |
| 输入 | Enhanced Input |
| 武器 | Gameplay Ability System (GAS) |
| UI | UMG + C++ 基类 |
| AI | AIModule + NavigationSystem（待实现） |
| 存档 | 双槽位 + 叙事标记 |

---

## 层级结构

```
L1  主菜单        ■■■■■■■■□□  85%   UI C++ 完成 / BP 重设父类待做
L2  外层游戏      ■■■■■■□□□□  60%   玩家 + 武器 + 门接口 / 无敌人、无交互
L3  游戏机        ■■■■■■■□□□  70%   取出/收起状态机 / 动画同步待修
L4  内层游戏      ■■■■■□□□□□  50%   关卡流送 + SceneCapture / 玩法空白
L5  过场          □□□□□□□□□□   0%   枚举占位
```

---

## 系统进度

### 已完成 ✅

| 系统 | 说明 |
|------|------|
| 层状态机 | 5 层 (L1~L5) + 转换验证 + 委托广播 |
| 存档系统 | 双槽 (OriginalSave / PlayerSave)、创建/读取/删除、自动保存、权限控制 |
| 叙事标记 | SetFlag / HasFlag / GetAllFlags + 自动保存 |
| 主菜单 UI | UMainMenuWidget / USaveSlotButtonWidget / UConfirmDialogWidget C++ 基类 |
| UI 管理 | UHorrorUIComponent 管理 5 个 Widget 生命周期 + PushPanel 堆栈 |
| PlayerController | 4 层 IMC 切换、子状态管理 (NORMAL/MENU/INTERACT/STUNNED) |
| 玩家角色 | 第一人称、移动/观察、武器切换、疲劳减速 |
| 武器系统 (GAS) | 属性集、GameplayTags、开火/瞄准/放置护身符 |
| 控制台组件 | 取出/收起/强制收起、动画进度驱动 |
| 门接口 | BPI_Door (CanPlaceTalisman / PlaceTalisman / HasActiveTalisman) |
| L4 管理器 | 关卡流送、SceneCapture2D + RenderTarget |
| 自动保存触发器 | BoxComponent 重叠触发 AutoSave |

### 部分完成 ⚠️

| 系统 | 已完成 | 缺失 |
|------|--------|------|
| 主菜单 UI | C++ 全部逻辑 | 4 个 BP 待重设父类、旧 BP 逻辑待清理 |
| 存档渲染目标 | M_RenderTarget + RT_InnerGameplay | 首次呼出黑屏（需提前加载流送关卡） |
| 武器系统 | 逻辑完成 | 无视觉特效、音效、弹丸 |
| 控制台动画 | 状态机完成 | 动画与计时器解耦，可能不同步 |
| 疲劳系统 | SetSpeedMultiplier | 无视觉/音效反馈 |
| L4 管理器 | 关卡加载 | 生命周期回调无消费者 |

### 未开始 ❌

| 系统 | 备注 |
|------|------|
| **敌人 AI** | EEnemyAIState 已定义、AIModule 已链接、OnEnemyShot 已广播 — 零实现 |
| **交互系统** | IA_Interact 已就绪、BPI_Door 已声明、门模型已导入 — 零 C++ 实现 |
| 音效 | Content/Audio/ 空 |
| 死亡/重生 | 叙事结局完成，玩家死亡未实现 |
| 醒来逻辑 | OnWakeUp 委托悬空、SUBSTATE_STUNNED 孤立 |
| L5 过场 | 枚举占位 |
| 设置菜单 | BP 占位，无实际功能 |
| 物品/背包 | 设计未开始 |

---

## 文件结构

```
Source/MyProject/
├── Core/           GameInstance / 类型 / 存档 / 触发器
├── GameModes/      3 个 GameMode (Base / Menu / Outer)
├── Player/         PlayerController / PlayerCharacterB
├── UI/             UIComponent / MainMenu / SaveSlot / ConfirmDialog
├── Console/        ConsoleComponent
├── Weapon/         GAS 组件 / 属性集 / GameplayTags / 3 个能力
├── InnerGameplay/  L4 管理器
└── (Enemy/)        未创建
    (Interaction/)  未创建

Content/
├── Blueprints/
│   ├── Core/       5 (GI / GameMode × 2 / PlayerController / InnerManager)
│   ├── Player/     2 (Character / Controller)
│   ├── UI/         9 (完整 UI Widget 集)
│   ├── Enemy/      空
│   └── Interaction/空
├── Maps/           L_MainMenu / L_AGameplay / L_BGameplay / Lvl_FirstPerson
├── Input/          11 IA + 4 IMC
├── Characters/     角色、动画、材质
├── Interact/       门 BP + BPI_Interactable 接口
└── Audio/          空
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
| `OnNarrativeFlagSet` | GI | ✅ | — |
| `OnGameCompleted` | GI | ✅ | — |
| `OnWakeUp` | GI | — | **从未广播** |
| `OnEnemyShot` | GI | ✅ (GA_FireGun) | **无消费者** (无敌人 AI) |
| `OnTalismanPlacedOnDoor` | GI | ✅ (GA_PlaceTalisman) | BP 门 |
| `OnInnerGameStarted` | GI | ✅ | — |
| `OnInnerGameEnded` | GI | ✅ | — |

---

## 下一步

1. BP 重设父类，让 C++ 主菜单生效
2. 敌人 AI — `EEnemyAIState` + BehaviorTree + 感知系统
3. 交互系统 — 拾取/检查/开门
4. STUNNED 醒来逻辑 + OnWakeUp 广播
5. 音效系统
