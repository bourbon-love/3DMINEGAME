// SceneManager.cpp
#include "SceneManager.h"
#include "Camera.h"
#include "keyboard.h"
#include "Box.h"
#include "MapEditor.h"
#include "mouse.h"
#include "UIManager.h"
#include "ball.h"
#include  "CharactorScene.h"
#include  "BattleHall.h"
#include  "MultiplayerBattle.h"
// 当前应用模式与转换状态
static AppMode g_CurrentAppMode = APP_MODE_PREVIEW;
static AppMode g_TargetAppMode = APP_MODE_PREVIEW;
static bool g_IsModeChanging = false;
static float g_TransitionProgress = 0.0f;
static const float TRANSITION_SPEED = 0.1f;

// 障碍物编辑状态
static bool g_IsEditingObstacles = false;

// 相机转换状态
static bool g_IsCameraTransitioning = false;

// 玩家状态
static int g_PlayerScore = 0;
static int g_PlayerLives = 3;
static bool g_GameInitialized = false;
// 添加上一个模式记录（用于暂停恢复）
static AppMode g_PreviousAppMode = APP_MODE_PREVIEW;
//获取前一个模式（用于暂停恢复）
AppMode GetPreviousAppMode() {
    return g_PreviousAppMode;
}


// 初始化场景管理器
void InitSceneManager() {
    g_CurrentAppMode = APP_MODE_PREVIEW;
    g_TargetAppMode = APP_MODE_PREVIEW;
    g_IsModeChanging = false;
    g_TransitionProgress = 0.0f;
    g_IsEditingObstacles = false;
    g_IsCameraTransitioning = false;
    g_PlayerScore = 0;
    g_PlayerLives = 3;
    g_GameInitialized = false;

    OutputDebugStringA("场景管理器初始化完成\n");

    //Mouse_SetVisible(true);
}

// 销毁场景管理器
void UninitSceneManager() {
    // 目前不需要特殊清理
    OutputDebugStringA("场景管理器已销毁\n");
}

// 更新场景管理器逻辑
// 在UpdateSceneManager函数中添加多人战斗模式的特殊更新
void UpdateSceneManager(float deltaTime) {
    // 处理Tab键模式切换（仅在预览和编辑模式间生效）
    static bool lastTab = false;
    bool nowTab = Keyboard_IsKeyDown(KK_TAB);

    if (nowTab && !lastTab) {
        // 在预览和编辑模式之间切换
        if (g_CurrentAppMode == APP_MODE_PREVIEW) {
            SwitchToMode(APP_MODE_EDIT);
        }
        else if (g_CurrentAppMode == APP_MODE_EDIT) {
            SwitchToMode(APP_MODE_PREVIEW);
        }
    }
    lastTab = nowTab;

    // 在编辑模式下，处理O键切换障碍物/地形编辑
    static bool lastOKey = false;
    bool nowOKey = Keyboard_IsKeyDown(KK_O);
    if (nowOKey && !lastOKey && g_CurrentAppMode == APP_MODE_EDIT) {
        ToggleObstacleEditingMode();
    }
    lastOKey = nowOKey;

    // 🔧 增强ESC键处理逻辑，支持多人战斗模式
    static bool lastEsc = false;
    bool nowEsc = Keyboard_IsKeyDown(KK_ESCAPE);
    if (nowEsc && !lastEsc) {
        if (g_CurrentAppMode == APP_MODE_GAME || g_CurrentAppMode == APP_MODE_MULTIPLAYER_BATTLE) {
            SwitchToPauseMode();
        }
        else if (g_CurrentAppMode == APP_MODE_PAUSE) {
            // 🔧 恢复到正确的游戏模式
            AppMode previousMode = GetPreviousAppMode();
            if (previousMode == APP_MODE_MULTIPLAYER_BATTLE) {
                SwitchToMultiplayerBattleMode();
            }
            else {
                SwitchToGameMode();
            }
        }
    }
    lastEsc = nowEsc;

    // 处理模式转换进度
    if (g_IsModeChanging) {
        g_TransitionProgress += deltaTime * 1.0f; // 转换速度

        if (g_TransitionProgress >= 1.0f) {
            g_TransitionProgress = 1.0f;
            g_IsModeChanging = false;
            g_CurrentAppMode = g_TargetAppMode;

            // 🔧 多人战斗模式无需相机过渡检查
            if (g_CurrentAppMode != APP_MODE_MULTIPLAYER_BATTLE &&
                g_CurrentAppMode != APP_MODE_BATTLE_HALL &&
                !IsCameraTransitioning()) {
                Camera* pCamera = GetCamera();
                if (g_CurrentAppMode == APP_MODE_EDIT) {
                    pCamera->AtPositionAngle = XMFLOAT3(EDIT_ANGLE_X, EDIT_ANGLE_Y, 0.0f);
                }
                else {
                    pCamera->AtPositionAngle = XMFLOAT3(PREVIEW_ANGLE_X, PREVIEW_ANGLE_Y, 0.0f);
                }
            }

            OutputDebugStringA("模式转换完成\n");
        }
    }

    // 🔧 多人战斗模式下的特殊更新
    if (g_CurrentAppMode == APP_MODE_MULTIPLAYER_BATTLE) {

        UpdateMultiplayerBattle(deltaTime);

        // 更新多人战斗UI
        UIManager::UpdateBattleUI(deltaTime);

        // 检查战斗是否结束
        if (MultiplayerBattleManager::GetBattleState() == BATTLE_FINISHED) {
            // 显示战斗结果
            UIManager::SetBattleUIState(BATTLE_UI_FINISHED);

            // 可以在这里添加战斗结束的特殊逻辑
            OutputDebugStringA("🏁 多人战斗已结束\n");
        }
    }

    // 战斗大厅模式下的特殊更新
    if (g_CurrentAppMode == APP_MODE_BATTLE_HALL) {
        UpdateBattleHall(deltaTime);
    }

    // 🔧 增强生命值检查，支持多人战斗模式
    if (g_CurrentAppMode == APP_MODE_GAME) {
        if (g_PlayerLives <= 0) {
            SwitchToGameOverMode();
        }
    }
    else if (g_CurrentAppMode == APP_MODE_MULTIPLAYER_BATTLE) {
        // 多人战斗模式下，检查是否所有玩家都死亡或只剩一个玩家
        // 这个逻辑可以在MultiplayerBattleManager中处理
        // 这里只是示例
    }

    // 角色模式下的特殊更新
    if (g_CurrentAppMode == APP_MODE_CHARACTER) {
        UpdateCharacterScene(deltaTime);
    }
}
// 获取当前应用模式
AppMode GetCurrentAppMode() {
    return g_CurrentAppMode;
}

AppMode GetTargetAppMode() {
    return g_TargetAppMode;
}

void SwitchToMode(AppMode newMode) {
    if (g_CurrentAppMode == newMode) return;

    // 🔧 特殊处理：从多人战斗模式退出
    if (g_CurrentAppMode == APP_MODE_MULTIPLAYER_BATTLE && newMode != APP_MODE_PAUSE) {
        // 清理多人战斗状态
        MultiplayerBattleManager::EndBattle();
        UIManager::SetBattleUIState(BATTLE_UI_FINISHED);

        OutputDebugStringA("🔧 正在退出多人战斗模式\n");
    }

    // 开始转换
    g_TargetAppMode = newMode;
    g_IsModeChanging = true;
    g_TransitionProgress = 0.0f;

    // 立即更新当前模式，但保持摄像机过渡
    g_CurrentAppMode = newMode;

   
    // 🔧 多人战斗模式特殊处理
    if (newMode == APP_MODE_MULTIPLAYER_BATTLE) {
        // 多人战斗模式不需要相机过渡，直接完成
        g_IsModeChanging = false;
        g_TransitionProgress = 1.0f;

        // 设置多人战斗相机
        SetupMultiplayerBattleCamera();

        // 初始化多人战斗UI
        UIManager::InitializeBattleUI();
        UIManager::SwitchToScreen(UI_SCREEN_MULTIPLAYER_BATTLE);

        OutputDebugStringA("🔧 已切换到多人战斗模式\n");
        return;
    }

    // 为角色模式添加特殊处理
    if (newMode == APP_MODE_CHARACTER) {
        // 角色选择模式不需要相机过渡，直接完成
        g_IsModeChanging = false;
        g_TransitionProgress = 1.0f;

        // 激活角色场景
        ActivateCharacterScene();
        // 切换UI屏幕
        UIManager::SwitchToScreen(UI_SCREEN_CHARACTER);

        OutputDebugStringA("已切换到角色选择模式\n");
        return;
    }

    // 战斗大厅模式特殊处理
    if (newMode == APP_MODE_BATTLE_HALL) {
        // 战斗大厅模式不需要相机过渡，直接完成
        g_IsModeChanging = false;
        g_TransitionProgress = 1.0f;

        ActivateBattleHall();
        UIManager::SwitchToScreen(UI_SCREEN_GAME); // 临时使用，后续可创建专门的UI屏幕
        OutputDebugStringA("已切换到战斗大厅模式\n");
        return;
    }

    // 对于其他模式，进行正常的相机过渡
    if (newMode != APP_MODE_CHARACTER && newMode != APP_MODE_MULTIPLAYER_BATTLE) {
        // 初始化摄像机过渡路径
        InitCameraTransitionPath((newMode == APP_MODE_EDIT) ? APP_MODE_PREVIEW : APP_MODE_EDIT, newMode);

        // 设置相机过渡状态
        SetCameraTransitioning(true);
    }

    // 🔧 完善UI屏幕切换逻辑
    if (newMode == APP_MODE_EDIT) {
        UIManager::SwitchToScreen(UI_SCREEN_EDIT);
    }
    else if (newMode == APP_MODE_GAME) {
        UIManager::SwitchToScreen(UI_SCREEN_GAME);
    }
    else if (newMode == APP_MODE_PREVIEW) {
        UIManager::SwitchToScreen(UI_SCREEN_MAINMENU);
    }
    else if (newMode == APP_MODE_PAUSE) {
        // 🔧 根据前一个模式设置暂停屏幕
        if (g_PreviousAppMode == APP_MODE_MULTIPLAYER_BATTLE) {
            UIManager::SwitchToScreen(UI_SCREEN_BATTLE_PAUSE);
        }
        else {
            UIManager::SwitchToScreen(UI_SCREEN_PAUSE);
        }
    }
    else if (newMode == APP_MODE_GAMEOVER) {
        UIManager::SwitchToScreen(UI_SCREEN_GAMEOVER);
    }
    else if (newMode == APP_MODE_CHARACTER) {
        UIManager::SwitchToScreen(UI_SCREEN_CHARACTER);
        InitCharacterScene();
    }
    else if (newMode == APP_MODE_BATTLE_HALL) {
        UIManager::SwitchToScreen(UI_SCREEN_BATTLE_LOBBY);
    }
    else if (newMode == APP_MODE_MULTIPLAYER_BATTLE) {
        UIManager::SwitchToScreen(UI_SCREEN_MULTIPLAYER_BATTLE);
    }
}

// 切换到游戏模式
// 特定游戏模式切换函数
void SwitchToGameMode() {
    if (g_CurrentAppMode == APP_MODE_GAME) return;

    // 如果是从主菜单开始新游戏，重置玩家状态
    if (g_CurrentAppMode == APP_MODE_PREVIEW && !g_GameInitialized) {
        g_PlayerScore = 0;
        g_PlayerLives = 3;
        g_GameInitialized = true;
    }

    g_CurrentAppMode = APP_MODE_GAME;
    g_TargetAppMode = APP_MODE_GAME;
    g_IsModeChanging = false;
    g_TransitionProgress = 1.0f;

    // 隐藏鼠标(游戏模式下不需要鼠标)
   // Mouse_SetVisible(false);

    // 更新UI显示
    UIManager::SwitchToScreen(UI_SCREEN_GAME);

    // 更新生命值和分数显示
    UpdatePlayerStatusUI();

    OutputDebugStringA("已切换到游戏模式\n");
}

// 切换到暂停模式
void SwitchToPauseMode() {
    if (g_CurrentAppMode == APP_MODE_PAUSE) return;

    // 保存当前模式
    g_PreviousAppMode = g_CurrentAppMode;
    g_CurrentAppMode = APP_MODE_PAUSE;
    g_TargetAppMode = APP_MODE_PAUSE;
    g_IsModeChanging = false;
    g_TransitionProgress = 1.0f;

    //// 显示鼠标(暂停界面需要鼠标操作)
    //Mouse_SetVisible(true);

    // 🔧 根据前一个模式选择合适的暂停UI
    if (g_PreviousAppMode == APP_MODE_MULTIPLAYER_BATTLE) {
        UIManager::SwitchToScreen(UI_SCREEN_BATTLE_PAUSE);
        UIManager::SetBattleUIState(BATTLE_UI_PAUSED);
        OutputDebugStringA("已切换到多人战斗暂停模式\n");
    }
    else {
        UIManager::SwitchToScreen(UI_SCREEN_PAUSE);
        OutputDebugStringA("已切换到常规暂停模式\n");
    }
}
// 切换到游戏结束模式
void SwitchToGameOverMode() {
    if (g_CurrentAppMode == APP_MODE_GAMEOVER) return;

    g_CurrentAppMode = APP_MODE_GAMEOVER;
    g_TargetAppMode = APP_MODE_GAMEOVER;
    g_IsModeChanging = false;
    g_TransitionProgress = 1.0f;
    g_GameInitialized = false;

    //// 显示鼠标(游戏结束界面需要鼠标操作)
    //Mouse_SetVisible(true);

    // 更新UI显示
    UIManager::SwitchToScreen(UI_SCREEN_HELP); // 临时使用HELP屏幕，之后会添加专门的GAMEOVER屏幕

    // 显示最终得分
    wchar_t scoreText[64];
    swprintf_s(scoreText, L"游戏结束! 最终得分: %d", g_PlayerScore);
    UIManager::ShowStatusInfo(scoreText, XMFLOAT4(1.0f, 0.3f, 0.3f, 1.0f), 10.0f);

    OutputDebugStringA("已切换到游戏结束模式\n");
}

void SwitchToBattleHallMode()
{
    if (g_CurrentAppMode == APP_MODE_BATTLE_HALL) return;

    g_CurrentAppMode = APP_MODE_BATTLE_HALL;
    g_TargetAppMode = APP_MODE_BATTLE_HALL;
    g_IsModeChanging = false;
    g_TransitionProgress = 1.0f;

    //// 显示鼠标
    //Mouse_SetVisible(true);

    // 激活战斗大厅
    ActivateBattleHall();

    // 更新UI显示（可以创建专门的战斗大厅UI屏幕）
    UIManager::SwitchToScreen(UI_SCREEN_BATTLE_LOBBY); // 临时使用游戏屏幕

    OutputDebugStringA("已切换到战斗大厅模式\n");
}


void SwitchToMultiplayerBattleMode()
{
    if (g_CurrentAppMode == APP_MODE_MULTIPLAYER_BATTLE) return;

    OutputDebugStringA("🚀 切换到多人战斗模式\n");

    // **修复点M**: 确保前置条件
    if (!MultiplayerBattleManager::IsInitialized()) {
        OutputDebugStringA("⚠️ 多人战斗管理器未初始化，先初始化\n");
        MultiplayerBattleManager::Init();
    }

    // 保存当前模式作为前一个模式
    g_PreviousAppMode = g_CurrentAppMode;

    g_CurrentAppMode = APP_MODE_MULTIPLAYER_BATTLE;
    g_TargetAppMode = APP_MODE_MULTIPLAYER_BATTLE;
    g_IsModeChanging = false;
    g_TransitionProgress = 1.0f;

    // **修复点N**: 安全的UI状态设置
    try {
        // 🔧 初始化多人战斗UI
        UIManager::InitializeBattleUI();
        UIManager::SetBattleUIState(BATTLE_UI_ACTIVE);

        // 🔧 切换到专门的多人战斗UI屏幕
        UIManager::SwitchToScreen(UI_SCREEN_MULTIPLAYER_BATTLE);
    }
    catch (...) {
        OutputDebugStringA("❌ UI初始化失败\n");
    }

    // **修复点O**: 安全的相机设置
    try {
        // 🔧 设置多人战斗专用的相机
        SetupMultiplayerBattleCamera();
    }
    catch (...) {
        OutputDebugStringA("❌ 相机设置失败\n");
    }

    // 更新玩家状态UI
    UpdatePlayerStatusUI();

    OutputDebugStringA("✅ 已切换到多人战斗模式\n");
}



// 返回主菜单
void ReturnToMainMenu() {
    // 🔧 如果在多人战斗中，先结束战斗
    if (g_CurrentAppMode == APP_MODE_MULTIPLAYER_BATTLE) {
        MultiplayerBattleManager::EndBattle();
        UIManager::SetBattleUIState(BATTLE_UI_FINISHED);
    }

    // 🔧 如果在战斗大厅中，先停用战斗大厅
    if (g_CurrentAppMode == APP_MODE_BATTLE_HALL) {
        DeactivateBattleHall();
    }

    g_CurrentAppMode = APP_MODE_PREVIEW;
    g_TargetAppMode = APP_MODE_PREVIEW;
    g_IsModeChanging = false;
    g_TransitionProgress = 1.0f;
    g_GameInitialized = false;

    //// 显示鼠标(主菜单需要鼠标操作)
    //Mouse_SetVisible(true);

    // 更新UI显示
    UIManager::SwitchToScreen(UI_SCREEN_MAINMENU);

    OutputDebugStringA("已返回主菜单\n");
}
bool IsModeChanging() {
    return g_IsModeChanging;
}

// 获取转换进度(0.0f到1.0f)
float GetModeTransitionProgress() {
    return g_TransitionProgress;
}

// 判断是否正在编辑障碍物
bool IsEditingObstacle() {
    return g_IsEditingObstacles;
}

// 切换障碍物/地形编辑
void ToggleObstacleEditingMode() {
    g_IsEditingObstacles = !g_IsEditingObstacles;

    // 重置选择状态
    if (g_IsEditingObstacles) {
        // 切换到障碍物编辑，重置地形UI选择
        int* selectedUIBoxIndex = GetSelectedUIBoxIndex();
        UIBox* uiBoxes = GetUIBoxes();
        if (*selectedUIBoxIndex >= 0) {
            uiBoxes[*selectedUIBoxIndex].selected = false;
            *selectedUIBoxIndex = -1;
        }
        OutputDebugStringA("已切换到障碍物编辑模式\n");
    }
    else {
        // 切换到地形编辑，重置障碍物UI选择
        int* selectedObstacleUIBoxIndex = GetSelectedObstacleUIBoxIndex();
        ObstacleUIBox* obstacleUIBoxes = GetObstacleUIBoxes();
        if (*selectedObstacleUIBoxIndex >= 0) {
            obstacleUIBoxes[*selectedObstacleUIBoxIndex].selected = false;
            *selectedObstacleUIBoxIndex = -1;
        }
        OutputDebugStringA("已切换到地形编辑模式\n");
    }
}
// 获取相机转换状态
bool IsCameraTransitioning() {
    return g_IsCameraTransitioning;
}

// 设置相机转换状态
void SetCameraTransitioning(bool isTransitioning) {
    g_IsCameraTransitioning = isTransitioning;
}

// 设置玩家分数
void SetPlayerScore(int score) {
    g_PlayerScore = score;
    UpdatePlayerStatusUI();
}

// 获取玩家分数
int GetPlayerScore() {
    return g_PlayerScore;
}

// 增加玩家分数
void IncreasePlayerScore(int points) {
    g_PlayerScore += points;
    UpdatePlayerStatusUI();

    // 显示得分增加的浮动文本
    BallObject* ball = GetBall();
    if (ball) {
        wchar_t scoreTextW[32];
        swprintf_s(scoreTextW, L"+%d", points);
        char scoreTextA[32];
        WideCharToMultiByte(CP_UTF8, 0,           // 或 CP_ACP
            scoreTextW, -1,
            scoreTextA, 32, nullptr, nullptr);
        // 假设有一个浮动文本系统
        FloatingTextSystem::ShowText(
            ball->position,
            scoreTextA,
            XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) // 黄色文本
        );
    }
}

// 设置玩家生命值
void SetPlayerLives(int lives) {
    g_PlayerLives = lives;
    UpdatePlayerStatusUI();
}

// 获取玩家生命值
int GetPlayerLives() {
    return g_PlayerLives;
}

// 减少玩家生命值
void DecreasePlayerLives() {
    if (g_PlayerLives > 0) {
        g_PlayerLives--;
        UpdatePlayerStatusUI();

        // 显示生命减少的视觉提示
        BallObject* ball = GetBall();
        if (ball) {
            // 播放粒子效果表示受伤
            ParticleSystem::CreateEffect(
                ball->position,
                20,               // 粒子数量
                XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), // 红色
                0.05f,            // 最小尺寸
                0.15f,            // 最大尺寸
                1.0f              // 生命周期
            );

            // 显示文本提示
            UIManager::ShowStatusInfo(L"生命值减少!", XMFLOAT4(1.0f, 0.3f, 0.3f, 1.0f), 2.0f);
        }

        // 如果生命值为0，游戏结束
        if (g_PlayerLives <= 0) {
            SwitchToGameOverMode();
        }
    }
}

// 完善 UpdatePlayerStatusUI 的实现
void UpdatePlayerStatusUI() {
    // 更新分数显示
    UIManager::UpdateScoreDisplay(g_PlayerScore);

    // 更新生命值显示
    UIManager::UpdateLivesDisplay(g_PlayerLives);

    // 更新金币显示
    BallObject* ball = GetBall();
    if (ball) {
        UIManager::UpdateCoinDisplay(ball->collectedCoins);
    }

    // 输出调试信息
    char debug[128];
    sprintf_s(debug, "UI状态更新: 分数=%d, 生命值=%d\n", g_PlayerScore, g_PlayerLives);
    OutputDebugStringA(debug);
}

// 🔧 新增：设置多人战斗专用相机
void SetupMultiplayerBattleCamera() {
    Camera* camera = GetCamera();

    // 多人战斗需要更广阔的视野
    camera->Position = XMFLOAT3(0.0f, 50.0f, -40.0f);
    camera->AtPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
    camera->UpVector = XMFLOAT3(0.0f, 1.0f, 0.0f);

    // 重新计算视图矩阵
    XMVECTOR eyePos = XMVectorSet(camera->Position.x, camera->Position.y, camera->Position.z, 1.0f);
    XMVECTOR focusPos = XMVectorSet(camera->AtPosition.x, camera->AtPosition.y, camera->AtPosition.z, 1.0f);
    XMVECTOR upVec = XMVectorSet(camera->UpVector.x, camera->UpVector.y, camera->UpVector.z, 0.0f);

    camera->ViewMatrix = XMMatrixLookAtLH(eyePos, focusPos, upVec);

    // 设置更适合多人战斗的投影矩阵
    camera->ProjectionMatrix = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(75.0f),  // 更广的视野角度
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.1f,
        1000.0f
    );

    OutputDebugStringA("🎥 多人战斗相机设置完成\n");
}
// 🔧 新增：检查战斗条件
void CheckBattleConditions() {
    // 检查是否只剩一个玩家存活
    int alivePlayers = 0;
    int lastAlivePlayer = -1;

    for (int i = 0; i < 4; i++) {
        BattlePlayerInfo* playerInfo = MultiplayerBattleManager::GetPlayer((PlayerID)i);
        if (playerInfo && playerInfo->isActive) {
            // 这里需要根据实际的游戏逻辑检查玩家是否存活
            // 暂时假设所有激活的玩家都存活
            alivePlayers++;
            lastAlivePlayer = i;
        }
    }

    // 如果只剩一个玩家，结束战斗
    if (alivePlayers <= 1) {
        EndMultiplayerBattle(lastAlivePlayer);
    }
}
// 🔧 新增：结束多人战斗
void EndMultiplayerBattle(int winnerID) {
    OutputDebugStringA("🏁 多人战斗结束\n");

    // 设置战斗结果
    if (winnerID >= 0) {
        BattlePlayerInfo* winner = MultiplayerBattleManager::GetPlayer((PlayerID)winnerID);
        if (winner) {
            char debugMsg[128];
            sprintf_s(debugMsg, "🏆 获胜者: %s\n", winner->playerName);
            OutputDebugStringA(debugMsg);

            // 显示战斗结果
            UIManager::ShowBattleResults(winnerID, L"Battle completed successfully!");
        }
    }
    else {
        // 平局或无获胜者
        UIManager::ShowBattleResults(-1, L"Battle ended in a draw!");
    }

    // 结束战斗管理器
    MultiplayerBattleManager::EndBattle();
}