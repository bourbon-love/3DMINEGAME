// Minimap.cpp

#include "Minimap.h"
#include "keyboard.h"
#include "mouse.h"
#include "UIManager.h"
#include "SceneManager.h"
#include "MultiBattle/MultiplayerMap.h"
using namespace std;
// 静态成员变量定义
MinimapSettings MinimapManager::s_settings;
MINIMAP_TYPE MinimapManager::s_currentType = MINIMAP_TRADITIONAL;
bool MinimapManager::s_initialized = false;
float MinimapManager::s_fadeTimer = 1.0f;
bool MinimapManager::s_fadingIn = true;
XMFLOAT4 MinimapManager::s_viewBounds = XMFLOAT4(-30.0f, -30.0f, 30.0f, 30.0f);
bool MinimapManager::s_mouseInMinimap = false;
XMFLOAT2 MinimapManager::s_mouseMinimapPos = XMFLOAT2(0.0f, 0.0f);

// ========================================
// 初始化和清理函数
// ========================================

void MinimapManager::Init() {
    if (s_initialized) return;

    OutputDebugStringA("🗺️ 初始化小地图系统\n");

    // 初始化默认设置
    s_settings = MinimapSettings();

    // 根据屏幕分辨率调整小地图位置
    s_settings.position.x = SCREEN_WIDTH - s_settings.size - s_settings.margin;
    s_settings.position.y = s_settings.margin;

    s_currentType = MINIMAP_TRADITIONAL;
    s_fadeTimer = 1.0f;
    s_fadingIn = true;
    s_mouseInMinimap = false;

    s_initialized = true;

    OutputDebugStringA("✅ 小地图系统初始化完成\n");
}

void MinimapManager::Uninit() {
    if (!s_initialized) return;

    OutputDebugStringA("🗺️ 清理小地图系统\n");

    s_initialized = false;

    OutputDebugStringA("✅ 小地图系统清理完成\n");
}

void MinimapManager::Update(float deltaTime) {
    if (!s_initialized) return;

    // 更新淡入淡出效果
    UpdateFadeEffect(deltaTime);

    // 更新视野范围
    UpdateViewBounds();

    // 更新鼠标状态
    UpdateMouseState();

    // 处理键盘输入
    HandleKeyboardInput();
}

// ========================================
// 外部接口函数实现
// ========================================

void InitMinimap() {
    MinimapManager::Init();
}

void UninitMinimap() {
    MinimapManager::Uninit();
}

void UpdateMinimap(float deltaTime) {
    MinimapManager::Update(deltaTime);
}

void DrawGameMinimap(ID3D11ShaderResourceView* gameTexture) {
    if (!MinimapManager::IsVisible()) return;

    AppMode currentMode = GetCurrentAppMode();

    if (currentMode == APP_MODE_GAME || currentMode == APP_MODE_PREVIEW) {
        MinimapManager::DrawTraditionalMinimap(gameTexture);
    }
}

void DrawBattleMinimapGeneral() {
    if (!MinimapManager::IsVisible()) return;

    AppMode currentMode = GetCurrentAppMode();

    if (currentMode == APP_MODE_MULTIPLAYER_BATTLE) {
        MinimapManager::DrawMultiplayerBattleMinimap();
    }
}

void HandleMinimapInput() {
    MinimapManager::HandleKeyboardInput();
}

// ========================================
// 设置管理函数
// ========================================

void MinimapManager::SetSettings(const MinimapSettings& settings) {
    s_settings = settings;
}

MinimapSettings MinimapManager::GetSettings() {
    return s_settings;
}

void MinimapManager::SetVisible(bool visible) {
    s_settings.isVisible = visible;

    if (visible) {
        s_fadingIn = true;
        s_fadeTimer = 0.0f;
    }
    else {
        s_fadingIn = false;
        s_fadeTimer = 1.0f;
    }

    char debug[64];
    sprintf_s(debug, "🗺️ 小地图可见性: %s\n", visible ? "显示" : "隐藏");
    OutputDebugStringA(debug);
}

bool MinimapManager::IsVisible() {
    return s_settings.isVisible && (s_fadeTimer > 0.0f);
}

void MinimapManager::SetType(MINIMAP_TYPE type) {
    if (s_currentType != type) {
        s_currentType = type;

        char debug[64];
        sprintf_s(debug, "🗺️ 小地图类型切换: %s\n",
            type == MINIMAP_TRADITIONAL ? "传统模式" : "多人战斗模式");
        OutputDebugStringA(debug);
    }
}

MINIMAP_TYPE MinimapManager::GetType() {
    return s_currentType;
}

void MinimapManager::SetZoom(float zoom) {
    s_settings.zoomLevel = std::max(0.5f, std::min(3.0f, zoom));

    char debug[64];
    sprintf_s(debug, "🔍 小地图缩放: %.2f\n", s_settings.zoomLevel);
    OutputDebugStringA(debug);
}

float MinimapManager::GetZoom() {
    return s_settings.zoomLevel;
}

void MinimapManager::ZoomIn() {
    SetZoom(s_settings.zoomLevel * 1.2f);
}

void MinimapManager::ZoomOut() {
    SetZoom(s_settings.zoomLevel / 1.2f);
}

void MinimapManager::SetPosition(float x, float y) {
    s_settings.position.x = x;
    s_settings.position.y = y;
}

XMFLOAT2 MinimapManager::GetPosition() {
    return s_settings.position;
}

void MinimapManager::SetAlpha(float alpha) {
    s_settings.alpha = max(0.0f, min(1.0f, alpha));
}

float MinimapManager::GetAlpha() {
    return s_settings.alpha * s_fadeTimer;
}

// ========================================
// 显示选项控制
// ========================================

void MinimapManager::SetShowPlayerPosition(bool show) {
    s_settings.showPlayerPosition = show;
}

void MinimapManager::SetShowItems(bool show) {
    s_settings.showItems = show;
}

void MinimapManager::SetShowObstacles(bool show) {
    s_settings.showObstacles = show;
}

void MinimapManager::SetShowSpawnPoints(bool show) {
    s_settings.showSpawnPoints = show;
}

void MinimapManager::SetShowViewRange(bool show) {
    s_settings.showViewRange = show;
}

void MinimapManager::ToggleVisibility() {
    SetVisible(!s_settings.isVisible);
}

// ========================================
// 内部辅助函数
// ========================================

void MinimapManager::UpdateFadeEffect(float deltaTime) {
    if (s_settings.isVisible) {
        if (s_fadingIn && s_fadeTimer < 1.0f) {
            s_fadeTimer += deltaTime * 3.0f; // 淡入速度
            if (s_fadeTimer >= 1.0f) {
                s_fadeTimer = 1.0f;
                s_fadingIn = false;
            }
        }
    }
    else {
        if (!s_fadingIn && s_fadeTimer > 0.0f) {
            s_fadeTimer -= deltaTime * 3.0f; // 淡出速度
            if (s_fadeTimer <= 0.0f) {
                s_fadeTimer = 0.0f;
                s_fadingIn = true;
            }
        }
    }
}

void MinimapManager::UpdateViewBounds() {
    Camera* camera = GetCamera();
    if (camera == nullptr) {
        s_viewBounds = XMFLOAT4(-30.0f, -30.0f, 30.0f, 30.0f);
        return;
    }

    // 根据相机位置和视野角度计算可见区域
    float viewRange = 25.0f / s_settings.zoomLevel; // 缩放影响视野范围

    s_viewBounds = XMFLOAT4(
        camera->Position.x - viewRange,  // 左边界
        camera->Position.z - viewRange,  // 上边界
        camera->Position.x + viewRange,  // 右边界
        camera->Position.z + viewRange   // 下边界
    );
}

void MinimapManager::UpdateMouseState() {
    Mouse_State mouse;
    Mouse_GetState(&mouse);

    // 检查鼠标是否在小地图区域内
    s_mouseInMinimap = IsPointInMinimapArea((float)mouse.x, (float)mouse.y);

    if (s_mouseInMinimap) {
        // 计算鼠标在小地图上的相对位置
        float relativeX = ((float)mouse.x - s_settings.position.x) / s_settings.size;
        float relativeY = ((float)mouse.y - s_settings.position.y) / s_settings.size;

        s_mouseMinimapPos = XMFLOAT2(relativeX, relativeY);
    }
}

XMFLOAT2 MinimapManager::WorldToMinimapCoords(float worldX, float worldZ) {
    // 将世界坐标转换为小地图坐标
    float mapRange = 60.0f / s_settings.zoomLevel; // 地图范围受缩放影响

    float normalizedX = (worldX + mapRange * 0.5f) / mapRange;
    float normalizedZ = (worldZ + mapRange * 0.5f) / mapRange;

    // 限制在0-1范围内
    normalizedX = std::max(0.0f, min(1.0f, normalizedX));
    normalizedZ = max(0.0f, min(1.0f, normalizedZ));

    return XMFLOAT2(
        s_settings.position.x + normalizedX * s_settings.size,
        s_settings.position.y + normalizedZ * s_settings.size
    );
}

XMFLOAT2 MinimapManager::MinimapToWorldCoords(float minimapX, float minimapY) {
    // 将小地图坐标转换为世界坐标
    float relativeX = (minimapX - s_settings.position.x) / s_settings.size;
    float relativeY = (minimapY - s_settings.position.y) / s_settings.size;

    float mapRange = 60.0f / s_settings.zoomLevel;

    float worldX = (relativeX - 0.5f) * mapRange;
    float worldZ = (relativeY - 0.5f) * mapRange;

    return XMFLOAT2(worldX, worldZ);
}

bool MinimapManager::IsPointInMinimapArea(float x, float y) {
    return (x >= s_settings.position.x &&
        x <= s_settings.position.x + s_settings.size &&
        y >= s_settings.position.y &&
        y <= s_settings.position.y + s_settings.size);
}

// ========================================
// 查询函数
// ========================================

bool MinimapManager::IsMouseInMinimap() {
    return s_mouseInMinimap;
}

XMFLOAT2 MinimapManager::GetMouseMinimapPosition() {
    return s_mouseMinimapPos;
}

XMFLOAT2 MinimapManager::GetMouseWorldPosition() {
    if (!s_mouseInMinimap) {
        return XMFLOAT2(0.0f, 0.0f);
    }

    Mouse_State mouse;
    Mouse_GetState(&mouse);

    return MinimapToWorldCoords((float)mouse.x, (float)mouse.y);
}


// ========================================
// 主要绘制函数
// ========================================

void MinimapManager::DrawMinimap(MINIMAP_TYPE type, ID3D11ShaderResourceView* gameTexture) {
    if (!IsVisible()) return;

    switch (type) {
    case MINIMAP_TRADITIONAL:
        DrawTraditionalMinimap(gameTexture);
        break;
    case MINIMAP_MULTIPLAYER_BATTLE:
        DrawMultiplayerBattleMinimap();
        break;
    }
}

void MinimapManager::DrawTraditionalMinimap(ID3D11ShaderResourceView* gameTexture) {
    if (!IsVisible()) return;

    // 保存当前渲染状态
    SetDepthEnable(false);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);

    float currentAlpha = GetAlpha();

    // 1. 绘制小地图背景
    DrawMinimapBackground(s_settings.size, s_settings.position);

    // 2. 绘制游戏纹理（如果提供）
    if (gameTexture != nullptr) {
        DrawGameTextureOnMinimap(gameTexture);
    }
    else {
        // 绘制程序化地图内容
        DrawProceduralMapContent();
    }

    // 3. 绘制网格
    if (s_settings.zoomLevel > 1.5f) { // 只在放大时显示网格
        DrawMinimapGrid(s_settings.size, s_settings.position);
    }

    // 4. 绘制地图元素
    if (s_settings.showObstacles) {
        DrawObstaclesOnMinimap(s_settings.size, s_settings.position);
    }

    if (s_settings.showItems) {
        DrawItemsOnMinimap(s_settings.size, s_settings.position);
    }

    if (s_settings.showSpawnPoints) {
        DrawSpawnPointsOnMinimap(s_settings.size, s_settings.position);
    }

    // 5. 绘制玩家位置
    if (s_settings.showPlayerPosition) {
        DrawPlayerPositionOnMinimap(s_settings.size, s_settings.position);
    }

    // 6. 绘制视野范围
    if (s_settings.showViewRange) {
        DrawViewRangeOnMinimap(s_settings.size, s_settings.position);
    }

    // 7. 绘制边框
    DrawMinimapBorder(s_settings.size, s_settings.position,
        XMFLOAT4(0.8f, 0.8f, 0.8f, currentAlpha));

    // 8. 绘制小地图标题
    DrawMinimapTitle();

    // 9. 绘制工具提示（如果鼠标在小地图上）
    if (s_mouseInMinimap) {
        DrawMinimapTooltip();
    }
}

void MinimapManager::DrawMultiplayerBattleMinimap() {
    if (!IsVisible()) return;

    // 保存当前渲染状态
    SetDepthEnable(false);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);

    float currentAlpha = GetAlpha();

    // 1. 绘制小地图背景
    DrawMinimapBackground(s_settings.size, s_settings.position);

    // 2. 绘制战斗地图分界线
    DrawBattleMapBoundaries(s_settings.size, s_settings.position);

    // 3. 绘制地图区域（4个玩家区域用不同颜色）
    DrawBattleMapRegions();

    // 4. 绘制多人玩家位置
    DrawMultiplayerPositions(s_settings.size, s_settings.position);

    // 5. 绘制死亡标记
    DrawPlayerDeathMarkers(s_settings.size, s_settings.position);

    // 6. 绘制出生点
    if (s_settings.showSpawnPoints) {
        DrawMultiplayerSpawnPoints();
    }

    // 7. 绘制边框
    DrawMinimapBorder(s_settings.size, s_settings.position,
        XMFLOAT4(1.0f, 0.5f, 0.0f, currentAlpha)); // 橙色边框表示战斗模式

    // 8. 绘制战斗信息
    DrawBattleInfo();

    // 9. 绘制工具提示
    if (s_mouseInMinimap) {
        DrawBattleMinimapTooltip();
    }
}

// ========================================
// 基础绘制辅助函数
// ========================================

void DrawMinimapBackground(float size, XMFLOAT2 position) {
    XMMATRIX worldMatrix = XMMatrixTranslation(
        position.x + size * 0.5f,
        position.y + size * 0.5f,
        0.0f
    );
    SetWorldMatrix(worldMatrix);

    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = XMFLOAT4(0.1f, 0.1f, 0.1f, MinimapManager::GetAlpha() * 0.9f);
    SetMaterial(material);

    DrawSprite(XMFLOAT2(size, size), material.Diffuse);
}

void DrawMinimapBorder(float size, XMFLOAT2 position, XMFLOAT4 borderColor) {
    const float BORDER_WIDTH = 2.0f;

    float centerX = position.x + size * 0.5f;
    float centerY = position.y + size * 0.5f;

    // 上边框
    SetWorldMatrix(XMMatrixTranslation(centerX, position.y - BORDER_WIDTH * 0.5f, 0.0f));
    DrawSprite(XMFLOAT2(size + BORDER_WIDTH * 2, BORDER_WIDTH), borderColor);

    // 下边框
    SetWorldMatrix(XMMatrixTranslation(centerX, position.y + size + BORDER_WIDTH * 0.5f, 0.0f));
    DrawSprite(XMFLOAT2(size + BORDER_WIDTH * 2, BORDER_WIDTH), borderColor);

    // 左边框
    SetWorldMatrix(XMMatrixTranslation(position.x - BORDER_WIDTH * 0.5f, centerY, 0.0f));
    DrawSprite(XMFLOAT2(BORDER_WIDTH, size), borderColor);

    // 右边框
    SetWorldMatrix(XMMatrixTranslation(position.x + size + BORDER_WIDTH * 0.5f, centerY, 0.0f));
    DrawSprite(XMFLOAT2(BORDER_WIDTH, size), borderColor);
}

void DrawMinimapGrid(float size, XMFLOAT2 position) {
    const float GRID_LINE_WIDTH = 1.0f;
    const int GRID_DIVISIONS = 10;
    XMFLOAT4 gridColor = XMFLOAT4(0.3f, 0.3f, 0.3f, MinimapManager::GetAlpha() * 0.5f);

    float cellSize = size / GRID_DIVISIONS;

    // 绘制垂直线
    for (int i = 1; i < GRID_DIVISIONS; i++) {
        float x = position.x + i * cellSize;
        SetWorldMatrix(XMMatrixTranslation(x, position.y + size * 0.5f, 0.0f));
        DrawSprite(XMFLOAT2(GRID_LINE_WIDTH, size), gridColor);
    }

    // 绘制水平线
    for (int i = 1; i < GRID_DIVISIONS; i++) {
        float y = position.y + i * cellSize;
        SetWorldMatrix(XMMatrixTranslation(position.x + size * 0.5f, y, 0.0f));
        DrawSprite(XMFLOAT2(size, GRID_LINE_WIDTH), gridColor);
    }
}

void DrawGameTextureOnMinimap(ID3D11ShaderResourceView* gameTexture) {
    if (gameTexture == nullptr) return;

    MinimapSettings settings = MinimapManager::GetSettings();

    // 绑定游戏纹理
    GetDeviceContext()->PSSetShaderResources(0, 1, &gameTexture);

    XMMATRIX worldMatrix = XMMatrixTranslation(
        settings.position.x + settings.size * 0.5f,
        settings.position.y + settings.size * 0.5f,
        0.0f
    );
    SetWorldMatrix(worldMatrix);

    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, MinimapManager::GetAlpha() * 0.8f);
    SetMaterial(material);

    DrawSprite(XMFLOAT2(settings.size, settings.size), material.Diffuse);

    // 解绑纹理
    ID3D11ShaderResourceView* nullSRV = nullptr;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
}

void DrawProceduralMapContent() {
    // 绘制程序化生成的地图内容（当没有游戏纹理时）
    MinimapSettings settings = MinimapManager::GetSettings();

    extern int GROUND_MAP[MAPSIZE_Z][MAPSIZE_X];

    float cellSize = settings.size / MAPSIZE_X;

    for (int z = 0; z < MAPSIZE_Z; z++) {
        for (int x = 0; x < MAPSIZE_X; x++) {
            int terrainType = GROUND_MAP[z][x];
            if (terrainType == 0) continue; // 跳过空地

            XMFLOAT4 terrainColor = GetTerrainColorByType(terrainType);
            terrainColor.w *= MinimapManager::GetAlpha();

            float posX = settings.position.x + x * cellSize + cellSize * 0.5f;
            float posY = settings.position.y + z * cellSize + cellSize * 0.5f;

            SetWorldMatrix(XMMatrixTranslation(posX, posY, 0.0f));
            DrawSprite(XMFLOAT2(cellSize, cellSize), terrainColor);
        }
    }
}

// ========================================
// 标记和图标绘制函数
// ========================================

void DrawPlayerPositionOnMinimap(float size, XMFLOAT2 position) {
    BallObject* ball = GetBall();
    if (ball == nullptr) return;

    XMFLOAT2 playerPos = MinimapManager::WorldToMinimapCoords(ball->position.x, ball->position.z);

    // 绘制玩家位置标记
    SetWorldMatrix(XMMatrixTranslation(playerPos.x, playerPos.y, 0.0f));

    XMFLOAT4 playerColor = XMFLOAT4(1.0f, 0.2f, 0.2f, MinimapManager::GetAlpha());
    DrawSprite(XMFLOAT2(6.0f, 6.0f), playerColor);

    // 绘制方向指示器
    DrawPlayerDirectionIndicator(playerPos.x, playerPos.y, ball);
}

void DrawPlayerDirectionIndicator(float mapX, float mapY, BallObject* ball) {
    // 计算玩家朝向（简化版本）
    Camera* camera = GetCamera();
    if (camera == nullptr) return;

    // 基于相机朝向计算方向
    XMFLOAT3 forward = XMFLOAT3(
        camera->AtPosition.x - camera->Position.x,
        0.0f,
        camera->AtPosition.z - camera->Position.z
    );

    float length = sqrtf(forward.x * forward.x + forward.z * forward.z);
    if (length > 0.001f) {
        forward.x /= length;
        forward.z /= length;
    }

    float angle = atan2f(forward.x, forward.z);

    // 绘制方向箭头
    XMMATRIX directionMatrix = XMMatrixRotationZ(angle) *
        XMMatrixTranslation(mapX, mapY - 8.0f, 0.0f);
    SetWorldMatrix(directionMatrix);

    XMFLOAT4 directionColor = XMFLOAT4(1.0f, 1.0f, 0.2f, MinimapManager::GetAlpha() * 0.8f);
    DrawSprite(XMFLOAT2(3.0f, 10.0f), directionColor);
}

void DrawItemsOnMinimap(float size, XMFLOAT2 position) {
    // 示例：绘制一些固定的道具位置
    XMFLOAT3 itemPositions[] = {
        XMFLOAT3(0.0f, 0.0f, 0.0f),     // 中心道具
        XMFLOAT3(-10.0f, 0.0f, 10.0f),  // 其他道具位置
        XMFLOAT3(10.0f, 0.0f, -10.0f),
        XMFLOAT3(-15.0f, 0.0f, -15.0f),
        XMFLOAT3(15.0f, 0.0f, 15.0f)
    };

    XMFLOAT4 itemColor = XMFLOAT4(1.0f, 0.84f, 0.0f, MinimapManager::GetAlpha());

    for (int i = 0; i < 5; i++) {
        XMFLOAT2 itemPos = MinimapManager::WorldToMinimapCoords(
            itemPositions[i].x, itemPositions[i].z);

        // 检查是否在小地图范围内
        if (itemPos.x >= position.x && itemPos.x <= position.x + size &&
            itemPos.y >= position.y && itemPos.y <= position.y + size) {

            SetWorldMatrix(XMMatrixTranslation(itemPos.x, itemPos.y, 0.0f));
            DrawSprite(XMFLOAT2(4.0f, 4.0f), itemColor);

            // 添加闪烁效果
            static float itemTimer = 0.0f;
            itemTimer += 0.016f;
            if (((int)(itemTimer * 3)) % 2 == 0) {
                XMFLOAT4 glowColor = XMFLOAT4(1.0f, 1.0f, 0.5f, MinimapManager::GetAlpha() * 0.5f);
                DrawSprite(XMFLOAT2(6.0f, 6.0f), glowColor);
            }
        }
    }
}

void DrawObstaclesOnMinimap(float size, XMFLOAT2 position) {
    extern int OBSTACLE_MAP[MAPSIZE_Z][MAPSIZE_X];

    XMFLOAT4 obstacleColor = XMFLOAT4(0.4f, 0.4f, 0.4f, MinimapManager::GetAlpha() * 0.7f);
    float cellSize = size / MAPSIZE_X;

    // 采样显示主要障碍物
    for (int z = 0; z < MAPSIZE_Z; z += 2) {
        for (int x = 0; x < MAPSIZE_X; x += 2) {
            if (OBSTACLE_MAP[z][x] != 0) {
                float worldX = (x - MAPSIZE_X / 2) * BOXSIZE_X;
                float worldZ = -(z - MAPSIZE_Z / 2) * BOXSIZE_Z;

                XMFLOAT2 obstaclePos = MinimapManager::WorldToMinimapCoords(worldX, worldZ);

                // 检查是否在小地图范围内
                if (obstaclePos.x >= position.x && obstaclePos.x <= position.x + size &&
                    obstaclePos.y >= position.y && obstaclePos.y <= position.y + size) {

                    SetWorldMatrix(XMMatrixTranslation(obstaclePos.x, obstaclePos.y, 0.0f));
                    DrawSprite(XMFLOAT2(cellSize, cellSize), obstacleColor);
                }
            }
        }
    }
}

void DrawSpawnPointsOnMinimap(float size, XMFLOAT2 position) {
    // 单人模式的出生点
    XMFLOAT3 spawnPoint = XMFLOAT3(0.0f, 0.0f, 0.0f); // 假设出生点在中心

    XMFLOAT2 spawnPos = MinimapManager::WorldToMinimapCoords(spawnPoint.x, spawnPoint.z);

    if (spawnPos.x >= position.x && spawnPos.x <= position.x + size &&
        spawnPos.y >= position.y && spawnPos.y <= position.y + size) {

        SetWorldMatrix(XMMatrixTranslation(spawnPos.x, spawnPos.y, 0.0f));

        XMFLOAT4 spawnColor = XMFLOAT4(0.0f, 1.0f, 0.0f, MinimapManager::GetAlpha());
        DrawSprite(XMFLOAT2(8.0f, 8.0f), spawnColor);

        // 绘制十字标记
        DrawSprite(XMFLOAT2(12.0f, 2.0f), spawnColor);
        DrawSprite(XMFLOAT2(2.0f, 12.0f), spawnColor);
    }
}

void DrawViewRangeOnMinimap(float size, XMFLOAT2 position) {
    XMFLOAT4 viewBounds = MinimapManager::GetViewBounds();

    // 将视野边界转换为小地图坐标
    XMFLOAT2 topLeft = MinimapManager::WorldToMinimapCoords(viewBounds.x, viewBounds.y);
    XMFLOAT2 bottomRight = MinimapManager::WorldToMinimapCoords(viewBounds.z, viewBounds.w);

    // 限制在小地图范围内
    topLeft.x = max(position.x, min(position.x + size, topLeft.x));
    topLeft.y = max(position.y, min(position.y + size, topLeft.y));
    bottomRight.x = max(position.x, min(position.x + size, bottomRight.x));
    bottomRight.y = max(position.y, min(position.y + size, bottomRight.y));

    if (topLeft.x < bottomRight.x && topLeft.y < bottomRight.y) {
        float borderWidth = 1.0f;
        XMFLOAT4 viewColor = XMFLOAT4(1.0f, 1.0f, 0.0f, MinimapManager::GetAlpha() * 0.6f);

        float centerX = (topLeft.x + bottomRight.x) * 0.5f;
        float centerY = (topLeft.y + bottomRight.y) * 0.5f;
        float width = bottomRight.x - topLeft.x;
        float height = bottomRight.y - topLeft.y;

        // 绘制视野范围边框
        // 上边框
        SetWorldMatrix(XMMatrixTranslation(centerX, topLeft.y, 0.0f));
        DrawSprite(XMFLOAT2(width, borderWidth), viewColor);

        // 下边框
        SetWorldMatrix(XMMatrixTranslation(centerX, bottomRight.y, 0.0f));
        DrawSprite(XMFLOAT2(width, borderWidth), viewColor);

        // 左边框
        SetWorldMatrix(XMMatrixTranslation(topLeft.x, centerY, 0.0f));
        DrawSprite(XMFLOAT2(borderWidth, height), viewColor);

        // 右边框
        SetWorldMatrix(XMMatrixTranslation(bottomRight.x, centerY, 0.0f));
        DrawSprite(XMFLOAT2(borderWidth, height), viewColor);
    }
}

// ========================================
// 多人战斗专用绘制函数
// ========================================

void DrawBattleMapBoundaries(float size, XMFLOAT2 position) {
    const float BOUNDARY_WIDTH = 2.0f;
    XMFLOAT4 boundaryColor = XMFLOAT4(0.6f, 0.6f, 0.6f, MinimapManager::GetAlpha() * 0.8f);

    float centerX = position.x + size * 0.5f;
    float centerY = position.y + size * 0.5f;

    // 垂直分界线（分隔左右区域）
    SetWorldMatrix(XMMatrixTranslation(centerX, centerY, 0.0f));
    DrawSprite(XMFLOAT2(BOUNDARY_WIDTH, size), boundaryColor);

    // 水平分界线（分隔上下区域）
    SetWorldMatrix(XMMatrixTranslation(centerX, centerY, 0.0f));
    DrawSprite(XMFLOAT2(size, BOUNDARY_WIDTH), boundaryColor);
}

void DrawBattleMapRegions() {
    MinimapSettings settings = MinimapManager::GetSettings();

    // 4个玩家区域的颜色
    XMFLOAT4 regionColors[4] = {
        XMFLOAT4(1.0f, 0.3f, 0.3f, 0.2f),  // 红色区域 - 玩家1
        XMFLOAT4(0.3f, 0.3f, 1.0f, 0.2f),  // 蓝色区域 - 玩家2
        XMFLOAT4(0.3f, 1.0f, 0.3f, 0.2f),  // 绿色区域 - 玩家3
        XMFLOAT4(1.0f, 1.0f, 0.3f, 0.2f)   // 黄色区域 - 玩家4
    };

    float halfSize = settings.size * 0.5f;

    for (int i = 0; i < 4; i++) {
        regionColors[i].w *= MinimapManager::GetAlpha();

        float offsetX = (i % 2) * halfSize;
        float offsetY = (i / 2) * halfSize;

        XMMATRIX regionMatrix = XMMatrixTranslation(
            settings.position.x + offsetX + halfSize * 0.5f,
            settings.position.y + offsetY + halfSize * 0.5f,
            0.0f
        );
        SetWorldMatrix(regionMatrix);

        DrawSprite(XMFLOAT2(halfSize, halfSize), regionColors[i]);
    }
}

void DrawMultiplayerPositions(float size, XMFLOAT2 position) {
    for (int i = 0; i < 4; i++) {
        BattlePlayerInfo* playerInfo = MultiplayerBattleManager::GetPlayer((PlayerID)i);
        if (playerInfo && playerInfo->isActive) {
            XMFLOAT3 spawnPos = MultiplayerBattleMap::GetPlayerSpawnPosition((PlayerID)i);
            XMFLOAT2 playerMapPos = MinimapManager::WorldToMinimapCoords(spawnPos.x, spawnPos.z);

            if (playerMapPos.x >= position.x && playerMapPos.x <= position.x + size &&
                playerMapPos.y >= position.y && playerMapPos.y <= position.y + size) {

                SetWorldMatrix(XMMatrixTranslation(playerMapPos.x, playerMapPos.y, 0.0f));

                XMFLOAT4 playerColor = playerInfo->playerColor;
                playerColor.w *= MinimapManager::GetAlpha();

                DrawSprite(XMFLOAT2(8.0f, 8.0f), playerColor);

                // 绘制玩家编号
                DrawPlayerNumber(i + 1, playerMapPos.x, playerMapPos.y + 12.0f);
            }
        }
    }
}

void DrawPlayerDeathMarkers(float size, XMFLOAT2 position) {
    XMFLOAT4 deathColor = XMFLOAT4(1.0f, 0.2f, 0.2f, MinimapManager::GetAlpha());

    for (int i = 0; i < 4; i++) {
        BattlePlayerInfo* playerInfo = MultiplayerBattleManager::GetPlayer((PlayerID)i);
        if (playerInfo && playerInfo->isActive && !playerInfo->isActive) { // 已死亡的玩家
            XMFLOAT3 spawnPos = MultiplayerBattleMap::GetPlayerSpawnPosition((PlayerID)i);
            XMFLOAT2 deathMapPos = MinimapManager::WorldToMinimapCoords(spawnPos.x, spawnPos.z);

            if (deathMapPos.x >= position.x && deathMapPos.x <= position.x + size &&
                deathMapPos.y >= position.y && deathMapPos.y <= position.y + size) {

                // 绘制X形死亡标记
                XMMATRIX deathMatrix1 = XMMatrixRotationZ(XMConvertToRadians(45.0f)) *
                    XMMatrixTranslation(deathMapPos.x, deathMapPos.y, 0.0f);
                SetWorldMatrix(deathMatrix1);
                DrawSprite(XMFLOAT2(12.0f, 2.0f), deathColor);

                XMMATRIX deathMatrix2 = XMMatrixRotationZ(XMConvertToRadians(-45.0f)) *
                    XMMatrixTranslation(deathMapPos.x, deathMapPos.y, 0.0f);
                SetWorldMatrix(deathMatrix2);
                DrawSprite(XMFLOAT2(12.0f, 2.0f), deathColor);
            }
        }
    }
}

void DrawMultiplayerSpawnPoints() {
    MinimapSettings settings = MinimapManager::GetSettings();
    XMFLOAT4 spawnColor = XMFLOAT4(0.0f, 1.0f, 0.0f, MinimapManager::GetAlpha() * 0.8f);

    for (int i = 0; i < 4; i++) {
        XMFLOAT3 spawnPos = MultiplayerBattleMap::GetPlayerSpawnPosition((PlayerID)i);
        XMFLOAT2 spawnMapPos = MinimapManager::WorldToMinimapCoords(spawnPos.x, spawnPos.z);

        if (spawnMapPos.x >= settings.position.x && spawnMapPos.x <= settings.position.x + settings.size &&
            spawnMapPos.y >= settings.position.y && spawnMapPos.y <= settings.position.y + settings.size) {

            SetWorldMatrix(XMMatrixTranslation(spawnMapPos.x, spawnMapPos.y, 0.0f));
            DrawSprite(XMFLOAT2(6.0f, 6.0f), spawnColor);
        }
    }
}


// ========================================
// 交互功能实现
// ========================================

void MinimapManager::HandleMouseClick(int mouseX, int mouseY) {
    if (!IsPointInMinimapArea((float)mouseX, (float)mouseY)) {
        return;
    }

    XMFLOAT2 worldPos = MinimapToWorldCoords((float)mouseX, (float)mouseY);

    OutputDebugStringA("📍 小地图点击\n");
    char debug[128];
    sprintf_s(debug, "   世界坐标: (%.2f, %.2f)\n", worldPos.x, worldPos.y);
    OutputDebugStringA(debug);

    // 移动相机到点击位置
    MoveCameraToWorldPosition(worldPos.x, worldPos.y);
}

void MinimapManager::HandleMouseMove(int mouseX, int mouseY) {
    UpdateMouseState();
}

void MinimapManager::HandleKeyboardInput() {
    // M键切换小地图显示
    static bool lastMKey = false;
    bool nowMKey = Keyboard_IsKeyDown(KK_M);

    if (nowMKey && !lastMKey) {
        ToggleVisibility();
    }
    lastMKey = nowMKey;

    // Plus/Minus键控制缩放
    static bool lastPlusKey = false;
    static bool lastMinusKey = false;

    bool nowPlusKey = Keyboard_IsKeyDown(KK_UP);
    bool nowMinusKey = Keyboard_IsKeyDown(KK_DOWN);

    if (nowPlusKey && !lastPlusKey) {
        ZoomIn();
    }

    if (nowMinusKey && !lastMinusKey) {
        ZoomOut();
    }

    lastPlusKey = nowPlusKey;
    lastMinusKey = nowMinusKey;

    // Ctrl+数字键控制显示选项
    bool ctrlPressed = Keyboard_IsKeyDown(KK_LEFTCONTROL) || Keyboard_IsKeyDown(KK_RIGHTCONTROL);

    if (ctrlPressed) {
        static bool lastKey1 = false, lastKey2 = false, lastKey3 = false, lastKey4 = false;

        bool nowKey1 = Keyboard_IsKeyDown(KK_D1);
        bool nowKey2 = Keyboard_IsKeyDown(KK_D2);
        bool nowKey3 = Keyboard_IsKeyDown(KK_D3);
        bool nowKey4 = Keyboard_IsKeyDown(KK_D4);

        if (nowKey1 && !lastKey1) {
            s_settings.showPlayerPosition = !s_settings.showPlayerPosition;
            OutputDebugStringA("🗺️ 切换玩家位置显示\n");
        }

        if (nowKey2 && !lastKey2) {
            s_settings.showItems = !s_settings.showItems;
            OutputDebugStringA("🗺️ 切换道具显示\n");
        }

        if (nowKey3 && !lastKey3) {
            s_settings.showObstacles = !s_settings.showObstacles;
            OutputDebugStringA("🗺️ 切换障碍物显示\n");
        }

        if (nowKey4 && !lastKey4) {
            s_settings.showViewRange = !s_settings.showViewRange;
            OutputDebugStringA("🗺️ 切换视野范围显示\n");
        }

        lastKey1 = nowKey1;
        lastKey2 = nowKey2;
        lastKey3 = nowKey3;
        lastKey4 = nowKey4;
    }
}

// ========================================
// UI绘制函数
// ========================================

void DrawMinimapTitle() {
    MinimapSettings settings = MinimapManager::GetSettings();
    const wchar_t* title = (MinimapManager::GetType() == MINIMAP_TRADITIONAL) ?
        L"MAP" : L"BATTLE";

    GeometricTextRenderer::DrawMinecraftText(
        settings.position.x + 5,
        settings.position.y - 20,
        title,
        XMFLOAT4(1.0f, 1.0f, 1.0f, MinimapManager::GetAlpha()),
        1.0f,
        false
    );
}

void DrawBattleInfo() {
    MinimapSettings settings = MinimapManager::GetSettings();

    // 绘制战斗回合信息
    int currentRound = UIManager::GetBattleRound();
    wchar_t roundText[32];
    swprintf_s(roundText, L"Round %d", currentRound);

    GeometricTextRenderer::DrawMinecraftText(
        settings.position.x + settings.size - 60,
        settings.position.y - 20,
        roundText,
        XMFLOAT4(1.0f, 1.0f, 0.3f, MinimapManager::GetAlpha()),
        0.8f,
        false
    );

    // 绘制存活玩家数量
    int alivePlayers = 0;
    for (int i = 0; i < 4; i++) {
        BattlePlayerInfo* playerInfo = MultiplayerBattleManager::GetPlayer((PlayerID)i);
        if (playerInfo && playerInfo->isActive) {
            alivePlayers++;
        }
    }

    wchar_t aliveText[32];
    swprintf_s(aliveText, L"%d/4 Alive", alivePlayers);

    GeometricTextRenderer::DrawMinecraftText(
        settings.position.x + 5,
        settings.position.y + settings.size + 5,
        aliveText,
        XMFLOAT4(0.0f, 1.0f, 0.0f, MinimapManager::GetAlpha()),
        0.8f,
        false
    );
}

void DrawPlayerNumber(int playerNumber, float x, float y) {
    wchar_t numberText[4];
    swprintf_s(numberText, L"%d", playerNumber);

    GeometricTextRenderer::DrawMinecraftText(
        x - 5, y,
        numberText,
        XMFLOAT4(1.0f, 1.0f, 1.0f, MinimapManager::GetAlpha()),
        0.7f,
        false
    );
}

void DrawMinimapTooltip() {
    if (!MinimapManager::IsMouseInMinimap()) return;

    Mouse_State mouse;
    Mouse_GetState(&mouse);

    XMFLOAT2 worldPos = MinimapManager::GetMouseWorldPosition();

    wchar_t tooltipText[64];
    swprintf_s(tooltipText, L"(%.1f, %.1f)", worldPos.x, worldPos.y);

    DrawMinimapTooltip(mouse.x, mouse.y, tooltipText);
}

void DrawBattleMinimapTooltip() {
    if (!MinimapManager::IsMouseInMinimap()) return;

    Mouse_State mouse;
    Mouse_GetState(&mouse);

    XMFLOAT2 worldPos = MinimapManager::GetMouseWorldPosition();

    // 确定鼠标在哪个玩家区域
    int playerRegion = -1;
    if (worldPos.x < 0 && worldPos.y < 0) playerRegion = 0;
    else if (worldPos.x >= 0 && worldPos.y < 0) playerRegion = 1;
    else if (worldPos.x < 0 && worldPos.y >= 0) playerRegion = 2;
    else if (worldPos.x >= 0 && worldPos.y >= 0) playerRegion = 3;

    wchar_t tooltipText[128];
    if (playerRegion >= 0) {
        BattlePlayerInfo* playerInfo = MultiplayerBattleManager::GetPlayer((PlayerID)playerRegion);
        if (playerInfo && playerInfo->isActive) {
            swprintf_s(tooltipText, L"Player %d Zone\n(%.1f, %.1f)",
                playerRegion + 1, worldPos.x, worldPos.y);
        }
        else {
            swprintf_s(tooltipText, L"Empty Zone %d\n(%.1f, %.1f)",
                playerRegion + 1, worldPos.x, worldPos.y);
        }
    }
    else {
        swprintf_s(tooltipText, L"(%.1f, %.1f)", worldPos.x, worldPos.y);
    }
    DrawMinimapTooltip(mouse.x, mouse.y, tooltipText);
}

void DrawMinimapTooltip(int mouseX, int mouseY, const wchar_t* text) {
    const float TOOLTIP_PADDING = 5.0f;
    const float TOOLTIP_OFFSET_X = 15.0f;
    const float TOOLTIP_OFFSET_Y = -10.0f;

    // 计算工具提示大小
    XMFLOAT2 textSize = GeometricTextRenderer::MeasureText(text, 0.8f);
    XMFLOAT2 tooltipSize = XMFLOAT2(textSize.x + TOOLTIP_PADDING * 2,
        textSize.y + TOOLTIP_PADDING * 2);

    float tooltipX = mouseX + TOOLTIP_OFFSET_X;
    float tooltipY = mouseY + TOOLTIP_OFFSET_Y;

    // 确保工具提示不超出屏幕边界
    if (tooltipX + tooltipSize.x > SCREEN_WIDTH) {
        tooltipX = mouseX - TOOLTIP_OFFSET_X - tooltipSize.x;
    }

    if (tooltipY < 0) {
        tooltipY = mouseY - TOOLTIP_OFFSET_Y + 20;
    }

    // 绘制工具提示背景
    SetWorldMatrix(XMMatrixTranslation(tooltipX + tooltipSize.x * 0.5f,
        tooltipY + tooltipSize.y * 0.5f, 0.0f));

    XMFLOAT4 bgColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.8f * MinimapManager::GetAlpha());
    DrawSprite(tooltipSize, bgColor);

    // 绘制工具提示边框
    const float BORDER_WIDTH = 1.0f;
    XMFLOAT4 borderColor = XMFLOAT4(0.8f, 0.8f, 0.8f, MinimapManager::GetAlpha());

    // 上边框
    SetWorldMatrix(XMMatrixTranslation(tooltipX + tooltipSize.x * 0.5f, tooltipY - BORDER_WIDTH * 0.5f, 0.0f));
    DrawSprite(XMFLOAT2(tooltipSize.x, BORDER_WIDTH), borderColor);

    // 下边框
    SetWorldMatrix(XMMatrixTranslation(tooltipX + tooltipSize.x * 0.5f, tooltipY + tooltipSize.y + BORDER_WIDTH * 0.5f, 0.0f));
    DrawSprite(XMFLOAT2(tooltipSize.x, BORDER_WIDTH), borderColor);

    // 左边框
    SetWorldMatrix(XMMatrixTranslation(tooltipX - BORDER_WIDTH * 0.5f, tooltipY + tooltipSize.y * 0.5f, 0.0f));
    DrawSprite(XMFLOAT2(BORDER_WIDTH, tooltipSize.y), borderColor);

    // 右边框
    SetWorldMatrix(XMMatrixTranslation(tooltipX + tooltipSize.x + BORDER_WIDTH * 0.5f, tooltipY + tooltipSize.y * 0.5f, 0.0f));
    DrawSprite(XMFLOAT2(BORDER_WIDTH, tooltipSize.y), borderColor);

    // 绘制工具提示文本
    GeometricTextRenderer::DrawMinecraftText(
        tooltipX + TOOLTIP_PADDING,
        tooltipY + TOOLTIP_PADDING,
        text,
        XMFLOAT4(1.0f, 1.0f, 1.0f, MinimapManager::GetAlpha()),
        0.8f,
        false
    );
}

// ========================================
// 相机和移动控制
// ========================================

void ProcessMinimapClick(float worldX, float worldZ) {
    MoveCameraToWorldPosition(worldX, worldZ);
}

void MoveCameraToWorldPosition(float worldX, float worldZ) {
    Camera* camera = GetCamera();
    if (camera == nullptr) return;

    // 计算目标位置
    XMFLOAT3 targetPos = XMFLOAT3(worldX, camera->Position.y, worldZ);
    XMFLOAT3 targetAt = XMFLOAT3(worldX, 0.0f, worldZ);

    // 平滑移动相机（这里简化为直接设置）
    camera->Position = targetPos;
    camera->AtPosition = targetAt;

    // 重新计算视图矩阵
    XMVECTOR eyePos = XMVectorSet(camera->Position.x, camera->Position.y, camera->Position.z, 1.0f);
    XMVECTOR focusPos = XMVectorSet(camera->AtPosition.x, camera->AtPosition.y, camera->AtPosition.z, 1.0f);
    XMVECTOR upVec = XMVectorSet(camera->UpVector.x, camera->UpVector.y, camera->UpVector.z, 0.0f);

    camera->ViewMatrix = XMMatrixLookAtLH(eyePos, focusPos, upVec);

    char debug[128];
    sprintf_s(debug, "📍 相机移动到: (%.2f, %.2f)\n", worldX, worldZ);
    OutputDebugStringA(debug);
}

// ========================================
// 工具和计算函数
// ========================================

XMFLOAT4 GetMinimapViewBounds() {
    return MinimapManager::GetViewBounds();
}

float CalculateMinimapScale(float zoomLevel) {
    // 基础比例尺：1个像素代表多少世界单位
    float baseScale = 60.0f / 200.0f; // 60世界单位对应200像素
    return baseScale / zoomLevel;
}

XMFLOAT4 GetTerrainColorByType(int terrainType) {
    switch (terrainType) {
    case 1: // NORMAL_GROUND
        return XMFLOAT4(0.6f, 0.4f, 0.2f, 1.0f); // 棕色
    case 2: // ICE_GROUND
        return XMFLOAT4(0.7f, 0.9f, 1.0f, 1.0f); // 浅蓝色
    case 3: // SAND_GROUND
        return XMFLOAT4(1.0f, 0.9f, 0.6f, 1.0f); // 沙色
    case 4: // GRASS_GROUND
        return XMFLOAT4(0.2f, 0.8f, 0.2f, 1.0f); // 绿色
    case 5: // WATER_GROUND
        return XMFLOAT4(0.2f, 0.4f, 0.8f, 1.0f); // 蓝色
    default:
        return XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f); // 灰色
    }
}

bool IsValidMinimapPosition(float x, float y) {
    MinimapSettings settings = MinimapManager::GetSettings();
    return (x >= settings.position.x &&
        x <= settings.position.x + settings.size &&
        y >= settings.position.y &&
        y <= settings.position.y + settings.size);
}

// ========================================
// 高级功能
// ========================================

void UpdateMinimapAutoZoom() {
    // 根据游戏模式自动调整缩放
    AppMode currentMode = GetCurrentAppMode();

    static float targetZoom = 1.0f;
    static float currentZoom = 1.0f;

    switch (currentMode) {
    case APP_MODE_GAME:
        targetZoom = 1.2f; // 单人游戏稍微放大
        break;
    case APP_MODE_MULTIPLAYER_BATTLE:
        targetZoom = 0.8f; // 多人战斗缩小以显示更大区域
        break;
    case APP_MODE_EDIT:
        targetZoom = 1.5f; // 编辑模式放大以便精确操作
        break;
    default:
        targetZoom = 1.0f;
        break;
    }

    // 平滑过渡到目标缩放
    const float ZOOM_SPEED = 2.0f;
    float deltaTime = 0.016f; // 假设60FPS

    if (abs(currentZoom - targetZoom) > 0.01f) {
        if (currentZoom < targetZoom) {
            currentZoom += ZOOM_SPEED * deltaTime;
            if (currentZoom > targetZoom) currentZoom = targetZoom;
        }
        else {
            currentZoom -= ZOOM_SPEED * deltaTime;
            if (currentZoom < targetZoom) currentZoom = targetZoom;
        }

        MinimapManager::SetZoom(currentZoom);
    }
}

void SaveMinimapScreenshot() {
    // 保存小地图截图的功能（高级功能）
    OutputDebugStringA("📸 保存小地图截图（功能未实现）\n");

    // 这里可以实现将当前小地图渲染到纹理并保存为文件的功能
    // 实现会比较复杂，需要创建离屏渲染目标等
}

void ExportMinimapAsImage() {
    // 导出小地图为图像文件的功能
    OutputDebugStringA("🖼️ 导出小地图图像（功能未实现）\n");

    // 这里可以实现导出功能
}

// ========================================
// 调试和性能监控
// ========================================

void DrawMinimapDebugInfo() {
    MinimapSettings settings = MinimapManager::GetSettings();

    // 绘制调试信息
    wchar_t debugText[256];
    swprintf_s(debugText,
        L"Minimap Debug:\nZoom: %.2f\nAlpha: %.2f\nType: %s\nMouse: %s",
        settings.zoomLevel,
        MinimapManager::GetAlpha(),
        (MinimapManager::GetType() == MINIMAP_TRADITIONAL) ? L"Traditional" : L"Battle",
        MinimapManager::IsMouseInMinimap() ? L"In" : L"Out"
    );

    GeometricTextRenderer::DrawMinecraftText(
        10, SCREEN_HEIGHT - 120,
        debugText,
        XMFLOAT4(1.0f, 1.0f, 0.0f, 0.8f),
        0.7f,
        false
    );
}

void LogMinimapPerformance() {
    static int frameCount = 0;
    static float timeAccumulator = 0.0f;

    frameCount++;
    timeAccumulator += 0.016f; // 假设60FPS

    if (timeAccumulator >= 5.0f) { // 每5秒输出一次性能信息
        char perfLog[256];
        sprintf_s(perfLog, "🗺️ 小地图性能: %d 帧/5秒, 平均FPS: %.1f\n",
            frameCount, frameCount / timeAccumulator);
        OutputDebugStringA(perfLog);

        frameCount = 0;
        timeAccumulator = 0.0f;
    }
}

// ========================================
// 小地图预设配置
// ========================================

void LoadMinimapPreset(const char* presetName) {
    MinimapSettings preset;

    if (strcmp(presetName, "minimal") == 0) {
        // 最小化预设
        preset.size = 150.0f;
        preset.alpha = 0.7f;
        preset.showItems = false;
        preset.showObstacles = false;
        preset.showViewRange = false;
    }
    else if (strcmp(presetName, "detailed") == 0) {
        // 详细预设
        preset.size = 250.0f;
        preset.alpha = 0.9f;
        preset.showItems = true;
        preset.showObstacles = true;
        preset.showViewRange = true;
        preset.showSpawnPoints = true;
    }
    else if (strcmp(presetName, "battle") == 0) {
        // 战斗预设
        preset.size = 200.0f;
        preset.alpha = 0.8f;
        preset.zoomLevel = 0.8f;
        preset.showPlayerPosition = true;
        preset.showSpawnPoints = true;
        preset.showItems = false;
        preset.showObstacles = false;
    }
    else {
        // 默认预设
        preset = MinimapSettings();
    }

    MinimapManager::SetSettings(preset);

    char debug[128];
    sprintf_s(debug, "🗺️ 加载小地图预设: %s\n", presetName);
    OutputDebugStringA(debug);
}

void SaveMinimapPreset(const char* presetName) {
    // 保存当前设置为预设
    MinimapSettings currentSettings = MinimapManager::GetSettings();

    // 这里可以实现将设置保存到文件的功能
    // 简化版本只输出调试信息
    char debug[128];
    sprintf_s(debug, "🗺️ 保存小地图预设: %s (功能未完全实现)\n", presetName);
    OutputDebugStringA(debug);
}

// ========================================
// 小地图动画效果
// ========================================

void UpdateMinimapAnimations(float deltaTime) {
    // 更新各种动画效果
    static float animationTimer = 0.0f;
    animationTimer += deltaTime;

    // 道具闪烁动画
    static float itemBlinkTimer = 0.0f;
    itemBlinkTimer += deltaTime * 3.0f;
    if (itemBlinkTimer > 6.28f) itemBlinkTimer = 0.0f;

    // 玩家位置脉冲动画
    static float playerPulseTimer = 0.0f;
    playerPulseTimer += deltaTime * 2.0f;
    if (playerPulseTimer > 6.28f) playerPulseTimer = 0.0f;

    // 这些定时器可以在相应的绘制函数中使用
}

// ========================================
// 辅助宏和内联函数
// ========================================

inline float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline XMFLOAT2 Lerp2D(XMFLOAT2 a, XMFLOAT2 b, float t) {
    return XMFLOAT2(
        Lerp(a.x, b.x, t),
        Lerp(a.y, b.y, t)
    );
}

inline float Distance2D(XMFLOAT2 a, XMFLOAT2 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

// ========================================
// 小地图系统完整性检查
// ========================================

bool ValidateMinimapSystem() {
    bool isValid = true;

    // 检查初始化状态
    if (!MinimapManager::IsInitialized) {
        OutputDebugStringA("❌ 小地图系统未初始化\n");
        isValid = false;
    }

    // 检查设置有效性
    MinimapSettings settings = MinimapManager::GetSettings();
    if (settings.size <= 0 || settings.size > 500) {
        OutputDebugStringA("❌ 小地图大小无效\n");
        isValid = false;
    }

    if (settings.alpha < 0 || settings.alpha > 1) {
        OutputDebugStringA("❌ 小地图透明度无效\n");
        isValid = false;
    }

    if (settings.zoomLevel <= 0 || settings.zoomLevel > 5) {
        OutputDebugStringA("❌ 小地图缩放级别无效\n");
        isValid = false;
    }

    if (isValid) {
        OutputDebugStringA("✅ 小地图系统验证通过\n");
    }

    return isValid;
}
