//Camera.cpp
// Camera.cpp 升级版：支持斜视视角，设置为你给定的推荐参数

#include	"Camera.h"
#include	"keyboard.h"
#include	"mouse.h"
#include    "Box.h"
#include    <algorithm>
#include    "MapEditor.h"
#include    "SceneManager.h"
//CameraMode g_CameraMode = CAMERA_MODE_PREVIEW;

extern ID3D11ShaderResourceView* g_TextureIce;    // 冰面纹理
extern ID3D11ShaderResourceView* g_TextureSand;   // 沙地纹理
extern ID3D11ShaderResourceView* g_TextureGrass;  // 草地纹理
extern ID3D11ShaderResourceView* g_TextureWater;  // 水面纹理
extern ID3D11ShaderResourceView* g_TextureWall;   // 墙体纹理
extern ID3D11ShaderResourceView* g_TextureGrid;   // 墙体纹理

extern ObstacleUIBox g_ObstacleUIBoxes[OBSTACLE_UI_BOX_COUNT];

static bool isLerpingCameraAngle = false;
extern UIBox g_UIBoxes[UI_BOX_COUNT];
extern int  g_SelectedUIBoxIndex;
extern HWND g_MainWnd;
HWND g_hWnd = NULL;
int g_HoverX;
int g_HoverZ;

static struct CameraPreset {
    XMFLOAT3 position;     // 摄像机位置
    XMFLOAT3 lookAt;       // 注视点
    XMFLOAT3 up;           // 上向量
};
static CameraPreset g_PreviewPreset, g_EditPreset;

//===============
//鼠标拖拽
static bool g_IsDraggingMap = false;
static int g_LastEditedX = -1;
static int g_LastEditedZ = -1;

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}



Camera	g_Camera;
Mouse_State	Mouse;
Camera* GetCamera() { return &g_Camera; }

//-----------------------------
//摄像机路径管理
//=============================
// 在Camera.cpp中添加以下代码来管理基于路径的摄像机过渡

// 定义摄像机路径关键点结构
struct CameraPathPoint {
    XMFLOAT3 position;     // 摄像机位置
    XMFLOAT3 lookAt;       // 注视点
    XMFLOAT3 up;           // 上向量
};

// 为每种转换定义路径关键点
static CameraPathPoint g_PreviewToEditPath[4] = {
    // 起点 - 预览模式位置（将在运行时填充实际值）
    { XMFLOAT3(0,0,0), XMFLOAT3(0,0,0), XMFLOAT3(0,1,0) },

    // 中间点1 - 稍微抬高并开始向编辑视角移动
    { XMFLOAT3(0,0,0), XMFLOAT3(0,0,0), XMFLOAT3(0,1,0) },

    // 中间点2 - 继续移动到编辑视角
    { XMFLOAT3(0,0,0), XMFLOAT3(0,0,0), XMFLOAT3(0,1,0) },

    // 终点 - 编辑模式位置（将在运行时填充实际值）
    { XMFLOAT3(0,0,0), XMFLOAT3(0,0,0), XMFLOAT3(0,1,0) }
};

static CameraPathPoint g_EditToPreviewPath[4] = {
    // 起点 - 编辑模式位置（将在运行时填充实际值）
    { XMFLOAT3(0,0,0), XMFLOAT3(0,0,0), XMFLOAT3(0,1,0) },

    // 中间点1 - 开始向预览视角移动
    { XMFLOAT3(0,0,0), XMFLOAT3(0,0,0), XMFLOAT3(0,1,0) },

    // 中间点2 - 继续移动到预览视角
    { XMFLOAT3(0,0,0), XMFLOAT3(0,0,0), XMFLOAT3(0,1,0) },

    // 终点 - 预览模式位置（将在运行时填充实际值）
    { XMFLOAT3(0,0,0), XMFLOAT3(0,0,0), XMFLOAT3(0,1,0) }
};

// 当前路径信息
static CameraPathPoint* g_CurrentPath = NULL;
static int g_PathPointCount = 0;
static float g_PathProgress = 0.0f;

// 初始化摄像机路径
void InitCameraTransitionPath(AppMode fromMode, AppMode toMode) {
    float cx = MAPSIZE_X * 0.5f;  // 地图中心X
    float cz = MAPSIZE_Z * 0.5f;  // 地图中心Z
    XMFLOAT3 mapCenter = XMFLOAT3(cx, 0.0f, -cz);

    if (fromMode == APP_MODE_PREVIEW && toMode == APP_MODE_EDIT) {
        // 从预览模式到编辑模式
        g_CurrentPath = g_PreviewToEditPath;
        g_PathPointCount = 4;

        // 设置起点 - 当前摄像机位置
        g_CurrentPath[0].position = g_Camera.Position;
        g_CurrentPath[0].lookAt = g_Camera.AtPosition;
        g_CurrentPath[0].up = g_Camera.UpVector;

        // 设置终点 - 编辑模式标准位置
        XMFLOAT3 editPos;
        editPos.x = mapCenter.x + EDIT_OFFSET_X;
        editPos.y = mapCenter.y + EDIT_OFFSET_Y;
        editPos.z = mapCenter.z + EDIT_OFFSET_Z;
        g_CurrentPath[3].position = editPos;
        g_CurrentPath[3].lookAt = mapCenter;
        g_CurrentPath[3].up = XMFLOAT3(0.0f, 1.0f, 0.0f);

        // 设置中间点1 - 向右侧移动点
        g_CurrentPath[1].position.x = mapCenter.x + 20.0f;
        g_CurrentPath[1].position.y = mapCenter.y + 15.0f;
        g_CurrentPath[1].position.z = mapCenter.z - 20.0f;
        g_CurrentPath[1].lookAt = mapCenter;
        g_CurrentPath[1].up = XMFLOAT3(0.0f, 1.0f, 0.0f);

        // 设置中间点2 - 接近编辑位置的点
        g_CurrentPath[2].position.x = editPos.x + 5.0f;
        g_CurrentPath[2].position.y = editPos.y - 5.0f;
        g_CurrentPath[2].position.z = editPos.z + 10.0f;
        g_CurrentPath[2].lookAt = mapCenter;
        g_CurrentPath[2].up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    }
    else if (fromMode == APP_MODE_EDIT && toMode == APP_MODE_PREVIEW) {
        // 从编辑模式到预览模式
        g_CurrentPath = g_EditToPreviewPath;
        g_PathPointCount = 4;

        // 设置起点 - 当前摄像机位置
        g_CurrentPath[0].position = g_Camera.Position;
        g_CurrentPath[0].lookAt = g_Camera.AtPosition;
        g_CurrentPath[0].up = g_Camera.UpVector;

        // 设置终点 - 预览模式标准位置
        XMFLOAT3 previewPos;
        previewPos.x = mapCenter.x + PREVIEW_OFFSET_X;
        previewPos.y = mapCenter.y + PREVIEW_OFFSET_Y;
        previewPos.z = mapCenter.z + PREVIEW_OFFSET_Z;
        g_CurrentPath[3].position = previewPos;
        g_CurrentPath[3].lookAt = mapCenter;
        g_CurrentPath[3].up = XMFLOAT3(0.0f, 1.0f, 0.0f);

        // 设置中间点1 - 向左侧移动点
        g_CurrentPath[1].position.x = mapCenter.x - 20.0f;
        g_CurrentPath[1].position.y = mapCenter.y + 15.0f;
        g_CurrentPath[1].position.z = mapCenter.z - 20.0f;
        g_CurrentPath[1].lookAt = mapCenter;
        g_CurrentPath[1].up = XMFLOAT3(0.0f, 1.0f, 0.0f);

        // 设置中间点2 - 接近预览位置的点
        g_CurrentPath[2].position.x = previewPos.x - 5.0f;
        g_CurrentPath[2].position.y = previewPos.y - 5.0f;
        g_CurrentPath[2].position.z = previewPos.z - 10.0f;
        g_CurrentPath[2].lookAt = mapCenter;
        g_CurrentPath[2].up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    }

    g_PathProgress = 0.0f;
    OutputDebugStringA("摄像机路径已初始化\n");
}

// 贝塞尔曲线插值
XMFLOAT3 CubicBezier(XMFLOAT3 p0, XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 p3, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    XMFLOAT3 result;
    // P = (1-t)³P₀ + 3(1-t)²tP₁ + 3(1-t)t²P₂ + t³P₃
    result.x = uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x;
    result.y = uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y;
    result.z = uuu * p0.z + 3 * uu * t * p1.z + 3 * u * tt * p2.z + ttt * p3.z;

    return result;
}
// 在Camera.cpp中添加
// 在Camera.cpp中添加

// 基于当前和目标模式计算一个平滑的环绕路径
void UpdateCameraTransition() {
    // 记录过渡状态
    static int transitionFrames = 0;
    transitionFrames++;

    // 设置最长过渡时间以防止卡住
    if (transitionFrames > 300) {
        // 直接跳到目标位置
        Camera* pCamera = GetCamera();
        AppMode targetMode = GetTargetAppMode();
        float cx = MAPSIZE_X * 0.5f;
        float cz = MAPSIZE_Z * 0.5f;

        if (targetMode == APP_MODE_EDIT) {
            pCamera->AtPositionAngle = XMFLOAT3(EDIT_ANGLE_X, EDIT_ANGLE_Y, 0.0f);
        }
        else {
            pCamera->AtPositionAngle = XMFLOAT3(PREVIEW_ANGLE_X, PREVIEW_ANGLE_Y, 0.0f);
        }

        pCamera->AtPosition = XMFLOAT3(cx, 0.0f, -cz);

        // 重置状态
        SetCameraTransitioning(false);
        transitionFrames = 0;
        OutputDebugStringA("摄像机过渡超时，强制完成\n");
        return;
    }

    // 获取目标模式
    AppMode targetMode = GetTargetAppMode();

    // 保存过渡起始位置和角度
    static XMFLOAT3 startPos, startAt, startAngle;
    static bool transitionInitialized = false;

    if (!transitionInitialized) {
        // 记录过渡开始时的状态
        startPos = g_Camera.Position;
        startAt = g_Camera.AtPosition;
        startAngle = g_Camera.AtPositionAngle;
        transitionInitialized = true;
        OutputDebugStringA("初始化摄像机过渡\n");
    }

    // 设置目标角度和注视点
    float cx = MAPSIZE_X * 0.5f;
    float cz = MAPSIZE_Z * 0.5f;
    XMFLOAT3 targetAt = XMFLOAT3(cx, 0.0f, -cz);
    XMFLOAT3 targetAngle;

    if (targetMode == APP_MODE_EDIT) {
        targetAngle = XMFLOAT3(EDIT_ANGLE_X, EDIT_ANGLE_Y, 0.0f);
    }
    else {
        targetAngle = XMFLOAT3(PREVIEW_ANGLE_X, PREVIEW_ANGLE_Y, 0.0f);
    }

    // 计算过渡进度 (0.0 - 1.0)
    const float TRANSITION_SPEED = 0.01f; // 较慢的速度确保平滑
    static float progress = 0.0f;
    progress += TRANSITION_SPEED;
    if (progress > 1.0f) progress = 1.0f;

    // =====================================
    // 单阶段平滑环绕过渡 - 使用环绕路径
    // =====================================

    // 首先，确保注视点始终保持在地图中心，平滑过渡
    g_Camera.AtPosition.x = Lerp(g_Camera.AtPosition.x, targetAt.x, 0.1f);
    g_Camera.AtPosition.y = Lerp(g_Camera.AtPosition.y, targetAt.y, 0.1f);
    g_Camera.AtPosition.z = Lerp(g_Camera.AtPosition.z, targetAt.z, 0.1f);

    // 计算初始和目标的偏移向量（相对于地图中心）
    XMFLOAT3 startOffset, targetOffset;
    startOffset.x = startPos.x - startAt.x;
    startOffset.y = startPos.y - startAt.y;
    startOffset.z = startPos.z - startAt.z;

    // 计算目标偏移量（基于目标角度和预设距离）
    XMVECTOR defaultOffset;
    if (targetMode == APP_MODE_EDIT) {
        defaultOffset = XMVectorSet(0.0f, EDIT_OFFSET_Y, EDIT_OFFSET_Z, 0.0f);
    }
    else {
        defaultOffset = XMVectorSet(0.0f, PREVIEW_OFFSET_Y, PREVIEW_OFFSET_Z, 0.0f);
    }

    XMMATRIX targetRot = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(targetAngle.x),
        XMConvertToRadians(targetAngle.y),
        0.0f);
    XMVECTOR targetOffsetVec = XMVector3TransformCoord(defaultOffset, targetRot);
    XMStoreFloat3(&targetOffset, targetOffsetVec);

    // 计算环绕路径的辅助点 - 向侧面偏移
    XMFLOAT3 midOffset;
    if (targetMode == APP_MODE_EDIT) {
        // 向左侧偏移环绕点
        midOffset.x = Lerp(startOffset.x, targetOffset.x, 0.3f) - 15.0f;
        midOffset.y = Lerp(startOffset.y, targetOffset.y, 0.5f);
        midOffset.z = Lerp(startOffset.z, targetOffset.z, 0.3f);
    }
    else {
        // 向右侧偏移环绕点
        midOffset.x = Lerp(startOffset.x, targetOffset.x, 0.3f) + 15.0f;
        midOffset.y = Lerp(startOffset.y, targetOffset.y, 0.5f);
        midOffset.z = Lerp(startOffset.z, targetOffset.z, 0.3f);
    }

    // 使用二次贝塞尔曲线计算摄像机位置
    float t = progress;
    float oneMinusT = 1.0f - t;

    // P = (1-t)²P₀ + 2(1-t)tP₁ + t²P₂
    XMFLOAT3 newOffset;
    newOffset.x = oneMinusT * oneMinusT * startOffset.x +
        2 * oneMinusT * t * midOffset.x +
        t * t * targetOffset.x;
    newOffset.y = oneMinusT * oneMinusT * startOffset.y +
        2 * oneMinusT * t * midOffset.y +
        t * t * targetOffset.y;
    newOffset.z = oneMinusT * oneMinusT * startOffset.z +
        2 * oneMinusT * t * midOffset.z +
        t * t * targetOffset.z;

    // 计算新的摄像机位置
    g_Camera.Position.x = g_Camera.AtPosition.x + newOffset.x;
    g_Camera.Position.y = g_Camera.AtPosition.y + newOffset.y;
    g_Camera.Position.z = g_Camera.AtPosition.z + newOffset.z;

    // 在接近结束时逐渐过渡到目标角度
    if (progress > 0.7f) {
        float angleT = (progress - 0.7f) / 0.3f; // 范围从0到1
        g_Camera.AtPositionAngle.x = Lerp(g_Camera.AtPositionAngle.x, targetAngle.x, angleT * 0.2f);
        g_Camera.AtPositionAngle.y = Lerp(g_Camera.AtPositionAngle.y, targetAngle.y, angleT * 0.2f);
    }

    // 检查是否完成过渡
    if (progress >= 1.0f) {
        // 确保最终位置和角度正确
        g_Camera.AtPositionAngle = targetAngle;

        // 重新计算最终偏移和位置
        XMMATRIX finalRot = XMMatrixRotationRollPitchYaw(
            XMConvertToRadians(targetAngle.x),
            XMConvertToRadians(targetAngle.y),
            0.0f);

        XMVECTOR finalOffset;
        if (targetMode == APP_MODE_EDIT) {
            finalOffset = XMVectorSet(0.0f, EDIT_OFFSET_Y, EDIT_OFFSET_Z, 0.0f);
        }
        else {
            finalOffset = XMVectorSet(0.0f, PREVIEW_OFFSET_Y, PREVIEW_OFFSET_Z, 0.0f);
        }

        XMVECTOR offsetVec = XMVector3TransformCoord(finalOffset, finalRot);
        XMStoreFloat3(&g_Camera.AtPositionOffset, offsetVec);

        g_Camera.Position.x = g_Camera.AtPosition.x + g_Camera.AtPositionOffset.x;
        g_Camera.Position.y = g_Camera.AtPosition.y + g_Camera.AtPositionOffset.y;
        g_Camera.Position.z = g_Camera.AtPosition.z + g_Camera.AtPositionOffset.z;

        // 重置过渡状态
        SetCameraTransitioning(false);
        transitionFrames = 0;
        progress = 0.0f;
        transitionInitialized = false;

        OutputDebugStringA("摄像机过渡完成\n");
    }

    // 直接更新视图矩阵
    g_Camera.ViewMatrix = XMMatrixLookAtLH(
        XMVectorSet(g_Camera.Position.x, g_Camera.Position.y, g_Camera.Position.z, 1.0f),
        XMVectorSet(g_Camera.AtPosition.x, g_Camera.AtPosition.y, g_Camera.AtPosition.z, 1.0f),
        XMVectorSet(g_Camera.UpVector.x, g_Camera.UpVector.y, g_Camera.UpVector.z, 0.0f)
    );
}

//===================================================




void InitCamera()
{
    // 获取窗口句柄
    g_hWnd = g_MainWnd;

    if (g_hWnd == NULL)
    {
        OutputDebugStringA("警告: 窗口句柄未初始化，尝试获取活动窗口\n");
        g_hWnd = GetActiveWindow();
    }

    //// 🔧 修复：初始化鼠标系统
    //if (g_hWnd != NULL) {
    //    Mouse_Initialize(g_hWnd);
    //    OutputDebugStringA("✅ 鼠标系统在相机初始化时成功初始化\n");
    //}
    //else {
    //    OutputDebugStringA("❌ 错误：无法获取有效的窗口句柄进行鼠标初始化\n");
    //}

    //g_CameraMode = CAMERA_MODE_PREVIEW;

    float cx = MAPSIZE_X * 0.5f;
    float cz = MAPSIZE_Z * 0.5f;
    g_Camera.AtPosition = XMFLOAT3(cx, 0.0f, -cz);
    g_Camera.AtPositionAngle = XMFLOAT3(PREVIEW_ANGLE_X, PREVIEW_ANGLE_Y, 0.0f);

    XMVECTOR defaultOffset = XMVectorSet(0.0f, PREVIEW_OFFSET_Y, PREVIEW_OFFSET_Z, 0.0f);
    XMMATRIX rot = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(g_Camera.AtPositionAngle.x),
        XMConvertToRadians(g_Camera.AtPositionAngle.y),
        0.0f);
    XMVECTOR offset = XMVector3TransformCoord(defaultOffset, rot);
    DirectX::XMStoreFloat3(&g_Camera.AtPositionOffset, offset);

    g_Camera.Position.x = g_Camera.AtPosition.x + g_Camera.AtPositionOffset.x;
    g_Camera.Position.y = g_Camera.AtPosition.y + g_Camera.AtPositionOffset.y;
    g_Camera.Position.z = g_Camera.AtPosition.z + g_Camera.AtPositionOffset.z;

    g_Camera.UpVector = XMFLOAT3(0.0f, 1.0f, 0.0f);
    g_Camera.fov = 23.5f;
    g_Camera.fovbuffer = 0.0f;
    g_Camera.nearclip = 0.5f;
    g_Camera.farclip = 1000.0f;
    g_Camera.Velocity = XMFLOAT3(0, 0, 0);

    Mouse_SetMode(MOUSE_POSITION_MODE_ABSOLUTE);
    //Mouse_SetVisible(GetCurrentAppMode() == APP_MODE_EDIT);

    g_Camera.ViewMatrix = XMMatrixLookAtLH(
        XMVectorSet(g_Camera.Position.x, g_Camera.Position.y, g_Camera.Position.z, 1.0f),
        XMVectorSet(g_Camera.AtPosition.x, g_Camera.AtPosition.y, g_Camera.AtPosition.z, 1.0f),
        XMVectorSet(g_Camera.UpVector.x, g_Camera.UpVector.y, g_Camera.UpVector.z, 0.0f)
    );

    g_Camera.ProjectionMatrix = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(g_Camera.fov),
        SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        g_Camera.nearclip,
        g_Camera.farclip
    );

    //DrawCamera();
    //isLerpingCameraAngle = false;
   // SetCameraTransitioning(false);
      // 重新计算视图矩阵
    g_Camera.ViewMatrix = XMMatrixLookAtLH(
        XMVectorSet(g_Camera.Position.x, g_Camera.Position.y, g_Camera.Position.z, 1.0f),
        XMVectorSet(g_Camera.AtPosition.x, g_Camera.AtPosition.y, g_Camera.AtPosition.z, 1.0f),
        XMVectorSet(g_Camera.UpVector.x, g_Camera.UpVector.y, g_Camera.UpVector.z, 0.0f)
    );

    // 将视图矩阵立即设置给渲染器
    SetViewMatrix(g_Camera.ViewMatrix);

    // 立即绘制一次相机，确保所有矩阵都被正确应用
    DrawCamera();

    // 输出当前摄像机参数
    char debugMsg[256];
    sprintf_s(debugMsg, "摄像机初始化完成 - 位置:(%.2f, %.2f, %.2f) 视角:(%.2f, %.2f, %.2f)\n",
        g_Camera.Position.x, g_Camera.Position.y, g_Camera.Position.z,
        g_Camera.AtPositionAngle.x, g_Camera.AtPositionAngle.y, g_Camera.AtPositionAngle.z);
    OutputDebugStringA(debugMsg);

    // 设置预览模式预设
    g_PreviewPreset.lookAt = XMFLOAT3(MAPSIZE_X * 0.5f, 0.0f, -MAPSIZE_Z * 0.5f);
    g_PreviewPreset.position = XMFLOAT3(
        g_PreviewPreset.lookAt.x + PREVIEW_OFFSET_X,
        g_PreviewPreset.lookAt.y + PREVIEW_OFFSET_Y,
        g_PreviewPreset.lookAt.z + PREVIEW_OFFSET_Z
    );
    g_PreviewPreset.up = XMFLOAT3(0.0f, 1.0f, 0.0f);

    // 设置编辑模式预设
    g_EditPreset.lookAt = XMFLOAT3(MAPSIZE_X * 0.5f, 0.0f, -MAPSIZE_Z * 0.5f);
    g_EditPreset.position = XMFLOAT3(
        g_EditPreset.lookAt.x + EDIT_OFFSET_X,
        g_EditPreset.lookAt.y + EDIT_OFFSET_Y,
        g_EditPreset.lookAt.z + EDIT_OFFSET_Z
    );
    g_EditPreset.up = XMFLOAT3(0.0f, 1.0f, 0.0f);
}


void	UninitCamera()
{

}

void UpdateCamera()
{
    // 如果是角色选择场景，什么都不推算，直接保持当前相机
    AppMode mode = GetCurrentAppMode();
    if (mode == APP_MODE_CHARACTER) { // ① 角色界面<br/> 
        SetViewMatrix(g_Camera.ViewMatrix);
        SetProjectionMatrix(g_Camera.ProjectionMatrix);
        return;
    }
    if (mode == APP_MODE_MULTIPLAYER_BATTLE) {
        // 多人战斗有专门的相机管理，这里完全不干扰
        SetViewMatrix(g_Camera.ViewMatrix);
        SetProjectionMatrix(g_Camera.ProjectionMatrix);
        return;
    }
    if (mode == APP_MODE_BATTLE_HALL) {
        // 战斗大厅有自己的摄像机管理，这里不要干扰
        SetViewMatrix(g_Camera.ViewMatrix);
        SetProjectionMatrix(g_Camera.ProjectionMatrix);
        return;
    }
// 获取当前应用模式
    AppMode currentMode = GetCurrentAppMode();

    // 只在非过渡状态下检查异常角度
    if (!IsCameraTransitioning()) {
        // 只在模式不变的情况下检查摄像机角度是否异常
        if (currentMode == APP_MODE_EDIT) {
            // 检查编辑模式角度是否正确
            float angleXDiff = fabs(g_Camera.AtPositionAngle.x - EDIT_ANGLE_X);
            float angleYDiff = fabs(g_Camera.AtPositionAngle.y - EDIT_ANGLE_Y);

            // 如果角度相差超过45度，认为是错误状态，强制纠正
            if (angleXDiff > 45.0f || angleYDiff > 45.0f) {
                OutputDebugStringA("检测到摄像机角度异常，强制修正为编辑模式视角\n");
                g_Camera.AtPositionAngle = XMFLOAT3(EDIT_ANGLE_X, EDIT_ANGLE_Y, 0.0f);
                // 设置相机过渡状态，让它平滑过渡到正确位置
                SetCameraTransitioning(true);
            }
        }
        else if (currentMode == APP_MODE_PREVIEW) {
            // 预览模式下只检查X角度，Y角度用户可能会旋转
            float angleXDiff = fabs(g_Camera.AtPositionAngle.x - PREVIEW_ANGLE_X);

            // 如果X角度偏差过大（例如变成负值），认为是意外情况
            if (angleXDiff > 45.0f) {
                OutputDebugStringA("检测到摄像机X角度异常，强制修正为预览模式视角\n");
                g_Camera.AtPositionAngle.x = PREVIEW_ANGLE_X;
                // 不修改Y角度，保留用户的旋转
                // 设置相机过渡状态，让它平滑过渡到正确位置
                SetCameraTransitioning(true);
            }
        }
    }

    BoxObject* g_Box = GetBox();
    Mouse_GetState(&Mouse);
    Mouse_ResetScrollWheelValue();

    // Set mouse to absolute coordinates mode, and adjust visibility based on current mode
    Mouse_SetMode(MOUSE_POSITION_MODE_ABSOLUTE);
   // Mouse_SetVisible(GetCurrentAppMode() == APP_MODE_EDIT);

    // Get current app mode and transition state
    AppMode currentCameraMode = GetCurrentAppMode();
    AppMode g_CurrentTargetAppMode = GetTargetAppMode();
    bool modeChanging = IsModeChanging();
    float transitionProgress = GetModeTransitionProgress();
    // 目标角度与注视点
    XMFLOAT3 targetAngle;
    float cx = MAPSIZE_X * 0.5f;
    float cz = MAPSIZE_Z * 0.5f;
    XMFLOAT3 targetAt = XMFLOAT3(cx, 0.0f, -cz);
    // 为不同模式设置不同的视角和距离
    XMVECTOR defaultOffset;
    AppMode targetMode = modeChanging ? g_CurrentTargetAppMode : currentCameraMode;

    if (targetMode == APP_MODE_PREVIEW) {
        defaultOffset = XMVectorSet(0.0f, PREVIEW_OFFSET_Y, PREVIEW_OFFSET_Z, 0.0f);
        targetAngle = XMFLOAT3(PREVIEW_ANGLE_X, PREVIEW_ANGLE_Y, 0.0f);
    }
    else { // 编辑模式
        defaultOffset = XMVectorSet(0.0f, EDIT_OFFSET_Y, EDIT_OFFSET_Z, 0.0f);
        targetAngle = XMFLOAT3(EDIT_ANGLE_X, EDIT_ANGLE_Y, 0.0f);
    }

    // 平滑过渡
    const float LERP_SPEED = 0.03f; // 降低过渡速度，使其更加平滑
    // 在UpdateCamera函数内部替换现有的过渡代码
   // 在UpdateCamera函数中替换现有的过渡逻辑
    if (IsCameraTransitioning()) {
        // 使用专用函数处理过渡
        UpdateCameraTransition();

        // 更新投影矩阵
        g_Camera.ProjectionMatrix = XMMatrixPerspectiveFovLH(
            XMConvertToRadians(g_Camera.fov),
            SCREEN_WIDTH / (float)SCREEN_HEIGHT,
            g_Camera.nearclip,
            g_Camera.farclip
        );

        // 设置矩阵
        SetViewMatrix(g_Camera.ViewMatrix);
        SetProjectionMatrix(g_Camera.ProjectionMatrix);

        // 退出函数，跳过其余的相机更新
        return;
    }
    else {
        // 非过渡状态下，只有在预览模式才允许自由移动摄像机
        if (currentCameraMode == APP_MODE_PREVIEW) {
            // 预览模式下平滑移动注视点（现有代码保持不变）
            g_Camera.AtPosition.x = Lerp(g_Camera.AtPosition.x, targetAt.x, /*LERP_SPEED **/ 0.5f);
            g_Camera.AtPosition.z = Lerp(g_Camera.AtPosition.z, targetAt.z, /*LERP_SPEED **/ 0.5f);
        }
    }

    // Smoothly move the look-at position
    g_Camera.AtPosition.x = Lerp(g_Camera.AtPosition.x, targetAt.x, LERP_SPEED);
    g_Camera.AtPosition.z = Lerp(g_Camera.AtPosition.z, targetAt.z, LERP_SPEED);

    // Build direction vectors
    XMVECTOR forwardVec = XMLoadFloat3(&g_Camera.AtPositionOffset);
    forwardVec = XMVector3Normalize(forwardVec) * -1.0f;
    XMVECTOR upVec = XMVectorSet(0, 1, 0, 0);
    XMVECTOR rightVec = XMVector3Normalize(XMVector3Cross(upVec, forwardVec));

    // Handle camera controls in PREVIEW mode
    if (currentCameraMode == APP_MODE_PREVIEW) {
        // Rotating with left mouse button
        static bool isDragging = false;
        static int lastMouseX = 0;
        static int lastMouseY = 0;

        if (Mouse.leftButton)
        {
            if (!isDragging) {
                // First press, record start point
                lastMouseX = Mouse.x;
                lastMouseY = Mouse.y;
                isDragging = true;
            }

            int dx = Mouse.x - lastMouseX;
            int dy = Mouse.y - lastMouseY;

            const float ROTATE_SPEED = 0.3f;
            // Clamp to -60 to 0 for a more angled camera
            g_Camera.AtPositionAngle.x = Clamp(g_Camera.AtPositionAngle.x - dy * ROTATE_SPEED, -60.0f, 0.0f);
            g_Camera.AtPositionAngle.y += dx * ROTATE_SPEED;

            // Update to new position
            lastMouseX = Mouse.x;
            lastMouseY = Mouse.y;
        }
        else
        {
            isDragging = false;
        }

        // Pan view with right mouse button
        if (Mouse.rightButton)
        {
            const float PAN_SPEED = 0.02f;
            g_Camera.AtPosition.x -= Mouse.x * PAN_SPEED;
            g_Camera.AtPosition.z += Mouse.y * PAN_SPEED;
        }

        // Keyboard movement
        const float MOVE_SPEED = 0.3f;

        if (Keyboard_IsKeyDown(KK_W))
        {
            g_Camera.AtPosition.x += XMVectorGetX(forwardVec) * MOVE_SPEED;
            g_Camera.AtPosition.z += XMVectorGetZ(forwardVec) * MOVE_SPEED;
        }
        if (Keyboard_IsKeyDown(KK_S))
        {
            g_Camera.AtPosition.x -= XMVectorGetX(forwardVec) * MOVE_SPEED;
            g_Camera.AtPosition.z -= XMVectorGetZ(forwardVec) * MOVE_SPEED;
        }
        if (Keyboard_IsKeyDown(KK_D))
        {
            g_Camera.AtPosition.x += XMVectorGetX(rightVec) * MOVE_SPEED;
            g_Camera.AtPosition.z += XMVectorGetZ(rightVec) * MOVE_SPEED;
        }
        if (Keyboard_IsKeyDown(KK_A))
        {
            g_Camera.AtPosition.x -= XMVectorGetX(rightVec) * MOVE_SPEED;
            g_Camera.AtPosition.z -= XMVectorGetZ(rightVec) * MOVE_SPEED;
        }
    }

    // Handle UI box selection and map editing in EDIT mode
    if (currentCameraMode == APP_MODE_EDIT)
    {
        static bool wasLeftButtonDown = false;
        bool isLeftButtonDown = Mouse.leftButton;

        if (isLeftButtonDown && !wasLeftButtonDown)
        {
            // Mouse just pressed down
            OutputDebugStringA("Mouse press detected...\n");

            if (IsEditingObstacle()) {
                // Obstacle edit mode
                HandleObstacleEditModeClick(Mouse.x, Mouse.y);
            }
            else {
                // Terrain edit mode
                HandleTerrainEditModeClick(Mouse.x, Mouse.y);
            }
        }
        else if (isLeftButtonDown && g_IsDraggingMap) {
            // Mouse left button held down and in dragging state
            HandleMapDragging();
        }
        else if (!isLeftButtonDown && wasLeftButtonDown) {
            // Mouse left button released
            g_IsDraggingMap = false;
            g_LastEditedX = -1;
            g_LastEditedZ = -1;
        }

        wasLeftButtonDown = isLeftButtonDown;
    }

    // Update hover position in each frame
    int hoverX, hoverZ;
    if (currentCameraMode == APP_MODE_EDIT && PickBoxUnderMouse(&hoverX, &hoverZ)) {
        g_HoverX = hoverX;
        g_HoverZ = hoverZ;
    }
    else {
        g_HoverX = -1;
        g_HoverZ = -1;
    }

    // Handle mouse wheel
    if (Mouse.scrollWheelValue > 0) g_Camera.fovbuffer += 0.1f;
    if (Mouse.scrollWheelValue < 0) g_Camera.fovbuffer -= 0.1f;
    Mouse_ResetScrollWheelValue();

    g_Camera.fov = Clamp(g_Camera.fov + g_Camera.fovbuffer, FOV_MIN, FOV_MAX);
    g_Camera.fovbuffer = 0.0f;

    // Calculate camera position
    XMMATRIX rot = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(g_Camera.AtPositionAngle.x),
        XMConvertToRadians(g_Camera.AtPositionAngle.y),
        0.0f);
    XMVECTOR offset = XMVector3TransformCoord(defaultOffset, rot);
    DirectX::XMStoreFloat3(&g_Camera.AtPositionOffset, offset);

    g_Camera.Position.x = g_Camera.AtPosition.x + g_Camera.AtPositionOffset.x;
    g_Camera.Position.y = g_Camera.AtPosition.y + g_Camera.AtPositionOffset.y;
    g_Camera.Position.z = g_Camera.AtPosition.z + g_Camera.AtPositionOffset.z;

    // Update view matrix
    g_Camera.ViewMatrix = XMMatrixLookAtLH(
        XMVectorSet(g_Camera.Position.x, g_Camera.Position.y, g_Camera.Position.z, 1.0f),
        XMVectorSet(g_Camera.AtPosition.x, g_Camera.AtPosition.y, g_Camera.AtPosition.z, 1.0f),
        XMVectorSet(g_Camera.UpVector.x, g_Camera.UpVector.y, g_Camera.UpVector.z, 0.0f)
    );

    // Update projection matrix
    g_Camera.ProjectionMatrix = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(g_Camera.fov),
        SCREEN_WIDTH / (float)SCREEN_HEIGHT,
        g_Camera.nearclip,
        g_Camera.farclip
    );
}
void	DrawCamera()
{
    //プロジェクション行列を作成
    g_Camera.ProjectionMatrix =
        XMMatrixPerspectiveFovLH(
            XMConvertToRadians(g_Camera.fov),
            (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
            g_Camera.nearclip,
            g_Camera.farclip
        );
    //DirectXへセット
    SetProjectionMatrix(g_Camera.ProjectionMatrix);

    //カメラ行列を作成
    XMVECTOR	eyev = XMLoadFloat3(&g_Camera.AtPosition);
    XMVECTOR	pos = XMLoadFloat3(&g_Camera.Position);
    XMVECTOR	up = XMLoadFloat3(&g_Camera.UpVector);

    g_Camera.ViewMatrix =
        XMMatrixLookAtLH(pos, eyev, up);

    //行列をセット
    SetViewMatrix(g_Camera.ViewMatrix);

}

XMFLOAT3	GetCameraRightVec()
{
    return	g_Camera.RitVec;
}
XMFLOAT3	GetCameraForwardVec()
{
    return	g_Camera.FwdVec;
}

XMMATRIX	GetViewMatrix()
{
    return	g_Camera.ViewMatrix;
}

XMMATRIX GetProjectMatrix()
{
    return g_Camera.ProjectionMatrix;
}

void		SetCameraVelocity(XMFLOAT3 vec)
{
    g_Camera.Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

// 在WorldToScreen函数中添加边界检查
bool WorldToScreen(const XMFLOAT3& worldPos, XMFLOAT2* screenPos) {
    XMVECTOR vWorldPos = XMLoadFloat3(&worldPos);

    XMMATRIX viewMatrix = GetViewMatrix();
    XMMATRIX projMatrix = GetProjectMatrix();

    XMVECTOR vViewPos = XMVector3TransformCoord(vWorldPos, viewMatrix);
    XMVECTOR vProjPos = XMVector3TransformCoord(vViewPos, projMatrix);

    float w = XMVectorGetW(vProjPos);
    if (w <= 0.0f) {
        return false;
    }

    // 执行透视除法
    XMFLOAT3 projPos;
    projPos.x = XMVectorGetX(vProjPos) / w;
    projPos.y = XMVectorGetY(vProjPos) / w;
    projPos.z = XMVectorGetZ(vProjPos) / w;

    // 检查是否在视锥体内
    if (projPos.x < -1.0f || projPos.x > 1.0f ||
        projPos.y < -1.0f || projPos.y > 1.0f ||
        projPos.z < 0.0f || projPos.z > 1.0f) {
        return false;
    }

    // 转换到屏幕坐标
    screenPos->x = (projPos.x * 0.5f + 0.5f) * SCREEN_WIDTH;
    screenPos->y = (-projPos.y * 0.5f + 0.5f) * SCREEN_HEIGHT;

    // 添加调试信息，检查坐标是否超出屏幕范围
    char debug[256];
    sprintf_s(debug, "屏幕坐标: (%.1f, %.1f) - 屏幕尺寸: %dx%d\n",
        screenPos->x, screenPos->y, SCREEN_WIDTH, SCREEN_HEIGHT);
    OutputDebugStringA(debug);

    return true;
}