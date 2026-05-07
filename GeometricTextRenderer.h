// GeometricTextRenderer.h
#pragma once
#include "main.h"
#include "renderer.h"
#include <vector>
#include <unordered_map>


// 全局变量
extern XMMATRIX g_WorldMatrix;
extern XMMATRIX g_ViewMatrix;
extern XMMATRIX g_ProjectionMatrix;


// 简化的几何文字渲染系统 - 专注于基础功能
class GeometricTextRenderer {
public:
    // 字符几何体结构
    struct CharacterGeometry {
        wchar_t character;
        float width;
        std::vector<XMFLOAT3> vertices;
        std::vector<uint32_t> indices;
    };

    // 顶点结构体 (与主渲染器兼容)
    struct TextVertex {
        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT4 color;
        XMFLOAT2 texCoord;
    };

    // 初始化和清理
    static void Init();
    static void Uninit();

    // 绘制文本函数
    static void DrawText3D(const XMFLOAT3& position, const wchar_t* text,
        const XMFLOAT4& color, float scale = 1.0f, bool billboard = true);

    static void DrawText2D(float x, float y, const wchar_t* text,
        const XMFLOAT4& color, float scale = 1.0f);

    static void DrawPixelText(float x, float y, const wchar_t* text,
        const XMFLOAT4& color, float scale = 1.0f);

    static void DrawEnhancedPixelText(float x, float y, const wchar_t* text, XMFLOAT4 color, float scale = 1.0f);
    //为标题文本创建边框/阴影效果

    static void DrawTitlePixelText(float x, float y, const wchar_t* text, XMFLOAT4 color, float scale = 1.5f);
    //用于警告的动画闪烁效果

    static void DrawFlickeringPixelText(float x, float y, const wchar_t* text, XMFLOAT4 baseColor, float scale = 1.0f);
    //用于特殊文本的动画颜色循环效果

    static void DrawRainbowPixelText(float x, float y, const wchar_t* text, float scale = 1.0f);

    // Minecraft 风格文字渲染 - 带黑色背景边框的粗体文字
    static void DrawMinecraftText(float x, float y, const wchar_t* text,
        const XMFLOAT4& color, float scale = 2.0f, bool centeredText = false);
    static XMFLOAT2 MeasureText(const wchar_t* text, float scale = 1.0f);

    

private:
    // 创建字符几何体
    static void CreateCharacterGeometry(wchar_t character, CharacterGeometry& geometry);

    static bool CreatePixelCharacter(wchar_t character, bool pixelGrid[9][7]);

    // 构建文本网格
    static void BuildTextMesh(const wchar_t* text, std::vector<TextVertex>& vertices,
        std::vector<uint32_t>& indices, const XMFLOAT4& color);

    // 静态成员变量
    static std::unordered_map<wchar_t, CharacterGeometry> m_characterCache;
    static bool m_initialized;

};
