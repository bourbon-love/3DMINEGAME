
#include "SceneRenderer.h"
#include "Renderer.h"
#include "Camera.h"
#include "Box.h"
#include "ball.h"
#include "Bg.h"
#include "SkyDome.h"
#include "ParticlaEffect.h"
#include "UIManager.h"
#include "CharactorScene.h"
#include "MapEditor.h"
#include "keyboard.h"
#include "Game.h"
#include  "BattleHall.h"




void SceneRenderer::InitAllScenes() {
    // 初始化所有共用组件
    InitBox();
    InitBg();
    InitBall();
    InitCamera();
    InitSkyDome();
    InitProceduralModels();
    InitPixelCharacters();
    InitCharacterScene();

    // 初始化系统
    ParticleSystem::Init();
    FloatingTextSystem::Init();
    UIManager::Init();

    OutputDebugStringA("所有场景初始化完成\n");
}

void SceneRenderer::UninitAllScenes() {
    // 清理所有组件
    UninitBox();
    UninitBg();
    UninitBall();
    UninitPixelCharacters();
    UninitCharacterScene();
    UninitCamera();
    UninitSkyDome();
    UninitProceduralModels();

    // 清理系统
    ParticleSystem::Uninit();
    FloatingTextSystem::Uninit();
    UIManager::Uninit();

    TextureAtlasManager::DestroyInstance();

    OutputDebugStringA("所有场景已清理\n");
}

void SceneRenderer::UpdateCurrentScene(float deltaTime) {
    AppMode currentMode = GetCurrentAppMode();

    // 更新通用系统
    UpdateSceneManager(deltaTime);
    UpdateCharacterScene(deltaTime);

    // 根据模式更新对应场景
    switch (currentMode) {
    case APP_MODE_PREVIEW:
        UpdatePreviewScene(deltaTime);
        break;

    case APP_MODE_GAME:
        UpdateGameScene(deltaTime);
        break;

    case APP_MODE_EDIT:
        UpdateEditScene(deltaTime);
        break;

    case APP_MODE_PAUSE:
        UpdatePauseScene(deltaTime);
        break;

    case APP_MODE_GAMEOVER:
        UpdateGameOverScene(deltaTime);
        break;

    case APP_MODE_CHARACTER:
        UpdateCharacterScene(deltaTime);
        break;
     case APP_MODE_BATTLE_HALL:  // 新增
         UpdateBattleHallScene(deltaTime);
         break;
    }
    // 更新UI系统（所有模式都需要）
    UIManager::Update(deltaTime);
    UIManager::HandleMouseEvents();
}

void SceneRenderer::DrawCurrentScene() {
    AppMode currentMode = GetCurrentAppMode();

    // 根据模式绘制对应场景
    switch (currentMode) {
    case APP_MODE_PREVIEW:
        DrawPreviewScene();
        break;

    case APP_MODE_GAME:
        DrawGameScene();
        break;

    case APP_MODE_EDIT:
        DrawEditScene();
        break;

    case APP_MODE_PAUSE:
        DrawPauseScene();
        break;

    case APP_MODE_GAMEOVER:
        DrawGameOverScene();
        break;

    case APP_MODE_CHARACTER:
        DrawCharacterScene();
        break;

    case APP_MODE_BATTLE_HALL:  // 新增
        DrawBattleHallScene();
        break;
    }
}

// ========================================
// 预览场景实现
// ========================================

void SceneRenderer::UpdatePreviewScene(float deltaTime) {
    // 更新通用组件
    UpdateCamera();
    UpdateSkyDome(deltaTime);
    UpdateProceduralModels(deltaTime);

    // 预览模式特有的更新
    UpdateBox(); // 地图显示

    // 键盘输入处理
    static bool lastF11 = false;
    bool nowF11 = Keyboard_IsKeyDown(KK_F11);
    if (nowF11 && !lastF11) {
      //  DebugPrint("预览模式：按下F11 - 切换相机调试工具\n");
    }
    lastF11 = nowF11;
}

void SceneRenderer::DrawPreviewScene() {
    // 第一阶段：渲染到纹理
    BeginPE();
    SetWorldViewProjection3D();
    SetDepthEnable(true);

    // 绘制3D环境
    Draw3DEnvironment();

    // 第二阶段：渲染到屏幕
    Begin();
    SetWorldViewProjection2D();
    SetDepthEnable(false);

    // 全屏显示主视图
    ID3D11ShaderResourceView* texture = GetPETexture();
    GetDeviceContext()->PSSetShaderResources(0, 1, &texture);

    XMMATRIX WorldMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(WorldMatrix);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT), XMFLOAT4(1, 1, 1, 1));

    // 解绑纹理
    ID3D11ShaderResourceView* nullSRV = NULL;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    // 绘制UI
    DrawUI();
}

// ========================================
// 游戏场景实现
// ========================================

void SceneRenderer::UpdateGameScene(float deltaTime) {
    // 更新通用组件
    UpdateCamera();
    UpdateSkyDome(deltaTime);
    UpdateProceduralModels(deltaTime);

    // 游戏特有的更新
    UpdateBox();
    UpdatePixelCharacters(deltaTime);
    UpdateBall();

    // 更新特效系统
    ParticleSystem::Update(deltaTime);
    FloatingTextSystem::Update(deltaTime);

    // 更新金币显示
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

void SceneRenderer::DrawGameScene() {
    // 第一阶段：渲染到纹理
    BeginPE();
    SetWorldViewProjection3D();
    SetDepthEnable(true);

    // 绘制3D环境
    Draw3DEnvironment();

    // 绘制游戏对象
    DrawCamera();
    DrawBox();
    DrawBall();
    ParticleSystem::Draw();
    FloatingTextSystem::Draw();

    // 第二阶段：渲染到屏幕
    Begin();
    SetWorldViewProjection2D();
    SetDepthEnable(false);

    // 全屏显示主视图
    ID3D11ShaderResourceView* texture = GetPETexture();
    GetDeviceContext()->PSSetShaderResources(0, 1, &texture);

    XMMATRIX WorldMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(WorldMatrix);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT), XMFLOAT4(1, 1, 1, 1));

    // 解绑纹理后绘制小地图
    ID3D11ShaderResourceView* nullSRV = NULL;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    // 绘制小地图
    const float MINIMAP_SIZE = 200.0f;
    const float MARGIN = 20.0f;
    ID3D11ShaderResourceView* minimapTexture = GetPETexture();
    if (minimapTexture != NULL) {
        GetDeviceContext()->PSSetShaderResources(0, 1, &minimapTexture);

        XMMATRIX worldMinimap = XMMatrixScaling(1.0f, 1.0f, 1.0f) *
            XMMatrixTranslation(SCREEN_WIDTH - MINIMAP_SIZE / 2 - MARGIN,
                MINIMAP_SIZE / 2 + MARGIN, 0.0f);
        SetWorldMatrix(worldMinimap);

        DrawSprite(XMFLOAT2(MINIMAP_SIZE, MINIMAP_SIZE), XMFLOAT4(1.0f, 1.0f, 1.0f, 0.7f));
    }

    // 绘制UI
    DrawUI();
}

// ========================================
// 编辑场景实现
// ========================================

void SceneRenderer::UpdateEditScene(float deltaTime) {
    // 更新通用组件
    UpdateCamera();
    UpdateSkyDome(deltaTime);
    UpdateProceduralModels(deltaTime);

    // 编辑模式特有的更新
    UpdateBox();
    UpdateBoxFade(deltaTime);
}

void SceneRenderer::DrawEditScene() {
    // 第一阶段：渲染到纹理
    BeginPE();
    SetWorldViewProjection3D();
    SetDepthEnable(true);

    // 绘制3D环境
    Draw3DEnvironment();
    DrawCamera();
    DrawBox();

    // 第二阶段：渲染到屏幕
    Begin();
    SetWorldViewProjection2D();
    SetDepthEnable(false);

    // 全屏显示纹理
    ID3D11ShaderResourceView* texture = GetPETexture();
    GetDeviceContext()->PSSetShaderResources(0, 1, &texture);

    XMMATRIX WorldMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(WorldMatrix);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT), XMFLOAT4(1, 1, 1, 1));

    // 解绑纹理
    ID3D11ShaderResourceView* nullSRV = NULL;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    // 绘制编辑模式UI
    DrawUIBoxes();
    DrawObstacleUIBoxes();

    // 绘制通用UI
    DrawUI();
}

// ========================================
// 其他场景实现
// ========================================

void SceneRenderer::UpdatePauseScene(float deltaTime) {
    // 暂停模式下只更新UI
    // 其他系统保持暂停状态
}

void SceneRenderer::DrawPauseScene() {
    // 绘制暂停场景的特殊效果
    BeginPE();
    SetWorldViewProjection3D();
    SetDepthEnable(true);

    Draw3DEnvironment();
    DrawCamera();
    DrawBox();
    DrawBall();

    Begin();
    SetWorldViewProjection2D();
    SetDepthEnable(false);

    // 绘制带灰色滤镜的主场景
    ID3D11ShaderResourceView* texture = GetPETexture();
    GetDeviceContext()->PSSetShaderResources(0, 1, &texture);

    XMMATRIX WorldMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(WorldMatrix);

    XMFLOAT4 pauseColor = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.8f);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT), pauseColor);

    // 绘制暂停面板
    XMMATRIX pausePanel = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(pausePanel);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH * 0.7f, SCREEN_HEIGHT * 0.7f),
        XMFLOAT4(0.1f, 0.1f, 0.3f, 0.7f));

    DrawUI();
}

void SceneRenderer::UpdateGameOverScene(float deltaTime) {
    // 游戏结束场景的更新逻辑
    // 可能包含特殊的相机动画等
}

void SceneRenderer::DrawGameOverScene() {
    // 类似暂停场景，但使用红色滤镜
    BeginPE();
    SetWorldViewProjection3D();
    SetDepthEnable(true);

    // 绘制游戏结束时的特殊视角
    static float rotation = 0.0f;
    rotation += 0.005f;
    if (rotation > 6.28f) rotation = 0.0f;

    Camera* cam = GetCamera();
    XMFLOAT3 originalPos = cam->Position;
    XMFLOAT3 originalAt = cam->AtPosition;

    // 环绕地图中心点
    float radius = 40.0f;
    cam->Position.x = sinf(rotation) * radius;
    cam->Position.z = cosf(rotation) * radius;
    cam->Position.y = 30.0f + sinf(rotation * 0.5f) * 10.0f;
    cam->AtPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);
    SetViewMatrix(cam->ViewMatrix);

    Draw3DEnvironment();
    DrawBox();

    // 恢复相机状态
    cam->Position = originalPos;
    cam->AtPosition = originalAt;

    Begin();
    SetWorldViewProjection2D();
    SetDepthEnable(false);

    // 红色滤镜
    ID3D11ShaderResourceView* texture = GetPETexture();
    GetDeviceContext()->PSSetShaderResources(0, 1, &texture);

    XMMATRIX WorldMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(WorldMatrix);

    XMFLOAT4 gameOverColor = XMFLOAT4(0.7f, 0.3f, 0.3f, 0.8f);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT), gameOverColor);

    DrawUI();
}

void SceneRenderer::UpdateCharacterScene(float deltaTime) {
    // 角色选择场景的更新
    ::UpdateCharacterScene(deltaTime); // 调用原有的角色场景更新函数
}

void SceneRenderer::DrawCharacterScene() {
    // 确保相机设置正确
    Camera* camera = GetCamera();
    SetViewMatrix(camera->ViewMatrix);
    SetProjectionMatrix(camera->ProjectionMatrix);

    BeginPE();
    SetWorldViewProjection3D();
    SetDepthEnable(true);

    DrawSkyDome();
    ::DrawCharacterScene(); // 调用原有的角色场景绘制函数

    Begin();
    SetWorldViewProjection2D();
    SetDepthEnable(false);

    // 全屏显示
    ID3D11ShaderResourceView* texture = GetPETexture();
    GetDeviceContext()->PSSetShaderResources(0, 1, &texture);

    XMMATRIX WorldMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(WorldMatrix);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT), XMFLOAT4(1, 1, 1, 1));

    DrawUI();
}

// ========================================
// 通用函数实现
// ========================================

void SceneRenderer::Draw3DEnvironment() {
    DrawSkyDome();
}

void SceneRenderer::DrawUI() {
    SetDepthEnable(false);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);
    SetWorldViewProjection2D();

    // 清除任何纹理绑定
    ID3D11ShaderResourceView* nullSRV = NULL;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    // 绘制UI
    UIManager::Draw();
}

void SceneRenderer::UpdateBattleHallScene(float deltaTime)
{
    // 更新通用组件
    UpdateCamera();
    UpdateSkyDome(deltaTime);
    UpdateProceduralModels(deltaTime);

    // 更新像素角色
    UpdatePixelCharacters(deltaTime);

    // 战斗大厅特有的更新已在 BattleHallManager::Update 中处理
}


void SceneRenderer::DrawBattleHallScene()
{
    // 确保相机设置正确
    Camera* camera = GetCamera();
    SetViewMatrix(camera->ViewMatrix);
    SetProjectionMatrix(camera->ProjectionMatrix);

    BeginPE();
    SetWorldViewProjection3D();
    SetDepthEnable(true);

    // 绘制3D环境
    DrawSkyDome();

    // 绘制战斗大厅内容
    DrawBattleHall();

    Begin();
    SetWorldViewProjection2D();
    SetDepthEnable(false);

    // 全屏显示
    ID3D11ShaderResourceView* texture = GetPETexture();
    GetDeviceContext()->PSSetShaderResources(0, 1, &texture);

    XMMATRIX WorldMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0);
    SetWorldMatrix(WorldMatrix);
    DrawSprite(XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT), XMFLOAT4(1, 1, 1, 1));

    // 解绑纹理
    ID3D11ShaderResourceView* nullSRV = NULL;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

    // 绘制UI
    DrawUI();
}

