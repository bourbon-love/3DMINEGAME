// CharacterScene.cpp - 简化的角色选择场景
#include "CharactorScene.h"
#include "renderer.h"
#include "keyboard.h"
#include "SceneManager.h"
#include "PixelCharacters.h"
#include "Camera.h"
#include "mouse.h"
#include "UIManager.h"
#include "BattleHall.h"
#include "FrameWork/TextureManager.h"
// 角色展示台结构
struct CharacterPlatform {
    XMFLOAT3 position;        // 平台位置
    XMFLOAT3 characterPos;    // 角色位置
    CHARACTER_TYPE type;      // 角色类型
    bool isSelected;          // 是否被选中
    float scaleAnimation;     // 缩放动画值
    XMFLOAT4 platformColor;   // 平台颜色
};

// 全局变量
static CharacterPlatform g_CharacterPlatforms[4];
static int g_SelectedCharacterIndex = 0;
static bool g_CharacterSceneActive = false;
static ID3D11Buffer* g_PlatformVertexBuffer = nullptr;
static ID3D11Buffer* g_PlatformIndexBuffer = nullptr;
// 全局纹理对象
//extern ID3D11ShaderResourceView* g_Texture;    //テクスチャ変数
//extern ID3D11ShaderResourceView* g_TextureIce;    // 冰面纹理
//extern ID3D11ShaderResourceView* g_TextureSand;   // 沙地纹理
//extern ID3D11ShaderResourceView* g_TextureGrass;  // 草地纹理
//extern ID3D11ShaderResourceView* g_TextureWater;  // 水面纹理
//extern ID3D11ShaderResourceView* g_TextureWall;   // 墙体纹理
//extern ID3D11ShaderResourceView* g_TextureGrid;   // 编辑纹理
//extern ID3D11ShaderResourceView* g_TexturePartical;   // 粒子纹理
//extern ID3D11ShaderResourceView* g_TextureCoin;   // 金币纹理
//extern ID3D11ShaderResourceView* g_TextureBomb;   // 炸弹纹理

// 🔧 新增：角色说明状态管理
static bool g_ShowingCharacterInfo = false;
static int g_InfoCharacterIndex = -1;
static float g_CharacterRotationAngle = 0.0f;
static float g_InfoTextureAlpha = 0.0f;
static bool g_IsRotatingOut = false;
static bool g_IsRotatingIn = false;

// 🔧 修改全局变量，为每个角色单独管理旋转和纹理状态
static float g_CharacterRotationAngles[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  // 每个角色独立旋转
static float g_CharacterInfoAlphas[4] = { 0.0f, 0.0f, 0.0f, 0.0f };      // 每个角色独立透明度
static bool g_CharacterInfoVisible[4] = { false, false, false, false };   // 每个角色独立可见性


//角色说明纹理（每个角色一张完整说明图）
ID3D11ShaderResourceView* g_CharacterInfoTextures[4] = { nullptr };
 
// 平台顶点数据
static VERTEX_3D g_PlatformVertices[24];
static UINT g_PlatformIndices[36];
// 🎨 全局变量存储平台索引数量
static int g_PlatformIndexCount = 0;

// 1. 在全局变量中添加选择确认状态
static bool g_CharacterConfirmed = false;
static int g_ConfirmedCharacterIndex = -1;


void CreatePlatformGeometry() {
    OutputDebugStringA("🎮 创建方盒子平台几何体（Box风格）\n");

    // 🎮 使用与Box.cpp完全相同的盒子尺寸和顶点结构
    // 平台尺寸：比普通Box稍大一些，更适合作为展示台
    float w = 1.5f;  // 宽度
    float h = 0.6f;  // 高度（比普通Box高一点）
    float d = 1.5f;  // 深度

    // 🎮 完全复制Box.cpp中的顶点结构，但调整尺寸
    // 使用静态数组，与Box.cpp保持一致的结构
    static VERTEX_3D vertices[36] = {
        // 顶面（天井） +Y - 与Box.cpp完全一致的UV映射
        { {-w,  h,  d}, {0, 1, 0}, {1,1,1,1}, {0.0f, 0.0f} },
        { { w,  h,  d}, {0, 1, 0}, {1,1,1,1}, {1.0f, 0.0f} },
        { {-w,  h, -d}, {0, 1, 0}, {1,1,1,1}, {0.0f, 1.0f} },
        { { w,  h,  d}, {0, 1, 0}, {1,1,1,1}, {1.0f, 0.0f} },
        { { w,  h, -d}, {0, 1, 0}, {1,1,1,1}, {1.0f, 1.0f} },
        { {-w,  h, -d}, {0, 1, 0}, {1,1,1,1}, {0.0f, 1.0f} },

        // 底面 -Y
        { {-w, -h, -d}, {0,-1, 0}, {1,1,1,1}, {0.0f, 0.0f} },
        { { w, -h, -d}, {0,-1, 0}, {1,1,1,1}, {1.0f, 0.0f} },
        { {-w, -h,  d}, {0,-1, 0}, {1,1,1,1}, {0.0f, 1.0f} },
        { { w, -h, -d}, {0,-1, 0}, {1,1,1,1}, {1.0f, 0.0f} },
        { { w, -h,  d}, {0,-1, 0}, {1,1,1,1}, {1.0f, 1.0f} },
        { {-w, -h,  d}, {0,-1, 0}, {1,1,1,1}, {0.0f, 1.0f} },

        // 前面 -Z
        { {-w,  h, -d}, {0, 0,-1}, {1,1,1,1}, {0.0f, 0.0f} },
        { { w,  h, -d}, {0, 0,-1}, {1,1,1,1}, {1.0f, 0.0f} },
        { {-w, -h, -d}, {0, 0,-1}, {1,1,1,1}, {0.0f, 1.0f} },
        { { w,  h, -d}, {0, 0,-1}, {1,1,1,1}, {1.0f, 0.0f} },
        { { w, -h, -d}, {0, 0,-1}, {1,1,1,1}, {1.0f, 1.0f} },
        { {-w, -h, -d}, {0, 0,-1}, {1,1,1,1}, {0.0f, 1.0f} },

        // 背面 +Z
        { { w,  h,  d}, {0, 0, 1}, {1,1,1,1}, {0.0f, 0.0f} },
        { {-w,  h,  d}, {0, 0, 1}, {1,1,1,1}, {1.0f, 0.0f} },
        { { w, -h,  d}, {0, 0, 1}, {1,1,1,1}, {0.0f, 1.0f} },
        { {-w,  h,  d}, {0, 0, 1}, {1,1,1,1}, {1.0f, 0.0f} },
        { {-w, -h,  d}, {0, 0, 1}, {1,1,1,1}, {1.0f, 1.0f} },
        { { w, -h,  d}, {0, 0, 1}, {1,1,1,1}, {0.0f, 1.0f} },

        // 左面 -X
        { {-w,  h,  d}, {-1,0,0}, {1,1,1,1}, {0.0f, 0.0f} },
        { {-w,  h, -d}, {-1,0,0}, {1,1,1,1}, {1.0f, 0.0f} },
        { {-w, -h,  d}, {-1,0,0}, {1,1,1,1}, {0.0f, 1.0f} },
        { {-w,  h, -d}, {-1,0,0}, {1,1,1,1}, {1.0f, 0.0f} },
        { {-w, -h, -d}, {-1,0,0}, {1,1,1,1}, {1.0f, 1.0f} },
        { {-w, -h,  d}, {-1,0,0}, {1,1,1,1}, {0.0f, 1.0f} },

        // 右面 +X
        { { w,  h, -d}, {1,0,0}, {1,1,1,1}, {0.0f, 0.0f} },
        { { w,  h,  d}, {1,0,0}, {1,1,1,1}, {1.0f, 0.0f} },
        { { w, -h, -d}, {1,0,0}, {1,1,1,1}, {0.0f, 1.0f} },
        { { w,  h,  d}, {1,0,0}, {1,1,1,1}, {1.0f, 0.0f} },
        { { w, -h,  d}, {1,0,0}, {1,1,1,1}, {1.0f, 1.0f} },
        { { w, -h, -d}, {1,0,0}, {1,1,1,1}, {0.0f, 1.0f} },
    };

    // 🎮 使用与Box.cpp完全相同的索引数组
    static UINT indices[36] = {
        0, 1, 2,    // 顶面
        3, 4, 5,

        6, 7, 8,    // 底面
        9,10,11,

        12,13,14,   // 前面
        15,16,17,

        18,19,20,   // 背面
        21,22,23,

        24,25,26,   // 左面
        27,28,29,

        30,31,32,   // 右面
        33,34,35,
    };

    // 🔧 清理旧缓冲区
    if (g_PlatformVertexBuffer) {
        g_PlatformVertexBuffer->Release();
        g_PlatformVertexBuffer = nullptr;
    }
    if (g_PlatformIndexBuffer) {
        g_PlatformIndexBuffer->Release();
        g_PlatformIndexBuffer = nullptr;
    }

    // 创建顶点缓冲区 - 使用与Box.cpp相同的创建方式
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = vertices;

    HRESULT hr = GetDevice()->CreateBuffer(&bd, &sd, &g_PlatformVertexBuffer);
    if (FAILED(hr)) {
        OutputDebugStringA("❌ 错误：创建Box风格平台顶点缓冲区失败\n");
        return;
    }

    // 创建索引缓冲区
    bd.ByteWidth = sizeof(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    sd.pSysMem = indices;

    hr = GetDevice()->CreateBuffer(&bd, &sd, &g_PlatformIndexBuffer);
    if (FAILED(hr)) {
        OutputDebugStringA("❌ 错误：创建Box风格平台索引缓冲区失败\n");
        if (g_PlatformVertexBuffer) {
            g_PlatformVertexBuffer->Release();
            g_PlatformVertexBuffer = nullptr;
        }
        return;
    }

    // 🎮 设置索引数量 - 与Box.cpp保持一致
    g_PlatformIndexCount = 36;

    OutputDebugStringA("✅ Box风格方盒子平台创建成功！索引数量: 36\n");
}

void DrawPlatform(const CharacterPlatform& platform) {
    // 设置渲染状态
    SetDepthEnable(true);
    SetBlendState(false);
    SetCulingMode(D3D11_CULL_BACK);

    // 🔧 移除旋转动画，使用静态位置
    XMMATRIX worldMatrix = XMMatrixTranslation(
        platform.position.x,
        platform.position.y,
        platform.position.z
    );
    SetWorldMatrix(worldMatrix);

    // 检查缓冲区有效性
    if (!g_PlatformVertexBuffer || !g_PlatformIndexBuffer || g_PlatformIndexCount <= 0) {
        OutputDebugStringA("⚠️ 平台缓冲区无效，重新创建\n");
        CreatePlatformGeometry();
        if (!g_PlatformVertexBuffer || !g_PlatformIndexBuffer || g_PlatformIndexCount <= 0) {
            OutputDebugStringA("❌ 无法创建有效的平台\n");
            return;
        }
    }

    // 设置材质
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    material.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    material.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

    if (platform.isSelected) {
        // 选中时轻微发光
        material.Emission = XMFLOAT4(0.1f, 0.1f, 0.15f, 0.0f);
        material.Specular = XMFLOAT4(0.4f, 0.4f, 0.5f, 1.0f);
    }
    SetMaterial(material);

    // 绑定顶点和索引缓冲区
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &g_PlatformVertexBuffer, &stride, &offset);
    GetDeviceContext()->IASetIndexBuffer(g_PlatformIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 选择并绑定纹理
    ID3D11ShaderResourceView* platformTexture = GetCharacterPlatformTexture(platform.type);
    if (platformTexture) {
        GetDeviceContext()->PSSetShaderResources(0, 1, &platformTexture);
    }

    // 绘制平台
    GetDeviceContext()->DrawIndexed(g_PlatformIndexCount, 0, 0);

    // 清理纹理绑定
    ID3D11ShaderResourceView* nullSRV = nullptr;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
}
void ActivateCharacterScene() {
    g_CharacterSceneActive = true;

    // 🔧 修复：设置更好的摄像机位置观看角色
    Camera* camera = GetCamera();

    // 设置摄像机位置：从侧面稍高的角度观看角色展示台
    camera->Position = XMFLOAT3(0.0f, 12.0f, -25.0f);  // 更远一些，更高一些
    camera->AtPosition = XMFLOAT3(0.0f, 2.0f, 0.0f);   // 看向稍微高一点的位置
    camera->UpVector = XMFLOAT3(0.0f, 1.0f, 0.0f);

    // 🔧 修复：手动重新计算视图矩阵
    XMVECTOR eyePos = XMVectorSet(camera->Position.x, camera->Position.y, camera->Position.z, 1.0f);
    XMVECTOR focusPos = XMVectorSet(camera->AtPosition.x, camera->AtPosition.y, camera->AtPosition.z, 1.0f);
    XMVECTOR upVec = XMVectorSet(camera->UpVector.x, camera->UpVector.y, camera->UpVector.z, 0.0f);

    camera->ViewMatrix = XMMatrixLookAtLH(eyePos, focusPos, upVec);

    // 🔧 修复：确保投影矩阵正确
    camera->ProjectionMatrix = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(45.0f),  // 使用更标准的FOV
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.1f,   // 近裁剪面
        1000.0f // 远裁剪面
    );

    OutputDebugStringA("角色选择场景已激活，摄像机已重新设置\n");

}

// 停用角色选择场景
void DeactivateCharacterScene() {
    g_CharacterSceneActive = false;
    OutputDebugStringA("角色选择场景已停用\n");
}

void InitCharacterScene() {
    OutputDebugStringA("🎮 初始化新的角色选择场景\n");

    // 现有的平台设置代码...
    float spacing = 8.0f;
    float startX = -12.0f;

    for (int i = 0; i < 4; i++) {
        g_CharacterPlatforms[i].position = XMFLOAT3(startX + i * spacing, -0.3f, 0.0f);
        g_CharacterPlatforms[i].characterPos = XMFLOAT3(startX + i * spacing, 1.2f, 0.0f);
        g_CharacterPlatforms[i].type = (CHARACTER_TYPE)i;
        g_CharacterPlatforms[i].isSelected = (i == 0);
        g_CharacterPlatforms[i].scaleAnimation = (i == 0) ? 1.2f : 1.0f;

        // 平台颜色设置...
        switch (i) {
        case 0: g_CharacterPlatforms[i].platformColor = XMFLOAT4(0.9f, 0.95f, 1.0f, 1.0f); break;
        case 1: g_CharacterPlatforms[i].platformColor = XMFLOAT4(0.8f, 1.0f, 0.8f, 1.0f); break;
        case 2: g_CharacterPlatforms[i].platformColor = XMFLOAT4(1.0f, 0.9f, 0.7f, 1.0f); break;
        case 3: g_CharacterPlatforms[i].platformColor = XMFLOAT4(1.0f, 0.8f, 0.7f, 1.0f); break;
        }
    }

    g_SelectedCharacterIndex = 0;
    g_CharacterSceneActive = false;

    // 重置新的状态变量
    g_ShowingCharacterInfo = false;
    g_InfoCharacterIndex = -1;
    g_CharacterRotationAngle = 0.0f;
    g_InfoTextureAlpha = 0.0f;

    CreatePlatformGeometry();

    // 🔧 加载角色说明纹理（4张完整说明图）
    const wchar_t* infoTextureFiles[4] = {
        L"asset\\texture\\doctor.png",
        L"asset\\texture\\soldier.png",
        L"asset\\texture\\scout.png",
        L"asset\\texture\\bombTech.png"
    };

    TexMetadata metadata;
    ScratchImage image;
    int successCount = 0;

    for (int i = 0; i < 4; i++) {
        HRESULT hr = LoadFromWICFile(infoTextureFiles[i], WIC_FLAGS_NONE, &metadata, image);
        if (SUCCEEDED(hr)) {
            hr = CreateShaderResourceView(GetDevice(), image.GetImages(),
                image.GetImageCount(), metadata, &g_CharacterInfoTextures[i]);

            if (SUCCEEDED(hr)) {
                char debug[256];
                sprintf_s(debug, "✅ 角色说明纹理加载成功: %d\n", i);
                OutputDebugStringA(debug);
                successCount++;
            }
            else {
                char debug[256];
                sprintf_s(debug, "❌ 创建角色说明纹理失败: %d\n", i);
                OutputDebugStringA(debug);
            }
        }
        else {
            char debug[256];
            sprintf_s(debug, "⚠️ 角色说明纹理文件加载失败: %d\n", i);
            OutputDebugStringA(debug);
        }
    }
  

    
}
void UninitCharacterScene() {
    if (g_PlatformVertexBuffer) {
        g_PlatformVertexBuffer->Release();
        g_PlatformVertexBuffer = nullptr;
    }

    if (g_PlatformIndexBuffer) {
        g_PlatformIndexBuffer->Release();
        g_PlatformIndexBuffer = nullptr;
    }
    for (int i = 0; i < 4; i++) {
        if (g_CharacterInfoTextures[i]) {
            g_CharacterInfoTextures[i]->Release();
            g_CharacterInfoTextures[i] = nullptr;
        }
    }

    // 重置状态
    g_ShowingCharacterInfo = false;
    g_InfoCharacterIndex = -1;
    g_CharacterRotationAngle = 0.0f;
    g_InfoTextureAlpha = 0.0f;
}

void UpdateCharacterScene(float deltaTime) {
    if (!g_CharacterSceneActive) return;
   
    // 🔧 使用统一的战斗大厅检查方法
    bool isFromBattleHall = BattleHallManager::IsFromBattleHall();

    // 处理从战斗大厅返回的逻辑
    static int returnToBattleHallTimer = 0;
    static bool waitingToReturn = false;

    // 处理来自战斗大厅的角色选择确认
    static bool showingConfirmation = false;

   

    // 简单的键盘选择
    static bool lastF1 = false, lastF2 = false, lastF3 = false, lastF4 = false;
    bool nowF1 = Keyboard_IsKeyDown(KK_D1);
    bool nowF2 = Keyboard_IsKeyDown(KK_D2);
    bool nowF3 = Keyboard_IsKeyDown(KK_D3);
    bool nowF4 = Keyboard_IsKeyDown(KK_D4);

    // 🔧 按键选择并显示角色信息
    if (nowF1 && !lastF1) {
        SelectCharacter(0);
        ShowCharacterInfo(0);
        showingConfirmation = isFromBattleHall;
        OutputDebugStringA("🎮 按键1：选择并显示角色0\n");
    }
    if (nowF2 && !lastF2) {
        SelectCharacter(1);
        ShowCharacterInfo(1);
        showingConfirmation = isFromBattleHall;
        OutputDebugStringA("🎮 按键2：选择并显示角色1\n");
    }
    if (nowF3 && !lastF3) {
        SelectCharacter(2);
        ShowCharacterInfo(2);
        showingConfirmation = isFromBattleHall;
        OutputDebugStringA("🎮 按键3：选择并显示角色2\n");
    }
    if (nowF4 && !lastF4) {
        SelectCharacter(3);
        ShowCharacterInfo(3);
        showingConfirmation = isFromBattleHall;
        OutputDebugStringA("🎮 按键4：选择并显示角色3\n");
    }

    lastF1 = nowF1; lastF2 = nowF2; lastF3 = nowF3; lastF4 = nowF4;

    // 🔧 **关键修复**: ENTER键确认选择（只在来自战斗大厅时有效）
    static bool lastEnter = false;
    bool nowEnter = Keyboard_IsKeyDown(KK_ENTER);

    // 🔧 修复条件检查 - 确保逻辑一致
    if (nowEnter && !lastEnter && isFromBattleHall && g_SelectedCharacterIndex >= 0) {
        // 🔧 关键修复：添加调试信息
        char debug[256];
        sprintf_s(debug, "🔑 ENTER键被按下！来自战斗大厅=%s, 选择索引=%d\n",
            isFromBattleHall ? "true" : "false", g_SelectedCharacterIndex);
        OutputDebugStringA(debug);

        // 确认选择角色
        CHARACTER_TYPE selectedCharacter = (CHARACTER_TYPE)g_SelectedCharacterIndex;
        BattleHallManager::SetCurrentPlayerCharacter(selectedCharacter);

        // 🔧 手动清除标记（因为SetCurrentPlayerCharacter可能在最后才清除）
        BattleHallManager::SetFromBattleHallFlag(false);

        // 清除等待状态
        waitingToReturn = false;
        returnToBattleHallTimer = 60;

        // 立即返回战斗大厅
        DeactivateCharacterScene();
        SwitchToBattleHallMode();

        OutputDebugStringA("🏛️ ENTER键确认角色选择，返回战斗大厅\n");
        g_CharacterConfirmed = true;
        showingConfirmation = false;
        lastEnter = nowEnter;
        return;
    }
    lastEnter = nowEnter;

    // 🔧 **倒计时逻辑移到这里，只在确认后执行**
    if (waitingToReturn && returnToBattleHallTimer > 0) {
        returnToBattleHallTimer--;
        if (returnToBattleHallTimer <= 0) {
            waitingToReturn = false;
            DeactivateCharacterScene();
            SwitchToBattleHallMode();
            OutputDebugStringA("🏛️ 确认后自动返回战斗大厅\n");
            return;
        }
    }
    // 🔧 ESC键隐藏所有角色信息
    static bool lastEsc = false;
    bool nowEsc = Keyboard_IsKeyDown(KK_ESCAPE);
    if (nowEsc && !lastEsc) {
        if (isFromBattleHall) {
            BattleHallManager::SetFromBattleHallFlag(false); // 清除标记
            waitingToReturn = false;  // 🔧 清除等待状态
            returnToBattleHallTimer = 0;
            DeactivateCharacterScene();
            SwitchToBattleHallMode();
            OutputDebugStringA("🎮 ESC键：从角色选择返回战斗大厅\n");
        }
        else {
            for (int i = 0; i < 4; i++) {
                HideCharacterInfoCompletely(i);
            }
            OutputDebugStringA("🎮 ESC键：隐藏所有角色信息\n");
        }
    }
    lastEsc = nowEsc;

  

    // 更新角色动画
    UpdatePixelCharacters(deltaTime);

    // 更新缩放动画
    for (int i = 0; i < 4; i++) {
        float targetScale = g_CharacterPlatforms[i].isSelected ? 1.3f : 1.0f;
        g_CharacterPlatforms[i].scaleAnimation = Lerp(
            g_CharacterPlatforms[i].scaleAnimation,
            targetScale,
            deltaTime * 5.0f
        );
    }
}
void SelectCharacter(int index) {
    if (index < 0 || index >= 4) return;

    // 🔧 隐藏所有其他角色的信息
    for (int i = 0; i < 4; i++) {
        if (i != index) {
            HideCharacterInfo(i);
        }
    }

    // 重置所有选择状态
    for (int i = 0; i < 4; i++) {
        g_CharacterPlatforms[i].isSelected = false;
    }

    // 设置新选择
    g_CharacterPlatforms[index].isSelected = true;
    g_SelectedCharacterIndex = index;


    // 🔧 使用统一的检查方法
    bool isFromBattleHall = BattleHallManager::IsFromBattleHall();
    // 检查是否是从战斗大厅进入的
    if (isFromBattleHall) {
        CHARACTER_TYPE selectedCharacter = (CHARACTER_TYPE)index;

        // 🔧 关键修复：不要在这里立即设置角色，等待用户确认
        // 显示选择确认
        char debug[128];
        sprintf_s(debug, "✅ 从战斗大厅选择角色 %d，等待确认（ENTER或双击）\n", index);
        OutputDebugStringA(debug);

        // 不需要立即返回，让玩家看到选择效果并进行确认
        return;
    }
}

void DrawCharacterScene() {
    if (!g_CharacterSceneActive) return;

    // 设置3D渲染状态
    SetWorldViewProjection3D();
    SetDepthEnable(true);
    SetBlendState(false);
    SetCulingMode(D3D11_CULL_BACK);

    // 设置相机矩阵
    Camera* camera = GetCamera();
    SetViewMatrix(camera->ViewMatrix);
    SetProjectionMatrix(camera->ProjectionMatrix);

    // 设置光照
    extern LIGHT Light;
    SetLight(Light);

    // 🔧 获取deltaTime用于角色旋转动画
    extern float g_DeltaTime;

    // 绘制每个平台和角色
    for (int i = 0; i < 4; i++) {
        const CharacterPlatform& platform = g_CharacterPlatforms[i];

        // 绘制平台（已移除旋转）
        DrawPlatform(platform);

        // 绘制角色（支持旋转动画）
        DrawCharacterWithRotation(platform, i, g_DeltaTime);


        // 🔧 在角色前方绘制说明纹理
        DrawCharacterInfoAt3DPosition(platform, i, g_DeltaTime);
        
    }

  
    // 🔧 新增：在2D UI阶段添加确认提示
    SetWorldViewProjection2D();
    SetDepthEnable(false);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);

    // 🔧 使用统一的检查方法
    bool isFromBattleHall = BattleHallManager::IsFromBattleHall();

    // 如果来自战斗大厅且已选择角色，显示确认提示
    if (isFromBattleHall && g_SelectedCharacterIndex >= 0) {
        // 绘制确认提示背景
        XMMATRIX bgMatrix = XMMatrixTranslation(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 150, 0);
        SetWorldMatrix(bgMatrix);

        MATERIAL bgMaterial;
        ZeroMemory(&bgMaterial, sizeof(bgMaterial));
        bgMaterial.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.7f);
        SetMaterial(bgMaterial);

        DrawSprite(XMFLOAT2(SCREEN_WIDTH - 100, 100), XMFLOAT4(0.0f, 0.0f, 0.0f, 0.7f));

        // 绘制确认提示文字
        GeometricTextRenderer::DrawMinecraftText(
            SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT - 180,
            L"Press ENTER to confirm selection",
            XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),
            2.0f,
            false
        );

        GeometricTextRenderer::DrawMinecraftText(
            SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT - 150,
            L"or double-click character",
            XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
            1.5f,
            false
        );

        GeometricTextRenderer::DrawMinecraftText(
            SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 120,
            L"ESC to cancel",
            XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f),
            1.2f,
            false
        );
    }

    // 清理纹理绑定
    ID3D11ShaderResourceView* nullSRV = nullptr;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
}
CHARACTER_TYPE GetSelectedCharacterType() {
    return g_CharacterPlatforms[g_SelectedCharacterIndex].type;
}

// 判断角色选择场景是否激活
bool IsCharacterSceneActive() {
    return g_CharacterSceneActive;
}

ID3D11ShaderResourceView* GetCharacterPlatformTexture(CHARACTER_TYPE type) {
    switch (type) {
    case CHARACTER_DOCTOR:
        return GET_TEXTURE(ICE);      // 医生使用冰面纹理
    case CHARACTER_SOLDIER:
        return GET_TEXTURE(GRASS);    // 士兵使用草地纹理
    case CHARACTER_SCOUT:
        return GET_TEXTURE(SAND);     // 侦察兵使用沙地纹理
    case CHARACTER_BOMB_TECH:
        return GET_TEXTURE(WALL);     // 拆弹手使用墙体纹理
    default:
        return GET_TEXTURE(GROUND);   // 默认纹理
    }
}


// 🔧 修改：角色绘制，支持旋转动画
void DrawCharacterWithRotation(const CharacterPlatform& platform, int characterIndex, float deltaTime) {
    if (!GetPixelCharactersInitialized()) return;

    // 🔧 每个角色独立的旋转逻辑
    float targetRotation = 0.0f;
    if (g_CharacterInfoVisible[characterIndex]) {
        targetRotation = XM_PI;  // 180度，转到背面
    }

    // 平滑旋转到目标角度
    float rotationSpeed = 6.0f;  // 旋转速度
    float currentRotation = g_CharacterRotationAngles[characterIndex];

    if (currentRotation < targetRotation) {
        currentRotation += rotationSpeed * deltaTime;
        if (currentRotation > targetRotation) {
            currentRotation = targetRotation;
        }
    }
    else if (currentRotation > targetRotation) {
        currentRotation -= rotationSpeed * deltaTime;
        if (currentRotation < targetRotation) {
            currentRotation = targetRotation;
        }
    }

    g_CharacterRotationAngles[characterIndex] = currentRotation;

    // 🔧 计算角色世界矩阵
    float characterScale = platform.scaleAnimation * 1.8f;
    XMFLOAT3 adjustedCharacterPos = platform.characterPos;
    adjustedCharacterPos.y += 0.5f;

    XMMATRIX characterWorld =
        XMMatrixScaling(characterScale, characterScale, characterScale) *
        XMMatrixRotationY(currentRotation) *  // 使用角色独立的旋转角度
        XMMatrixTranslation(
            adjustedCharacterPos.x,
            adjustedCharacterPos.y,
            adjustedCharacterPos.z
        );

    // 根据角色类型绘制对应的像素角色
    switch (platform.type) {
    case CHARACTER_DOCTOR:
        if (GetPixelCharacterValid(CHARACTER_DOCTOR)) {
            DrawPixelCharacter(&g_Doctor, characterWorld);
        }
        break;
    case CHARACTER_SOLDIER:
        if (GetPixelCharacterValid(CHARACTER_SOLDIER)) {
            DrawPixelCharacter(&g_Soldier, characterWorld);
        }
        break;
    case CHARACTER_SCOUT:
        if (GetPixelCharacterValid(CHARACTER_SCOUT)) {
            DrawPixelCharacter(&g_Scout, characterWorld);
        }
        break;
    case CHARACTER_BOMB_TECH:
        if (GetPixelCharacterValid(CHARACTER_BOMB_TECH)) {
            DrawPixelCharacter(&g_BombTech, characterWorld);
        }
        break;
    }
}
//void DrawCharacterInfoTexture(int characterIndex) {
//    if (characterIndex < 0 || characterIndex >= 4) {
//        OutputDebugStringA("❌ DrawCharacterInfoTexture: 无效索引\n");
//        return;
//    }
//
//    if (!g_CharacterInfoTextures[characterIndex]) {
//        char debug[128];
//        sprintf_s(debug, "❌ DrawCharacterInfoTexture: 角色 %d 纹理为空\n", characterIndex);
//        OutputDebugStringA(debug);
//        return;
//    }
//
//    OutputDebugStringA("🎨 开始绘制角色说明纹理\n");
//
//    // 设置2D渲染状态
//    SetWorldViewProjection2D();
//    SetDepthEnable(false);
//    SetBlendState(true);
//    SetCulingMode(D3D11_CULL_NONE);
//
//    // 🔧 修复1：加快淡入速度，并设置最小显示alpha
//    float alpha = g_InfoTextureAlpha;
//    if (g_ShowingCharacterInfo && g_InfoCharacterIndex == characterIndex) {
//        // 快速淡入
//        alpha += 0.1f;  // 从0.05f改为0.1f，加快淡入
//        if (alpha > 1.0f) alpha = 1.0f;
//    }
//    else {
//        // 淡出
//        alpha -= 0.1f;
//        if (alpha < 0.0f) alpha = 0.0f;
//    }
//    g_InfoTextureAlpha = alpha;
//
//    // 🔧 修复2：降低显示阈值，并强制显示
//    if (g_ShowingCharacterInfo && g_InfoCharacterIndex == characterIndex) {
//        // 强制设置最小alpha值，确保能看到
//        if (alpha < 0.3f) alpha = 0.3f;
//
//        char debug[128];
//        sprintf_s(debug, "🎨 绘制角色 %d 纹理，Alpha=%.2f\n", characterIndex, alpha);
//        OutputDebugStringA(debug);
//
//        // 设置材质
//        MATERIAL material;
//        ZeroMemory(&material, sizeof(material));
//        material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, alpha);
//        material.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
//        material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
//        material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
//        SetMaterial(material);
//
//        // 🔧 修复3：调整显示位置和大小，确保在屏幕内
//        float infoX = SCREEN_WIDTH - 450;  // 右侧，留更多边距
//        float infoY = SCREEN_HEIGHT / 2;   // 垂直居中
//        float infoW = 400;                 // 宽度
//        float infoH = 500;                 // 高度
//
//        // 确保不超出屏幕
//        if (infoX < 0) infoX = 50;
//        if (infoY - infoH / 2 < 0) infoY = infoH / 2 + 50;
//
//        char posDebug[128];
//        sprintf_s(posDebug, "🎨 纹理位置: X=%.1f, Y=%.1f, W=%.1f, H=%.1f\n",
//            infoX, infoY, infoW, infoH);
//        OutputDebugStringA(posDebug);
//
//        XMMATRIX infoMatrix = XMMatrixTranslation(infoX, infoY, 0);
//        SetWorldMatrix(infoMatrix);
//
//        GetDeviceContext()->PSSetShaderResources(0, 1, &g_CharacterInfoTextures[characterIndex]);
//        DrawSprite(XMFLOAT2(infoW, infoH), XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));
//
//        OutputDebugStringA("✅ 纹理绘制命令已发送\n");
//
//        // 清理纹理绑定
//        ID3D11ShaderResourceView* nullSRV = nullptr;
//        GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
//    }
//    else {
//        OutputDebugStringA("⚠️ 不满足绘制条件，跳过绘制\n");
//    }
//}
// 🔧 新增：显示角色说明
void DrawCharacterInfoTexture(int characterIndex) {
    char entryDebug[128];
    sprintf_s(entryDebug, "🎨 DrawCharacterInfoTexture 调用: 请求角色=%d, 当前InfoIndex=%d\n",
        characterIndex, g_InfoCharacterIndex);
    OutputDebugStringA(entryDebug);

    if (characterIndex < 0 || characterIndex >= 4) {
        OutputDebugStringA("❌ DrawCharacterInfoTexture: 无效索引\n");
        return;
    }

    if (!g_CharacterInfoTextures[characterIndex]) {
        char debug[128];
        sprintf_s(debug, "❌ DrawCharacterInfoTexture: 角色 %d 纹理为空\n", characterIndex);
        OutputDebugStringA(debug);
        return;
    }

    // 🔧 状态检查
    if (!g_ShowingCharacterInfo) {
        OutputDebugStringA("⚠️ g_ShowingCharacterInfo = false，跳过绘制\n");
        return;
    }

    if (g_InfoCharacterIndex != characterIndex) {
        char debug[128];
        sprintf_s(debug, "⚠️ 索引不匹配: 请求=%d, 当前InfoIndex=%d，跳过绘制\n",
            characterIndex, g_InfoCharacterIndex);
        OutputDebugStringA(debug);
        return;
    }

    OutputDebugStringA("🎨 开始绘制角色说明纹理\n");

    // 设置2D渲染状态
    SetWorldViewProjection2D();
    SetDepthEnable(false);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);

    // 更新alpha值
    float alpha = g_InfoTextureAlpha;
    if (g_ShowingCharacterInfo && g_InfoCharacterIndex == characterIndex) {
        alpha += 0.05f;
        if (alpha > 1.0f) alpha = 1.0f;
    }
    g_InfoTextureAlpha = alpha;

    // 确保最小可见度
    if (alpha < 0.5f) alpha = 0.5f;

    char debug[128];
    sprintf_s(debug, "🎨 绘制角色 %d 纹理，Alpha=%.2f\n", characterIndex, alpha);
    OutputDebugStringA(debug);

    // 设置材质
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, alpha);
    material.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    SetMaterial(material);

    // 计算显示位置
    float infoX = SCREEN_WIDTH - 450;
    float infoY = SCREEN_HEIGHT / 2;
    float infoW = 400;
    float infoH = 500;

    char posDebug[128];
    sprintf_s(posDebug, "🎨 纹理位置: X=%.1f, Y=%.1f, W=%.1f, H=%.1f\n",
        infoX, infoY, infoW, infoH);
    OutputDebugStringA(posDebug);

    XMMATRIX infoMatrix = XMMatrixTranslation(infoX, infoY, 0);
    SetWorldMatrix(infoMatrix);

    GetDeviceContext()->PSSetShaderResources(0, 1, &g_CharacterInfoTextures[characterIndex]);
    DrawSprite(XMFLOAT2(infoW, infoH), XMFLOAT4(1.0f, 1.0f, 1.0f, alpha));

    OutputDebugStringA("✅ 纹理绘制命令已发送\n");

    // 清理纹理绑定
    ID3D11ShaderResourceView* nullSRV = nullptr;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
}
void ShowCharacterInfo(int characterIndex) {
    if (characterIndex < 0 || characterIndex >= 4) return;
    if (!g_CharacterInfoTextures[characterIndex]) return;

    // 🔧 首先隐藏所有其他角色的信息
    for (int i = 0; i < 4; i++) {
        if (i != characterIndex) {
            HideCharacterInfo(i);
        }
    }

    // 🔧 启用该角色的信息显示
    g_CharacterInfoVisible[characterIndex] = true;
    g_CharacterInfoAlphas[characterIndex] = 0.0f;  // 从完全透明开始渐入

    char debug[128];
    sprintf_s(debug, "✅ 显示角色 %d 的信息，隐藏其他角色\n", characterIndex);
    OutputDebugStringA(debug);
}
// 🔧 新函数：隐藏角色信息
void HideCharacterInfo(int characterIndex) {
    if (characterIndex < 0 || characterIndex >= 4) return;

    g_CharacterInfoVisible[characterIndex] = false;
    g_CharacterInfoAlphas[characterIndex] = 0.0f;
    // 🔧 注意：不要重置旋转角度，让角色慢慢转回来
    // g_CharacterRotationAngles[characterIndex] = 0.0f;  // 注释掉这行

    char debug[128];
    sprintf_s(debug, "🔧 隐藏角色 %d 的信息\n", characterIndex);
    OutputDebugStringA(debug);
}
// 🔧 修改 HideCharacterInfo 重载版本，用于完全重置角色状态
void HideCharacterInfoCompletely(int characterIndex) {
    if (characterIndex < 0 || characterIndex >= 4) return;

    g_CharacterInfoVisible[characterIndex] = false;
    g_CharacterInfoAlphas[characterIndex] = 0.0f;
    g_CharacterRotationAngles[characterIndex] = 0.0f;  // 重置旋转角度

    char debug[128];
    sprintf_s(debug, "🔧 完全重置角色 %d 的状态\n", characterIndex);
    OutputDebugStringA(debug);
}
//void DrawCharacterInfoAt3DPosition(const CharacterPlatform& platform, int characterIndex, float deltaTime) {
//    if (characterIndex < 0 || characterIndex >= 4) return;
//    if (!g_CharacterInfoTextures[characterIndex]) return;
//    if (!g_CharacterInfoVisible[characterIndex]) return;
//
//    // 🔧 更新纹理透明度（渐入效果）
//    float targetAlpha = 1.0f;
//    float currentAlpha = g_CharacterInfoAlphas[characterIndex];
//
//    float fadeSpeed = 3.0f;  // 渐入速度
//    if (currentAlpha < targetAlpha) {
//        currentAlpha += fadeSpeed * deltaTime;
//        if (currentAlpha > targetAlpha) {
//            currentAlpha = targetAlpha;
//        }
//    }
//
//    g_CharacterInfoAlphas[characterIndex] = currentAlpha;
//
//    // 只有当角色旋转到一定角度后才开始显示纹理
//    float rotationProgress = g_CharacterRotationAngles[characterIndex] / XM_PI;
//    if (rotationProgress < 0.5f) {
//        return;  // 角色还没转到一半，不显示纹理
//    }
//
//    // 🔧 计算纹理在角色正前方的3D位置
//    XMFLOAT3 texturePos = platform.characterPos;
//    texturePos.y += 0.5f;  // 稍微抬高
//    texturePos.z += 2.5f;  // 在角色前方
//
//    // 🔧 创建面向摄像机的纹理矩阵（Billboard效果）
//    Camera* camera = GetCamera();
//    XMVECTOR cameraPos = XMVectorSet(camera->Position.x, camera->Position.y, camera->Position.z, 1.0f);
//    XMVECTOR texturePosition = XMVectorSet(texturePos.x, texturePos.y, texturePos.z, 1.0f);
//
//    // 计算朝向摄像机的方向
//    XMVECTOR toCamera = XMVectorSubtract(cameraPos, texturePosition);
//    toCamera = XMVector3Normalize(toCamera);
//
//    // 创建朝向摄像机的旋转矩阵
//    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//    XMVECTOR right = XMVector3Cross(up, toCamera);
//    right = XMVector3Normalize(right);
//    up = XMVector3Cross(toCamera, right);
//
//    XMMATRIX billboardMatrix;
//    billboardMatrix.r[0] = right;
//    billboardMatrix.r[1] = up;
//    billboardMatrix.r[2] = toCamera;
//    billboardMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
//
//    // 🔧 设置纹理大小和变换
//    float textureScale = 2.0f;  // 纹理大小
//    XMMATRIX scaleMatrix = XMMatrixScaling(textureScale, textureScale, 1.0f);
//    XMMATRIX translateMatrix = XMMatrixTranslationFromVector(texturePosition);
//
//    XMMATRIX textureWorld = scaleMatrix * billboardMatrix * translateMatrix;
//
//    // 🔧 设置3D渲染状态用于纹理
//    SetDepthEnable(true);
//    SetBlendState(true);
//    SetCulingMode(D3D11_CULL_NONE);
//
//    // 设置材质
//    MATERIAL material;
//    ZeroMemory(&material, sizeof(material));
//    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, currentAlpha);
//    material.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
//    material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
//    material.Emission = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);  // 轻微发光
//    SetMaterial(material);
//
//    SetWorldMatrix(textureWorld);
//
//    // 绑定纹理并绘制
//    GetDeviceContext()->PSSetShaderResources(0, 1, &g_CharacterInfoTextures[characterIndex]);
//
//    // 🔧 使用现有的 DrawBillboard 函数绘制3D纹理
//    float textureWidth = 3.0f;   // 纹理宽度
//    float textureHeight = 4.0f;  // 纹理高度
//    XMFLOAT4 textureColor = XMFLOAT4(1.0f, 1.0f, 1.0f, currentAlpha);
//
//    DrawBillboard(XMFLOAT2(textureWidth, textureHeight), textureColor);
//
//
//    // 清理纹理绑定
//    ID3D11ShaderResourceView* nullSRV = nullptr;
//    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
//
//    char debug[128];
//    sprintf_s(debug, "🎨 在角色 %d 前方绘制纹理，Alpha=%.2f，旋转进度=%.2f\n",
//        characterIndex, currentAlpha, rotationProgress);
//    OutputDebugStringA(debug);
//}

void DrawCharacterInfoAt3DPosition(const CharacterPlatform& platform, int characterIndex, float deltaTime) {
    if (characterIndex < 0 || characterIndex >= 4) return;
    if (!g_CharacterInfoTextures[characterIndex]) return;
    if (!g_CharacterInfoVisible[characterIndex]) return;

    // 🔧 更新纹理透明度（渐入效果）
    float targetAlpha = 1.0f;
    float currentAlpha = g_CharacterInfoAlphas[characterIndex];

    float fadeSpeed = 3.0f;  // 渐入速度
    if (currentAlpha < targetAlpha) {
        currentAlpha += fadeSpeed * deltaTime;
        if (currentAlpha > targetAlpha) {
            currentAlpha = targetAlpha;
        }
    }

    g_CharacterInfoAlphas[characterIndex] = currentAlpha;

    // 只有当角色旋转到一定角度后才开始显示纹理
    float rotationProgress = g_CharacterRotationAngles[characterIndex] / XM_PI;
    if (rotationProgress < 0.5f) {
        return;  // 角色还没转到一半，不显示纹理
    }

    // 🔧 修复：计算纹理在角色正前方的位置
    XMFLOAT3 texturePos = platform.characterPos;
    texturePos.y += 0.5f;  // 稍微抬高

    // 🔧 关键修复：角色旋转180度后，前方应该是-Z方向
    float currentRotation = g_CharacterRotationAngles[characterIndex];

    // 根据角色当前旋转角度计算前方向
    float frontX = sinf(currentRotation) * 2.5f;  // 前方X偏移
    float frontZ = cosf(currentRotation) * 2.5f;  // 前方Z偏移

    texturePos.x += frontX;
    texturePos.z += frontZ;

    // 🔧 设置3D渲染状态用于纹理
    SetDepthEnable(true);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);

    // 设置材质
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, currentAlpha);
    material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    material.Emission = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);  // 轻微发光
    SetMaterial(material);

    // 🔧 修复：设置正确的世界矩阵
    // 纹理应该面向摄像机，但也要考虑角色的朝向
    Camera* camera = GetCamera();
    XMVECTOR cameraPos = XMVectorSet(camera->Position.x, camera->Position.y, camera->Position.z, 1.0f);
    XMVECTOR texturePosition = XMVectorSet(texturePos.x, texturePos.y, texturePos.z, 1.0f);

    // 计算朝向摄像机的方向
    XMVECTOR toCamera = XMVectorSubtract(cameraPos, texturePosition);
    toCamera = XMVector3Normalize(toCamera);

    // 🔧 修复镜像问题：确保正确的坐标系
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR right = XMVector3Cross(up, toCamera);  // 右 = 上 × 前
    right = XMVector3Normalize(right);
    up = XMVector3Cross(toCamera, right);           // 上 = 前 × 右

    // 🔧 创建正确的billboard矩阵
    XMMATRIX billboardMatrix;
    billboardMatrix.r[0] = XMVectorScale(right, -1.0f);  // 🔧 反转X轴修复镜像
    billboardMatrix.r[1] = up;
    billboardMatrix.r[2] = toCamera;
    billboardMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    // 设置纹理大小和变换
    float textureScale = 3.0f;  // 纹理大小
    XMMATRIX scaleMatrix = XMMatrixScaling(textureScale, textureScale, 1.0f);
    XMMATRIX translateMatrix = XMMatrixTranslation(texturePos.x, texturePos.y, texturePos.z);

    XMMATRIX textureWorld = scaleMatrix * billboardMatrix * translateMatrix;
    SetWorldMatrix(textureWorld);

    // 绑定纹理
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_CharacterInfoTextures[characterIndex]);

    // 🔧 使用现有的 DrawBillboard 函数绘制3D纹理
    float textureWidth = 4.5f;   // 纹理宽度
    float textureHeight = 6.0f;  // 纹理高度
    XMFLOAT4 textureColor = XMFLOAT4(1.0f, 1.0f, 1.0f, currentAlpha);

    DrawBillboard(XMFLOAT2(textureWidth, textureHeight), textureColor);

    // 清理纹理绑定
    ID3D11ShaderResourceView* nullSRV = nullptr;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);

}