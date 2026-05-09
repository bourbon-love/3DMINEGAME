// CharacterScene.h - 简化的角色选择场景
#pragma once

#include "main.h"
#include "renderer.h"
#include "PixelCharacters.h"

// 前向声明
struct CharacterPlatform;

// 日语纹理全局变量声明
//extern ID3D11ShaderResourceView* g_CharacterInfoTextures[4];

// 角色场景管理函数
void InitCharacterScene();           // 初始化角色选择场景
void UninitCharacterScene();        // 清理角色选择场景
void UpdateCharacterScene(float deltaTime); // 更新角色选择场景
void DrawCharacterScene();          // 绘制角色选择场景

void ActivateCharacterScene();      // 激活角色选择场景
void DeactivateCharacterScene();    // 停用角色选择场景
bool IsCharacterSceneActive();      // 判断角色选择场景是否激活

void SelectCharacter(int index);    // 选择角色
CHARACTER_TYPE GetSelectedCharacterType(); // 获取当前选中的角色

// 内部使用函数
void CreatePlatformGeometry();      // 创建平台几何体
void DrawPlatform(const CharacterPlatform& platform); // 绘制平台
ID3D11ShaderResourceView* GetCharacterPlatformTexture(CHARACTER_TYPE type);

// 🔧 新的角色说明系统函数
void DrawCharacterWithRotation(const CharacterPlatform& platform, int characterIndex, float deltaTime);
void DrawCharacterInfoTexture(int characterIndex);
void ShowCharacterInfo(int characterIndex);
// 🔧 3D纹理显示相关函数（最终版本）
void DrawCharacterInfoAt3DPosition(const CharacterPlatform& platform, int characterIndex, float deltaTime);
void HideCharacterInfo(int characterIndex);
void HideCharacterInfoCompletely(int characterIndex);
void TestCharacterInfoDisplay();