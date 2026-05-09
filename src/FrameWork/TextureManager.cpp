#include "TextureManager.h"
#include "TextureManager.h"
#include "../DirectXTex.h"

using namespace DirectX;

// 静?成??量定?
ID3D11ShaderResourceView* TextureManager::s_textures[TEXTURE_TYPE_COUNT] = { nullptr };
bool TextureManager::s_initialized = false;

// ?理文件路径映射
const wchar_t* TextureManager::GetTextureFilePath(TEXTURE_TYPE type) {
    static const wchar_t* texturePaths[TEXTURE_TYPE_COUNT] = {
        L"asset\\texture\\mapchip.png",      // TEXTURE_DEFAULT
        L"asset\\texture\\START.png",        // TEXTURE_START
        L"asset\\texture\\GOAL.png",         // TEXTURE_GOAL
        L"asset\\texture\\ice.png",          // TEXTURE_ICE
        L"asset\\texture\\sand.png",         // TEXTURE_SAND
        L"asset\\texture\\grass.png",        // TEXTURE_GRASS
        L"asset\\texture\\water.png",        // TEXTURE_WATER
        L"asset\\texture\\wall.png",         // TEXTURE_WALL
        L"asset\\texture\\grid2.png",        // TEXTURE_GRID
        L"asset\\texture\\Ground.png",       // TEXTURE_GROUND
        L"asset\\texture\\lava.png",         // TEXTURE_GROUND_UI
        L"asset\\texture\\partical.png",     // TEXTURE_PARTICLE
        L"asset\\texture\\GOLD.png",         // TEXTURE_COIN
        L"asset\\texture\\BOOM.png",         // TEXTURE_BOMB
        L"asset\\texture\\doctor.png",       // TEXTURE_CHARACTER_DOCTOR
        L"asset\\texture\\soldier.png",      // TEXTURE_CHARACTER_SOLDIER
        L"asset\\texture\\scout.png",        // TEXTURE_CHARACTER_SCOUT
        L"asset\\texture\\bombTech.png"      // TEXTURE_CHARACTER_BOMB_TECH
    };

    if (type >= 0 && type < TEXTURE_TYPE_COUNT) {
        return texturePaths[type];
    }
    return nullptr;
}

// ?取?理名称（用于??）
const char* TextureManager::GetTextureName(TEXTURE_TYPE type) {
    static const char* textureNames[TEXTURE_TYPE_COUNT] = {
        "Default",
        "Start",
        "Goal",
        "Ice",
        "Sand",
        "Grass",
        "Water",
        "Wall",
        "Grid",
        "Ground",
        "GroundUI",
        "Particle",
        "Coin",
        "Bomb",
        "Doctor Info",
        "Soldier Info",
        "Scout Info",
        "BombTech Info"
    };

    if (type >= 0 && type < TEXTURE_TYPE_COUNT) {
        return textureNames[type];
    }
    return "Unknown";
}

// 初始化?理管理器
bool TextureManager::Init() {
    if (s_initialized) {
        OutputDebugStringA("?? ?理管理器已?初始化，跳?重?初始化\n");
        return true;
    }

    OutputDebugStringA("?? ?始初始化?理管理器...\n");

    // 初始化所有?理指??nullptr
    for (int i = 0; i < TEXTURE_TYPE_COUNT; i++) {
        s_textures[i] = nullptr;
    }

    TexMetadata metadata;
    ScratchImage image;
    int successCount = 0;
    int failCount = 0;

    // 加?所有?理
    for (int i = 0; i < TEXTURE_TYPE_COUNT; i++) {
        const wchar_t* filePath = GetTextureFilePath((TEXTURE_TYPE)i);
        if (!filePath) {
            char debug[256];
            sprintf_s(debug, "? ?理?型 %d 没有??的文件路径\n", i);
            OutputDebugStringA(debug);
            failCount++;
            continue;
        }

        HRESULT hr = LoadFromWICFile(filePath, WIC_FLAGS_NONE, &metadata, image);
        if (SUCCEEDED(hr)) {
            hr = CreateShaderResourceView(GetDevice(), image.GetImages(),
                image.GetImageCount(), metadata, &s_textures[i]);

            if (SUCCEEDED(hr)) {
                char debug[256];
                sprintf_s(debug, "? 成功加??理: %s\n", GetTextureName((TEXTURE_TYPE)i));
                OutputDebugStringA(debug);
                successCount++;
            }
            else {
                char debug[256];
                sprintf_s(debug, "? ?建?理?源??失?: %s\n", GetTextureName((TEXTURE_TYPE)i));
                OutputDebugStringA(debug);
                failCount++;
            }
        }
        else {
            char debug[256];
            sprintf_s(debug, "?? 加??理文件失?: %s\n", GetTextureName((TEXTURE_TYPE)i));
            OutputDebugStringA(debug);
            failCount++;
        }
    }

    s_initialized = true;

    char summary[256];
    sprintf_s(summary, "?? ?理管理器初始化完成: 成功 %d, 失? %d\n", successCount, failCount);
    OutputDebugStringA(summary);

    return successCount > 0; // 只要有一个?理加?成功就算初始化成功
}

// 清理?理管理器
void TextureManager::Uninit() {
    if (!s_initialized) {
        return;
    }

    OutputDebugStringA("?? ?始清理?理管理器...\n");

    int releasedCount = 0;
    for (int i = 0; i < TEXTURE_TYPE_COUNT; i++) {
        if (s_textures[i]) {
            s_textures[i]->Release();
            s_textures[i] = nullptr;
            releasedCount++;
        }
    }

    s_initialized = false;

    char debug[256];
    sprintf_s(debug, "?? ?理管理器清理完成: ?放了 %d 个?理\n", releasedCount);
    OutputDebugStringA(debug);
}

// ?取指定?型的?理
ID3D11ShaderResourceView* TextureManager::GetTexture(TEXTURE_TYPE type) {
    if (!s_initialized) {
        OutputDebugStringA("? ?理管理器未初始化！\n");
        return nullptr;
    }

    if (type < 0 || type >= TEXTURE_TYPE_COUNT) {
        char debug[256];
        sprintf_s(debug, "? 无效的?理?型: %d\n", type);
        OutputDebugStringA(debug);
        return nullptr;
    }

    if (!s_textures[type]) {
        char debug[256];
        sprintf_s(debug, "?? ?理未加?: %s (??重新加?)\n", GetTextureName(type));
        OutputDebugStringA(debug);

        // ??重新加??个?理
        ReloadTexture(type);
    }

    return s_textures[type];
}

// ???理是否已加?
bool TextureManager::IsTextureLoaded(TEXTURE_TYPE type) {
    if (type < 0 || type >= TEXTURE_TYPE_COUNT) {
        return false;
    }

    return s_textures[type] != nullptr;
}

// 重新加??个?理
bool TextureManager::ReloadTexture(TEXTURE_TYPE type) {
    if (type < 0 || type >= TEXTURE_TYPE_COUNT) {
        return false;
    }

    // 如果已存在，先?放
    if (s_textures[type]) {
        s_textures[type]->Release();
        s_textures[type] = nullptr;
    }

    const wchar_t* filePath = GetTextureFilePath(type);
    if (!filePath) {
        return false;
    }

    TexMetadata metadata;
    ScratchImage image;

    HRESULT hr = LoadFromWICFile(filePath, WIC_FLAGS_NONE, &metadata, image);
    if (SUCCEEDED(hr)) {
        hr = CreateShaderResourceView(GetDevice(), image.GetImages(),
            image.GetImageCount(), metadata, &s_textures[type]);

        if (SUCCEEDED(hr)) {
            char debug[256];
            sprintf_s(debug, "? 重新加??理成功: %s\n", GetTextureName(type));
            OutputDebugStringA(debug);
            return true;
        }
    }

    char debug[256];
    sprintf_s(debug, "? 重新加??理失?: %s\n", GetTextureName(type));
    OutputDebugStringA(debug);
    return false;
}

