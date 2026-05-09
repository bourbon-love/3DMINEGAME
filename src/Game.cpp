
#include "Main.h"
#include "Renderer.h"
#include "Manager.h"
#include "Sprite.h"
#include "Game.h"
#include "keyboard.h"
#include "Manager.h"
#include "Box.h"	
#include "Camera.h"
#include "ball.h"
#include "Bg.h"	
#include <chrono>
#include "SkyDome.h"
#include "CameraDebugTool.h"
#include <stdio.h>
#include <stdarg.h>
#include "MapEditor.h"
#include "ParticlaEffect.h"
#include "GeometricTextRenderer.h"
#include "UIManager.h"
#include "SceneManager.h"
#include "ProceduralModels.h"
#include "TextureAtlas.h"
#include "CharactorScene.h"
#include "BattleHall.h"
#include "MultiplayerBattle.h" 
#include "mouse.h"
#include "MiniMap.h"
#include "MultiBattle/MultiplayerMap.h"
#include "FrameWork/TextureManager.h"
using namespace std::chrono;

static steady_clock::time_point g_LastTime;
float g_DeltaTime = 0.016f;
static bool pause = false;
static bool g_IsRenderingBox = false;
int g_FrameCount;

// 调试视觉指示器
void DrawKeyStateIndicator()
{
    int x = 10;
    int y = 10;
    char keyInfo[256];

    bool f11State = Keyboard_IsKeyDown(KK_F11);
    sprintf_s(keyInfo, "F11 (Toggle Debug): %s", f11State ? "DOWN" : "UP");
    OutputDebugStringA(keyInfo);
    OutputDebugStringA("\n");

    y += 20;

    for (int i = 0; i < 12; i++) {
        bool keyState = Keyboard_IsKeyDown((Keyboard_Keys)(KK_F1 + i));
        sprintf_s(keyInfo, "F%d: %s", i + 1, keyState ? "DOWN" : "UP");
        if (keyState) {
            OutputDebugStringA(keyInfo);
            OutputDebugStringA("\n");
        }
        y += 20;
    }

    bool shiftState = Keyboard_IsKeyDown(KK_LEFTSHIFT) || Keyboard_IsKeyDown(KK_RIGHTSHIFT);
    bool ctrlState = Keyboard_IsKeyDown(KK_LEFTCONTROL) || Keyboard_IsKeyDown(KK_RIGHTCONTROL);

    sprintf_s(keyInfo, "SHIFT: %s  CTRL: %s",
        shiftState ? "DOWN" : "UP",
        ctrlState ? "DOWN" : "UP");

    if (shiftState || ctrlState) {
        OutputDebugStringA(keyInfo);
        OutputDebugStringA("\n");
    }
}

void debugSprite(XMFLOAT3 position, float rotate, XMFLOAT2 size, XMFLOAT4 color)
{
    XMMATRIX TranslationMatrix = XMMatrixTranslation(position.x, position.y, 0.0f);
    XMMATRIX RotationMatrix = XMMatrixRotationZ(XMConvertToRadians(rotate));
    XMMATRIX ScalingMatrix = XMMatrixIdentity();
    XMMATRIX WorldMatrix = ScalingMatrix * RotationMatrix * TranslationMatrix;
    SetWorldMatrix(WorldMatrix);
    DrawSprite(size, color);
}

void SetPause(bool flg)
{
    pause = flg;
}

bool GetPause()
{
    return pause;
}

void InitGame()
{
    // 🔧 场景管理器初始化
    InitSceneManager();

    // 🔧 基础系统初始化
    GeometricTextRenderer::Init();
    TextureManager::Init();
    // 🔧 地图和环境初始化
    InitBox();
    InitBg();
    InitSkyDome();
    InitProceduralModels();

    // 🔧 角色和场景初始化
    InitPixelCharacters();
    InitCharacterScene();

    // 🔧 战斗系统初始化
    InitBattleHall();
    InitMultiplayerBattle();  // 调用MultiplayerBattle.cpp中的函数

    // 🔧 游戏对象初始化
    InitBall();

    // 🔧 相机系统初始化
    InitCamera();

    // 🔧 特效系统初始化
    ParticleSystem::Init();
    FloatingTextSystem::Init();

    // 🔧 UI系统初始化
    UIManager::Init();

    // 🔧 纹理管理初始化
    TextureAtlasManager::GetInstance();

    //Mouse_SetVisible(true);  // 强制显示鼠标

    OutputDebugStringA("======= Game Initialized =======\n");
    OutputDebugStringA("All systems: Ready\n");
    OutputDebugStringA("Press F11 to activate Camera Debug Tool\n");
    OutputDebugStringA("Press P to print camera parameters\n");
    OutputDebugStringA("================================\n");
}

void UninitGame()
{
    // 🔧 按照初始化的反序进行清理

    // 纹理管理清理
    TextureAtlasManager::DestroyInstance();

    // UI系统清理
    UIManager::Uninit();

    // 特效系统清理
    FloatingTextSystem::Uninit();
    ParticleSystem::Uninit();

    // 相机系统清理
    UninitCamera();

    // 游戏对象清理
    UninitBall();

    // 战斗系统清理
    UninitMultiplayerBattle();  // 调用MultiplayerBattle.cpp中的函数
    UninitBattleHall();

    // 角色和场景清理
    UninitCharacterScene();
    UninitPixelCharacters();

    // 地图和环境清理
    UninitProceduralModels();
    UninitSkyDome();
    UninitBg();
    UninitBox();
    TextureManager::Uninit();
    // 基础系统清理
    GeometricTextRenderer::Uninit();

    // 场景管理器清理
    UninitSceneManager();

    OutputDebugStringA("Game Uninitialized\n");
}

void UpdateGame()
{

    Camera* pcamera = GetCamera();

    // 🔧 场景管理器更新（处理模式切换等）
    UpdateSceneManager(g_DeltaTime);

    // 🔧 角色场景更新
    UpdateCharacterScene(g_DeltaTime);
    
    // 获取当前游戏模式
    AppMode currentMode = GetCurrentAppMode();

    // 🔧 根据模式进行相应的更新
    if (currentMode != APP_MODE_PAUSE &&
        currentMode != APP_MODE_GAMEOVER &&
        currentMode != APP_MODE_CHARACTER)
    {
        // 非暂停状态下更新游戏
        UpdateDeltaTime();

        // 🔧 基础系统更新
        UpdateCamera();
        UpdateSkyDome(g_DeltaTime);
        UpdateProceduralModels(g_DeltaTime);

        // 🔧 根据不同模式更新不同的游戏内容
        switch (currentMode) {
        case APP_MODE_GAME:
            // 单人游戏模式
            UpdateBox();
            UpdatePixelCharacters(g_DeltaTime);
            UpdateBall();
            ParticleSystem::Update(g_DeltaTime);
            FloatingTextSystem::Update(g_DeltaTime);

            // 更新金币显示
            UpdateSinglePlayerCoins();
            break;

        case APP_MODE_MULTIPLAYER_BATTLE: {
            // 🔧 多人战斗模式（具体实现在MultiplayerBattle.cpp中）
            UpdateBox();
            UpdatePixelCharacters(g_DeltaTime);
            ParticleSystem::Update(g_DeltaTime);
            FloatingTextSystem::Update(g_DeltaTime);
            UpdateMultiplayerBattle(g_DeltaTime);

            // 🔧 定期调试多人战斗状态
            static int battleDebugCounter = 0;
            if (++battleDebugCounter % 600 == 0) { // 每10秒调试一次
                DebugMultiplayerBattle();
            }
            break;
        }
        case APP_MODE_EDIT:
            // 编辑模式
            UpdateBox();
            UpdateBoxFade(g_DeltaTime);
            break;

        case APP_MODE_PREVIEW:
            // 预览模式
            UpdateBox();
            UpdateBall();
            break;

        case APP_MODE_BATTLE_HALL:
            // 战斗大厅模式
            UpdateBattleHall(g_DeltaTime);
            break;
        }

        // 🔧 通用系统更新
        UIManager::Update(g_DeltaTime);

        // 🔧 调试功能更新
        UpdateDebugControls();
    }
    else {
        // 🔧 暂停状态或特殊模式下的更新
        if (currentMode == APP_MODE_PAUSE || currentMode == APP_MODE_GAMEOVER) {
            UIManager::Update(g_DeltaTime);
        }
        else if (currentMode == APP_MODE_CHARACTER) {
            UpdateCharacterScene(g_DeltaTime);
        }
    }

    g_FrameCount++;
    UIManager::HandleMouseEvents();
}

void DrawGame()
{
    // 防止重复绘制
    static int lastFrameDrawn = -1;
    if (lastFrameDrawn == g_FrameCount) {
        OutputDebugStringA("错误：DrawGame在同一帧内被多次调用！\n");
        return;
    }
    lastFrameDrawn = g_FrameCount;

    if (g_IsRenderingBox) {
        OutputDebugStringA("警告：DrawBox函数被重复调用！\n");
        return;
    }
    g_IsRenderingBox = true;

    AppMode currentMode = GetCurrentAppMode();

    // 🔧 第一阶段：3D场景渲染到纹理
    Draw3DScene(currentMode);

    // 🔧 第二阶段：2D UI和后处理渲染到屏幕
    Draw2DOverlay(currentMode);

    g_IsRenderingBox = false;
}

// 🔧 分离的3D场景绘制函数
void Draw3DScene(AppMode currentMode)
{
    BeginPE(); // 切换到渲染纹理

    SetWorldViewProjection3D();
    SetDepthEnable(true);

    // 绘制天空盒
    DrawSkyDome();

    // 🔧 根据模式绘制不同的3D内容
    switch (currentMode) {
    case APP_MODE_BATTLE_HALL:
        DrawBattleHall();
        break;

    case APP_MODE_CHARACTER:
        if (IsCharacterSceneActive()) {
            Camera* camera = GetCamera();
            SetViewMatrix(camera->ViewMatrix);
            SetProjectionMatrix(camera->ProjectionMatrix);
            DrawCharacterScene();
        }
        break;

    case APP_MODE_MULTIPLAYER_BATTLE:

        DrawMultiplayerBattleScene();
        break;

    default:
        DrawStandardGameScene(currentMode);
        break;
    }
}

// 🔧 分离的2D覆盖层绘制函数
void Draw2DOverlay(AppMode currentMode)
{
    Begin(); // 切换回默认渲染目标

    SetWorldViewProjection2D();
    SetDepthEnable(false);

    ID3D11ShaderResourceView* ptexture = GetPETexture();

    // 🔧 根据模式应用不同的后处理效果
    switch (currentMode) {
    case APP_MODE_EDIT:
        DrawEditModeOverlay(ptexture);
        break;

    case APP_MODE_PAUSE:
        DrawPauseModeOverlay(ptexture);
        break;

    case APP_MODE_GAMEOVER:
        DrawGameOverOverlay(ptexture);
        break;

    case APP_MODE_MULTIPLAYER_BATTLE:
        // 🔧 多人战斗模式覆盖层（调用专门的绘制函数）
        DrawMultiplayerBattleOverlay(ptexture);
        break;

    default:
        DrawStandardGameOverlay(ptexture, currentMode);
        break;
    }

    // 🔧 最后绘制UI
    SetDepthEnable(false);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);
    SetWorldViewProjection2D();

    ID3D11ShaderResourceView* nullSRV = NULL;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    UIManager::Draw();
}

// 🔧 更新单人游戏金币逻辑
void UpdateSinglePlayerCoins()
{
    static int lastCoinCount = 0;
    BallObject* ball = GetBall();
    if (ball && ball->collectedCoins != lastCoinCount) {
        UIManager::UpdateCoinDisplay(ball->collectedCoins);
        IncreasePlayerScore(100);
        lastCoinCount = ball->collectedCoins;

        if (ball->collectedCoins > 0) {
            ParticleSystem::CreateEffect(
                ball->position,
                10,
                XMFLOAT4(1.0f, 0.84f, 0.0f, 1.0f),
                0.05f,
                0.1f,
                0.5f
            );
        }
    }
}

// 🔧 调试控制更新
void UpdateDebugControls()
{
    static bool lastF11 = false;
    bool nowF11 = Keyboard_IsKeyDown(KK_F11);
    if (nowF11 && !lastF11) {
        DebugPrint("按下F11 - 切换相机调试工具\n");
    }
    lastF11 = nowF11;

    static bool lastP = false;
    bool nowP = Keyboard_IsKeyDown(KK_P);
    if (nowP && !lastP) {
        DebugPrint("按下P - 打印相机参数\n");
        PrintCameraParameters();
    }
    lastP = nowP;
}

// 🔧 标准游戏场景绘制
void DrawStandardGameScene(AppMode currentMode)
{
    DrawCamera();

    if (currentMode != APP_MODE_GAMEOVER) {
        DrawBox();

        if (currentMode == APP_MODE_GAME || currentMode == APP_MODE_PREVIEW) {
            DrawBall();
            ParticleSystem::Draw();
            FloatingTextSystem::Draw();
        }
    }
    else {
        // 游戏结束特殊视角
        DrawGameOverScene();
    }
}

// 🔧 游戏结束场景绘制
void DrawGameOverScene()
{
    static float rotation = 0.0f;
    rotation += 0.005f;
    if (rotation > 6.28f) rotation = 0.0f;

    Camera* cam = GetCamera();
    XMFLOAT3 originalPos = cam->Position;
    XMFLOAT3 originalAt = cam->AtPosition;

    float radius = 40.0f;
    cam->Position.x = sinf(rotation) * radius;
    cam->Position.z = cosf(rotation) * radius;
    cam->Position.y = 30.0f + sinf(rotation * 0.5f) * 10.0f;
    cam->AtPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
    SetViewMatrix(cam->ViewMatrix);

    DrawBox();

    cam->Position = originalPos;
    cam->AtPosition = originalAt;
    cam->ViewMatrix = XMMatrixLookAtLH(
        XMVectorSet(originalPos.x, originalPos.y, originalPos.z, 1.0f),
        XMVectorSet(originalAt.x, originalAt.y, originalAt.z, 1.0f),
        XMVectorSet(cam->UpVector.x, cam->UpVector.y, cam->UpVector.z, 0.0f)
    );
    SetViewMatrix(cam->ViewMatrix);
}

// 🔧 编辑模式覆盖层
void DrawEditModeOverlay(ID3D11ShaderResourceView* ptexture)
{
    GetDeviceContext()->PSSetShaderResources(0, 1, &ptexture);
    XMMATRIX WorldMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(WorldMatrix);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT), XMFLOAT4(1, 1, 1, 1));

    ID3D11ShaderResourceView* nullSRV = NULL;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    DrawUIBoxes();
    DrawObstacleUIBoxes();
}

// 🔧 暂停模式覆盖层
void DrawPauseModeOverlay(ID3D11ShaderResourceView* ptexture)
{
    GetDeviceContext()->PSSetShaderResources(0, 1, &ptexture);
    XMMATRIX WorldMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(WorldMatrix);

    XMFLOAT4 pauseColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.8f);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT), pauseColor);

    XMMATRIX pausePanel = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(pausePanel);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH * 0.7f, SCREEN_HEIGHT * 0.7f), XMFLOAT4(0.1f, 0.1f, 0.3f, 0.7f));
}

// 🔧 游戏结束覆盖层
void DrawGameOverOverlay(ID3D11ShaderResourceView* ptexture)
{
    GetDeviceContext()->PSSetShaderResources(0, 1, &ptexture);
    XMMATRIX WorldMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(WorldMatrix);

    XMFLOAT4 gameOverColor = XMFLOAT4(0.7f, 0.3f, 0.3f, 0.8f);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT), gameOverColor);

    XMMATRIX gameOverPanel = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(gameOverPanel);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH * 0.7f, SCREEN_HEIGHT * 0.7f), XMFLOAT4(0.3f, 0.1f, 0.1f, 0.7f));
}

// 🔧 标准游戏覆盖层（包含传统小地图）
void DrawStandardGameOverlay(ID3D11ShaderResourceView* ptexture, AppMode currentMode)
{
    GetDeviceContext()->PSSetShaderResources(0, 1, &ptexture);
    XMMATRIX WorldMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(WorldMatrix);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT), XMFLOAT4(1, 1, 1, 1));

    ID3D11ShaderResourceView* nullSRV = NULL;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    // 🔧 只在单人模式下显示传统小地图
    if (currentMode == APP_MODE_GAME || currentMode == APP_MODE_PREVIEW) {
        DrawTraditionalMinimap(ptexture);  // 调用专门的小地图绘制函数
    }
}

void DrawTraditionalMinimap(ID3D11ShaderResourceView* ptexture)
{
    // 绘制30x30地图的小地图
    const float MINIMAP_SIZE = 200.0f;
    const float MARGIN = 20.0f;

    // 设置小地图位置
    XMMATRIX worldMinimap = XMMatrixTranslation(
        SCREEN_WIDTH - MINIMAP_SIZE / 2 - MARGIN,
        MINIMAP_SIZE / 2 + MARGIN,
        0.0f);
    SetWorldMatrix(worldMinimap);

    if (ptexture != nullptr) {
        GetDeviceContext()->PSSetShaderResources(0, 1, &ptexture);
        DrawSprite(XMFLOAT2(MINIMAP_SIZE, MINIMAP_SIZE), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.7f));

        ID3D11ShaderResourceView* nullSRV = nullptr;
        GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
    }
}

void UpdateDeltaTime()
{
    static bool first = true;
    auto now = steady_clock::now();
    if (first) {
        g_LastTime = now;
        first = false;
    }
    duration<float> diff = now - g_LastTime;
    g_DeltaTime = diff.count();
    g_LastTime = now;
}


// 🔧 **新增**: 专门的多人战斗调试函数
void DebugMultiplayerBattle() {
    if (GetCurrentAppMode() != APP_MODE_MULTIPLAYER_BATTLE) return;

    // 输出当前战斗状态
    char debugInfo[512];
    sprintf_s(debugInfo,
        "🎮 多人战斗调试信息:\n"
        "- 活跃玩家数: %d\n"
        "- 战斗状态: %d\n"
        "- 战斗模式: %s\n"
        "- 地图尺寸: %dx%d -> 显示为15x15\n",
        MultiplayerBattleManager::GetActivePlayerCount(),
        (int)MultiplayerBattleManager::GetBattleState(),
        MultiplayerBattleManager::GetIsBattleMode() ? "激活" : "未激活",
        BATTLE_MAP_SIZE_X, BATTLE_MAP_SIZE_Z
    );
    OutputDebugStringA(debugInfo);

    // 验证地图数据
    int nonZeroGroundTiles = 0;
    int nonZeroObstacleTiles = 0;

    for (int z = 0; z < BATTLE_MAP_SIZE_Z; z++) {
        for (int x = 0; x < BATTLE_MAP_SIZE_X; x++) {
            if (MultiplayerBattleMap::GetBattleGroundTile(x, z) != 0) {
                nonZeroGroundTiles++;
            }
            if (MultiplayerBattleMap::GetBattleObstacleTile(x, z) != 0) {
                nonZeroObstacleTiles++;
            }
        }
    }

    char mapDebug[256];
    sprintf_s(mapDebug,
        "🗺️ 30x30战斗地图统计:\n"
        "- 非空地形格子: %d/%d\n"
        "- 障碍物格子: %d/%d\n",
        nonZeroGroundTiles, BATTLE_MAP_SIZE_X * BATTLE_MAP_SIZE_Z,
        nonZeroObstacleTiles, BATTLE_MAP_SIZE_X * BATTLE_MAP_SIZE_Z
    );
    OutputDebugStringA(mapDebug);
}