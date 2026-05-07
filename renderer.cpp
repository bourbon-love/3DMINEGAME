/*==============================================================================

   レンダリング管理 [renderer.cpp]
														 Author :
														 Date   :
--------------------------------------------------------------------------------

==============================================================================*/
#include "renderer.h"
#include "sprite.h"


//*********************************************************
// 構造体
//*********************************************************


//*****************************************************************************
// グローバル変数:
//*****************************************************************************
D3D_FEATURE_LEVEL       g_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

ID3D11Device*           g_D3DDevice = NULL;
ID3D11DeviceContext*    g_ImmediateContext = NULL;
IDXGISwapChain*         g_SwapChain = NULL;
ID3D11RenderTargetView* g_RenderTargetView = NULL;
ID3D11DepthStencilView* g_DepthStencilView = NULL;
//レンダリングテクスチャ変数をグローバル変数で作成
ID3D11RenderTargetView*		g_PERenderTargetView = NULL;
ID3D11ShaderResourceView*   g_PEShaderResourceView = NULL;


ID3D11VertexShader*     g_VertexShader = NULL;
ID3D11PixelShader*      g_PixelShader = NULL;
ID3D11InputLayout*      g_VertexLayout = NULL;
ID3D11Buffer*			g_ConstantBuffer = NULL;
ID3D11Buffer*			g_MaterialBuffer = NULL;
ID3D11Buffer*			g_WorldMatrixBuffer = NULL;
ID3D11Buffer*			g_LightBuffer = NULL;

static bool g_CurrentDepthWriteEnable = true; // 默认启用深度写入
// 缓存两套深度状态对象（写入 / 不写入）
static ID3D11DepthStencilState* g_DepthState_ZWriteOn = nullptr;
static ID3D11DepthStencilState* g_DepthState_ZWriteOff = nullptr;
ID3D11ShaderResourceView* g_TextureWhite = NULL;
XMMATRIX				g_WorldMatrix;
XMMATRIX				g_ViewMatrix;
XMMATRIX				g_ProjectionMatrix; 
static MATERIAL g_CurrentMaterial = {
	{1.0f, 1.0f, 1.0f, 1.0f}, // Ambient
	{1.0f, 1.0f, 1.0f, 1.0f}, // Diffuse
	{0.0f, 0.0f, 0.0f, 0.0f}, // Specular
	{0.0f, 0.0f, 0.0f, 0.0f}, // Emission
	0.0f                      // Shininess
};


ID3D11DepthStencilState* g_DepthStateEnable;
ID3D11DepthStencilState* g_DepthStateDisable;

ID3D11BlendState* g_BlendState{};
ID3D11BlendState* g_BlendStateATC{};


ID3DBlob* pSkyVSBlob = NULL;
ID3DBlob* pSkyPSBlob = NULL;
ID3DBlob* pErrorBlob = NULL;

// 当前字体大小
static float g_FontSize = 16.0f;  // 默认16pt
// 在renderer.cpp文件顶部添加全局变量来跟踪渲染状态
static bool g_CurrentDepthState = true;  // 默认启用深度测试
static bool g_CurrentBlendState = false; // 默认禁用混合
static D3D11_CULL_MODE g_CurrentCullMode = D3D11_CULL_BACK; // 默认背面裁剪
// 在 renderer.cpp 中添加以下函数实现

// 获取当前深度测试状态
bool GetDepthEnable(void)
{
	return g_CurrentDepthState;
}
void SetDepthEnable(bool Enable)
{
	g_CurrentDepthState = Enable;
	ID3D11DepthStencilState* use =
		Enable ? (g_CurrentDepthWriteEnable ? g_DepthState_ZWriteOn
			: g_DepthState_ZWriteOff)
		: g_DepthState_ZWriteOff;   // 关闭测试时无所谓写不写
	g_ImmediateContext->OMSetDepthStencilState(use, 0);
}
// ▶ PATCH-END

void SetBlendState(bool Enable)
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (Enable)
	{
		g_ImmediateContext->OMSetBlendState(g_BlendState, blendFactor, 0xffffffff);
	}
	else
	{
		g_ImmediateContext->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	}

	// 记录当前状态
	g_CurrentBlendState = Enable;
}

// 获取当前混合状态
bool GetBlendState(void)
{
	return  g_CurrentBlendState;
}

void CreateWhiteTexture()
{
	// 创建一个1x1白色纹理
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	// 白色像素数据
	UINT whitePixel = 0xffffffff;
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &whitePixel;
	data.SysMemPitch = sizeof(UINT);
	data.SysMemSlicePitch = 0;

	ID3D11Texture2D* pTexture = NULL;
	g_D3DDevice->CreateTexture2D(&desc, &data, &pTexture);

	// 创建着色器资源视图
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	g_D3DDevice->CreateShaderResourceView(pTexture, &srvDesc, &g_TextureWhite);

	// 释放临时纹理
	if (pTexture) pTexture->Release();
}

ID3D11Device* GetDevice( void )
{
	return g_D3DDevice;
}


ID3D11DeviceContext* GetDeviceContext( void )
{
	return g_ImmediateContext;
}



void SetWorldViewProjection3D(void)
{
	g_ProjectionMatrix = XMMatrixIdentity();
	g_ViewMatrix = XMMatrixIdentity();//行列を単位行列にして初期化
	g_WorldMatrix = XMMatrixIdentity();
}

void SetWorldViewProjection2D( void )
{
	XMMATRIX worldViewProjection;

	g_ProjectionMatrix = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
	g_ViewMatrix = XMMatrixIdentity();//行列を単位行列にして初期化
	g_WorldMatrix = XMMatrixIdentity();
}


void SetWorldMatrix( XMMATRIX WorldMatrix )
{
	g_WorldMatrix = WorldMatrix;

	XMMATRIX worldViewProjection = g_WorldMatrix * g_ViewMatrix * g_ProjectionMatrix;
	worldViewProjection = XMMatrixTranspose(worldViewProjection);

	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, worldViewProjection);
	GetDeviceContext()->UpdateSubresource(g_ConstantBuffer, 0, NULL, &matrix, 0, 0);

	XMMATRIX world = XMMatrixTranspose(g_WorldMatrix);
	XMStoreFloat4x4(&matrix, world);	//////////////Lighting追加
	GetDeviceContext()->UpdateSubresource(g_WorldMatrixBuffer, 0, NULL, &matrix, 0, 0);
}

void SetViewMatrix( XMMATRIX ViewMatrix )
{
	g_ViewMatrix = ViewMatrix;

	XMMATRIX worldViewProjection = g_WorldMatrix * g_ViewMatrix * g_ProjectionMatrix;
	worldViewProjection = XMMatrixTranspose(worldViewProjection);

	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, worldViewProjection);
	GetDeviceContext()->UpdateSubresource(g_ConstantBuffer, 0, NULL, &matrix, 0, 0);
}

void SetProjectionMatrix( XMMATRIX ProjectionMatrix )
{
	g_ProjectionMatrix = ProjectionMatrix;

	XMMATRIX worldViewProjection = g_WorldMatrix * g_ViewMatrix * g_ProjectionMatrix;
	worldViewProjection = XMMatrixTranspose(worldViewProjection);

	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, worldViewProjection);
	GetDeviceContext()->UpdateSubresource(g_ConstantBuffer, 0, NULL, &matrix, 0, 0);
}



//void SetMaterial( MATERIAL Material )
//{
//
//	GetDeviceContext()->UpdateSubresource( g_MaterialBuffer, 0, NULL, &Material, 0, 0 );
//
//}
// 在renderer.cpp中修改SetMaterial函数，保存当前设置的材质
void SetMaterial(MATERIAL Material)
{
	// 保存当前材质
	g_CurrentMaterial = Material;

	// 更新常量缓冲区
	GetDeviceContext()->UpdateSubresource(g_MaterialBuffer, 0, NULL, &Material, 0, 0);
}
void SetLight(LIGHT	Light)////////////Lighting追加
{

	GetDeviceContext()->UpdateSubresource(g_LightBuffer, 0, NULL, &Light, 0, 0);

}

//void SetCulingMode(D3D11_CULL_MODE flag)
//{
//	D3D11_RASTERIZER_DESC rd;
//	ZeroMemory(&rd, sizeof(rd));
//	rd.FillMode = D3D11_FILL_SOLID;
//	rd.CullMode = flag;
//
//	rd.DepthClipEnable = TRUE;
//	rd.MultisampleEnable = FALSE;
//
//	ID3D11RasterizerState* rs;
//	g_D3DDevice->CreateRasterizerState(&rd, &rs);
//
//	g_ImmediateContext->RSSetState(rs);
//
//
//}
// 修复SetCulingMode函数
void SetCulingMode(D3D11_CULL_MODE flag)
{
	D3D11_RASTERIZER_DESC rd;
	ZeroMemory(&rd, sizeof(rd));
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = flag;
	rd.DepthClipEnable = TRUE;
	rd.MultisampleEnable = FALSE;

	ID3D11RasterizerState* rs;
	g_D3DDevice->CreateRasterizerState(&rd, &rs);
	g_ImmediateContext->RSSetState(rs);

	// 记录当前状态
	g_CurrentCullMode = flag;

	// 释放临时创建的状态对象
	rs->Release();
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitRenderer(HINSTANCE hInstance, HWND hWnd, BOOL bWindow)
{
	HRESULT hr = S_OK;

	// デバイス、スワップチェーン、コンテキスト生成
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = SCREEN_WIDTH;
	sd.BufferDesc.Height = SCREEN_HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&g_SwapChain,
		&g_D3DDevice,
		&g_FeatureLevel,
		&g_ImmediateContext);
	if (FAILED(hr))
		return hr;


	// レンダーターゲットビュー生成、設定
	ID3D11Texture2D* pBackBuffer = NULL;
	g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	g_D3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_RenderTargetView);
	pBackBuffer->Release();



	//ステンシル用テクスチャー作成
	ID3D11Texture2D* depthTexture = NULL;
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = sd.BufferDesc.Width;
	td.Height = sd.BufferDesc.Height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	td.SampleDesc = sd.SampleDesc;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;
	g_D3DDevice->CreateTexture2D(&td, NULL, &depthTexture);

	//ステンシルターゲット作成
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(dsvd));
	dsvd.Format = td.Format;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Flags = 0;
	g_D3DDevice->CreateDepthStencilView(depthTexture, &dsvd, &g_DepthStencilView);


	g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, g_DepthStencilView);


	// ビューポート設定
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)SCREEN_WIDTH;
	vp.Height = (FLOAT)SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_ImmediateContext->RSSetViewports(1, &vp);



	// ラスタライザステート設定
	D3D11_RASTERIZER_DESC rd;
	ZeroMemory(&rd, sizeof(rd));
	rd.FillMode = D3D11_FILL_SOLID;
	//	rd.CullMode = D3D11_CULL_NONE;	//カリングしない（裏も表も表示しちゃう）
	rd.CullMode = D3D11_CULL_BACK;	//裏面をカリングする（裏面は表示しない）
	//	rd.CullMode = D3D11_CULL_FRONT;	//表面をカリングする（表面は表示しない）

	rd.DepthClipEnable = TRUE;
	rd.MultisampleEnable = FALSE;

	ID3D11RasterizerState* rs;
	g_D3DDevice->CreateRasterizerState(&rd, &rs);

	g_ImmediateContext->RSSetState(rs);




	// ブレンドステート設定
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//	ID3D11BlendState* blendState = NULL;
	g_D3DDevice->CreateBlendState(&blendDesc, &g_BlendState);
	g_ImmediateContext->OMSetBlendState(g_BlendState, blendFactor, 0xffffffff);

	blendDesc.AlphaToCoverageEnable = TRUE;
	g_D3DDevice->CreateBlendState(&blendDesc, &g_BlendStateATC);
	g_ImmediateContext->OMSetBlendState(g_BlendStateATC, blendFactor, 0xffffffff);




	//// 深度ステンシルステート設定
	//D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	//ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	//depthStencilDesc.DepthEnable = TRUE;
	//depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	//depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	//depthStencilDesc.StencilEnable = FALSE;

	//g_D3DDevice->CreateDepthStencilState(&depthStencilDesc, &g_DepthStateEnable);//深度有効ステート

	////depthStencilDesc.DepthEnable = FALSE;
	//depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//g_D3DDevice->CreateDepthStencilState(&depthStencilDesc, &g_DepthStateDisable);//深度無効ステート

	//g_ImmediateContext->OMSetDepthStencilState(g_DepthStateEnable, NULL);
	// ▶ PATCH-BEGIN  深度ステンシルステート設定
	D3D11_DEPTH_STENCIL_DESC dsd{};
	dsd.DepthEnable = TRUE;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	dsd.StencilEnable = FALSE;

	/* ZWrite = ON */
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	g_D3DDevice->CreateDepthStencilState(&dsd, &g_DepthState_ZWriteOn);

	/* ZWrite = OFF */
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	g_D3DDevice->CreateDepthStencilState(&dsd, &g_DepthState_ZWriteOff);

	/* 默认使用 ON */
	g_ImmediateContext->OMSetDepthStencilState(g_DepthState_ZWriteOn, 0);
	g_CurrentDepthState = true;   // 深度测试 = ON
	g_CurrentDepthWriteEnable = true;   // 深度写入 = ON
	// ▶ PATCH-END




	// サンプラーステート設定
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	//	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState* samplerState = NULL;
	g_D3DDevice->CreateSamplerState(&samplerDesc, &samplerState);

	g_ImmediateContext->PSSetSamplers(0, 1, &samplerState);



	// 頂点シェーダコンパイル・生成
	ID3DBlob* pErrorBlob;
	ID3DBlob* pVSBlob = NULL;
	hr = D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "VertexShaderPolygon", "vs_4_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShader);

	// 入力レイアウト生成
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	g_D3DDevice->CreateInputLayout(layout,
		numElements,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&g_VertexLayout);

	pVSBlob->Release();
	//=========================//

	// ピクセルシェーダコンパイル・生成
	ID3DBlob* pPSBlob = NULL;
	hr = D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "PixelShaderPolygon", "ps_4_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPSBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	g_D3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShader);

	pPSBlob->Release();

	

	// 定数バッファ生成
	D3D11_BUFFER_DESC hBufferDesc;
	hBufferDesc.ByteWidth = sizeof(XMMATRIX);
	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	hBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hBufferDesc.CPUAccessFlags = 0;
	hBufferDesc.MiscFlags = 0;
	hBufferDesc.StructureByteStride = sizeof(float);

	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_ConstantBuffer);
	g_ImmediateContext->VSSetConstantBuffers(0, 1, &g_ConstantBuffer);

	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_WorldMatrixBuffer);///////Lighting追加
	g_ImmediateContext->VSSetConstantBuffers(3, 1, &g_WorldMatrixBuffer);

	hBufferDesc.ByteWidth = sizeof(MATERIAL);
	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	hBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hBufferDesc.CPUAccessFlags = 0;
	hBufferDesc.MiscFlags = 0;
	hBufferDesc.StructureByteStride = sizeof(float);

	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_MaterialBuffer);
	g_ImmediateContext->VSSetConstantBuffers(1, 1, &g_MaterialBuffer);

	// 关键修改：确保材质缓冲也绑定到像素着色器
	g_ImmediateContext->PSSetConstantBuffers(1, 1, &g_MaterialBuffer);
	hBufferDesc.ByteWidth = sizeof(LIGHT);
	g_D3DDevice->CreateBuffer(&hBufferDesc, NULL, &g_LightBuffer);///////Lighting追加
	g_ImmediateContext->PSSetConstantBuffers(2, 1, &g_LightBuffer);




	// 入力レイアウト設定
	g_ImmediateContext->IASetInputLayout(g_VertexLayout);

	// シェーダ設定
	g_ImmediateContext->VSSetShader(g_VertexShader, NULL, 0);
	g_ImmediateContext->PSSetShader(g_PixelShader, NULL, 0);





	{	//20250212追加　　レンダリングテクスチャ
	ID3D11Texture2D* ppTexture = NULL;
	D3D11_TEXTURE2D_DESC td;	//テクスチャ作成用デスクリプタ構造体関数
	ZeroMemory(&td, sizeof(td));//構造体を0初期化

	td.Width = sd.BufferDesc.Width;//構造体ｓｄはinit関数で最初で作られているので流用
	td.Height = sd.BufferDesc.Height;//バックバッファのサイズ情報が格納されている

	td.MipLevels = 1;//ミップマップの数　　0は限界まで作る
	td.ArraySize = 1;

	td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//ピクセルフォーマット
	td.SampleDesc = sd.SampleDesc;
	td.Usage = D3D11_USAGE_DEFAULT;

	//使用法のフラグをレンダリングテクスチャ用に設定
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;
	//構造体tdの設定に従ってテクスチャ領域を作成
	g_D3DDevice->CreateTexture2D(&td, NULL, &ppTexture);

	//レンダーターゲットビュー作成
	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	ZeroMemory(&rtvd, sizeof(rtvd));
	rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//ピクセルフォーマット
	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;//テクスチャの種類
	//作成したテクスチャ領域をレンダーターゲットとして設定
	g_D3DDevice->CreateRenderTargetView(ppTexture, &rtvd, &g_PERenderTargetView);

	//シェーダーリソースビュー作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	ZeroMemory(&srvd, sizeof(srvd));
	srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//ピクセルフォーマット
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;//テクスチャ種類
	srvd.Texture2D.MipLevels = 1;

	//シェーダーからアクセス可能にする
	g_D3DDevice->CreateShaderResourceView(ppTexture, &srvd, &g_PEShaderResourceView);
	}

	CreateWhiteTexture();
	return S_OK;
}


//=============================================================================
// 終了処理
//=============================================================================
void UninitRenderer(void)
{
	// オブジェクト解放
	//20250212追加
	if (g_PERenderTargetView)   g_PERenderTargetView->Release();
	if (g_PEShaderResourceView)  g_PEShaderResourceView->Release();

	if( g_ConstantBuffer )		g_ConstantBuffer->Release();
	if( g_MaterialBuffer )		g_MaterialBuffer->Release();
	if( g_VertexLayout )		g_VertexLayout->Release();
	if( g_VertexShader )		g_VertexShader->Release();
	if( g_PixelShader )			g_PixelShader->Release();


	if( g_ImmediateContext )	g_ImmediateContext->ClearState();
	if( g_RenderTargetView )	g_RenderTargetView->Release();
	if( g_SwapChain )			g_SwapChain->Release();
	if( g_ImmediateContext )	g_ImmediateContext->Release();
	if( g_D3DDevice )			g_D3DDevice->Release();

	// ▶ PATCH-BEGIN  UninitRenderer 深度对象释放
	if (g_DepthState_ZWriteOn)  g_DepthState_ZWriteOn->Release();
	if (g_DepthState_ZWriteOff) g_DepthState_ZWriteOff->Release();
	// ▶ PATCH-END

}


//=============================================================================
// バックバッファクリア
//=============================================================================
void Clear(void)
{
	// バックバッファクリア
	float ClearColor[4] = { 0.4f, 0.2f, 0.2f, 1.0f };
	g_ImmediateContext->ClearRenderTargetView( g_RenderTargetView, ClearColor );
	g_ImmediateContext->ClearDepthStencilView( g_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}


//=============================================================================
// プレゼント
//=============================================================================
void Present(void)
{
	g_SwapChain->Present( 0, 0 );
}

//
void SetATCEnable(bool Enable)
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (Enable)
	{
		g_ImmediateContext->OMSetBlendState(g_BlendStateATC, blendFactor, 0xffffffff);
	}
	else
	{
		g_ImmediateContext->OMSetBlendState(g_BlendState, blendFactor, 0xffffffff);
	}
}

ID3D11ShaderResourceView* GetPETexture()
{
	return g_PEShaderResourceView;
}

//レンダリングーターゲットをテクスチャへの切り替える関数
void BeginPE()
{
	//レンダーターゲットとｚバファをセット
	g_ImmediateContext->OMSetRenderTargets(1, &g_PERenderTargetView, g_DepthStencilView);

	//分かりやすいようにレンダーターゲットを緑でクリアしておく
	float ClearColor[4] = { 0.2f,0.5f,0.8f,1.0f };
	g_ImmediateContext->ClearRenderTargetView(g_PERenderTargetView,ClearColor);
	//zバッファのクリア
	g_ImmediateContext->ClearDepthStencilView(g_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Begin()
{
	//デフォルトのレンダーターゲットをバッファにセット
	g_ImmediateContext->OMSetRenderTargets(1, &g_RenderTargetView, g_DepthStencilView);


	//バックバッファクリア（デバッグ用に赤色でクリア）
	float ClearColor[4] = { 0.5f,0.0f,0.0f,1.0f };
	g_ImmediateContext->ClearRenderTargetView(g_RenderTargetView, ClearColor);
	g_ImmediateContext->ClearDepthStencilView(g_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

//
// 获取当前裁剪模式
D3D11_CULL_MODE GetCullingMode(void)
{
	return g_CurrentCullMode;
}

XMMATRIX GetWorldMatrix()
{
	return g_WorldMatrix;
}

void GetMaterial(MATERIAL* outMaterial)
{
	if (outMaterial) {
		// 返回当前材质
		*outMaterial = g_CurrentMaterial;
	}
}

// 添加到 renderer.cpp 中

bool GetDepthWriteEnable(void)
{
	return g_CurrentDepthWriteEnable;
}

//void SetDepthWriteEnable(bool Enable)
//{
//	// 创建新的深度状态
//	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
//	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
//	depthStencilDesc.DepthEnable = g_CurrentDepthState; // 保持当前深度测试状态
//
//	// 根据参数设置深度写入掩码
//	if (Enable)
//		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
//	else
//		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
//
//	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
//	depthStencilDesc.StencilEnable = FALSE;
//
//	// 创建深度状态
//	ID3D11DepthStencilState* depthState;
//	g_D3DDevice->CreateDepthStencilState(&depthStencilDesc, &depthState);
//
//	// 设置深度状态
//	g_ImmediateContext->OMSetDepthStencilState(depthState, 0);
//
//	// 记录当前状态
//	g_CurrentDepthWriteEnable = Enable;
//
//	// 释放临时创建的状态对象
//	depthState->Release();
//}
// ▶ PATCH-BEGIN  SetDepthWriteEnable
void SetDepthWriteEnable(bool Enable)
{
	if (Enable == g_CurrentDepthWriteEnable) return;   // 已是目标状态
	g_CurrentDepthWriteEnable = Enable;

	ID3D11DepthStencilState* use =
		g_CurrentDepthWriteEnable ? g_DepthState_ZWriteOn
		: g_DepthState_ZWriteOff;

	// 若同时深度测试被关，也无妨——WriteMask 不影响关闭状态
	g_ImmediateContext->OMSetDepthStencilState(use, 0);
}
// ▶ PATCH-END
