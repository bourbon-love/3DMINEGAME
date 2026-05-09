
//ParticleEffect.cpp

#include "ParticlaEffect.h"
#include "sprite.h"
#include "Camera.h"
#include "renderer.h"
#include <algorithm>
#include  "Ball.h"
#include  "renderer.h"
#include  "GeometricTextRenderer.h"
// 静态成员初始化
ID3D11ShaderResourceView* ParticleSystem::m_particleTexture = nullptr;
std::vector<FloatingText> FloatingTextSystem::m_floatingTexts;
extern ID3D11ShaderResourceView* g_FontTexture;  // 字体纹理

// ===== 粒子系统实现 =====

void ParticleSystem::Init() {
    // 加载粒子纹理（如果需要在这里加载）
    // 注意：通常纹理已经在其他地方加载，所以这里可能只是初始化一些变量
}

void ParticleSystem::Uninit() {
    // 释放资源
    // 如果纹理是在这里创建的，需要在这里释放
    // 如果是在其他地方创建并只是引用，则不需要释放
}

// 深灰色沙地粒子效果


// 通用粒子效果创建函数
void ParticleSystem::Update(float deltaTime) {
    BallObject* ball = GetBall();

    // 限制deltaTime，防止时间步长过大
    deltaTime = std::min(deltaTime, 0.05f);

    for (auto& particle : ball->activeEffects) {
        if (!particle.active) continue;

        // 保存旧位置
        XMFLOAT3 oldPos = particle.position;

        // 更新位置
        particle.position.x += particle.velocity.x * deltaTime;
        particle.position.y += particle.velocity.y * deltaTime;
        particle.position.z += particle.velocity.z * deltaTime;

        // 根据颜色调整重力和行为
        bool isHealEffect = (particle.color.y > 0.8f && particle.color.x < 0.3f);
        bool isSlipperyEffect = (particle.color.z > 0.8f && particle.color.x < 0.6f);
        bool isSlowEffect = (particle.color.x > 0.4f && particle.color.y > 0.4f && particle.color.y < 0.6f);

        // 应用重力和特殊效果
        if (isHealEffect) {
            // 治疗粒子：弱重力，慢慢上升
            particle.velocity.y -= 0.02f * deltaTime; // 更弱的重力

            // 螺旋上升效果增强
            float spiralAngle = (particle.maxLife - particle.life) * 6.0f;
            particle.position.x += sinf(spiralAngle) * 0.015f;
            particle.position.z += cosf(spiralAngle) * 0.015f;
        }
        else if (isSlipperyEffect) {
            // 冰面粒子：几乎没有重力，水平运动
            particle.velocity.y -= 0.01f * deltaTime;

            // 略微降低速度模拟摩擦
            particle.velocity.x *= (1.0f - 0.4f * deltaTime);
            particle.velocity.z *= (1.0f - 0.4f * deltaTime);
        }
        else if (isSlowEffect) {
            // 获取生命周期进度
            float lifeProgress = 1.0f - (particle.life / particle.maxLife);

            // 根据粒子尺寸确定其行为
            if (particle.size < 0.1f) {  // 小型上升粒子
                // 极小的重力
                particle.velocity.y -= 0.01f * deltaTime;

                // 强烈的升腾旋转效果
                float spiralAngle = lifeProgress * 12.0f;
                float spiralRadius = 0.02f * (1.0f - lifeProgress * lifeProgress);

                particle.position.x += sinf(spiralAngle) * spiralRadius;
                particle.position.z += cosf(spiralAngle) * spiralRadius;

                // 脉动效果 - 尺寸和亮度变化
                float pulseAmount = sinf(lifeProgress * 25.0f) * 0.2f;
                particle.size = particle.size * (1.0f + pulseAmount * 0.5f);

                // 颜色脉动 - 在蓝色和紫色之间变化
                if (pulseAmount > 0) {
                    particle.color.x = std::min(0.6f, particle.color.x + pulseAmount * 0.3f);  // 增加红色分量
                }
                else {
                    particle.color.x = std::max(0.0f, particle.color.x + pulseAmount * 0.3f);  // 减少红色分量
                }

                // 生命周期后期向上加速
                if (lifeProgress > 0.6f && particle.velocity.y < 0.08f) {
                    particle.velocity.y += 0.008f * deltaTime;
                }
            }
            else if (particle.size < 0.15f) {  // 中型扩散粒子
                // 轻微重力
                particle.velocity.y -= 0.03f * deltaTime;

                // 颜色增强 - 随时间增加蓝色分量
                particle.color.z = std::min(1.0f, particle.color.z + 0.1f * deltaTime);

                // 轻微旋转和摇摆效果
                if (lifeProgress > 0.3f) {
                    float wobbleAngle = lifeProgress * 20.0f;
                    float wobbleAmount = 0.002f * (1.0f - lifeProgress);
                    particle.position.x += sinf(wobbleAngle) * wobbleAmount;
                    particle.position.z += cosf(wobbleAngle) * wobbleAmount;
                }
            }
            else {  // 大型跳跃粒子
                // 正常重力
                particle.velocity.y -= 0.1f * deltaTime;

                // 反弹效果
                if (particle.position.y <= 0.05f && particle.velocity.y < 0) {
                    // 反弹
                    particle.velocity.y = -particle.velocity.y * 0.5f;

                    // 反弹时产生闪光效果 - 颜色瞬变为更亮的蓝紫色
                    if (particle.velocity.y > 0.02f) {
                        particle.color.x += 0.2f;  // 增加红色分量
                        particle.color.z = 1.0f;   // 最大蓝色
                        particle.size *= 1.1f;     // 瞬间增大尺寸
                    }
                }

                // 颜色随时间变化 - 由蓝转紫
                if (lifeProgress > 0.5f) {
                    particle.color.x = std::min(0.8f, particle.color.x + 0.08f * deltaTime);  // 增加红色分量
                }
            }

            // 所有粒子的共同行为

            // 接近地面的粒子
            if (particle.position.y <= 0.03f) {
                if (particle.velocity.y < 0) {
                    particle.velocity.y = 0;
                }

                // 快速减速
                particle.velocity.x *= 0.92f;
                particle.velocity.z *= 0.92f;

                // 地面粒子颜色加深为深蓝/深紫
                particle.color.z = std::max(0.6f, particle.color.z - 0.05f * deltaTime);
            }

        }
        else if (particle.size < 0.12f) {  // 中型扩散粒子
            // 中等重力
            particle.velocity.y -= 0.04f * deltaTime;

            // 轻微旋转效果
            if (rand() % 20 == 0) {
                float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
                float turnForce = 0.001f;
                particle.velocity.x += cosf(angle) * turnForce;
                particle.velocity.z += sinf(angle) * turnForce;
            }
        }
        else {  // 大型跳跃粒子
            // 正常重力
            particle.velocity.y -= 0.08f * deltaTime;

            // 接近地面时反弹
            if (particle.position.y <= 0.05f && particle.velocity.y < 0) {
                particle.velocity.y = -particle.velocity.y * 0.4f; // 反弹，但损失能量

                // 创建小型冲击粒子
                if (particle.velocity.y > 0.02f && rand() % 3 == 0) {
                    // 这里可以添加代码来在碰撞处生成额外的小粒子
                    // 由于需要访问球体对象，这部分逻辑可能需要单独处理
                }
            }
        }

        // 地面粒子减速效果
        if (particle.position.y <= 0.01f) {
            if (particle.velocity.y < 0) {
                particle.velocity.y = 0;
            }
            particle.velocity.x *= 0.94f;
            particle.velocity.z *= 0.94f;
        }
    

        else {
            // 标准重力
            particle.velocity.y -= 0.05f * deltaTime;
        }

        // 减少生命值
        particle.life -= deltaTime;

        // 更新透明度 - 只在生命周期的最后25%开始淡出
        if (particle.life < particle.maxLife * 0.25f) {
            float alpha = particle.life / (particle.maxLife * 0.25f);
            particle.color.w = alpha;
        }

        // 粒子消失
        if (particle.life <= 0) {
            particle.active = false;
        }
    }

    // 删除不活跃的粒子
    size_t oldSize = ball->activeEffects.size();

    if (oldSize > 0) {
        ball->activeEffects.erase(
            std::remove_if(ball->activeEffects.begin(), ball->activeEffects.end(),
                [](const ParticleEffect& p) { return !p.active; }),
            ball->activeEffects.end());
    }
}

void ParticleSystem::Draw() {
    BallObject* ball = GetBall();

    // 如果没有粒子效果，直接返回
    if (ball->activeEffects.empty()) return;

    // 每秒输出一次粒子数量信息，而不是每帧
    static DWORD lastOutputTime = 0;
    DWORD currentTime = GetTickCount();
    if (currentTime - lastOutputTime > 1000) {  // 每1秒输出一次
        char debug[64];
        sprintf_s(debug, "正在绘制 %zu 个粒子\n", ball->activeEffects.size());
        OutputDebugStringA(debug);
        lastOutputTime = currentTime;
    }
    // 保存当前渲染状态
    bool oldBlendEnable = GetBlendState();

    // 启用混合
    SetBlendState(true);

    // 使用粒子纹理
    ID3D11ShaderResourceView* particleTexture = m_particleTexture;

    for (const auto& particle : ball->activeEffects) {
        if (!particle.active) continue;

        // 创建广告牌矩阵
        XMMATRIX viewMatrix = GetViewMatrix();
        viewMatrix.r[3].m128_f32[0] = 0;
        viewMatrix.r[3].m128_f32[1] = 0;
        viewMatrix.r[3].m128_f32[2] = 0;
        viewMatrix.r[3].m128_f32[3] = 1;
        XMMATRIX billboardMatrix = XMMatrixTranspose(viewMatrix);
        billboardMatrix.r[3].m128_f32[0] = particle.position.x;
        billboardMatrix.r[3].m128_f32[1] = particle.position.y;
        billboardMatrix.r[3].m128_f32[2] = particle.position.z;
        billboardMatrix.r[3].m128_f32[3] = 1;

        // 应用缩放
        XMMATRIX scaleMatrix = XMMatrixScaling(particle.size, particle.size, particle.size);
        billboardMatrix = scaleMatrix * billboardMatrix;

        SetWorldMatrix(billboardMatrix);

        // 设置材质颜色
        MATERIAL material;
        ZeroMemory(&material, sizeof(material));
        material.Diffuse = particle.color;
        SetMaterial(material);

        // 设置纹理
        GetDeviceContext()->PSSetShaderResources(0, 1, &particleTexture);

        // 绘制一个简单的广告牌
        XMFLOAT2 size = XMFLOAT2(1.0f, 1.0f); // 使用1.0的标准化尺寸，实际大小由缩放矩阵控制
        DrawBillboard(size, material.Diffuse);
    }

    // 恢复混合状态
    SetBlendState(oldBlendEnable);
}

void ParticleSystem::CreateEffect(XMFLOAT3 position, int particleCount, XMFLOAT4 color, float speed, float size, float lifetime) {
    // 获取球体对象引用
    BallObject* ball = GetBall();

    // 确定这是什么类型的效果（根据颜色）
    bool isHealEffect = (color.y > 0.8f && color.x < 0.3f);     // 绿色为主：治疗
    bool isSlipperyEffect = (color.z > 0.8f && color.x < 0.6f); // 蓝色为主：冰面
    bool isDamageEffect = (color.x > 0.8f && color.y < 0.4f);   // 红色为主：伤害
    bool isSlowEffect = (color.x > 0.4f && color.y > 0.4f && color.y < 0.6f && color.z < 0.4f);  // 灰色：沙地

    // 对于沙地效果，调整生成位置在小球下方
    if (isSlowEffect) {
        position.y -= 0.1f; // 降低生成位置
    }
    else {
        // 确保位置有一定高度，避免被地面遮挡
        position.y += 0.3f;
    }

    for (int i = 0; i < particleCount; i++) {
        ParticleEffect particle;
        particle.position = position;

        // 基本随机角度
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float height = (float)(rand() % 100) / 100.0f;

        // 根据效果类型调整初始速度
        if (isHealEffect) {
            // 治疗效果：主要向上运动，螺旋上升
            particle.velocity.x = cosf(angle) * speed * 0.7f;
            particle.velocity.y = (0.7f + height * 0.3f) * speed * 3.5f; // 增强向上移动
            particle.velocity.z = sinf(angle) * speed * 0.7f;
        }
        else if (isSlipperyEffect) {
            // 冰面效果：主要水平运动，几乎不向上，扩散更广
            particle.velocity.x = cosf(angle) * speed * 2.5f;
            particle.velocity.y = height * speed * 0.5f; // 很少向上
            particle.velocity.z = sinf(angle) * speed * 2.5f;
        }
        else if (isDamageEffect) {
            // 伤害效果：爆发式向四周散开
            particle.velocity.x = cosf(angle) * speed * 2.0f;
            particle.velocity.y = (0.2f + height * 0.8f) * speed * 2.5f; // 向上但分散
            particle.velocity.z = sinf(angle) * speed * 2.0f;
        }
        else if (isSlowEffect) {
            // 沙地效果：更多水平扩散，缓慢上升
            particle.velocity.x = cosf(angle) * speed * 1.2f;
            particle.velocity.y = height * speed * 0.8f; // 少量向上
            particle.velocity.z = sinf(angle) * speed * 1.2f;
        }
        else {
            // 默认运动
            particle.velocity.x = cosf(angle) * speed * 1.3f;
            particle.velocity.y = height * speed * 2.0f;
            particle.velocity.z = sinf(angle) * speed * 1.3f;
        }

        // 设置粒子属性
        particle.color = color;

        // 为沙地效果特别调整大小
        if (isSlowEffect) {
            particle.size = size * 1.5f * (0.9f + (float)(rand() % 30) / 100.0f); // 沙地粒子更大
        }
        else {
            particle.size = size * 1.2f * (0.8f + (float)(rand() % 40) / 100.0f); // 其他粒子也略大
        }

        particle.life = lifetime * (0.8f + (float)(rand() % 40) / 100.0f); // 生命周期有少许随机变化
        particle.maxLife = particle.life;
        particle.active = true;

        ball->activeEffects.push_back(particle);
    }
}
// 特定类型的效果包装函数
void ParticleSystem::CreateHealEffect(XMFLOAT3 position, int count) {
    CreateEffect(position, count, XMFLOAT4(0.2f, 0.9f, 0.3f, 0.9f), 0.06f, 0.15f, 1.2f);
}

void ParticleSystem::CreateDamageEffect(XMFLOAT3 position, int count) {
    CreateEffect(position, count, XMFLOAT4(1.0f, 0.2f, 0.1f, 0.9f), 0.08f, 0.12f, 0.8f);
}

void ParticleSystem::CreateSlowEffect(XMFLOAT3 position, int count) {
    BallObject* ball = GetBall();

    // 大幅增加粒子数量
    count = count * 2.5;

    // 沙地基础位置稍微降低
    position.y -= 0.03f;

    // 第一批：地面扩散的深灰色粒子
    for (int i = 0; i < count / 2; i++) {
        ParticleEffect particle;

        // 扩大随机范围，使效果更加明显
        float randomOffsetX = ((float)(rand() % 100) / 100.0f - 0.5f) * 0.3f;
        float randomOffsetZ = ((float)(rand() % 100) / 100.0f - 0.5f) * 0.3f;

        particle.position = position;
        particle.position.x += randomOffsetX;
        particle.position.z += randomOffsetZ;

        // 水平扩散的速度
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float speed = 0.07f + ((float)(rand() % 50) / 100.0f) * 0.08f;

        particle.velocity.x = cosf(angle) * speed;
        particle.velocity.y = 0.04f + ((float)(rand() % 40) / 100.0f) * 0.05f;
        particle.velocity.z = sinf(angle) * speed;

        // 深灰色粒子
        float colorVar = ((float)(rand() % 20) / 100.0f);
        particle.color = XMFLOAT4(
            0.2f + colorVar * 0.1f,        // 深灰色 - 低红色
            0.2f + colorVar * 0.1f,        // 深灰色 - 低绿色
            0.2f + colorVar * 0.1f,        // 深灰色 - 低蓝色
            1.0f                           // 完全不透明
        );

        // 增大粒子尺寸
        particle.size = 0.08f + ((float)(rand() % 40) / 100.0f) * 0.07f;
        particle.life = 1.2f + ((float)(rand() % 40) / 100.0f) * 0.7f;
        particle.maxLife = particle.life;
        particle.active = true;

        ball->activeEffects.push_back(particle);
    }

    // 第二批：上升的中灰色粒子
    for (int i = 0; i < count / 2; i++) {
        ParticleEffect particle;

        float randomOffsetX = ((float)(rand() % 100) / 100.0f - 0.5f) * 0.35f;
        float randomOffsetZ = ((float)(rand() % 100) / 100.0f - 0.5f) * 0.35f;

        particle.position = position;
        particle.position.x += randomOffsetX;
        particle.position.z += randomOffsetZ;

        // 主要是向上的速度
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        float hSpeed = 0.02f + ((float)(rand() % 40) / 100.0f) * 0.05f;
        float vSpeed = 0.15f + ((float)(rand() % 70) / 100.0f) * 0.15f;

        particle.velocity.x = cosf(angle) * hSpeed;
        particle.velocity.y = vSpeed;
        particle.velocity.z = sinf(angle) * hSpeed;

        // 中灰色粒子
        float grayVar = ((float)(rand() % 30) / 100.0f);
        particle.color = XMFLOAT4(
            0.4f + grayVar * 0.1f,         // 中灰色 - 中红色
            0.4f + grayVar * 0.1f,         // 中灰色 - 中绿色
            0.4f + grayVar * 0.1f,         // 中灰色 - 中蓝色
            0.9f                           // 高不透明度
        );

        // 增大尺寸
        particle.size = 0.05f + ((float)(rand() % 30) / 100.0f) * 0.06f;
        particle.life = 1.8f + ((float)(rand() % 60) / 100.0f) * 1.0f;
        particle.maxLife = particle.life;
        particle.active = true;

        ball->activeEffects.push_back(particle);
    }

    //// 第三批：大型极深灰色粒子
    //for (int i = 0; i < count / 3; i++) {
    //    ParticleEffect particle;

    //    float randomOffsetX = ((float)(rand() % 100) / 100.0f - 0.5f) * 0.25f;
    //    float randomOffsetZ = ((float)(rand() % 100) / 100.0f - 0.5f) * 0.25f;

    //    particle.position = position;
    //    particle.position.x += randomOffsetX;
    //    particle.position.z += randomOffsetZ;

    //    // 跳跃式的运动
    //    float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
    //    float speed = 0.06f + ((float)(rand() % 50) / 100.0f) * 0.07f;

    //    particle.velocity.x = cosf(angle) * speed;
    //    particle.velocity.y = 0.2f + ((float)(rand() % 60) / 100.0f) * 0.15f;
    //    particle.velocity.z = sinf(angle) * speed;

    //    // 极深灰色粒子
    //    float darkVar = ((float)(rand() % 20) / 100.0f);
    //    particle.color = XMFLOAT4(
    //        0.1f + darkVar * 0.1f,         // 非常深的灰色 - 低红色
    //        0.1f + darkVar * 0.1f,         // 非常深的灰色 - 低绿色
    //        0.1f + darkVar * 0.1f,         // 非常深的灰色 - 低蓝色
    //        1.0f                           // 完全不透明
    //    );

    //    // 显著增大尺寸
    //    particle.size = 0.15f + ((float)(rand() % 40) / 100.0f) * 0.1f;
    //    particle.life = 1.0f + ((float)(rand() % 50) / 100.0f) * 0.6f;
    //    particle.maxLife = particle.life;
    //    particle.active = true;

    //    ball->activeEffects.push_back(particle);
    //}

    //// 第四批：爆发效果 - 亮灰色粒子
    //for (int i = 0; i < count / 6; i++) {
    //    ParticleEffect particle;

    //    // 中心位置，模拟冲击波
    //    particle.position = position;

    //    // 爆发式向外运动
    //    float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
    //    float burstSpeed = 0.15f + ((float)(rand() % 50) / 100.0f) * 0.1f;

    //    particle.velocity.x = cosf(angle) * burstSpeed;
    //    particle.velocity.y = 0.05f + ((float)(rand() % 40) / 100.0f) * 0.1f;
    //    particle.velocity.z = sinf(angle) * burstSpeed;

    //    // 亮灰色粒子
    //    particle.color = XMFLOAT4(
    //        0.6f,                          // 亮灰色 - 中高红色
    //        0.6f,                          // 亮灰色 - 中高绿色
    //        0.6f,                          // 亮灰色 - 中高蓝色
    //        0.9f                           // 高不透明度
    //    );

    //    // 较大尺寸但短生命周期
    //    particle.size = 0.12f + ((float)(rand() % 30) / 100.0f) * 0.08f;
    //    particle.life = 0.4f + ((float)(rand() % 30) / 100.0f) * 0.3f;
    //    particle.maxLife = particle.life;
    //    particle.active = true;

    //    ball->activeEffects.push_back(particle);
    //}
}

void ParticleSystem::CreateSlipperyEffect(XMFLOAT3 position, int count) {
    CreateEffect(position, count, XMFLOAT4(0.5f, 0.8f, 1.0f, 0.7f), 0.08f, 0.18f, 1.0f);
}

// 获取/设置粒子纹理
ID3D11ShaderResourceView* ParticleSystem::GetParticleTexture() {
    return m_particleTexture;
}

void ParticleSystem::SetParticleTexture(ID3D11ShaderResourceView* texture) {
    m_particleTexture = texture;
}

// ===== 浮动文本系统实现 =====

void FloatingTextSystem::Init() {
    m_floatingTexts.clear();
}

void FloatingTextSystem::Uninit() {
    m_floatingTexts.clear();
}

void FloatingTextSystem::ShowText(XMFLOAT3 position, const char* text, XMFLOAT4 color) {
    FloatingText floatingText;
    floatingText.text = text;

    floatingText.position = position;
    floatingText.position.y += 1.0f;

    floatingText.color = color;
    floatingText.color.w = 1.0f;  // 确保完全不透明

    floatingText.timer = 3.0f;    // 从2秒增加到3秒
    floatingText.alpha = 1.0f;
    floatingText.size = 2.0f;

    m_floatingTexts.push_back(floatingText);

   
}

void FloatingTextSystem::Update(float deltaTime) {
    deltaTime = std::min(deltaTime, 0.1f);

    for (auto& text : m_floatingTexts) {
        // 文本向上浮动
        text.position.y += 0.3f * deltaTime;  // 减慢上升速度

        // 水平摆动效果
        text.position.x += sinf(text.timer * 3.0f) * 0.01f * deltaTime;

        // 减少计时器
        text.timer -= deltaTime;

        // 更平滑的淡出效果
        if (text.timer < 1.0f) {  // 最后1秒开始淡出
            text.alpha = text.timer;
            text.size = 2.0f - (1.0f - text.timer) * 0.5f; // 更小的缩放变化
        }
    }

    // 移除过期的文本
    size_t beforeSize = m_floatingTexts.size();
    m_floatingTexts.erase(
        std::remove_if(m_floatingTexts.begin(), m_floatingTexts.end(),
            [](const FloatingText& t) { return t.timer <= 0; }),
        m_floatingTexts.end()
    );

    if (beforeSize != m_floatingTexts.size()) {
        char debug[128];
        sprintf_s(debug, "移除了 %zu 个过期文本，剩余 %zu 个\n",
            beforeSize - m_floatingTexts.size(), m_floatingTexts.size());
        OutputDebugStringA(debug);
    }
}
void FloatingTextSystem::Draw() {
    if (m_floatingTexts.empty()) return;
    
    // 保存渲染状态
    bool oldDepthEnable = GetDepthEnable();
    bool oldBlendEnable = GetBlendState();
    D3D11_CULL_MODE oldCullMode = GetCullingMode();

    // 设置文本渲染状态
    SetDepthEnable(false);
    SetBlendState(true);
    SetCulingMode(D3D11_CULL_NONE);

    for (const auto& text : m_floatingTexts) {
        // 将文本从多字节转换为宽字符
        wchar_t wideText[256] = { 0 };
        MultiByteToWideChar(CP_UTF8, 0, text.text.c_str(), -1, wideText, 256);

        // 使用新的几何文字渲染系统绘制文本
        XMFLOAT4 color = text.color;
        color.w = text.alpha;  // 应用淡出效果

        GeometricTextRenderer::DrawText3D(
            text.position,
            wideText,
            color,
            text.size, // 使用原始的大小值
            true       // 启用广告牌模式
        );

       
    }

    // 恢复渲染状态
    SetDepthEnable(oldDepthEnable);
    SetBlendState(oldBlendEnable);
    SetCulingMode(oldCullMode);
}