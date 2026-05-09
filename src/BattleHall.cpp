#include "BattleHall.h"
#include "renderer.h"
#include "Camera.h"
#include "mouse.h"
#include "keyboard.h"
#include "UIManager.h"
#include "SceneManager.h"
#include "PixelCharacters.h"
#include "GeometricTextRenderer.h"
#include "MultiplayerBattle.h"
#include "MultiBattle/MultiplayerMap.h"
#include "FrameWork/TextureManager.h"
// 静态成员变量定义
bool BattleHallManager::s_isActive = false;
BATTLE_HALL_STATE BattleHallManager::s_hallState = HALL_WAITING_PLAYERS;
PlayerSlot BattleHallManager::s_playerSlots[4];
CHARACTER_TYPE BattleHallManager::s_currentPlayerCharacter = CHARACTER_DOCTOR;
bool BattleHallManager::s_hasSelectedCharacter = false;
int BattleHallManager::s_currentPlayerSlot = -1;

ID3D11Buffer* BattleHallManager::s_slotVertexBuffer = nullptr;
ID3D11Buffer* BattleHallManager::s_slotIndexBuffer = nullptr;
int BattleHallManager::s_slotIndexCount = 0;
// 全局标记管理
static bool s_fromBattleHallFlag = false;
bool BattleHallManager::s_pendingBattleStart = false;
int BattleHallManager::s_battleStartDelay = 0;

XMFLOAT3 BattleHallManager::s_previousCameraPos;
XMFLOAT3 BattleHallManager::s_previousCameraAt;
XMFLOAT3 BattleHallManager::s_previousCameraUp;



void BattleHallManager::Init() {
    OutputDebugStringA("🏛️ 初始化战斗大厅\n");

    s_isActive = false;
    s_hallState = HALL_WAITING_PLAYERS;
    s_hasSelectedCharacter = false;
    s_currentPlayerSlot = -1;

    // 初始化玩家位置
    InitializeSlotPositions();

    // 创建位置几何体
    CreateSlotGeometry();

    OutputDebugStringA("✅ 战斗大厅初始化完成\n");
}

void BattleHallManager::Uninit() {
    // 清理缓冲区
    if (s_slotVertexBuffer) {
        s_slotVertexBuffer->Release();
        s_slotVertexBuffer = nullptr;
    }

    if (s_slotIndexBuffer) {
        s_slotIndexBuffer->Release();
        s_slotIndexBuffer = nullptr;
    }

    s_isActive = false;
    OutputDebugStringA("🏛️ 战斗大厅已清理\n");
}

void BattleHallManager::InitializeSlotPositions() {
    // 设置4个玩家位置，呈矩形排列
    float spacing = 8.0f;
    float depth = 6.0f;

    // 玩家位置布局：
    // P1    P2
    // P3    P4
    XMFLOAT3 basePositions[4] = {
        XMFLOAT3(-spacing, 0.0f, -depth),   // 玩家1 - 左前
        XMFLOAT3(spacing, 0.0f, -depth),    // 玩家2 - 右前
        XMFLOAT3(-spacing, 0.0f, depth),    // 玩家3 - 左后
        XMFLOAT3(spacing, 0.0f, depth)      // 玩家4 - 右后
    };

    // 位置颜色
    XMFLOAT4 slotColors[4] = {
        XMFLOAT4(0.9f, 0.3f, 0.3f, 1.0f),  // 红色
        XMFLOAT4(0.3f, 0.3f, 0.9f, 1.0f),  // 蓝色
        XMFLOAT4(0.3f, 0.9f, 0.3f, 1.0f),  // 绿色
        XMFLOAT4(0.9f, 0.9f, 0.3f, 1.0f)   // 黄色
    };

    for (int i = 0; i < 4; i++) {
        s_playerSlots[i].state = SLOT_EMPTY;
        s_playerSlots[i].characterType = CHARACTER_DOCTOR;
        s_playerSlots[i].isReady = false;
        s_playerSlots[i].position = basePositions[i];
        s_playerSlots[i].characterPos = XMFLOAT3(
            basePositions[i].x,
            basePositions[i].y + 1.5f,
            basePositions[i].z
        );
        s_playerSlots[i].slotColor = slotColors[i];
        s_playerSlots[i].scaleAnimation = 1.0f;
        s_playerSlots[i].playerID = i + 1;
        sprintf_s(s_playerSlots[i].playerName, "Player %d", i + 1);
    }
}

void BattleHallManager::CreateSlotGeometry() {
    // 使用与平台相同的盒子几何体
    float w = 2.0f;  // 宽度
    float h = 0.3f;  // 高度
    float d = 2.0f;  // 深度

    static VERTEX_3D vertices[36] = {
        // 顶面 +Y
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

    static UINT indices[36] = {
        0, 1, 2,    3, 4, 5,    // 顶面
        6, 7, 8,    9,10,11,    // 底面
        12,13,14,   15,16,17,   // 前面
        18,19,20,   21,22,23,   // 背面
        24,25,26,   27,28,29,   // 左面
        30,31,32,   33,34,35,   // 右面
    };

    // 清理旧缓冲区
    if (s_slotVertexBuffer) {
        s_slotVertexBuffer->Release();
        s_slotVertexBuffer = nullptr;
    }
    if (s_slotIndexBuffer) {
        s_slotIndexBuffer->Release();
        s_slotIndexBuffer = nullptr;
    }

    // 创建顶点缓冲区
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = vertices;

    HRESULT hr = GetDevice()->CreateBuffer(&bd, &sd, &s_slotVertexBuffer);
    if (FAILED(hr)) {
        OutputDebugStringA("❌ 错误：创建战斗大厅位置顶点缓冲区失败\n");
        return;
    }

    // 创建索引缓冲区
    bd.ByteWidth = sizeof(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    sd.pSysMem = indices;

    hr = GetDevice()->CreateBuffer(&bd, &sd, &s_slotIndexBuffer);
    if (FAILED(hr)) {
        OutputDebugStringA("❌ 错误：创建战斗大厅位置索引缓冲区失败\n");
        if (s_slotVertexBuffer) {
            s_slotVertexBuffer->Release();
            s_slotVertexBuffer = nullptr;
        }
        return;
    }

    s_slotIndexCount = 36;
    OutputDebugStringA("✅ 战斗大厅位置几何体创建成功\n");
}

void BattleHallManager::ActivateBattleHall() {
    if (s_isActive) return;

    s_isActive = true;
    s_hallState = HALL_WAITING_PLAYERS;

    // 保存当前相机状态
    Camera* camera = GetCamera();
    s_previousCameraPos = camera->Position;
    s_previousCameraAt = camera->AtPosition;
    s_previousCameraUp = camera->UpVector;

    //强制停止任何正在进行的相机过渡
    SetCameraTransitioning(false);

    // 设置战斗大厅相机
    SetupBattleHallCamera();

    //强制更新相机矩阵，确保立即生效
    camera->ViewMatrix = XMMatrixLookAtLH(
        XMVectorSet(camera->Position.x, camera->Position.y, camera->Position.z, 1.0f),
        XMVectorSet(camera->AtPosition.x, camera->AtPosition.y, camera->AtPosition.z, 1.0f),
        XMVectorSet(camera->UpVector.x, camera->UpVector.y, camera->UpVector.z, 0.0f)
    );

    // 切换UI屏幕
    UIManager::SwitchToScreen(UI_SCREEN_BATTLE_LOBBY); // 临时使用游戏屏幕

    OutputDebugStringA("🏛️ 战斗大厅已激活\n");
}

void BattleHallManager::DeactivateBattleHall() {
    if (!s_isActive) return;

    s_isActive = false;

    // 恢复之前的相机状态
    RestorePreviousCamera();

    OutputDebugStringA("🏛️ 战斗大厅已停用\n");
}

void BattleHallManager::SetupBattleHallCamera() {
    Camera* camera = GetCamera();

    // 设置摄像机位置：从侧面稍高的角度观看4个位置
    camera->Position = XMFLOAT3(20.0f, 45.0f, -50.0f);
    camera->AtPosition = XMFLOAT3(0.0f, 0.0f, 0.0f);   // 看向中心
    camera->UpVector = XMFLOAT3(0.0f, 1.0f, 0.0f);

    // 重新计算视图矩阵
    XMVECTOR eyePos = XMVectorSet(camera->Position.x, camera->Position.y, camera->Position.z, 1.0f);
    XMVECTOR focusPos = XMVectorSet(camera->AtPosition.x, camera->AtPosition.y, camera->AtPosition.z, 1.0f);
    XMVECTOR upVec = XMVectorSet(camera->UpVector.x, camera->UpVector.y, camera->UpVector.z, 0.0f);

    camera->ViewMatrix = XMMatrixLookAtLH(eyePos, focusPos, upVec);

    // 设置投影矩阵
    camera->ProjectionMatrix = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(60.0f),
        (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        0.1f,
        1000.0f
    );
    // 🔧 强制更新相机的所有相关参数
    camera->AtPositionAngle = XMFLOAT3(0.0f, 0.0f, 0.0f);  // 重置角度参数
    char debug[256];
    sprintf_s(debug, "🎥 战斗大厅摄像机设置: 位置(%.1f,%.1f,%.1f) 目标(%.1f,%.1f,%.1f)\n",
        camera->Position.x, camera->Position.y, camera->Position.z,
        camera->AtPosition.x, camera->AtPosition.y, camera->AtPosition.z);
    OutputDebugStringA(debug);
}

void BattleHallManager::RestorePreviousCamera() {
    Camera* camera = GetCamera();
    camera->Position = s_previousCameraPos;
    camera->AtPosition = s_previousCameraAt;
    camera->UpVector = s_previousCameraUp;

    // 重新计算视图矩阵
    XMVECTOR eyePos = XMVectorSet(camera->Position.x, camera->Position.y, camera->Position.z, 1.0f);
    XMVECTOR focusPos = XMVectorSet(camera->AtPosition.x, camera->AtPosition.y, camera->AtPosition.z, 1.0f);
    XMVECTOR upVec = XMVectorSet(camera->UpVector.x, camera->UpVector.y, camera->UpVector.z, 0.0f);

    camera->ViewMatrix = XMMatrixLookAtLH(eyePos, focusPos, upVec);
}

void BattleHallManager::Update(float deltaTime) {
    if (!s_isActive) return;

    // 🔧 **移除延迟启动逻辑** - 直接在StartBattle中处理所有逻辑

    // 更新动画
    UpdateAnimations(deltaTime);
    HandaleKeyboardInput();
    HandleMouseInput();

    // 处理ESC键返回主菜单
    static bool lastEsc = false;
    bool nowEsc = Keyboard_IsKeyDown(KK_ESCAPE);
    if (nowEsc && !lastEsc) {
        DeactivateBattleHall();
        ReturnToMainMenu();
    }
    lastEsc = nowEsc;

    // 处理空格键准备/取消准备
    static bool lastSpace = false;
    bool nowSpace = Keyboard_IsKeyDown(KK_SPACE);
    if (nowSpace && !lastSpace) {
        TogglePlayerReady();
    }
    lastSpace = nowSpace;

    // 🔧 **简化的自动开始逻辑**
    static int autoStartTimer = 0;
    static bool countdownStarted = false;

    bool allReady = AllPlayersReady();
    bool canStart = CanStartBattle();

    // 🔧 **防止在启动过程中重新触发倒计时**
    if (s_hallState == HALL_STARTING_BATTLE) {
        return;  // 如果正在启动，就不要再处理倒计时了
    }

    if (allReady && canStart) {
        if (!countdownStarted) {
            autoStartTimer = 180; // 3秒倒计时
            countdownStarted = true;
            OutputDebugStringA("🚀 所有玩家准备完毕，启动3秒倒计时！\n");
        }

        if (autoStartTimer > 0) {
            autoStartTimer--;

            if (autoStartTimer % 30 == 0) {
                float secondsLeft = autoStartTimer / 60.0f;
                wchar_t countdownText[64];
                swprintf_s(countdownText, L"Starting battle in %.1f seconds...", secondsLeft);
                UIManager::ShowStatusInfo(countdownText,
                    XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), 0.6f);
            }
        }

        // 🔧 **直接调用StartBattle，不使用延迟机制**
        if (autoStartTimer <= 0 && countdownStarted) {
            OutputDebugStringA("🚀 倒计时结束，直接开始战斗！\n");
            StartBattle();
            countdownStarted = false;
            return;
        }
    }
    else {
        if (countdownStarted) {
            autoStartTimer = 0;
            countdownStarted = false;
            OutputDebugStringA("⏱️ 有玩家取消准备，停止倒计时\n");
        }
    }
}
void BattleHallManager::UpdateAnimations(float deltaTime) {
    // 更新位置缩放动画
    for (int i = 0; i < 4; i++) {
        float targetScale = 1.0f;

        // 如果位置被占用，轻微放大
        if (s_playerSlots[i].state != SLOT_EMPTY) {
            targetScale = 1.1f;
        }

        // 如果是当前玩家的位置，进一步放大
        if (i == s_currentPlayerSlot) {
            targetScale = 1.2f;
        }

        // 平滑过渡
        float lerpSpeed = 3.0f * deltaTime;
        s_playerSlots[i].scaleAnimation = Lerp(s_playerSlots[i].scaleAnimation, targetScale, lerpSpeed);
    }
}

void BattleHallManager::HandleMouseInput() {
    Mouse_State mouse;
    Mouse_GetState(&mouse);

    static bool wasLeftButtonDown = false;
    bool isLeftButtonDown = mouse.leftButton;

    if (!isLeftButtonDown && wasLeftButtonDown) {
        // 鼠标释放时检查点击的位置
        int clickedSlot = GetSlotUnderMouse(mouse.x, mouse.y);

        if (clickedSlot >= 0 && clickedSlot < 4) {
            SelectPlayerSlot(clickedSlot);
        }
    }

    wasLeftButtonDown = isLeftButtonDown;
}

void BattleHallManager::HandaleKeyboardInput()
{

    static bool lastKey1 = false, lastKey2 = false, lastKey3 = false, lastKey4 = false;
    bool nowKey1 = Keyboard_IsKeyDown(KK_D1);
    bool nowKey2 = Keyboard_IsKeyDown(KK_D2);
    bool nowKey3 = Keyboard_IsKeyDown(KK_D3);
    bool nowKey4 = Keyboard_IsKeyDown(KK_D4);

    // 数字键1-4选择位置
    if (nowKey1 && !lastKey1) {
        SelectPlayerSlotByKey(0);
        OutputDebugStringA("⌨️ 按键1：选择位置1\n");
    }
    if (nowKey2 && !lastKey2) {
        SelectPlayerSlotByKey(1);
        OutputDebugStringA("⌨️ 按键2：选择位置2\n");
    }
    if (nowKey3 && !lastKey3) {
        SelectPlayerSlotByKey(2);
        OutputDebugStringA("⌨️ 按键3：选择位置3\n");
    }
    if (nowKey4 && !lastKey4) {
        SelectPlayerSlotByKey(3);
        OutputDebugStringA("⌨️ 按键4：选择位置4\n");
    }

    lastKey1 = nowKey1; lastKey2 = nowKey2; lastKey3 = nowKey3; lastKey4 = nowKey4;
}

int BattleHallManager::GetSlotUnderMouse(int mouseX, int mouseY) {
    // 简化的鼠标位置检测
    // 将屏幕分为4个区域对应4个玩家位置

    RECT clientRect;
    GetClientRect(g_hWnd, &clientRect);
    float windowWidth = (float)(clientRect.right - clientRect.left);
    float windowHeight = (float)(clientRect.bottom - clientRect.top);

    // 转换鼠标坐标
    float normalizedX = (float)mouseX / windowWidth;
    float normalizedY = (float)mouseY / windowHeight;

    // 检查是否在有效区域内（屏幕中央区域）
    if (normalizedY < 0.3f || normalizedY > 0.8f) return -1;

    // 根据X坐标和Y坐标确定点击的位置
    int slotX = (normalizedX < 0.5f) ? 0 : 1;  // 左右
    int slotZ = (normalizedY < 0.6f) ? 0 : 1;  // 前后

    return slotZ * 2 + slotX;  // 转换为0-3的索引
}

bool BattleHallManager::SelectPlayerSlot(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= 4) return false;

    PlayerSlot& slot = s_playerSlots[slotIndex];

    // 如果点击的是空位置
    if (slot.state == SLOT_EMPTY) {
        // 如果当前玩家没有选择角色，进入角色选择界面
        if (!s_hasSelectedCharacter) {
            OutputDebugStringA("🎮 玩家未选择角色，进入角色选择界面\n");

            // 🔧 关键修复：记住要占用的位置
            s_currentPlayerSlot = slotIndex;

            // 🔧 设置一个全局标记，表示是从战斗大厅进入角色选择
            OutputDebugStringA("🔧 准备设置战斗大厅标记为 true\n");
            SetFromBattleHallFlag(true);

            // 🔧 立即验证标记是否设置成功
            bool flagCheck = IsFromBattleHall();
            char flagDebug[128];
            sprintf_s(flagDebug, "🔧 标记设置后验证: %s\n", flagCheck ? "true" : "false");
            OutputDebugStringA(flagDebug);

            // 暂时停用战斗大厅（但保持激活状态用于检查）
            s_isActive = false;  // 暂时设为false，但不调用DeactivateBattleHall()

            // 切换到角色选择模式
            SwitchToMode(APP_MODE_CHARACTER);
            ActivateCharacterScene();
            UIManager::SwitchToScreen(UI_SCREEN_CHARACTER);

            // 🔧 再次验证标记
            bool flagCheck2 = IsFromBattleHall();
            char flagDebug2[128];
            sprintf_s(flagDebug2, "🔧 场景切换后验证: %s\n", flagCheck2 ? "true" : "false");
            OutputDebugStringA(flagDebug2);

            return true;
        }
        else {
            // 如果已有角色，直接占用位置
            if (s_currentPlayerSlot >= 0) {
                // 清空之前的位置
                s_playerSlots[s_currentPlayerSlot].state = SLOT_EMPTY;
            }

            // 占用新位置
            slot.state = SLOT_OCCUPIED_LOCAL;
            slot.characterType = s_currentPlayerCharacter;
            slot.isReady = true;
            s_currentPlayerSlot = slotIndex;

            char debug[128];
            sprintf_s(debug, "🎮 玩家占用位置 %d，角色类型 %d\n", slotIndex, s_currentPlayerCharacter);
            OutputDebugStringA(debug);

            return true;
        }
    }
    // 如果点击的是当前玩家占用的位置，可以移除
    else if (slot.state == SLOT_OCCUPIED_LOCAL && slotIndex == s_currentPlayerSlot) {
        slot.state = SLOT_EMPTY;
        slot.isReady = false;
        s_currentPlayerSlot = -1;

        char debug[128];
        sprintf_s(debug, "🎮 玩家离开位置 %d\n", slotIndex);
        OutputDebugStringA(debug);

        return true;
    }

    return false;
}
void BattleHallManager::SetPlayerCharacter(int slotIndex, CHARACTER_TYPE character) {
    if (slotIndex < 0 || slotIndex >= 4) return;

    s_playerSlots[slotIndex].characterType = character;

    // 如果是当前玩家的位置，更新当前角色
    if (slotIndex == s_currentPlayerSlot) {
        s_currentPlayerCharacter = character;
        s_hasSelectedCharacter = true;
    }
}

void BattleHallManager::SetCurrentPlayerCharacter(CHARACTER_TYPE character) {
    s_currentPlayerCharacter = character;
    s_hasSelectedCharacter = true;


    //// 🔧 清除"来自战斗大厅"标记
    //s_fromBattleHallFlag = false;

    // 🔧 重新激活战斗大厅
    s_isActive = true;

    // 如果玩家之前预选了一个位置，现在自动占用它
    if (s_currentPlayerSlot >= 0 && s_currentPlayerSlot < 4) {
        PlayerSlot& slot = s_playerSlots[s_currentPlayerSlot];
        slot.state = SLOT_OCCUPIED_LOCAL;
        slot.characterType = character;
        slot.isReady = true;

        char debug[128];
        sprintf_s(debug, "🎮 角色选择完成，自动占用位置 %d，角色类型 %d\n", s_currentPlayerSlot, character);
        OutputDebugStringA(debug);
    }

    s_fromBattleHallFlag = false;
}

bool BattleHallManager::CanStartBattle() {
    // 检查是否至少有2个玩家
    int occupiedSlots = 0;
    for (int i = 0; i < 4; i++) {
        if (s_playerSlots[i].state == SLOT_OCCUPIED_LOCAL ||
            s_playerSlots[i].state == SLOT_OCCUPIED_REMOTE) { // 🔧 包含AI玩家
            occupiedSlots++;
        }
    }
    return occupiedSlots >= 2;
}


void BattleHallManager::StartBattle() {
    OutputDebugStringA("🚀 MultiplayerBattleManager::StartBattle() 被调用\n");

    if (!CanStartBattle()) {
        OutputDebugStringA("❌ CanStartBattle() 返回 false\n");
        UIManager::ShowStatusInfo(L"至少需要2个玩家才能开始战斗！",
            XMFLOAT4(1.0f, 0.3f, 0.3f, 1.0f), 3.0f);
        return;
    }

    if (s_hallState == HALL_STARTING_BATTLE) {
        OutputDebugStringA("⚠️ 战斗已经在启动中，忽略重复调用\n");
        return;
    }

    OutputDebugStringA("✅ CanStartBattle() 通过，开始战斗流程\n");

    s_hallState = HALL_STARTING_BATTLE;

    UIManager::ShowStatusInfo(L"正在合并地图...",
        XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), 2.0f);

    OutputDebugStringA("🔧 初始化多人战斗管理器\n");
    MultiplayerBattleManager::Init();

    int activePlayers = 0;
    for (int i = 0; i < 4; i++) {
        if (s_playerSlots[i].state != SLOT_EMPTY) {
            OutputDebugStringA("🎮 设置玩家到多人战斗管理器\n");
            MultiplayerBattleManager::SetPlayerActive((PlayerID)i, true, s_playerSlots[i].playerName);
            MultiplayerBattleManager::LoadPlayerMapFromSlot((PlayerID)i, 0);
            activePlayers++;

            char debug[128];
            sprintf_s(debug, "🎮 玩家 %d 已设置：%s\n", i, s_playerSlots[i].playerName);
            OutputDebugStringA(debug);
        }
    }

    if (activePlayers < 2) {
        OutputDebugStringA("❌ 错误：活跃玩家数量不足\n");
        s_hallState = HALL_WAITING_PLAYERS; // 重置状态
        return;
    }

    OutputDebugStringA("🗺️ 开始地图合并...\n");
    BattlePlayerInfo* players = new BattlePlayerInfo[MAX_PLAYERS];
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i] = *MultiplayerBattleManager::GetPlayer((PlayerID)i);
    }

    bool mergeSuccess = MultiplayerBattleMap::MergeMapsForBattle(players, activePlayers);
    delete[] players;

    if (!mergeSuccess) { 
        OutputDebugStringA("❌ 地图合并失败，取消战斗\n");
        UIManager::ShowStatusInfo(L"地图合并失败！",
            XMFLOAT4(1.0f, 0.3f, 0.3f, 1.0f), 3.0f);
        s_hallState = HALL_WAITING_PLAYERS; // 重置状态
        return;
    }

    OutputDebugStringA("✅ 地图合并成功，继续启动战斗\n");

    // 停用战斗大厅
    DeactivateBattleHall();

    // 启动多人战斗
    MultiplayerBattleManager::StartBattle();

    OutputDebugStringA("🎉 战斗成功启动！\n");
}
void BattleHallManager::Draw() {
    if (!s_isActive) return;

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

    // 绘制每个玩家位置
    for (int i = 0; i < 4; i++) {
        DrawPlayerSlot(s_playerSlots[i]);
    }

    // 绘制UI
    DrawSlotUI();

    // 清理纹理绑定
    ID3D11ShaderResourceView* nullSRV = nullptr;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
}

void BattleHallManager::DrawPlayerSlot(const PlayerSlot& slot) {
    // 设置渲染状态
    SetDepthEnable(true);
    SetBlendState(false);
    SetCulingMode(D3D11_CULL_BACK);

    // 计算世界矩阵（包含缩放动画）
    XMMATRIX scaleMatrix = XMMatrixScaling(slot.scaleAnimation, slot.scaleAnimation, slot.scaleAnimation);
    XMMATRIX worldMatrix = scaleMatrix * XMMatrixTranslation(slot.position.x, slot.position.y, slot.position.z);
    SetWorldMatrix(worldMatrix);

    // 检查缓冲区有效性
    if (!s_slotVertexBuffer || !s_slotIndexBuffer || s_slotIndexCount <= 0) {
        OutputDebugStringA("⚠️ 战斗大厅位置缓冲区无效\n");
        return;
    }

    // 设置材质
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = slot.slotColor;
    material.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    material.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);

    // 如果位置被占用，添加发光效果
    if (slot.state != SLOT_EMPTY) {
        material.Emission = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
        material.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    }

    SetMaterial(material);

    // 绑定顶点和索引缓冲区
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &s_slotVertexBuffer, &stride, &offset);
    GetDeviceContext()->IASetIndexBuffer(s_slotIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11ShaderResourceView* slotTexture = nullptr;
    switch (slot.state) {
    case SLOT_EMPTY:
        slotTexture = GET_TEXTURE(GRID);
        break;
    case SLOT_OCCUPIED_LOCAL:
        // 根据角色类型选择纹理
        switch (slot.characterType) {
        case CHARACTER_DOCTOR: slotTexture = GET_TEXTURE(ICE); break;
        case CHARACTER_SOLDIER: slotTexture = GET_TEXTURE(GRASS); break;
        case CHARACTER_SCOUT: slotTexture = GET_TEXTURE(SAND); break;
        case CHARACTER_BOMB_TECH: slotTexture = GET_TEXTURE(WALL); break;
        default: slotTexture = GET_TEXTURE(GROUND); break;
        }
        break;
    case SLOT_OCCUPIED_REMOTE:
        slotTexture = GET_TEXTURE(GROUND);
        break;
    default:
        slotTexture = GET_TEXTURE(GRID);
        break;
    }

    if (slotTexture) {
        GetDeviceContext()->PSSetShaderResources(0, 1, &slotTexture);
    }

    // 绘制位置平台
    GetDeviceContext()->DrawIndexed(s_slotIndexCount, 0, 0);

    // 如果位置被占用，绘制角色
    if (slot.state != SLOT_EMPTY && GetPixelCharactersInitialized()) {
        // 计算角色世界矩阵
        float characterScale = slot.scaleAnimation * 1.5f;
        XMMATRIX characterWorld =
            XMMatrixScaling(characterScale, characterScale, characterScale) *
            XMMatrixTranslation(slot.characterPos.x, slot.characterPos.y, slot.characterPos.z);

        // 根据角色类型绘制对应的像素角色
        switch (slot.characterType) {
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

    // 清理纹理绑定
    ID3D11ShaderResourceView* nullSRV = nullptr;
    GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
}

void BattleHallManager::DrawSlotUI() {
    // 设置2D渲染状态
    SetWorldViewProjection2D();
    SetDepthEnable(false);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);

    // 绘制标题
    GeometricTextRenderer::DrawMinecraftText(
        SCREEN_WIDTH / 2 - 150, 50,
        L"BATTLE HALL",
        XMFLOAT4(1.0f, 1.0f, 0.5f, 1.0f),
        3.0f,
        false
    );

    // 🔧 更新说明文字，包含键盘控制
    GeometricTextRenderer::DrawMinecraftText(
        SCREEN_WIDTH / 2 - 300, 100,
        L"Press 1-4 to select slot, SPACE to ready/unready, ESC to exit",
        XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f),
        1.5f,
        false
    );

    // 🔧 显示当前玩家状态
    if (s_hasSelectedCharacter) {
        const wchar_t* characterNames[] = { L"Doctor", L"Soldier", L"Scout", L"Bomb Tech" };
        wchar_t playerStatus[256];

        if (s_currentPlayerSlot >= 0) {
            bool isReady = s_playerSlots[s_currentPlayerSlot].isReady;
            swprintf_s(playerStatus, L"You: %s at Slot %d [%s]",
                characterNames[s_currentPlayerCharacter],
                s_currentPlayerSlot + 1,
                isReady ? L"READY" : L"NOT READY");
        }
        else {
            swprintf_s(playerStatus, L"You: %s [No Slot Selected]",
                characterNames[s_currentPlayerCharacter]);
        }

        GeometricTextRenderer::DrawMinecraftText(
            50.0f, 130.0f,
            playerStatus,
            XMFLOAT4(1.0f, 1.0f, 0.3f, 1.0f),
            1.3f,
            false
        );
    }

    // 绘制玩家位置信息
    float startY = 160.0f;
    for (int i = 0; i < 4; i++) {
        const PlayerSlot& slot = s_playerSlots[i];
        float yPos = startY + i * 35.0f;

        wchar_t slotInfo[128];
        if (slot.state == SLOT_EMPTY) {
            swprintf_s(slotInfo, L"Slot %d: [EMPTY] - Press %d to join", i + 1, i + 1);
        }
        else {
            const wchar_t* characterNames[] = { L"Doctor", L"Soldier", L"Scout", L"Bomb Tech" };
            const wchar_t* readyStatus = slot.isReady ? L"READY" : L"NOT READY";

            // 🔧 标记当前玩家的位置
            if (i == s_currentPlayerSlot) {
                swprintf_s(slotInfo, L"Slot %d: %s [%s] ← YOU",
                    i + 1, characterNames[slot.characterType], readyStatus);
            }
            else {
                swprintf_s(slotInfo, L"Slot %d: %s [%s]",
                    i + 1, characterNames[slot.characterType], readyStatus);
            }
        }

        // 🔧 根据状态设置颜色
        XMFLOAT4 textColor;
        if (slot.state == SLOT_EMPTY) {
            textColor = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);  // 灰色
        }
        else if (slot.isReady) {
            textColor = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);  // 绿色 - 准备好
        }
        else {
            textColor = XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f);  // 橙色 - 未准备
        }

        GeometricTextRenderer::DrawMinecraftText(
            50.0f, yPos,
            slotInfo,
            textColor,
            1.2f,
            false
        );
    }

    // 🔧 根据情况显示不同的行动提示
    float bottomY = SCREEN_HEIGHT - 120.0f;

    if (s_currentPlayerSlot >= 0) {
        // 玩家已选择位置
        bool isReady = s_playerSlots[s_currentPlayerSlot].isReady;
        if (!isReady) {
            GeometricTextRenderer::DrawMinecraftText(
                SCREEN_WIDTH / 2 - 150, bottomY,
                L"Press SPACE to ready up!",
                XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
                1.8f,
                false
            );
        }
        else {
            GeometricTextRenderer::DrawMinecraftText(
                SCREEN_WIDTH / 2 - 200, bottomY,
                L"Waiting for other players... (SPACE to unready)",
                XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),
                1.5f,
                false
            );
        }
    }
    else if (s_hasSelectedCharacter) {
        // 有角色但没选位置
        GeometricTextRenderer::DrawMinecraftText(
            SCREEN_WIDTH / 2 - 150, bottomY,
            L"Press 1-4 to select a slot!",
            XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),
            1.8f,
            false
        );
    }
    else {
        // 没有角色
        GeometricTextRenderer::DrawMinecraftText(
            SCREEN_WIDTH / 2 - 200, bottomY,
            L"Press 1-4 to select slot and choose character",
            XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
            1.5f,
            false
        );
    }

    // 🔧 显示自动开始战斗的状态
    if (AllPlayersReady()) {
        GeometricTextRenderer::DrawMinecraftText(
            SCREEN_WIDTH / 2 - 100, bottomY + 40,
            L"Starting battle...",
            XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
            2.0f,
            false
        );
    }
    else if (CanStartBattle()) {
        int readyCount = 0;
        int totalCount = 0;
        for (int i = 0; i < 4; i++) {
            if (s_playerSlots[i].state != SLOT_EMPTY) {
                totalCount++;
                if (s_playerSlots[i].isReady) readyCount++;
            }
        }

        wchar_t statusText[64];
        swprintf_s(statusText, L"Ready: %d/%d players", readyCount, totalCount);
        GeometricTextRenderer::DrawMinecraftText(
            SCREEN_WIDTH / 2 - 100, bottomY + 40,
            statusText,
            XMFLOAT4(0.7f, 0.7f, 1.0f, 1.0f),
            1.3f,
            false
        );
    }

    // 绘制控制说明
    GeometricTextRenderer::DrawMinecraftText(
        SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 50,
        L"ESC: Return to Main Menu",
        XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f),
        1.0f,
        false
    );
}

void BattleHallManager::SetFromBattleHallFlag(bool flag) {
    s_fromBattleHallFlag = flag;
    char debug[128];
    sprintf_s(debug, "🏛️ 战斗大厅标记设置: %s (地址: %p)\n",
        flag ? "true" : "false", &s_fromBattleHallFlag);
    OutputDebugStringA(debug);
}

bool BattleHallManager::IsFromBattleHall() {
    char debug[128];
    sprintf_s(debug, "🏛️ 战斗大厅标记查询: %s (地址: %p)\n",
        s_fromBattleHallFlag ? "true" : "false", &s_fromBattleHallFlag);
    OutputDebugStringA(debug);
    return s_fromBattleHallFlag;
}// 🔧 新增：通过键盘选择位置
bool BattleHallManager::SelectPlayerSlotByKey(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= 4) return false;

    PlayerSlot& slot = s_playerSlots[slotIndex];

    // 🔧 如果当前玩家没有角色，先进入角色选择
    if (!s_hasSelectedCharacter) {
        OutputDebugStringA("🎮 玩家未选择角色，进入角色选择界面\n");

        s_currentPlayerSlot = slotIndex;
        SetFromBattleHallFlag(true);
        s_isActive = false;

        SwitchToMode(APP_MODE_CHARACTER);
        ActivateCharacterScene();
        UIManager::SwitchToScreen(UI_SCREEN_CHARACTER);
        return true;
    }

    // 🔧 如果点击的是空位置，占用它
    if (slot.state == SLOT_EMPTY) {
        // 清空之前的位置
        if (s_currentPlayerSlot >= 0) {
            s_playerSlots[s_currentPlayerSlot].state = SLOT_EMPTY;
            s_playerSlots[s_currentPlayerSlot].isReady = false;
        }

        // 占用新位置
        slot.state = SLOT_OCCUPIED_LOCAL;
        slot.characterType = s_currentPlayerCharacter;
        slot.isReady = false;  // 🔧 新占用的位置默认未准备
        s_currentPlayerSlot = slotIndex;

        char debug[128];
        sprintf_s(debug, "⌨️ 玩家占用位置 %d，角色类型 %d\n", slotIndex, s_currentPlayerCharacter);
        OutputDebugStringA(debug);
        return true;
    }
    // 🔧 如果点击的是当前玩家占用的位置，离开该位置
    else if (slot.state == SLOT_OCCUPIED_LOCAL && slotIndex == s_currentPlayerSlot) {
        slot.state = SLOT_EMPTY;
        slot.isReady = false;
        s_currentPlayerSlot = -1;

        char debug[128];
        sprintf_s(debug, "⌨️ 玩家离开位置 %d\n", slotIndex);
        OutputDebugStringA(debug);
        return true;
    }

    return false;
}

// 切换当前玩家的准备状态
void BattleHallManager::TogglePlayerReady() {
    if (s_currentPlayerSlot < 0 || s_currentPlayerSlot >= 4) {
        OutputDebugStringA("⚠️ 玩家未占用任何位置，无法准备\n");
        return;
    }

    PlayerSlot& slot = s_playerSlots[s_currentPlayerSlot];
    if (slot.state != SLOT_OCCUPIED_LOCAL) {
        OutputDebugStringA("⚠️ 当前位置状态异常\n");
        return;
    }

    // 切换准备状态
    slot.isReady = !slot.isReady;

    char debug[128];
    sprintf_s(debug, "🎯 玩家位置 %d %s\n",
        s_currentPlayerSlot,
        slot.isReady ? "已准备" : "取消准备");
    OutputDebugStringA(debug);

    if (slot.isReady) {
        UIManager::ShowStatusInfo(L"已准备！正在添加AI玩家...",
            XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), 2.0f);

        // 🔧 修复：先填充AI玩家
        AutoFillAIPlayers();

        // 🔧 修复：检查并显示状态
        if (AllPlayersReady() && CanStartBattle()) {
            UIManager::ShowStatusInfo(L"所有玩家准备完毕！准备开始战斗...",
                XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), 3.0f);
            OutputDebugStringA("🚀 所有玩家准备完毕！\n");
        }
        else {
            OutputDebugStringA("⚠️ 还有玩家未准备或条件不满足\n");

            // 调试：检查每个玩家的状态
            for (int i = 0; i < 4; i++) {
                char slotDebug[128];
                sprintf_s(slotDebug, "位置 %d: 状态=%d, 准备=%s\n",
                    i, s_playerSlots[i].state,
                    s_playerSlots[i].isReady ? "是" : "否");
                OutputDebugStringA(slotDebug);
            }
        }
    }
    else {
        UIManager::ShowStatusInfo(L"取消准备",
            XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f), 2.0f);

        // 取消准备时移除AI玩家
        RemoveAIPlayers();
    }
}
// 检查是否所有玩家都准备好
bool BattleHallManager::AllPlayersReady() {
    int occupiedCount = 0;
    int readyCount = 0;

    for (int i = 0; i < 4; i++) {
        if (s_playerSlots[i].state == SLOT_OCCUPIED_LOCAL ||
            s_playerSlots[i].state == SLOT_OCCUPIED_REMOTE) { // 🔧 包含AI玩家
            occupiedCount++;
            if (s_playerSlots[i].isReady) {
                readyCount++;
            }
        }
    }

    // 至少2个玩家，且所有占用位置的玩家都准备好
    return (occupiedCount >= 2) && (occupiedCount == readyCount);
}
// 🔧 新增：自动填充AI玩家
void BattleHallManager::AutoFillAIPlayers() {
    OutputDebugStringA("🤖 开始填充AI玩家到空位置...\n");

    // AI角色类型轮换
    CHARACTER_TYPE aiCharacters[] = {
        CHARACTER_SOLDIER,   // AI玩家1：士兵
        CHARACTER_SCOUT,     // AI玩家2：侦察兵  
        CHARACTER_BOMB_TECH, // AI玩家3：拆弹手
        CHARACTER_DOCTOR     // AI玩家4：医生（如果需要第4个）
    };

    int aiIndex = 0;
    int aiCount = 0;

    for (int i = 0; i < 4; i++) {
        PlayerSlot& slot = s_playerSlots[i];

        // 跳过已被真实玩家占用的位置
        if (slot.state == SLOT_OCCUPIED_LOCAL) {
            continue;
        }

        // 填充空位置为AI玩家
        if (slot.state == SLOT_EMPTY) {
            slot.state = SLOT_OCCUPIED_REMOTE;  // 使用REMOTE标记AI玩家
            slot.characterType = aiCharacters[aiIndex % 4];
            slot.isReady = true;  // AI玩家立即准备
            sprintf_s(slot.playerName, "AI Player %d", i + 1);

            char debug[128];
            sprintf_s(debug, "🤖 AI玩家已加入位置 %d，角色: %d\n", i, slot.characterType);
            OutputDebugStringA(debug);

            aiIndex++;
            aiCount++;

            // 🔧 最多添加3个AI，确保总共4个玩家
            if (aiCount >= 3) break;
        }
    }

    char summary[128];
    sprintf_s(summary, "🤖 AI填充完成！添加了 %d 个AI玩家\n", aiCount);
    OutputDebugStringA(summary);

    // 🔧 检查是否可以开始战斗
    if (AllPlayersReady()) {
        OutputDebugStringA("🚀 所有玩家（包括AI）准备完毕！\n");
        // 延迟1.5秒后自动开始，给玩家看到AI加入的效果
        ScheduleAutoStart(90); // 90帧 ≈ 1.5秒
    }
}
// 🔧 新增：移除AI玩家
void BattleHallManager::RemoveAIPlayers() {
    OutputDebugStringA("🤖 移除所有AI玩家...\n");

    int removedCount = 0;
    for (int i = 0; i < 4; i++) {
        PlayerSlot& slot = s_playerSlots[i];

        // 只移除AI玩家（REMOTE标记的）
        if (slot.state == SLOT_OCCUPIED_REMOTE) {
            slot.state = SLOT_EMPTY;
            slot.isReady = false;
            slot.characterType = CHARACTER_DOCTOR; // 重置为默认
            sprintf_s(slot.playerName, "Player %d", i + 1);
            removedCount++;

            char debug[128];
            sprintf_s(debug, "🤖 已移除位置 %d 的AI玩家\n", i);
            OutputDebugStringA(debug);
        }
    }

    char summary[128];
    sprintf_s(summary, "🤖 AI移除完成！移除了 %d 个AI玩家\n", removedCount);
    OutputDebugStringA(summary);

    // 取消自动开始
    CancelAutoStart();
}
// 🔧 新增：计划自动开始战斗
void BattleHallManager::ScheduleAutoStart(int frames) {
    static int autoStartTimer = 0;
    autoStartTimer = frames;

    char debug[128];
    sprintf_s(debug, "⏱️ 计划在 %d 帧后自动开始战斗\n", frames);
    OutputDebugStringA(debug);
}

// 🔧 新增：取消自动开始
void BattleHallManager::CancelAutoStart() {
    static int autoStartTimer = 0;
    autoStartTimer = 0;
    OutputDebugStringA("⏱️ 已取消自动开始战斗\n");
}
// 外部函数实现
void InitBattleHall() {
    BattleHallManager::Init();
}

void UninitBattleHall() {
    BattleHallManager::Uninit();
}

void UpdateBattleHall(float deltaTime) {
    BattleHallManager::Update(deltaTime);
}

void DrawBattleHall() {
    BattleHallManager::Draw();
}

bool IsBattleHallActive() {
    return BattleHallManager::IsActive();
}

void ActivateBattleHall() {
    BattleHallManager::ActivateBattleHall();
}

void DeactivateBattleHall() {
    BattleHallManager::DeactivateBattleHall();
}