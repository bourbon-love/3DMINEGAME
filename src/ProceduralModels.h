// ProceduralModels.h
#pragma once

#include "main.h"
#include "renderer.h"

// 定义程序化模型结构体
typedef struct {
    ID3D11Buffer* VertexBuffer;
    ID3D11Buffer* IndexBuffer;
    int VertexCount;
    int IndexCount;
    XMFLOAT4 Color;
} ProceduralModel;

// 全局模型对象
extern ProceduralModel g_CoinModel;
extern ProceduralModel g_BombModel;
extern ProceduralModel g_FuseModel; // 新增：引信模型

// 初始化和清理
void InitProceduralModels();
void UninitProceduralModels();
void UpdateProceduralModels(float deltaTime);

// 获取当前动画参数
float GetCoinRotation();
float GetBombRotation();
float GetFuseGlowIntensity(); // 新增：获取引信发光强度

// 创建模型
void CreateProceduralCoin(ProceduralModel* model);
void CreateProceduralBomb(ProceduralModel* model);
void CreateProceduralFuse(ProceduralModel* model); // 新增：创建引信

// 绘制程序化模型
void DrawProceduralModel(ProceduralModel* model, XMMATRIX worldMatrix);
void DrawCompleteBomb(XMMATRIX worldMatrix); // 新增：绘制完整炸弹