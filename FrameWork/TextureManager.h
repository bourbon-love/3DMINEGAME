#pragma once
#pragma once

#include "../main.h"
#include "../renderer.h"

// ?理?型枚?
enum TEXTURE_TYPE {
    TEXTURE_DEFAULT = 0,        // 默??理
    TEXTURE_START,              // 起点?理
    TEXTURE_GOAL,               // ?点?理
    TEXTURE_ICE,                // 冰面?理
    TEXTURE_SAND,               // 沙地?理
    TEXTURE_GRASS,              // 草地?理
    TEXTURE_WATER,              // 水面?理
    TEXTURE_WALL,               // ?体?理
    TEXTURE_GRID,               // 网格?理
    TEXTURE_GROUND,             // 地面?理
    TEXTURE_GROUND_UI,          // 地面UI?理
    TEXTURE_PARTICLE,           // 粒子?理
    TEXTURE_COIN,               // 金??理
    TEXTURE_BOMB,               // 炸??理

    // 角色?明?理
    TEXTURE_CHARACTER_DOCTOR,   // 医生?明
    TEXTURE_CHARACTER_SOLDIER,  // 士兵?明
    TEXTURE_CHARACTER_SCOUT,    // ?察兵?明
    TEXTURE_CHARACTER_BOMB_TECH,// 拆?手?明

    TEXTURE_TYPE_COUNT          // ?理?数
};

// ?理管理器?
class TextureManager {
private:
    static ID3D11ShaderResourceView* s_textures[TEXTURE_TYPE_COUNT];
    static bool s_initialized;

    // ?理文件路径映射
    static const wchar_t* GetTextureFilePath(TEXTURE_TYPE type);

public:
    // 初始化?理管理器
    static bool Init();

    // 清理?理管理器
    static void Uninit();

    // ?取指定?型的?理
    static ID3D11ShaderResourceView* GetTexture(TEXTURE_TYPE type);

    // ???理是否已加?
    static bool IsTextureLoaded(TEXTURE_TYPE type);

    // 重新加??个?理
    static bool ReloadTexture(TEXTURE_TYPE type);

    // ?取?理名称（用于??）
    static const char* GetTextureName(TEXTURE_TYPE type);
};

// 便捷的宏定?，用于快速?取?理
#define GET_TEXTURE(type) TextureManager::GetTexture(TEXTURE_##type)