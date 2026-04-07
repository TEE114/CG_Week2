#pragma once
#include <Windows.h>

#include "ImGui/imconfig.h"
#include  "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
//#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imstb_rectpack.h"
#include "ImGui/imstb_truetype.h"
#include "ImGui/imstb_textedit.h"

bool InitImGui(HWND hwnd, ID3D11Device* g_pd3dDevice, ID3D11DeviceContext* g_pImmediateContext);
void CleanImGui();