// MapEditor.h
#pragma once

#include "main.h"
#include "renderer.h"
#include "Box.h"

extern float g_UIScaleFactor;

// 地形UI盒子相关
extern UIBox g_UIBoxes[UI_BOX_COUNT];
extern int g_SelectedUIBoxIndex;

// 障碍物UI盒子相关
extern ObstacleUIBox g_ObstacleUIBoxes[OBSTACLE_UI_BOX_COUNT];
extern int g_SelectedObstacleUIBoxIndex;

// 拖动地图相关变量
extern bool g_IsDraggingMap;
extern int g_LastEditedX;
extern int g_LastEditedZ;

// 初始化和清理
void InitMapEditor();
void UninitMapEditor();
void UpdateMapEditor(float dt);

// UI盒子管理
void InitUIBoxes();
void DrawUIBoxes();
void InitObstacleUIBoxes();
void DrawObstacleUIBoxes();

// 拾取和编辑函数
int PickUIBoxUnderMouse(int mouseX, int mouseY);
int PickObstacleUIBoxUnderMouse(int mouseX, int mouseY);
void ApplyEditToBox(int x, int z, MAP_ELEMENT_TYPE type);
void ApplyObstacleEditToBox(int x, int z, MAP_ELEMENT_TYPE type);
bool PickBoxUnderMouse(int* outX, int* outZ);
bool RayIntersectsAABB(XMVECTOR rayOrigin,
    XMVECTOR rayDir,
    XMVECTOR boxMin,
    XMVECTOR boxMax,
    float* outDistance);

// 编辑操作处理函数
void HandleObstacleEditModeClick(int mouseX, int mouseY);
void HandleTerrainEditModeClick(int mouseX, int mouseY);
void HandleMapDragging();

// 地图文件操作
void SaveMapToFile(const char* filename);
void LoadMapFromFile(const char* filename);
// 包装函数：用于UI按钮回调
bool SaveCurrentMap();
bool LoadCurrentMap();
// 辅助UI绘制函数
void DrawEditorHelp();

// 访问器函数
UIBox* GetUIBoxes();
int* GetSelectedUIBoxIndex();
ObstacleUIBox* GetObstacleUIBoxes();
int* GetSelectedObstacleUIBoxIndex();
bool* GetIsDraggingMap();
int* GetLastEditedX();
int* GetLastEditedZ();

// 反馈显示
void ShowFeedbackText(XMFLOAT3 worldPosition, const char* message, XMFLOAT4 color);

// 战斗地图相关函数
void CreateBattleMap();