#pragma once
#pragma once

#include "main.h"
#include "renderer.h"
#include  <iostream>
// 粒子效果结构体
struct ParticleEffect {
    XMFLOAT3 position;
    XMFLOAT3 velocity;
    XMFLOAT4 color;
    float size;
    float life;
    float maxLife;
    bool active;
};

// 粒子系统类
class ParticleSystem {
public:
    // 初始化和清理
    static void Init();
    static void Uninit();

    // 创建各种类型的粒子效果
    static void CreateEffect(XMFLOAT3 position, int particleCount, XMFLOAT4 color, float speed, float size, float lifetime);

    // 特定效果类型的便捷函数
    static void CreateHealEffect(XMFLOAT3 position, int count);
    static void CreateDamageEffect(XMFLOAT3 position, int count);
    static void CreateSlowEffect(XMFLOAT3 position, int count);
    static void CreateSlipperyEffect(XMFLOAT3 position, int count);

    // 更新和绘制粒子
    static void Update(float deltaTime);
    static void Draw();

    // 获取纹理资源
    static ID3D11ShaderResourceView* GetParticleTexture();
    static void SetParticleTexture(ID3D11ShaderResourceView* texture);

private:
    static ID3D11ShaderResourceView* m_particleTexture;
};

// 浮动文本相关
struct FloatingText {
    std::string text;
    XMFLOAT3 position;
    XMFLOAT4 color;
    float timer;
    float alpha;
    float size;
};

class FloatingTextSystem {
public:
    static void Init();
    static void Uninit();

    static void ShowText(XMFLOAT3 position, const char* text, XMFLOAT4 color);
    static void Update(float deltaTime);
    static void Draw();

private:
    static std::vector<FloatingText> m_floatingTexts;
};