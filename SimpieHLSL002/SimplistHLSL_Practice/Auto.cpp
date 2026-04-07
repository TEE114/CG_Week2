#include "Auto.h"


bool InitImGui(HWND hwnd, ID3D11Device* g_pd3dDevice, ID3D11DeviceContext* g_pImmediateContext)
{
	// Make process DPI aware and obtain main monitor scale
	ImGui_ImplWin32_EnableDpiAwareness();
	float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

	// 初始化 Dear ImGui 上下文（核心第一步）
	IMGUI_CHECKVERSION();           // 1. 校验 ImGui 版本兼容性
	ImGui::CreateContext();         // 2. 创建 ImGui 全局上下文（必须调用）
	ImGuiIO& io = ImGui::GetIO();   // 3. 获取 IO 配置对象（控制输入输出/全局配置）
	(void)io;                       // 4. 避免“未使用变量”编译警告（无实际功能）

	// 启用基础交互控制
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // 启用键盘导航控制
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // 启用游戏手柄导航控制

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pImmediateContext);
	return true;
}

void CleanImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}






