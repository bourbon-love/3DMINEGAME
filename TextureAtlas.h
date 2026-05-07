// TextureAtlas.h
#pragma once

#include "main.h"
#include "renderer.h"
#include <unordered_map>
#include <string>

// 图集中单个纹理的信息
struct AtlasSprite {
    XMFLOAT2 uvMin;              // 左上角UV坐标
    XMFLOAT2 uvMax;              // 右下角UV坐标
    XMFLOAT2 pixelSize;         // 原始像素尺寸
    XMFLOAT2 normalizedSize;     // 归一化尺寸(0-1)
    std::string name;            // 纹理标识名称
};

// 纹理图集类
class TextureAtlas {
private:
    ID3D11ShaderResourceView* m_atlasTexture;  // 图集纹理
    XMFLOAT2 m_atlasSize;                      // 图集总尺寸
    std::unordered_map<std::string, AtlasSprite> m_sprites;  // 所有精灵的映射
    bool m_initialized;

public:
    TextureAtlas();
    ~TextureAtlas();

    // 从JSON或XML配置文件加载图集信息
    bool LoadAtlas(const char* imagePath, const char* configPath);

    // 手动定义图集（用于简单测试）
    bool LoadAtlasManual(const char* imagePath, int width, int height);

    // 手动添加精灵定义
    void AddSprite(const std::string& name, int x, int y, int width, int height);

    // 获取特定精灵的信息
    const AtlasSprite* GetSprite(const std::string& name) const;

    // 获取图集纹理
    ID3D11ShaderResourceView* GetTexture() const { return m_atlasTexture; }

    // 释放资源
    void Release();

    // 获取图集尺寸
    XMFLOAT2 GetAtlasSize() const { return m_atlasSize; }
};

// 纹理图集管理器 - 单例模式
class TextureAtlasManager {
private:
    static TextureAtlasManager* m_instance;
    std::unordered_map<std::string, TextureAtlas*> m_atlases;

    TextureAtlasManager() {}

public:
    static TextureAtlasManager* GetInstance();
    static void DestroyInstance();

    // 加载一个图集
    bool LoadAtlas(const std::string& atlasName, const char* imagePath, const char* configPath);

    // 获取图集
    TextureAtlas* GetAtlas(const std::string& atlasName);

    // 释放所有图集
    void ReleaseAll();
};