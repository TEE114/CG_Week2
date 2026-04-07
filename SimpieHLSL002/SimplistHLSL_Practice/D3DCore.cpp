#include "D3DCore.h"

ID3D11Device* g_pd3dDevice = nullptr;//设备
ID3D11DeviceContext* g_pImmediateContext = nullptr;//设备上下文
IDXGISwapChain* g_pSwapChain = nullptr;//交换链
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;//渲染目标视图
ID3D11DepthStencilView* g_pDepthStencilView = nullptr;//深度缓冲视图
ID3D11VertexShader* g_pVS = nullptr;//顶点着色器
ID3D11PixelShader* g_pPS = nullptr;//像素着色器
ID3D11InputLayout* g_pInputLayout = nullptr;//输入布局
ID3D11Buffer* g_pVertexBuffer = nullptr;//顶点缓冲区
ID3D11Buffer* g_pIndexBuffer = nullptr;//索引缓冲区
ID3D11Buffer* g_pMVPBuffer = nullptr;//常量缓冲区（MVP矩阵）

float g_PosX=0, g_PosY=0, g_PosZ=-5, g_RotX=0, g_RotY=0, g_RotZ=0;
float scaleX = 1.0f, scaleY = 1.0f, scaleZ = 1.0f;
XMFLOAT4 g_Color[8] = {
{ 1.0f, 0.0f, 0.0f, 1.0f },
{ 0.0f, 1.0f, 0.0f, 1.0f },
{ 0.0f, 0.0f, 1.0f, 1.0f },
{ 1.0f, 1.0f, 0.0f, 1.0f },
{ 1.0f, 0.0f, 1.0f, 1.0f },
{ 0.0f, 1.0f, 1.0f, 1.0f },
{ 0.5f, 0.5f, 0.5f, 1.0f },
{ 1.0f, 1.0f, 1.0f, 1.0f },
};
bool g_UseOrtho = false; // false=透视, true=正交

//编译Shader
HRESULT CompilerShader(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	ID3DBlob* pErrorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 0, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr) && pErrorBlob)
	{
		OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
	}
	return hr;
}

//初始化D3D
bool InitD3D(HWND hWnd)
{
	HRESULT hr;

	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, nullptr, &g_pImmediateContext);
	if (FAILED(hr))return false;

	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))return false;


	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
	if (FAILED(hr))return false;
	pBackBuffer->Release();


	// ====================== 深度缓冲（3D必须） ======================
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = 800;
	depthDesc.Height = 600;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* pDepthBuffer = nullptr;
	hr = g_pd3dDevice->CreateTexture2D(&depthDesc, nullptr, &pDepthBuffer);
	if (FAILED(hr))return false;

	hr = g_pd3dDevice->CreateDepthStencilView(pDepthBuffer, nullptr, &g_pDepthStencilView);
	if (FAILED(hr))return false;
	pDepthBuffer->Release();

	//g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);2D渲染不需要深度缓冲，3D渲染必须绑定深度缓冲，否则无法正确处理遮挡关系
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

	D3D11_VIEWPORT vp = {};
	vp.Width = 800;
	vp.Height = 600;
	vp.MaxDepth = 1.0f;
	g_pImmediateContext->RSSetViewports(1, &vp);

	return true;
}

//初始化Shader（关联HLSL）
void InitShader()
{
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;

	// 1. 编译顶点着色器

	HRESULT hr = CompilerShader(L"VertexShader.hlsl", "main", "vs_5_0", &pVSBlob);
	if (pVSBlob == nullptr)
	{
		MessageBoxA(NULL, "pVSBlob 是空指针！\n检查文件路径是否正确", "错误", MB_OK);

		return;
	}
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "顶点着色器编译失败！\n检查 VertexShader.hlsl 是否存在、语法是否正确", "错误", MB_OK);
		return;
	}


	// 2. 编译像素着色器
	hr = CompilerShader(L"./PixelShader.hlsl", "main", "ps_5_0", &pPSBlob);
	if (pPSBlob == nullptr)
	{
		MessageBoxA(NULL, "pPSBlob 是空指针！\n检查文件路径是否正确", "错误", MB_OK);

		return;
	}
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "像素着色器编译失败！\n检查 PixelShader.hlsl 是否存在、语法是否正确", "错误", MB_OK);
		return;
	}


	// 3. 创建着色器
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVS);
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "创建顶点着色器失败！", "错误", MB_OK);
		return;
	}

	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPS);
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "创建像素着色器失败！", "错误", MB_OK);
		return;
	}

	// 4. 输入布局
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	hr = g_pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &g_pInputLayout);
	if (FAILED(hr))
	{
		MessageBoxA(NULL, "创建输入布局失败！", "错误", MB_OK);
		return;
	}

	// MVP 常量缓冲区
	D3D11_BUFFER_DESC mvpDesc = {};
	mvpDesc.ByteWidth = sizeof(MVP);
	mvpDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mvpDesc.Usage = D3D11_USAGE_DEFAULT;
	mvpDesc.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&mvpDesc, nullptr, &g_pMVPBuffer);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateBuffer 失败", "错误", MB_OK);
		return;
	}

	pVSBlob->Release();
	pPSBlob->Release();
}

//初始化三角形顶点
void InitVertexBuffer()
{
	HRESULT hr;
	Vertex vertices[] = {
		// 下底面 (z = -1)
	{ -1.0f, -1.0f, -1.0f, g_Color[0].x,g_Color[0].y,g_Color[0].z,g_Color[0].w}, // v0: 左下
	{ 1.0f, -1.0f, -1.0f, g_Color[1].x,g_Color[1].y,g_Color[1].z,g_Color[1].w}, // v1: 右下
	{ 1.0f,  1.0f, -1.0f, g_Color[2].x,g_Color[2].y,g_Color[2].z,g_Color[2].w}, // v2: 右上
	{-1.0f,  1.0f, -1.0f, g_Color[3].x,g_Color[3].y,g_Color[3].z,g_Color[3].w}, // v3: 左上
	// 上底面 (z = 1)
	{-0.5f, -0.5f,  1.0f, g_Color[4].x,g_Color[4].y,g_Color[4].z,g_Color[4].w}, // v4: 上左下
	{ 0.5f, -0.5f,  1.0f, g_Color[5].x,g_Color[5].y,g_Color[5].z,g_Color[5].w}, // v5: 上右下
	{ 0.5f,  0.5f,  1.0f, g_Color[6].x,g_Color[6].y,g_Color[6].z,g_Color[6].w}, // v6: 上右上
	{-0.5f,  0.5f,  1.0f, g_Color[7].x,g_Color[7].y,g_Color[7].z,g_Color[7].w}, // v7: 上左上
	};

	//顶点缓冲区
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(vertices);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//提供初始数据
	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices; InitData.SysMemPitch = 0; InitData.SysMemSlicePitch = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateBuffer:VB 失败", "错误", MB_OK);
		return;
	}

	// 索引：6个三角形 = 18个索引
	WORD indices[] = {
		// 下底面 (v0, v1, v2, v3)
	0, 1, 2,  0, 2, 3,
	// 上底面 (v4, v5, v6, v7)
	4, 6, 5,  4, 7, 6,
	// 前面 (v1, v2, v6, v5)
	1, 2, 6,  1, 6, 5,
	// 后面 (v0, v3, v7, v4)
	0, 3, 7,  0, 7, 4,
	// 左面 (v0, v1, v5, v4)
	0, 1, 5,  0, 5, 4,
	// 右面 (v2, v3, v7, v6)
	2, 3, 7,  2, 7, 6,
	};

	//索引缓冲区
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(indices);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	//提供初始数据
	D3D11_SUBRESOURCE_DATA idata = {};
	idata.pSysMem = indices; idata.SysMemPitch = 0; idata.SysMemSlicePitch = 0;
	hr = g_pd3dDevice->CreateBuffer(&ibd, &idata, &g_pIndexBuffer);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateBuffer:IB 失败", "错误", MB_OK);
		return;
	}
}
//渲染
void Render()
{
	//g_pImmediateContext->RSSetState(nullptr); // 关闭背面剔除 → 强制显示所有面
	//g_pImmediateContext->OMSetDepthStencilState(NULL, 0); // 关闭深度测试！
	// 关闭背面剔除 —— 所有三角形都渲染，不管顺序
	D3D11_RASTERIZER_DESC rsDesc = {};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_NONE; // 不剔除任何面

	ID3D11RasterizerState* pRS = nullptr;
	g_pd3dDevice->CreateRasterizerState(&rsDesc, &pRS);
	g_pImmediateContext->RSSetState(pRS);
	pRS->Release();

	// 开启深度测试，3D 遮挡才正确
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	ID3D11DepthStencilState* pDS = nullptr;
	g_pd3dDevice->CreateDepthStencilState(&dsDesc, &pDS);
	g_pImmediateContext->OMSetDepthStencilState(pDS, 0);
	pDS->Release();

	float clearColor[] = { 0.12f,0.12f,0.12f,1.0f };

	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, clearColor);//清除渲染目标
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);//清除深度缓冲

	// ====================== MVP 矩阵（真正3D透视） ======================
	MVP mvp;
	//mvp.model = XMMatrixIdentity();// 模型矩阵（不变）

	mvp.model = XMMatrixScaling(scaleX, scaleY, scaleZ) * XMMatrixRotationX(g_RotX) * XMMatrixRotationY(g_RotY) * XMMatrixRotationZ(g_RotZ) * XMMatrixTranslation(g_PosX, g_PosY, g_PosZ);//世界矩阵：放缩+平移+旋转
	//mvp.model = matWorld * mvp.model;// 模型矩阵 = 世界矩阵

	mvp.view = XMMatrixLookAtLH
	(
		XMVectorSet(0.0f, 0.0f, 5.0f, 0),// 观察点
		XMVectorSet(0, 0.2, 0, 0),// 目标点
		XMVectorSet(0, 10, 0, 0)
	);// 视图矩阵
	if(g_UseOrtho)
		mvp.proj = XMMatrixOrthographicLH(15, 15, 0.1f, 100);// 正交投影矩阵
	else
	mvp.proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, 800.0f / 600, 0.1f, 100);// 45°透视投影矩阵

	mvp.model = XMMatrixTranspose(mvp.model);
	mvp.view = XMMatrixTranspose(mvp.view);
	mvp.proj = XMMatrixTranspose(mvp.proj);

	// 直接上传整个结构体
	g_pImmediateContext->UpdateSubresource(g_pMVPBuffer, 0, NULL, &mvp, 0, 0);// 更新常量缓冲区数据:把数据从 CPU 复制到 GPU 内存（只是搬运，GPU 还不知道用在哪）

	g_pImmediateContext->IASetInputLayout(g_pInputLayout);
	g_pImmediateContext->VSSetShader(g_pVS, nullptr, 0);
	g_pImmediateContext->PSSetShader(g_pPS, nullptr, 0);
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pMVPBuffer);// 绑定常量缓冲区到顶点着色器

	//绑定顶点缓冲区
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);// 绑定顶点缓冲
	g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);// 绑定索引缓冲 
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);// 设置拓扑为三角形列表
	//g_pImmediateContext->Draw(3, 0);
	g_pImmediateContext->DrawIndexed(36, 0, 0);
	
}
void CleanD3D()
{
	//释放资源
	g_pMVPBuffer->Release();
	g_pVertexBuffer->Release();
	g_pIndexBuffer->Release();
	g_pInputLayout->Release();
	g_pVS->Release();
	g_pPS->Release();
	g_pDepthStencilView->Release();
	g_pRenderTargetView->Release();
	g_pSwapChain->Release();
	g_pImmediateContext->Release();
	g_pd3dDevice->Release();
}