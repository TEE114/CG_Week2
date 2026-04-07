#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;

//顶点结构（和HLSL对应）
struct Vertex
{
	float x, y, z;
	float r, g, b, a;
};

__declspec(align(16))//常量缓冲区必须16字节对齐
struct MVP
{
	XMMATRIX model;
	XMMATRIX view;
	XMMATRIX proj;
};

extern float g_PosX, g_PosY, g_PosZ, g_RotX, g_RotY, g_RotZ;
extern float scaleX, scaleY, scaleZ;
extern XMFLOAT4 g_Color[8];
extern bool g_UseOrtho;

//全局d3d对象
extern ID3D11Device* g_pd3dDevice;//设备
extern ID3D11DeviceContext* g_pImmediateContext;//设备上下文
extern IDXGISwapChain* g_pSwapChain;//交换链
extern ID3D11RenderTargetView* g_pRenderTargetView;//渲染目标视图
extern ID3D11DepthStencilView* g_pDepthStencilView;//深度缓冲视图
extern ID3D11VertexShader* g_pVS;//顶点着色器
extern ID3D11PixelShader* g_pPS;//像素着色器
extern ID3D11InputLayout* g_pInputLayout;//输入布局
extern ID3D11Buffer* g_pVertexBuffer;//顶点缓冲区
extern ID3D11Buffer* g_pIndexBuffer;//索引缓冲区
extern ID3D11Buffer* g_pMVPBuffer;//常量缓冲区（MVP矩阵）

HRESULT CompilerShader(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
bool InitD3D(HWND hWnd);
void InitShader();
void InitVertexBuffer();
void Render();
void CleanD3D();
