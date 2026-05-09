// SceneManager.h
#pragma once

#include "main.h"
#include "box.h"

// 应用状态/模式
enum AppMode {
    APP_MODE_PREVIEW,   // 用于预览地图
    APP_MODE_EDIT,      // 用于编辑地图
    APP_MODE_GAME,      // 用于实际游戏
    APP_MODE_MULTIPLAYER_BATTLE,//多人战斗模式
    APP_MODE_PAUSE,     // 用于暂停界面
    APP_MODE_GAMEOVER,  // 用于游戏结束界面
    APP_MODE_CHARACTER, // 用于角色选择
    APP_MODE_BATTLE_HALL,  //战斗大厅模式
    APP_MODE_COUNT
};

// 场景管理器初始化与清理
void InitSceneManager();
void UninitSceneManager();

// 场景管理器更新
void UpdateSceneManager(float deltaTime);

// 模式获取与切换
AppMode GetCurrentAppMode();
AppMode GetTargetAppMode();
AppMode GetPreviousAppMode();

void SwitchToMode(AppMode newMode);
bool IsModeChanging();
float GetModeTransitionProgress();

void SwitchToGameMode();      // 切换到游戏模式
void SwitchToPauseMode();     // 切换到暂停界面
void SwitchToGameOverMode();  // 切换到游戏结束界面
void SwitchToBattleHallMode();//
void SwitchToMultiplayerBattleMode();
//多人战斗更新函数
void UpdateMultiplayerBattle(float deltaTime);
void ReturnToMainMenu();      // 返回主菜单（预览模式）

// 判断是否正在编辑障碍物
bool IsEditingObstacle();

// 切换障碍物/地形编辑
void ToggleObstacleEditingMode();

// 获取镜头转换状态
bool IsCameraTransitioning();
void SetCameraTransitioning(bool isTransitioning);

// 玩家状态相关
void SetPlayerScore(int score);  // 设置玩家分数
int GetPlayerScore();            // 获取玩家分数
void SetPlayerLives(int lives);  // 设置玩家生命值
int GetPlayerLives();            // 获取玩家生命值
void DecreasePlayerLives();      // 减少玩家生命值
void IncreasePlayerScore(int points); // 增加玩家分数
void UpdatePlayerStatusUI();
void SetupMultiplayerBattleCamera();//设置专用摄像机
void CheckBattleConditions();    //检查战斗条件
void EndMultiplayerBattle(int winnerID);//结束多人战斗
