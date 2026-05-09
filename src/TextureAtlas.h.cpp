// TextureAtlas.cpp


#include "TextureAtlas.h"
#include <fstream>
#include <sstream>

TextureAtlas::TextureAtlas()
    : m_atlasTexture(nullptr)
    , m_atlasSize(0.0f, 0.0f)
    , m_initialized(false) {
}

TextureAtlas::~TextureAtlas() {
    Release();
}

bool TextureAtlas::LoadAtlasManual(const char* imagePath, int width, int height) {
    // 加载纹理图像
    TexMetadata metadata;
    ScratchImage image;

    HRESULT hr = LoadFromWICFile(
        std::wstring(imagePath, imagePath + strlen(imagePath)).c_str(),
        WIC_FLAGS_NONE,
        &metadata,
        image
    );

    if (FAILED(hr)) {
        OutputDebugStringA("无法加载图集纹理文件\n");
        return false;
    }

    // 创建着色器资源视图
    hr = CreateShaderResourceView(
        GetDevice(),
        image.GetImages(),
        image.GetImageCount(),
        metadata,
        &m_atlasTexture
    );

    if (FAILED(hr)) {
        OutputDebugStringA("无法创建图集着色器资源视图\n");
        return false;
    }

    m_atlasSize = XMFLOAT2((float)width, (float)height);
    m_initialized = true;

    OutputDebugStringA("图集纹理加载成功\n");
    return true;
}

void TextureAtlas::AddSprite(const std::string& name, int x, int y, int width, int height) {
    if (!m_initialized) return;

    AtlasSprite sprite;
    sprite.name = name;
    sprite.pixelSize = XMFLOAT2((float)width, (float)height);

    // 计算UV坐标（归一化到0-1范围）
    sprite.uvMin.x = (float)x / m_atlasSize.x;
    sprite.uvMin.y = (float)y / m_atlasSize.y;
    sprite.uvMax.x = (float)(x + width) / m_atlasSize.x;
    sprite.uvMax.y = (float)(y + height) / m_atlasSize.y;

    // 计算归一化尺寸
    sprite.normalizedSize.x = (float)width / m_atlasSize.x;
    sprite.normalizedSize.y = (float)height / m_atlasSize.y;

    m_sprites[name] = sprite;

    char debug[256];
    sprintf_s(debug, "添加精灵: %s, UV=(%.3f,%.3f)-(%.3f,%.3f)\n",
        name.c_str(), sprite.uvMin.x, sprite.uvMin.y, sprite.uvMax.x, sprite.uvMax.y);
    OutputDebugStringA(debug);
}

const AtlasSprite* TextureAtlas::GetSprite(const std::string& name) const {
    auto it = m_sprites.find(name);
    if (it != m_sprites.end()) {
        return &it->second;
    }
    return nullptr;
}

bool TextureAtlas::LoadAtlas(const char* imagePath, const char* configPath) {
    // 这里可以实现从JSON或XML读取配置的逻辑
    // 为简化，我们暂时只使用手动配置版本
    return false;
}

void TextureAtlas::Release() {
    if (m_atlasTexture) {
        m_atlasTexture->Release();
        m_atlasTexture = nullptr;
    }
    m_sprites.clear();
    m_initialized = false;
}

// TextureAtlasManager实现
TextureAtlasManager* TextureAtlasManager::m_instance = nullptr;

TextureAtlasManager* TextureAtlasManager::GetInstance() {
    if (!m_instance) {
        m_instance = new TextureAtlasManager();
    }
    return m_instance;
}

void TextureAtlasManager::DestroyInstance() {
    if (m_instance) {
        m_instance->ReleaseAll();
        delete m_instance;
        m_instance = nullptr;
    }
}

bool TextureAtlasManager::LoadAtlas(const std::string& atlasName, const char* imagePath, const char* configPath) {
    TextureAtlas* atlas = new TextureAtlas();

    // 暂时使用手动加载方式，可以替换为从配置文件加载
    if (atlas->LoadAtlasManual(imagePath, 1024, 1024)) {  // 假设图集大小为1024x1024
        m_atlases[atlasName] = atlas;
        return true;
    }

    delete atlas;
    return false;
}

TextureAtlas* TextureAtlasManager::GetAtlas(const std::string& atlasName) {
    auto it = m_atlases.find(atlasName);
    if (it != m_atlases.end()) {
        return it->second;
    }
    return nullptr;
}

void TextureAtlasManager::ReleaseAll() {
    for (auto& pair : m_atlases) {
        delete pair.second;
    }
    m_atlases.clear();
}
