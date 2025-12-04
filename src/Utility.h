#pragma once
#include <imgui/imgui_impl_win32.h>
#include <WinUser.h>
#include"buttons/Arrays.hpp"

#include"simpleini/SimpleIni.h"

class Vision;
struct AppState;
//class Config;

class Config {
private:
	const char* filename = "fih_Settings.ini";
	CSimpleIniA ini;
	bool fihButton = false;
	bool stopButton = false;
	bool inventoryButton = false;
public:
	//bool firstLaunch = true;
	int areaRadius;
	int fihKey;
	int stopFihKey;
	int inventoryKey;
	void saveConfig() {
		
		
		ini.SetLongValue("Keys", "fihKey", fihKey);
		ini.SetLongValue("Keys", "stopFihKey", stopFihKey);
		ini.SetLongValue("Keys", "inventoryKey", inventoryKey);
		ini.SetLongValue("Values", "areaRadius", areaRadius);
		SI_Error rc = ini.SaveFile(filename);
	}
	void createDefaultCnf() {
		areaRadius = 200;
		fihKey = VK_NUMPAD5;
		stopFihKey = VK_NUMPAD0;
		inventoryKey = 'I';

		saveConfig();
	}
	void loadConfig() { 
		SI_Error rc = ini.LoadFile(filename);
		if (rc < 0) {
			createDefaultCnf();
			ini.LoadFile(filename);
		}

		areaRadius = ini.GetLongValue("Values", "areaRadius");
		fihKey = ini.GetLongValue("Keys", "fihKey");
		stopFihKey = ini.GetLongValue("Keys", "stopFihKey");
		inventoryKey = ini.GetLongValue("Keys", "inventoryKey");
	}
	
	void Hotkey(int* k, const char* label, bool& button, const ImVec2& size_arg = ImVec2(0, 0))
	{
		ImGui::PushID(label);

		//button = false;
		if (!button) {
			if (ImGui::Button(KeyNames[*k], size_arg))
				button = true;
		}
		else
		{
			ImGui::Button(label, size_arg);

			for (auto& Key : KeyCodes)
			{
				if (GetAsyncKeyState(Key) & 0x8000)
                {
                    *k = Key;
					button = false;
					break;
				}
			}
		}
		ImGui::PopID();
	}

	void configWindow(bool& confOpen) {

		ImGui::Begin("Configuration", &confOpen);

		ImGui::Text("Start Fishing"); ImGui::SameLine(); Hotkey(&fihKey, "...##1", fihButton);
		
		ImGui::Text("Stop Fishing"); ImGui::SameLine(); Hotkey(&stopFihKey, "...##2", stopButton);
		
		ImGui::Text("Inventory "); ImGui::SameLine(); Hotkey(&inventoryKey, "...##2", inventoryButton);

		ImGui::End();
	}
};


struct AppState {
public:
	//numbers
	
	const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	//bool
	
	std::atomic<bool> shouldExit = false;
	std::atomic<bool> fihing = false;
	std::thread fishingThread;
	bool confOpen = false;
	
private:
	std::string& statusMessage;
	Config& cnf;

	bool showDemoWindow = false;
	bool showAnotherWindow = false;
	bool showMainWindow = true;
	bool showHint = true;
	bool debug = false;
	
	
	bool fihButtonClicked = false;
	bool stopFihButtonClicked = false;
	

	ImGuiWindowFlags flazhoks =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;
public:
	AppState(Config& cnf, std::string& statusMessage) : cnf(cnf), statusMessage(statusMessage)  {

	}
	void manage() {
		if (confOpen) {
			
			cnf.configWindow(confOpen);
		}

		if (showMainWindow)
			ShowMainWindow();

		/*if (showDemoWindow)
			ImGui::ShowDemoWindow(&showDemoWindow);*/

		if (debug)
			debugWindow();

		/*if (fihing.load()) {

			if (!fishingThread.joinable()) {
				fishingThread = std::thread(&Vision::startCapture, &vizu, std::ref(fihing), std::ref(shouldExit));
			}

			if (GetAsyncKeyState(cnf.stopFih) & 0x8000) {
				fihing = false;
			}
		}
		else {

			if (fishingThread.joinable()) {
				fishingThread.join();
			}
		}*/
	}
private:
	void ShowMainWindow()
	{
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::Begin("FihBot v0.0.0.0", 0, flazhoks);

		if (ImGui::BeginTable("split", 3))
		{
			ImGui::TableNextColumn(); ImGui::Checkbox("demo window", &showDemoWindow);
			ImGui::TableNextColumn(); ImGui::Checkbox("debug,", &debug);

			ImGui::EndTable();
		}

		if (ImGui::Button("config")) {
			confOpen = true;
		}

		ImGui::SliderInt("areaRadius", &cnf.areaRadius, 200, 250);
		if (ImGui::Button("Start"))
		{
			fihing.store(true);
		}
		if (ImGui::Button("Stop"))
		{
			fihing.store(false);
		}

		if (ImGui::Button("close app"))
			shouldExit = true;

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.f / io.Framerate, io.Framerate);
		ImGui::End();
	}

	

	void debugWindow()
	{
		ImGui::SetNextWindowSize(ImVec2(200, 200));
		ImGui::Begin("debug");
		ImGui::Text(statusMessage.c_str());
		ImGui::End();
	};
};


