// PixelCharacters.h
#pragma once

#include "main.h"
#include "renderer.h"

// 角色类型枚举
typedef enum {
    CHARACTER_DOCTOR,     // 医生
    CHARACTER_SOLDIER,    // 士兵
    CHARACTER_SCOUT,      // 侦察兵
    CHARACTER_BOMB_TECH,  // 拆弹手
    CHARACTER_TYPE_COUNT  // 角色类型总数
} CHARACTER_TYPE;

// 定义像素角色模型结构体
typedef struct {
    ID3D11Buffer* VertexBuffer;    // 顶点缓冲区
    ID3D11Buffer* IndexBuffer;     // 索引缓冲区
    int VertexCount;               // 顶点数量
    int IndexCount;                // 索引数量
    XMFLOAT4 MainColor;            // 主体颜色
    XMFLOAT4 SecondaryColor;       // 次要颜色
    XMFLOAT4 DetailColor;          // 细节颜色
    CHARACTER_TYPE Type;           // 角色类型
} PixelCharacter;



// 全局角色对象
extern PixelCharacter g_Doctor;     // 医生
extern PixelCharacter g_Soldier;    // 士兵
extern PixelCharacter g_Scout;      // 侦察兵
extern PixelCharacter g_BombTech;   // 拆弹手

// 全局动画参数数组 - 供 Ball.cpp 访问
extern float g_CharacterBobHeights[CHARACTER_TYPE_COUNT];
extern float g_CharacterArmSwings[CHARACTER_TYPE_COUNT];

// 设置角色动画参数的函数
void SetCharacterBobHeight(CHARACTER_TYPE type, float height);  // 设置角色上下移动高度
void SetCharacterArmSwing(CHARACTER_TYPE type, float swing);    // 设置角色手臂摆动角度

// 重置角色动画参数
void ResetCharacterAnimation(CHARACTER_TYPE type);              // 重置指定角色的动画参数
// 初始化和清理
void InitPixelCharacters();         // 初始化所有角色模型
void UninitPixelCharacters();       // 清理所有角色模型
void UpdatePixelCharacters(float deltaTime); // 更新角色动画

// 角色动画参数
float GetCharacterBobHeight(CHARACTER_TYPE type); // 获取角色上下移动高度
float GetCharacterArmSwing(CHARACTER_TYPE type);  // 获取角色手臂摆动角度

// 创建角色模型
void CreatePixelDoctor(PixelCharacter* character);      // 创建医生模型
void CreatePixelSoldier(PixelCharacter* character);     // 创建士兵模型
void CreatePixelScout(PixelCharacter* character);       // 创建侦察兵模型
void CreatePixelBombTech(PixelCharacter* character);    // 创建拆弹手模型

// 添加到 PixelCharacters.h 中
void CreateRealisticHead(VERTEX_3D* vertices, unsigned int* indices,
    int& vertexIndex, int& indexIndex, float centerX, float centerY, float centerZ,
    XMFLOAT4 skinColor, XMFLOAT4 hairColor, XMFLOAT4 eyeColor, CHARACTER_TYPE type);

void CreateRealisticTorso(VERTEX_3D* vertices, unsigned int* indices,
    int& vertexIndex, int& indexIndex, float centerX, float centerY, float centerZ,
    XMFLOAT4 shirtColor, XMFLOAT4 detailColor, CHARACTER_TYPE type);

void CreateRealisticArm(VERTEX_3D* vertices, unsigned int* indices,
    int& vertexIndex, int& indexIndex, float shoulderX, float shoulderY, float shoulderZ,
    bool isLeft, XMFLOAT4 skinColor, XMFLOAT4 sleeveColor);

void CreateRealisticLeg(VERTEX_3D* vertices, unsigned int* indices,
    int& vertexIndex, int& indexIndex, float hipX, float hipY, float hipZ,
    bool isLeft, XMFLOAT4 pantColor, XMFLOAT4 shoeColor);

void CreateRealisticPixelCharacter(PixelCharacter* character,
    XMFLOAT4 skinColor, XMFLOAT4 hairColor, XMFLOAT4 eyeColor,
    XMFLOAT4 shirtColor, XMFLOAT4 pantColor, XMFLOAT4 shoeColor,
    XMFLOAT4 detailColor, CHARACTER_TYPE type);

// 绘制像素角色
void DrawPixelCharacter(PixelCharacter* character, XMMATRIX worldMatrix); // 绘制角色
void DrawAllPixelCharacters(XMMATRIX* worldMatrices);  // 绘制所有角色

bool GetPixelCharactersInitialized();
bool GetPixelCharacterValid(CHARACTER_TYPE type);