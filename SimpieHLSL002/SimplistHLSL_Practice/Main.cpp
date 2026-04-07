#include "D3DCore.h"
#include "Auto.h"

static bool showWindow = true;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd,msg, wParam, lParam))return true;
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//=========================主函数=================================
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	//1.创建窗口
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"D3D11WindowClass";

	RegisterClassEx(&wc);
	HWND hWnd = CreateWindowEx(0, wc.lpszClassName, L"3D 四棱台", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, nullptr, nullptr, hInstance, nullptr);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	//2.初始化D3D
	if (!InitD3D(hWnd))
	{
		MessageBoxA(NULL, "D3D初始化失败", "错误", MB_OK);
		return 0;
	}
	//3.初始化Shader，关联HLSL
	InitShader();
	//4.初始化顶点
	InitVertexBuffer();

	if(!InitImGui(hWnd, g_pd3dDevice, g_pImmediateContext))
	{
		MessageBoxA(NULL, "ImGui 初始化失败！", "错误", MB_OK);
		return 0;
	}


	//5.消息循环+渲染
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else
		{
			ImGui_ImplWin32_NewFrame();  // Win32 新帧
			ImGui_ImplDX11_NewFrame();   // D3D11 新帧
			
			ImGui::NewFrame();
			ImGui::Checkbox("Demo Window", &showWindow);
			if (showWindow)
			{
				ImGui::Begin("MyGui", &showWindow, 0);
				ImGui::ShowDemoWindow();
				ImGui::Checkbox(u8"正交投影(Ortho)", &g_UseOrtho);
				ImGui::Separator();
				ImGui::Text("Vertex Colors"); // 在窗口中显示文本
				for (int i = 0; i < 8; i++)
				{
					char buf[32];
					sprintf_s(buf, _countof(buf), "Color %d", i);
					ImGui::ColorEdit4(buf, (float*)&g_Color[i]); // 显示颜色编辑器，允许修改顶点颜色
				}
				ImGui::Separator();
				ImGui::SliderFloat("PosX", &g_PosX, -10.0f, 10.0f);
				ImGui::SliderFloat("PosY", &g_PosY, -10.0f, 10.0f);
				ImGui::SliderFloat("PosZ", &g_PosZ, -10.0f, 10.0f);
				ImGui::Separator();
				ImGui::SliderFloat("RotX", &g_RotX, -3.14f, 3.14f);
				ImGui::SliderFloat("RotY", &g_RotY, -3.14f, 3.14f);
				ImGui::SliderFloat("RotZ", &g_RotZ, -3.14f, 3.14f);
				ImGui::Separator();
				ImGui::SliderFloat("ScaleX", &scaleX, -0.1f, 15.0f);
				ImGui::SliderFloat("ScaleY", &scaleY, -0.1f, 15.0f);
				ImGui::SliderFloat("ScaleZ", &scaleZ, -0.1f, 15.0f);
				ImGui::End();
			}
			Vertex vertices[] = {
				// 下底面
				{ -1.0f, -1.0f, -1.0f, g_Color[0].x,g_Color[0].y,g_Color[0].z,g_Color[0].w },
				{ 1.0f, -1.0f, -1.0f, g_Color[1].x,g_Color[1].y,g_Color[1].z,g_Color[1].w },
				{ 1.0f,  1.0f, -1.0f, g_Color[2].x,g_Color[2].y,g_Color[2].z,g_Color[2].w },
				{-1.0f,  1.0f, -1.0f, g_Color[3].x,g_Color[3].y,g_Color[3].z,g_Color[3].w },
				// 上底面
				{-0.5f, -0.5f, 1.0f, g_Color[4].x,g_Color[4].y,g_Color[4].z,g_Color[4].w },
				{ 0.5f, -0.5f, 1.0f, g_Color[5].x,g_Color[5].y,g_Color[5].z,g_Color[5].w },
				{ 0.5f,  0.5f, 1.0f, g_Color[6].x,g_Color[6].y,g_Color[6].z,g_Color[6].w },
				{-0.5f,  0.5f, 1.0f, g_Color[7].x,g_Color[7].y,g_Color[7].z,g_Color[7].w },
			};

			D3D11_MAPPED_SUBRESOURCE mapped;
			// 1. 映射：CPU获得可写指针，GPU暂时禁止访问
			g_pImmediateContext->Map(g_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
			// 2. 拷贝：CPU写入新顶点数据
			memcpy(mapped.pData, vertices, sizeof(vertices));
			// 3. 解除映射：GPU重新获得访问权
			g_pImmediateContext->Unmap(g_pVertexBuffer, 0);

			Render();
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			g_pSwapChain->Present(0, 0);
		}
	}
	
	CleanImGui();
	CleanD3D();
	UnregisterClass(wc.lpszClassName, wc.hInstance);
	return 0;
}