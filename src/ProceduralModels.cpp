// ProceduralModels.cpp - Improved version
#include "ProceduralModels.h"

// 全局模型对象
ProceduralModel g_CoinModel;
ProceduralModel g_BombModel;
ProceduralModel g_FuseModel; 


// 动画参数
static float g_CoinRotation = 0.0f;
static float g_BombRotation = 0.0f;
static float g_FuseGlowIntensity = 0.0f;
static bool g_FuseGlowIncreasing = true;

// 初始化程序化模型
void InitProceduralModels()
{
    // 初始化结构体
    ZeroMemory(&g_CoinModel, sizeof(ProceduralModel));
    ZeroMemory(&g_BombModel, sizeof(ProceduralModel));
    ZeroMemory(&g_FuseModel, sizeof(ProceduralModel));


    // 创建金币模型 - 改进版
    CreateProceduralCoin(&g_CoinModel);

    // 创建炸弹模型 - 改进版
    CreateProceduralBomb(&g_BombModel);

    // 创建引信模型
    CreateProceduralFuse(&g_FuseModel);

    OutputDebugStringA("程序化模型初始化成功\n");
}

// 更新模型动画 - 增强版
void UpdateProceduralModels(float deltaTime)
{
    // 更新金币旋转角度 - 光滑的旋转
    static float coinSpeed = 1.5f; // 稍微快一点
    g_CoinRotation += coinSpeed * deltaTime;
    if (g_CoinRotation >= XM_2PI)
        g_CoinRotation -= XM_2PI;

    // 更新炸弹旋转角度 - 更缓慢的旋转
    g_BombRotation += 0.3f * deltaTime;
    if (g_BombRotation >= XM_2PI)
        g_BombRotation -= XM_2PI;

    // 更新引信发光效果
    static float glowDirection = 1.0f;

    g_FuseGlowIntensity += glowDirection * 1.5f * deltaTime;

    if (g_FuseGlowIntensity >= 1.0f) {
        g_FuseGlowIntensity = 1.0f;
        glowDirection = -1.0f;
    }
    else if (g_FuseGlowIntensity <= 0.5f) {
        g_FuseGlowIntensity = 0.5f;
        glowDirection = 1.0f;
    }
}
// 获取当前旋转角度和动画参数
float GetCoinRotation() { return g_CoinRotation; }
float GetBombRotation() { return g_BombRotation; }
float GetFuseGlowIntensity() { return g_FuseGlowIntensity; }

// 清理程序化模型
void UninitProceduralModels()
{
    // 释放金币模型资源
    if (g_CoinModel.VertexBuffer) {
        g_CoinModel.VertexBuffer->Release();
        g_CoinModel.VertexBuffer = NULL;
    }
    if (g_CoinModel.IndexBuffer) {
        g_CoinModel.IndexBuffer->Release();
        g_CoinModel.IndexBuffer = NULL;
    }

    // 释放炸弹模型资源
    if (g_BombModel.VertexBuffer) {
        g_BombModel.VertexBuffer->Release();
        g_BombModel.VertexBuffer = NULL;
    }
    if (g_BombModel.IndexBuffer) {
        g_BombModel.IndexBuffer->Release();
        g_BombModel.IndexBuffer = NULL;
    }

    // 释放引信模型资源
    if (g_FuseModel.VertexBuffer) {
        g_FuseModel.VertexBuffer->Release();
        g_FuseModel.VertexBuffer = NULL;
    }
    if (g_FuseModel.IndexBuffer) {
        g_FuseModel.IndexBuffer->Release();
        g_FuseModel.IndexBuffer = NULL;
    }

   
    OutputDebugStringA("程序化模型清理成功\n");
}



void CreateProceduralCoin(ProceduralModel* model)
{
    // 定义金币的参数
    const int sides = 32;          // 硬币周边的分段数
    const float radius = 0.4f;     // 硬币半径
    const float thickness = 0.08f; // 硬币厚度 - 非常薄

    // 设置金色
    model->Color = XMFLOAT4(1.0f, 0.84f, 0.0f, 1.0f); // 金色

    // 计算顶点和索引数量
    // 上下表面 + 侧面
    model->VertexCount = 2 * (sides + 1) + sides * 4;
    model->IndexCount = sides * 3 * 2 + sides * 6;

    // 创建顶点数据
    VERTEX_3D* vertices = new VERTEX_3D[model->VertexCount];

    // 1. 顶面（正面）
    // 顶面中心点
    vertices[0].Position = XMFLOAT3(0.0f, thickness / 2, 0.0f);
    vertices[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
    vertices[0].Diffuse = model->Color;
    vertices[0].TexCoord = XMFLOAT2(0.5f, 0.5f);

    // 顶面边缘点
    for (int i = 0; i < sides; i++) {
        float angle = XM_2PI * i / sides;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);

        vertices[i + 1].Position = XMFLOAT3(x, thickness / 2, z);
        vertices[i + 1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

        // 浮雕图案效果 - 微调顶点法线和颜色
        float embossAngle = angle * 8.0f; // 8个浮雕图案
        float embossFactor = 0.2f * sinf(embossAngle); // 浮雕强度

        // 这里不改变Position，只调整法线和颜色
        vertices[i + 1].Normal.x = embossFactor * 0.3f * cosf(angle);
        vertices[i + 1].Normal.z = embossFactor * 0.3f * sinf(angle);
        // 确保法线归一化
        float normLength = sqrtf(
            vertices[i + 1].Normal.x * vertices[i + 1].Normal.x +
            vertices[i + 1].Normal.y * vertices[i + 1].Normal.y +
            vertices[i + 1].Normal.z * vertices[i + 1].Normal.z
        );
        vertices[i + 1].Normal.x /= normLength;
        vertices[i + 1].Normal.y /= normLength;
        vertices[i + 1].Normal.z /= normLength;

        // 浮雕图案的颜色变化
        vertices[i + 1].Diffuse = XMFLOAT4(
            model->Color.x * (1.0f - embossFactor * 0.15f),
            model->Color.y * (1.0f - embossFactor * 0.1f),
            model->Color.z * (1.0f - embossFactor * 0.2f),
            1.0f
        );

        vertices[i + 1].TexCoord = XMFLOAT2(0.5f + 0.5f * cosf(angle), 0.5f + 0.5f * sinf(angle));
    }

    // 2. 底面（背面）
    // 底面中心点
    int baseIndex = sides + 1;
    vertices[baseIndex].Position = XMFLOAT3(0.0f, -thickness / 2, 0.0f);
    vertices[baseIndex].Normal = XMFLOAT3(0.0f, -1.0f, 0.0f);
    vertices[baseIndex].Diffuse = XMFLOAT4(
        model->Color.x * 0.9f,
        model->Color.y * 0.9f,
        model->Color.z * 0.9f,
        1.0f
    ); // 背面稍暗
    vertices[baseIndex].TexCoord = XMFLOAT2(0.5f, 0.5f);

    // 底面边缘点
    for (int i = 0; i < sides; i++) {
        float angle = XM_2PI * i / sides;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);

        vertices[baseIndex + i + 1].Position = XMFLOAT3(x, -thickness / 2, z);
        vertices[baseIndex + i + 1].Normal = XMFLOAT3(0.0f, -1.0f, 0.0f);

        // 不同的浮雕花纹
        float embossAngle = angle * 12.0f; // 12个浮雕图案，与正面不同
        float embossFactor = 0.2f * cosf(embossAngle);

        vertices[baseIndex + i + 1].Normal.x = embossFactor * 0.3f * cosf(angle);
        vertices[baseIndex + i + 1].Normal.z = embossFactor * 0.3f * sinf(angle);
        // 确保法线归一化
        float normLength = sqrtf(
            vertices[baseIndex + i + 1].Normal.x * vertices[baseIndex + i + 1].Normal.x +
            vertices[baseIndex + i + 1].Normal.y * vertices[baseIndex + i + 1].Normal.y +
            vertices[baseIndex + i + 1].Normal.z * vertices[baseIndex + i + 1].Normal.z
        );
        vertices[baseIndex + i + 1].Normal.x /= normLength;
        vertices[baseIndex + i + 1].Normal.y /= normLength;
        vertices[baseIndex + i + 1].Normal.z /= normLength;

        // 背面比正面稍暗
        vertices[baseIndex + i + 1].Diffuse = XMFLOAT4(
            model->Color.x * (0.85f - embossFactor * 0.15f),
            model->Color.y * (0.85f - embossFactor * 0.1f),
            model->Color.z * (0.85f - embossFactor * 0.2f),
            1.0f
        );

        vertices[baseIndex + i + 1].TexCoord = XMFLOAT2(0.5f + 0.5f * cosf(angle), 0.5f + 0.5f * sinf(angle));
    }

    // 3. 侧面 - 直接连接上下表面的边缘
    baseIndex = 2 * (sides + 1);
    for (int i = 0; i < sides; i++) {
        float angle = XM_2PI * i / sides;
        float nextAngle = XM_2PI * ((i + 1) % sides) / sides;

        float x1 = radius * cosf(angle);
        float z1 = radius * sinf(angle);
        float x2 = radius * cosf(nextAngle);
        float z2 = radius * sinf(nextAngle);

        // 每个侧面是一个四边形，由两个三角形组成
        // 法线朝外
        float nx = cosf((angle + nextAngle) / 2);
        float nz = sinf((angle + nextAngle) / 2);

        // 第一个顶点（顶面边缘）
        vertices[baseIndex + i * 4].Position = XMFLOAT3(x1, thickness / 2, z1);
        vertices[baseIndex + i * 4].Normal = XMFLOAT3(nx, 0.0f, nz);
        vertices[baseIndex + i * 4].Diffuse = XMFLOAT4(
            model->Color.x * 0.8f,
            model->Color.y * 0.8f,
            model->Color.z * 0.8f,
            1.0f
        ); // 侧面稍暗
        vertices[baseIndex + i * 4].TexCoord = XMFLOAT2((float)i / sides, 0.0f);

        // 第二个顶点（下一个顶面边缘）
        vertices[baseIndex + i * 4 + 1].Position = XMFLOAT3(x2, thickness / 2, z2);
        vertices[baseIndex + i * 4 + 1].Normal = XMFLOAT3(nx, 0.0f, nz);
        vertices[baseIndex + i * 4 + 1].Diffuse = XMFLOAT4(
            model->Color.x * 0.8f,
            model->Color.y * 0.8f,
            model->Color.z * 0.8f,
            1.0f
        );
        vertices[baseIndex + i * 4 + 1].TexCoord = XMFLOAT2((float)(i + 1) / sides, 0.0f);

        // 第三个顶点（底面边缘）
        vertices[baseIndex + i * 4 + 2].Position = XMFLOAT3(x1, -thickness / 2, z1);
        vertices[baseIndex + i * 4 + 2].Normal = XMFLOAT3(nx, 0.0f, nz);
        vertices[baseIndex + i * 4 + 2].Diffuse = XMFLOAT4(
            model->Color.x * 0.75f,
            model->Color.y * 0.75f,
            model->Color.z * 0.75f,
            1.0f
        ); // 底部比顶部稍暗
        vertices[baseIndex + i * 4 + 2].TexCoord = XMFLOAT2((float)i / sides, 1.0f);

        // 第四个顶点（下一个底面边缘）
        vertices[baseIndex + i * 4 + 3].Position = XMFLOAT3(x2, -thickness / 2, z2);
        vertices[baseIndex + i * 4 + 3].Normal = XMFLOAT3(nx, 0.0f, nz);
        vertices[baseIndex + i * 4 + 3].Diffuse = XMFLOAT4(
            model->Color.x * 0.75f,
            model->Color.y * 0.75f,
            model->Color.z * 0.75f,
            1.0f
        );
        vertices[baseIndex + i * 4 + 3].TexCoord = XMFLOAT2((float)(i + 1) / sides, 1.0f);
    }

    // 创建索引数据
    unsigned int* indices = new unsigned int[model->IndexCount];
    int indexCount = 0;

    // 顶面索引
    for (int i = 0; i < sides; i++) {
        indices[indexCount++] = 0; // 中心点
        indices[indexCount++] = i + 1;
        indices[indexCount++] = ((i + 1) % sides) + 1;
    }

    // 底面索引
    int bottomStart = sides + 1;
    for (int i = 0; i < sides; i++) {
        indices[indexCount++] = bottomStart; // 中心点
        indices[indexCount++] = bottomStart + ((i + 1) % sides) + 1;
        indices[indexCount++] = bottomStart + i + 1;
    }

    // 侧面索引
    baseIndex = 2 * (sides + 1);
    for (int i = 0; i < sides; i++) {
        // 每个侧面是四边形，由两个三角形组成
        // 第一个三角形
        indices[indexCount++] = baseIndex + i * 4;
        indices[indexCount++] = baseIndex + i * 4 + 1;
        indices[indexCount++] = baseIndex + i * 4 + 2;

        // 第二个三角形
        indices[indexCount++] = baseIndex + i * 4 + 1;
        indices[indexCount++] = baseIndex + i * 4 + 3;
        indices[indexCount++] = baseIndex + i * 4 + 2;
    }

    // 创建顶点缓冲区
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(VERTEX_3D) * model->VertexCount;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = vertices;

    GetDevice()->CreateBuffer(&bd, &sd, &model->VertexBuffer);

    // 创建索引缓冲区
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(unsigned int) * model->IndexCount;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    sd.pSysMem = indices;

    GetDevice()->CreateBuffer(&bd, &sd, &model->IndexBuffer);

    // 释放临时数据
    delete[] vertices;
    delete[] indices;
}
// 创建炸弹模型 - 改进版
// 创建炸弹模型 - 调整为中心白色向外渐变为黑色的圆形
void CreateProceduralBomb(ProceduralModel* model)
{
    // 定义球体参数
    const int slices = 32;  // 增加切片以提高质量
    const int stacks = 32;  // 增加层数以提高质量
    const float radius = 0.4f;

    // 设置基础颜色
    model->Color = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f); // 黑色基础

    // 计算顶点和索引数量
    model->VertexCount = (slices + 1) * (stacks + 1);
    model->IndexCount = slices * stacks * 6;

    // 创建顶点数据
    VERTEX_3D* vertices = new VERTEX_3D[model->VertexCount];

    // 顶部留一个小孔用于放置引信
    const float topHoleRadius = 0.05f;
    const float topHoleAngle = asinf(topHoleRadius / radius);

    // 生成球体顶点
    int index = 0;
    for (int stack = 0; stack <= stacks; stack++) {
        float phi = XM_PI * stack / stacks;

        // 为顶部引信孔调整角度
        if (phi < topHoleAngle) {
            phi = topHoleAngle;
        }

        for (int slice = 0; slice <= slices; slice++) {
            float theta = XM_2PI * slice / slices;

            // 球体参数方程
            float x = radius * sinf(phi) * cosf(theta);
            float y = radius * cosf(phi);
            float z = radius * sinf(phi) * sinf(theta);

            // 设置顶点属性
            vertices[index].Position = XMFLOAT3(x, y, z);

            // 法线方向 - 从球心向外
            float length = sqrtf(x * x + y * y + z * z);
            if (length > 0.0001f) {
                vertices[index].Normal = XMFLOAT3(x / length, y / length, z / length);
            }
            else {
                vertices[index].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
            }

            // 颜色渐变 - 从中心白色到外边缘黑色
            // 使用phi角度（从顶部到底部）作为渐变因子
            float gradientFactor = sinf(phi); // 值在0到1之间变化

            // 颜色从白色渐变到黑色
            float r = 1.0f - gradientFactor * 0.9f; // 从1.0变化到0.1
            float g = 1.0f - gradientFactor * 0.9f;
            float b = 1.0f - gradientFactor * 0.9f;

            // 顶部留一圈黑色，用于引信连接处
            if (phi <= topHoleAngle * 2.0f) {
                r = g = b = 0.1f;
            }

            vertices[index].Diffuse = XMFLOAT4(r, g, b, 1.0f);

            // 纹理坐标
            vertices[index].TexCoord = XMFLOAT2((float)slice / slices, (float)stack / stacks);

            index++;
        }
    }

    // 创建索引数据
    unsigned int* indices = new unsigned int[model->IndexCount];
    index = 0;
    for (int stack = 0; stack < stacks; stack++) {
        for (int slice = 0; slice < slices; slice++) {
            // 计算顶点索引
            int v1 = stack * (slices + 1) + slice;
            int v2 = stack * (slices + 1) + (slice + 1);
            int v3 = (stack + 1) * (slices + 1) + slice;
            int v4 = (stack + 1) * (slices + 1) + (slice + 1);

            // 跳过顶部洞的三角形
            if (stack == 0 && asinf(topHoleRadius / radius) > 0) {
                continue;
            }

            // 第一个三角形
            indices[index++] = v1;
            indices[index++] = v2;
            indices[index++] = v3;

            // 第二个三角形
            indices[index++] = v3;
            indices[index++] = v2;
            indices[index++] = v4;
        }
    }

    // 调整索引计数以匹配实际使用的索引数量
    model->IndexCount = index;

    // 创建顶点缓冲区
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(VERTEX_3D) * model->VertexCount;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = vertices;

    GetDevice()->CreateBuffer(&bd, &sd, &model->VertexBuffer);

    // 创建索引缓冲区
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(unsigned int) * model->IndexCount;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    sd.pSysMem = indices;

    GetDevice()->CreateBuffer(&bd, &sd, &model->IndexBuffer);

    // 释放临时数据
    delete[] vertices;
    delete[] indices;
}
// 新函数: 创建炸弹引信
// // 创建炸弹引信 - 修复版
// 创建炸弹引信 - 加大尺寸
// 创建炸弹引信 - 加长并改为白色
void CreateProceduralFuse(ProceduralModel* model)
{
    // 调整引信参数 - 增加长度和粗细
    const int segments = 16;       // 周围的分段数
    const int fuseSegments = 12;   // 引信的分段数
    const float radius = 0.06f;    // 引信半径 - 加粗
    const float length = 0.6f;     // 引信长度 - 加长为原来两倍
    const float bend = 0.15f;      // 引信弯曲量 - 增加弯曲度

    // 设置颜色 - 白色引信
    model->Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // 白色

    // 计算顶点和索引数量
    model->VertexCount = (segments + 1) * (fuseSegments + 1);
    model->IndexCount = segments * fuseSegments * 6;

    // 创建顶点数据
    VERTEX_3D* vertices = new VERTEX_3D[model->VertexCount];

    // 生成引信顶点
    int index = 0;
    for (int i = 0; i <= fuseSegments; i++) {
        float t = (float)i / fuseSegments;

        // 引信沿曲线路径
        float fuseX = bend * sinf(t * XM_PI);
        float fuseY = length * t;
        float fuseZ = bend * 0.5f * sinf(t * XM_PI * 2.0f); // 添加Z方向的弯曲

        // 沿路径的切向量（用于构建引信横截面）
        float tangentX = bend * XM_PI * cosf(t * XM_PI);
        float tangentY = length;
        float tangentZ = bend * XM_PI * cosf(t * XM_PI * 2.0f);

        // 规范化切向量
        float tangentLength = sqrtf(tangentX * tangentX + tangentY * tangentY + tangentZ * tangentZ);
        if (tangentLength > 0.0001f) {
            tangentX /= tangentLength;
            tangentY /= tangentLength;
            tangentZ /= tangentLength;
        }
        else {
            tangentX = 0.0f;
            tangentY = 1.0f;
            tangentZ = 0.0f;
        }

        // 构建横截面坐标系
        float nx, ny, nz;
        if (fabsf(tangentY) > 0.99f) {
            // 特殊情况：切线几乎垂直，选择X轴作为参考
            nx = 1.0f;
            ny = 0.0f;
            nz = 0.0f;
        }
        else {
            // 一般情况：使用全局上方向叉乘切线
            nx = -tangentZ;
            ny = 0.0f;
            nz = tangentX;
            float nLength = sqrtf(nx * nx + ny * ny + nz * nz);
            if (nLength > 0.0001f) {
                nx /= nLength;
                ny /= nLength;
                nz /= nLength;
            }
            else {
                nx = 1.0f;
                ny = 0.0f;
                nz = 0.0f;
            }
        }

        // 第二个垂直向量（叉乘得到）
        float bx = tangentY * nz - tangentZ * ny;
        float by = tangentZ * nx - tangentX * nz;
        float bz = tangentX * ny - tangentY * nx;
        float bLength = sqrtf(bx * bx + by * by + bz * bz);
        if (bLength > 0.0001f) {
            bx /= bLength;
            by /= bLength;
            bz /= bLength;
        }

        // 引信半径变化 - 底部略粗，顶部略细
        float radiusVar = radius * (1.0f - t * 0.3f);

        for (int j = 0; j <= segments; j++) {
            float angle = XM_2PI * j / segments;
            float cosA = cosf(angle);
            float sinA = sinf(angle);

            // 计算横截面上的点
            float dx = radiusVar * (nx * cosA + bx * sinA);
            float dy = radiusVar * (ny * cosA + by * sinA);
            float dz = radiusVar * (nz * cosA + bz * sinA);

            vertices[index].Position = XMFLOAT3(fuseX + dx, fuseY + dy, fuseZ + dz);

            // 顶点法线指向外侧
            vertices[index].Normal = XMFLOAT3(dx / radiusVar, dy / radiusVar, dz / radiusVar);

            // 引信颜色 - 纯白色，顶端稍带红色
            float r = 1.0f;
            float g = 1.0f;
            float b = 1.0f;

            // 顶端稍微发红
            if (i <= 1) {
                r = 1.0f;
                g = 0.6f;
                b = 0.4f;
            }
            else if (i <= 2) {
                // 平滑过渡
                float blend = (2.0f - (float)i);
                g = 0.6f + (1.0f - 0.6f) * (1.0f - blend);
                b = 0.4f + (1.0f - 0.4f) * (1.0f - blend);
            }

            // 确保所有顶点完全不透明
            vertices[index].Diffuse = XMFLOAT4(r, g, b, 1.0f);

            // 纹理坐标
            vertices[index].TexCoord = XMFLOAT2((float)j / segments, t);

            index++;
        }
    }

    // 创建索引数据
    unsigned int* indices = new unsigned int[model->IndexCount];
    index = 0;
    for (int i = 0; i < fuseSegments; i++) {
        for (int j = 0; j < segments; j++) {
            // 计算顶点索引
            int v1 = i * (segments + 1) + j;
            int v2 = i * (segments + 1) + (j + 1) % (segments + 1);
            int v3 = (i + 1) * (segments + 1) + j;
            int v4 = (i + 1) * (segments + 1) + (j + 1) % (segments + 1);

            // 第一个三角形
            indices[index++] = v1;
            indices[index++] = v2;
            indices[index++] = v3;

            // 第二个三角形
            indices[index++] = v3;
            indices[index++] = v2;
            indices[index++] = v4;
        }
    }

    // 创建顶点缓冲区和索引缓冲区
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(VERTEX_3D) * model->VertexCount;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.pSysMem = vertices;

    GetDevice()->CreateBuffer(&bd, &sd, &model->VertexBuffer);

    // 创建索引缓冲区
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(unsigned int) * model->IndexCount;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    sd.pSysMem = indices;

    GetDevice()->CreateBuffer(&bd, &sd, &model->IndexBuffer);

    // 释放临时数据
    delete[] vertices;
    delete[] indices;
}
// 绘制完整的炸弹（主体 + 引信）- 调整引信位置
void DrawCompleteBomb(XMMATRIX worldMatrix)
{
    // 首先绘制炸弹主体
    DrawProceduralModel(&g_BombModel, worldMatrix);

    // 调整引信的位置 - 更高的位置以适应加长的引信
    XMMATRIX fuseMatrix = XMMatrixTranslation(0.0f, 0.38f, 0.0f);

    // 添加一些摆动效果
    static float fuseWobble = 0.0f;
    fuseWobble += 0.01f;
    XMMATRIX fuseRotation = XMMatrixRotationX(sinf(fuseWobble) * 0.02f) *
        XMMatrixRotationZ(cosf(fuseWobble * 0.7f) * 0.03f);

    // 合并变换
    fuseMatrix = fuseRotation * fuseMatrix * worldMatrix;

    // 绘制引信
    DrawProceduralModel(&g_FuseModel, fuseMatrix);
}

// 修改DrawProceduralModel函数，优化材质设置
void DrawProceduralModel(ProceduralModel* model, XMMATRIX worldMatrix)
{
    // 保存当前状态
    MATERIAL oldMaterial;
    GetMaterial(&oldMaterial);
    XMMATRIX oldWorldMatrix = GetWorldMatrix();
    bool oldBlendState = GetBlendState();
    D3D11_CULL_MODE oldCullMode = GetCullingMode();

    // 保存当前纹理
    ID3D11ShaderResourceView* oldSRV = NULL;
    GetDeviceContext()->PSSetShaderResources(0, 1, &oldSRV);

    // 设置世界矩阵
    SetWorldMatrix(worldMatrix);

    // 设置渲染状态
    SetBlendState(false); // 禁用混合以确保不透明
    SetCulingMode(D3D11_CULL_BACK);
    SetDepthEnable(true);
    SetDepthWriteEnable(true);

    // 为特定模型设置材质
    MATERIAL material;
    ZeroMemory(&material, sizeof(material));

    if (model == &g_CoinModel) {
        // 金币 - 高品质金属材质，更加真实
        material.Diffuse = XMFLOAT4(1.0f, 0.84f, 0.0f, 1.0f);    // 金色
        material.Ambient = XMFLOAT4(0.6f, 0.5f, 0.0f, 1.0f);     // 金色环境光
        material.Specular = XMFLOAT4(1.0f, 0.95f, 0.7f, 90.0f);  // 锐利的金属高光
        material.Emission = XMFLOAT4(0.15f, 0.12f, 0.0f, 1.0f);  // 轻微发光
    }
    else if (model == &g_BombModel) {
        // 炸弹 - 注意：顶点颜色已经包含了从白到黑的渐变
        material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);    // 白色基础色，让顶点颜色显示
        material.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);    // 较暗的环境光
        material.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 20.0f);  // 适中的高光
        material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);   // 无发光
    }
    else if (model == &g_FuseModel) {
        // 引信 - 白色基调，顶端发光
        float glowIntensity = GetFuseGlowIntensity();

        material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);        // 白色
        material.Ambient = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);        // 适中的环境光
        material.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 10.0f);      // 柔和的高光

        // 引信顶部发光效果
        material.Emission = XMFLOAT4(
            1.0f * glowIntensity,                 // 红色
            0.6f * glowIntensity,                 // 橙色
            0.3f * glowIntensity,                 // 淡黄色
            1.0f                                  // 不透明
        );
    }

    // 设置材质
    SetMaterial(material);

    // 使用白色纹理，避免纹理干扰
    GetDeviceContext()->PSSetShaderResources(0, 1, &g_TextureWhite);

    // 绘制模型
    UINT stride = sizeof(VERTEX_3D);
    UINT offset = 0;
    GetDeviceContext()->IASetVertexBuffers(0, 1, &model->VertexBuffer, &stride, &offset);
    GetDeviceContext()->IASetIndexBuffer(model->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    GetDeviceContext()->DrawIndexed(model->IndexCount, 0, 0);

    // 恢复所有状态
    SetWorldMatrix(oldWorldMatrix);
    SetMaterial(oldMaterial);
    SetBlendState(oldBlendState);
    SetCulingMode(oldCullMode);

    // 恢复原始纹理
    if (oldSRV) {
        GetDeviceContext()->PSSetShaderResources(0, 1, &oldSRV);
        oldSRV->Release(); // 释放临时引用
    }
    else {
        // 如果之前没有纹理，解绑纹理
        ID3D11ShaderResourceView* nullSRV = NULL;
        GetDeviceContext()->PSSetShaderResources(0, 1, &nullSRV);
    }
}