#include "SkyDome.h"
#include "Camera.h"
#include <algorithm>

//全局变量
SkyDome g_SkyDome;
ID3D11Buffer* g_SkyVertexBuffer = NULL;
ID3D11Buffer* g_SkyIndexBuffer = NULL;
ID3D11VertexShader* g_SkyVS = NULL;
ID3D11PixelShader* g_SkyPS = NULL;
ID3D11InputLayout* g_SkyLayout = NULL;
ID3D11Buffer* g_SkyConstantBuffer = NULL;


//天空穹顶着色器常量缓存结构体
struct SKY_CONSTANT_BUFFER {

	XMFLOAT4X4  ViewProjection;
	XMFLOAT4	TopColor;
	XMFLOAT4	MiddleColor;
	XMFLOAT4	BottomColor;
	XMFLOAT4	SunPosition;
	float		Time;
	float      CloudDensity;
	float      CloudScale;
	float      CloudSharpness;
};

//天空穹顶顶点结构体
struct SKY_VERTEX {
	XMFLOAT3 Position;
};

//获取天空穹顶对象
SkyDome* GetSkyDome() {
	return &g_SkyDome;
}

//创建球顶点
void CreatSphereVertices(float radius, int slices, int stacks,
						SKY_VERTEX** vertices, UINT* vertexCount,
						UINT** indices, UINT* indexCount)
{
	*vertexCount = (slices + 1) * (stacks + 1);
	*vertices = new SKY_VERTEX[*vertexCount];

	//创建顶点
	for (int stack = 0; stack <= stacks; stack++) {
		float phi = XM_PI * stack / stacks;
		float y = radius * cosf(phi);
		float r = radius * sinf(phi);

		for (int slice = 0; slice <= slices; slice++) {
			float theta = 2.0f * XM_PI * slice / slices;
			float x = r * sinf(theta);
			float z = r * cosf(theta);

			int index = stack * (slices + 1) + slice;
			(*vertices)[index].Position = XMFLOAT3(x, y, z);
		}

	}

	//创建索引
	*indexCount = 6 * slices * stacks;
	*indices = new UINT[*indexCount];

	int index = 0;
	for(int stack = 0; stack < stacks; stack++)
	{
		for (int slice = 0; slice < slices; slice++)
		{
			int v1 = stack * (slices + 1) + slice;
			int v2 = stack * (slices + 1) + (slice + 1);
			int v3 = (stack + 1) * (slices + 1) + slice;
			int v4 = (stack + 1) * (slices + 1) + (slice + 1);

			// 第一个三角形
			(*indices)[index++] = v1;
			(*indices)[index++] = v3;
			(*indices)[index++] = v2;

			// 第二个三角形
			(*indices)[index++] = v2;
			(*indices)[index++] = v3;
			(*indices)[index++] = v4;
		}
	}
}

//初始化天空穹顶
void InitSkyDome()
{
	//设置天空穹顶属性
	g_SkyDome.Use = true;
	g_SkyDome.Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_SkyDome.Rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_SkyDome.Scale = XMFLOAT3(1000.0f, 1000.0f, 1000.0f);

	g_SkyDome.TopColor = XMFLOAT4(0.2f, 0.4f, 0.8f, 1.0f);
	g_SkyDome.MiddleColor = XMFLOAT4(0.5f, 0.65f, 0.9f, 1.0f);  // 中蓝色中间
	g_SkyDome.BottomColor = XMFLOAT4(0.8f, 0.85f, 1.0f, 1.0f);  // 浅蓝色底部
	g_SkyDome.SunPosition = XMFLOAT3(0.5f, -0.15f, 0.9f);        // 设置太阳位置
	g_SkyDome.Time = 0.0f;                                     // 初始时间

	//设置云参数
	g_SkyDome.CloudDensity = 0.45f;		// 云密度 (0.0-1.0)
	g_SkyDome.CloudScale = 0.9f;		// 云缩放 (值越小云越大)
	g_SkyDome.CloudSharpness = 0.6f;	// 云锐利度 (值越大，云边缘越清晰)

	//创建天空球几何体
	SKY_VERTEX* vertices = NULL;
	UINT vertexCount = 0;
	UINT* indices = NULL;
	UINT  indexCount = 0;

	CreatSphereVertices(1.0f, 50, 50,
		&vertices, &vertexCount, &indices, &indexCount);

	//创建顶点缓冲区
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(SKY_VERTEX) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory(&vertexData, sizeof(vertexData));
	vertexData.pSysMem = vertices;

	GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &g_SkyVertexBuffer);

	//创建索引缓冲区
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(UINT) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemory(&indexData, sizeof(indexData));
	indexData.pSysMem = indices;

	GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &g_SkyIndexBuffer);

	//释放临时内存
	delete[] vertices;
	delete[] indices;

	//创建常量缓冲区
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.ByteWidth = sizeof(SKY_CONSTANT_BUFFER);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	GetDevice()->CreateBuffer(&constantBufferDesc, NULL, &g_SkyConstantBuffer);

	//编译和创建着色器
	//顶点着色器
	ID3DBlob* pVSBlob = NULL;
	ID3DBlob* pErrorBlob = NULL;

	HRESULT hr = D3DCompileFromFile(L"skyshader.hlsl", NULL, NULL, "skyVS", "vs_4_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0, &pVSBlob, &pErrorBlob);

	if (FAILED(hr)) {
		if (pErrorBlob) {

			MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS Compile Error",
				MB_OK | MB_ICONERROR);
			pErrorBlob->Release();
		}
		return;
	}

	GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_SkyVS);
	
	//创建输入布局
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}
	};
	UINT numElements = ARRAYSIZE(layout);

	GetDevice()->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_SkyLayout);

	pVSBlob->Release();

	//像素着色器
	ID3DBlob* pPsBlob = NULL;

	hr = D3DCompileFromFile(L"skyshader.hlsl", NULL, NULL, "skyPS", "ps_4_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0, &pPsBlob, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS Compile Error", MB_OK | MB_ICONERROR);
			pErrorBlob->Release();
		}
		return;
	}

	GetDevice()->CreatePixelShader(pPsBlob->GetBufferPointer(), pPsBlob->GetBufferSize(), NULL, &g_SkyPS);

	pPsBlob->Release();
}

//释放天空资源
void UninitSkyDome()
{
	if (g_SkyConstantBuffer) { g_SkyConstantBuffer->Release(); g_SkyConstantBuffer = nullptr; }
	if (g_SkyLayout) { g_SkyLayout->Release(); g_SkyLayout = nullptr; }
	if (g_SkyPS) { g_SkyPS->Release(); g_SkyPS = nullptr; }
	if (g_SkyVS) { g_SkyVS->Release(); g_SkyVS = nullptr; }
	if (g_SkyIndexBuffer) { g_SkyIndexBuffer->Release(); g_SkyIndexBuffer = nullptr; }
	if (g_SkyVertexBuffer) { g_SkyVertexBuffer->Release(); g_SkyVertexBuffer = nullptr; }
}
//更新天空穹顶
void UpdateSkyDome(float deltaTime)
{
	//获取摄像机位置，是天空始终跟随摄像机
	Camera* pCamera = GetCamera();
	g_SkyDome.Position = pCamera->Position;

	// 更新时间参数 (用于动画效果)
	g_SkyDome.Time += deltaTime*0.5f;
	
	// 使用正弦函数创建平滑变化的参数，而不是直接使用取模运算
	float cycle1 = sinf(g_SkyDome.Time * 0.2f) * 0.5f + 0.5f; // 平滑的0-1循环
	float cycle2 = cosf(g_SkyDome.Time * 0.15f) * 0.5f + 0.5f; // 另一个不同频率的0-1循环

	// 云密度使用平滑变化
	g_SkyDome.CloudDensity = 0.5f + cycle1 * 0.1f; // 平滑变化的云密度

	// 云缩放也使用平滑变化
	g_SkyDome.CloudScale = 0.85f + cycle2 * 0.15f; // 平滑变化的云缩放

	// 云锐利度使用平滑变化
	g_SkyDome.CloudSharpness = 0.6f + sinf(g_SkyDome.Time * 0.1f) * 0.1f; // 平滑变化的锐利度

	float angle = g_SkyDome.Time * 0.1f;
	g_SkyDome.SunPosition.x = cosf(angle);

	g_SkyDome.SunPosition.y = -0.15f + sinf(angle * 0.15f) * 0.1f;//让太阳高度也稍微变化
	
	g_SkyDome.SunPosition.z = sinf(angle);

	// 归一化太阳方向向量
	float length = sqrtf(g_SkyDome.SunPosition.x * g_SkyDome.SunPosition.x +
					  	 g_SkyDome.SunPosition.y * g_SkyDome.SunPosition.y +
						 g_SkyDome.SunPosition.z * g_SkyDome.SunPosition.z);

	g_SkyDome.SunPosition.x /= length;
	g_SkyDome.SunPosition.y /= length;
	g_SkyDome.SunPosition.z /= length;
}


void DrawSkyDome()
{
	if (!g_SkyDome.Use) return;

	// 检查必要的资源是否已初始化
	if (!g_SkyVS || !g_SkyPS || !g_SkyLayout || !g_SkyVertexBuffer ||
		!g_SkyIndexBuffer || !g_SkyConstantBuffer) {
		OutputDebugStringA("错误：天空穹顶资源未初始化\n");
		return;
	}

	//保存当前渲染状态
	ID3D11DepthStencilState* oldDepthState = nullptr;
	UINT oldstencilRef = 0;
	GetDeviceContext()->OMGetDepthStencilState(&oldDepthState, &oldstencilRef);

	//创建并设置天空穹顶专用深度状态
	static ID3D11DepthStencilState* skyDepthState = nullptr;
	if (!skyDepthState) {
		D3D11_DEPTH_STENCIL_DESC depthDesc;
		ZeroMemory(&depthDesc, sizeof(depthDesc));
		depthDesc.DepthEnable = true;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		HRESULT hr = GetDevice()->CreateDepthStencilState(&depthDesc, &skyDepthState);
		if (FAILED(hr)) {
			OutputDebugStringA("错误：创建天空深度状态失败\n");
			if (oldDepthState) oldDepthState->Release();
			return;
		}
	}

	GetDeviceContext()->OMSetDepthStencilState(skyDepthState, 0);

	// 设置剔除模式
	D3D11_CULL_MODE oldCullMode = GetCullingMode();
	SetCulingMode(D3D11_CULL_FRONT);

	//保存当前渲染管线状态 - 修正版本
	ID3D11VertexShader* oldVS = nullptr;
	ID3D11PixelShader* oldPS = nullptr;
	ID3D11InputLayout* oldLayout = nullptr;
	ID3D11Buffer* oldVSConstantBuffers[1] = { nullptr };
	ID3D11Buffer* oldPSConstantBuffers[1] = { nullptr };
	ID3D11ClassInstance* oldVSClassInstances[256] = { nullptr };
	ID3D11ClassInstance* oldPSClassInstances[256] = { nullptr };
	UINT oldVSNumClassInstances = 256;
	UINT oldPSNumClassInstances = 256;

	GetDeviceContext()->VSGetShader(&oldVS, oldVSClassInstances, &oldVSNumClassInstances);
	GetDeviceContext()->PSGetShader(&oldPS, oldPSClassInstances, &oldPSNumClassInstances);
	GetDeviceContext()->IAGetInputLayout(&oldLayout);
	GetDeviceContext()->VSGetConstantBuffers(0, 1, oldVSConstantBuffers);
	GetDeviceContext()->PSGetConstantBuffers(0, 1, oldPSConstantBuffers);

	//设置天空着色器的输入布局
	GetDeviceContext()->VSSetShader(g_SkyVS, NULL, 0);
	GetDeviceContext()->PSSetShader(g_SkyPS, NULL, 0);
	GetDeviceContext()->IASetInputLayout(g_SkyLayout);

	//设置常量缓冲区数据
	SKY_CONSTANT_BUFFER skyConstants;

	// 获取摄像机的视图投影矩阵
	Camera* pCamera = GetCamera();
	if (!pCamera) {
		OutputDebugStringA("错误：无法获取相机\n");
		// 恢复状态
		GetDeviceContext()->OMSetDepthStencilState(oldDepthState, oldstencilRef);
		if (oldDepthState) oldDepthState->Release();
		return;
	}

	XMMATRIX view = GetViewMatrix();
	XMMATRIX projection = GetProjectMatrix();

	// 设置位置和缩放
	XMMATRIX scale = XMMatrixScaling(g_SkyDome.Scale.x, g_SkyDome.Scale.y, g_SkyDome.Scale.z);
	XMMATRIX translation = XMMatrixTranslation(g_SkyDome.Position.x, g_SkyDome.Position.y, g_SkyDome.Position.z);
	XMMATRIX world = scale * translation;

	// 合并世界、视图和投影矩阵
	XMMATRIX viewProjectionMatrix = world * view * projection;
	viewProjectionMatrix = XMMatrixTranspose(viewProjectionMatrix);

	XMStoreFloat4x4(&skyConstants.ViewProjection, viewProjectionMatrix);

	skyConstants.TopColor = g_SkyDome.TopColor;
	skyConstants.MiddleColor = g_SkyDome.MiddleColor;
	skyConstants.BottomColor = g_SkyDome.BottomColor;
	skyConstants.SunPosition = XMFLOAT4(g_SkyDome.SunPosition.x, g_SkyDome.SunPosition.y, g_SkyDome.SunPosition.z, 0.0f);
	skyConstants.Time = g_SkyDome.Time;
	skyConstants.CloudDensity = g_SkyDome.CloudDensity;
	skyConstants.CloudScale = g_SkyDome.CloudScale;
	skyConstants.CloudSharpness = g_SkyDome.CloudSharpness;

	GetDeviceContext()->UpdateSubresource(g_SkyConstantBuffer, 0, NULL, &skyConstants, 0, 0);
	GetDeviceContext()->VSSetConstantBuffers(0, 1, &g_SkyConstantBuffer);
	GetDeviceContext()->PSSetConstantBuffers(0, 1, &g_SkyConstantBuffer);

	//设置顶点缓冲区和索引缓冲区
	UINT stride = sizeof(SKY_VERTEX);
	UINT offset = 0;

	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_SkyVertexBuffer, &stride, &offset);
	GetDeviceContext()->IASetIndexBuffer(g_SkyIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//绘制天空球
	GetDeviceContext()->DrawIndexed(50 * 50 * 6, 0, 0);

	// 恢复先前的渲染状态
	GetDeviceContext()->VSSetShader(oldVS, NULL, 0);
	GetDeviceContext()->PSSetShader(oldPS, NULL, 0);
	GetDeviceContext()->IASetInputLayout(oldLayout);
	GetDeviceContext()->VSSetConstantBuffers(0, 1, oldVSConstantBuffers);
	GetDeviceContext()->PSSetConstantBuffers(0, 1, oldPSConstantBuffers);

	// 安全释放引用
	if (oldVS) oldVS->Release();
	if (oldPS) oldPS->Release();
	if (oldLayout) oldLayout->Release();
	if (oldVSConstantBuffers[0]) oldVSConstantBuffers[0]->Release();
	if (oldPSConstantBuffers[0]) oldPSConstantBuffers[0]->Release();

	// 释放类实例
	for (UINT i = 0; i < oldVSNumClassInstances; i++) {
		if (oldVSClassInstances[i]) oldVSClassInstances[i]->Release();
	}
	for (UINT i = 0; i < oldPSNumClassInstances; i++) {
		if (oldPSClassInstances[i]) oldPSClassInstances[i]->Release();
	}

	// 恢复深度状态
	GetDeviceContext()->OMSetDepthStencilState(oldDepthState, oldstencilRef);
	if (oldDepthState) oldDepthState->Release();

	// 恢复剔除模式
	SetCulingMode(oldCullMode);
}

