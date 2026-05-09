// skydome.h
#pragma once

#include "main.h"
#include "renderer.h"

//天空穹顶类
class SkyDome {
public:
    bool Use;
    XMFLOAT3 Position;
    XMFLOAT3 Scale;
    XMFLOAT3 Rotation;
    XMFLOAT4 TopColor;      //天空顶部颜色
    XMFLOAT4 MiddleColor;   //天空中间颜色
    XMFLOAT4 BottomColor;   //天空底部颜色
    XMFLOAT3 SunPosition;   //太阳位置
    float Time;             //时间参数
    float CloudDensity;     //云密度
    float CloudScale;       //云的缩放比例
    float CloudSharpness;   //云的锐利度

};

void InitSkyDome();
void UninitSkyDome();
void UpdateSkyDome(float deltaTime);
void DrawSkyDome();

SkyDome* GetSkyDome();
