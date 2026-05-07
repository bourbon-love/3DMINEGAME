#pragma once
#ifndef MULTIPLAYER_BATTLE_MAP_H
#define MULTIPLAYER_BATTLE_MAP_H

#include "../main.h"
#include "../Box.h"
#include "../MultiplayerBattle.h"

// 前向声明
struct BattlePlayerInfo;
enum PlayerID;

class MultiplayerBattleMap {
public:
    // 地?初始化
    static void Init();
    static void Uninit();
    
    // 地?生成和?用
    static bool MergeMapsForBattle(BattlePlayerInfo* players, int activePlayerCount);
    static void ApplyBattleMapToWorld();
    
    // 地?工具函数
    static void GetPlayerMapOffset(PlayerID playerID, int* offsetX, int* offsetZ);
    static XMFLOAT3 GetPlayerSpawnPosition(PlayerID playerID);
    
    // 地?数据??
    static int GetBattleGroundTile(int x, int z);
    static int GetBattleObstacleTile(int x, int z);
    static void SetBattleGroundTile(int x, int z, int value);
    static void SetBattleObstacleTile(int x, int z, int value);
    
    // 地?数据指?（供其他系???）
    static int* GetBattleGroundMapPtr() { return (int*)s_battleGroundMap; }
    static int* GetBattleObstacleMapPtr() { return (int*)s_battleObstacleMap; }

private:
    // 地?数据
    static int s_battleGroundMap[BATTLE_MAP_SIZE_Z][BATTLE_MAP_SIZE_X];
    static int s_battleObstacleMap[BATTLE_MAP_SIZE_Z][BATTLE_MAP_SIZE_X];
    
    // 地?生成?助函数
    static void GenerateTestMapForPlayer(int playerID, int battleX, int battleZ, int localX, int localZ);
    static void GenerateEnhancedPlayerMap(int playerID, int battleX, int battleZ, int localX, int localZ);
    static void AddPlayerSpawnPoint(int playerID, int offsetX, int offsetZ);
    static void CreateConnectionAreas();
    
    // 地?采?函数
    static int SampleBattleMapTerrain(int centerX, int centerZ);
    static int SampleBattleMapObstacle(int centerX, int centerZ);
};

#endif // MULTIPLAYER_BATTLE_MAP_H