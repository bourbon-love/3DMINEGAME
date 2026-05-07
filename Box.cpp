//Box.cpp

#include "Box.h"
#include "model.h"
#include "Collision.h"
#include <algorithm>
#include "Camera.h"
#include "mouse.h"
#include "MapEditor.h" 
#include "ParticlaEffect.h"
#include "GeometricTextRenderer.h"
#include "SceneManager.h"
#include "ProceduralModels.h"
#include "FrameWork/TextureManager.h"
static ID3D11Buffer* g_VertexBuffer = NULL;//テスト用頂点バッファ
static ID3D11Buffer* g_IndexBuffer = NULL;//テスト用インデックスバッファ//追加


BoxObject g_Box[BATTLE_MAP_SIZE_X * BATTLE_MAP_SIZE_Z];
BoxObject g_Obstacle[BATTLE_MAP_SIZE_X * BATTLE_MAP_SIZE_Z];

// 移动力消耗表 (indexed by MAP_ELEMENT_TYPE 0-12)
const int MOVEMENT_COST[13] = {
    99,  // 0  NONE
    1,   // 1  NORMAL_GROUND
    99,  // 2  WALL
    99,  // 3  TREE
    1,   // 4  ICE_GROUND
    2,   // 5  SAND_GROUND
    1,   // 6  GRASS_GROUND
    3,   // 7  WATER
    1,   // 8  START
    1,   // 9  GOAL
    1,   // 10 GRID
    1,   // 11 COIN
    1,   // 12 BOMB
};

// 地形特殊效果表 (indexed by MAP_ELEMENT_TYPE 0-12)
const TerrainEffect TERRAIN_EFFECTS[13] = {
    EFFECT_NONE,      // 0  NONE
    EFFECT_NONE,      // 1  NORMAL_GROUND
    EFFECT_NONE,      // 2  WALL
    EFFECT_NONE,      // 3  TREE
    EFFECT_SLIPPERY,  // 4  ICE_GROUND
    EFFECT_SLOW,      // 5  SAND_GROUND
    EFFECT_HEAL,      // 6  GRASS_GROUND
    EFFECT_DAMAGE,    // 7  WATER
    EFFECT_NONE,      // 8  START
    EFFECT_NONE,      // 9  GOAL
    EFFECT_NONE,      // 10 GRID
    EFFECT_NONE,      // 11 COIN
    EFFECT_NONE,      // 12 BOMB
};

static bool g_ProceduralModelsRenderedThisFrame = false;
static int g_LastFrameRendered = -1; // 用于跟踪帧
static bool g_AllProceduralModelsRendered = false;

MODEL* Tree;
//MODEL* Bomb;
LIGHT Light;	//////////////////////////Lighting追加
static bool g_IsRenderingProceduralModels = false;
// 地图数据
int GROUND_MAP[MAPSIZE_Z][MAPSIZE_X] =
{
    // 第一行 (0)
    10,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    // 第二行 (1)
    10,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    // 第三行 (2)
    10,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    // 第四行 (3)
    10,6,6,4,4,4,4,4,4,4,4,4,6,6,6,
    // 第五行 (4)
    6,6,6,4,4,4,4,4,4,4,4,4,6,6,6,
    // 第六行 (5)
    6,6,6,4,4,1,1,1,1,1,1,4,6,6,6,
    // 第七行 (6)
    6,6,6,4,4,1,1,1,1,1,1,4,6,6,6,
    // 第八行 (7)
    6,6,6,4,4,1,1,1,1,1,1,4,6,6,6,
    // 第九行 (8)
    6,6,6,4,4,1,1,1,1,1,1,4,6,6,6,
    // 第十行 (9)
    6,6,6,4,4,1,1,1,1,1,1,4,6,6,6,
    // 第十一行 (10)
    6,6,6,4,4,4,4,4,4,4,4,4,6,6,6,
    // 第十二行 (11)
    6,6,6,4,4,4,4,4,4,4,4,4,6,6,6,
    // 第十三行 (12)
    6,6,6,5,5,5,5,5,5,5,5,5,6,6,6,
    // 第十四行 (13)
    6,6,6,5,5,5,5,5,5,5,5,5,6,6,6,
    // 第十五行 (14)
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
};


// 障碍物地图数据
int OBSTACLE_MAP[MAPSIZE_Z][MAPSIZE_X] = { 0 };



void CreateGround()
{
    OutputDebugStringA("🗺️ CreateGround: 开始创建地面对象\n");

    for (int i = 0; i < MAPSIZE_X * MAPSIZE_Z; i++) {
        g_Box[i] = BoxObject{};
    }

    int createdBoxes = 0;

    //地面作成 - 基于15x15显示地图
    for (int z = 0; z < MAPSIZE_Z; z++)
    {
        for (int x = 0; x < MAPSIZE_X; x++)
        {
            if (GROUND_MAP[z][x] != 0)
            {
                int i = z * MAPSIZE_X + x; // 直接计算索引而不是搜索

                g_Box[i].ObjectNo = GROUND_MAP[z][x];
                g_Box[i].Use = true;
                g_Box[i].position.x = x * BOXSIZE_X + (BOXSIZE_X / 2);
                g_Box[i].position.y = 0.0f - (BOXSIZE_Y / 2);
                g_Box[i].position.z = -z * BOXSIZE_Z + (BOXSIZE_Z / 2);
                g_Box[i].rotate = XMFLOAT3(0.0f, 0.0f, 0.0f);
                g_Box[i].scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
                g_Box[i].TexID = GET_TEXTURE(GROUND);
                g_Box[i].Radius = BOX_RADIUS;
                g_Box[i].SizeMin.x = g_Box[i].SizeMin.y = g_Box[i].SizeMin.z = -BOXSIZE_X / 2;
                g_Box[i].SizeMax.x = g_Box[i].SizeMax.y = g_Box[i].SizeMax.z = BOXSIZE_X / 2;

                g_Box[i].groundType = (MAP_ELEMENT_TYPE)GROUND_MAP[z][x];

                switch (GROUND_MAP[z][x])
                {
                case NORMAL_GROUND:
                    g_Box[i].TexID = GET_TEXTURE(GROUND);
                    g_Box[i].editTexID = GET_TEXTURE(GRID);
                    g_Box[i].movementCost = 1;
                    g_Box[i].effect = EFFECT_NONE;
                    break;
                case ICE_GROUND:
                    g_Box[i].TexID = GET_TEXTURE(ICE);
                    g_Box[i].editTexID = GET_TEXTURE(GRID);
                    g_Box[i].movementCost = 1;
                    g_Box[i].effect = EFFECT_SLIPPERY;
                    break;
                case SAND_GROUND:
                    g_Box[i].TexID = GET_TEXTURE(SAND);
                    g_Box[i].editTexID = GET_TEXTURE(GRID);
                    g_Box[i].movementCost = 2;
                    g_Box[i].effect = EFFECT_SLOW;
                    break;
                case GRASS_GROUND:
                    g_Box[i].TexID = GET_TEXTURE(GRASS);
                    g_Box[i].editTexID = GET_TEXTURE(GRID);
                    g_Box[i].movementCost = 1;
                    g_Box[i].effect = EFFECT_HEAL;
                    break;
                case WATER:
                    g_Box[i].TexID = GET_TEXTURE(WATER);
                    g_Box[i].editTexID = GET_TEXTURE(GRID);
                    g_Box[i].movementCost = 99; // 不可通行
                    g_Box[i].effect = EFFECT_DAMAGE;
                    break;
                case GRID:
                    g_Box[i].editTexID = GET_TEXTURE(GRID);
                    g_Box[i].TexID = GET_TEXTURE(GRID);
                    g_Box[i].movementCost = 1;
                    g_Box[i].effect = EFFECT_NONE;
                    break;
                default:
                    g_Box[i].TexID = GET_TEXTURE(GROUND);
                    g_Box[i].editTexID = GET_TEXTURE(GRID);
                    g_Box[i].movementCost = 1;
                    g_Box[i].effect = EFFECT_NONE;
                    break;
                }

                // 🔧 **新增**: 初始化动画和渐变属性
                g_Box[i].isFading = false;
                g_Box[i].fadeProgress = 1.0f;
                g_Box[i].fadeDelay = 0.0f;
                g_Box[i].hoverHeightLerp = 0.0f;
                g_Box[i].hoverScaleLerp = 1.0f;

                createdBoxes++;
            }
        }
    }

    char debugMsg[128];
    sprintf_s(debugMsg, "✅ CreateGround完成: 创建了%d个地面对象\n", createdBoxes);
    OutputDebugStringA(debugMsg);
}

void CreateObstacle()
{
    OutputDebugStringA("🗺️ CreateObstacle: 开始创建障碍物对象\n");

    static bool hasBeenInitialized = false;

    for (int i = 0; i < MAPSIZE_X * MAPSIZE_Z; i++) {
        g_Obstacle[i] = BoxObject{};
    }

    int createdObstacles = 0;

    for (int z = 0; z < MAPSIZE_Z; z++)
    {
        for (int x = 0; x < MAPSIZE_X; x++)
        {
            if (OBSTACLE_MAP[z][x] != 0)
            {
                int i = z * MAPSIZE_X + x; // 直接计算索引

                g_Obstacle[i].ObjectNo = OBSTACLE_MAP[z][x];
                g_Obstacle[i].Use = true;
                g_Obstacle[i].position.x = x * BOXSIZE_X + (BOXSIZE_X / 2);
                g_Obstacle[i].position.z = -z * BOXSIZE_Z + (BOXSIZE_Z / 2);
                g_Obstacle[i].rotate = XMFLOAT3(0.0f, 0.0f, 0.0f);
                g_Obstacle[i].scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
                g_Obstacle[i].Radius = BOX_RADIUS;

                switch (OBSTACLE_MAP[z][x])
                {
                case WALL:
                    g_Obstacle[i].position.y = (BOXSIZE_Y / 2 + 0.01f);
                    g_Obstacle[i].TexID = GET_TEXTURE(WALL);
                    g_Obstacle[i].model = NULL;
                    g_Obstacle[i].useProcedural = false;
                    g_Obstacle[i].SizeMin = XMFLOAT3(-BOXSIZE_X / 2, -BOXSIZE_Y / 2, -BOXSIZE_Z / 2);
                    g_Obstacle[i].SizeMax = XMFLOAT3(BOXSIZE_X / 2, BOXSIZE_Y / 2, BOXSIZE_Z / 2);
                    break;

                case TREE:
                    g_Obstacle[i].position.y = 0.5f;
                    g_Obstacle[i].model = Tree;
                    g_Obstacle[i].TexID = NULL;
                    g_Obstacle[i].useProcedural = false;
                    g_Obstacle[i].SizeMin = XMFLOAT3(-0.5f, -0.5f, -0.5f);
                    g_Obstacle[i].SizeMax = XMFLOAT3(0.5f, 0.5f, 0.5f);
                    break;

                case START:
                    g_Obstacle[i].position.y = (BOXSIZE_Y / 2);
                    g_Obstacle[i].TexID = GET_TEXTURE(START);
                    g_Obstacle[i].model = NULL;
                    g_Obstacle[i].useProcedural = false;
                    g_Obstacle[i].SizeMin = XMFLOAT3(-BOXSIZE_X / 2, -BOXSIZE_Y / 2, -BOXSIZE_Z / 2);
                    g_Obstacle[i].SizeMax = XMFLOAT3(BOXSIZE_X / 2, BOXSIZE_Y / 2, BOXSIZE_Z / 2);
                    break;

                case COIN:
                    g_Obstacle[i].position.y = (BOXSIZE_Y / 2);
                    g_Obstacle[i].model = NULL;
                    g_Obstacle[i].TexID = GET_TEXTURE(COIN);
                    g_Obstacle[i].useProcedural = true;
                    g_Obstacle[i].proceduralType = 1; // 金币类型
                    g_Obstacle[i].SizeMin = XMFLOAT3(-BOXSIZE_X / 2, -BOXSIZE_Y / 2, -BOXSIZE_Z / 2);
                    g_Obstacle[i].SizeMax = XMFLOAT3(BOXSIZE_X / 2, BOXSIZE_Y / 2, BOXSIZE_Z / 2);
                    break;

                case BOMB:
                    g_Obstacle[i].position.y = (BOXSIZE_Y / 2);
                    g_Obstacle[i].model = NULL;
                    g_Obstacle[i].TexID = GET_TEXTURE(BOMB);
                    g_Obstacle[i].useProcedural = true;
                    g_Obstacle[i].proceduralType = 2; // 炸弹类型
                    g_Obstacle[i].SizeMin = XMFLOAT3(-BOXSIZE_X / 2, -BOXSIZE_Y / 2, -BOXSIZE_Z / 2);
                    g_Obstacle[i].SizeMax = XMFLOAT3(BOXSIZE_X / 2, BOXSIZE_Y / 2, BOXSIZE_Z / 2);
                    break;

                case GOAL:
                    g_Obstacle[i].position.y = (BOXSIZE_Y / 2);
                    g_Obstacle[i].TexID = GET_TEXTURE(GOAL);
                    g_Obstacle[i].model = NULL;
                    g_Obstacle[i].useProcedural = false;
                    g_Obstacle[i].SizeMin = XMFLOAT3(-BOXSIZE_X / 2, -BOXSIZE_Y / 2, -BOXSIZE_Z / 2);
                    g_Obstacle[i].SizeMax = XMFLOAT3(BOXSIZE_X / 2, BOXSIZE_Y / 2, BOXSIZE_Z / 2);
                    break;

                default:
                    g_Obstacle[i].position.y = (BOXSIZE_Y / 2);
                    g_Obstacle[i].TexID = GET_TEXTURE(GROUND);
                    g_Obstacle[i].model = NULL;
                    g_Obstacle[i].useProcedural = false;
                    g_Obstacle[i].SizeMin = XMFLOAT3(-BOXSIZE_X / 2, -BOXSIZE_Y / 2, -BOXSIZE_Z / 2);
                    g_Obstacle[i].SizeMax = XMFLOAT3(BOXSIZE_X / 2, BOXSIZE_Y / 2, BOXSIZE_Z / 2);
                    break;
                }

                createdObstacles++;
            }
        }
    }

   

    hasBeenInitialized = true;
}
// 盒子顶点索引数据
UINT boxIndex[36] =
{
    0, 1, 2,    //天井
    3, 4, 5,

    6, 7, 8,    //前面
    9,10,11,

    12,13,14,    //右側面
    15,16,17,

    18,19,20,    //背面
    21,22,23,

    24,25,26,    //左側面
    27,28,29,

    30,31,32,    //底面    未使用
    33,34,35,
};

#define BOX_NUM_VERTEX    (36)

// 盒子顶点数据
VERTEX_3D Box[BOX_NUM_VERTEX] =
{
    // 顶面（天井） +Y
    { {-0.5f,  0.5f,  0.5f}, {0, 1, 0}, {1,1,1,1}, {0.0f, 0.0f} },
    { { 0.5f,  0.5f,  0.5f}, {0, 1, 0}, {1,1,1,1}, {1.0f, 0.0f} },
    { {-0.5f,  0.5f, -0.5f}, {0, 1, 0}, {1,1,1,1}, {0.0f, 1.0f} },
    { { 0.5f,  0.5f,  0.5f}, {0, 1, 0}, {1,1,1,1}, {1.0f, 0.0f} },
    { { 0.5f,  0.5f, -0.5f}, {0, 1, 0}, {1,1,1,1}, {1.0f, 1.0f} },
    { {-0.5f,  0.5f, -0.5f}, {0, 1, 0}, {1,1,1,1}, {0.0f, 1.0f} },

    // 底面 -Y
    { {-0.5f, -0.5f, -0.5f}, {0,-1, 0}, {1,1,1,1}, {0.0f, 0.0f} },
    { { 0.5f, -0.5f, -0.5f}, {0,-1, 0}, {1,1,1,1}, {1.0f, 0.0f} },
    { {-0.5f, -0.5f,  0.5f}, {0,-1, 0}, {1,1,1,1}, {0.0f, 1.0f} },
    { { 0.5f, -0.5f, -0.5f}, {0,-1, 0}, {1,1,1,1}, {1.0f, 0.0f} },
    { { 0.5f, -0.5f,  0.5f}, {0,-1, 0}, {1,1,1,1}, {1.0f, 1.0f} },
    { {-0.5f, -0.5f,  0.5f}, {0,-1, 0}, {1,1,1,1}, {0.0f, 1.0f} },

    // 前面 -Z
    { {-0.5f,  0.5f, -0.5f}, {0, 0,-1}, {1,1,1,1}, {0.0f, 0.0f} },
    { { 0.5f,  0.5f, -0.5f}, {0, 0,-1}, {1,1,1,1}, {1.0f, 0.0f} },
    { {-0.5f, -0.5f, -0.5f}, {0, 0,-1}, {1,1,1,1}, {0.0f, 1.0f} },
    { { 0.5f,  0.5f, -0.5f}, {0, 0,-1}, {1,1,1,1}, {1.0f, 0.0f} },
    { { 0.5f, -0.5f, -0.5f}, {0, 0,-1}, {1,1,1,1}, {1.0f, 1.0f} },
    { {-0.5f, -0.5f, -0.5f}, {0, 0,-1}, {1,1,1,1}, {0.0f, 1.0f} },

    // 背面 +Z
    { { 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1,1,1,1}, {0.0f, 0.0f} },
    { {-0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1,1,1,1}, {1.0f, 0.0f} },
    { { 0.5f, -0.5f,  0.5f}, {0, 0, 1}, {1,1,1,1}, {0.0f, 1.0f} },
    { {-0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1,1,1,1}, {1.0f, 0.0f} },
    { {-0.5f, -0.5f,  0.5f}, {0, 0, 1}, {1,1,1,1}, {1.0f, 1.0f} },
    { { 0.5f, -0.5f,  0.5f}, {0, 0, 1}, {1,1,1,1}, {0.0f, 1.0f} },

    // 左面 -X
    { {-0.5f,  0.5f,  0.5f}, {-1,0,0}, {1,1,1,1}, {0.0f, 0.0f} },
    { {-0.5f,  0.5f, -0.5f}, {-1,0,0}, {1,1,1,1}, {1.0f, 0.0f} },
    { {-0.5f, -0.5f,  0.5f}, {-1,0,0}, {1,1,1,1}, {0.0f, 1.0f} },
    { {-0.5f,  0.5f, -0.5f}, {-1,0,0}, {1,1,1,1}, {1.0f, 0.0f} },
    { {-0.5f, -0.5f, -0.5f}, {-1,0,0}, {1,1,1,1}, {1.0f, 1.0f} },
    { {-0.5f, -0.5f,  0.5f}, {-1,0,0}, {1,1,1,1}, {0.0f, 1.0f} },

    // 右面 +X
    { { 0.5f,  0.5f, -0.5f}, {1,0,0}, {1,1,1,1}, {0.0f, 0.0f} },
    { { 0.5f,  0.5f,  0.5f}, {1,0,0}, {1,1,1,1}, {1.0f, 0.0f} },
    { { 0.5f, -0.5f, -0.5f}, {1,0,0}, {1,1,1,1}, {0.0f, 1.0f} },
    { { 0.5f,  0.5f,  0.5f}, {1,0,0}, {1,1,1,1}, {1.0f, 0.0f} },
    { { 0.5f, -0.5f,  0.5f}, {1,0,0}, {1,1,1,1}, {1.0f, 1.0f} },
    { { 0.5f, -0.5f, -0.5f}, {1,0,0}, {1,1,1,1}, {0.0f, 1.0f} },
};

// 初始化盒子
void InitBox()
{
    if (!TextureManager::Init()) {
        OutputDebugStringA("❌ 纹理管理器初始化失败\n");
        return;
    }

    // 加载树模型
    Tree = ModelLoad("asset\\model\\tree.fbx");
    // 创建顶点缓冲区
    {
        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(VERTEX_3D) * BOX_NUM_VERTEX;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
        GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

        // 获取顶点缓冲区的写入指针
        D3D11_MAPPED_SUBRESOURCE msr;
        GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
        VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

        // 复制顶点数据
        CopyMemory(&vertex[0], &Box[0], sizeof(VERTEX_3D) * BOX_NUM_VERTEX);

        // 完成写入
        GetDeviceContext()->Unmap(g_VertexBuffer, 0);
    }

    // 创建索引缓冲区
    {
        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(UINT) * BOX_NUM_VERTEX;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
        GetDevice()->CreateBuffer(&bd, NULL, &g_IndexBuffer);

        // 获取索引缓冲区的写入指针
        D3D11_MAPPED_SUBRESOURCE msr;
        GetDeviceContext()->Map(g_IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
        UINT* index = (UINT*)msr.pData;

        // 复制索引数据
        CopyMemory(&index[0], &boxIndex[0], sizeof(UINT) * BOX_NUM_VERTEX);

        // 完成写入
        GetDeviceContext()->Unmap(g_IndexBuffer, 0);
    }

    // 加载纹理
    TexMetadata metadata;
    ScratchImage image;

    

    // 设置光照
    Light.Direction = XMFLOAT4(1.0f, -1.0f, 1.0f, 0.0f);
    Light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    Light.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    Light.Enable = true;
    XMVECTOR vec = XMLoadFloat4(&Light.Direction);
    vec = XMVector4Normalize(vec);
    XMStoreFloat4(&Light.Direction, vec);

    // 初始化对象
    ZeroMemory(&g_Box[0], sizeof(BoxObject) * MAPSIZE_X * MAPSIZE_Z);
    ZeroMemory(&g_Obstacle[0], sizeof(BoxObject) * MAPSIZE_X * MAPSIZE_Z);

    TextureManager::Init();

    // 创建地面和障碍物
    CreateGround();
    CreateObstacle();

    // 初始化地图编辑器
    InitMapEditor();

    // 初始化UI盒子
    InitUIBoxes();

    // 初始化障碍物UI盒子
    InitObstacleUIBoxes();

    // 默认为地形编辑模式
    IsEditingObstacle();

    UpdateBoxTactical();
    g_SelectedObstacleUIBoxIndex = -1;
}

// 清理盒子资源
void UninitBox()
{
    ModelRelease(Tree);
  

    if (g_VertexBuffer != NULL)
    {
        g_VertexBuffer->Release();
        g_VertexBuffer = NULL;
    }

    if (g_IndexBuffer != NULL)
    {
        g_IndexBuffer->Release();
        g_IndexBuffer = NULL;
    }

   

    // 清理地图编辑器
    UninitMapEditor();
    // 清理其他全局变量（如果需要）
    g_SelectedObstacleUIBoxIndex = -1;
}

// 更新盒子
void UpdateBox()
{
    BoxObject* boxes = GetBox();
    for (int i = 0; i < MAPSIZE_X * MAPSIZE_Z; ++i)
    {
        if (!boxes[i].Use)
            continue;

        int gridX = i % MAPSIZE_X;
        int gridZ = i / MAPSIZE_X;

        bool isHover = (gridX == g_HoverX && gridZ == g_HoverZ);

        // 目标值
        float targetHeight = isHover ? 0.1f : 0.0f;
        float targetScale = isHover ? 1.1f : 1.0f;

        // 插值速度（可调整）
        const float SPEED = 0.2f;

        // 高度缓动
        boxes[i].hoverHeightLerp += (targetHeight - boxes[i].hoverHeightLerp) * SPEED;

        // 缩放缓动
        boxes[i].hoverScaleLerp += (targetScale - boxes[i].hoverScaleLerp) * SPEED;
    }
}

// 更新盒子淡入淡出效果
void UpdateBoxFade(float dt)
{
    for (int i = 0; i < MAPSIZE_X * MAPSIZE_Z; i++) {
        if (!g_Box[i].Use || !g_Box[i].isFading) continue;

        if (g_Box[i].fadeDelay > 0.0f) {
            g_Box[i].fadeDelay -= dt;
        }
        else {
            g_Box[i].fadeProgress += dt * 1.5f;
            if (g_Box[i].fadeProgress >= 1.0f) {
                g_Box[i].fadeProgress = 1.0f;
                // 注意：在编辑模式下，我们不重置isFading标志
                if (GetCurrentAppMode() == APP_MODE_EDIT) {
                    //g_Box[i].isFading = false;
                    g_Box[i].fadeProgress = 1.0f;
                }
            }
        }
    }
}

// 绘制盒子
void DrawBox()
{
    // 在DrawBox函数开头添加模式检查
    AppMode currentMode = GetCurrentAppMode();
    int maxX = (currentMode == APP_MODE_MULTIPLAYER_BATTLE) ? BATTLE_MAP_SIZE_X : MAPSIZE_X;
    int maxZ = (currentMode == APP_MODE_MULTIPLAYER_BATTLE) ? BATTLE_MAP_SIZE_Z : MAPSIZE_Z;
    int maxObjects = maxX * maxZ;
    // 设置光照
    SetLight(Light);
    SetATCEnable(true);

    // 在开始的地方重置状态
    extern int g_FrameCount; // 确保在Game.cpp中定义
    static int g_LastFrameRendered = -1;

    if (g_LastFrameRendered != g_FrameCount) {
        g_AllProceduralModelsRendered = false;
        g_LastFrameRendered = g_FrameCount;
        //OutputDebugStringA("新的一帧开始，重置程序化模型渲染状态\n");
    }


    // 绘制地面
    {
        MATERIAL material;
        ZeroMemory(&material, sizeof(MATERIAL));
        material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

        for (int i = 0; i < (maxObjects); i++)
        {
            if (g_Box[i].Use != true) continue;

            // 设置位置和缩放（包括悬停效果）
            XMFLOAT3 pos = g_Box[i].position;
            XMFLOAT3 scale = g_Box[i].scale;

            int gridX = i % MAPSIZE_X;
            int gridZ = i / MAPSIZE_X;

            if (gridX == g_HoverX && gridZ == g_HoverZ) {
                pos.y += g_Box[i].hoverHeightLerp;
                scale.x *= g_Box[i].hoverScaleLerp;
                scale.y *= g_Box[i].hoverScaleLerp;
                scale.z *= g_Box[i].hoverScaleLerp;
            }

            // 设置世界矩阵
            XMMATRIX TranslationMatrix = XMMatrixTranslation(pos.x, pos.y, pos.z);
            XMMATRIX ScalingMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
            XMMATRIX WorldMatrix = ScalingMatrix * TranslationMatrix;
            SetWorldMatrix(WorldMatrix);

            // 设置顶点缓冲区
            UINT stride = sizeof(VERTEX_3D);
            UINT offset = 0;
            GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
            GetDeviceContext()->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
            GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // 选择要显示的纹理
            ID3D11ShaderResourceView* texToUse = NULL;

            // 保存当前深度写入状态
            bool oldDepthWriteEnable = GetDepthWriteEnable();

            material.Diffuse = XMFLOAT4(1, 1, 1, 1);
            SetMaterial(material);
            SetATCEnable(false);

            // ★ 编辑模式下——如果要高亮“未修改格子”，换贴图即可；这里先统一贴上 g_TextureGrid
            if (GetCurrentAppMode() == APP_MODE_EDIT && !g_Box[i].isFading) {
                texToUse = GET_TEXTURE(GRID);          // 网格贴图
            }
            else {
                texToUse = g_Box[i].TexID ? g_Box[i].TexID : GET_TEXTURE(GROUND);
            }



        if (texToUse) {
            GetDeviceContext()->PSSetShaderResources(0, 1, &texToUse);
            SetCulingMode(D3D11_CULL_BACK);

            // 修正：绘制所有面 - 使用全部BOX_NUM_VERTEX而不是减去6
            GetDeviceContext()->DrawIndexed(BOX_NUM_VERTEX, 0, 0);

          
        }
        }

    }
    // 绘制障碍物
    {
        MATERIAL material;
        ZeroMemory(&material, sizeof(MATERIAL));
        material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
      
        // 程序化模型（金币、炸弹）的集中渲染
        if (!g_AllProceduralModelsRendered) {
            //OutputDebugStringA("开始集中渲染所有程序化模型\n");

            // 一次性渲染所有金币和炸弹
            for (int i = 0; i < maxObjects; i++) {
                if (g_Obstacle[i].Use && g_Obstacle[i].useProcedural) {
                    // 设置基本变换矩阵
                    XMMATRIX TranslationMatrix = XMMatrixTranslation(
                        g_Obstacle[i].position.x,
                        g_Obstacle[i].position.y,
                        g_Obstacle[i].position.z
                    );
                    XMMATRIX RotationMatrix = XMMatrixRotationY(g_Obstacle[i].rotate.y);
                    XMMATRIX ScalingMatrix = XMMatrixScaling(
                        g_Obstacle[i].scale.x,
                        g_Obstacle[i].scale.y,
                        g_Obstacle[i].scale.z
                    );

                    // 在Box.cpp中的DrawBox函数中
                    if (g_Obstacle[i].ObjectNo == COIN) {
                        float rotation = GetCoinRotation();
                        // 使金币旋转时更好地展示其扁平特性
  // 轻微倾斜以便能看到厚度，然后围绕Y轴旋转
                        float tiltAngle = XM_PI / 6.0f; // 30度倾斜
                        XMMATRIX TiltMatrix = XMMatrixRotationX(tiltAngle);
                        XMMATRIX RotMatrixY = XMMatrixRotationY(rotation);

                        XMMATRIX WorldMatrix = ScalingMatrix * TiltMatrix * RotMatrixY * TranslationMatrix;
                        DrawProceduralModel(&g_CoinModel, WorldMatrix);
                    }
                    else if (g_Obstacle[i].ObjectNo == BOMB) {
                        float rotation = GetBombRotation();
                        XMMATRIX RotMatrixFull = XMMatrixRotationY(rotation * 0.5f);
                        XMMATRIX WorldMatrix = ScalingMatrix * RotMatrixFull * TranslationMatrix;
                        // 使用综合渲染函数，同时绘制炸弹和引信
                        DrawCompleteBomb(WorldMatrix);
                    }
                }
            }

            g_AllProceduralModelsRendered = true;
            //OutputDebugStringA("完成所有程序化模型渲染\n");
        }

        for (int i = 0; i < (MAPSIZE_Z * MAPSIZE_X); i++)
        {
            if (g_Obstacle[i].Use != true) continue;

            // 跳过已处理的程序化模型
            if (g_Obstacle[i].useProcedural) continue;
            /* 1️⃣ 保存进入这一轮时的剔除模式 */
            D3D11_CULL_MODE oldCull = GetCullingMode();
            // 设置基本变换矩阵
            XMMATRIX TranslationMatrix = XMMatrixTranslation(
                g_Obstacle[i].position.x,
                g_Obstacle[i].position.y,
                g_Obstacle[i].position.z
            );
            XMMATRIX RotationMatrix = XMMatrixRotationY(g_Obstacle[i].rotate.y);
            XMMATRIX ScalingMatrix = XMMatrixScaling(
                g_Obstacle[i].scale.x,
                g_Obstacle[i].scale.y,
                g_Obstacle[i].scale.z
            );

            // 设置顶点缓冲区
            UINT stride = sizeof(VERTEX_3D);
            UINT offset = 0;
            GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

            // 根据对象类型选择渲染方式
            switch (g_Obstacle[i].ObjectNo)
            {
     
            case TREE:
                if (g_Obstacle[i].model) {
                    XMMATRIX WorldMatrix = ScalingMatrix * RotationMatrix * TranslationMatrix;
                    SetWorldMatrix(WorldMatrix);
                    ModelDraw(g_Obstacle[i].model);
                }
                break;

                // 处理其他类型的障碍物
            default:
                if (g_Obstacle[i].TexID) {
                    // 设置世界矩阵
                    XMMATRIX WorldMatrix = ScalingMatrix * RotationMatrix * TranslationMatrix;
                    SetWorldMatrix(WorldMatrix);

                    // 设置索引缓冲区和渲染状态
                    GetDeviceContext()->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
                    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                    // 设置材质和纹理
                    SetMaterial(material);
                    GetDeviceContext()->PSSetShaderResources(0, 1, &g_Obstacle[i].TexID);

                    // 根据对象类型设置剔除模式
                    if (g_Obstacle[i].ObjectNo == START || g_Obstacle[i].ObjectNo == GOAL) {
                        SetCulingMode(D3D11_CULL_NONE);
                    }
                    else {
                        SetCulingMode(D3D11_CULL_BACK);
                    }

                    // 绘制 - 使用所有面
                    GetDeviceContext()->DrawIndexed(BOX_NUM_VERTEX, 0, 0);
                }
                break;
            }
            SetCulingMode(oldCull);
        }
    }
}
// 绘制盒子轮廓
void DrawBoxOutline(BoxObject& box)
{
    // 保存当前的渲染状态
    D3D11_PRIMITIVE_TOPOLOGY oldTopology;
    GetDeviceContext()->IAGetPrimitiveTopology(&oldTopology);

    // 设置线框模式
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    // 创建临时材质
    MATERIAL outlineMaterial;
    ZeroMemory(&outlineMaterial, sizeof(MATERIAL));
    outlineMaterial.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); // 黑色边界
    SetMaterial(outlineMaterial);

    // 不要改变世界矩阵，直接使用当前的

    // 确保正确绑定缓冲区
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
    GetDeviceContext()->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // 绘制边界 - 使用不同的绘制调用
    // 注意：可能需要专门为线框模式准备不同的索引缓冲区
    GetDeviceContext()->Draw(BOX_NUM_VERTEX, 0); // 不使用索引绘制

    // 恢复原始拓扑
    GetDeviceContext()->IASetPrimitiveTopology(oldTopology);
}

// 触发全部 Box 的淡蓝色渐变
void TriggerBoxFade()
{
    for (int z = 0; z < MAPSIZE_Z; z++) {
        for (int x = 0; x < MAPSIZE_X; x++) {
            int index = z * MAPSIZE_X + x;
            BoxObject& box = g_Box[index]; // 你的 box 数组
            box.isFading = true;
            box.fadeProgress = 0.0f;
            box.fadeDelay = z * 0.05f; // 上到下依次延迟（或改为 x 控制左右）
        }
    }
}

// 获取盒子对象数组
BoxObject* GetBox()
{
    return &g_Box[0];
}

// 获取障碍物对象数组
BoxObject* GetObstacle()
{
    return &g_Obstacle[0];
}

// 地面碰撞检测
int GroundCollision()
{
    BallObject* pball = GetBall();

    for (int i = 0; i < MAPSIZE_Z * MAPSIZE_X; i++)
    {
        if (g_Box[i].Use == true)
        {
            for (int n = 0; n < 5; n++)
            {
                bool hit = CollisionBoxBox(pball, &g_Box[i]);
                if (hit)
                {
                    //地面に落ちたら座標を補正する
                    float posy = g_Box[i].position.y;
                    posy += (g_Box[i].SizeMax.y + pball->Radius);
                    pball->position.y = posy;

                    return i;
                }
                else
                {
                    continue;
                }
            }
        }
    }

    for (int i = 0; i < MAPSIZE_Z * MAPSIZE_X; i++)
    {
        if (g_Obstacle[i].Use == true)
        {
            for (int n = 0; n < 5; n++)
            {
                bool hit = CollisionBoxBox(pball, &g_Obstacle[i]);
                if (hit)
                {
                    //地面に落ちたら座標を補正する
                    float posy = g_Obstacle[i].position.y;
                    posy += (g_Obstacle[i].SizeMax.y + pball->Radius);
                    pball->position.y = posy;
                    return i;
                }
                else
                {
                    continue;
                }
            }

        }
    }
    return -1;
}

// 障碍物碰撞检测
int ObstacleCollision()
{
    BallObject* pball = GetBall();
    XMFLOAT3 point = XMFLOAT3(0, 0, 0);
    XMVECTOR vec = XMLoadFloat3(&pball->Velocity);
    vec = XMVector3Normalize(vec);
    XMStoreFloat3(&point, vec);

    //前後左右のボールの表面の座標
    XMFLOAT3 fwd, back, left, right;
    XMFLOAT3 pos = pball->position;
    fwd = XMFLOAT3(pos.x, pos.y, pos.z + pball->Radius);
    back = XMFLOAT3(pos.x, pos.y, pos.z - pball->Radius);
    left = XMFLOAT3(pos.x - pball->Radius, pos.y, pos.z);
    right = XMFLOAT3(pos.x + pball->Radius, pos.y, pos.z);

    for (int i = 0; i < MAPSIZE_Z * MAPSIZE_X; i++)
    {
        if ((g_Obstacle[i].Use == true) &&
            (g_Obstacle[i].ObjectNo != COIN) &&
            (g_Obstacle[i].ObjectNo != BOMB))
        {
            XMFLOAT3 boxMin = XMFLOAT3(g_Obstacle[i].position.x + g_Obstacle[i].SizeMin.x,
                g_Obstacle[i].position.y + g_Obstacle[i].SizeMin.y,
                g_Obstacle[i].position.z + g_Obstacle[i].SizeMin.z);
            XMFLOAT3 boxMax = XMFLOAT3(g_Obstacle[i].position.x + g_Obstacle[i].SizeMax.x,
                g_Obstacle[i].position.y + g_Obstacle[i].SizeMax.y,
                g_Obstacle[i].position.z + g_Obstacle[i].SizeMax.z);

            float hit;
            //前
            hit = CollisionBoxPoint(fwd, &g_Obstacle[i]);
            if (hit == true)
            {
                pball->Velocity.z *= -1.0f;
                return 0;
            }
            //後ろ
            hit = CollisionBoxPoint(back, &g_Obstacle[i]);
            if (hit == true)
            {
                pball->Velocity.z *= -1.0f;
                return 0;
            }
            //左
            hit = CollisionBoxPoint(left, &g_Obstacle[i]);
            if (hit == true)
            {
                pball->Velocity.x *= -1.0f;
                return 0;
            }
            //右
            hit = CollisionBoxPoint(right, &g_Obstacle[i]);
            if (hit == true)
            {
                pball->Velocity.x *= -1.0f;
                return 0;
            }
        }
    }
    return -1;
}

// 障碍物地面碰撞检测
int ObstacleGroundCollision()
{
    BallObject* pball = GetBall();
    XMFLOAT3 point = XMFLOAT3(0, 0, 0);

    for (int i = 0; i < MAPSIZE_Z * MAPSIZE_X; i++)
    {
        if ((g_Obstacle[i].Use == true) && (g_Obstacle[i].ObjectNo != START))
        {
            float hit = CollisionSphereBox(pball, &g_Obstacle[i], &point);
            if (hit != 0.0f)
            {
                XMFLOAT3 boxMin = XMFLOAT3(g_Obstacle[i].position.x + g_Obstacle[i].SizeMin.x,
                    g_Obstacle[i].position.y + g_Obstacle[i].SizeMin.y,
                    g_Obstacle[i].position.z + g_Obstacle[i].SizeMin.z);
                XMFLOAT3 boxMax = XMFLOAT3(g_Obstacle[i].position.x + g_Obstacle[i].SizeMax.x,
                    g_Obstacle[i].position.y + g_Obstacle[i].SizeMax.y,
                    g_Obstacle[i].position.z + g_Obstacle[i].SizeMax.z);

                //+y上方から？
                if (boxMax.y <= point.y)
                {
                    //ボールの位置補正
                    pball->position.y = boxMax.y + pball->Radius;
                    return i;
                }
            }
        }
    }
    return -1;
}

// 更新地面盒子的战旗游戏属性
void UpdateBoxTactical() {
    for (int i = 0; i < (MAPSIZE_Z * MAPSIZE_X); i++) {
        if (g_Box[i].Use) {
            // 根据地面类型设置移动力消耗和特效
            g_Box[i].movementCost = MOVEMENT_COST[g_Box[i].groundType];
            g_Box[i].effect = TERRAIN_EFFECTS[g_Box[i].groundType];
        }
    }
}

// 判断格子是否可通行
bool IsGridWalkable(int x, int z) {
    char debug[256];
    sprintf_s(debug, "检查格子 (%d,%d) 是否可通行\n", x, z);
    OutputDebugStringA(debug);

    // 地图边界检查
    if (x < 0 || x >= MAPSIZE_X || z < 0 || z >= MAPSIZE_Z) {
        OutputDebugStringA("  格子超出地图边界，不可通行\n");
        return false;
    }

    // 检查地面是否存在
    bool hasGround = false;
    for (int i = 0; i < (MAPSIZE_Z * MAPSIZE_X); i++) {
        if (g_Box[i].Use) {
            int gridX = (int)((g_Box[i].position.x - (BOXSIZE_X / 2)) / BOXSIZE_X);
            int gridZ = (int)(-(g_Box[i].position.z - (BOXSIZE_Z / 2)) / BOXSIZE_Z);

            if (gridX == x && gridZ == z) {
                int cost = g_Box[i].movementCost;
                sprintf_s(debug, "  找到格子 (%d,%d)，移动力消耗: %d\n", gridX, gridZ, cost);
                OutputDebugStringA(debug);

                if (cost >= 99) {
                    OutputDebugStringA("  地形不可通行\n");
                    return false;
                }
                hasGround = true;
                break;
            }
        }
    }

    if (!hasGround) {
        OutputDebugStringA("  没有找到对应的地面格子，不可通行\n");
        return false;
    }

    // 检查是否有阻挡性障碍物
    for (int i = 0; i < (MAPSIZE_Z * MAPSIZE_X); i++) {
        if (g_Obstacle[i].Use) {
            int objX = (int)((g_Obstacle[i].position.x - (BOXSIZE_X / 2)) / BOXSIZE_X);
            int objZ = (int)(-(g_Obstacle[i].position.z - (BOXSIZE_Z / 2)) / BOXSIZE_Z);

            if (objX == x && objZ == z) {
                // 只有金币和炸弹可以通过，其他障碍物都阻挡
                if (g_Obstacle[i].ObjectNo != COIN && g_Obstacle[i].ObjectNo != BOMB) {
                    sprintf_s(debug, "  格子被障碍物阻挡(类型:%d)，不可通行\n", g_Obstacle[i].ObjectNo);
                    OutputDebugStringA(debug);
                    return false;
                }
            }
        }
    }

    OutputDebugStringA("  格子可通行\n");
    return true;
}
// 获取格子的移动力消耗
int GetGridMovementCost(int x, int z) {
    for (int i = 0; i < (MAPSIZE_Z * MAPSIZE_X); i++) {
        if (g_Box[i].Use) {
            int gridX = (int)((g_Box[i].position.x - (BOXSIZE_X / 2)) / BOXSIZE_X);
            int gridZ = (int)(-(g_Box[i].position.z - (BOXSIZE_Z / 2)) / BOXSIZE_Z);

            if (gridX == x && gridZ == z) {
                return g_Box[i].movementCost;
            }
        }
    }
    return 99;  // 默认为不可通行
}

// 获取格子的特殊效果
TerrainEffect GetGridEffect(int x, int z) {
    for (int i = 0; i < (MAPSIZE_Z * MAPSIZE_X); i++) {
        if (g_Box[i].Use) {
            int gridX = (int)((g_Box[i].position.x - (BOXSIZE_X / 2)) / BOXSIZE_X);
            int gridZ = (int)(-(g_Box[i].position.z - (BOXSIZE_Z / 2)) / BOXSIZE_Z);

            if (gridX == x && gridZ == z) {
                return g_Box[i].effect;
            }
        }
    }
    return EFFECT_NONE;
}

void ApplyGridEffect(int x, int z) {
    TerrainEffect effect = GetGridEffect(x, z);
    BallObject* ball = GetBall();

    static int lastGridX = -999;
    static int lastGridZ = -999;
    static bool textCreated = false;

    if (lastGridX != x || lastGridZ != z) {
        lastGridX = x;
        lastGridZ = z;
        textCreated = false;
    }

    if (!textCreated && effect != EFFECT_NONE) {
        switch (effect) {
        case EFFECT_SLOW: {
            int oldMovementPoints = ball->maxMovementPoints;
            ball->maxMovementPoints = std::max(1, ball->maxMovementPoints - 1);
            ParticleSystem::CreateSlowEffect(ball->position, 60);

            if (!textCreated) {
                // 显示多语言支持
                FloatingTextSystem::ShowText(ball->position, "SLOWDOWN!", XMFLOAT4(0.9f, 0.85f, 0.7f, 1.0f));
                textCreated = true;
                OutputDebugStringA("创建SLOWDOWN文本\n");

              

            }
        }
                        break;

        case EFFECT_HEAL: {
            int oldHealth = ball->health;
            ball->health = std::min(ball->maxHealth, ball->health + 5);
            int healAmount = ball->health - oldHealth;

            if (healAmount > 0 && !textCreated) {
                ParticleSystem::CreateHealEffect(ball->position, 50);
                FloatingTextSystem::ShowText(ball->position, "HEALING!", XMFLOAT4(0.3f, 1.0f, 0.4f, 1.0f));
                textCreated = true;
                OutputDebugStringA("创建HEALING文本\n");
            }
        }
                        break;

        case EFFECT_SLIPPERY: {
            ball->isSlipping = true;

            if (rand() % 100 < 50 && !textCreated) {
                ParticleSystem::CreateSlipperyEffect(ball->position, 60);
                FloatingTextSystem::ShowText(ball->position, "SLIPP!", XMFLOAT4(0.6f, 0.9f, 1.0f, 1.0f));
                textCreated = true;
                OutputDebugStringA("创建SLIPP文本\n");
            }
        }
                            break;

        case EFFECT_DAMAGE: {
            int oldHealth = ball->health;
            ball->health = std::max(0, ball->health - 10);
            int damageAmount = oldHealth - ball->health;

            if (damageAmount > 0 && !textCreated) {
                ParticleSystem::CreateDamageEffect(ball->position, 60);
                FloatingTextSystem::ShowText(ball->position, "HURTING!", XMFLOAT4(1.0f, 0.3f, 0.2f, 1.0f));
                textCreated = true;
                OutputDebugStringA("创建HURTING文本\n");
            }
        }
                          break;

        default:
            break;
        }
    }
}

// 给Box.cpp添加碰撞检测函数
bool CheckBallBoxCollision(BallObject* ball, BoxObject* box) {
    if (!ball || !box || !box->Use) return false;

    // 球体边界框
    float ballRadius = ball->Radius;
    XMFLOAT3 ballMin = XMFLOAT3(
        ball->position.x - ballRadius,
        ball->position.y - ballRadius,
        ball->position.z - ballRadius
    );
    XMFLOAT3 ballMax = XMFLOAT3(
        ball->position.x + ballRadius,
        ball->position.y + ballRadius,
        ball->position.z + ballRadius
    );

    // 盒子边界框
    XMFLOAT3 boxMin = XMFLOAT3(
        box->position.x - box->scale.x * 0.5f,
        box->position.y - box->scale.y * 0.5f,
        box->position.z - box->scale.z * 0.5f
    );
    XMFLOAT3 boxMax = XMFLOAT3(
        box->position.x + box->scale.x * 0.5f,
        box->position.y + box->scale.y * 0.5f,
        box->position.z + box->scale.z * 0.5f
    );

    // AABB碰撞检测
    if (ballMax.x < boxMin.x || ballMin.x > boxMax.x) return false;
    if (ballMax.y < boxMin.y || ballMin.y > boxMax.y) return false;
    if (ballMax.z < boxMin.z || ballMin.z > boxMax.z) return false;

    return true;
}


// 🔧 **在Box.cpp中添加这些函数实现**
ID3D11Buffer* GetBoxVertexBuffer() {
    return g_VertexBuffer;
}

ID3D11Buffer* GetBoxIndexBuffer() {
    return g_IndexBuffer;
}

int GetBoxIndexCount() {
    return 36; // BOX_NUM_VERTEX
}


// 调试函数，显示当前地图状态
void DebugCurrentMapState() {
    OutputDebugStringA("🔍 当前地图状态调试:\n");

    // 检查GROUND_MAP数组
    int groundTileCount = 0;
    for (int z = 0; z < MAPSIZE_Z; z++) {
        for (int x = 0; x < MAPSIZE_X; x++) {
            if (GROUND_MAP[z][x] != 0) {
                groundTileCount++;
            }
        }
    }

    // 检查g_Box数组
    int activeBoxCount = 0;
    for (int i = 0; i < MAPSIZE_X * MAPSIZE_Z; i++) {
        if (g_Box[i].Use) {
            activeBoxCount++;
        }
    }

    char debugMsg[256];
    sprintf_s(debugMsg,
        "🔍 地图数组状态:\n"
        "- GROUND_MAP非零格子: %d/%d\n"
        "- g_Box激活对象: %d/%d\n"
        "- 当前应用模式: %d\n",
        groundTileCount, MAPSIZE_X * MAPSIZE_Z,
        activeBoxCount, MAPSIZE_X * MAPSIZE_Z,
        (int)GetCurrentAppMode()
    );
    OutputDebugStringA(debugMsg);
}