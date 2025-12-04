#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>


#include<Windows.h>
#include<dwmapi.h>
#include<d3d11.h>
#include <tchar.h>
#include<thread>

#include<imgui/imgui.h>
#include<imgui/imgui_impl_dx11.h>
#include<imgui/imgui_impl_win32.h>


#include"Vision.h" //Utility is already included in vision.h


static ID3D11Device*           g_pd3dDevice = nullptr;
static ID3D11DeviceContext*    g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*         g_pSwapChain = nullptr;
static bool                    g_SwapChainOccluded = false;
static UINT                    g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;


bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
//void ShowMainWindow();

//void debugWindow(std::string statusMessage);
//void configWindow();

ImGuiStyle SetupImGuiStyle();

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


Config cnf;
Vision vizu(cnf.areaRadius, cnf.fihKey, cnf.stopFihKey);
AppState state(cnf, vizu.statusMessage);

INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show) {
	
	cnf.loadConfig();
		
	// Make process DPI aware and obtain main monitor scale
	ImGui_ImplWin32_EnableDpiAwareness();
	float mainScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0,0 }, MONITOR_DEFAULTTOPRIMARY));
	//statements, windows, etc
	
	
	
	

	//window
	WNDCLASSEXW wc = { sizeof(wc), ACS_TRANSPARENT, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"_", nullptr };
	::RegisterClassExW(&wc);
	HWND hwnd = ::CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED, wc.lpszClassName, L"A", WS_POPUP, 0, 0, state.screenWidth, state.screenHeight, nullptr, nullptr, wc.hInstance, nullptr);

	//SetLayeredWindowAttributes(hwnd, 0, RGB(0, 0, 0), LWA_ALPHA);
	SetLayeredWindowAttributes(hwnd, 0, RGB(0, 0, 0), LWA_COLORKEY);
	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(hwnd, &margins);

	//init d3d
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);
	//setup imgui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF("E:/IT/repos/imguiTest/external/fonts/Impact.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	//style and scaling
	ImGuiStyle style = SetupImGuiStyle();
	style.ScaleAllSizes(mainScale);
	style.FontScaleDpi = mainScale;
	io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
	io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.


	 //When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	

	//main loop
	while (!state.shouldExit.load())
	{
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				state.shouldExit = true;

		}
		if (state.shouldExit.load())
			break;


		//handle window mini,azed or screen locked
		if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
		{
			::Sleep(10);
			continue;
		}
		g_SwapChainOccluded = false;

		//handle resize
		if (g_ResizeHeight != 0 && g_ResizeWidth != 0)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			g_ResizeWidth = g_ResizeHeight = 0;
			CreateRenderTarget();
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		/*if (state.confOpen)
			configWindow();

		if (state.showMainWindow)
			ShowMainWindow();

		if (state.showDemoWindow)
			ImGui::ShowDemoWindow(&state.showDemoWindow);

		if (state.debug)
			debugWindow(vizu.statusMessage);

		if (state.fihing.load()) {

			if (!fishingThread.joinable()) { 
				fishingThread = std::thread(&Vision::startCapture, &vizu, std::ref(state.fihing), std::ref(state.shouldExit));
			}

			if (GetAsyncKeyState(cnf.stopFih) & 0x8000) {
				state.fihing = false;
			}
		}
		else {
			
			if (fishingThread.joinable()) {
				fishingThread.join();
			}
		}*/
		state.manage();
		if (state.fihing.load()) {

			if (!state.fishingThread.joinable()) {
				state.fishingThread = std::thread(&Vision::startCapture, &vizu, std::ref(state.fihing), std::ref(state.shouldExit));
			}

			if (GetAsyncKeyState(cnf.stopFihKey) & 0x8000) {
				state.fihing = false;
			}
		}
		else {

			if (state.fishingThread.joinable()) {
				state.fishingThread.join();
			}
		}
	
		
			
			//rendering
			ImGui::Render();
			const float clear_color_with_alpha[4] = { 0, 0, 0, 0 };
			g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
			g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			//SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

			// Update and Render additional Platform Windows
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}

			
			//present
			HRESULT hr = g_pSwapChain->Present(1, 0);
			g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
		}
	cnf.saveConfig();
	state.fihing = false;
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		CleanupDeviceD3D();
		::DestroyWindow(hwnd);
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);

		return 0;
	};

bool CreateDeviceD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;

	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice ,&featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED)
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain)        { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice)        { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView)
	{
		g_mainRenderTargetView->Release();
		g_mainRenderTargetView = nullptr;
	}
}
//
//void ShowMainWindow()
//{
//	ImGuiIO& io = ImGui::GetIO(); (void)io;
//	ImGui::Begin("FihBot v0.0.0.0", 0, state.flazhoks);
//
//	if (ImGui::BeginTable("split", 3))
//	{
//		ImGui::TableNextColumn(); ImGui::Checkbox("demo window", &state.showDemoWindow);
//		ImGui::TableNextColumn(); ImGui::Checkbox("debug,", &state.debug);
//
//		ImGui::EndTable();
//	}
//	
//	if (ImGui::Button("config")) {
//		state.confOpen = true;
//	}
//
//	ImGui::SliderInt("areaRadius", &cnf.areaRadius, 200, 250);
//	if (ImGui::Button("Start"))
//	{
//		state.fihing.store(true);
//	}
//	if (ImGui::Button("Stop"))
//	{
//		state.fihing.store(false);
//	}
//
//	if (ImGui::Button("close app"))
//		state.shouldExit = true;
//
//	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.f / io.Framerate, io.Framerate);
//	ImGui::End();
//}
//
//void configWindow() {
//	ImGui::Begin("Configuration", &state.confOpen);
//	ImGui::Text("Start Fishing"); ImGui::SameLine();
//	char fihKeyname[64];
//	char stopFihKeyname[64];
//	GetKeyNameTextA(cnf.fihKey, fihKeyname, sizeof(fihKeyname));
//	GetKeyNameTextA(cnf.stopFih, stopFihKeyname, sizeof(stopFihKeyname));
//	if (ImGui::Button(fihKeyname)) {
//
//	}
//	ImGui::Text("Stop Fishing"); ImGui::SameLine();
//	if (ImGui::Button(stopFihKeyname)) { //реализации пока нет
//
//	}
//	ImGui::End();
//}
//
//void debugWindow(std::string statusMessage)
//{
//	ImGui::SetNextWindowSize(ImVec2(200, 200));
//	ImGui::Begin("debug");
//	ImGui::Text(statusMessage.c_str());
//	ImGui::End();
//}


// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
	switch (msg)
	{
	/*case WM_KEYDOWN:
		if (state.confOpen) {
			if (state.fihClicked) {
				GetKeyNameTextA(lParam, cnf.fihKeyname, sizeof(cnf.fihKeyname));
			}
			if (state.stopfihClicked) {
				GetKeyNameTextA(lParam, cnf.stopFihKeyname, sizeof(cnf.stopFihKeyname));
			}
		}
		
		return EXIT_SUCCESS;*/

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
		g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) //Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

ImGuiStyle SetupImGuiStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.5f;
	style.WindowPadding = ImVec2(11.7f, 6.0f);
	style.WindowRounding = 3.3f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(20.0f, 20.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(20.0f, 9.9f);
	style.FrameRounding = 0.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 14.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 4.0f;
	style.TabBorderSize = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.58431375f, 0.59607846f, 0.6156863f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0627451f, 0.06666667f, 0.08627451f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.043137256f, 0.047058824f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.043137256f, 0.047058824f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.10980392f, 0.11372549f, 0.13333334f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.10980392f, 0.11372549f, 0.13333334f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0627451f, 0.06666667f, 0.08627451f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.05882353f, 0.5294118f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.05882353f, 0.5294118f, 0.9764706f, 0.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.047058824f, 0.050980393f, 0.0627451f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.043137256f, 0.047058824f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.043137256f, 0.047058824f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.043137256f, 0.047058824f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.043137256f, 0.047058824f, 0.05882353f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.10980392f, 0.11372549f, 0.13333334f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.14509805f, 0.14901961f, 0.18431373f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4862745f, 0.4862745f, 0.4862745f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 1.0f, 1.0f, 0.22746783f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.81960785f, 0.81960785f, 0.81960785f, 0.3304721f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.22745098f, 0.44313726f, 0.75686276f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.20784314f, 0.47058824f, 0.8509804f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.05882353f, 0.5294118f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.3882353f, 0.3882353f, 0.3882353f, 0.62f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.13725491f, 0.4392157f, 0.8f, 0.78f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.13725491f, 0.4392157f, 0.8f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.34901962f, 0.34901962f, 0.34901962f, 0.17f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.95f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.0f, 0.4745098f, 1.0f, 0.931f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.20784314f, 0.20784314f, 0.20784314f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.91764706f, 0.9254902f, 0.93333334f, 0.9862f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.7411765f, 0.81960785f, 0.9137255f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.3882353f, 0.3882353f, 0.3882353f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.42745098f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392f, 0.69803923f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.44705883f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.7764706f, 0.8666667f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.5686275f, 0.5686275f, 0.6392157f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.6784314f, 0.6784314f, 0.7372549f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.29803923f, 0.29803923f, 0.29803923f, 0.09f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.95f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.8f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.69803923f, 0.69803923f, 0.69803923f, 0.7f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.2f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.35f);

	return style;
}
