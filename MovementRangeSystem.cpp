#include "MovementRangeSystem.h"
#include <map>
#include "SceneManager.h"
#include "ParticlaEffect.h"


std::vector<ReachableGrid> MovementRangeSystem::s_reachableGrids;
MovementDisplayType MovementRangeSystem::s_displayType = DISPLAY_COMBINED;
PlayerID MovementRangeSystem::s_currentPlayer = PLAYER_1;
bool MovementRangeSystem::s_isVisible = false;
float MovementRangeSystem::s_animationTime = 0.0f;


void MovementRangeSystem::Init()
{
    s_reachableGrids.clear();
    s_displayType = DISPLAY_COMBINED;
    s_currentPlayer = PLAYER_1;
    s_isVisible = false;
    s_animationTime = 0.0f;
}

void MovementRangeSystem::Uninit()
{
    s_reachableGrids.clear();
    s_isVisible = false;

}

void MovementRangeSystem::Update(float deltaTime)
{
    //更新动画时间
    s_animationTime += deltaTime;

    //为可到达格子更新动画效果
    for (auto& grid : s_reachableGrids) {
        //脉动效果
        float pulsePhase = s_animationTime * 3.0f 
            + (grid.x + grid.z) * 0.5f;
        grid.intensity = 0.5f + 0.3f * sinf(pulsePhase);

        //根据剩余移动力调整颜色
        float movementRatio = (float)grid.remainingMovement / 10.0f;//假设最大移动力10

        if (grid.remainingMovement >= 7) {
            // 高移动力：绿色
            grid.displayColor = XMFLOAT4(0.2f, 1.0f, 0.3f, grid.intensity);
        }
        else if (grid.remainingMovement >= 4) {
            // 中等移动力：黄色
            grid.displayColor = XMFLOAT4(1.0f, 1.0f, 0.2f, grid.intensity);
        }
        else if (grid.remainingMovement >= 1) {
            // 低移动力：橙色
            grid.displayColor = XMFLOAT4(1.0f, 0.6f, 0.1f, grid.intensity);
        }
        else {
            // 刚好到达：红色
            grid.displayColor = XMFLOAT4(1.0f, 0.2f, 0.2f, grid.intensity);
        }
    }
}

void MovementRangeSystem::Draw()
{
    if (!s_isVisible || s_reachableGrids.empty()) return;

    // 总是同时绘制高亮格子和粒子效果
    DrawHighlightTiles();
    DrawParticleEffects();
}

void MovementRangeSystem::CalculateMovementRange(PlayerID playerID, int startX, int startZ, int movementPoints) {
    s_reachableGrids.clear();
    s_currentPlayer = playerID;

    // GridNode 结构体定义
    struct GridNode {
        int x, z;
        int totalCost;
    };

    // 使用 lambda 比较器创建最小堆
    auto compare = [](const GridNode& a, const GridNode& b) {
        return a.totalCost > b.totalCost; // 最小堆
        };

    std::priority_queue<GridNode, std::vector<GridNode>, decltype(compare)> openList(compare);
    std::set<std::pair<int, int>> closedList;
    std::map<std::pair<int, int>, int> costMap;

    // 起始点 - 修复：使用函数参数 startX, startZ
    openList.push({ startX, startZ, 0 });
    costMap[{startX, startZ}] = 0;

    // 八个方向（包括对角线）
    int dx[] = { -1, -1, -1,  0,  0,  1,  1,  1 };
    int dz[] = { -1,  0,  1, -1,  1, -1,  0,  1 };

    while (!openList.empty()) {
        GridNode current = openList.top();
        openList.pop();

        auto currentPair = std::make_pair(current.x, current.z);
        if (closedList.find(currentPair) != closedList.end()) {
            continue;
        }

        closedList.insert(currentPair);

        // 如果当前格子在移动范围内，添加到可达列表
        if (current.totalCost <= movementPoints) {
            ReachableGrid reachable;
            reachable.x = current.x;
            reachable.z = current.z;
            reachable.movementCost = current.totalCost;
            reachable.remainingMovement = movementPoints - current.totalCost;
            reachable.isOptimalPath = true;
            reachable.intensity = 1.0f;

            s_reachableGrids.push_back(reachable);
        }

        // 探索相邻格子
        for (int i = 0; i < 8; i++) {
            int newX = current.x + dx[i];
            int newZ = current.z + dz[i];

            // 边界检查
            int mapSizeX = (GetCurrentAppMode() == APP_MODE_MULTIPLAYER_BATTLE) ? BATTLE_MAP_SIZE_X : MAPSIZE_X;
            int mapSizeZ = (GetCurrentAppMode() == APP_MODE_MULTIPLAYER_BATTLE) ? BATTLE_MAP_SIZE_Z : MAPSIZE_Z;

            if (newX < 0 || newX >= mapSizeX || newZ < 0 || newZ >= mapSizeZ) {
                continue;
            }

            // 检查是否可通行
            if (!IsGridWalkable(newX, newZ)) {
                continue;
            }

            // 计算移动消耗
            int gridCost = GetGridMovementCost(newX, newZ);

            // 对角线移动额外消耗
            if (i % 2 == 0) { // 对角线方向
                gridCost = gridCost * 14 / 10; // 1.4倍消耗
            }

            int newTotalCost = current.totalCost + gridCost;

            // 超出移动范围则跳过
            if (newTotalCost > movementPoints) {
                continue;
            }

            auto newPair = std::make_pair(newX, newZ);

            // 如果已经访问过且代价不更优，跳过
            if (costMap.find(newPair) != costMap.end() &&
                costMap[newPair] <= newTotalCost) {
                continue;
            }

            costMap[newPair] = newTotalCost;
            openList.push({ newX, newZ, newTotalCost });
        }
    }

    char debugMsg[256];
    sprintf_s(debugMsg, "🎯 玩家%d从(%d,%d)出发，移动力%d，可达格子数：%zu\n",
        playerID, startX, startZ, movementPoints, s_reachableGrids.size());
    OutputDebugStringA(debugMsg);
}
void MovementRangeSystem::ShowMovementRange(PlayerID playerID, MovementDisplayType displayType)
{
    s_currentPlayer = playerID;
    s_displayType = displayType;
    s_isVisible = true;
    s_animationTime = 0.0f;

    // 获取玩家当前位置
    BattlePlayerInfo* player = MultiplayerBattleManager::GetPlayer(playerID);
    if (!player || !player->isActive) {
        OutputDebugStringA("❌ 无效的玩家ID或玩家未激活\n");
        return;
    }

    // 将世界坐标转换为格子坐标
    XMFLOAT3 playerPos = player->spawnPosition;
    int playerX = (int)(playerPos.x / BOXSIZE_X);
    int playerZ = (int)(-playerPos.z / BOXSIZE_Z);

    // 假设玩家有10点移动力（可以从玩家数据中获取）
    int movementPoints = 8; // 可以根据角色类型调整

    CalculateMovementRange(playerID, playerX, playerZ, movementPoints);

}

void MovementRangeSystem::HideMovementRange()
{
    s_isVisible = false;
    s_reachableGrids.clear();
}

// 方法1：高亮格子显示
void MovementRangeSystem::DrawHighlightTiles() {
    // 保存当前渲染状态
    bool oldBlendEnable = GetBlendState();
    bool oldDepthEnable = GetDepthEnable();

    SetBlendState(true);
    SetDepthEnable(true);

    for (const auto& grid : s_reachableGrids) {
        // 计算世界位置
        float worldX = grid.x * BOXSIZE_X + (BOXSIZE_X / 2);
        float worldY = BOXSIZE_Y / 2 + 0.02f; // 略高于地面
        float worldZ = -grid.z * BOXSIZE_Z + (BOXSIZE_Z / 2);

        // 设置世界矩阵
        XMMATRIX TranslationMatrix = XMMatrixTranslation(worldX, worldY, worldZ);
        XMMATRIX ScalingMatrix = XMMatrixScaling(0.9f, 0.1f, 0.9f); // 扁平的指示器
        XMMATRIX WorldMatrix = ScalingMatrix * TranslationMatrix;
        SetWorldMatrix(WorldMatrix);

        // 设置材质
        MATERIAL material;
        ZeroMemory(&material, sizeof(MATERIAL));
        material.Diffuse = grid.displayColor;
        material.Emission = XMFLOAT4(
            grid.displayColor.x * 0.3f,
            grid.displayColor.y * 0.3f,
            grid.displayColor.z * 0.3f,
            0.0f
        ); // 轻微发光效果
        SetMaterial(material);

        // 获取Box的缓冲区并绘制
        ID3D11Buffer* vertexBuffer = GetBoxVertexBuffer();
        ID3D11Buffer* indexBuffer = GetBoxIndexBuffer();

        if (vertexBuffer && indexBuffer) {
            UINT stride = sizeof(VERTEX_3D);
            UINT offset = 0;
            GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
            GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
            GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            GetDeviceContext()->DrawIndexed(GetBoxIndexCount(), 0, 0);
        }
    }

    // 恢复渲染状态
    SetBlendState(oldBlendEnable);
    SetDepthEnable(oldDepthEnable);
}

// 方法3：粒子效果显示
void MovementRangeSystem::DrawParticleEffects() {
    static float lastParticleTime = 0.0f;

    // 每0.5秒产生一次粒子
    if (s_animationTime - lastParticleTime > 0.3f) {
        for (const auto& grid : s_reachableGrids) {
            XMFLOAT3 worldPos = XMFLOAT3(
                grid.x * BOXSIZE_X + (BOXSIZE_X / 2),
                BOXSIZE_Y + 0.3f,
                -grid.z * BOXSIZE_Z + (BOXSIZE_Z / 2)
            );

            // 根据剩余移动力选择粒子颜色
            XMFLOAT4 particleColor = grid.displayColor;
            particleColor.w = 0.6f;

            // 创建少量粒子
            ParticleSystem::CreateEffect(worldPos, 3, particleColor, 0.02f, 0.08f, 1.0f);
        }
        lastParticleTime = s_animationTime;
    }
}

bool MovementRangeSystem::IsGridReachable(int x, int z)
{
    for (const auto& grid : s_reachableGrids) {
        if (grid.x == x && grid.z == z) {
            return true;
        }
    }
    return false;
}

int MovementRangeSystem::GetMovementCostToGrid(int x, int z)
{
    for (const auto& grid : s_reachableGrids) {
        if (grid.x == x && grid.z == z) {
            return grid.movementCost;
        }
    }
    return -1;
}

//外部接口实现
void InitMovementRangeSystem() {
    MovementRangeSystem::Init();
}

void UninitMovementRangeSystem() {
    MovementRangeSystem::Uninit();
}

void UpdateMovementRangeSystem(float deltaTime) {
    MovementRangeSystem::Update(deltaTime);
}

void DrawMovementRangeSystem() {
    MovementRangeSystem::Draw();
}

void ShowPlayerMovementRange(int playerID) {
    MovementRangeSystem::ShowMovementRange((PlayerID)playerID, DISPLAY_COMBINED);
}

void HidePlayerMovementRange() {
    MovementRangeSystem::HideMovementRange();
}