#include "MapEditor.h"
#include "sprite.h"
#include "Camera.h"
#include <fstream>
#include <string>
#include  "GeometricTextRenderer.h"
#include  "ParticlaEffect.h"
#include  "SceneManager.h"
#include  "ProceduralModels.h"
#include "FrameWork/TextureManager.h"
// 拖动地图相关变量
bool g_IsDraggingMap = false;
int g_LastEditedX = -1;
int g_LastEditedZ = -1;
UIBox g_UIBoxes[UI_BOX_COUNT];
int g_SelectedUIBoxIndex = -1;
ObstacleUIBox g_ObstacleUIBoxes[OBSTACLE_UI_BOX_COUNT];
int g_SelectedObstacleUIBoxIndex = -1;
float g_UIScaleFactor=5.0f;

//// 外部变量引用
//extern int g_HoverX;
//extern int g_HoverZ;
//
//extern ID3D11ShaderResourceView* g_Texture;
//extern ID3D11ShaderResourceView* g_Texture2;
//extern ID3D11ShaderResourceView* g_Texture3;
//extern ID3D11ShaderResourceView* g_TextureIce;
//extern ID3D11ShaderResourceView* g_TextureSand;
//extern ID3D11ShaderResourceView* g_TextureGrass;
//extern ID3D11ShaderResourceView* g_TextureWater;
//extern ID3D11ShaderResourceView* g_TextureWall;
//extern ID3D11ShaderResourceView* g_TextureGrid;
//extern ID3D11ShaderResourceView* g_TextureGround;   // 地面纹理
//extern ID3D11ShaderResourceView* g_TextureGroundUI;   // 地面纹理
//extern ID3D11ShaderResourceView* g_TextureCoin;   // 金币纹理
//extern ID3D11ShaderResourceView* g_TextureBomb;   // 炸弹纹理
//
// 全局模型对象
extern ProceduralModel g_CoinModel;
extern ProceduralModel g_BombModel;

extern MODEL* Tree;
extern MODEL* Bomb;


extern int GROUND_MAP[MAPSIZE_Z][MAPSIZE_X];
extern int OBSTACLE_MAP[MAPSIZE_Z][MAPSIZE_X];

// 外部函数引用
extern void CreateGround();
extern void CreateObstacle();

// 地形编辑模式相关函数
//-------------------------


void DrawUIBoxes()
{
    // 只在编辑模式下显示，且不在障碍物编辑模式
    if (GetCurrentAppMode() != APP_MODE_EDIT || IsEditingObstacle()) return;


    // 保存当前的深度测试和混合状态
    bool oldDepthEnable = GetDepthEnable();
    bool oldBlendEnable = GetBlendState();

    // 禁用深度测试，启用混合
    SetDepthEnable(false);
    SetBlendState(true);

    // 准备渲染2D精灵
    SetWorldViewProjection2D();

    // 盒子尺寸
    const float BOX_SIZE = 50.0f;

    ID3D11ShaderResourceView* textures[UI_BOX_COUNT] = {
      GET_TEXTURE(GROUND),
      GET_TEXTURE(ICE),
      GET_TEXTURE(SAND),
      GET_TEXTURE(GRASS),
      GET_TEXTURE(WATER)
    };
   
    for (int i = 0; i < UI_BOX_COUNT; i++) {
        // 直接在指定位置绘制纹理，不绘制背景
        XMMATRIX world = XMMatrixTranslation(
            g_UIBoxes[i].position.x,
            g_UIBoxes[i].position.y,
            0.0f);
        SetWorldMatrix(world);

        // 创建材质 - 只使用基本的白色材质
        MATERIAL material;
        ZeroMemory(&material, sizeof(MATERIAL));

        // 选中时高亮显示
        if (g_UIBoxes[i].selected) {
            material.Diffuse = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.0f);
        }
        else {
            material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        }

        SetMaterial(material);

        // 确保纹理有效
        if (textures[i] != NULL) {
            // 设置纹理
            GetDeviceContext()->PSSetShaderResources(0, 1, &textures[i]);

            // 绘制纹理
            DrawSprite(XMFLOAT2(BOX_SIZE, BOX_SIZE), material.Diffuse);


        }

    }
        

    // 恢复原有状态
    SetDepthEnable(oldDepthEnable);
    SetBlendState(oldBlendEnable);

}

// 初始化UI盒子的简化版本
void InitUIBoxes()
{
    // 设置不同类型的UI盒子
    MAP_ELEMENT_TYPE boxTypes[UI_BOX_COUNT] = {
        NORMAL_GROUND, ICE_GROUND, SAND_GROUND, GRASS_GROUND, WATER
    };

    OutputDebugStringA("初始化UI盒子 - 简化版...\n");

    // 硬编码位置 - 确保位置合理
    const float yPosition = 50.0f;
    const float positions[UI_BOX_COUNT] = { 200.0f, 300.0f, 400.0f, 500.0f, 600.0f };

    ID3D11ShaderResourceView* textures[UI_BOX_COUNT] = {
      GET_TEXTURE(GROUND),
      GET_TEXTURE(ICE),
      GET_TEXTURE(SAND),
      GET_TEXTURE(GRASS),
      GET_TEXTURE(WATER)
    };

    // 输出纹理指针调试信息
    for (int i = 0; i < UI_BOX_COUNT; i++) {
        char debugMsg[256];
        sprintf_s(debugMsg, "初始化: 纹理[%d] = %p\n", i, textures[i]);
        OutputDebugStringA(debugMsg);
    }

    for (int i = 0; i < UI_BOX_COUNT; i++) {
        g_UIBoxes[i].type = boxTypes[i];
        g_UIBoxes[i].position = XMFLOAT3(positions[i], yPosition, 0.0f);
        g_UIBoxes[i].scale = XMFLOAT3(1.0f, 1.0f, 1.0f); // 使用1.0的缩放
        g_UIBoxes[i].selected = false;
        g_UIBoxes[i].TexID = textures[i];

        char debugMsg[256];
        sprintf_s(debugMsg, "UI盒子 %d: 类型=%d, 位置=(%.1f,%.1f), 纹理=%p\n",
            i, boxTypes[i], positions[i], yPosition, g_UIBoxes[i].TexID);
        OutputDebugStringA(debugMsg);
    }

    OutputDebugStringA("UI盒子初始化完成\n");
}

// 检查是否点击了UI盒子
int PickUIBoxUnderMouse(int mouseX, int mouseY)
{
    if (GetCurrentAppMode() != APP_MODE_EDIT || IsEditingObstacle()) return -1;

    // 获取当前窗口信息
    RECT clientRect;
    GetClientRect(g_hWnd, &clientRect);

    // 计算窗口实际尺寸
    int windowWidth = clientRect.right - clientRect.left;
    int windowHeight = clientRect.bottom - clientRect.top;

    // 转换鼠标坐标到标准化坐标(0-1范围)
    float normalizedX = (float)mouseX / windowWidth;
    float normalizedY = (float)mouseY / windowHeight;

    // 转换到实际渲染尺寸
    float renderX = normalizedX * SCREEN_WIDTH;
    float renderY = normalizedY * SCREEN_HEIGHT;

   
    // 盒子尺寸和点击边距
    const float BOX_SIZE = 50.0f;
    const float CLICK_MARGIN = 20.0f;

    for (int i = 0; i < UI_BOX_COUNT; i++) {
        float minX = g_UIBoxes[i].position.x - (BOX_SIZE / 2 + CLICK_MARGIN);
        float maxX = g_UIBoxes[i].position.x + (BOX_SIZE / 2 + CLICK_MARGIN);
        float minY = g_UIBoxes[i].position.y - (BOX_SIZE / 2 + CLICK_MARGIN);
        float maxY = g_UIBoxes[i].position.y + (BOX_SIZE / 2 + CLICK_MARGIN);

        // 使用转换后的坐标进行检测
        if (renderX >= minX && renderX <= maxX && renderY >= minY && renderY <= maxY) {
            return i;
        }
    }

    return -1;
}

void DrawObstacleUIBoxes()
{
    // 只在编辑模式下且处于障碍物编辑状态时显示
    if (GetCurrentAppMode() != APP_MODE_EDIT || !IsEditingObstacle()) return;


    // 保存当前渲染状态
    bool oldDepthEnable = GetDepthEnable();
    bool oldBlendEnable = GetBlendState();

    // 设置渲染状态
    SetDepthEnable(false);
    SetBlendState(true);

    // 准备渲染2D精灵
    SetWorldViewProjection2D();

    // 盒子尺寸
    const float BOX_SIZE = 50.0f;

    // 显示标题 - 使用DrawMinecraftText
    GeometricTextRenderer::DrawMinecraftText(
        SCREEN_WIDTH / 2 - 100, 10,
        L"Obstacle Edit Mode",
        XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
        1.2f * g_UIScaleFactor,
        false
    );
    // 根据类型设置不同的颜色 - 使用彩色方块代替纹理
    for (int i = 0; i < OBSTACLE_UI_BOX_COUNT; i++) {
        // 设置矩阵
        XMMATRIX world = XMMatrixTranslation(
            g_ObstacleUIBoxes[i].position.x,
            g_ObstacleUIBoxes[i].position.y,
            0.0f);
        SetWorldMatrix(world);

        // 创建材质
        MATERIAL material;
        ZeroMemory(&material, sizeof(MATERIAL));

        // 根据类型设置不同的颜色
        switch (g_ObstacleUIBoxes[i].type) {
        case NONE:
            material.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f); // 灰色
            break;
        case WALL:
            material.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f); // 浅灰色
            break;
        case TREE:
            material.Diffuse = XMFLOAT4(0.0f, 0.6f, 0.0f, 1.0f); // 绿色
            break;
        case START:
            material.Diffuse = XMFLOAT4(0.0f, 0.6f, 1.0f, 1.0f); // 蓝色
            break;
        case COIN:
            material.Diffuse = XMFLOAT4(1.0f, 0.84f, 0.0f, 1.0f); // 金色
            break;
        case BOMB:
            material.Diffuse = XMFLOAT4(0.7f, 0.0f, 0.0f, 1.0f); // 红色
            break;
        default:
            material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
            break;
        }

        // 选中时高亮显示
        if (g_ObstacleUIBoxes[i].selected) {
            material.Diffuse.x = std::min(material.Diffuse.x + 0.2f, 1.0f);
            material.Diffuse.y = std::min(material.Diffuse.y + 0.2f, 1.0f);
            material.Diffuse.z = std::min(material.Diffuse.z + 0.2f, 1.0f);
        }

        SetMaterial(material);

        if (g_ObstacleUIBoxes[i].type == COIN && GET_TEXTURE(COIN) != NULL) {
            ID3D11ShaderResourceView* coinTexture = GET_TEXTURE(COIN);
            GetDeviceContext()->PSSetShaderResources(0, 1, &coinTexture);
        }
        else if (g_ObstacleUIBoxes[i].type == BOMB && GET_TEXTURE(BOMB) != NULL) {
            ID3D11ShaderResourceView* bombTexture = GET_TEXTURE(BOMB);
            GetDeviceContext()->PSSetShaderResources(0, 1, &bombTexture);
        }
        else if (g_ObstacleUIBoxes[i].TexID != NULL) {
            GetDeviceContext()->PSSetShaderResources(0, 1, &g_ObstacleUIBoxes[i].TexID);
        }
        else {
            // 使用基本纹理作为备用
            ID3D11ShaderResourceView* gridTexture = GET_TEXTURE(GRID);
            GetDeviceContext()->PSSetShaderResources(0, 1, &gridTexture);
        }

        // 绘制
        DrawSprite(XMFLOAT2(BOX_SIZE, BOX_SIZE), material.Diffuse);

        // 绘制障碍物名称
        XMFLOAT2 textPos = XMFLOAT2(g_ObstacleUIBoxes[i].position.x - BOX_SIZE / 2,
            g_ObstacleUIBoxes[i].position.y + BOX_SIZE / 2 + 5);


        GeometricTextRenderer::DrawMinecraftText(
            textPos.x, textPos.y,
            g_ObstacleUIBoxes[i].description,
            XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
            0.4f * g_UIScaleFactor,
            false
        );
     }

    // 恢复原有状态
    SetDepthEnable(oldDepthEnable);
    SetBlendState(oldBlendEnable);

}

// 初始化障碍物UI盒子的简化版本
void InitObstacleUIBoxes()
{
    // 设置不同类型的障碍物UI盒子
    MAP_ELEMENT_TYPE boxTypes[OBSTACLE_UI_BOX_COUNT] = {
        NONE,    // 无障碍物（用于清除）
        WALL,    // 墙
        TREE,    // 树
        START,    // 起点
         COIN,    // 金币（新增）
        BOMB     // 炸弹（新增）
    };

    // 障碍物描述
    const wchar_t* descriptions[OBSTACLE_UI_BOX_COUNT] = {
        L"CLEAR",
        L"WALL",
        L"TREE",
        L"START",
        L"COIN",
        L"BOMB"
    };


    // 设置位置 - 放在地形UI盒子下方
    const float yPosition = 120.0f;
  //  const float positions[OBSTACLE_UI_BOX_COUNT] = { 200.0f, 300.0f, 400.0f, 500.0f };
    const float startX = 200.0f;   // 第 0 个格子的中心
    const float spacing = 100.0f;   // 每格水平间隔
  
    // 准备纹理数组
    ID3D11ShaderResourceView* textures[OBSTACLE_UI_BOX_COUNT] = {

        GET_TEXTURE(WALL),
        GET_TEXTURE(GROUND),
        GET_TEXTURE(START),
        GET_TEXTURE(COIN),
        GET_TEXTURE(BOMB),
        GET_TEXTURE(GRID)
    };

    for (int i = 0; i < OBSTACLE_UI_BOX_COUNT; i++) {
        g_ObstacleUIBoxes[i].type = boxTypes[i];
        g_ObstacleUIBoxes[i].position = XMFLOAT3(startX + i * spacing,
            yPosition,
            0.0f);
        g_ObstacleUIBoxes[i].scale = XMFLOAT3(1.0f, 1.0f, 1.0f); // 使用1.0的缩放
        g_ObstacleUIBoxes[i].selected = false;
        g_ObstacleUIBoxes[i].description = descriptions[i];
        g_ObstacleUIBoxes[i].TexID = textures[i];

      
    }

    OutputDebugStringA("障碍物UI盒子初始化完成\n");
}

// 应用障碍物编辑函数更新
// 编辑障碍物
void ApplyObstacleEditToBox(int x, int z, MAP_ELEMENT_TYPE type)
{
    // 检查坐标是否在地图范围内
    if (x < 0 || x >= MAPSIZE_X || z < 0 || z >= MAPSIZE_Z) {
        OutputDebugStringA("错误: 坐标超出地图范围\n");
        return;
    }
    char dbg[128];
    sprintf_s(dbg, "ApplyObstacleEditToBox : (%d,%d) type=%d\n", x, z, type);
    OutputDebugStringA(dbg);


    // 更新障碍物地图数据
    OBSTACLE_MAP[z][x] = (int)type;

    // 查找当前位置是否已有障碍物
    int existingObstacleIndex = -1;
    for (int i = 0; i < MAPSIZE_X * MAPSIZE_Z; i++) {
        if (g_Obstacle[i].Use) {
            // 将物理坐标转换为网格坐标
            int objX = (int)((g_Obstacle[i].position.x - (BOXSIZE_X / 2)) / BOXSIZE_X);
            int objZ = (int)(-(g_Obstacle[i].position.z - (BOXSIZE_Z / 2)) / BOXSIZE_Z);

            if (objX == x && objZ == z) {
                existingObstacleIndex = i;
                break;
            }
        }
    }

    // 如果是NONE类型，移除现有障碍物
    if (type == NONE) {
        if (existingObstacleIndex >= 0) {
            g_Obstacle[existingObstacleIndex].Use = false;
            OutputDebugStringA("移除障碍物\n");
        }
        return;
    }

    // 如果当前位置没有障碍物，寻找一个空闲位置创建新障碍物
    if (existingObstacleIndex < 0) {
        int i = 0;
        while (i < MAPSIZE_X * MAPSIZE_Z && g_Obstacle[i].Use) {
            i++;
        }

        if (i >= MAPSIZE_X * MAPSIZE_Z) {
            OutputDebugStringA("错误: 障碍物数组已满\n");
            return;
        }

        existingObstacleIndex = i;

        // 确保完全重置障碍物的所有属性
        ZeroMemory(&g_Obstacle[existingObstacleIndex], sizeof(BoxObject));
    }

    // 配置障碍物的基本属性
    g_Obstacle[existingObstacleIndex].ObjectNo = (int)type;
    g_Obstacle[existingObstacleIndex].Use = true;
    g_Obstacle[existingObstacleIndex].position.x = x * BOXSIZE_X + (BOXSIZE_X / 2);
    g_Obstacle[existingObstacleIndex].position.y = (BOXSIZE_Y / 2); // 确保高度始终正确
    g_Obstacle[existingObstacleIndex].position.z = -z * BOXSIZE_Z + (BOXSIZE_Z / 2);
    g_Obstacle[existingObstacleIndex].rotate = XMFLOAT3(0.0f, 0.0f, 0.0f);
    g_Obstacle[existingObstacleIndex].scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
    g_Obstacle[existingObstacleIndex].Radius = BOX_RADIUS;

    // 确保碰撞盒大小正确设置
    g_Obstacle[existingObstacleIndex].SizeMin.x = g_Obstacle[existingObstacleIndex].SizeMin.y = g_Obstacle[existingObstacleIndex].SizeMin.z = -BOXSIZE_X / 2;
    g_Obstacle[existingObstacleIndex].SizeMax.x = g_Obstacle[existingObstacleIndex].SizeMax.y = g_Obstacle[existingObstacleIndex].SizeMax.z = BOXSIZE_X / 2;

    // 根据类型设置高度、纹理和程序化模型标志
    g_Obstacle[existingObstacleIndex].model = NULL; // 默认无模型
    g_Obstacle[existingObstacleIndex].useProcedural = false; // 默认不使用程序化模型
    // 根据类型设置高度、纹理和程序化模型标志
    switch (type) {
    case WALL:
        g_Obstacle[existingObstacleIndex].position.y = (BOXSIZE_Y / 2);
        g_Obstacle[existingObstacleIndex].TexID = GET_TEXTURE(WALL);
        break;
    case TREE:
        g_Obstacle[existingObstacleIndex].position.y = 0.5f;
        g_Obstacle[existingObstacleIndex].model = Tree;
        break;
    case START:
        g_Obstacle[existingObstacleIndex].position.y = (BOXSIZE_Y / 2);
        g_Obstacle[existingObstacleIndex].TexID = GET_TEXTURE(START);
        break;
    case COIN:
        g_Obstacle[existingObstacleIndex].position.y = (BOXSIZE_Y / 2+0.025);
        g_Obstacle[existingObstacleIndex].TexID = GET_TEXTURE(COIN); // 仅用于UI
        g_Obstacle[existingObstacleIndex].useProcedural = true;
        g_Obstacle[existingObstacleIndex].proceduralType = 1;
        g_Obstacle[existingObstacleIndex].model = NULL; // 确保模型为NULL
        OutputDebugStringA("放置金币 - 使用程序化模型\n");
        break;
    case BOMB:
        g_Obstacle[existingObstacleIndex].position.y = (BOXSIZE_Y / 2);
        g_Obstacle[existingObstacleIndex].TexID = GET_TEXTURE(BOMB); // 仅用于UI
        g_Obstacle[existingObstacleIndex].useProcedural = true;
        g_Obstacle[existingObstacleIndex].proceduralType = 2;
        g_Obstacle[existingObstacleIndex].model = NULL; // 确保模型为NULL
        OutputDebugStringA("放置炸弹 - 使用程序化模型\n");
        break;
    default:
        g_Obstacle[existingObstacleIndex].position.y = (BOXSIZE_Y / 2);
        g_Obstacle[existingObstacleIndex].TexID = GET_TEXTURE(GROUND);
        break;
    }
    // 输出调试信息
    char debug[256];
    sprintf_s(debug, "放置障碍物: 类型=%d, 位置=(%d,%d), 使用程序化=%d\n",
        type, x, z, g_Obstacle[existingObstacleIndex].useProcedural);
    OutputDebugStringA(debug);

    // 在放置或移除障碍物后添加反馈
    XMFLOAT3 worldPos = XMFLOAT3(
        x * BOXSIZE_X + (BOXSIZE_X / 2),
        BOXSIZE_Y / 2,
        -z * BOXSIZE_Z + (BOXSIZE_Z / 2));

    const char* obstacleNames[] = {
        "Cleared", "Wall Added", "Tree Planted", "Start Point Set","bomb is place","coin is place"
    };

    // 检查类型有效性
    if (type >= 0 && type < 6) {
        ShowFeedbackText(worldPos, obstacleNames[type], XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f));
    }
    // 如果是程序化模型（金币或炸弹）
    if (type == COIN || type == BOMB) {
        // 确保位置和旋转正确
        g_Obstacle[existingObstacleIndex].position.x = x * BOXSIZE_X + (BOXSIZE_X / 2);
        g_Obstacle[existingObstacleIndex].position.y = (BOXSIZE_Y / 2);
        g_Obstacle[existingObstacleIndex].position.z = -z * BOXSIZE_Z + (BOXSIZE_Z / 2);
        g_Obstacle[existingObstacleIndex].rotate = XMFLOAT3(0.0f, 0.0f, 0.0f);
        g_Obstacle[existingObstacleIndex].scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

        // 设置碰撞边界
        g_Obstacle[existingObstacleIndex].SizeMin = XMFLOAT3(-BOXSIZE_X / 2, -BOXSIZE_Y / 2, -BOXSIZE_Z / 2);
        g_Obstacle[existingObstacleIndex].SizeMax = XMFLOAT3(BOXSIZE_X / 2, BOXSIZE_Y / 2, BOXSIZE_Z / 2);
        g_Obstacle[existingObstacleIndex].Radius = BOX_RADIUS;

        // 根据类型设置纹理和程序化模型标记
        if (type == COIN) {
            g_Obstacle[existingObstacleIndex].model = NULL; // 不使用普通模型
            g_Obstacle[existingObstacleIndex].TexID = GET_TEXTURE(COIN); // 仅用于UI
            g_Obstacle[existingObstacleIndex].useProcedural = true;
            g_Obstacle[existingObstacleIndex].proceduralType = 1; // 1表示金币
        }
        else { // BOMB
            g_Obstacle[existingObstacleIndex].model = NULL; // 不使用普通模型
            g_Obstacle[existingObstacleIndex].TexID = GET_TEXTURE(BOMB); // 仅用于UI
            g_Obstacle[existingObstacleIndex].useProcedural = true;
            g_Obstacle[existingObstacleIndex].proceduralType = 2; // 2表示炸弹
        }

        // 确保模型指针有效
        if (!g_Obstacle[existingObstacleIndex].model) {
            OutputDebugStringA("警告：程序化模型为空！\n");
            // 使用备选方案 - 如果模型为空，使用纹理
            g_Obstacle[existingObstacleIndex].model = NULL;
        }
    }

  
}

// 检测是否点击了障碍物UI盒子
int PickObstacleUIBoxUnderMouse(int mouseX, int mouseY)
{
    if (GetCurrentAppMode() != APP_MODE_EDIT || !IsEditingObstacle()) return -1;

    // 获取当前窗口信息
    RECT clientRect;
    GetClientRect(g_hWnd, &clientRect);

    // 计算窗口实际尺寸
    int windowWidth = clientRect.right - clientRect.left;
    int windowHeight = clientRect.bottom - clientRect.top;

    // 转换鼠标坐标到标准化坐标(0-1范围)
    float normalizedX = (float)mouseX / windowWidth;
    float normalizedY = (float)mouseY / windowHeight;

    // 转换到实际渲染尺寸
    float renderX = normalizedX * SCREEN_WIDTH;
    float renderY = normalizedY * SCREEN_HEIGHT;

    char debugMsg[256];
    sprintf_s(debugMsg, "原始鼠标: (%d,%d), 转换后: (%.1f,%.1f)\n",
        mouseX, mouseY, renderX, renderY);
    OutputDebugStringA(debugMsg);

    // 盒子尺寸和点击边距
    const float BOX_SIZE = 50.0f;
    const float CLICK_MARGIN = 20.0f;

    for (int i = 0; i < OBSTACLE_UI_BOX_COUNT; i++) {
        float minX = g_ObstacleUIBoxes[i].position.x - (BOX_SIZE / 2 + CLICK_MARGIN);
        float maxX = g_ObstacleUIBoxes[i].position.x + (BOX_SIZE / 2 + CLICK_MARGIN);
        float minY = g_ObstacleUIBoxes[i].position.y - (BOX_SIZE / 2 + CLICK_MARGIN);
        float maxY = g_ObstacleUIBoxes[i].position.y + (BOX_SIZE / 2 + CLICK_MARGIN);

        // 使用转换后的坐标进行检测
        if (renderX >= minX && renderX <= maxX && renderY >= minY && renderY <= maxY) {
            OutputDebugStringA("命中障碍物UI盒子!\n");
            return i;
        }
    }

    return -1;
}

// 编辑地形
void ApplyEditToBox(int x, int z, MAP_ELEMENT_TYPE type)
{
    // 检查坐标是否在地图范围内
    if (x < 0 || x >= MAPSIZE_X || z < 0 || z >= MAPSIZE_Z) {
        OutputDebugStringA("错误: 坐标超出地图范围\n");
        return;
    }

    int i = z * MAPSIZE_X + x;

    if (!g_Box[i].Use) {
        OutputDebugStringA("错误: 该位置没有盒子对象\n");
        return;
    }

    // 保存原始类型用于调试
    MAP_ELEMENT_TYPE oldType = g_Box[i].groundType;
    ID3D11ShaderResourceView* oldTexID = g_Box[i].TexID; // 保存旧纹理指针用于调试

    // 更改盒子类型
    g_Box[i].groundType = type;

    // 更新纹理 (将在退出编辑模式时应用)
    ID3D11ShaderResourceView* newTexID = NULL;
    switch (type)
    {
    case NORMAL_GROUND:
        newTexID = GET_TEXTURE(GROUND);
        break;
    case ICE_GROUND:
        newTexID = GET_TEXTURE(ICE);
        break;
    case SAND_GROUND:
        newTexID = GET_TEXTURE(SAND);
        break;
    case GRASS_GROUND:
        // 确保草地纹理指针有效
        if (GET_TEXTURE(GRASS) != NULL) {
            newTexID = GET_TEXTURE(GRASS);
        }
        else {
            OutputDebugStringA("警告: 草地纹理指针为NULL!\n");
            newTexID = GET_TEXTURE(GROUND); // 使用默认纹理作为回退
        }
        break;
    case WATER:
        newTexID = GET_TEXTURE(WATER);
        break;
    default:
        newTexID = GET_TEXTURE(GROUND);
        break;
    }

    // 更新纹理
    g_Box[i].TexID = newTexID;

    // 强制在编辑模式下显示实际纹理而不是网格纹理
    // 这是关键修改点 - 确保盒子在编辑时显示正确的纹理
    g_Box[i].isFading = true;
    g_Box[i].fadeProgress = 1.0f; // 设为最大值确保完全显示

    // 更新GROUND_MAP数组
    GROUND_MAP[z][x] = (int)type;

    // 更新其他属性
    g_Box[i].movementCost = MOVEMENT_COST[g_Box[i].groundType];
    g_Box[i].effect = TERRAIN_EFFECTS[g_Box[i].groundType];

    // 添加跳动效果
    g_Box[i].hoverHeightLerp = 0.2f;
    g_Box[i].hoverScaleLerp = 1.2f;

    // 在更改地形后添加反馈
    XMFLOAT3 worldPos = XMFLOAT3(
        x * BOXSIZE_X + (BOXSIZE_X / 2),
        BOXSIZE_Y / 2,
        -z * BOXSIZE_Z + (BOXSIZE_Z / 2));

    const char* terrainNames[] = {
        "Normal Ground", "Ice", "Sand", "Grass", "Water"
    };

    // 检查类型有效性
    if (type >= 0 && type < 5) {
        ShowFeedbackText(worldPos, terrainNames[type], XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f));
    }
}


// 初始化地图编辑器
void InitMapEditor()
{
    // 初始化地形UI
    InitUIBoxes();

    // 初始化障碍物UI
    InitObstacleUIBoxes();

    // 默认为地形编辑模式
    IsEditingObstacle();
    g_SelectedObstacleUIBoxIndex = -1;
    g_SelectedUIBoxIndex = -1;
}

// 清理地图编辑器
void UninitMapEditor()
{
    // 重置UI状态
    g_SelectedUIBoxIndex = -1;
    g_SelectedObstacleUIBoxIndex = -1;
    
}

// 更新地图编辑器
void UpdateMapEditor(float dt)
{
    // 目前没有特定的更新逻辑
    // 可以在这里添加UI动画或其他效果
}

// 绘制编辑器帮助信息
void DrawEditorHelp()
{
    if (GetCurrentAppMode() != APP_MODE_EDIT) return;
    // 保存当前矩阵状态
    XMMATRIX savedWorldMatrix = g_WorldMatrix;
    XMMATRIX savedViewMatrix = g_ViewMatrix;
    XMMATRIX savedProjMatrix = g_ProjectionMatrix;

    // 保存当前渲染状态
    bool savedDepthEnable = GetDepthEnable();
    bool savedBlendEnable = GetBlendState();

    // 设置渲染状态
    SetDepthEnable(false);
    SetBlendState(true);


     
        //// 使用像素风格文本显示帮助信息
        //GeometricTextRenderer::DrawEnhancedPixelText(10, 70, L"EDITING OPERATIONS", XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f), 1.5f*4.0f);
        //GeometricTextRenderer::DrawTitlePixelText(10, 100, L"CLICK TERRAIN BLOCK TO PAINT", XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f), 1.0f * 4.0f);
        //GeometricTextRenderer::DrawRainbowPixelText(10, 130, L"CTRL+S TO SAVE, CTRL+L TO LOAD", 1.0f * 4.0f);

        //if (IsEditingObstacle()) {
        //    GeometricTextRenderer::DrawFlickeringPixelText(10, 160, L"OBSTACLE EDIT MODE", XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f), 1.0f * 4.0f);
        //}
        //else {
        //    GeometricTextRenderer::DrawFlickeringPixelText(10, 160, L"TERRAIN EDIT MODE", XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), 1.0f * 4.0f);
        //}

             // 使用DrawMinecraftText替代其他文本渲染方法
        GeometricTextRenderer::DrawMinecraftText(10, 70, L"EDITING OPERATIONS",
            XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f), 1.5f * 4.0f, false);

        GeometricTextRenderer::DrawMinecraftText(10, 100, L"CLICK TERRAIN BLOCK TO PAINT",
            XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f), 1.0f * 4.0f, false);

        GeometricTextRenderer::DrawMinecraftText(10, 130, L"CTRL+S TO SAVE, CTRL+L TO LOAD",
            XMFLOAT4(1.0f, 0.8f, 0.2f, 1.0f), 1.0f * 4.0f, false);

        if (IsEditingObstacle()) {
            GeometricTextRenderer::DrawMinecraftText(10, 160, L"OBSTACLE EDIT MODE",
                XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f), 1.0f * 4.0f, false);
        }
        else {
            GeometricTextRenderer::DrawMinecraftText(10, 160, L"TERRAIN EDIT MODE",
                XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), 1.0f * 4.0f, false);
        }

    // 恢复原始矩阵状态
    g_WorldMatrix = savedWorldMatrix;
    g_ViewMatrix = savedViewMatrix;
    g_ProjectionMatrix = savedProjMatrix;

    // 更新常量缓冲区
    XMMATRIX worldViewProj = g_WorldMatrix * g_ViewMatrix * g_ProjectionMatrix;
    worldViewProj = XMMatrixTranspose(worldViewProj);
    XMFLOAT4X4 matrix;
    XMStoreFloat4x4(&matrix, worldViewProj);

    // 恢复渲染状态
    SetDepthEnable(savedDepthEnable);
    SetBlendState(savedBlendEnable);
}

// 保存地图配置到文件
void SaveMapToFile(const char* filename)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        char errMsg[256];
        sprintf_s(errMsg, "无法打开文件进行保存: %s\n", filename);
        OutputDebugStringA(errMsg);
        return;
    }

    // 保存地形数据
    for (int z = 0; z < MAPSIZE_Z; z++) {
        for (int x = 0; x < MAPSIZE_X; x++) {
            file.write(reinterpret_cast<const char*>(&GROUND_MAP[z][x]), sizeof(int));
        }
    }

    // 保存障碍物数据
    for (int z = 0; z < MAPSIZE_Z; z++) {
        for (int x = 0; x < MAPSIZE_X; x++) {
            file.write(reinterpret_cast<const char*>(&OBSTACLE_MAP[z][x]), sizeof(int));
        }
    }

    file.close();

    char msg[256];
    sprintf_s(msg, "地图已保存到文件: %s\n", filename);
    OutputDebugStringA(msg);
}

// 从文件加载地图配置
void LoadMapFromFile(const char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        char errMsg[256];
        sprintf_s(errMsg, "无法打开文件进行加载: %s\n", filename);
        OutputDebugStringA(errMsg);
        return;
    }

    // 清除现有对象
    for (int i = 0; i < MAPSIZE_X * MAPSIZE_Z; i++) {
        g_Box[i].Use = false;
        g_Obstacle[i].Use = false;
    }

    // 加载地形数据
    for (int z = 0; z < MAPSIZE_Z; z++) {
        for (int x = 0; x < MAPSIZE_X; x++) {
            file.read(reinterpret_cast<char*>(&GROUND_MAP[z][x]), sizeof(int));
        }
    }

    // 加载障碍物数据
    for (int z = 0; z < MAPSIZE_Z; z++) {
        for (int x = 0; x < MAPSIZE_X; x++) {
            file.read(reinterpret_cast<char*>(&OBSTACLE_MAP[z][x]), sizeof(int));
        }
    }

    file.close();

    // 重新创建地形和障碍物对象
    CreateGround();
    CreateObstacle();

    char msg[256];
    sprintf_s(msg, "地图已从文件加载: %s\n", filename);
    OutputDebugStringA(msg);
}

// 访问器函数，用于在Camera.cpp等地方访问UI盒子
UIBox* GetUIBoxes() {
    return g_UIBoxes;
}

// 访问选中的UI盒子索引
int* GetSelectedUIBoxIndex() {
    return &g_SelectedUIBoxIndex;
}

// 访问障碍物UI盒子
ObstacleUIBox* GetObstacleUIBoxes() {
    return g_ObstacleUIBoxes;
}

// 访问选中的障碍物UI盒子索引
int* GetSelectedObstacleUIBoxIndex() {
    return &g_SelectedObstacleUIBoxIndex;
}

// 判断当前是否处于障碍物编辑模式
void ShowFeedbackText(XMFLOAT3 worldPosition, const char* message, XMFLOAT4 color) {
    // 调整位置，确保文本显示在地形上方
    worldPosition.y += 1.0f;

    // 使用 FloatingTextSystem 显示文本
    FloatingTextSystem::ShowText(worldPosition, message, color);

    char debug[256];
    sprintf_s(debug, "显示反馈文本: %s 在位置(%.2f, %.2f, %.2f)\n",
        message, worldPosition.x, worldPosition.y, worldPosition.z);
    OutputDebugStringA(debug);
}
void CreateBattleMap()
{
    // 复用现有的CreateGround()和CreateObstacle()逻辑
    CreateGround();
    CreateObstacle();
    OutputDebugStringA("✅ 多人战斗地图创建完成\n");
}


// 修改这些函数以接收鼠标状态作为参数
void HandleTerrainEditModeClick(int mouseX, int mouseY)
{
    // 先检查是否点击了UI盒子
    int pickedUIBox = PickUIBoxUnderMouse(mouseX, mouseY);

    if (pickedUIBox >= 0) {
        // 输出选中信息
        OutputDebugStringA("选中了UI盒子!\n");

        // 取消之前选中的盒子
        UIBox* uiBoxes = GetUIBoxes();
        int* selectedUIBoxIndex = GetSelectedUIBoxIndex();
        if (*selectedUIBoxIndex >= 0) {
            uiBoxes[*selectedUIBoxIndex].selected = false;
        }

        // 更新选中索引和盒子状态
        *selectedUIBoxIndex = pickedUIBox;
        uiBoxes[pickedUIBox].selected = true;

        char msg[128];
        sprintf_s(msg, "选中的UI盒子类型: %d\n", uiBoxes[pickedUIBox].type);
        OutputDebugStringA(msg);

        // 不启动拖动操作，因为我们是在UI盒子上点击的
        g_IsDraggingMap = false;
    }
    else {
        // 在检测到点击地图盒子后执行的代码
        int pickedX, pickedZ;
        if (PickBoxUnderMouse(&pickedX, &pickedZ)) {
            // 如果有UI盒子被选中，改变点击的地图盒子类型
            int* selectedUIBoxIndex = GetSelectedUIBoxIndex();
            if (*selectedUIBoxIndex >= 0) {
                // 获取UI盒子
                UIBox* uiBoxes = GetUIBoxes();

                // 获取选中的地形类型
                MAP_ELEMENT_TYPE newType = uiBoxes[*selectedUIBoxIndex].type;

                // 应用编辑
                ApplyEditToBox(pickedX, pickedZ, newType);

                // 记录最后编辑的位置并开始拖动操作
                g_LastEditedX = pickedX;
                g_LastEditedZ = pickedZ;
                g_IsDraggingMap = true;
            }
            else {
                OutputDebugStringA("警告: 未选中UI盒子，无法修改地图\n");
                g_IsDraggingMap = false;
            }
        }
        else {
            g_IsDraggingMap = false;
        }
    }
}

void HandleObstacleEditModeClick(int mouseX, int mouseY)
{
    // 检查是否点击了障碍物UI盒子
    int pickedObstacleUIBox = PickObstacleUIBoxUnderMouse(mouseX, mouseY);

    if (pickedObstacleUIBox >= 0) {
        // 选中了障碍物UI盒子
        OutputDebugStringA("选中了障碍物UI盒子!\n");

        // 取消之前选中的盒子
        int* selectedObstacleUIBoxIndex = GetSelectedObstacleUIBoxIndex();
        ObstacleUIBox* obstacleUIBoxes = GetObstacleUIBoxes();
        if (*selectedObstacleUIBoxIndex >= 0) {
            obstacleUIBoxes[*selectedObstacleUIBoxIndex].selected = false;
        }

        // 更新选中索引和盒子状态
        *selectedObstacleUIBoxIndex = pickedObstacleUIBox;
        obstacleUIBoxes[pickedObstacleUIBox].selected = true;

        char msg[128];
        sprintf_s(msg, "选中的障碍物类型: %d\n", obstacleUIBoxes[pickedObstacleUIBox].type);
        OutputDebugStringA(msg);

        // 不启动拖动操作
        g_IsDraggingMap = false;
    }
    else {
        // 检测点击地图格子
        int pickedX, pickedZ;
        if (PickBoxUnderMouse(&pickedX, &pickedZ)) {
            // 如果有障碍物UI盒子被选中，改变点击的地图格子
            int* selectedObstacleUIBoxIndex = GetSelectedObstacleUIBoxIndex();
            ObstacleUIBox* obstacleUIBoxes = GetObstacleUIBoxes();
            if (*selectedObstacleUIBoxIndex >= 0) {
                // 获取选中的障碍物类型
                MAP_ELEMENT_TYPE newType = obstacleUIBoxes[*selectedObstacleUIBoxIndex].type;

                // 应用障碍物编辑
                ApplyObstacleEditToBox(pickedX, pickedZ, newType);

                // 记录最后编辑的位置并开始拖动操作
                g_LastEditedX = pickedX;
                g_LastEditedZ = pickedZ;
                g_IsDraggingMap = true;
            }
            else {
                OutputDebugStringA("警告: 未选中障碍物UI盒子，无法修改地图\n");
                g_IsDraggingMap = false;
            }
        }
        else {
            g_IsDraggingMap = false;
        }
    }
}
void HandleMapDragging()
{
    // 鼠标左键持续按下并处于拖动状态
    int currentX, currentZ;
    if (PickBoxUnderMouse(&currentX, &currentZ)) {
        // 如果当前位置与上次编辑的位置不同
        if (currentX != g_LastEditedX || currentZ != g_LastEditedZ) {
            if (IsEditingObstacle()) {
                // 障碍物编辑模式
                int* selectedObstacleUIBoxIndex = GetSelectedObstacleUIBoxIndex();
                ObstacleUIBox* obstacleUIBoxes = GetObstacleUIBoxes();
                if (*selectedObstacleUIBoxIndex >= 0) {
                    MAP_ELEMENT_TYPE newType = obstacleUIBoxes[*selectedObstacleUIBoxIndex].type;

                    // 应用障碍物编辑
                    ApplyObstacleEditToBox(currentX, currentZ, newType);
                }
            }
            else {
                // 地形编辑模式
                int* selectedUIBoxIndex = GetSelectedUIBoxIndex();
                if (*selectedUIBoxIndex >= 0) {
                    UIBox* uiBoxes = GetUIBoxes();
                    MAP_ELEMENT_TYPE newType = uiBoxes[*selectedUIBoxIndex].type;

                    // 应用地形编辑
                    ApplyEditToBox(currentX, currentZ, newType);
                }
            }

            // 更新最后编辑的位置
            g_LastEditedX = currentX;
            g_LastEditedZ = currentZ;
        }
    }
}

bool PickBoxUnderMouse(int* outX, int* outZ)
{
    /*  if (g_CameraMode != CAMERA_MODE_EDIT)
          return false;*/

    Camera* g_camera = GetCamera();

    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(g_hWnd, &pt);

    RECT clientRect;
    GetClientRect(g_hWnd, &clientRect);
    float screenWidth = (float)(clientRect.right - clientRect.left);
    float screenHeight = (float)(clientRect.bottom - clientRect.top);

    if (pt.x < 0 || pt.y < 0 || pt.x >= screenWidth || pt.y >= screenHeight)
        return false;

    // Step 1: 计算鼠标世界空间射线
    float ndcX = (2.0f * pt.x) / screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * pt.y) / screenHeight;

    XMVECTOR nearPoint = XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
    XMVECTOR farPoint = XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);
    XMMATRIX viewProj = XMMatrixMultiply(g_camera->ViewMatrix, g_camera->ProjectionMatrix);
    XMMATRIX invViewProj = XMMatrixInverse(nullptr, viewProj);

    XMVECTOR rayOrigin = XMVector3TransformCoord(nearPoint, invViewProj);
    XMVECTOR rayTarget = XMVector3TransformCoord(farPoint, invViewProj);
    XMVECTOR rayDir = XMVector3Normalize(rayTarget - rayOrigin);

    // Step 2: 遍历所有 Box，找最近命中的 Box
    BoxObject* boxes = GetBox();
    float closestDist = FLT_MAX;
    int pickedIndex = -1;

    for (int i = 0; i < MAPSIZE_X * MAPSIZE_Z; ++i)
    {
        if (!boxes[i].Use)
            continue;

        XMFLOAT3 min = {
            boxes[i].position.x - BOX_RADIUS,
            boxes[i].position.y - BOX_RADIUS,
            boxes[i].position.z - BOX_RADIUS,
        };
        XMFLOAT3 max = {
            boxes[i].position.x + BOX_RADIUS,
            boxes[i].position.y + BOX_RADIUS,
            boxes[i].position.z + BOX_RADIUS,
        };

        float dist = 0.0f;
        if (RayIntersectsAABB(rayOrigin, rayDir, XMLoadFloat3(&min), XMLoadFloat3(&max), &dist)) {
            if (dist < closestDist) {
                closestDist = dist;
                pickedIndex = i;
            }
        }
    }

    // Step 3: 返回命中的 Box 的网格坐标
    if (pickedIndex >= 0)
    {
        *outX = pickedIndex % MAPSIZE_X;
        *outZ = pickedIndex / MAPSIZE_X;

        // DrawBoxOutline(boxes[pickedIndex]);  // 高亮画框


        return true;
    }

    return false;
}


// ======================== 射线检测 Box ========================
bool RayIntersectsAABB(
    XMVECTOR rayOrigin,
    XMVECTOR rayDir,
    XMVECTOR boxMin,
    XMVECTOR boxMax,
    float* outDistance)
{
    float tMin = 0.0f;
    float tMax = FLT_MAX;

    for (int i = 0; i < 3; ++i) {
        float rayO = XMVectorGetByIndex(rayOrigin, i);
        float rayD = XMVectorGetByIndex(rayDir, i);
        float bMin = XMVectorGetByIndex(boxMin, i);
        float bMax = XMVectorGetByIndex(boxMax, i);

        if (fabs(rayD) < 1e-6f) {
            // 射线平行于 slab，如果起点不在 slab 内则不相交
            if (rayO < bMin || rayO > bMax)
                return false;
        }
        else {
            float ood = 1.0f / rayD;
            float t1 = (bMin - rayO) * ood;
            float t2 = (bMax - rayO) * ood;
            if (t1 > t2) std::swap(t1, t2);
            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);
            if (tMin > tMax)
                return false;
        }
    }

    if (outDistance) *outDistance = tMin;
    return true;
}


// 新增：包装SaveMapToFile函数，用于UI回调
bool SaveCurrentMap() {
    try {
        // 使用一个固定的文件名或让用户选择一个文件
        const char* filename = "maps/default_map.dat";

        // 创建maps目录（如果不存在）
        CreateDirectory("maps", NULL);

        // 调用原始保存函数
        SaveMapToFile(filename);

        char msg[256];
        sprintf_s(msg, "地图已成功保存到: %s\n", filename);
        OutputDebugStringA(msg);

        return true;
    }
    catch (...) {
        OutputDebugStringA("保存地图时发生错误\n");
        return false;
    }
}

// 新增：包装LoadMapFromFile函数，用于UI回调
bool LoadCurrentMap() {
    try {
        // 使用一个固定的文件名或让用户选择一个文件
        const char* filename = "maps/default_map.dat";

        // 检查文件是否存在
        FILE* file;
        errno_t err = fopen_s(&file, filename, "rb");
        if (err != 0 || file == NULL) {
            char errMsg[256];
            sprintf_s(errMsg, "无法打开文件: %s\n", filename);
            OutputDebugStringA(errMsg);
            return false;
        }
        fclose(file);

        // 调用原始加载函数
        LoadMapFromFile(filename);

        char msg[256];
        sprintf_s(msg, "地图已成功加载自: %s\n", filename);
        OutputDebugStringA(msg);

        return true;
    }
    catch (...) {
        OutputDebugStringA("加载地图时发生错误\n");
        return false;
    }
}

bool* GetIsDraggingMap() {
    return &g_IsDraggingMap;
}

int* GetLastEditedX() {
    return &g_LastEditedX;
}

int* GetLastEditedZ() {
    return &g_LastEditedZ;
}