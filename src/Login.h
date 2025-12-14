#pragma once
#include<string>
#include"simpleini/SimpleIni.h"
#include<keyauth/auth.hpp>
#include<keyauth/skStr.h>
#include<imgui/imgui_impl_win32.h>

//#include"Vision.h" 
#include"Utility.h"
//#include"Config.h"

//globals header?
static ID3D11Device*           g_pd3dDevice = nullptr;
static ID3D11DeviceContext*    g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*         g_pSwapChain = nullptr;
static bool                    g_SwapChainOccluded = false;
static UINT                    g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

Config config;
Vision vizu(config);
AppState state(config, vizu);


std::string name = skCrypt("Albion FihBot").decrypt(); // App name
std::string ownerid = skCrypt("avFNTfvMb4").decrypt(); // Account ID
std::string version = skCrypt("1.0").decrypt(); // Application version. Used for automatic downloads see video here https://www.youtube.com/watch?v=kW195PLCBKs
std::string url = skCrypt("https://keyauth.win/api/1.3/").decrypt(); // change if using KeyAuth custom domains feature
std::string path = skCrypt("").decrypt(); // (OPTIONAL) see tutorial here https://www.youtube.com/watch?v=I9rxt821gMk&t=1s

KeyAuth::api KeyAuthApp(name, ownerid, version, url, path);
std::atomic<bool> isLogged = false;
char licenseBuffer[64] = "";

const char* keyFilename = "key.ini";
std::string username, password, key, TfaCode; // keep this before the auto-login with saved file.
// because if you don't and the user has 2FA on, then they won't be asked for 2FA code and can't login.

CSimpleIniA iniKey;

void sessionStatus() {
	KeyAuthApp.check(); // do NOT specify true usually, it is slower and will get you blocked from API
	if (!KeyAuthApp.response.success) {
		exit(0);
	}

	if (KeyAuthApp.response.isPaid) {
		while (isLogged.load()) {
			std::this_thread::sleep_for(std::chrono::seconds(20)); // this MUST be included or else you get blocked from API
			KeyAuthApp.check();
			if (!KeyAuthApp.response.success) {
				exit(0);
			}
		}
	}
}

void checkAuthenticated(std::string ownerid) {
	while (isLogged.load()) {
		if (GlobalFindAtomA(ownerid.c_str()) == 0) {
			exit(13);
		}
		std::this_thread::sleep_for(std::chrono::seconds(5)); // thread interval
	}
}

void keyAuthInit() {
	KeyAuthApp.init();
	if (!KeyAuthApp.response.success)
	{

		MessageBoxA(NULL, "KeyAuth cant initialize", "Error", MB_OK | MB_ICONERROR);
		Sleep(1500);
		exit(1);
	}
	
}

bool keySucces()
{
	
	//ini.SetUnicode();
	SI_Error rc = iniKey.LoadFile(keyFilename);
	if (rc < 0)
	{
		key = "You can buy key";
		iniKey.SetValue("license", "key", key.c_str());
		iniKey.SaveFile(keyFilename);
		
		KeyAuthApp.license(key, "");

		if (KeyAuthApp.response.message.empty()) {
			MessageBoxA(NULL, "KeyAuth does not respond", "Error", MB_OK | MB_ICONERROR);
			exit(11);
		}


	}
	else
	{

		key = iniKey.GetValue("license", "key");
		KeyAuthApp.license(key);

		if (KeyAuthApp.response.message.empty()) {
			MessageBoxA(NULL, "KeyAuth does not respond", "Error", MB_OK | MB_ICONERROR);
			exit(11);
		}

		


	}
	return KeyAuthApp.response.success;
}

bool tryLogin(std::string license) {
	
	KeyAuthApp.license(license);
	if (KeyAuthApp.response.success) {
		iniKey.SetValue("license", "key", license.c_str());
		iniKey.SaveFile(keyFilename);
	}
	return KeyAuthApp.response.success;
}

void AuthorizationWindow() {
	ImGui::SetNextWindowSize(ImVec2(400, 150));
	if (ImGui::Begin("Authorization", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings)) {
		
		
		ImGui::InputText("License Key", licenseBuffer, IM_ARRAYSIZE(licenseBuffer), ImGuiInputTextFlags_Password);

		if (ImGui::Button("Login", ImVec2(100, 40))) {
			//std::string licenseString = licenseBuffer;
			isLogged = tryLogin(licenseBuffer);
			
		}
		ImGui::SameLine();
		if (ImGui::Button("Close", ImVec2(100, 40))) {
			state.shouldExit = true;
		}
		if (!isLogged.load()) {
			if (KeyAuthApp.response.message != "Initialized" && KeyAuthApp.response.message != "Session is validated.") {
				ImGui::Text(KeyAuthApp.response.message.c_str());
			}
		}
		ImGui::End();
	}
}

std::string tm_to_readable_time(tm ctx) {
	char buffer[80];

	strftime(buffer, sizeof(buffer), "%a %m/%d/%y %H:%M:%S %Z", &ctx);

	return std::string(buffer);
}

static std::time_t string_to_timet(std::string timestamp) {
	auto cv = strtol(timestamp.c_str(), NULL, 10); // long

	return (time_t)cv;
}

static std::tm timet_to_tm(time_t timestamp) {
	std::tm context;

	localtime_s(&context, &timestamp);

	return context;
}