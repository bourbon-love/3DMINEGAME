#pragma once

//Box.h

#include "main.h"
#include "renderer.h"
#include "model.h"
#include "sprite.h"
#include <fstream>
#include <string>
#include "ProceduralModels.h"
#define MAPSIZE_X    (15)
#define MAPSIZE_Z    (15)
#define BATTLE_MAP_SIZE_X    (30)
#define BATTLE_MAP_SIZE_Z    (30)

#define BOXSIZE_X    (1.0f)
#define BOXSIZE_Y    (1.0f)
#define BOXSIZE_Z    (1.0f)
#define BOX_RADIUS    (0.5f)
#define BOX_MAX        (100)        //BOXの数

#define UI_BOX_COUNT 5  // 显示的UI盒子数量
#define OBSTACLE_UI_BOX_COUNT 6  // 显示的障碍物UI盒子数量

class  BoxObject;
struct ObstacleUIBox;


extern BoxObject g_Box[BATTLE_MAP_SIZE_X * BATTLE_MAP_SIZE_Z];
extern BoxObject g_Obstacle[BATTLE_MAP_SIZE_X * BATTLE_MAP_SIZE_Z];
extern int g_HoverX;
extern int g_HoverZ;

// 声明全局变量
extern ObstacleUIBox g_ObstacleUIBoxes[OBSTACLE_UI_BOX_COUNT];
extern int g_SelectedObstacleUIBoxIndex;  // 当前选中的障碍物UI盒子索引

//地图元素类型
enum MAP_ELEMENT_TYPE {
    NONE = 0,        //NONE
    NORMAL_GROUND = 1,
    WALL = 2,
    TREE = 3,
    ICE_GROUND = 4,
    SAND_GROUND = 5,
    GRASS_GROUND = 6,
    WATER = 7,
    START = 8,
    GOAL = 9,
    GRID = 10,
    COIN=11,
    BOMB=12
};

// UI盒子结构体
struct UIBox {
    MAP_ELEMENT_TYPE type;  // 盒子类型
    XMFLOAT3 position;      // 屏幕上的位置
    XMFLOAT3 scale;         // 缩放
    bool selected;          // 是否被选中
    ID3D11ShaderResourceView* TexID;  // 纹理
};

// 障碍物UI盒子结构体
struct ObstacleUIBox {
    MAP_ELEMENT_TYPE type;  // 障碍物类型
    XMFLOAT3 position;      // 屏幕上的位置
    XMFLOAT3 scale;         // 缩放
    bool selected;          // 是否被选中
    ID3D11ShaderResourceView* TexID;  // 纹理
    const wchar_t* description;  // 障碍物描述
};

//地形效果影响
enum TerrainEffect {
    EFFECT_NONE,
    EFFECT_SLIPPERY,    // 冰面效果：可能额外移动一格
    EFFECT_SLOW,        // 沙地效果：下一回合移动力减少
    EFFECT_HEAL,        // 草地效果：回复少量生命
    EFFECT_DAMAGE       // 水面/岩浆效果：受到伤害
};

// 每种地形类型的移动力消耗 (indexed by MAP_ELEMENT_TYPE, 0-12)
extern const int MOVEMENT_COST[13];

// 每种地形的特殊效果 (indexed by MAP_ELEMENT_TYPE, 0-12)
extern const TerrainEffect TERRAIN_EFFECTS[13];

//网格位置结构体
struct GridPosition {
    int x;
    int z;

    //判断两个位置是否相等
    bool operator==(const GridPosition& other) const {
        return x == other.x && z == other.z;
    }
    //判断两个位置是否不等
    bool operator!=(const GridPosition& other) const {
        return x != other.x || z != other.z;
    }
};

class BoxObject
{
public:
    bool        Use;
    int            ObjectNo;
    XMFLOAT3    position;
    XMFLOAT3    scale;
    XMFLOAT3    rotate;
    ID3D11ShaderResourceView* TexID;
    MODEL* model;
    float        Radius;
    XMFLOAT3    SizeMin;
    XMFLOAT3    SizeMax;

    MAP_ELEMENT_TYPE groundType;

    //战旗
    int movementCost; //移动力消耗
    TerrainEffect effect;//地形消耗

    bool useProcedural;  // 标记是否使用程序化模型
    int proceduralType;  // 程序化模型类型（例如 1=金币，2=炸弹）
    //渐变
    bool isFading = false;
    float fadeProgress = 0.0f;    //渐变进度
    float fadeDelay = 0.0f;        //渐变延迟            

    bool isEditMode;
    ID3D11ShaderResourceView* gameTexID; // 游戏模式纹理
    ID3D11ShaderResourceView* editTexID; // 编辑模式纹理
    float fadeAlpha;

    float hoverHeightLerp = 0.0f;  // 当前抬高值（缓动）
    float hoverScaleLerp = 1.0f;   // 当前缩放值（缓动）
};

// 游戏玩家（球体）对象前向声明
class BallObject;

// 核心功能
void    InitBox();
void    UninitBox();
void    UpdateBox();
void    DrawBox();

void CreateGround();
void CreateObstacle();


// 获取对象数组
BoxObject* GetBox();
BoxObject* GetObstacle();

// 碰撞检测
int        GroundCollision();
int        ObstacleCollision();
int        ObstacleGroundCollision();

// 战术游戏相关
void UpdateBoxTactical();
bool IsGridWalkable(int x, int z);
TerrainEffect GetGridEffect(int x, int z);
int GetGridMovementCost(int x, int z);
void ApplyGridEffect(int x, int z);

// 视觉效果
void TriggerBoxFade();
void UpdateBoxFade(float dt);
void DrawBoxOutline(BoxObject& box);

void InitObstacleUIBoxes();
void DrawObstacleUIBoxes();

void SaveMapToFile(const char* filename);
void LoadMapFromFile(const char* filename);
bool CheckBallBoxCollision(BallObject* ball, BoxObject* box);


ID3D11Buffer* GetBoxVertexBuffer();
ID3D11Buffer* GetBoxIndexBuffer();
int GetBoxIndexCount();
//调试函数
void DebugCurrentMapState();
