#pragma once
#include<string>
#include"simpleini/SimpleIni.h"
#include<keyauth/auth.hpp>
#include<keyauth/skStr.h>
#include<imgui/imgui_impl_win32.h>

//#include"Vision.h" 
//#include"Utility.h"
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


//std::string name = skCrypt("Albion FihBot").decrypt(); // App name
//std::string ownerid = skCrypt("avFNTfvMb4").decrypt(); // Account ID
//std::string version = skCrypt("1.1").decrypt(); // Application version. Used for automatic downloads see video here 
//std::string url = skCrypt("https://keyauth.win/api/1.3/").decrypt(); // change if using KeyAuth custom domains feature
//std::string path = skCrypt("").decrypt(); // (OPTIONAL) see tutorial here 

//KeyAuth::api KeyAuthApp(name, ownerid, version, url, path);

std::atomic<bool> isLogged = true;
//char licenseBuffer[64] = "";

//const char* keyFilename = "key.ini";
//std::string username, password, key, TfaCode; // keep this before the auto-login with saved file.
// because if you don't and the user has 2FA on, then they won't be asked for 2FA code and can't login.

//CSimpleIniA iniKey;

//void sessionStatus() {
//
//	//KeyAuthApp.check(); // do NOT specify true usually, it is slower and will get you blocked from API
//	//if (!KeyAuthApp.response.success) {
//	//	dffessageBoxA(NULL, KeyAuthApp.response.message.c_str(), "Error", MB_OK | MB_ICONERROR);
//	//	exit(1);
//	//}
//	
//	if (KeyAuthApp.response.isPaid) {
//		while (isLogged.load()) {
//			 
//			KeyAuthApp.check();
//			if (!KeyAuthApp.response.success) {
//				MessageBoxA(NULL, KeyAuthApp.response.message.c_str(), skCrypt("Error"), MB_OK | MB_ICONERROR);
//				Sleep(2500);
//				exit(1);
//			}
//			std::this_thread::sleep_for(std::chrono::seconds(20)); // this MUST be included or else you get blocked from API
//		}
//	}
//	
//}

//void checkAuthenticated(std::string ownerid) {
//	while (isLogged.load()) {
//		if (GlobalFindAtomA(ownerid.c_str()) == 0) {
//			//MessageBoxA(NULL, "Cant find owner id", "Error", MB_OK | MB_ICONERROR);
//			Sleep(2500);
//			exit(1);
//		}
//		std::this_thread::sleep_for(std::chrono::seconds(5)); // thread interval
//	}
//}

//void keyAuthInit() {
//	KeyAuthApp.init();
//	if (!KeyAuthApp.response.success)
//	{
//
//		MessageBoxA(NULL, KeyAuthApp.response.message.c_str(), skCrypt("Error"), MB_OK | MB_ICONERROR);
//		Sleep(2500);
//		exit(1);
//	}
//	
//}

//bool keySucces()
//{
//	
//	
//	SI_Error rc = iniKey.LoadFile(keyFilename);
//	if (rc < 0)
//	{
//		key = skCrypt("You can buy key TG @Sektor223");
//		iniKey.SetValue(skCrypt("license"), skCrypt("key"), key.c_str());
//		iniKey.SaveFile(keyFilename);
//		
//		KeyAuthApp.license(key, "");
//
//		if (KeyAuthApp.response.message.empty()) {
//			MessageBoxA(NULL, skCrypt("KeyAuth does not respond"), skCrypt("Error"), MB_OK | MB_ICONERROR);
//			exit(11);
//		}
//
//
//	}
//	else
//	{
//
//		key = iniKey.GetValue(skCrypt("license"), skCrypt("key"));
//		KeyAuthApp.license(key);
//
//		if (KeyAuthApp.response.message.empty()) {
//			MessageBoxA(NULL, skCrypt("KeyAuth does not respond"), skCrypt("Error"), MB_OK | MB_ICONERROR);
//			exit(11);
//		}
//
//		
//
//
//	}
//	return KeyAuthApp.response.success;
//}

////bool tryLogin(std::string license) {
//	
//	//KeyAuthApp.license(license);
//	if (KeyAuthApp.response.success) {
//		iniKey.SetValue(skCrypt("license"), skCrypt("key"), license.c_str());
//		iniKey.SaveFile(keyFilename);
//	}
//	return KeyAuthApp.response.success;
//}

//void AuthorizationWindow() {
//	ImGui::SetNextWindowSize(ImVec2(400, 150));
//	if (ImGui::Begin(skCrypt("Authorization"), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings)) {
//		
//		
//		ImGui::InputText(skCrypt("License Key"), licenseBuffer, IM_ARRAYSIZE(licenseBuffer), ImGuiInputTextFlags_Password);
//
//		if (ImGui::Button(skCrypt("Login"), ImVec2(100, 40))) {
//			//std::string licenseString = licenseBuffer;
//			isLogged = tryLogin(licenseBuffer);
//			
//		}
//		ImGui::SameLine();
//		if (ImGui::Button(skCrypt("Close"), ImVec2(100, 40))) {
//			state.shouldExit = true;
//		}
//		if (!isLogged.load()) {
//			
//			ImGui::Text(KeyAuthApp.response.message.c_str());
//			
//		}
//		ImGui::End();
//	}
//}

struct futureLogin { //this is shit
	
	futureLogin(){

	}
	
	std::string tm_to_readable_time(tm ctx) {
		char buffer[80];

		strftime(buffer, sizeof(buffer), "%d/%m/%y %H:%M:%S", &ctx);

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

	std::string getTitle() {
		return  "FihBot v1.1";//version + " " + Bykvi + expire;
	}
	
private:
	
	//std::string expire = tm_to_readable_time(timet_to_tm(string_to_timet(KeyAuthApp.user_data.subscriptions[0].expiry)));
	//std::string Bykvi = skCrypt("Active Until: ").decrypt();

};

