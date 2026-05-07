// MultiplayerBattle.cpp
// 专注于多人战斗核心功能的实现

#include "MultiplayerBattle.h"
#include "Camera.h"
#include "UIManager.h"
#include "MapEditor.h"
#include "Box.h"
#include "SceneManager.h"
#include "PixelCharacters.h"
#include "ParticlaEffect.h"
#include "ball.h"
#include <map>        
#include <algorithm>  
#include "MovementRangeSystem.h"
#include "keyboard.h"
#include "mouse.h"
#include "MultiBattle/MultiplayerMap.h"

// 静态成员变量定义
BattlePlayerInfo MultiplayerBattleManager::s_players[MAX_PLAYERS];
BattleState MultiplayerBattleManager::s_currentState = BATTLE_WAITING_PLAYERS;
int MultiplayerBattleManager::s_activePlayerCount = 0;
bool MultiplayerBattleManager::s_isBattleMode = false;
PlayerID MultiplayerBattleManager::s_currentPlayer = PLAYER_1;
bool MultiplayerBattleManager::s_isShowingMovement = false;
bool MultiplayerBattleManager::s_wasMousePressed = false;
// 外部纹理变量声明
extern ID3D11ShaderResourceView* g_Texture2;        // 起点纹理
extern ID3D11ShaderResourceView* g_TextureGround;   // 地面纹理
extern ID3D11ShaderResourceView* g_TextureIce;      // 冰面纹理
extern ID3D11ShaderResourceView* g_TextureSand;     // 沙地纹理
extern ID3D11ShaderResourceView* g_TextureGrass;    // 草地纹理
extern ID3D11ShaderResourceView* g_TextureWater;    // 水面纹理
extern ID3D11ShaderResourceView* g_TextureWall;     // 墙体纹理
extern ID3D11ShaderResourceView* g_TextureCoin;     // 金币纹理
extern ID3D11ShaderResourceView* g_TextureBomb;     // 炸弹纹理

// 外部模型变量声明
extern MODEL* Tree;

// ========================================
// 核心初始化和清理
// ========================================

bool MultiplayerBattleManager::PickBattleGridUnderMouse(int* outX, int* outZ)
{
    Camera* camera = GetCamera();

    // 获取鼠标位置
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(g_hWnd, &pt);

    RECT clientRect;
    GetClientRect(g_hWnd, &clientRect);
    float screenWidth = (float)(clientRect.right - clientRect.left);
    float screenHeight = (float)(clientRect.bottom - clientRect.top);

    if (pt.x < 0 || pt.y < 0 || pt.x >= screenWidth || pt.y >= screenHeight)
        return false;

    // 🔧 使用与地图编辑器相同的射线计算方法
    float ndcX = (2.0f * pt.x) / screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * pt.y) / screenHeight;

    XMVECTOR nearPoint = XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
    XMVECTOR farPoint = XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);
    XMMATRIX viewProj = XMMatrixMultiply(camera->ViewMatrix, camera->ProjectionMatrix);
    XMMATRIX invViewProj = XMMatrixInverse(nullptr, viewProj);

    XMVECTOR rayOrigin = XMVector3TransformCoord(nearPoint, invViewProj);
    XMVECTOR rayTarget = XMVector3TransformCoord(farPoint, invViewProj);
    XMVECTOR rayDir = XMVector3Normalize(rayTarget - rayOrigin);

    // 🔧 遍历30x30战斗地图的所有格子
    BoxObject* boxes = GetBox();
    float closestDist = FLT_MAX;
    int pickedIndex = -1;

    for (int i = 0; i < BATTLE_MAP_SIZE_X * BATTLE_MAP_SIZE_Z; ++i) {
        if (!boxes[i].Use) continue;

        XMFLOAT3 min = {
            boxes[i].position.x - BOX_RADIUS,
            boxes[i].position.y - BOX_RADIUS,
            boxes[i].position.z - BOX_RADIUS,
        };
        XMFLOAT3 max = {
            boxes[i].position.x + BOX_RADIUS,
            boxes[i].position.y + BOX_RADIUS,
            boxes[i].position.z + BOX_RADIUS,
        };

        float dist = 0.0f;
        if (RayIntersectsAABB(rayOrigin, rayDir, XMLoadFloat3(&min), XMLoadFloat3(&max), &dist)) {
            if (dist < closestDist) {
                closestDist = dist;
                pickedIndex = i;
            }
        }
    }

    // 🔧 转换为30x30格子坐标
    if (pickedIndex >= 0) {
        *outX = pickedIndex % BATTLE_MAP_SIZE_X;
        *outZ = pickedIndex / BATTLE_MAP_SIZE_X;

        char debugMsg[256];
        sprintf_s(debugMsg, "🎯 射线检测命中: 索引%d -> 格子(%d,%d)\n",
            pickedIndex, *outX, *outZ);
        OutputDebugStringA(debugMsg);

        return true;
    }

    return false;
}

bool MultiplayerBattleManager::RayIntersectsAABB(
      XMVECTOR rayOrigin,
    XMVECTOR rayDir,
    XMVECTOR boxMin,
    XMVECTOR boxMax,
    float* outDistance) {

    float tMin = 0.0f;
    float tMax = FLT_MAX;

    for (int i = 0; i < 3; ++i) {
        float rayO = XMVectorGetByIndex(rayOrigin, i);
        float rayD = XMVectorGetByIndex(rayDir, i);
        float bMin = XMVectorGetByIndex(boxMin, i);
        float bMax = XMVectorGetByIndex(boxMax, i);

        if (fabs(rayD) < 1e-6f) {
            // 射线平行于 slab，如果起点不在 slab 内则不相交
            if (rayO < bMin || rayO > bMax)
                return false;
        }
        else {
            float ood = 1.0f / rayD;
            float t1 = (bMin - rayO) * ood;
            float t2 = (bMax - rayO) * ood;
            if (t1 > t2) std::swap(t1, t2);
            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);
            if (tMin > tMax)
                return false;
        }
    }

    if (outDistance) *outDistance = tMin;
    return true;
}
void MultiplayerBattleManager::HandleMouseInput()
{
    if (s_currentState != BATTLE_IN_PROGRESS) return;

    PlayerID currentPlayer = s_currentPlayer;
    BattlePlayerInfo* player = GetPlayer(currentPlayer);

    if (!player || !player->isActive || !player->isCurrentTurn) return;

    // 显示/隐藏移动范围（空格键）
    if (Keyboard_IsKeyDownTrigger(KK_SPACE)) {
        if (s_isShowingMovement) {
            HideMovementRange();
        }
        else {
            ShowCurrentPlayerMovementRange();
        }
    }

    // 玩家切换（1-4数字键）
    if (Keyboard_IsKeyDownTrigger(KK_D1)) SetCurrentPlayer(PLAYER_1);
    if (Keyboard_IsKeyDownTrigger(KK_D2)) SetCurrentPlayer(PLAYER_2);
    if (Keyboard_IsKeyDownTrigger(KK_D3)) SetCurrentPlayer(PLAYER_3);
    if (Keyboard_IsKeyDownTrigger(KK_D4)) SetCurrentPlayer(PLAYER_4);

    // 结束回合（回车键）
    if (Keyboard_IsKeyDownTrigger(KK_ENTER)) {
        EndPlayerTurn();
    }

    // 🔧 简化的鼠标点击处理 - 不再需要传递鼠标坐标
    Mouse_State mouseState;
    Mouse_GetState(&mouseState);

    if (mouseState.leftButton && !s_wasMousePressed) {
        s_wasMousePressed = true;
        HandleMouseClick(0, 0); // 坐标参数不再需要，函数内部会自动获取
    }
    else if (!mouseState.leftButton) {
        s_wasMousePressed = false;
    }
}

void MultiplayerBattleManager::HandleMouseClick(int mouseX, int mouseY)
{
    int gridX, gridZ;

    // 使用射线检测获取准确的格子坐标
    if (PickBattleGridUnderMouse(&gridX, &gridZ)) {
        char debugMsg[256];
        sprintf_s(debugMsg, "🖱️ 鼠标点击检测到格子: (%d,%d)\n", gridX, gridZ);
        OutputDebugStringA(debugMsg);

        // 检查是否在移动范围内
        if (IsValidMoveTarget(gridX, gridZ)) {
            // 执行移动
            MovePlayerTo(s_currentPlayer, gridX, gridZ);
            OutputDebugStringA("✅ 执行鼠标点击移动\n");
        }
        else {
            OutputDebugStringA("❌ 点击位置不在移动范围内\n");

            // 🔧 调试：显示当前可移动的格子
            auto reachableGrids = MovementRangeSystem::GetreachaGrid();
            if (reachableGrids.empty()) {
                OutputDebugStringA("❌ 没有可移动的格子！移动范围系统可能未正确初始化\n");
            }
            else {
                char availableMsg[512] = "可移动格子: ";
                for (const auto& grid : reachableGrids) {
                    char temp[32];
                    sprintf_s(temp, "(%d,%d) ", grid.x, grid.z);
                    strcat_s(availableMsg, temp);
                }
                strcat_s(availableMsg, "\n");
                OutputDebugStringA(availableMsg);
            }
        }
    }
    else {
        OutputDebugStringA("❌ 鼠标点击未检测到有效格子\n");
    }
}

XMFLOAT3 MultiplayerBattleManager::ScreenToWorldCoordinate(int screenX, int screenY)
{
   
    return XMFLOAT3(0.0f, 0.0f, 0.0f);
}
void MultiplayerBattleManager::EndPlayerTurn()
{
    // 切换到下一个活跃玩家
    int nextPlayer = (s_currentPlayer + 1) % MAX_PLAYERS;

    // 找到下一个活跃玩家
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (s_players[nextPlayer].isActive) {
            SetCurrentPlayer((PlayerID)nextPlayer);
            break;
        }
        nextPlayer = (nextPlayer + 1) % MAX_PLAYERS;
    }
}

void MultiplayerBattleManager::Init() {

    // 初始化玩家数据
    for (int i = 0; i < MAX_PLAYERS; i++) {
        s_players[i].isActive = false;
        sprintf_s(s_players[i].playerName, "Player%d", i + 1);

        // 设置玩家颜色
        switch (i) {
        case PLAYER_1: s_players[i].playerColor = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f); break; // 红色
        case PLAYER_2: s_players[i].playerColor = XMFLOAT4(0.2f, 0.2f, 1.0f, 1.0f); break; // 蓝色
        case PLAYER_3: s_players[i].playerColor = XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f); break; // 绿色
        case PLAYER_4: s_players[i].playerColor = XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f); break; // 黄色
        }

        s_players[i].spawnPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
    }

    MultiplayerBattleMap::Init();
    // 初始化移动范围系统
    InitMovementRangeSystem();

    // 为每个玩家设置移动力
    for (int i = 0; i < MAX_PLAYERS; i++) {
        s_players[i].movementPoints = 8;      // 当前移动力
        s_players[i].maxMovementPoints = 8;   // 最大移动力
        s_players[i].isCurrentTurn = (i == 0); // 玩家1先开始
        s_players[i].gridPosition = XMFLOAT2(0, 0);
    }

    s_currentPlayer = PLAYER_1;
    s_isShowingMovement = false;
   

    s_currentState = BATTLE_WAITING_PLAYERS;
    s_activePlayerCount = 0;
    s_isBattleMode = false;
}

void MultiplayerBattleManager::Uninit() {
    s_isBattleMode = false;
    s_currentState = BATTLE_WAITING_PLAYERS;
    s_activePlayerCount = 0;
    MultiplayerBattleMap::Uninit();
    UninitMovementRangeSystem();
}

// ========================================
// 玩家管理
// ========================================

void MultiplayerBattleManager::SetPlayerActive(PlayerID playerID, bool active, const char* name) {
    if (playerID < 0 || playerID >= MAX_PLAYERS) return;

    bool wasActive = s_players[playerID].isActive;
    s_players[playerID].isActive = active;

    if (name) {
        strncpy_s(s_players[playerID].playerName, name, sizeof(s_players[playerID].playerName) - 1);
    }

    // 更新活跃玩家数量
    if (active && !wasActive) {
        s_activePlayerCount++;
    }
    else if (!active && wasActive) {
        s_activePlayerCount--;
    }

    char msg[128];
    sprintf_s(msg, "🎮 玩家%d %s, 当前活跃玩家: %d\n",
        playerID, active ? "加入" : "离开", s_activePlayerCount);
    OutputDebugStringA(msg);
}

void MultiplayerBattleManager::LoadPlayerMapFromSlot(PlayerID playerID, int mapSlot) {
    if (playerID < 0 || playerID >= MAX_PLAYERS) return;

   
}

void MultiplayerBattleManager::StartBattle() {
    if (s_activePlayerCount < 2) {
        OutputDebugStringA("❌ 玩家数量不足，无法开始战斗\n");
        return;
    }

    // 先应用地图，再设置其他状态
    MultiplayerBattleMap::ApplyBattleMapToWorld();

    //  在切换模式之前先设置相机
    SetupBattleCamera();

    // 停用战斗大厅并切换模式
    extern void DeactivateBattleHall();
    DeactivateBattleHall();


    // 设置战斗模式
    s_isBattleMode = true;
    s_currentState = BATTLE_IN_PROGRESS;

    // 切换到多人战斗模式
    SwitchToMode(APP_MODE_MULTIPLAYER_BATTLE);

    // 设置多人战斗专用相机
    SetupBattleCamera();

    // 初始化玩家位置
    InitializePlayerPositions();

    SetCurrentPlayer(PLAYER_1);

}

void MultiplayerBattleManager::SetupBattleCamera() {
    Camera* camera = GetCamera();

    // 计算30x30地图的真实中心点
    float mapCenterX = (BATTLE_MAP_SIZE_X * BOXSIZE_X) / 2.0f;  // 30格的中心
    float mapCenterZ = -(BATTLE_MAP_SIZE_Z * BOXSIZE_Z) / 2.0f; // 注意Z轴负方向

    // 🔧 **修复**: 设置摄像机位置 - 从更高更远的位置俯视整个30x30地图
    camera->Position = XMFLOAT3(mapCenterX, 80.0f, mapCenterZ - 60.0f);  // 在地图中心的北侧高空
    camera->AtPosition = XMFLOAT3(mapCenterX, 0.0f, mapCenterZ);         // 看向地图中心
    camera->UpVector = XMFLOAT3(0.0f, 1.0f, 0.0f);

    // 立即计算并设置视图矩阵
    XMVECTOR eyePos = XMVectorSet(camera->Position.x, camera->Position.y, camera->Position.z, 1.0f);
    XMVECTOR focusPos = XMVectorSet(camera->AtPosition.x, camera->AtPosition.y, camera->AtPosition.z, 1.0f);
    XMVECTOR upVec = XMVectorSet(camera->UpVector.x, camera->UpVector.y, camera->UpVector.z, 0.0f);

    camera->ViewMatrix = XMMatrixLookAtLH(eyePos, focusPos, upVec);

    // 设置更广的视野以覆盖30x30地图
    camera->ProjectionMatrix = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(75.0f),  // 适中的FOV
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.1f,
        1000.0f
    );

    // ，防止其他系统干扰
    camera->AtPositionAngle = XMFLOAT3(0.0f, 0.0f, 0.0f);
    camera->AtPositionOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);

}

void MultiplayerBattleManager::InitializePlayerPositions() {
    OutputDebugStringA("🎮 初始化玩家位置\n");

    // 为每个活跃玩家设置正确的出生位置
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (s_players[i].isActive) {
            // 🔧 **重新计算出生点**
            s_players[i].spawnPosition = MultiplayerBattleMap::GetPlayerSpawnPosition((PlayerID)i);

            char msg[256];
            sprintf_s(msg, "🎮 玩家%d最终出生位置: (%.1f, %.1f, %.1f)\n",
                i, s_players[i].spawnPosition.x,
                s_players[i].spawnPosition.y,
                s_players[i].spawnPosition.z);
            OutputDebugStringA(msg);
        }
    }
}
void MultiplayerBattleManager::EndBattle() {
    s_isBattleMode = false;
    s_currentState = BATTLE_FINISHED;

    OutputDebugStringA("🏁 战斗结束\n");

    // 显示战斗结果
    UIManager::ShowStatusInfo(L"战斗结束！", XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f), 3.0f);

    // 返回预览模式
    SwitchToMode(APP_MODE_PREVIEW);
}

// ========================================
// 更新逻辑
// ========================================

void MultiplayerBattleManager::Update(float deltaTime) {
    if (!s_isBattleMode) return;

    // 更新移动范围系统
    UpdateMovementRangeSystem(deltaTime);

    // 更新战斗逻辑
    switch (s_currentState) {
    case BATTLE_IN_PROGRESS:
        UpdateBattleLogic(deltaTime);
        break;
    case BATTLE_FINISHED:
        // 战斗结束后的处理
        break;
    }
}

void MultiplayerBattleManager::UpdateBattleLogic(float deltaTime) {
    // 检查胜利条件
    int alivePlayers = 0;
    int lastAlivePlayer = -1;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (s_players[i].isActive) {
            // 这里应该检查实际的玩家存活状态
            alivePlayers++;
            lastAlivePlayer = i;
        }
    }
    // 添加鼠标点击处理
    HandleMouseInput();

    //// 更新玩家移动动画
    //UpdatePlayerMovement(deltaTime);
    // 如果只剩一个玩家，结束战斗
    if (alivePlayers <= 1) {
        EndBattle();
    }
}

// ========================================
// 工具函数
// ========================================



void MultiplayerBattleManager::ShowCurrentPlayerMovementRange()
{
    if (s_currentPlayer >= 0 && s_currentPlayer < MAX_PLAYERS &&
        s_players[s_currentPlayer].isActive) {

        ShowPlayerMovementRange(s_currentPlayer);
        s_isShowingMovement = true;

        char msg[128];
        sprintf_s(msg, "🎯 显示玩家%d的移动范围\n", s_currentPlayer);
        OutputDebugStringA(msg);
    }
}

void MultiplayerBattleManager::HideMovementRange()
{
    HidePlayerMovementRange();
    s_isShowingMovement = false;

}

bool MultiplayerBattleManager::IsValidMoveTarget(int x, int z)
{
    return MovementRangeSystem::IsGridReachable(x,z);
}

void MultiplayerBattleManager::MovePlayerTo(PlayerID playerID, int targetX, int targetZ)
{
    if (playerID < 0 || playerID >= MAX_PLAYERS || !s_players[playerID].isActive) {
        return;
    }

    // 检查目标位置是否可达
    if (!IsValidMoveTarget(targetX, targetZ)) {
        OutputDebugStringA("❌ 目标位置不可达\n");
        return;
    }

    // 计算移动消耗
    int movementCost = MovementRangeSystem::GetMovementCostToGrid(targetX, targetZ);
    if (movementCost > s_players[playerID].movementPoints) {
        OutputDebugStringA("❌ 移动力不足\n");
        return;
    }

    // 直接移动到目标位置（无动画版本）
    s_players[playerID].spawnPosition = XMFLOAT3(
        targetX * BOXSIZE_X + (BOXSIZE_X / 2),
        BOXSIZE_Y + 1.0f,
        -targetZ * BOXSIZE_Z + (BOXSIZE_Z / 2)
    );

    s_players[playerID].gridPosition = XMFLOAT2((float)targetX, (float)targetZ);
    s_players[playerID].movementPoints -= movementCost;

    // 隐藏移动范围
    HideMovementRange();

    char msg[256];
    sprintf_s(msg, "✅ 玩家%d移动到(%d,%d)，消耗%d移动力，剩余%d\n",
        playerID, targetX, targetZ, movementCost, s_players[playerID].movementPoints);
    OutputDebugStringA(msg);
}

void MultiplayerBattleManager::SetCurrentPlayer(PlayerID playerID)
{
    // 重置所有玩家的回合状态
    for (int i = 0; i < MAX_PLAYERS; i++) {
        s_players[i].isCurrentTurn = false;
    }

    // 设置当前玩家
    if (playerID >= 0 && playerID < MAX_PLAYERS && s_players[playerID].isActive) {
        s_currentPlayer = playerID;
        s_players[playerID].isCurrentTurn = true;

        // 恢复移动力
        s_players[playerID].movementPoints = s_players[playerID].maxMovementPoints;

        int playerX = (int)(s_players[playerID].spawnPosition.x / BOXSIZE_X);
        int playerZ = (int)(-s_players[playerID].spawnPosition.z / BOXSIZE_Z);

        // 直接调用移动范围计算
        MovementRangeSystem::CalculateMovementRange(playerID, playerX, playerZ, s_players[playerID].movementPoints);
        MovementRangeSystem::SetVisible(true);
        s_isShowingMovement = true;
        char msg[256];
        sprintf_s(msg, "🎮 切换到玩家%d的回合，位置(%d,%d)，移动力%d\n",
            playerID, playerX, playerZ, s_players[playerID].movementPoints);
        OutputDebugStringA(msg);
    }
}



bool MultiplayerBattleManager::IsInitialized() {
    return s_isBattleMode || s_currentState != BATTLE_WAITING_PLAYERS;
}

BattlePlayerInfo* MultiplayerBattleManager::GetPlayer(PlayerID playerID) {
    if (playerID < 0 || playerID >= MAX_PLAYERS) {
        return nullptr;
    }
    return &s_players[playerID];
}

// ========================================
// 外部接口函数（供Game.cpp调用）
// ========================================

void InitMultiplayerBattle() {
    MultiplayerBattleManager::Init();
}

void UninitMultiplayerBattle() {
    MultiplayerBattleManager::Uninit();
}

void UpdateMultiplayerBattle(float deltaTime) {
    MultiplayerBattleManager::Update(deltaTime);
}

void DrawMultiplayerBattleScene() {

    Camera* camera = GetCamera();
    SetViewMatrix(camera->ViewMatrix);
    SetProjectionMatrix(camera->ProjectionMatrix);

    // 绘制标准游戏场景
    DrawCamera();
    DrawBox();

    //绘制移动范围指示器
    DrawMovementRangeSystem();

    // 绘制所有活跃玩家
    DrawAllPlayers();

    // 绘制特效
    ParticleSystem::Draw();
    FloatingTextSystem::Draw();

}

void DrawMultiplayerBattleOverlay(ID3D11ShaderResourceView* ptexture) {
    // 全屏显示主视图
    GetDeviceContext()->PSSetShaderResources(0, 1, &ptexture);
    XMMATRIX WorldMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(WorldMatrix);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

    // 解绑纹理
    ID3D11ShaderResourceView* nullSRV = NULL;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    // 绘制多人战斗专用小地图
    DrawEnhancedBattleMinimap();

    //显示当前玩家信息和移动力
    DrawCurrentPlayerInfo();

    //显示控制说明
    DrawBattleControlHelp();
}

void DrawAllPlayers() {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (MultiplayerBattleManager::s_players[i].isActive) {
            XMFLOAT3 pos = MultiplayerBattleManager::s_players[i].spawnPosition;

            // 🔧 **修复**: 使用像素角色系统绘制玩家
            if (GetPixelCharactersInitialized()) {
                XMMATRIX playerWorld = XMMatrixScaling(0.8f, 0.8f, 0.8f) *
                    XMMatrixTranslation(pos.x, pos.y + 1.0f, pos.z);

                // 根据玩家ID绘制不同角色
                switch (i) {
                case 0:
                    if (GetPixelCharacterValid(CHARACTER_DOCTOR)) {
                        DrawPixelCharacter(&g_Doctor, playerWorld);
                    }
                    break;
                case 1:
                    if (GetPixelCharacterValid(CHARACTER_SOLDIER)) {
                        DrawPixelCharacter(&g_Soldier, playerWorld);
                    }
                    break;
                case 2:
                    if (GetPixelCharacterValid(CHARACTER_SCOUT)) {
                        DrawPixelCharacter(&g_Scout, playerWorld);
                    }
                    break;
                case 3:
                    if (GetPixelCharacterValid(CHARACTER_BOMB_TECH)) {
                        DrawPixelCharacter(&g_BombTech, playerWorld);
                    }
                    break;
                }
            }
            else {
                // 🔧 **修复**: 使用Box系统的访问函数
                XMMATRIX playerWorld = XMMatrixScaling(1.0f, 2.0f, 1.0f) *
                    XMMatrixTranslation(pos.x, pos.y + 1.0f, pos.z);
                SetWorldMatrix(playerWorld);

                // 设置玩家颜色
                MATERIAL playerMaterial;
                ZeroMemory(&playerMaterial, sizeof(playerMaterial));
                playerMaterial.Diffuse = MultiplayerBattleManager::s_players[i].playerColor;
                SetMaterial(playerMaterial);

                // 🔧 **使用访问函数获取缓冲区**
                ID3D11Buffer* vertexBuffer = GetBoxVertexBuffer();
                ID3D11Buffer* indexBuffer = GetBoxIndexBuffer();

                if (vertexBuffer && indexBuffer) {
                    UINT stride = sizeof(VERTEX_3D);
                    UINT offset = 0;
                    GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
                    GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
                    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    GetDeviceContext()->DrawIndexed(GetBoxIndexCount(), 0, 0);
                }
            }
        }
    }
}
void DrawBattleMinimap() {
    // 绘制30x30地图的小地图
    const float MINIMAP_SIZE = 250.0f;
    const float MARGIN = 20.0f;

    // 设置小地图位置
    XMMATRIX worldMinimap = XMMatrixTranslation(
        SCREEN_WIDTH - MINIMAP_SIZE / 2 - MARGIN,
        MINIMAP_SIZE / 2 + MARGIN,
        0.0f);
    SetWorldMatrix(worldMinimap);

    // 绘制小地图背景
    DrawSprite(XMFLOAT2(MINIMAP_SIZE, MINIMAP_SIZE), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.8f));

    // 绘制分界线
    DrawMapDividers(MINIMAP_SIZE, MARGIN);

    // 绘制玩家位置
    DrawPlayerMarkers(MINIMAP_SIZE, MARGIN);
}

void DrawMapDividers(float minimapSize, float margin) {
    // 绘制4个区域的分界线
    float centerX = SCREEN_WIDTH - margin - minimapSize / 2;
    float centerY = margin + minimapSize / 2;

    // 垂直分界线
    SetWorldMatrix(XMMatrixTranslation(centerX, centerY, 0.0f));
    DrawSprite(XMFLOAT2(2.0f, minimapSize), XMFLOAT4(0.5f, 0.5f, 0.5f, 0.8f));

    // 水平分界线
    SetWorldMatrix(XMMatrixTranslation(centerX, centerY, 0.0f));
    DrawSprite(XMFLOAT2(minimapSize, 2.0f), XMFLOAT4(0.5f, 0.5f, 0.5f, 0.8f));
}

void DrawPlayerMarkers(float minimapSize, float margin) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (MultiplayerBattleManager::s_players[i].isActive) {
            XMFLOAT3 spawnPos = MultiplayerBattleManager::s_players[i].spawnPosition;

            // 转换世界坐标到小地图坐标
            float mapX = SCREEN_WIDTH - margin - minimapSize +
                (spawnPos.x / (30.0f * BOXSIZE_X)) * minimapSize;
            float mapY = margin +
                (-spawnPos.z / (30.0f * BOXSIZE_Z)) * minimapSize;

            // 绘制玩家标记
            SetWorldMatrix(XMMatrixTranslation(mapX, mapY, 0.0f));
            DrawSprite(XMFLOAT2(8.0f, 8.0f),
                MultiplayerBattleManager::s_players[i].playerColor);
        }
    }
}

void DrawEnhancedBattleMinimap() {
    const float MINIMAP_SIZE = 300.0f; // 比普通小地图大一些
    const float MARGIN = 20.0f;

    // 设置小地图位置 (右上角)
    float minimapX = SCREEN_WIDTH - MINIMAP_SIZE / 2 - MARGIN;
    float minimapY = MINIMAP_SIZE / 2 + MARGIN;

    XMMATRIX worldMinimap = XMMatrixTranslation(minimapX, minimapY, 0.0f);
    SetWorldMatrix(worldMinimap);

    // 绘制小地图背景
    DrawSprite(XMFLOAT2(MINIMAP_SIZE, MINIMAP_SIZE), XMFLOAT4(0.1f, 0.1f, 0.1f, 0.9f));

    // 🔧 **新增**: 绘制4个玩家区域的边界
    DrawPlayerRegionBorders(minimapX, minimapY, MINIMAP_SIZE);

    // 🔧 **新增**: 绘制玩家位置标记
    DrawPlayerPositions(minimapX, minimapY, MINIMAP_SIZE);

    // 绘制小地图标题
    GeometricTextRenderer::DrawMinecraftText(
        minimapX - MINIMAP_SIZE / 2 + 5, minimapY - MINIMAP_SIZE / 2 + 5,
        L"Battle Map (4 Players)",
        XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
        1.0f,
        false
    );
}

void DrawPlayerRegionBorders(float centerX, float centerY, float mapSize) {
    float halfSize = mapSize / 2;

    // 垂直分界线 (分割左右区域)
    XMMATRIX verticalLine = XMMatrixTranslation(centerX, centerY, 0.0f);
    SetWorldMatrix(verticalLine);
    DrawSprite(XMFLOAT2(2.0f, mapSize), XMFLOAT4(1.0f, 1.0f, 0.0f, 0.8f)); // 黄色分界线

    // 水平分界线 (分割上下区域)  
    XMMATRIX horizontalLine = XMMatrixTranslation(centerX, centerY, 0.0f);
    SetWorldMatrix(horizontalLine);
    DrawSprite(XMFLOAT2(mapSize, 2.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 0.8f)); // 黄色分界线

    // 为每个区域添加颜色标识
    XMFLOAT4 playerColors[4] = {
        XMFLOAT4(1.0f, 0.2f, 0.2f, 0.3f), // 玩家1 - 红色区域
        XMFLOAT4(0.2f, 0.2f, 1.0f, 0.3f), // 玩家2 - 蓝色区域  
        XMFLOAT4(0.2f, 1.0f, 0.2f, 0.3f), // 玩家3 - 绿色区域
        XMFLOAT4(1.0f, 1.0f, 0.2f, 0.3f)  // 玩家4 - 黄色区域
    };

    // 绘制4个有色区域背景
    float quarterSize = mapSize / 2;
    XMFLOAT2 regionPositions[4] = {
        XMFLOAT2(centerX - quarterSize / 2, centerY - quarterSize / 2), // 左上
        XMFLOAT2(centerX + quarterSize / 2, centerY - quarterSize / 2), // 右上  
        XMFLOAT2(centerX - quarterSize / 2, centerY + quarterSize / 2), // 左下
        XMFLOAT2(centerX + quarterSize / 2, centerY + quarterSize / 2)  // 右下
    };

    for (int i = 0; i < 4; i++) {
        if (MultiplayerBattleManager::s_players[i].isActive) {
            XMMATRIX regionMatrix = XMMatrixTranslation(regionPositions[i].x, regionPositions[i].y, 0.0f);
            SetWorldMatrix(regionMatrix);
            DrawSprite(XMFLOAT2(quarterSize - 4, quarterSize - 4), playerColors[i]);
        }
    }
}

void DrawPlayerPositions(float centerX, float centerY, float mapSize) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!MultiplayerBattleManager::s_players[i].isActive) continue;

        
        XMFLOAT3 playerWorldPos = MultiplayerBattleManager::s_players[i].spawnPosition;

        // 将世界坐标转换为小地图坐标
        float normalizedX = playerWorldPos.x / (BATTLE_MAP_SIZE_X * BOXSIZE_X);
        float normalizedZ = -playerWorldPos.z / (BATTLE_MAP_SIZE_Z * BOXSIZE_Z);

        // 转换到小地图屏幕坐标
        float mapX = centerX - mapSize / 2 + normalizedX * mapSize;
        float mapY = centerY - mapSize / 2 + normalizedZ * mapSize;

        // 绘制玩家标记
        XMMATRIX playerMarker = XMMatrixTranslation(mapX, mapY, 0.0f);
        SetWorldMatrix(playerMarker);

        // 使用玩家颜色绘制标记
        DrawSprite(XMFLOAT2(12.0f, 12.0f), MultiplayerBattleManager::s_players[i].playerColor);

        // 🔧 **新增**: 在玩家标记旁显示玩家编号
        wchar_t playerNum[8];
        swprintf_s(playerNum, L"P%d", i + 1);
        GeometricTextRenderer::DrawMinecraftText(
            mapX + 8, mapY - 8,
            playerNum,
            XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
            0.8f,
            false
        );
    }
}
void DrawCurrentPlayerInfo() {
    PlayerID currentPlayer = MultiplayerBattleManager::GetCurrentPlayer();
    BattlePlayerInfo* player = MultiplayerBattleManager::GetPlayer(currentPlayer);

    if (!player || !player->isActive) return;

    // 保存渲染状态
    bool savedDepthEnable = GetDepthEnable();
    bool savedBlendEnable = GetBlendState();
    SetDepthEnable(false);
    SetBlendState(true);

    // 显示当前玩家信息
    wchar_t playerInfo[256];
    swprintf_s(playerInfo, L"Player %d Turn - Movement: %d/%d",
        currentPlayer + 1,
        player->movementPoints,
        player->maxMovementPoints);

    GeometricTextRenderer::DrawMinecraftText(
        20, SCREEN_HEIGHT - 100,
        playerInfo,
        player->playerColor,
        1.5f,
        false
    );

    // 显示移动力条
    DrawMovementPointsBar(currentPlayer);

    // 恢复渲染状态
    SetDepthEnable(savedDepthEnable);
    SetBlendState(savedBlendEnable);
}

void DrawMovementPointsBar(PlayerID playerID) {
    BattlePlayerInfo* player = MultiplayerBattleManager::GetPlayer(playerID);
    if (!player) return;

    float barWidth = 200.0f;
    float barHeight = 20.0f;
    float barX = 20.0f;
    float barY = SCREEN_HEIGHT - 70.0f;

    // 背景条
    XMMATRIX bgMatrix = XMMatrixTranslation(barX + barWidth / 2, barY, 0.0f);
    SetWorldMatrix(bgMatrix);
    DrawSprite(XMFLOAT2(barWidth, barHeight), XMFLOAT4(0.2f, 0.2f, 0.2f, 0.8f));

    // 移动力条
    float fillRatio = (float)player->movementPoints / (float)player->maxMovementPoints;
    float fillWidth = barWidth * fillRatio;

    XMFLOAT4 barColor;
    if (fillRatio > 0.6f) {
        barColor = XMFLOAT4(0.2f, 1.0f, 0.3f, 0.8f); // 绿色
    }
    else if (fillRatio > 0.3f) {
        barColor = XMFLOAT4(1.0f, 1.0f, 0.2f, 0.8f); // 黄色
    }
    else {
        barColor = XMFLOAT4(1.0f, 0.3f, 0.2f, 0.8f); // 红色
    }

    if (fillWidth > 0) {
        XMMATRIX fillMatrix = XMMatrixTranslation(barX + fillWidth / 2, barY, 0.0f);
        SetWorldMatrix(fillMatrix);
        DrawSprite(XMFLOAT2(fillWidth, barHeight), barColor);
    }
}

void DrawBattleControlHelp() {
    // 控制说明
    const wchar_t* helpTexts[] = {
        L"SPACE - Show/Hide Movement Range",
        L"TAB - Change Display Type",
        L"1-4 - Switch Player",
        L"ENTER - End Turn",
        L"Click - Move to Target"
    };

    float startY = 20.0f;
    for (int i = 0; i < 5; i++) {
        GeometricTextRenderer::DrawMinecraftText(
            SCREEN_WIDTH - 350, startY + i * 25,
            helpTexts[i],
            XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
            0.8f,
            false
        );
    }
}