#pragma once
#include"simpleini/SimpleIni.h"
#include"buttons/Arrays.hpp"
#include <imgui/imgui_impl_win32.h>

class Config {
private:
	const char* filename = "fih_Settings.ini";
	CSimpleIniA ini;
	bool fihButton = false;
	bool stopButton = false;
	bool inventoryButton = false; 
	
	
	

public:

	int ButtonX = 100;
	int ButtonY = 36;
	ImVec2 standartButton = ImVec2(ButtonX, ButtonY);
	ImGuiWindowFlags flazhoks =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoResize;

	int throwTimeMs;
	int areaRadius;
	int fihKey;
	int stopFihKey;
	int inventoryKey;
	
	void saveConfig() {

		ini.SetLongValue("Keys", "fihKey", fihKey);
		ini.SetLongValue("Keys", "stopFihKey", stopFihKey);
		ini.SetLongValue("Keys", "inventoryKey", inventoryKey);
		ini.SetLongValue("Values", "areaRadius", areaRadius);
		ini.SetLongValue("Values", "throwTimeMs", throwTimeMs);
		SI_Error rc = ini.SaveFile(filename);
	}

	void loadConfig() {
		SI_Error rc = ini.LoadFile(filename);
		if (rc < 0) {
			createDefaultCnf();
			ini.LoadFile(filename);
		}

		
		fihKey =       ini.GetLongValue("Keys", "fihKey");
		stopFihKey =   ini.GetLongValue("Keys", "stopFihKey");
		inventoryKey = ini.GetLongValue("Keys", "inventoryKey");
		throwTimeMs =  ini.GetLongValue("Values", "throwTimeMs");
		areaRadius =   ini.GetLongValue("Values", "areaRadius");
	}

	void window(bool& bindsOpen) {

		ImGui::Begin("Binds", &bindsOpen, flazhoks);
		ImGui::BeginGroup();
		ImGui::Text("Start Fishing"); ImGui::SameLine(ButtonX); Hotkey(&fihKey, fihButton, standartButton);
		
		ImGui::Text("Stop Fishing");  ImGui::SameLine(ButtonX); Hotkey(&stopFihKey, stopButton, standartButton);

		ImGui::Text("Inventory");     ImGui::SameLine(ButtonX); Hotkey(&inventoryKey, inventoryButton, standartButton);


		ImGui::EndGroup();

		ImGui::End();
	}

	void settingsWindow(bool& settingsOpen) {
		ImGui::Begin("Settings", &settingsOpen, flazhoks);
		//slider area radius
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Radius of capture area in pixels\ndefault value = 200");
		} 
		ImGui::SameLine();
		ImGui::SliderInt("Area Size", &areaRadius, 200, 250);
		//slider throwTimeMs
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Determines how far the fishing rod will be thrown in milliseconds\n Values above 550 not recomended\nDefault value = 350");
		}
		ImGui::SameLine();
		ImGui::SliderInt("Throw time", &throwTimeMs, 200, 800);
		ImGui::End();
	}

private:
	void createDefaultCnf() {
		areaRadius = 200;
		throwTimeMs = 350;
		fihKey = VK_NUMPAD5;
		stopFihKey = VK_NUMPAD0;
		inventoryKey = 'I';

		saveConfig();
	}
	void Hotkey(int* k, bool& button, const ImVec2& size_arg = ImVec2(0, 0))
	{
		ImGui::PushID("...");


		if (!button) {
			if (ImGui::Button(KeyNames[*k], size_arg))
				button = true;
		}
		else
		{
			ImGui::Button("...", size_arg);

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
};