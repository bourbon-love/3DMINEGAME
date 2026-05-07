
//Bg.cpp

#include	"Bg.h"
#include	"sprite.h"

BgObject	g_Bg1;	//空
static	ID3D11Buffer* g_VertexBuffer = NULL;//テスト用頂点バッファ
static	ID3D11Buffer* g_IndexBuffer = NULL;//テスト用インデックスバッファ//追加


void	InitBg()
{
#define	FIELD_NUM_VERTEX	(4)
	VERTEX_3D	Box[FIELD_NUM_VERTEX] =
	{
		{//頂点V0 LEFT-TOP
			XMFLOAT3(-3000.0f, 0.0f, 3000.0f),	//頂点座標
			XMFLOAT3(0.0f,1.0f,0.0f),		//法線
			XMFLOAT4(1.0f,1.0f,1.0f,1.0f),	//カラー
			XMFLOAT2(0.0f,0.0f)				//テクスチャ座標
		},
		{//頂点V1 LEFT-TOP
			XMFLOAT3(3000.0f, 0.0f, 3000.0f),	//頂点座標
			XMFLOAT3(0.0f,1.0f,0.0f),		//法線
			XMFLOAT4(1.0f,1.0f,1.0f,1.0f),	//カラー
			XMFLOAT2(100.0f,0.0f)				//テクスチャ座標
		},
		{//頂点V2 LEFT-TOP
			XMFLOAT3(-3000.0f, 0.0f, -3000.0f),	//頂点座標
			XMFLOAT3(0.0f,1.0f,0.0f),		//法線
			XMFLOAT4(1.0f,1.0f,1.0f,1.0f),	//カラー
			XMFLOAT2(0.0f,100.0f)				//テクスチャ座標
		},
		{//頂点V1 LEFT-TOP
			XMFLOAT3(3000.0f, 0.0f, -3000.0f),	//頂点座標
			XMFLOAT3(0.0f,1.0f,0.0f),		//法線
			XMFLOAT4(1.0f,1.0f,1.0f,1.0f),	//カラー
			XMFLOAT2(100.0f,100.0f)				//テクスチャ座標
		}
	};

	//頂点バッファ作成
	{
		D3D11_BUFFER_DESC	bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(VERTEX_3D) * FIELD_NUM_VERTEX;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

		//頂点バッファの書き込み先のポインターを取得
		D3D11_MAPPED_SUBRESOURCE	msr;
		GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
		VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

		//頂点データをコピー
		CopyMemory(&vertex[0], &Box[0], sizeof(VERTEX_3D) * FIELD_NUM_VERTEX);
		//書き込み完了
		GetDeviceContext()->Unmap(g_VertexBuffer, 0);
	}


	g_Bg1.position = XMFLOAT3(0.0f, -0.1f, 0.0f);
	g_Bg1.rotate = XMFLOAT3(0, 0, 0);
	g_Bg1.scale = XMFLOAT3(1, 1, 1);
	g_Bg1.height = 0.0f;
	g_Bg1.Use = true;

	TexMetadata		metadata;
	ScratchImage	image;
	LoadFromWICFile(L"asset\\texture\\Field.jpg", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(GetDevice(), image.GetImages(),
		image.GetImageCount(), metadata, &g_Bg1.TexID);
	assert(g_Bg1.TexID);




}
void	UninitBg()
{
	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
	}

	if (g_Bg1.TexID != NULL)
	{
		g_Bg1.TexID->Release();
	}
}
void	UpdateBg()
{
	g_Bg1.rotate.y += 0.015f;

	g_Bg1.position.y = sinf(XMConvertToRadians(g_Bg1.height)) * 0.05f + (-0.1f);
	g_Bg1.height += 0.5f;

}
void	DrawBg()
{
	if (g_Bg1.Use != true) return;

	//平行移動行列作成
	XMMATRIX	TranslationMatrix =
		XMMatrixTranslation(
			g_Bg1.position.x,
			g_Bg1.position.y,
			g_Bg1.position.z
		);
	//回転行列作成
	XMMATRIX	RotationMatrix =
		XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(g_Bg1.rotate.x),
			XMConvertToRadians(g_Bg1.rotate.y),
			XMConvertToRadians(g_Bg1.rotate.z)
		);
	//スケーリング行列作成
	XMMATRIX	ScalingMatrix =
		XMMatrixScaling(
			g_Bg1.scale.x,
			g_Bg1.scale.y,
			g_Bg1.scale.z
		);
	//ワールド行列作成 ※乗算の順番に注意
	XMMATRIX	WorldMatrix =
		ScalingMatrix *
		RotationMatrix *
		TranslationMatrix;
	//ワールド行列をセット
	SetWorldMatrix(WorldMatrix);

	//頂点バッファをセット
	UINT	stride = sizeof(VERTEX_3D);
	UINT	offset = 0;
	GetDeviceContext()->IASetVertexBuffers(
		0,
		1,
		&g_VertexBuffer,
		&stride,
		&offset
	);

	//インデックスバッファをセット
	GetDeviceContext()->IASetIndexBuffer(g_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);//<<追加

	//プリミティブトポロジーの設定
	GetDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
	);
	//マテリアル設定
	MATERIAL	material;
	ZeroMemory(&material, sizeof(MATERIAL));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	SetMaterial(material);

	//テクスチャセット
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Bg1.TexID);

	SetCulingMode(D3D11_CULL_BACK);

	GetDeviceContext()->Draw(FIELD_NUM_VERTEX, 0);//インデックス無し描画

}




