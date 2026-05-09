#pragma once
#include "Main.h"
#include "CharactorScene.h"

// 战斗大厅相关的枚举和结构
enum BATTLE_HALL_STATE {
    HALL_WAITING_PLAYERS = 0,
    HALL_READY_TO_START,
    HALL_STARTING_BATTLE
};

// 玩家位置状态
enum PLAYER_SLOT_STATE {
    SLOT_EMPTY = 0,          // 空位
    SLOT_OCCUPIED_LOCAL,     // 本地玩家占用
    SLOT_OCCUPIED_REMOTE     // 远程玩家占用（为将来联机准备）
};

// 玩家位置信息
struct PlayerSlot {
    PLAYER_SLOT_STATE state;        // 位置状态
    CHARACTER_TYPE characterType;   // 角色类型
    bool isReady;                   // 是否准备好
    XMFLOAT3 position;              // 3D世界位置
    XMFLOAT3 characterPos;          // 角色显示位置
    XMFLOAT4 slotColor;             // 位置颜色
    float scaleAnimation;           // 缩放动画
    int playerID;                   // 玩家ID (1-4)
    char playerName[32];            // 玩家名称
};

// 战斗大厅管理器
class BattleHallManager {
public:
    // 初始化和清理
    static void Init();
    static void Uninit();

    // 更新和绘制
    static void Update(float deltaTime);
    static void Draw();

    // 场景控制
    static void ActivateBattleHall();
    static void DeactivateBattleHall();
    static bool IsActive() { return s_isActive; }

    // 玩家位置管理
    static bool SelectPlayerSlot(int slotIndex);           // 选择玩家位置
    static void SetPlayerCharacter(int slotIndex, CHARACTER_TYPE character); // 设置角色
    static void RemovePlayerFromSlot(int slotIndex);       // 移除玩家
    static bool IsSlotEmpty(int slotIndex);                // 检查位置是否为空

    // 状态管理
    static BATTLE_HALL_STATE GetHallState() { return s_hallState; }
    static bool CanStartBattle();                          // 是否可以开始战斗
    static void StartBattle();                            // 开始战斗

    // 当前玩家管理
    static void SetCurrentPlayerCharacter(CHARACTER_TYPE character);
    static CHARACTER_TYPE GetCurrentPlayerCharacter() { return s_currentPlayerCharacter; }
    static bool HasCurrentPlayerCharacter() { return s_hasSelectedCharacter; }

    // 鼠标交互
    static int GetSlotUnderMouse(int mouseX, int mouseY);
    // 新增：标记管理
    static void SetFromBattleHallFlag(bool flag);
    static bool IsFromBattleHall();
private:
    // 内部状态
    static bool s_isActive;
    static BATTLE_HALL_STATE s_hallState;
    static PlayerSlot s_playerSlots[4];
    static CHARACTER_TYPE s_currentPlayerCharacter;
    static bool s_hasSelectedCharacter;
    static int s_currentPlayerSlot;                        // 当前玩家占用的位置 (-1表示未占用)

    // 3D渲染相关
    static ID3D11Buffer* s_slotVertexBuffer;
    static ID3D11Buffer* s_slotIndexBuffer;
    static int s_slotIndexCount;

    // 内部函数
    static void CreateSlotGeometry();
    static void DrawPlayerSlot(const PlayerSlot& slot);
    static void DrawSlotUI();
    static void HandleMouseInput();
    static void HandaleKeyboardInput();
    static void UpdateAnimations(float deltaTime);
    static void InitializeSlotPositions();
    static void TogglePlayerReady();
    static bool AllPlayersReady();
    static bool SelectPlayerSlotByKey(int slotIndex);
    // 相机管理
    static void SetupBattleHallCamera();
    static void RestorePreviousCamera();
    static XMFLOAT3 s_previousCameraPos;
    static XMFLOAT3 s_previousCameraAt;
    static XMFLOAT3 s_previousCameraUp;

    // AI相关函数
    static void AutoFillAIPlayers();
    static void RemoveAIPlayers();
    static void ScheduleAutoStart(int frames);
    static void CancelAutoStart();

    // 延迟切换相关变量
    static bool s_pendingBattleStart;
    static int s_battleStartDelay;

};

// 外部函数声明
extern void InitBattleHall();
extern void UninitBattleHall();
extern void UpdateBattleHall(float deltaTime);
extern void DrawBattleHall();
extern bool IsBattleHallActive();
extern void ActivateBattleHall();
extern void DeactivateBattleHall();