/*==============================================================================

   頂点管理 [sprite.cpp]
														 Author :
														 Date   :
--------------------------------------------------------------------------------

==============================================================================*/
#include	"main.h"
#include	"sprite.h"
#include "renderer.h"
#include <d2d1.h>
#include <dwrite.h>

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define NUM_SPRITEVERTEX 4

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void SetVertex(void);


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer				*g_VertexBuffer = NULL;		// 頂点情報

// DirectWrite相关对象
static ID2D1Factory* g_D2DFactory = NULL;
static ID2D1RenderTarget* g_D2DRenderTarget = NULL;
static IDWriteFactory* g_DWriteFactory = NULL;
static IDWriteTextFormat* g_TextFormat = NULL;
static ID2D1SolidColorBrush* g_TextBrush = NULL;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitSprite(void)
{
	ID3D11Device *pDevice = GetDevice();

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * NUM_SPRITEVERTEX;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitSprite(void)
{
	// 頂点バッファの解放
	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
}

//=============================================================================
// 描画処理
//====================

//void DrawSprite(XMFLOAT2 size, XMFLOAT4 color)
//{
//
//	//頂点の作成
//	D3D11_MAPPED_SUBRESOURCE msr;
//	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
//
//	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;
//
//	float size_W = size.x * 0.5f;
//	float size_H = size.y * 0.5f;
//
//	//左上 中心からサイズの半分の場所で座標を作る
//	vertex[0].Position = XMFLOAT3(-size_W, -size_H, 0.0f);
//	vertex[0].Diffuse = color;
//	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
//	//右上 中心からサイズの半分の場所で座標を作る
//	vertex[1].Position = XMFLOAT3(size_W, -size_H, 0.0f);
//	vertex[1].Diffuse = color;
//	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
//	//左下 中心からサイズの半分の場所で座標を作る
//	vertex[2].Position = XMFLOAT3(-size_W, size_H, 0.0f);
//	vertex[2].Diffuse = color;
//	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
//	//右下 中心からサイズの半分の場所で座標を作る
//	vertex[3].Position = XMFLOAT3(size_W, size_H, 0.0f);
//	vertex[3].Diffuse = color;
//	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);
//
//	GetDeviceContext()->Unmap(g_VertexBuffer, 0);
//
//
//	// 頂点バッファ設定
//	UINT stride = sizeof(VERTEX_3D);
//	UINT offset = 0;
//	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
//
//	// プリミティブトポロジ設定
//	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//
//	// マテリアル設定
//	MATERIAL material;
//	ZeroMemory(&material, sizeof(material));
//	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//	SetMaterial(material);
//
//	// ポリゴン描画
//	GetDeviceContext()->Draw(4, 0);
//}

void DrawSprite(XMFLOAT2 size, XMFLOAT4 color)
{
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float halfWidth = size.x / 2.0f;
	float halfHeight = size.y / 2.0f;

	vertex[0].Position = XMFLOAT3(-halfWidth, -halfHeight, 0.0f);
	vertex[0].Diffuse = color;
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

	vertex[1].Position = XMFLOAT3(halfWidth, -halfHeight, 0.0f);
	vertex[1].Diffuse = color;
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

	vertex[2].Position = XMFLOAT3(-halfWidth, halfHeight, 0.0f);
	vertex[2].Diffuse = color;
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

	vertex[3].Position = XMFLOAT3(halfWidth, halfHeight, 0.0f);
	vertex[3].Diffuse = color;
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	for (int i = 0; i < 4; i++) {
		vertex[i].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	}

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	// 顶点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = color;
	SetMaterial(material);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ポリゴン描画
	GetDeviceContext()->Draw(NUM_SPRITEVERTEX, 0);
}
void DrawBillboard(XMFLOAT2 size, XMFLOAT4 color)
{
	//頂点の作成
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float size_W = size.x * 0.5f;
	float size_H = size.y * 0.5f;

	//左上 中心からサイズの半分の場所で座標を作る
	vertex[0].Position = XMFLOAT3(-size_W, size_H, 0.0f);
	vertex[0].Diffuse = color;
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	//右上 中心からサイズの半分の場所で座標を作る
	vertex[1].Position = XMFLOAT3(size_W, size_H, 0.0f);
	vertex[1].Diffuse = color;
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	//左下 中心からサイズの半分の場所で座標を作る
	vertex[2].Position = XMFLOAT3(-size_W, -size_H, 0.0f);
	vertex[2].Diffuse = color;
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	//右下 中心からサイズの半分の場所で座標を作る
	vertex[3].Position = XMFLOAT3(size_W, -size_H, 0.0f);
	vertex[3].Diffuse = color;
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);


	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// ポリゴン描画
	GetDeviceContext()->Draw(4, 0);



}
// 在 sprite.cpp 中添加以下函数实现

void DrawSpriteLeftTop(XMFLOAT2 position, XMFLOAT2 size, XMFLOAT2 texCoordMin, XMFLOAT2 texCoordMax, XMFLOAT4 color, float angle)
{
	// 顶点创建
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float centerX = position.x + size.x / 2;
	float centerY = position.y + size.y / 2;

	// 如果有角度，则创建旋转矩阵
	XMMATRIX rotMat = XMMatrixIdentity();
	if (angle != 0.0f) {
		rotMat = XMMatrixRotationZ(XMConvertToRadians(angle));
	}

	// 左上 - 以左上角为原点
	XMVECTOR pos = XMVectorSet(position.x - centerX, position.y - centerY, 0.0f, 0.0f);
	pos = XMVector3TransformCoord(pos, rotMat);
	vertex[0].Position = XMFLOAT3(XMVectorGetX(pos) + centerX, XMVectorGetY(pos) + centerY, 0.0f);
	vertex[0].Diffuse = color;
	vertex[0].TexCoord = XMFLOAT2(texCoordMin.x, texCoordMin.y);

	// 右上
	pos = XMVectorSet(position.x + size.x - centerX, position.y - centerY, 0.0f, 0.0f);
	pos = XMVector3TransformCoord(pos, rotMat);
	vertex[1].Position = XMFLOAT3(XMVectorGetX(pos) + centerX, XMVectorGetY(pos) + centerY, 0.0f);
	vertex[1].Diffuse = color;
	vertex[1].TexCoord = XMFLOAT2(texCoordMax.x, texCoordMin.y);

	// 左下
	pos = XMVectorSet(position.x - centerX, position.y + size.y - centerY, 0.0f, 0.0f);
	pos = XMVector3TransformCoord(pos, rotMat);
	vertex[2].Position = XMFLOAT3(XMVectorGetX(pos) + centerX, XMVectorGetY(pos) + centerY, 0.0f);
	vertex[2].Diffuse = color;
	vertex[2].TexCoord = XMFLOAT2(texCoordMin.x, texCoordMax.y);

	// 右下
	pos = XMVectorSet(position.x + size.x - centerX, position.y + size.y - centerY, 0.0f, 0.0f);
	pos = XMVector3TransformCoord(pos, rotMat);
	vertex[3].Position = XMFLOAT3(XMVectorGetX(pos) + centerX, XMVectorGetY(pos) + centerY, 0.0f);
	vertex[3].Diffuse = color;
	vertex[3].TexCoord = XMFLOAT2(texCoordMax.x, texCoordMax.y);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	// 顶点缓冲设置
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// 绘制设置
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 材质设置
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	// 绘制
	GetDeviceContext()->Draw(4, 0);
}

void DrawSpriteUV(XMFLOAT2 size, XMFLOAT4 color, XMFLOAT2 uvMin, XMFLOAT2 uvMax)
{
	

	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float halfWidth = size.x * 0.5f;
	float halfHeight = size.y * 0.5f;

	vertex[0].Position = XMFLOAT3(-halfWidth, -halfHeight, 0.0f);
	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[0].Diffuse = color;
	vertex[0].TexCoord = XMFLOAT2(uvMin.x, uvMin.y);

	vertex[1].Position = XMFLOAT3(halfWidth, -halfHeight, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[1].Diffuse = color;
	vertex[1].TexCoord = XMFLOAT2(uvMax.x, uvMin.y);

	vertex[2].Position = XMFLOAT3(-halfWidth, halfHeight, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[2].Diffuse = color;
	vertex[2].TexCoord = XMFLOAT2(uvMin.x, uvMax.y);

	vertex[3].Position = XMFLOAT3(halfWidth, halfHeight, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	vertex[3].Diffuse = color;
	vertex[3].TexCoord = XMFLOAT2(uvMax.x, uvMax.y);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = color;
	SetMaterial(material);

	// 绘制
	GetDeviceContext()->Draw(4, 0);

}

