#include "Camera.h"
#include "keyboard.h"
#include <stdio.h>
#include <Windows.h>

// 简单的调试输出函数
void DebugPrint(const char* format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, format, args);
    va_end(args);

    // 同时输出到控制台和调试窗口
    printf("%s", buffer);
    OutputDebugStringA(buffer);
}

// 打印当前相机参数函数
void PrintCameraParameters()
{
    Camera* camera = GetCamera();

    DebugPrint("\n=== 当前相机参数 ===\n");
    DebugPrint("位置: (%.2f, %.2f, %.2f)\n",
        camera->Position.x, camera->Position.y, camera->Position.z);

    DebugPrint("注视点: (%.2f, %.2f, %.2f)\n",
        camera->AtPosition.x, camera->AtPosition.y, camera->AtPosition.z);

    DebugPrint("角度: (%.2f, %.2f, %.2f)\n",
        camera->AtPositionAngle.x, camera->AtPositionAngle.y, camera->AtPositionAngle.z);

    DebugPrint("FOV: %.2f\n", camera->fov);

    DebugPrint("=========================================\n");

    // 为了复制到代码中的定义格式
    DebugPrint("\n// 定义格式:\n");
    DebugPrint("#define CAMERA_ANGLE_X (%.2ff)\n", camera->AtPositionAngle.x);
    DebugPrint("#define CAMERA_ANGLE_Y (%.2ff)\n", camera->AtPositionAngle.y);
    DebugPrint("#define CAMERA_OFFSET_Y (%.2ff)\n", camera->AtPositionOffset.y);
    DebugPrint("#define CAMERA_OFFSET_Z (%.2ff)\n", camera->AtPositionOffset.z);
}

// 在Game.cpp的UpdateGame函数中添加以下代码
void UpdateCameraDebug()
{
    static bool lastP = false;
    bool nowP = Keyboard_IsKeyDown(KK_P);

    // 按P键时打印相机参数
    if (nowP && !lastP)
    {
        PrintCameraParameters();
    }

    lastP = nowP;
}