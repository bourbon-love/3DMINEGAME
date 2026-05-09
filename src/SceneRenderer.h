// ========================================
// 场景管理头文件 SceneRenderer.h
// ========================================

#pragma once
#include "main.h"
#include "SceneManager.h"

class SceneRenderer {
public:
    // 初始化所有场景
    static void InitAllScenes();

    // 清理所有场景
    static void UninitAllScenes();

    // 更新当前场景
    static void UpdateCurrentScene(float deltaTime);

    // 绘制当前场景
    static void DrawCurrentScene();

private:
    // 预览场景
    static void UpdatePreviewScene(float deltaTime);
    static void DrawPreviewScene();

    // 游戏场景
    static void UpdateGameScene(float deltaTime);
    static void DrawGameScene();

    // 编辑场景
    static void UpdateEditScene(float deltaTime);
    static void DrawEditScene();

    // 暂停场景
    static void UpdatePauseScene(float deltaTime);
    static void DrawPauseScene();

    // 游戏结束场景
    static void UpdateGameOverScene(float deltaTime);
    static void DrawGameOverScene();

    // 角色选择场景
    static void UpdateCharacterScene(float deltaTime);
    static void DrawCharacterScene();

    // 通用的3D环境绘制
    static void Draw3DEnvironment();

    // 通用的UI绘制
    static void DrawUI();
    //战斗大厅
    static void UpdateBattleHallScene(float deltaTime);
    static void DrawBattleHallScene();
};