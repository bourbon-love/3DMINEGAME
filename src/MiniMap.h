#pragma once
// Minimap.h
// 小地图系统头文件 - 支持单人和多人模式


#include "main.h"
#include "renderer.h"
#include "Camera.h"
#include "ball.h"
#include "Box.h"
#include "GeometricTextRenderer.h"
#include "MultiplayerBattle.h"

// 小地图设置结构
struct MinimapSettings {
    bool isVisible;                 // 是否显示小地图
    float size;                     // 小地图大小
    float margin;                   // 边距
    float alpha;                    // 透明度
    float zoomLevel;                // 缩放级别
    bool showPlayerPosition;        // 显示玩家位置
    bool showItems;                 // 显示道具
    bool showObstacles;             // 显示障碍物
    bool showSpawnPoints;           // 显示出生点
    bool showViewRange;             // 显示视野范围
    XMFLOAT2 position;              // 小地图位置

    // 构造函数
    MinimapSettings() :
        isVisible(true),
        size(200.0f),
        margin(20.0f),
        alpha(0.9f),
        zoomLevel(1.0f),
        showPlayerPosition(true),
        showItems(true),
        showObstacles(false),
        showSpawnPoints(true),
        showViewRange(false),
        position(0.0f, 0.0f)
    {
    }
};

// 小地图类型枚举
enum MINIMAP_TYPE {
    MINIMAP_TRADITIONAL,        // 传统小地图（单人模式）
    MINIMAP_MULTIPLAYER_BATTLE  // 多人战斗小地图
};

// 小地图管理器类
class MinimapManager {
private:
    static MinimapSettings s_settings;
    static MINIMAP_TYPE s_currentType;
    static bool s_initialized;
    static float s_fadeTimer;
    static bool s_fadingIn;
    static XMFLOAT4 s_viewBounds;       // 当前视野范围
    static bool s_mouseInMinimap;       // 鼠标是否在小地图区域
    static XMFLOAT2 s_mouseMinimapPos;  // 鼠标在小地图上的相对位置

    // 内部辅助函数
    static void UpdateFadeEffect(float deltaTime);
    static void UpdateViewBounds();
    static void UpdateMouseState();

public:
    // 初始化和清理
    static void Init();
    static void Uninit();
    static void Update(float deltaTime);

    static XMFLOAT2 WorldToMinimapCoords(float worldX, float worldZ);
    static XMFLOAT2 MinimapToWorldCoords(float minimapX, float minimapY);
    static bool IsPointInMinimapArea(float x, float y);
    // 主要绘制函数
    static void DrawMinimap(MINIMAP_TYPE type, ID3D11ShaderResourceView* gameTexture = nullptr);
    static void DrawTraditionalMinimap(ID3D11ShaderResourceView* gameTexture);
    static void DrawMultiplayerBattleMinimap();

    // 设置管理
    static void SetSettings(const MinimapSettings& settings);
    static MinimapSettings GetSettings();
    static void SetVisible(bool visible);
    static bool IsVisible();
    static void SetType(MINIMAP_TYPE type);
    static MINIMAP_TYPE GetType();

    // 缩放和位置控制
    static void SetZoom(float zoom);
    static float GetZoom();
    static void ZoomIn();
    static void ZoomOut();
    static void SetPosition(float x, float y);
    static XMFLOAT2 GetPosition();

    // 透明度控制
    static void SetAlpha(float alpha);
    static float GetAlpha();

    // 显示选项
    static void SetShowPlayerPosition(bool show);
    static void SetShowItems(bool show);
    static void SetShowObstacles(bool show);
    static void SetShowSpawnPoints(bool show);
    static void SetShowViewRange(bool show);

    // 交互功能
    static void HandleMouseClick(int mouseX, int mouseY);
    static void HandleMouseMove(int mouseX, int mouseY);
    static void HandleKeyboardInput();
    static void ToggleVisibility();

    // 查询函数
    static bool IsMouseInMinimap();
    static XMFLOAT2 GetMouseMinimapPosition();
    static XMFLOAT2 GetMouseWorldPosition();

    //私有成员接口
    static bool IsInitialized() { return s_initialized; }
    static XMFLOAT4 GetViewBounds() { return s_viewBounds; }
};

// 外部接口函数（供Game.cpp调用）
extern void InitMinimap();
extern void UninitMinimap();
extern void UpdateMinimap(float deltaTime);
extern void DrawGameMinimap(ID3D11ShaderResourceView* gameTexture);
//extern void DrawBattleMinimap();
extern void HandleMinimapInput();
extern void DrawBattleMinimapGeneral();

// 小地图绘制辅助函数
extern void DrawMinimapBackground(float size, XMFLOAT2 position);
extern void DrawMinimapBorder(float size, XMFLOAT2 position, XMFLOAT4 borderColor);
extern void DrawMinimapGrid(float size, XMFLOAT2 position);

// 标记和图标绘制函数
extern void DrawPlayerPositionOnMinimap(float size, XMFLOAT2 position);
extern void DrawItemsOnMinimap(float size, XMFLOAT2 position);
extern void DrawObstaclesOnMinimap(float size, XMFLOAT2 position);
extern void DrawSpawnPointsOnMinimap(float size, XMFLOAT2 position);
extern void DrawViewRangeOnMinimap(float size, XMFLOAT2 position);

// 多人战斗专用绘制函数
extern void DrawBattleMapBoundaries(float size, XMFLOAT2 position);
extern void DrawMultiplayerPositions(float size, XMFLOAT2 position);
extern void DrawPlayerDeathMarkers(float size, XMFLOAT2 position);

// 交互功能
extern void ProcessMinimapClick(float worldX, float worldZ);
extern void MoveCameraToWorldPosition(float worldX, float worldZ);
extern void DrawMinimapTooltip(int mouseX, int mouseY, const wchar_t* text);

// 工具函数
extern XMFLOAT4 GetMinimapViewBounds();
extern float CalculateMinimapScale(float zoomLevel);
extern XMFLOAT4 GetTerrainColorByType(int terrainType);
extern bool IsValidMinimapPosition(float x, float y);

// 在 MiniMap.h 的外部接口函数部分添加
extern void DrawBattleInfo();
extern void DrawBattleMapRegions();
extern void DrawBattleMinimapTooltip();
extern void DrawMinimapTitle();
extern void DrawPlayerNumber(int playerNumber, float x, float y);
extern void DrawGameTextureOnMinimap(ID3D11ShaderResourceView* gameTexture);
extern void DrawProceduralMapContent();
extern void DrawPlayerDirectionIndicator(float mapX, float mapY, BallObject* ball);
// 在外部接口函数部分添加
extern void DrawMultiplayerSpawnPoints();
extern void DrawMinimapTooltip();  // 无参数版本