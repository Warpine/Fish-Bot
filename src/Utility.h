#pragma once
#include <imgui/imgui_impl_win32.h>

#include"Config.h"
#include"Vision.h"


//class Config {
//private:
//	const char* filename = "fih_Settings.ini";
//	CSimpleIniA ini;
//	bool fihButton = false;
//	bool stopButton = false;
//	bool inventoryButton = false;
//public:
//	//bool firstLaunch = true;
//	int areaRadius;
//	int fihKey;
//	int stopFihKey;
//	int inventoryKey;
//	void saveConfig() {
//		
//		
//		ini.SetLongValue("Keys", "fihKey", fihKey);
//		ini.SetLongValue("Keys", "stopFihKey", stopFihKey);
//		ini.SetLongValue("Keys", "inventoryKey", inventoryKey);
//		ini.SetLongValue("Values", "areaRadius", areaRadius);
//		SI_Error rc = ini.SaveFile(filename);
//	}
//	
//	void loadConfig() { 
//		SI_Error rc = ini.LoadFile(filename);
//		if (rc < 0) {
//			createDefaultCnf();
//			ini.LoadFile(filename);
//		}
//
//		areaRadius = ini.GetLongValue("Values", "areaRadius");
//		fihKey = ini.GetLongValue("Keys", "fihKey");
//		stopFihKey = ini.GetLongValue("Keys", "stopFihKey");
//		inventoryKey = ini.GetLongValue("Keys", "inventoryKey");
//	}
//	
//	
//
//	void window(bool& confOpen) {
//
//		ImGui::Begin("Configuration", &confOpen);
//
//		ImGui::Text("Start Fishing"); ImGui::SameLine(); Hotkey(&fihKey, fihButton);
//		
//		ImGui::Text("Stop Fishing"); ImGui::SameLine(); Hotkey(&stopFihKey, stopButton);
//		
//		ImGui::Text("Inventory "); ImGui::SameLine(); Hotkey(&inventoryKey, inventoryButton);
//
//		ImGui::End();
//	}
//
//private:
//	void createDefaultCnf() {
//		areaRadius = 200;
//		fihKey = VK_NUMPAD5;
//		stopFihKey = VK_NUMPAD0;
//		inventoryKey = 'I';
//
//		saveConfig();
//	}
//	void Hotkey(int* k, bool& button, const ImVec2& size_arg = ImVec2(0, 0))
//	{
//		ImGui::PushID("...");
//
//
//		if (!button) {
//			if (ImGui::Button(KeyNames[*k], size_arg))
//				button = true;
//		}
//		else
//		{
//			ImGui::Button("...", size_arg);
//
//			for (auto& Key : KeyCodes)
//			{
//				if (GetAsyncKeyState(Key) & 0x8000)
//				{
//					*k = Key;
//					button = false;
//					break;
//				}
//			}
//		}
//		ImGui::PopID();
//	}
//};


struct AppState {
public:
	
	
	std::atomic<bool> shouldExit = false;
	std::atomic<bool> fihing = false;
	std::thread fishingThread;
	
	
private:
	Vision& vision;
	Config& config;

	
	//bool showAnotherWindow = false;
	bool showMainWin = true;
	bool showHint = true;
	bool debug = false;
	bool bindsOpen = false;
	bool settingsOpen = false;
	
	
	
public:
	AppState(Config& cnf, Vision& vision) : config(cnf), vision(vision) {}

	void manage() {
		if (bindsOpen && showMainWin) {
			config.window(bindsOpen);
		}
		if (settingsOpen && showMainWin) {
			config.settingsWindow(settingsOpen);
		}

		if (showMainWin)
			ShowMainWindow();


		if (debug)
			vision.debugWindow();
		
		
		
	}
private:
	void ShowMainWindow()
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::Begin("FihBot v0.0.0.0", 0, config.flazhoks);
		
		if (ImGui::Button("Start", ImVec2(config.ButtonX, config.ButtonY*2)))
		{
			fihing.store(true);
		}
		ImGui::SameLine(config.ButtonX*3);



		ImGui::BeginGroup();
		if (ImGui::Button("Binds", config.standartButton)) {

			if (!bindsOpen) { bindsOpen = true; }

			else { bindsOpen = false; }

		}
		
		if (ImGui::Button("Settings", config.standartButton)) {

			if (!settingsOpen) { settingsOpen = true; }

			else { settingsOpen = false; }
		}
		if (ImGui::Button("Close", config.standartButton))
			shouldExit = true;
		ImGui::EndGroup();
		

		

		
		ImGui::Checkbox("debug", &debug);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.f / io.Framerate, io.Framerate);
		ImGui::End();
	}

	

};


