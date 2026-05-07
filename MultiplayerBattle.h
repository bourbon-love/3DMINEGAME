
#ifndef MULTIPLAYER_BATTLE_H
#define MULTIPLAYER_BATTLE_H

#include "main.h"
#include "Box.h"



// BATTLE_MAP_SIZE_X / BATTLE_MAP_SIZE_Z defined in Box.h
const int SINGLE_MAP_SIZE = 15;
const int MAX_PLAYERS = 4;

// 玩家ID枚举
enum PlayerID {
    PLAYER_1 = 0,
    PLAYER_2 = 1,
    PLAYER_3 = 2,
    PLAYER_4 = 3
};

// 战斗状态枚举
enum BattleState {
    BATTLE_WAITING_PLAYERS,
    BATTLE_IN_PROGRESS,
    BATTLE_FINISHED
};

// 玩家信息结构
struct BattlePlayerInfo {
    bool isActive;
    char playerName[32];
    XMFLOAT4 playerColor;
    XMFLOAT3 spawnPosition;
    int health;
    int maxHealth;
    int score;
    bool isAlive;
    int movementPoints;        // 当前移动力
    int maxMovementPoints;     // 最大移动力
    bool isCurrentTurn;        // 是否为当前回合
    XMFLOAT2 gridPosition;     // 格子位置

    // 构造函数
    BattlePlayerInfo() {
        isActive = false;
        strcpy_s(playerName, "Unknown");
        playerColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        spawnPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
        health = 100;
        maxHealth = 100;
        score = 0;
        isAlive = true;
    }
};

// 多人战斗管理器类
class MultiplayerBattleManager {
private:
    static bool s_wasMousePressed;
    static bool PickBattleGridUnderMouse(int* outX, int* outZ);
    static bool RayIntersectsAABB(XMVECTOR rayOrigin, XMVECTOR rayDir,
        XMVECTOR boxMin, XMVECTOR boxMax, float* outDistance);
public:
    static void HandleMouseInput();
    //static void UpdatePlayerMovement(float deltaTime);
    static void HandleMouseClick(int mouseX, int mouseY);
    static XMFLOAT3 ScreenToWorldCoordinate(int screenX, int screenY);
    static void EndPlayerTurn();
    // 确保静态成员正确声明
    static BattlePlayerInfo s_players[MAX_PLAYERS];
    static BattleState s_currentState;
    static int s_activePlayerCount;
    static bool s_isBattleMode;

    // 核心功能
    static void Init();
    static void Uninit();
    static void Update(float deltaTime);

    // 玩家管理
    static void SetPlayerActive(PlayerID playerID, bool active, const char* name = nullptr);
    static void LoadPlayerMapFromSlot(PlayerID playerID, int mapSlot);
    static BattlePlayerInfo* GetPlayer(PlayerID playerID);

    // 战斗流程
    static void StartBattle();
    static void EndBattle();

    // 状态查询
    static BattleState GetBattleState() { return s_currentState; }
    static bool IsInitialized();

    //移动范围相关函数
    static void ShowCurrentPlayerMovementRange();
    static void HideMovementRange();
    static bool IsValidMoveTarget(int x, int z);
    static void MovePlayerTo(PlayerID playerID, int targetX, int targetZ);
    static void SetCurrentPlayer(PlayerID playerID);
    static PlayerID GetCurrentPlayer() { return s_currentPlayer; }
    static int GetActivePlayerCount() { return s_activePlayerCount; }
    static bool GetIsBattleMode() { return s_isBattleMode; }
private:

    static PlayerID s_currentPlayer;
    static bool s_isShowingMovement;

    static void SetupBattleCamera();
    static void InitializePlayerPositions();
    static void UpdateBattleLogic(float deltaTime);
};

// 外部接口函数声明
void InitMultiplayerBattle();
void UninitMultiplayerBattle();
void UpdateMultiplayerBattle(float deltaTime);

// 渲染相关函数
void DrawMultiplayerBattleScene();
void DrawMultiplayerBattleOverlay(ID3D11ShaderResourceView* ptexture);
void DrawAllPlayers();
void DrawBattleMinimap();
void DrawMapDividers(float minimapSize, float margin);
void DrawPlayerMarkers(float minimapSize, float margin);
void DrawEnhancedBattleMinimap();
void DrawPlayerRegionBorders(float centerX, float centerY, float mapSize);
void DrawPlayerPositions(float centerX, float centerY, float mapSize);
void DrawCurrentPlayerInfo();
void DrawMovementPointsBar(PlayerID playerID);
void DrawBattleControlHelp();

#endif // MULTIPLAYER_BATTLE_H