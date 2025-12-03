#pragma once
#include <imgui/imgui_impl_win32.h>
#include <WinUser.h>
//#include<chrono>
#include"inicpp.h"
//namespace binds 
//{
//	inline int fihKey = VK_NUMPAD5;
//	inline int stopFih = VK_NUMPAD0;
//	//inline int inventory = VK_I
//}
class Config {
private:
	std::string filename = "fih_Settings.ini";
public:
	bool firstLaunch = true;
	int areaRadius;
	int fihKey;
	int stopFih;
	void saveConfig() {
		ini::IniFile settings;
		settings["launch"]["first launch"] = firstLaunch;
		settings["Values"]["area radius"] = areaRadius;
		settings["Keys"]["fih key"] = fihKey;
		settings["Keys"]["stop fih"] = stopFih;
		settings.save(filename);
	}
	void createDefaultCnf() {
		areaRadius = 200;
		firstLaunch = false;
		fihKey = VK_NUMPAD5;
		stopFih = VK_ESCAPE;
	    

		saveConfig();
	}
	void loadConfig() { 
		ini::IniFile settings;
		settings.load(filename);
		firstLaunch = settings["launch"]["first launch"].as<bool>(); 
		areaRadius =  settings["Values"]["area radius"].as<int>();
		fihKey =      settings["Keys"]["fih key"].as<int>();
		stopFih =     settings["Keys"]["stop fih"].as<int>();
	}

};


struct AppState {
public:
	//numbers
	
	const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	//bool
	bool showDemoWindow = false;
	bool showAnotherWindow = false;
	bool showMainWindow = true;
	bool showHint = true;
	std::atomic<bool> shouldExit = false;
	std::atomic<bool> fihing = false;
	bool debug = false;
	//auto start = std::chrono::high_resolution_clock::now();
	//window flags
	ImGuiWindowFlags flazhoks = 
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;
	
	AppState() {
		
	}

};


