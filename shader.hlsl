// =======================
// Constant Buffers
// =======================
cbuffer ConstantBuffer : register(b0)
{
    matrix WorldViewProjection;
};
cbuffer MaterialBuffer : register(b1)
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    float3 Dummy;
};
struct LIGHT
{
    float4 Direction;
    float4 Diffuse;
    float4 Ambient;
    bool Enable;
    float3 Dummy;
};
cbuffer LightBuffer : register(b2)
{
    LIGHT Light;
};
cbuffer WorldMatrixBuffer : register(b3)
{
    matrix World;
};
//*****************************************************************************
// (新) highlight シェーダ
//*****************************************************************************
cbuffer HighlightBuffer : register(b4)
{
    float4 g_HighlightColor; // RGBA
}

// =======================
// 天空盒视图投影
// =======================
cbuffer ViewProjectionBuffer : register(b5)
{
    matrix g_ViewProjection;
};

// =======================
// Vertex Shader Input/Output
// =======================
struct VS_IN
{
    float4 Position : POSITION0;
    float4 Normal : NORMAL0;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
};
struct VS_OUT
{
    float4 Position : SV_POSITION;
    float4 Normal : NORMAL0;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

// =======================
// 通用多边形渲染 Vertex Shader
// =======================
VS_OUT VertexShaderPolygon(VS_IN input)
{
    VS_OUT output;
    output.Position = mul(input.Position, WorldViewProjection);
    float4 normal = float4(input.Normal.xyz, 0.0f);
    output.Normal = mul(normal, World);
    output.Color = input.Color * Diffuse;
    output.TexCoord = input.TexCoord;
    return output;
}

// =======================
// 通用多边形渲染 Pixel Shader
// =======================
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

float4 PixelShaderPolygon(VS_OUT input) : SV_Target
{
    float3 light = 1.0f;
    float4 texColor;
    if (input.Color.a < 0.99f)
    {
        texColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        texColor = g_Texture.Sample(g_SamplerState, input.TexCoord);
    }

    float4 outputColor = texColor;
    if (Light.Enable)
    {
        float3 normal = normalize(input.Normal.xyz);
        light = dot(normal, -Light.Direction.xyz);
        light = clamp(light, 0.1f, 1.0f);
        light += Light.Ambient.rgb;
        outputColor *= (input.Color * Light.Diffuse);
    }
    else
    {
        outputColor *= input.Color;
    }

    outputColor.rgb *= light;
    if (outputColor.a <= 0.0f)
        discard;
    return outputColor;
}

// =======================
// 天空盒 Shader
// =======================
struct SKY_VS_IN
{
    float3 Position : POSITION;
};
struct SKY_VS_OUT
{
    float4 Position : SV_POSITION;
    float3 WorldDir : TEXCOORD0;
};

SKY_VS_OUT SkyGradientVS(SKY_VS_IN input)
{
    SKY_VS_OUT output;
    float4 pos = float4(input.Position, 1.0f);
    output.Position = mul(pos, g_ViewProjection);
    output.WorldDir = normalize(input.Position);
    return output;
}

float4 SkyGradientPS(SKY_VS_OUT input) : SV_Target
{
    //float3 topColor = float3(0.2, 0.4, 0.9);
    //float3 bottomColor = float3(1.0, 1.0, 1.0);
    //float t = saturate(input.WorldDir.y * 0.5 + 0.5);
    //float3 color = lerp(bottomColor, topColor, t);
    //return float4(color, 1.0);
    float3 dir = normalize(input.WorldDir);
    return float4(saturate(abs(dir)), 1.0); // 红 X，绿 Y，蓝 Z
  
}
