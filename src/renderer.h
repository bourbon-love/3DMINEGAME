/*==============================================================================

   レンダリング管理[renderer.h]
														 Author :
														 Date   :
--------------------------------------------------------------------------------

==============================================================================*/
#pragma once

#include "main.h"
//*********************************************************
// 構造体
//*********************************************************

// 頂点構造体
struct VERTEX_3D
{
	XMFLOAT3 Position;	//頂点座標　XMFLOAT3 ＝　float x,y,z
	XMFLOAT3 Normal;	//法線ベクトル 
	XMFLOAT4 Diffuse;	//色  XMFLOAT4 = float x,y,z,w
	XMFLOAT2 TexCoord;	//テクスチャ座標 XMFLOAT2 = float x,y
};

// マテリアル構造体
struct MATERIAL
{
	XMFLOAT4	Ambient;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Specular;
	XMFLOAT4	Emission;
	float		Shininess;
	float		Dummy[3];//16byte境界用
};
//ライト構造体	//Lighting追加
struct LIGHT
{
	XMFLOAT4	Direction;
	XMFLOAT4	Diffuse;
	XMFLOAT4	Ambient;
	BOOL		Enable;			//12/11
	float		Dummy[3];		//12/11

};
extern ID3D11VertexShader* g_SkyBoxVS;
extern ID3D11PixelShader* g_SkyBoxPS;
extern ID3D11InputLayout* g_SkyBoxLayout;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitRenderer(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void UninitRenderer(void);

void Clear(void);
void Present(void);

ID3D11Device *GetDevice( void );
ID3D11DeviceContext *GetDeviceContext( void );

void SetDepthEnable( bool Enable );

void SetWorldViewProjection2D(void);
void SetWorldViewProjection3D(void);


void SetWorldMatrix(XMMATRIX WorldMatrix );
void SetViewMatrix(XMMATRIX ViewMatrix );
void SetProjectionMatrix(XMMATRIX ProjectionMatrix );


void SetMaterial( MATERIAL Material );

void SetLight(LIGHT	Light);	////////////Lighting追加

void SetCulingMode(D3D11_CULL_MODE flag);//カリング制御

void SetATCEnable(bool Enable);	//AlphaToCoverage制御

// 深度测试状态获取和设置
bool GetDepthEnable(void);
void SetDepthEnable(bool Enable);

// 混合状态获取和设置
bool GetBlendState(void);
void SetBlendState(bool Enable);


ID3D11ShaderResourceView* GetPETexture();
void BeginPE();
void Begin();


// 获取当前裁剪模式
D3D11_CULL_MODE GetCullingMode(void);

XMMATRIX GetWorldMatrix();

void GetMaterial(MATERIAL* outMaterial);
// 设置深度写入状态
void SetDepthWriteEnable(bool Enable);

bool GetDepthWriteEnable(void);

extern ID3D11ShaderResourceView* g_TextureWhite;
void CreateWhiteTexture();