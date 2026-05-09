// PixelCharacters.cpp - 第1部分（头文件、全局变量和初始化/清理函数）
#include "PixelCharacters.h"

// 全局角色对象
PixelCharacter g_Doctor;     // 医生
PixelCharacter g_Soldier;    // 士兵
PixelCharacter g_Scout;      // 侦察兵
PixelCharacter g_BombTech;   // 拆弹手

// 纹理资源
extern ID3D11ShaderResourceView* g_TextureWhite; // 白色纹理

// 动画参数 
float g_CharacterBobHeights[CHARACTER_TYPE_COUNT] = { 0.0f, 0.0f, 0.0f, 0.0f };
float g_CharacterArmSwings[CHARACTER_TYPE_COUNT] = { 0.0f, 0.0f, 0.0f, 0.0f };
static float g_AnimationTime = 0.0f; // 动画计时器
static bool g_PixelCharactersInitialized = false;

// 方块尺寸常量
const float BLOCK_SIZE = 0.035f;  // 减小基础方块大小，增加细节度

// 🎮 新的真实比例常量 - 添加到文件顶部
const int HEAD_WIDTH = 6;        // 头部宽度
const int HEAD_HEIGHT = 7;       // 头部高度  
const int HEAD_DEPTH = 6;        // 头部深度
const int NECK_WIDTH = 3;        // 颈部宽度
const int NECK_HEIGHT = 2;       // 颈部高度

// 身体比例 - 更真实的躯干
const int CHEST_WIDTH = 12;       // 胸部宽度
const int CHEST_HEIGHT = 10;      // 胸部高度
const int CHEST_DEPTH = 4;       // 胸部深度
const int WAIST_WIDTH = 10;       // 腰部宽度
const int WAIST_HEIGHT = 4;      // 腰部高度

// 手臂比例 - 分段更明显
const int SHOULDER_SIZE = 3;     // 肩膀大小
const int UPPER_ARM_WIDTH = 3;   // 上臂宽度
const int UPPER_ARM_LENGTH = 6;  // 上臂长度
const int FOREARM_WIDTH = 2;     // 前臂宽度（更细）
const int FOREARM_LENGTH = 6;    // 前臂长度
const int HAND_WIDTH = 3;        // 手宽度
const int HAND_HEIGHT = 2;       // 手高度
const int HAND_DEPTH = 4;        // 手深度

// 腿部比例 - 更真实的腿型
const int HIP_WIDTH = 7;         // 髋部宽度
const int THIGH_WIDTH = 4;       // 大腿宽度
const int THIGH_LENGTH = 8;      // 大腿长度
const int KNEE_SIZE = 3;         // 膝盖大小
const int SHIN_WIDTH = 3;        // 小腿宽度（更细）
const int SHIN_LENGTH = 8;       // 小腿长度
const int FOOT_WIDTH = 3;        // 脚宽度
const int FOOT_HEIGHT = 2;       // 脚高度
const int FOOT_LENGTH = 5;       // 脚长度

// 🎮 绘制所有像素角色 - 兼容性函数
void DrawAllPixelCharacters(XMMATRIX* worldMatrices)
{
    if (!g_PixelCharactersInitialized) return;

    DrawPixelCharacter(&g_Doctor, worldMatrices[CHARACTER_DOCTOR]);
    DrawPixelCharacter(&g_Soldier, worldMatrices[CHARACTER_SOLDIER]);
    DrawPixelCharacter(&g_Scout, worldMatrices[CHARACTER_SCOUT]);
    DrawPixelCharacter(&g_BombTech, worldMatrices[CHARACTER_BOMB_TECH]);
}

// 🎮 增强的像素方块添加函数 - 支持更精细的控制
void AddDetailedPixelBlock(VERTEX_3D* vertices, unsigned int* indices,
    int& vertexIndex, int& indexIndex,
    float x, float y, float z,
    float width, float height, float depth,
    XMFLOAT4 color, XMFLOAT3 normal = XMFLOAT3(0.0f, 1.0f, 0.0f))
{
    // 增强的顶点添加，包含更好的法线和纹理坐标
    int startVertex = vertexIndex;

    // 前面 (+Z)
    vertices[vertexIndex++] = { XMFLOAT3(x, y, z + depth), XMFLOAT3(0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 1.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x + width, y, z + depth), XMFLOAT3(0.0f, 0.0f, 1.0f), color, XMFLOAT2(1.0f, 1.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x, y + height, z + depth), XMFLOAT3(0.0f, 0.0f, 1.0f), color, XMFLOAT2(0.0f, 0.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x + width, y + height, z + depth), XMFLOAT3(0.0f, 0.0f, 1.0f), color, XMFLOAT2(1.0f, 0.0f) };

    // 后面 (-Z)
    vertices[vertexIndex++] = { XMFLOAT3(x + width, y, z), XMFLOAT3(0.0f, 0.0f, -1.0f), color, XMFLOAT2(0.0f, 1.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x, y, z), XMFLOAT3(0.0f, 0.0f, -1.0f), color, XMFLOAT2(1.0f, 1.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x + width, y + height, z), XMFLOAT3(0.0f, 0.0f, -1.0f), color, XMFLOAT2(0.0f, 0.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x, y + height, z), XMFLOAT3(0.0f, 0.0f, -1.0f), color, XMFLOAT2(1.0f, 0.0f) };

    // 顶面 (+Y)
    vertices[vertexIndex++] = { XMFLOAT3(x, y + height, z), XMFLOAT3(0.0f, 1.0f, 0.0f), color, XMFLOAT2(0.0f, 0.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x + width, y + height, z), XMFLOAT3(0.0f, 1.0f, 0.0f), color, XMFLOAT2(1.0f, 0.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x, y + height, z + depth), XMFLOAT3(0.0f, 1.0f, 0.0f), color, XMFLOAT2(0.0f, 1.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x + width, y + height, z + depth), XMFLOAT3(0.0f, 1.0f, 0.0f), color, XMFLOAT2(1.0f, 1.0f) };

    // 底面 (-Y)
    vertices[vertexIndex++] = { XMFLOAT3(x, y, z + depth), XMFLOAT3(0.0f, -1.0f, 0.0f), color, XMFLOAT2(0.0f, 0.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x + width, y, z + depth), XMFLOAT3(0.0f, -1.0f, 0.0f), color, XMFLOAT2(1.0f, 0.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x, y, z), XMFLOAT3(0.0f, -1.0f, 0.0f), color, XMFLOAT2(0.0f, 1.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x + width, y, z), XMFLOAT3(0.0f, -1.0f, 0.0f), color, XMFLOAT2(1.0f, 1.0f) };

    // 左面 (-X)
    vertices[vertexIndex++] = { XMFLOAT3(x, y, z), XMFLOAT3(-1.0f, 0.0f, 0.0f), color, XMFLOAT2(0.0f, 1.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x, y, z + depth), XMFLOAT3(-1.0f, 0.0f, 0.0f), color, XMFLOAT2(1.0f, 1.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x, y + height, z), XMFLOAT3(-1.0f, 0.0f, 0.0f), color, XMFLOAT2(0.0f, 0.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x, y + height, z + depth), XMFLOAT3(-1.0f, 0.0f, 0.0f), color, XMFLOAT2(1.0f, 0.0f) };

    // 右面 (+X)
    vertices[vertexIndex++] = { XMFLOAT3(x + width, y, z + depth), XMFLOAT3(1.0f, 0.0f, 0.0f), color, XMFLOAT2(0.0f, 1.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x + width, y, z), XMFLOAT3(1.0f, 0.0f, 0.0f), color, XMFLOAT2(1.0f, 1.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x + width, y + height, z + depth), XMFLOAT3(1.0f, 0.0f, 0.0f), color, XMFLOAT2(0.0f, 0.0f) };
    vertices[vertexIndex++] = { XMFLOAT3(x + width, y + height, z), XMFLOAT3(1.0f, 0.0f, 0.0f), color, XMFLOAT2(1.0f, 0.0f) };

    // 添加索引（36个索引，6个面 × 2个三角形 × 3个顶点）
    int v = startVertex;

    // 前面
    indices[indexIndex++] = v + 0; indices[indexIndex++] = v + 1; indices[indexIndex++] = v + 2;
    indices[indexIndex++] = v + 2; indices[indexIndex++] = v + 1; indices[indexIndex++] = v + 3;

    // 后面
    indices[indexIndex++] = v + 4; indices[indexIndex++] = v + 5; indices[indexIndex++] = v + 6;
    indices[indexIndex++] = v + 6; indices[indexIndex++] = v + 5; indices[indexIndex++] = v + 7;

    // 顶面
    indices[indexIndex++] = v + 8; indices[indexIndex++] = v + 9; indices[indexIndex++] = v + 10;
    indices[indexIndex++] = v + 10; indices[indexIndex++] = v + 9; indices[indexIndex++] = v + 11;

    // 底面
    indices[indexIndex++] = v + 12; indices[indexIndex++] = v + 13; indices[indexIndex++] = v + 14;
    indices[indexIndex++] = v + 14; indices[indexIndex++] = v + 13; indices[indexIndex++] = v + 15;

    // 左面
    indices[indexIndex++] = v + 16; indices[indexIndex++] = v + 17; indices[indexIndex++] = v + 18;
    indices[indexIndex++] = v + 18; indices[indexIndex++] = v + 17; indices[indexIndex++] = v + 19;

    // 右面
    indices[indexIndex++] = v + 20; indices[indexIndex++] = v + 21; indices[indexIndex++] = v + 22;
    indices[indexIndex++] = v + 22; indices[indexIndex++] = v + 21; indices[indexIndex++] = v + 23;
}

// PixelCharacters.cpp - Part 2: 详细身体部位创建函数
// 这部分包含创建头部、躯干、手臂、腿部的详细函数

// 🔧 修改CreateRealisticHead函数 - 改善连接
void CreateRealisticHead(VERTEX_3D* vertices, unsigned int* indices,
    int& vertexIndex, int& indexIndex,
    float centerX, float centerY, float centerZ,
    XMFLOAT4 skinColor, XMFLOAT4 hairColor, XMFLOAT4 eyeColor, CHARACTER_TYPE type)
{
    float headW = BLOCK_SIZE * HEAD_WIDTH;
    float headH = BLOCK_SIZE * HEAD_HEIGHT;
    float headD = BLOCK_SIZE * HEAD_DEPTH;

    // 1. 主头部 - 稍微向下延伸，与颈部重叠
    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        centerX - headW / 2, centerY - BLOCK_SIZE * 0.5f, centerZ - headD / 2,  // 向下延伸0.5个单位
        headW, headH + BLOCK_SIZE, headD, skinColor);  // 高度增加1个单位

    // 2. 左眼 - 稍微嵌入头部
    float eyeSize = BLOCK_SIZE * 1.5f;
    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        centerX - eyeSize * 1.2f, centerY + headH * 0.6f, centerZ + headD / 2 - BLOCK_SIZE * 0.3f,  // 更嵌入
        eyeSize, eyeSize, BLOCK_SIZE * 0.6f, eyeColor);  // 减少突出

    // 3. 右眼 - 稍微嵌入头部
    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        centerX + eyeSize * 0.2f, centerY + headH * 0.6f, centerZ + headD / 2 - BLOCK_SIZE * 0.3f,
        eyeSize, eyeSize, BLOCK_SIZE * 0.6f, eyeColor);

    // 4. 鼻子 - 更小，更嵌入
    float noseSize = BLOCK_SIZE * 0.8f;  // 更小
    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        centerX - noseSize / 2, centerY + headH * 0.4f, centerZ + headD / 2 - BLOCK_SIZE * 0.2f,
        noseSize, noseSize, BLOCK_SIZE * 0.4f, skinColor);

    // 5. 嘴巴 - 更嵌入
    float mouthW = BLOCK_SIZE * 2.0f;
    float mouthH = BLOCK_SIZE * 0.5f;
    XMFLOAT4 mouthColor = XMFLOAT4(0.2f, 0.1f, 0.1f, 1.0f);
    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        centerX - mouthW / 2, centerY + headH * 0.2f, centerZ + headD / 2 - BLOCK_SIZE * 0.1f,
        mouthW, mouthH, BLOCK_SIZE * 0.3f, mouthColor);

    // 6. 头发/帽子 - 与头部重叠更多
    XMFLOAT4 headwearColor = hairColor;
    float hairH = headH * 0.4f;

    switch (type) {
    case CHARACTER_DOCTOR:
        headwearColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        break;
    case CHARACTER_SOLDIER:
        headwearColor = XMFLOAT4(0.2f, 0.3f, 0.1f, 1.0f);
        hairH = headH * 0.5f;
        break;
    case CHARACTER_SCOUT:
        headwearColor = XMFLOAT4(0.4f, 0.3f, 0.2f, 1.0f);
        break;
    case CHARACTER_BOMB_TECH:
        headwearColor = XMFLOAT4(0.1f, 0.1f, 0.2f, 1.0f);
        hairH = headH * 0.6f;
        break;
    }

    // 头发与头部重叠，向下延伸
    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        centerX - headW * 0.55f, centerY + headH * 0.5f, centerZ - headD * 0.55f,  // 向下移动
        headW * 1.1f, hairH + BLOCK_SIZE, headD * 1.1f, headwearColor);  // 高度增加

    // 7. 颈部 - 与头部和躯干都重叠
    float neckW = BLOCK_SIZE * NECK_WIDTH;
    float neckH = BLOCK_SIZE * NECK_HEIGHT;
    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        centerX - neckW / 2, centerY - neckH - BLOCK_SIZE * 0.5f, centerZ - neckW / 2,  // 向上延伸
        neckW, neckH + BLOCK_SIZE, neckW, skinColor);  // 高度增加，与头部重叠
}

// 🔧 修改CreateRealisticTorso函数 - 改善连接
void CreateRealisticTorso(VERTEX_3D* vertices, unsigned int* indices,
    int& vertexIndex, int& indexIndex,
    float centerX, float centerY, float centerZ,
    XMFLOAT4 shirtColor, XMFLOAT4 detailColor, CHARACTER_TYPE type)
{
    // 1. 胸部 - 向上延伸与颈部重叠
    float chestW = BLOCK_SIZE * CHEST_WIDTH;
    float chestH = BLOCK_SIZE * CHEST_HEIGHT;
    float chestD = BLOCK_SIZE * CHEST_DEPTH;

    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        centerX - chestW / 2, centerY - BLOCK_SIZE * 0.5f, centerZ - chestD / 2,  // 向上延伸
        chestW, chestH + BLOCK_SIZE, chestD, shirtColor);  // 高度增加

    // 2. 腰部 - 与胸部重叠
    float waistW = BLOCK_SIZE * WAIST_WIDTH;
    float waistH = BLOCK_SIZE * WAIST_HEIGHT;

    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        centerX - waistW / 2, centerY - waistH - BLOCK_SIZE * 0.5f, centerZ - chestD / 2,  // 向上重叠
        waistW, waistH + BLOCK_SIZE, chestD, shirtColor);  // 高度增加

    // 3. 职业特色装饰 - 嵌入胸部
    float decorW = chestW * 0.4f;
    float decorH = chestH * 0.3f;

    switch (type) {
    case CHARACTER_DOCTOR:
        // 红十字 - 稍微嵌入
        AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
            centerX - decorW / 2, centerY + chestH * 0.3f, centerZ + chestD / 2 - BLOCK_SIZE * 0.1f,
            decorW, decorH, BLOCK_SIZE * 0.3f, XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f));
        break;
    case CHARACTER_SOLDIER:
        // 军徽
        AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
            centerX - decorW / 2, centerY + chestH * 0.4f, centerZ + chestD / 2 - BLOCK_SIZE * 0.1f,
            decorW, decorH, BLOCK_SIZE * 0.3f, XMFLOAT4(0.7f, 0.7f, 0.1f, 1.0f));
        break;
    case CHARACTER_SCOUT:
        // 装备带 - 环绕身体
        AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
            centerX - chestW * 0.55f, centerY + chestH * 0.2f, centerZ - chestD * 0.55f,
            chestW * 1.1f, BLOCK_SIZE * 0.8f, chestD * 1.1f, XMFLOAT4(0.4f, 0.25f, 0.1f, 1.0f));
        break;
    case CHARACTER_BOMB_TECH:
        // 黄色警示条 - 环绕身体
        AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
            centerX - chestW * 0.55f, centerY + chestH * 0.1f, centerZ - chestD * 0.55f,
            chestW * 1.1f, BLOCK_SIZE * 1.2f, chestD * 1.1f, XMFLOAT4(0.9f, 0.8f, 0.1f, 1.0f));
        break;
    }
}


// 最小改动的两步修正

// 🔧 修改1：CreateRealisticArm - 让调用方决定偏移，函数内部不改X
void CreateRealisticArm(VERTEX_3D* vertices, unsigned int* indices,
    int& vertexIndex, int& indexIndex,
    float shoulderX, float shoulderY, float shoulderZ,
    bool isLeft, XMFLOAT4 skinColor, XMFLOAT4 sleeveColor)
{
    float side = isLeft ? -1.0f : 1.0f;

    // 1. 肩膀 - ★ 直接用shoulderX，不再偏移
    float shoulderSize = BLOCK_SIZE * SHOULDER_SIZE;
    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        shoulderX, shoulderY - BLOCK_SIZE * 0.5f, shoulderZ - shoulderSize / 2,
        shoulderSize, shoulderSize + BLOCK_SIZE, shoulderSize, sleeveColor);

    // 2. 上臂 - ★ 上臂根部同理，直接用shoulderX
    float upperArmW = BLOCK_SIZE * UPPER_ARM_WIDTH;
    float upperArmL = BLOCK_SIZE * UPPER_ARM_LENGTH;
    float upperArmX = shoulderX;  // <- 删除0.6f偏移

    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        upperArmX, shoulderY - upperArmL - BLOCK_SIZE * 0.3f, shoulderZ - upperArmW / 2,
        upperArmW, upperArmL + BLOCK_SIZE, upperArmW, sleeveColor);

    // 3. 肘部关节
    float elbowSize = BLOCK_SIZE * 2.5f;
    float elbowY = shoulderY - upperArmL - BLOCK_SIZE * 0.5f;
    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        upperArmX, elbowY - elbowSize / 2, shoulderZ - elbowSize / 2,
        elbowSize, elbowSize, elbowSize, skinColor);

    // 4. 前臂
    float forearmW = BLOCK_SIZE * FOREARM_WIDTH;
    float forearmL = BLOCK_SIZE * FOREARM_LENGTH;
    float forearmY = elbowY - elbowSize / 2;

    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        upperArmX + (upperArmW - forearmW) / 2, forearmY - forearmL + BLOCK_SIZE * 0.3f, shoulderZ - forearmW / 2,
        forearmW, forearmL, forearmW, skinColor);

    // 5. 手腕
    float wristSize = BLOCK_SIZE * 1.5f;
    float wristY = forearmY - forearmL + BLOCK_SIZE * 0.5f;
    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        upperArmX + (upperArmW - wristSize) / 2, wristY - wristSize / 2, shoulderZ - wristSize / 2,
        wristSize, wristSize, wristSize, skinColor);

    // 6. 手部
    float handW = BLOCK_SIZE * HAND_WIDTH;
    float handH = BLOCK_SIZE * HAND_HEIGHT;
    float handD = BLOCK_SIZE * HAND_DEPTH;

    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        upperArmX + (upperArmW - handW) / 2, wristY - wristSize / 2 - handH + BLOCK_SIZE * 0.2f, shoulderZ - handD / 2,
        handW, handH, handD, skinColor);
}

// 🔧 修改2：CreateRealisticLeg - 直接使用传入的hipX
void CreateRealisticLeg(VERTEX_3D* vertices, unsigned int* indices,
    int& vertexIndex, int& indexIndex,
    float hipX, float hipY, float hipZ,
    bool isLeft, XMFLOAT4 pantColor, XMFLOAT4 shoeColor)
{
    float side = isLeft ? -1.0f : 1.0f;

    // 1. 髋部 - ★ 直接使用传入的hipX
    float hipW = BLOCK_SIZE * HIP_WIDTH;
    float hipH = BLOCK_SIZE * 2;
    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        hipX, hipY - hipH + BLOCK_SIZE * 0.5f, hipZ - hipH / 2,
        hipW * 0.8f, hipH, hipH, pantColor);

    // 2. 大腿 - ★ 直接使用hipX
    float thighW = BLOCK_SIZE * THIGH_WIDTH;
    float thighL = BLOCK_SIZE * THIGH_LENGTH;
    float thighX = hipX;  // <- 删除复杂的偏移计算

    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        thighX, hipY - hipH - thighL + BLOCK_SIZE * 0.5f, hipZ - thighW / 2,
        thighW, thighL + BLOCK_SIZE, thighW, pantColor);

    // 3. 膝盖
    float kneeSize = BLOCK_SIZE * KNEE_SIZE;
    float kneeY = hipY - hipH - thighL;
    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        thighX, kneeY - kneeSize / 2, hipZ - kneeSize / 2,
        kneeSize, kneeSize, kneeSize, pantColor);

    // 4. 小腿
    float shinW = BLOCK_SIZE * SHIN_WIDTH;
    float shinL = BLOCK_SIZE * SHIN_LENGTH;
    float shinY = kneeY - kneeSize / 2;

    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        thighX + (thighW - shinW) / 2, shinY - shinL + BLOCK_SIZE * 0.3f, hipZ - shinW / 2,
        shinW, shinL, shinW, pantColor);

    // 5. 脚部
    float footW = BLOCK_SIZE * FOOT_WIDTH;
    float footH = BLOCK_SIZE * FOOT_HEIGHT;
    float footL = BLOCK_SIZE * FOOT_LENGTH;

    AddDetailedPixelBlock(vertices, indices, vertexIndex, indexIndex,
        thighX + (thighW - footW) / 2, shinY - shinL - footH + BLOCK_SIZE * 0.2f, hipZ - shinW / 2,
        footW, footH, footL, shoeColor);
}

// 🔧 修改3：CreateRealisticPixelCharacter - 用纵向锚点系统
void CreateRealisticPixelCharacter(PixelCharacter* character,
    XMFLOAT4 skinColor, XMFLOAT4 hairColor, XMFLOAT4 eyeColor,
    XMFLOAT4 shirtColor, XMFLOAT4 pantColor, XMFLOAT4 shoeColor,
    XMFLOAT4 detailColor, CHARACTER_TYPE type)
{
    OutputDebugStringA("🎮 开始创建纵向锚点角色...\n");

    // 安全检查
    if (!character) {
        OutputDebugStringA("❌ character指针为空\n");
        return;
    }

    ID3D11Device* device = GetDevice();
    if (!device) {
        OutputDebugStringA("❌ 无法获取设备\n");
        return;
    }

    const int REALISTIC_PARTS = 32;
    character->VertexCount = REALISTIC_PARTS * 24;
    character->IndexCount = REALISTIC_PARTS * 36;

    VERTEX_3D* vertices = new(std::nothrow) VERTEX_3D[character->VertexCount];
    unsigned int* indices = new(std::nothrow) unsigned int[character->IndexCount];

    if (!vertices || !indices) {
        if (vertices) delete[] vertices;
        if (indices) delete[] indices;
        return;
    }

    memset(vertices, 0, sizeof(VERTEX_3D) * character->VertexCount);
    memset(indices, 0, sizeof(unsigned int) * character->IndexCount);

    int vertexIndex = 0;
    int indexIndex = 0;

    try {
        // 🔧 纵向锚点系统 - 从脚底开始往上累加
        float footH = BLOCK_SIZE * FOOT_HEIGHT;
        float shinL = BLOCK_SIZE * SHIN_LENGTH;
        float thighL = BLOCK_SIZE * THIGH_LENGTH;
        float hipH = BLOCK_SIZE * 2.0f;           // 髋部高度
        float waistH = BLOCK_SIZE * WAIST_HEIGHT;
        float chestH = BLOCK_SIZE * CHEST_HEIGHT;
        float neckH = BLOCK_SIZE * NECK_HEIGHT;
        float headH = BLOCK_SIZE * HEAD_HEIGHT;

        // 横向尺寸
        float chestW = BLOCK_SIZE * CHEST_WIDTH;
        float hipW = BLOCK_SIZE * HIP_WIDTH;

        // 角色中心
        float centerX = 0.0f;
        float centerZ = 0.0f;

        // ground → 脚 → 小腿 → 大腿 → 髋 → 腰 → 胸 → 颈 → 头
        float yGround = 0.0f;
        float yHip = yGround + footH + shinL + thighL;      // 髋部底
        float yWaist = yHip + hipH;                         // 腰部底
        float yChest = yWaist + waistH;                     // 胸部底
        float yShoulder = yChest + chestH;                  // 肩膀高度
        float yNeckTop = yShoulder + neckH;                 // 颈顶
        float yHead = yNeckTop;                             // 头底

        // 创建各部位 - 传递精确的锚点位置
        CreateRealisticTorso(vertices, indices, vertexIndex, indexIndex,
            centerX, yChest, centerZ, shirtColor, detailColor, type);

        // 手臂 - 让调用方决定X偏移
        CreateRealisticArm(vertices, indices, vertexIndex, indexIndex,
            centerX + chestW * 0.5f, yShoulder, centerZ, false, skinColor, shirtColor);
        CreateRealisticArm(vertices, indices, vertexIndex, indexIndex,
            centerX - chestW * 0.5f, yShoulder, centerZ, true, skinColor, shirtColor);

        // 腿部 - 让调用方决定X偏移
        CreateRealisticLeg(vertices, indices, vertexIndex, indexIndex,
            centerX + hipW * 0.3f, yHip, centerZ, false, pantColor, shoeColor);
        CreateRealisticLeg(vertices, indices, vertexIndex, indexIndex,
            centerX - hipW * 0.3f, yHip, centerZ, true, pantColor, shoeColor);

        // 头部
        CreateRealisticHead(vertices, indices, vertexIndex, indexIndex,
            centerX, yHead + neckH, centerZ, skinColor, hairColor, eyeColor, type);

        OutputDebugStringA("✅ 纵向锚点角色创建成功\n");

    }
    catch (...) {
        OutputDebugStringA("❌ 角色创建异常\n");
        delete[] vertices;
        delete[] indices;
        return;
    }

    // 创建缓冲区
    HRESULT hr;

    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VERTEX_3D) * character->VertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexData;
    ZeroMemory(&vertexData, sizeof(vertexData));
    vertexData.pSysMem = vertices;

    hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &character->VertexBuffer);
    if (FAILED(hr)) {
        delete[] vertices;
        delete[] indices;
        return;
    }

    D3D11_BUFFER_DESC indexBufferDesc;
    ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned int) * character->IndexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexData;
    ZeroMemory(&indexData, sizeof(indexData));
    indexData.pSysMem = indices;

    hr = device->CreateBuffer(&indexBufferDesc, &indexData, &character->IndexBuffer);
    if (FAILED(hr)) {
        character->VertexBuffer->Release();
        character->VertexBuffer = nullptr;
        delete[] vertices;
        delete[] indices;
        return;
    }

    delete[] vertices;
    delete[] indices;

    OutputDebugStringA("✅ 纵向锚点角色缓冲区创建成功\n");
}


// 🎮 增强版角色创建函数

void CreatePixelDoctor(PixelCharacter* character)
{
    character->Type = CHARACTER_DOCTOR;

    // 医生专业配色
    XMFLOAT4 skinColor = XMFLOAT4(0.95f, 0.87f, 0.73f, 1.0f);    // 肤色
    XMFLOAT4 hairColor = XMFLOAT4(0.4f, 0.3f, 0.2f, 1.0f);      // 棕色头发
    XMFLOAT4 eyeColor = XMFLOAT4(0.2f, 0.4f, 0.8f, 1.0f);       // 蓝色眼睛
    XMFLOAT4 whiteCoat = XMFLOAT4(0.96f, 0.96f, 1.0f, 1.0f);    // 白大褂
    XMFLOAT4 darkPants = XMFLOAT4(0.2f, 0.2f, 0.4f, 1.0f);      // 深色裤子
    XMFLOAT4 blackShoes = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);     // 黑色鞋子
    XMFLOAT4 redCross = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);       // 红十字

    CreateRealisticPixelCharacter(character, skinColor, hairColor, eyeColor,
        whiteCoat, darkPants, blackShoes, redCross, CHARACTER_DOCTOR);

    character->MainColor = whiteCoat;
    character->SecondaryColor = darkPants;
    character->DetailColor = redCross;
}

void CreatePixelScout(PixelCharacter* character)
{
    character->Type = CHARACTER_SCOUT;

    // 侦察兵户外配色
    XMFLOAT4 fairSkin = XMFLOAT4(0.9f, 0.8f, 0.7f, 1.0f);       // 白皙肤色
    XMFLOAT4 sandyHair = XMFLOAT4(0.6f, 0.5f, 0.3f, 1.0f);     // 沙色头发
    XMFLOAT4 greenEyes = XMFLOAT4(0.2f, 0.6f, 0.3f, 1.0f);      // 绿色眼睛
    XMFLOAT4 camouflageGreen = XMFLOAT4(0.4f, 0.5f, 0.3f, 1.0f); // 迷彩绿
    XMFLOAT4 camouflageBrown = XMFLOAT4(0.5f, 0.35f, 0.2f, 1.0f); // 迷彩棕
    XMFLOAT4 brownBoots = XMFLOAT4(0.3f, 0.2f, 0.1f, 1.0f);     // 棕色靴子
    XMFLOAT4 khakiGear = XMFLOAT4(0.6f, 0.55f, 0.4f, 1.0f);     // 卡其色装备

    CreateRealisticPixelCharacter(character, fairSkin, sandyHair, greenEyes,
        camouflageGreen, camouflageBrown, brownBoots, khakiGear, CHARACTER_SCOUT);

    character->MainColor = camouflageGreen;
    character->SecondaryColor = camouflageBrown;
    character->DetailColor = khakiGear;
}

void CreatePixelBombTech(PixelCharacter* character)
{
    character->Type = CHARACTER_BOMB_TECH;

    // 拆弹手防护配色
    XMFLOAT4 paleSkin = XMFLOAT4(0.9f, 0.85f, 0.75f, 1.0f);     // 苍白肤色
    XMFLOAT4 darkHair = XMFLOAT4(0.15f, 0.15f, 0.2f, 1.0f);    // 深色头发
    XMFLOAT4 focusedEyes = XMFLOAT4(0.3f, 0.5f, 0.7f, 1.0f);    // 专注蓝眼
    XMFLOAT4 hazmatSuit = XMFLOAT4(0.15f, 0.15f, 0.2f, 1.0f);   // 防护服
    XMFLOAT4 reinforcedPants = XMFLOAT4(0.1f, 0.1f, 0.15f, 1.0f); // 加强裤子
    XMFLOAT4 heavyBoots = XMFLOAT4(0.05f, 0.05f, 0.1f, 1.0f);   // 重型靴子
    XMFLOAT4 yellowWarning = XMFLOAT4(0.9f, 0.8f, 0.1f, 1.0f);  // 黄色警示

    CreateRealisticPixelCharacter(character, paleSkin, darkHair, focusedEyes,
        hazmatSuit, reinforcedPants, heavyBoots, yellowWarning, CHARACTER_BOMB_TECH);

    character->MainColor = hazmatSuit;
    character->SecondaryColor = yellowWarning;
    character->DetailColor = XMFLOAT4(0.2f, 0.2f, 0.25f, 1.0f);
}
void CreatePixelSoldier(PixelCharacter* character)
{
    character->Type = CHARACTER_SOLDIER;

    // 士兵军事配色
    XMFLOAT4 tanSkin = XMFLOAT4(0.85f, 0.75f, 0.65f, 1.0f);     // 晒黑肤色
    XMFLOAT4 darkHair = XMFLOAT4(0.2f, 0.15f, 0.1f, 1.0f);     // 深色头发
    XMFLOAT4 brownEyes = XMFLOAT4(0.3f, 0.2f, 0.1f, 1.0f);      // 棕色眼睛
    XMFLOAT4 militaryGreen = XMFLOAT4(0.25f, 0.35f, 0.15f, 1.0f); // 军绿色
    XMFLOAT4 darkGreen = XMFLOAT4(0.15f, 0.25f, 0.1f, 1.0f);    // 深军绿
    XMFLOAT4 blackBoots = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);     // 黑色军靴
    XMFLOAT4 goldBadge = XMFLOAT4(0.8f, 0.7f, 0.2f, 1.0f);      // 金色徽章

    CreateRealisticPixelCharacter(character, tanSkin, darkHair, brownEyes,
        militaryGreen, darkGreen, blackBoots, goldBadge, CHARACTER_SOLDIER);

    character->MainColor = militaryGreen;
    character->SecondaryColor = darkGreen;
    character->DetailColor = goldBadge;

}
// 🎮 增强的绘制函数 - 支持更细致的角色渲染
void DrawPixelCharacter(PixelCharacter* character, XMMATRIX worldMatrix)
{
    if (!character->VertexBuffer || !character->IndexBuffer) {
        return;
    }

    // 保存状态
    MATERIAL oldMaterial;
    GetMaterial(&oldMaterial);
    XMMATRIX oldWorldMatrix = GetWorldMatrix();
    bool oldBlendState = GetBlendState();
    D3D11_CULL_MODE oldCullMode = GetCullingMode();

    // 设置渲染状态
    SetBlendState(false);
    SetCulingMode(D3D11_CULL_BACK);
    SetDepthEnable(true);
    SetDepthWriteEnable(true);

    // 获取动画参数
    float bobHeight = GetCharacterBobHeight(character->Type);
    float armSwing = GetCharacterArmSwing(character->Type);

    // 根据角色类型设置增强材质
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));

    switch (character->Type)
    {
    case CHARACTER_DOCTOR:
        // 医生：清洁明亮
        material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        material.Ambient = XMFLOAT4(0.8f, 0.8f, 0.85f, 1.0f);
        material.Specular = XMFLOAT4(0.5f, 0.5f, 0.6f, 25.0f);
        material.Emission = XMFLOAT4(0.08f, 0.08f, 0.12f, 0.0f);
        break;

    case CHARACTER_SOLDIER:
        // 士兵：坚硬粗糙
        material.Diffuse = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
        material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        material.Specular = XMFLOAT4(0.15f, 0.15f, 0.1f, 8.0f);
        material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
        break;

    case CHARACTER_SCOUT:
        // 侦察兵：自然质感
        material.Diffuse = XMFLOAT4(0.95f, 0.9f, 0.85f, 1.0f);
        material.Ambient = XMFLOAT4(0.65f, 0.6f, 0.55f, 1.0f);
        material.Specular = XMFLOAT4(0.2f, 0.2f, 0.15f, 12.0f);
        material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
        break;

    case CHARACTER_BOMB_TECH:
        // 拆弹手：高科技感
        material.Diffuse = XMFLOAT4(0.85f, 0.85f, 0.95f, 1.0f);
        material.Ambient = XMFLOAT4(0.35f, 0.35f, 0.4f, 1.0f);
        material.Specular = XMFLOAT4(0.7f, 0.7f, 0.8f, 35.0f);
        material.Emission = XMFLOAT4(0.03f, 0.08f, 0.03f, 0.0f);
        break;
    }

    SetMaterial(material);

    // 使用白色纹理
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_TextureWhite);

    // 应用增强的动画变换
    XMMATRIX bobMatrix = XMMatrixTranslation(0.0f, bobHeight, 0.0f);

    // 为不同角色添加轻微的摇摆动画
    float swayAngle = armSwing * 0.1f; // 轻微的身体摇摆
    XMMATRIX swayMatrix = XMMatrixRotationY(swayAngle);

    XMMATRIX characterMatrix = swayMatrix * bobMatrix * worldMatrix;
    SetWorldMatrix(characterMatrix);

    // 绘制完整角色
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &character->VertexBuffer, &stride, &offset);
    GetDeviceContext()->IASetIndexBuffer(character->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 绘制所有22个部件
    GetDeviceContext()->DrawIndexed(character->IndexCount, 0, 0);

    // 恢复状态
    SetWorldMatrix(oldWorldMatrix);
    SetMaterial(oldMaterial);
    SetBlendState(oldBlendState);
    SetCulingMode(oldCullMode);
}
//// 🎮 更新角色动画 - 增强版
//void UpdatePixelCharacters(float deltaTime)
//{
//    // 更新动画计时器
//    g_AnimationTime += deltaTime;
//    if (g_AnimationTime >= XM_2PI) {
//        g_AnimationTime -= XM_2PI;
//    }
//
//    // 更新各角色的上下摆动高度（更自然的动作）
//    g_CharacterBobHeights[CHARACTER_DOCTOR] = sinf(g_AnimationTime * 1.8f) * 0.025f;     // 医生：平稳专业
//    g_CharacterBobHeights[CHARACTER_SOLDIER] = sinf(g_AnimationTime * 2.5f) * 0.02f;     // 士兵：军事步伐
//    g_CharacterBobHeights[CHARACTER_SCOUT] = sinf(g_AnimationTime * 3.2f) * 0.035f;      // 侦察兵：敏捷轻快
//    g_CharacterBobHeights[CHARACTER_BOMB_TECH] = sinf(g_AnimationTime * 1.2f) * 0.015f;  // 拆弹手：谨慎稳重
//
//    // 更新各角色的手臂摆动角度（更细腻的动作）
//    g_CharacterArmSwings[CHARACTER_DOCTOR] = sinf(g_AnimationTime * 1.8f) * 0.12f;       // 医生：温和摆动
//    g_CharacterArmSwings[CHARACTER_SOLDIER] = sinf(g_AnimationTime * 2.5f) * 0.18f;      // 士兵：有力摆动
//    g_CharacterArmSwings[CHARACTER_SCOUT] = sinf(g_AnimationTime * 3.2f) * 0.22f;        // 侦察兵：活跃摆动
//    g_CharacterArmSwings[CHARACTER_BOMB_TECH] = sinf(g_AnimationTime * 1.2f) * 0.08f;    // 拆弹手：小心摆动
//}
// 🎮 更新动画 - 为真实角色优化
void UpdatePixelCharacters(float deltaTime)
{
    g_AnimationTime += deltaTime;
    if (g_AnimationTime >= XM_2PI) {
        g_AnimationTime -= XM_2PI;
    }

    // 更自然的角色动画 - 每个角色有独特的个性

    // 医生：稳重专业的动作
    g_CharacterBobHeights[CHARACTER_DOCTOR] = sinf(g_AnimationTime * 1.5f) * 0.02f;
    g_CharacterArmSwings[CHARACTER_DOCTOR] = sinf(g_AnimationTime * 1.5f) * 0.1f;

    // 士兵：有力规律的步伐
    g_CharacterBobHeights[CHARACTER_SOLDIER] = sinf(g_AnimationTime * 2.2f) * 0.018f;
    g_CharacterArmSwings[CHARACTER_SOLDIER] = sinf(g_AnimationTime * 2.2f) * 0.15f;

    // 侦察兵：敏捷轻快的动作
    g_CharacterBobHeights[CHARACTER_SCOUT] = sinf(g_AnimationTime * 3.0f) * 0.03f;
    g_CharacterArmSwings[CHARACTER_SCOUT] = sinf(g_AnimationTime * 3.0f) * 0.2f;

    // 拆弹手：谨慎缓慢的动作
    g_CharacterBobHeights[CHARACTER_BOMB_TECH] = sinf(g_AnimationTime * 1.0f) * 0.012f;
    g_CharacterArmSwings[CHARACTER_BOMB_TECH] = sinf(g_AnimationTime * 1.0f) * 0.06f;
}

// 🎮 初始化像素角色模型 - 增强版
void InitPixelCharacters()
{
    // 初始化结构体
    ZeroMemory(&g_Doctor, sizeof(PixelCharacter));
    ZeroMemory(&g_Soldier, sizeof(PixelCharacter));
    ZeroMemory(&g_Scout, sizeof(PixelCharacter));
    ZeroMemory(&g_BombTech, sizeof(PixelCharacter));

    // 创建各角色模型
    CreatePixelDoctor(&g_Doctor);
    CreatePixelSoldier(&g_Soldier);
    CreatePixelScout(&g_Scout);
    CreatePixelBombTech(&g_BombTech);

    g_PixelCharactersInitialized = true;
    OutputDebugStringA("🎮 增强版像素角色模型初始化成功\n");
}

// 🎮 清理像素角色模型 - 保持原有逻辑
void UninitPixelCharacters()
{
    // 释放医生模型资源
    if (g_Doctor.VertexBuffer) {
        g_Doctor.VertexBuffer->Release();
        g_Doctor.VertexBuffer = NULL;
    }
    if (g_Doctor.IndexBuffer) {
        g_Doctor.IndexBuffer->Release();
        g_Doctor.IndexBuffer = NULL;
    }

    // 释放士兵模型资源
    if (g_Soldier.VertexBuffer) {
        g_Soldier.VertexBuffer->Release();
        g_Soldier.VertexBuffer = NULL;
    }
    if (g_Soldier.IndexBuffer) {
        g_Soldier.IndexBuffer->Release();
        g_Soldier.IndexBuffer = NULL;
    }

    // 释放侦察兵模型资源
    if (g_Scout.VertexBuffer) {
        g_Scout.VertexBuffer->Release();
        g_Scout.VertexBuffer = NULL;
    }
    if (g_Scout.IndexBuffer) {
        g_Scout.IndexBuffer->Release();
        g_Scout.IndexBuffer = NULL;
    }

    // 释放拆弹手模型资源
    if (g_BombTech.VertexBuffer) {
        g_BombTech.VertexBuffer->Release();
        g_BombTech.VertexBuffer = NULL;
    }
    if (g_BombTech.IndexBuffer) {
        g_BombTech.IndexBuffer->Release();
        g_BombTech.IndexBuffer = NULL;
    }

    g_PixelCharactersInitialized = false;
    OutputDebugStringA("🎮 增强版像素角色模型清理成功\n");
}

// 保持原有的辅助函数
float GetCharacterBobHeight(CHARACTER_TYPE type) {
    if (type >= 0 && type < CHARACTER_TYPE_COUNT) {
        return g_CharacterBobHeights[type];
    }
    return 0.0f;
}

float GetCharacterArmSwing(CHARACTER_TYPE type) {
    if (type >= 0 && type < CHARACTER_TYPE_COUNT) {
        return g_CharacterArmSwings[type];
    }
    return 0.0f;
}

void SetCharacterBobHeight(CHARACTER_TYPE type, float height) {
    if (type >= 0 && type < CHARACTER_TYPE_COUNT) {
        g_CharacterBobHeights[type] = height;
    }
}

void SetCharacterArmSwing(CHARACTER_TYPE type, float swing) {
    if (type >= 0 && type < CHARACTER_TYPE_COUNT) {
        g_CharacterArmSwings[type] = swing;
    }
}

void ResetCharacterAnimation(CHARACTER_TYPE type) {
    if (type >= 0 && type < CHARACTER_TYPE_COUNT) {
        g_CharacterBobHeights[type] = 0.0f;
        g_CharacterArmSwings[type] = 0.0f;
    }
}

bool GetPixelCharactersInitialized() {
    return g_PixelCharactersInitialized;
}

bool GetPixelCharacterValid(CHARACTER_TYPE type) {
    if (!g_PixelCharactersInitialized) return false;

    switch (type) {
    case CHARACTER_DOCTOR:
        return (g_Doctor.VertexBuffer != nullptr && g_Doctor.IndexBuffer != nullptr);
    case CHARACTER_SOLDIER:
        return (g_Soldier.VertexBuffer != nullptr && g_Soldier.IndexBuffer != nullptr);
    case CHARACTER_SCOUT:
        return (g_Scout.VertexBuffer != nullptr && g_Scout.IndexBuffer != nullptr);
    case CHARACTER_BOMB_TECH:
        return (g_BombTech.VertexBuffer != nullptr && g_BombTech.IndexBuffer != nullptr);
    default:
        return false;
    }
}