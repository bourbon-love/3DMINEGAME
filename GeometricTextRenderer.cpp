// GeometricTextRenderer.cpp
#include "GeometricTextRenderer.h"
#include "Camera.h"

// 静态成员变量定义
std::unordered_map<wchar_t, GeometricTextRenderer::CharacterGeometry> GeometricTextRenderer::m_characterCache;
bool GeometricTextRenderer::m_initialized = false;

void GeometricTextRenderer::Init() {
    if (m_initialized) return;

    // 预生成常用字符的几何体
    for (wchar_t c = L'0'; c <= L'9'; c++) {
        CharacterGeometry geom;
        CreateCharacterGeometry(c, geom);
        m_characterCache[c] = geom;
    }

    for (wchar_t c = L'A'; c <= L'Z'; c++) {
        CharacterGeometry geom;
        CreateCharacterGeometry(c, geom);
        m_characterCache[c] = geom;
    }

    // 添加一些基本符号
    wchar_t symbols[] = L"!?.,;:+-*/=()[]{}<>\"'\\|_";
    for (size_t i = 0; i < wcslen(symbols); i++) {
        CharacterGeometry geom;
        CreateCharacterGeometry(symbols[i], geom);
        m_characterCache[symbols[i]] = geom;
    }

    m_initialized = true;
    OutputDebugStringA("GeometricTextRenderer 初始化成功\n");
}

void GeometricTextRenderer::Uninit() {
    m_characterCache.clear();
    m_initialized = false;

   
}

void GeometricTextRenderer::CreateCharacterGeometry(wchar_t character, CharacterGeometry& geometry) {
    geometry.character = character;
    geometry.vertices.clear();
    geometry.indices.clear();

    // 像素风格参数
    float scale = 4.0f;
    float pixelSize = 0.12f * scale; // 每个像素的大小
    float gap = 0.02f * scale;      // 像素间的小间隙，增强像素感

    // 字符的基础宽度
    float width = 0.8f * scale;

    // 创建字符的像素网格
    bool pixelGrid[9][7] = { false };
    CreatePixelCharacter(character, pixelGrid);

    // 根据像素网格创建几何体
    int pixelCount = 0;
    for (int y = 0; y < 9; y++) {
        for (int x = 0; x < 7; x++) {
            if (pixelGrid[y][x]) {
                //// 计算像素位置
                //float pixelX = x * (pixelSize + gap);
                //float pixelY = (8 - y) * (pixelSize + gap); // 反转Y轴，使字符正确显示
                 // 计算像素位置 - 修正Y轴方向
                float pixelX = x * (pixelSize + gap);
                // 修正：直接使用y而不是(8-y)，或者根据需要调整
                float pixelY = y * (pixelSize + gap); // 不再反转Y轴
                // 添加一个像素(方块)的顶点
                int baseIndex = geometry.vertices.size();

                // 像素的四个角
                geometry.vertices.push_back(XMFLOAT3(pixelX, pixelY, 0.0f));                        // 左下
                geometry.vertices.push_back(XMFLOAT3(pixelX + pixelSize, pixelY, 0.0f));            // 右下
                geometry.vertices.push_back(XMFLOAT3(pixelX + pixelSize, pixelY + pixelSize, 0.0f));// 右上
                geometry.vertices.push_back(XMFLOAT3(pixelX, pixelY + pixelSize, 0.0f));            // 左上

                // 添加像素的索引(两个三角形)
                geometry.indices.push_back(baseIndex);
                geometry.indices.push_back(baseIndex + 1);
                geometry.indices.push_back(baseIndex + 2);

                geometry.indices.push_back(baseIndex);
                geometry.indices.push_back(baseIndex + 2);
                geometry.indices.push_back(baseIndex + 3);

                pixelCount++;
            }
        }
    }

    // 设置字符宽度，确保像素字符间有适当间距
    geometry.width = pixelCount > 0 ? 7 * (pixelSize + gap) + gap : (0.5f * scale);
}

bool GeometricTextRenderer::CreatePixelCharacter(wchar_t character, bool pixelGrid[9][7])
{
    

        // Initialize all pixels to empty
        for (int y = 0; y < 9; y++) {
            for (int x = 0; x < 7; x++) {
                pixelGrid[y][x] = false;
            }
        }

        // Define pixel patterns for each supported character with improved readability
        switch (character) {
            // Uppercase Letters
        case L'A': case L'a': {
            // More distinct 'A' with thicker strokes and clearer shape
            pixelGrid[0][3] = true;                                   // Top point
            pixelGrid[1][2] = pixelGrid[1][4] = true;                 // Upper arms
            pixelGrid[2][1] = pixelGrid[2][5] = true;                 // Middle arms
            pixelGrid[3][0] = pixelGrid[3][6] = true;                 // Lower arms
            pixelGrid[4][0] = pixelGrid[4][1] = pixelGrid[4][2] = pixelGrid[4][3] =
                pixelGrid[4][4] = pixelGrid[4][5] = pixelGrid[4][6] = true; // Horizontal bar
            pixelGrid[5][0] = pixelGrid[5][6] = true;                 // Left and right sides
            pixelGrid[6][0] = pixelGrid[6][6] = true;                 // Bottom
            pixelGrid[7][0] = pixelGrid[7][6] = true;                 // Extended bottom
            return true;
        }
        case L'B': case L'b': {
            // Improved 'B' with more distinct rounded parts
            pixelGrid[0][0] = pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][0] = pixelGrid[1][5] = true;
            pixelGrid[2][0] = pixelGrid[2][5] = true;
            pixelGrid[3][0] = pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[4][0] = pixelGrid[4][5] = true;
            pixelGrid[5][0] = pixelGrid[5][5] = true;
            pixelGrid[6][0] = pixelGrid[6][5] = true;
            pixelGrid[7][0] = pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'C': case L'c': {
            // Enhanced 'C' with clearer curve
            pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;    // Top arc
            pixelGrid[1][1] = pixelGrid[1][5] = true;                      // Upper corners
            pixelGrid[2][0] = true;                                        // Left edge
            pixelGrid[3][0] = true;                                        // Left edge
            pixelGrid[4][0] = true;                                        // Left edge
            pixelGrid[5][0] = true;                                        // Left edge
            pixelGrid[6][1] = pixelGrid[6][5] = true;                      // Lower corners
            pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;    // Bottom arc
            return true;
        }
        case L'D': case L'd': {
            // Improved 'D' with better defined curve
            pixelGrid[0][0] = pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][0] = pixelGrid[1][5] = true;
            pixelGrid[2][0] = pixelGrid[2][6] = true;
            pixelGrid[3][0] = pixelGrid[3][6] = true;
            pixelGrid[4][0] = pixelGrid[4][6] = true;
            pixelGrid[5][0] = pixelGrid[5][6] = true;
            pixelGrid[6][0] = pixelGrid[6][5] = true;
            pixelGrid[7][0] = pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'E': case L'e': {
            // Enhanced 'E' with consistent thickness
            pixelGrid[0][0] = pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = true;
            pixelGrid[1][0] = true;
            pixelGrid[2][0] = true;
            pixelGrid[3][0] = pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[4][0] = true;
            pixelGrid[5][0] = true;
            pixelGrid[6][0] = true;
            pixelGrid[7][0] = pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = pixelGrid[7][5] = true;
            return true;
        }
        case L'F': case L'f': {
            // Improved 'F' with consistent thickness
            pixelGrid[0][0] = pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = true;
            pixelGrid[1][0] = true;
            pixelGrid[2][0] = true;
            pixelGrid[3][0] = pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[4][0] = true;
            pixelGrid[5][0] = true;
            pixelGrid[6][0] = true;
            pixelGrid[7][0] = true;
            return true;
        }
        case L'G': case L'g': {
            // Enhanced 'G' with better defined curve and tail
            pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][1] = pixelGrid[1][5] = true;
            pixelGrid[2][0] = true;
            pixelGrid[3][0] = true;
            pixelGrid[4][0] = pixelGrid[4][4] = pixelGrid[4][5] = pixelGrid[4][6] = true;
            pixelGrid[5][0] = pixelGrid[5][6] = true;
            pixelGrid[6][1] = pixelGrid[6][5] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'H': case L'h': {
            // Improved 'H' with consistent thickness
            pixelGrid[0][0] = pixelGrid[0][6] = true;
            pixelGrid[1][0] = pixelGrid[1][6] = true;
            pixelGrid[2][0] = pixelGrid[2][6] = true;
            pixelGrid[3][0] = pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = pixelGrid[3][5] = pixelGrid[3][6] = true;
            pixelGrid[4][0] = pixelGrid[4][6] = true;
            pixelGrid[5][0] = pixelGrid[5][6] = true;
            pixelGrid[6][0] = pixelGrid[6][6] = true;
            pixelGrid[7][0] = pixelGrid[7][6] = true;
            return true;
        }
        case L'I': case L'i': {
            // Enhanced 'I' with serifs for better readability
            pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = true;
            pixelGrid[1][3] = true;
            pixelGrid[2][3] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][3] = true;
            pixelGrid[6][3] = true;
            pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = pixelGrid[7][5] = true;
            return true;
        }
        case L'J': case L'j': {
            // Improved 'J' with better hook
            pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = pixelGrid[0][6] = true;
            pixelGrid[1][5] = true;
            pixelGrid[2][5] = true;
            pixelGrid[3][5] = true;
            pixelGrid[4][5] = true;
            pixelGrid[5][1] = pixelGrid[5][5] = true;
            pixelGrid[6][1] = pixelGrid[6][5] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'K': case L'k': {
            // Enhanced 'K' with clearer diagonal strokes
            pixelGrid[0][0] = pixelGrid[0][5] = true;
            pixelGrid[1][0] = pixelGrid[1][4] = true;
            pixelGrid[2][0] = pixelGrid[2][3] = true;
            pixelGrid[3][0] = pixelGrid[3][2] = true;
            pixelGrid[4][0] = pixelGrid[4][2] = true;
            pixelGrid[5][0] = pixelGrid[5][3] = true;
            pixelGrid[6][0] = pixelGrid[6][4] = true;
            pixelGrid[7][0] = pixelGrid[7][5] = true;
            return true;
        }
        case L'L': case L'l': {
            // Improved 'L' with consistent thickness
            pixelGrid[0][0] = true;
            pixelGrid[1][0] = true;
            pixelGrid[2][0] = true;
            pixelGrid[3][0] = true;
            pixelGrid[4][0] = true;
            pixelGrid[5][0] = true;
            pixelGrid[6][0] = true;
            pixelGrid[7][0] = pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = pixelGrid[7][5] = true;
            return true;
        }
        case L'M': case L'm': {
            // Enhanced 'M' with clearer middle diagonal
            pixelGrid[0][0] = pixelGrid[0][6] = true;
            pixelGrid[1][0] = pixelGrid[1][1] = pixelGrid[1][5] = pixelGrid[1][6] = true;
            pixelGrid[2][0] = pixelGrid[2][2] = pixelGrid[2][4] = pixelGrid[2][6] = true;
            pixelGrid[3][0] = pixelGrid[3][3] = pixelGrid[3][6] = true;
            pixelGrid[4][0] = pixelGrid[4][6] = true;
            pixelGrid[5][0] = pixelGrid[5][6] = true;
            pixelGrid[6][0] = pixelGrid[6][6] = true;
            pixelGrid[7][0] = pixelGrid[7][6] = true;
            return true;
        }
        case L'N': case L'n': {
            // Improved 'N' with clearer diagonal
            pixelGrid[0][0] = pixelGrid[0][6] = true;
            pixelGrid[1][0] = pixelGrid[1][1] = pixelGrid[1][6] = true;
            pixelGrid[2][0] = pixelGrid[2][2] = pixelGrid[2][6] = true;
            pixelGrid[3][0] = pixelGrid[3][3] = pixelGrid[3][6] = true;
            pixelGrid[4][0] = pixelGrid[4][4] = pixelGrid[4][6] = true;
            pixelGrid[5][0] = pixelGrid[5][5] = pixelGrid[5][6] = true;
            pixelGrid[6][0] = pixelGrid[6][6] = true;
            pixelGrid[7][0] = pixelGrid[7][6] = true;
            return true;
        }
        case L'O': case L'o': {
            // Enhanced 'O' with rounder shape
            pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][1] = pixelGrid[1][5] = true;
            pixelGrid[2][0] = pixelGrid[2][6] = true;
            pixelGrid[3][0] = pixelGrid[3][6] = true;
            pixelGrid[4][0] = pixelGrid[4][6] = true;
            pixelGrid[5][0] = pixelGrid[5][6] = true;
            pixelGrid[6][1] = pixelGrid[6][5] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'P': case L'p': {
            // Improved 'P' with better loop
            pixelGrid[0][0] = pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][0] = pixelGrid[1][5] = true;
            pixelGrid[2][0] = pixelGrid[2][5] = true;
            pixelGrid[3][0] = pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[4][0] = true;
            pixelGrid[5][0] = true;
            pixelGrid[6][0] = true;
            pixelGrid[7][0] = true;
            return true;
        }
        case L'Q': case L'q': {
            // Enhanced 'Q' with better tail
            pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][1] = pixelGrid[1][5] = true;
            pixelGrid[2][0] = pixelGrid[2][6] = true;
            pixelGrid[3][0] = pixelGrid[3][6] = true;
            pixelGrid[4][0] = pixelGrid[4][6] = true;
            pixelGrid[5][0] = pixelGrid[5][4] = pixelGrid[5][6] = true;
            pixelGrid[6][1] = pixelGrid[6][5] = pixelGrid[6][6] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = pixelGrid[7][7] = true;
            return true;
        }
        case L'R': case L'r': {
            // Improved 'R' with better leg
            pixelGrid[0][0] = pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][0] = pixelGrid[1][5] = true;
            pixelGrid[2][0] = pixelGrid[2][5] = true;
            pixelGrid[3][0] = pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[4][0] = pixelGrid[4][3] = true;
            pixelGrid[5][0] = pixelGrid[5][4] = true;
            pixelGrid[6][0] = pixelGrid[6][5] = true;
            pixelGrid[7][0] = pixelGrid[7][6] = true;
            return true;
        }
        case L'S': case L's': {
            // Enhanced 'S' with more balanced curves
            pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = true;
            pixelGrid[1][0] = pixelGrid[1][6] = true;
            pixelGrid[2][0] = true;
            pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[4][5] = pixelGrid[4][6] = true;
            pixelGrid[5][6] = true;
            pixelGrid[6][0] = pixelGrid[6][6] = true;
            pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = pixelGrid[7][5] = true;
            return true;
        }
        case L'T': case L't': {
            // Improved 'T' with consistent thickness
            pixelGrid[0][0] = pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = pixelGrid[0][6] = true;
            pixelGrid[1][3] = true;
            pixelGrid[2][3] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][3] = true;
            pixelGrid[6][3] = true;
            pixelGrid[7][3] = true;
            return true;
        }
        case L'U': case L'u': {
            // Enhanced 'U' with better curve
            pixelGrid[0][0] = pixelGrid[0][6] = true;
            pixelGrid[1][0] = pixelGrid[1][6] = true;
            pixelGrid[2][0] = pixelGrid[2][6] = true;
            pixelGrid[3][0] = pixelGrid[3][6] = true;
            pixelGrid[4][0] = pixelGrid[4][6] = true;
            pixelGrid[5][0] = pixelGrid[5][6] = true;
            pixelGrid[6][1] = pixelGrid[6][5] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'V': case L'v': {
            // Improved 'V' with smoother diagonal
            pixelGrid[0][0] = pixelGrid[0][6] = true;
            pixelGrid[1][0] = pixelGrid[1][6] = true;
            pixelGrid[2][0] = pixelGrid[2][6] = true;
            pixelGrid[3][1] = pixelGrid[3][5] = true;
            pixelGrid[4][1] = pixelGrid[4][5] = true;
            pixelGrid[5][2] = pixelGrid[5][4] = true;
            pixelGrid[6][2] = pixelGrid[6][4] = true;
            pixelGrid[7][3] = true;
            return true;
        }
        case L'W': case L'w': {
            // Enhanced 'W' with clearer middle peaks
            pixelGrid[0][0] = pixelGrid[0][6] = true;
            pixelGrid[1][0] = pixelGrid[1][6] = true;
            pixelGrid[2][0] = pixelGrid[2][6] = true;
            pixelGrid[3][0] = pixelGrid[3][3] = pixelGrid[3][6] = true;
            pixelGrid[4][0] = pixelGrid[4][3] = pixelGrid[4][6] = true;
            pixelGrid[5][1] = pixelGrid[5][3] = pixelGrid[5][5] = true;
            pixelGrid[6][1] = pixelGrid[6][5] = true;
            pixelGrid[7][2] = pixelGrid[7][4] = true;
            return true;
        }
        case L'X': case L'x': {
            // Improved 'X' with clearer diagonals
            pixelGrid[0][0] = pixelGrid[0][6] = true;
            pixelGrid[1][1] = pixelGrid[1][5] = true;
            pixelGrid[2][2] = pixelGrid[2][4] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][2] = pixelGrid[5][4] = true;
            pixelGrid[6][1] = pixelGrid[6][5] = true;
            pixelGrid[7][0] = pixelGrid[7][6] = true;
            return true;
        }
        case L'Y': case L'y': {
            // Enhanced 'Y' with clearer fork
            pixelGrid[0][0] = pixelGrid[0][6] = true;
            pixelGrid[1][1] = pixelGrid[1][5] = true;
            pixelGrid[2][2] = pixelGrid[2][4] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][3] = true;
            pixelGrid[6][3] = true;
            pixelGrid[7][3] = true;
            return true;
        }
        case L'Z': case L'z': {
            // Improved 'Z' with clearer diagonal
            pixelGrid[0][0] = pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = pixelGrid[0][6] = true;
            pixelGrid[1][5] = pixelGrid[1][6] = true;
            pixelGrid[2][4] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][2] = true;
            pixelGrid[5][1] = true;
            pixelGrid[6][0] = true;
            pixelGrid[7][0] = pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = pixelGrid[7][5] = pixelGrid[7][6] = true;
            return true;
        }

                 // Numbers with improved visibility
        case L'0': {
            // Enhanced '0' with clearer oval shape and diagonal line
            pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][1] = pixelGrid[1][5] = true;
            pixelGrid[2][0] = pixelGrid[2][5] = pixelGrid[2][6] = true;
            pixelGrid[3][0] = pixelGrid[3][4] = pixelGrid[3][6] = true;
            pixelGrid[4][0] = pixelGrid[4][3] = pixelGrid[4][6] = true;
            pixelGrid[5][0] = pixelGrid[5][2] = pixelGrid[5][6] = true;
            pixelGrid[6][1] = pixelGrid[6][5] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'1': {
            // Improved '1' with clearer shape
            pixelGrid[0][3] = true;
            pixelGrid[1][2] = pixelGrid[1][3] = true;
            pixelGrid[2][1] = pixelGrid[2][3] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][3] = true;
            pixelGrid[6][3] = true;
            pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = pixelGrid[7][5] = true;
            return true;
        }
        case L'2': {
            // Enhanced '2' with clearer curve and diagonal
            pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = true;
            pixelGrid[1][0] = pixelGrid[1][6] = true;
            pixelGrid[2][6] = true;
            pixelGrid[3][5] = true;
            pixelGrid[4][4] = true;
            pixelGrid[5][3] = true;
            pixelGrid[6][2] = true;
            pixelGrid[7][0] = pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = pixelGrid[7][5] = pixelGrid[7][6] = true;
            return true;
        }
        case L'3': {
            // Improved '3' with better curves
            pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][0] = pixelGrid[1][5] = true;
            pixelGrid[2][5] = true;
            pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[4][5] = true;
            pixelGrid[5][6] = true;
            pixelGrid[6][0] = pixelGrid[6][5] = true;
            pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'4': {
            // Enhanced '4' with clearer form
            pixelGrid[0][4] = true;
            pixelGrid[1][3] = pixelGrid[1][4] = true;
            pixelGrid[2][2] = pixelGrid[2][4] = true;
            pixelGrid[3][1] = pixelGrid[3][4] = true;
            pixelGrid[4][0] = pixelGrid[4][4] = true;
            pixelGrid[5][0] = pixelGrid[5][1] = pixelGrid[5][2] = pixelGrid[5][3] = pixelGrid[5][4] = pixelGrid[5][5] = pixelGrid[5][6] = true;
            pixelGrid[6][4] = true;
            pixelGrid[7][4] = true;
            return true;
        }
        case L'5': {
            // Improved '5' with more distinct shape
            pixelGrid[0][0] = pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = pixelGrid[0][6] = true;
            pixelGrid[1][0] = true;
            pixelGrid[2][0] = true;
            pixelGrid[3][0] = pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[4][5] = true;
            pixelGrid[5][6] = true;
            pixelGrid[6][0] = pixelGrid[6][5] = true;
            pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'6': {
            // Enhanced '6' with clearer loop
            pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][1] = true;
            pixelGrid[2][0] = true;
            pixelGrid[3][0] = pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[4][0] = pixelGrid[4][5] = true;
            pixelGrid[5][0] = pixelGrid[5][5] = true;
            pixelGrid[6][1] = pixelGrid[6][4] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = true;
            return true;
        }
        case L'7': {
            // Improved '7' with clearer angle
            pixelGrid[0][0] = pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = pixelGrid[0][6] = true;
            pixelGrid[1][6] = true;
            pixelGrid[2][5] = true;
            pixelGrid[3][4] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][2] = true;
            pixelGrid[6][2] = true;
            pixelGrid[7][2] = true;
            return true;
        }
        case L'8': {
            // Enhanced '8' with clearer loops
            pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = true;
            pixelGrid[1][0] = pixelGrid[1][6] = true;
            pixelGrid[2][0] = pixelGrid[2][6] = true;
            pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = pixelGrid[3][5] = true;
            pixelGrid[4][0] = pixelGrid[4][6] = true;
            pixelGrid[5][0] = pixelGrid[5][6] = true;
            pixelGrid[6][0] = pixelGrid[6][6] = true;
            pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = pixelGrid[7][5] = true;
            return true;
        }
        case L'9': {
            // Improved '9' with clearer loop
            pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][0] = pixelGrid[1][5] = true;
            pixelGrid[2][0] = pixelGrid[2][5] = true;
            pixelGrid[3][0] = pixelGrid[3][5] = true;
            pixelGrid[4][1] = pixelGrid[4][2] = pixelGrid[4][3] = pixelGrid[4][4] = pixelGrid[4][5] = pixelGrid[4][6] = true;
            pixelGrid[5][6] = true;
            pixelGrid[6][5] = true;
            pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }

                 // Special characters with improved visibility
        case L'.': {
            // Larger, more visible dot
            pixelGrid[6][2] = pixelGrid[6][3] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = true;
            return true;
        }
        case L',': {
            // Enhanced comma with better tail
            pixelGrid[6][2] = pixelGrid[6][3] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = true;
            pixelGrid[8][1] = pixelGrid[8][2] = true;
            return true;
        }
        case L'!': {
            // Improved exclamation mark with clearer dot
            pixelGrid[0][3] = true;
            pixelGrid[1][3] = true;
            pixelGrid[2][3] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][3] = true;
            // Gap
            pixelGrid[7][3] = pixelGrid[7][4] = true;
            pixelGrid[8][3] = pixelGrid[8][4] = true;
            return true;
        }
        case L'?': {
            // Enhanced question mark with clearer dot
            pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][0] = pixelGrid[1][5] = true;
            pixelGrid[2][5] = true;
            pixelGrid[3][4] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][3] = true;
            // Gap
            pixelGrid[7][3] = pixelGrid[7][4] = true;
            pixelGrid[8][3] = pixelGrid[8][4] = true;
            return true;
        }
        case L':': {
            // Clearer colon with larger dots
            pixelGrid[2][3] = pixelGrid[2][4] = true;
            pixelGrid[3][3] = pixelGrid[3][4] = true;

            pixelGrid[6][3] = pixelGrid[6][4] = true;
            pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L';': {
            // Enhanced semicolon with larger dots and better tail
            pixelGrid[2][3] = pixelGrid[2][4] = true;
            pixelGrid[3][3] = pixelGrid[3][4] = true;

            pixelGrid[6][3] = pixelGrid[6][4] = true;
            pixelGrid[7][3] = pixelGrid[7][4] = true;
            pixelGrid[8][2] = pixelGrid[8][3] = true;
            return true;
        }
        case L'+': {
            // Improved plus with clearer lines
            pixelGrid[2][3] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][1] = pixelGrid[4][2] = pixelGrid[4][3] = pixelGrid[4][4] = pixelGrid[4][5] = true;
            pixelGrid[5][3] = true;
            pixelGrid[6][3] = true;
            return true;
        }
        case L'-': {
            // Wider minus sign
            pixelGrid[4][1] = pixelGrid[4][2] = pixelGrid[4][3] = pixelGrid[4][4] = pixelGrid[4][5] = true;
            return true;
        }
        case L'*': {
            // Enhanced asterisk with clearer spokes
            pixelGrid[2][1] = pixelGrid[2][5] = true;
            pixelGrid[3][2] = pixelGrid[3][4] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][2] = pixelGrid[5][4] = true;
            pixelGrid[6][1] = pixelGrid[6][5] = true;
            return true;
        }
        case L'/': {
            // Clearer forward slash
            pixelGrid[0][6] = true;
            pixelGrid[1][5] = true;
            pixelGrid[2][4] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][2] = true;
            pixelGrid[5][1] = true;
            pixelGrid[6][0] = true;
            return true;
        }
        case L'\\': {
            // Clearer backslash
            pixelGrid[0][0] = true;
            pixelGrid[1][1] = true;
            pixelGrid[2][2] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][4] = true;
            pixelGrid[5][5] = true;
            pixelGrid[6][6] = true;
            return true;
        }
        case L'=': {
            // Enhanced equals with wider lines
            pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = pixelGrid[3][5] = true;
            pixelGrid[5][1] = pixelGrid[5][2] = pixelGrid[5][3] = pixelGrid[5][4] = pixelGrid[5][5] = true;
            return true;
        }
        case L'_': {
            // Wider underscore
            pixelGrid[7][0] = pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = pixelGrid[7][5] = pixelGrid[7][6] = true;
            return true;
        }
        case L'(': {
            // Improved left parenthesis with smoother curve
            pixelGrid[0][4] = pixelGrid[0][5] = true;
            pixelGrid[1][3] = true;
            pixelGrid[2][2] = true;
            pixelGrid[3][2] = true;
            pixelGrid[4][2] = true;
            pixelGrid[5][2] = true;
            pixelGrid[6][3] = true;
            pixelGrid[7][4] = pixelGrid[7][5] = true;
            return true;
        }
        case L')': {
            // Improved right parenthesis with smoother curve
            pixelGrid[0][1] = pixelGrid[0][2] = true;
            pixelGrid[1][3] = true;
            pixelGrid[2][4] = true;
            pixelGrid[3][4] = true;
            pixelGrid[4][4] = true;
            pixelGrid[5][4] = true;
            pixelGrid[6][3] = true;
            pixelGrid[7][1] = pixelGrid[7][2] = true;
            return true;
        }
        case L'[': {
            // Enhanced left bracket with clearer shape
            pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = true;
            pixelGrid[1][2] = true;
            pixelGrid[2][2] = true;
            pixelGrid[3][2] = true;
            pixelGrid[4][2] = true;
            pixelGrid[5][2] = true;
            pixelGrid[6][2] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = pixelGrid[7][5] = true;
            return true;
        }
        case L']': {
            // Enhanced right bracket with clearer shape
            pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][4] = true;
            pixelGrid[2][4] = true;
            pixelGrid[3][4] = true;
            pixelGrid[4][4] = true;
            pixelGrid[5][4] = true;
            pixelGrid[6][4] = true;
            pixelGrid[7][1] = pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'{': {
            // Improved left brace with clearer curves
            pixelGrid[0][4] = true;
            pixelGrid[1][3] = true;
            pixelGrid[2][3] = true;
            pixelGrid[3][2] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][3] = true;
            pixelGrid[6][4] = true;
            return true;
        }
        case L'}': {
            // Improved right brace with clearer curves
            pixelGrid[0][2] = true;
            pixelGrid[1][3] = true;
            pixelGrid[2][3] = true;
            pixelGrid[3][4] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][3] = true;
            pixelGrid[6][2] = true;
            return true;
        }
        case L'<': {
            // Enhanced less than with clearer shape
            pixelGrid[2][5] = true;
            pixelGrid[3][4] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][2] = true;
            pixelGrid[6][3] = true;
            pixelGrid[7][4] = true;
            pixelGrid[8][5] = true;
            return true;
        }
        case L'>': {
            // Enhanced greater than with clearer shape
            pixelGrid[2][1] = true;
            pixelGrid[3][2] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][4] = true;
            pixelGrid[6][3] = true;
            pixelGrid[7][2] = true;
            pixelGrid[8][1] = true;
            return true;
        }
        case L'"': {
            // Improved quotation mark with clearer shape
            pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][4] = pixelGrid[0][5] = true;
            pixelGrid[1][1] = pixelGrid[1][2] = pixelGrid[1][4] = pixelGrid[1][5] = true;
            pixelGrid[2][1] = pixelGrid[2][2] = pixelGrid[2][4] = pixelGrid[2][5] = true;
            return true;
        }
        case L'\'': {
            // Enhanced apostrophe with clearer shape
            pixelGrid[0][3] = true;
            pixelGrid[1][3] = true;
            pixelGrid[2][3] = true;
            return true;
        }
        case L'|': {
            // Thicker vertical bar
            pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][3] = pixelGrid[1][4] = true;
            pixelGrid[2][3] = pixelGrid[2][4] = true;
            pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[4][3] = pixelGrid[4][4] = true;
            pixelGrid[5][3] = pixelGrid[5][4] = true;
            pixelGrid[6][3] = pixelGrid[6][4] = true;
            pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L' ': {
            // Space - no pixels needed
            return true;
        }
        case L'#': {
            // Enhanced hash with clearer grid
            pixelGrid[1][1] = pixelGrid[1][5] = true;
            pixelGrid[2][1] = pixelGrid[2][5] = true;
            pixelGrid[3][0] = pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = pixelGrid[3][5] = pixelGrid[3][6] = true;
            pixelGrid[4][1] = pixelGrid[4][5] = true;
            pixelGrid[5][1] = pixelGrid[5][5] = true;
            pixelGrid[6][0] = pixelGrid[6][1] = pixelGrid[6][2] = pixelGrid[6][3] = pixelGrid[6][4] = pixelGrid[6][5] = pixelGrid[6][6] = true;
            pixelGrid[7][1] = pixelGrid[7][5] = true;
            return true;
        }
        case L'@': {
            // Improved at symbol with clearer curves
            pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][1] = pixelGrid[1][5] = true;
            pixelGrid[2][0] = pixelGrid[2][3] = pixelGrid[2][4] = pixelGrid[2][5] = pixelGrid[2][6] = true;
            pixelGrid[3][0] = pixelGrid[3][3] = pixelGrid[3][6] = true;
            pixelGrid[4][0] = pixelGrid[4][3] = pixelGrid[4][5] = pixelGrid[4][6] = true;
            pixelGrid[5][0] = pixelGrid[5][3] = pixelGrid[5][4] = pixelGrid[5][6] = true;
            pixelGrid[6][1] = pixelGrid[6][2] = pixelGrid[6][3] = pixelGrid[6][5] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'&': {
            // Enhanced ampersand with clearer curves
            pixelGrid[0][2] = pixelGrid[0][3] = true;
            pixelGrid[1][1] = pixelGrid[1][4] = true;
            pixelGrid[2][1] = pixelGrid[2][4] = true;
            pixelGrid[3][2] = pixelGrid[3][3] = true;
            pixelGrid[4][1] = pixelGrid[4][3] = pixelGrid[4][5] = true;
            pixelGrid[5][0] = pixelGrid[5][4] = true;
            pixelGrid[6][1] = pixelGrid[6][5] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = pixelGrid[7][6] = true;
            return true;
        }
        case L'^': {
            // Improved caret with clearer point
            pixelGrid[1][3] = true;
            pixelGrid[2][2] = pixelGrid[2][4] = true;
            pixelGrid[3][1] = pixelGrid[3][5] = true;
            return true;
        }
        case L'%': {
            // Enhanced percent with clearer diagonal and dots
            pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][5] = true;
            pixelGrid[1][0] = pixelGrid[1][3] = pixelGrid[1][4] = true;
            pixelGrid[2][0] = pixelGrid[2][1] = pixelGrid[2][4] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][2] = true;
            pixelGrid[5][1] = pixelGrid[5][4] = pixelGrid[5][5] = true;
            pixelGrid[6][0] = pixelGrid[6][3] = pixelGrid[6][6] = true;
            pixelGrid[7][2] = pixelGrid[7][5] = pixelGrid[7][6] = true;
            return true;
        }

                 // 中文冒号
        case L'：': {
            // Improved colon with larger dots
            pixelGrid[2][2] = pixelGrid[2][3] = pixelGrid[2][4] = true;
            pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = true;

            pixelGrid[5][2] = pixelGrid[5][3] = pixelGrid[5][4] = true;
            pixelGrid[6][2] = pixelGrid[6][3] = pixelGrid[6][4] = true;
            return true;
        }

                 // 中文标点和常用字符
        case L'。': {
            // Chinese period
            pixelGrid[5][2] = pixelGrid[5][3] = pixelGrid[5][4] = true;
            pixelGrid[6][2] = pixelGrid[6][3] = pixelGrid[6][4] = true;
            return true;
        }
        case L'，': {
            // Chinese comma
            pixelGrid[5][3] = pixelGrid[5][4] = true;
            pixelGrid[6][3] = pixelGrid[6][4] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = true;
            return true;
        }
        case L'！': {
            // Chinese exclamation
            pixelGrid[0][3] = pixelGrid[0][4] = true;
            pixelGrid[1][3] = pixelGrid[1][4] = true;
            pixelGrid[2][3] = pixelGrid[2][4] = true;
            pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[4][3] = pixelGrid[4][4] = true;
            pixelGrid[6][3] = pixelGrid[6][4] = true;
            pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'？': {
            // Chinese question mark
            pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][3] = pixelGrid[0][4] = pixelGrid[0][5] = true;
            pixelGrid[1][0] = pixelGrid[1][6] = true;
            pixelGrid[2][6] = true;
            pixelGrid[3][5] = true;
            pixelGrid[4][4] = true;
            pixelGrid[5][3] = true;
            pixelGrid[7][3] = true;
            return true;
        }

                 // Extended characters for better visual appeal
        case L'☺': {
            // Simple smiley face
            pixelGrid[1][2] = pixelGrid[1][3] = pixelGrid[1][4] = true;
            pixelGrid[2][1] = pixelGrid[2][5] = true;
            pixelGrid[3][0] = pixelGrid[3][2] = pixelGrid[3][4] = pixelGrid[3][6] = true;
            pixelGrid[4][0] = pixelGrid[4][6] = true;
            pixelGrid[5][0] = pixelGrid[5][3] = pixelGrid[5][6] = true;
            pixelGrid[6][1] = pixelGrid[6][2] = pixelGrid[6][4] = pixelGrid[6][5] = true;
            pixelGrid[7][2] = pixelGrid[7][3] = pixelGrid[7][4] = true;
            return true;
        }
        case L'♥': {
            // Heart
            pixelGrid[0][1] = pixelGrid[0][2] = pixelGrid[0][4] = pixelGrid[0][5] = true;
            pixelGrid[1][0] = pixelGrid[1][1] = pixelGrid[1][2] = pixelGrid[1][3] = pixelGrid[1][4] = pixelGrid[1][5] = pixelGrid[1][6] = true;
            pixelGrid[2][0] = pixelGrid[2][1] = pixelGrid[2][2] = pixelGrid[2][3] = pixelGrid[2][4] = pixelGrid[2][5] = pixelGrid[2][6] = true;
            pixelGrid[3][0] = pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = pixelGrid[3][5] = pixelGrid[3][6] = true;
            pixelGrid[4][1] = pixelGrid[4][2] = pixelGrid[4][3] = pixelGrid[4][4] = pixelGrid[4][5] = true;
            pixelGrid[5][2] = pixelGrid[5][3] = pixelGrid[5][4] = true;
            pixelGrid[6][3] = true;
            return true;
        }
        case L'♣': {
            // Club
            pixelGrid[0][3] = true;
            pixelGrid[1][2] = pixelGrid[1][3] = pixelGrid[1][4] = true;
            pixelGrid[2][1] = pixelGrid[2][2] = pixelGrid[2][3] = pixelGrid[2][4] = pixelGrid[2][5] = true;
            pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[4][1] = pixelGrid[4][5] = true;
            pixelGrid[5][1] = pixelGrid[5][2] = pixelGrid[5][4] = pixelGrid[5][5] = true;
            pixelGrid[6][2] = pixelGrid[6][3] = pixelGrid[6][4] = true;
            pixelGrid[7][3] = true;
            return true;
        }
        case L'♦': {
            // Diamond
            pixelGrid[0][3] = true;
            pixelGrid[1][2] = pixelGrid[1][4] = true;
            pixelGrid[2][1] = pixelGrid[2][5] = true;
            pixelGrid[3][0] = pixelGrid[3][6] = true;
            pixelGrid[4][1] = pixelGrid[4][5] = true;
            pixelGrid[5][2] = pixelGrid[5][4] = true;
            pixelGrid[6][3] = true;
            return true;
        }
        case L'♠': {
            // Spade
            pixelGrid[0][3] = true;
            pixelGrid[1][2] = pixelGrid[1][3] = pixelGrid[1][4] = true;
            pixelGrid[2][1] = pixelGrid[2][2] = pixelGrid[2][3] = pixelGrid[2][4] = pixelGrid[2][5] = true;
            pixelGrid[3][0] = pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = pixelGrid[3][5] = pixelGrid[3][6] = true;
            pixelGrid[4][2] = pixelGrid[4][3] = pixelGrid[4][4] = true;
            pixelGrid[5][2] = pixelGrid[5][4] = true;
            pixelGrid[6][1] = pixelGrid[6][2] = pixelGrid[6][3] = pixelGrid[6][4] = pixelGrid[6][5] = true;
            pixelGrid[7][3] = true;
            return true;
        }
        case L'→': {
            // Right arrow
            pixelGrid[3][0] = pixelGrid[3][1] = pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = true;
            pixelGrid[2][3] = pixelGrid[2][4] = pixelGrid[2][5] = true;
            pixelGrid[4][3] = pixelGrid[4][4] = pixelGrid[4][5] = true;
            return true;
        }
        case L'←': {
            // Left arrow
            pixelGrid[3][2] = pixelGrid[3][3] = pixelGrid[3][4] = pixelGrid[3][5] = pixelGrid[3][6] = true;
            pixelGrid[2][1] = pixelGrid[2][2] = pixelGrid[2][3] = true;
            pixelGrid[4][1] = pixelGrid[4][2] = pixelGrid[4][3] = true;
            return true;
        }
        case L'↑': {
            // Up arrow
            pixelGrid[0][3] = true;
            pixelGrid[1][2] = pixelGrid[1][3] = pixelGrid[1][4] = true;
            pixelGrid[2][1] = pixelGrid[2][3] = pixelGrid[2][5] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][3] = true;
            pixelGrid[6][3] = true;
            return true;
        }
        case L'↓': {
            // Down arrow
            pixelGrid[1][3] = true;
            pixelGrid[2][3] = true;
            pixelGrid[3][3] = true;
            pixelGrid[4][3] = true;
            pixelGrid[5][1] = pixelGrid[5][3] = pixelGrid[5][5] = true;
            pixelGrid[6][2] = pixelGrid[6][3] = pixelGrid[6][4] = true;
            pixelGrid[7][3] = true;
            return true;
        }

                 // Default for unknown characters - create a filled block for better visibility
        default: {
            for (int y = 1; y < 7; y++) {
                for (int x = 1; x < 6; x++) {
                    pixelGrid[y][x] = true;
                }
            }
            return false;
        }
    }
}




void GeometricTextRenderer::BuildTextMesh(const wchar_t* text, std::vector<TextVertex>& vertices,
    std::vector<uint32_t>& indices, const XMFLOAT4& color) {

    // 优化：缓存相同文本的网格数据
    static std::unordered_map<std::wstring, std::pair<std::vector<TextVertex>, std::vector<uint32_t>>> meshCache;

    std::wstring textString(text);
    std::pair<std::vector<TextVertex>, std::vector<uint32_t>>* cachedMesh = nullptr;

    // 查看缓存中是否已有这个文本的网格数据
    if (meshCache.find(textString) != meshCache.end()) {
        cachedMesh = &meshCache[textString];

        // 复制顶点，但更新颜色
        vertices = cachedMesh->first;
        for (auto& vertex : vertices) {
            vertex.color = color;
        }

        indices = cachedMesh->second;
        return;
    }

    // 如果没有缓存，正常构建网格
    float xOffset = 0.0f;
    uint32_t baseVertex = 0;
    float spacing = 0.1f; // 字符间距

    // 添加一个静态调试标志
    static bool debugOutput = false;  // 设为false关闭调试输出



    while (*text) {
        wchar_t c = *text++;

        // 获取或创建字符几何体
        if (m_characterCache.find(c) == m_characterCache.end()) {
            CharacterGeometry geom;
            CreateCharacterGeometry(c, geom);
            m_characterCache[c] = geom;
        }

        const CharacterGeometry& geom = m_characterCache[c];

        // 处理空格的情况
        if (c == L' ' || geom.vertices.empty()) {
            xOffset += geom.width + spacing;
            continue;
        }
        if (debugOutput) {
            char debugMsg[256];
            sprintf_s(debugMsg, "  处理字符: '%c', 宽度: %.2f, 顶点数: %zu, 索引数: %zu\n",
                (char)c, geom.width, geom.vertices.size(), geom.indices.size());
            OutputDebugStringA(debugMsg);
        }
        // 添加顶点
        for (const auto& vertex : geom.vertices) {
            TextVertex tv;
            tv.position = XMFLOAT3(vertex.x + xOffset, vertex.y, vertex.z);
            tv.normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
            tv.color = color;
            tv.texCoord = XMFLOAT2(0.0f, 0.0f);
            vertices.push_back(tv);
        }

        // 添加索引
        for (uint32_t idx : geom.indices) {
            indices.push_back(baseVertex + idx);
        }

        baseVertex += (uint32_t)geom.vertices.size();
        xOffset += geom.width + spacing; // 添加间距
    }
    // 存入缓存，限制缓存大小以防内存泄漏
    if (meshCache.size() > 1000) {
        // 如果缓存太大，清除一半
        auto it = meshCache.begin();
        for (size_t i = 0; i < 500 && it != meshCache.end(); ++i, ++it) {
            it = meshCache.erase(it);
        }
    }

    meshCache[textString] = std::make_pair(vertices, indices);
}

void GeometricTextRenderer::DrawText3D(const XMFLOAT3& position, const wchar_t* text,
    const XMFLOAT4& color, float scale, bool billboard) {

    if (!m_initialized || !text || *text == L'\0') return;

    // 保存当前状态
    XMMATRIX oldWorld = g_WorldMatrix; // 现在通过外部变量访问
    XMMATRIX oldView = g_ViewMatrix;
    XMMATRIX oldProj = g_ProjectionMatrix;
    bool oldDepth = GetDepthEnable();
    bool oldBlend = GetBlendState();
    D3D11_CULL_MODE oldCull = GetCullingMode();

    // 设置适合文本渲染的状态
    SetDepthEnable(true);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);

    // 构建文本网格
    std::vector<TextVertex> vertices;
    std::vector<uint32_t> indices;
    BuildTextMesh(text, vertices, indices, color);

    if (vertices.empty() || indices.empty()) {
        // 恢复状态
        SetWorldMatrix(oldWorld);
        SetDepthEnable(oldDepth);
        SetBlendState(oldBlend);
        SetCulingMode(oldCull);
        return;
    }

    // 创建顶点和索引缓冲区
    D3D11_BUFFER_DESC vbd = { 0 };
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = (UINT)(sizeof(TextVertex) * vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData = { 0 };
    vInitData.pSysMem = vertices.data();

    ID3D11Buffer* vertexBuffer = nullptr;
    HRESULT hr = GetDevice()->CreateBuffer(&vbd, &vInitData, &vertexBuffer);

    if (FAILED(hr)) {
        OutputDebugStringA("创建文本顶点缓冲区失败\n");
        return;
    }

    D3D11_BUFFER_DESC ibd = { 0 };
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = (UINT)(sizeof(uint32_t) * indices.size());
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData = { 0 };
    iInitData.pSysMem = indices.data();

    ID3D11Buffer* indexBuffer = nullptr;
    hr = GetDevice()->CreateBuffer(&ibd, &iInitData, &indexBuffer);

    if (FAILED(hr)) {
        vertexBuffer->Release();
        OutputDebugStringA("创建文本索引缓冲区失败\n");
        return;
    }

    // 计算世界矩阵
    XMMATRIX worldMatrix;

    if (billboard) {
        // 广告牌效果 - 总是面向摄像机
        XMMATRIX viewMatrix = GetViewMatrix();

        // 移除视图矩阵的平移部分
        viewMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

        // 取反矩阵来创建广告牌
        XMVECTOR determinant;
        worldMatrix = XMMatrixInverse(&determinant, viewMatrix);

        // 应用位置
        worldMatrix.r[3] = XMVectorSet(position.x, position.y, position.z, 1.0f);
    }
    else {
        // 普通3D文本 - 使用指定位置
        worldMatrix = XMMatrixTranslation(position.x, position.y, position.z);
    }

    // 应用缩放
    worldMatrix = XMMatrixScaling(scale, scale, scale) * worldMatrix;

    // 设置世界矩阵
    SetWorldMatrix(worldMatrix);

    // 设置顶点和索引缓冲区
    UINT stride = sizeof(TextVertex);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 设置材质
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = color;
    SetMaterial(material);

    // 绘制文本
    GetDeviceContext()->DrawIndexed((UINT)indices.size(), 0, 0);

    // 释放资源
    vertexBuffer->Release();
    indexBuffer->Release();

    // 恢复状态
    SetWorldMatrix(oldWorld);
    SetViewMatrix(oldView);
    SetProjectionMatrix(oldProj);
    SetDepthEnable(oldDepth);
    SetBlendState(oldBlend);
    SetCulingMode(oldCull);
}

void GeometricTextRenderer::DrawText2D(float x, float y, const wchar_t* text,
    const XMFLOAT4& color, float scale) {

    if (!m_initialized || !text || *text == L'\0') return;

  /*  char debug[256];
    sprintf_s(debug, "绘制2D文本: %S 在位置(%.1f,%.1f) 缩放:%.1f\n",
        text, x, y, scale);
    OutputDebugStringA(debug);*/

    // 保存当前状态
    XMMATRIX oldWorld = g_WorldMatrix;
    XMMATRIX oldView = g_ViewMatrix;
    XMMATRIX oldProj = g_ProjectionMatrix;
    bool oldDepth = GetDepthEnable();
    bool oldBlend = GetBlendState();
    D3D11_CULL_MODE oldCull = GetCullingMode();

    // 设置正交投影
    SetWorldViewProjection2D();

    // 设置适合2D文本的状态
    SetDepthEnable(false);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);

    // 临时将光照禁用，确保文本颜色不受光照影响

   

    // 构建文本网格
    std::vector<TextVertex> vertices;
    std::vector<uint32_t> indices;
    BuildTextMesh(text, vertices, indices, color);

    if (vertices.empty() || indices.empty()) {
        OutputDebugStringA("警告: 生成的2D文本网格为空\n");

        // 恢复状态
        SetWorldMatrix(oldWorld);
        SetViewMatrix(oldView);
        SetProjectionMatrix(oldProj);
        SetDepthEnable(oldDepth);
        SetBlendState(oldBlend);
        SetCulingMode(oldCull);
        return;
    }


    // 创建顶点和索引缓冲区
    D3D11_BUFFER_DESC vbd = { 0 };
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = (UINT)(sizeof(TextVertex) * vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData = { 0 };
    vInitData.pSysMem = vertices.data();

    ID3D11Buffer* vertexBuffer = nullptr;
    HRESULT hr = GetDevice()->CreateBuffer(&vbd, &vInitData, &vertexBuffer);

    if (FAILED(hr)) {
        OutputDebugStringA("创建2D文本顶点缓冲区失败\n");
        return;
    }

    D3D11_BUFFER_DESC ibd = { 0 };
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = (UINT)(sizeof(uint32_t) * indices.size());
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData = { 0 };
    iInitData.pSysMem = indices.data();

    ID3D11Buffer* indexBuffer = nullptr;
    hr = GetDevice()->CreateBuffer(&ibd, &iInitData, &indexBuffer);

    if (FAILED(hr)) {
        vertexBuffer->Release();
        OutputDebugStringA("创建2D文本索引缓冲区失败\n");
        return;
    }

    // 设置世界矩阵 - 位置和缩放
    // 修改: 使用恰当的原点位置，确保文本从左上角开始绘制
    /*XMMATRIX worldMatrix = XMMatrixScaling(scale, scale, 1.0f) *
        XMMatrixTranslation(x, y, 0.0f);
    SetWorldMatrix(worldMatrix);*/

    XMMATRIX worldMatrix = XMMatrixScaling(scale * 2.0f, scale * 2.0f, 1.0f) *
        XMMatrixTranslation(x, y, 0.0f);
    SetWorldMatrix(worldMatrix);

    // 设置顶点和索引缓冲区
    UINT stride = sizeof(TextVertex);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 设置材质 - 使用亮色
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = color;
    material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    material.Emission = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    SetMaterial(material);

    // 强制光照禁用
    LIGHT light;
    ZeroMemory(&light, sizeof(light));
    light.Enable = false;
    SetLight(light);

   
    GetDeviceContext()->DrawIndexed((UINT)indices.size(), 0, 0);


  
    // 释放资源
    vertexBuffer->Release();
    indexBuffer->Release();

    // 恢复状态
    SetWorldMatrix(oldWorld);
    SetViewMatrix(oldView);
    SetProjectionMatrix(oldProj);
    SetDepthEnable(oldDepth);
    SetBlendState(oldBlend);
    SetCulingMode(oldCull);

}

void GeometricTextRenderer::DrawPixelText(float x, float y, const wchar_t* text, const XMFLOAT4& color, float scale)
{
    if (!m_initialized || !text || *text == L'\0') return;

        

    // 保存当前状态
    XMMATRIX oldWorld = g_WorldMatrix;
    XMMATRIX oldView = g_ViewMatrix;
    XMMATRIX oldProj = g_ProjectionMatrix;
    bool oldDepth = GetDepthEnable();
    bool oldBlend = GetBlendState();
    D3D11_CULL_MODE oldCull = GetCullingMode();

    // 设置正交投影
    SetWorldViewProjection2D();

    // 设置适合2D文本的状态
    SetDepthEnable(false);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);

    // 构建文本网格
    std::vector<TextVertex> vertices;
    std::vector<uint32_t> indices;
    BuildTextMesh(text, vertices, indices, color);

    if (vertices.empty() || indices.empty()) {
        OutputDebugStringA("警告: 生成的2D文本网格为空\n");

        // 恢复状态
        SetWorldMatrix(oldWorld);
        SetViewMatrix(oldView);
        SetProjectionMatrix(oldProj);
        SetDepthEnable(oldDepth);
        SetBlendState(oldBlend);
        SetCulingMode(oldCull);
        return;
    }

    // 创建顶点和索引缓冲区
    D3D11_BUFFER_DESC vbd = { 0 };
    vbd.Usage = D3D11_USAGE_DEFAULT;
    vbd.ByteWidth = (UINT)(sizeof(TextVertex) * vertices.size());
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vInitData = { 0 };
    vInitData.pSysMem = vertices.data();

    ID3D11Buffer* vertexBuffer = nullptr;
    HRESULT hr = GetDevice()->CreateBuffer(&vbd, &vInitData, &vertexBuffer);

    if (FAILED(hr)) {
        OutputDebugStringA("创建2D文本顶点缓冲区失败\n");
        return;
    }

    D3D11_BUFFER_DESC ibd = { 0 };
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.ByteWidth = (UINT)(sizeof(uint32_t) * indices.size());
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA iInitData = { 0 };
    iInitData.pSysMem = indices.data();

    ID3D11Buffer* indexBuffer = nullptr;
    hr = GetDevice()->CreateBuffer(&ibd, &iInitData, &indexBuffer);

    if (FAILED(hr)) {
        vertexBuffer->Release();
        OutputDebugStringA("创建2D文本索引缓冲区失败\n");
        return;
    }

    // 修正 - 直接使用屏幕坐标，而不是转换为中心原点
    XMMATRIX worldMatrix = XMMatrixScaling(scale, scale, 1.0f) *
        XMMatrixTranslation(x, y, 0.0f);
    SetWorldMatrix(worldMatrix);

    // 设置顶点和索引缓冲区
    UINT stride = sizeof(TextVertex);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 设置像素风格的材质 - 无光泽的纯色
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));
    material.Diffuse = color;
    material.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    SetMaterial(material);

    // 绘制文本
    GetDeviceContext()->DrawIndexed((UINT)indices.size(), 0, 0);

    // 释放资源
    vertexBuffer->Release();
    indexBuffer->Release();


    // 恢复状态，但保持2D渲染需要的状态
    SetWorldMatrix(oldWorld);
    SetViewMatrix(oldView);
    SetProjectionMatrix(oldProj);
    //// 恢复状态
    //SetWorldMatrix(oldWorld);
    //SetViewMatrix(oldView);
    //SetProjectionMatrix(oldProj);
    //SetDepthEnable(oldDepth);
    //SetBlendState(oldBlend);
    //SetCulingMode(oldCull);
}


void GeometricTextRenderer::DrawEnhancedPixelText(float x, float y, const wchar_t* text, XMFLOAT4 color, float scale)
{
   

    // Adjust position for screen boundaries if needed
    float screenY = y;
    if (y > SCREEN_HEIGHT * 0.7f) {
        screenY = y - SCREEN_HEIGHT * 0.1f;
    }

    // Apply text color variation based on content for visual interest
    // For numeric text, add slight blue tint
    if (iswdigit(text[0])) {
        color.z = std::min(1.0f, color.z + 0.2f); // Increase blue component
    }
    // For capital letters, add slight green tint
    else if (iswupper(text[0])) {
        color.y = std::min(1.0f, color.y + 0.2f); // Increase green component
    }

    // Add pixelation effect by nudging scale to integer values
    // This ensures the pixels align properly on the screen
    float pixelPerfectScale = floor(scale * 4.0f) / 4.0f;

    // Use enhanced pixel text renderer
    GeometricTextRenderer::DrawPixelText(x, screenY, text, color, pixelPerfectScale);

   
}

void GeometricTextRenderer::DrawTitlePixelText(float x, float y, const wchar_t* text, XMFLOAT4 color, float scale)
{
    // Draw shadow/border effect
    XMFLOAT4 shadowColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.8f);

    // Draw text shadow in multiple directions for border effect
    GeometricTextRenderer::DrawPixelText(x + 2, y + 2, text, shadowColor, scale);
    GeometricTextRenderer::DrawPixelText(x + 2, y, text, shadowColor, scale);
    GeometricTextRenderer::DrawPixelText(x, y + 2, text, shadowColor, scale);
    GeometricTextRenderer::DrawPixelText(x - 2, y, text, shadowColor, scale);
    GeometricTextRenderer::DrawPixelText(x, y - 2, text, shadowColor, scale);

    // Draw main text on top
    GeometricTextRenderer::DrawPixelText(x, y, text, color, scale);

}

void GeometricTextRenderer::DrawFlickeringPixelText(float x, float y, const wchar_t* text, XMFLOAT4 baseColor, float scale)
{
    // Use the system timer to create a flicker effect
    static float flickerTimer = 0.0f;
    flickerTimer += 0.016f; // Assuming ~60 FPS

    // Create flickering effect with sin wave
    float flicker = (sin(flickerTimer * 10.0f) + 1.0f) * 0.3f;

    // Apply flicker to color
    XMFLOAT4 flickerColor = XMFLOAT4(
        std::min(1.0f, baseColor.x + flicker),
        std::min(1.0f, baseColor.y + flicker),
        std::min(1.0f, baseColor.z + flicker),
        baseColor.w
    );

    // Draw with flickering color
    GeometricTextRenderer::DrawPixelText(x, y, text, flickerColor, scale);

}

void GeometricTextRenderer::DrawRainbowPixelText(float x, float y, const wchar_t* text, float scale)
{
    // Use timer for rainbow cycle
    static float rainbowTimer = 0.0f;
    rainbowTimer += 0.005f;

    // Calculate base color based on time
    float r = sin(rainbowTimer) * 0.5f + 0.5f;
    float g = sin(rainbowTimer + 2.0f) * 0.5f + 0.5f;
    float b = sin(rainbowTimer + 4.0f) * 0.5f + 0.5f;
    XMFLOAT4 baseColor = XMFLOAT4(r, g, b, 1.0f);

    // Draw with rainbow color
    GeometricTextRenderer::DrawPixelText(x, y, text, baseColor, scale);

}

void GeometricTextRenderer::DrawMinecraftText(float x, float y, const wchar_t* text,
    const XMFLOAT4& color, float scale, bool centeredText) {

    if (!m_initialized || !text || *text == L'\0') return;

    // 将浮点坐标取整以避免每帧的像素抖动
    x = floorf(x);
    y = floorf(y);

    // 缓存文本渲染 - 每次渲染相同文本时避免重建
    static std::unordered_map<std::wstring, std::pair<std::vector<TextVertex>, std::vector<uint32_t>>> textCache;
    static std::unordered_map<std::wstring, float> textWidthCache;

    std::wstring textString(text);
    float textWidth = 0.0f;

    // 计算/获取文本宽度
    if (centeredText) {
        // 检查缓存中是否有宽度
        if (textWidthCache.find(textString) != textWidthCache.end()) {
            textWidth = textWidthCache[textString] * scale;
        }
        else {
            // 计算宽度并缓存
            const wchar_t* p = text;
            while (*p) {
                wchar_t c = *p++;
                if (m_characterCache.find(c) == m_characterCache.end()) {
                    CharacterGeometry geom;
                    CreateCharacterGeometry(c, geom);
                    m_characterCache[c] = geom;
                }
                textWidth += m_characterCache[c].width + 0.1f;
            }
            textWidthCache[textString] = textWidth;
            textWidth *= scale;
        }
        x -= textWidth / 2.0f;
    }

    // 简化边框渲染 - 减少重复绘制次数
   //  只绘制4个角落和4条边，而不是遍历所有像素位置
    XMFLOAT4 shadowColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.85f);

    // 背景 - 使用单一绘制调用
    DrawPixelText(x , y , text, shadowColor, scale * 1.2f);

    // 前景文本
    XMFLOAT4 brightColor = color;
    brightColor.x = std::min(1.0f, color.x * 1.3f);
    brightColor.y = std::min(1.0f, color.y * 1.3f);
    brightColor.z = std::min(1.0f, color.z * 1.3f);

  //  DrawPixelText(x, y, text, brightColor, scale);
}


XMFLOAT2 GeometricTextRenderer::MeasureText(const wchar_t* text, float scale)
{
    XMFLOAT2 size = XMFLOAT2(0.0f, 0.0f);
    float maxHeight = 0.0f;
    float width = 0.0f;
    float spacing = 0.1f; // 字符间距

    while (*text) {
        wchar_t c = *text++;

        // 检查字符是否在缓存中
        if (m_characterCache.find(c) == m_characterCache.end()) {
            CharacterGeometry geom;
            CreateCharacterGeometry(c, geom);
            m_characterCache[c] = geom;
        }

        const CharacterGeometry& geom = m_characterCache[c];
        width += geom.width * scale + spacing * scale;

        // 找出最大高度
        maxHeight = std::max(maxHeight, 9.0f * 0.12f * scale * 4.0f); // 根据CreateCharacterGeometry中的参数调整
    }

    size.x = width;
    size.y = maxHeight;
    return size;
}




