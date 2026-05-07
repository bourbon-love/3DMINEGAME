#pragma once


#include <vector>
#include <queue>
#include <set>
#include "MultiplayerBattle.h"
#include "Box.h"


enum MovementDisplayType {

	DISPLAY_NONE,
	DISPLAY_COMBINED
};

//可达性信息结构
struct ReachableGrid
{
    int x, z;                   // 格子坐标
    int movementCost;           // 到达此格子的总移动力消耗
    int remainingMovement;      // 到达后剩余移动力
    bool isOptimalPath;         // 是否为最优路径
    XMFLOAT4 displayColor;      // 显示颜色
    float intensity;            // 显示强度(0.0-1.0
};
class MovementRangeSystem {

private:
    static std::vector<ReachableGrid> s_reachableGrids;
    static MovementDisplayType        s_displayType;
    static PlayerID                   s_currentPlayer;
    static bool                       s_isVisible;
    static float                      s_animationTime;

public:
    static void Init();
    static void Uninit();
    static void Update(float deltaTime);
    static void Draw();

    //核心功能
    static void CalculateMovementRange(PlayerID playerID, int startX, int startZ,
        int movementPoints);
    static void ShowMovementRange(PlayerID playerID,
        MovementDisplayType displayType = DISPLAY_COMBINED);
    static void HideMovementRange();
    static void DrawHighlightTiles();
    static void DrawParticleEffects();
    static bool IsGridReachable(int x, int z);
    static int GetMovementCostToGrid(int x, int z);

    //显示控制
    static void SetDisplayType(MovementDisplayType type) 
    { s_displayType = type; }

    static MovementDisplayType GetDisplayType()
    {     return s_displayType;}

    static void SetVisible(bool visible)
    {        s_isVisible = visible;}
    
    //工具函数
    static std::vector<ReachableGrid> GetreachaGrid()
    {       return s_reachableGrids;    }

    static void ClearReachableGrid()    
    {        s_reachableGrids.clear();    }
};


//外部接口
void InitMovementRangeSystem();

void UninitMovementRangeSystem();

void UpdateMovementRangeSystem(float deltaTime);
void DrawMovementRangeSystem();


void ShowPlayerMovementRange(int playerID);

void HidePlayerMovementRange();