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
	bool foodButton = false;
	
public:
	Config() {
		ini.SetUnicode();
	}
	int ButtonX = 100;
	int ButtonY = 36;
	ImVec2 standartButton = ImVec2(ButtonX, ButtonY);
	ImGuiWindowFlags flazhoks =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar;

	bool useSalad;
	bool usePie;
	bool useBait;

	int cycles;
	int throwTimeMs;
	int areaRadius;
	int fihKey;
	int stopFihKey;
	int inventoryKey;
	int foodKey;

	void saveConfig() {

		ini.SetLongValue("Keys", "fihKey", fihKey);
		ini.SetLongValue("Keys", "stopFihKey", stopFihKey);
		ini.SetLongValue("Keys", "inventoryKey", inventoryKey);
		ini.SetLongValue("Keys", "foodKey", foodKey);

		ini.SetLongValue("Values", "areaRadius", areaRadius);
		ini.SetLongValue("Values", "throwTimeMs", throwTimeMs);
		ini.SetLongValue("Values", "cycles", cycles);

		ini.SetBoolValue("Bools", "usePie", usePie);
		ini.SetBoolValue("Bools", "useSalad", useSalad);
		ini.SetBoolValue("Bools", "useBait", useBait);
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
		foodKey =      ini.GetLongValue("Keys", "foodKey");

		throwTimeMs =  ini.GetLongValue("Values", "throwTimeMs");
		areaRadius =   ini.GetLongValue("Values", "areaRadius");
		cycles =       ini.GetLongValue("Values", "cycles");

		usePie =       ini.GetBoolValue("Bools", "usePie");
		useSalad =     ini.GetBoolValue("Bools", "useSalad");
		useBait =      ini.GetBoolValue("Bools", "useBait");
	}

	void window(bool& bindsOpen) {

		ImGui::Begin("Binds", &bindsOpen, flazhoks);
		ImGui::BeginGroup();
		ImGui::Text("Start Fishing"); ImGui::SameLine(ButtonX); Hotkey(&fihKey, fihButton, standartButton);
		
		ImGui::Text("Stop Fishing");  ImGui::SameLine(ButtonX); Hotkey(&stopFihKey, stopButton, standartButton);

		ImGui::Text("Inventory");     ImGui::SameLine(ButtonX); Hotkey(&inventoryKey, inventoryButton, standartButton);

		ImGui::Text("Food Slot");     ImGui::SameLine(ButtonX); Hotkey(&foodKey, foodButton, standartButton);


		ImGui::EndGroup();

		if (ImGui::Button("Save Binds")) { saveConfig(); }
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Saves settings too");
		}
		ImGui::End();
	}

	void settingsWindow(bool& settingsOpen) {
		
		ImGui::Begin("Settings", &settingsOpen, flazhoks);
		
		
		//slider areaRadius
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Radius of capture area in pixels\nDefault value = 200");
		} 
		ImGui::SameLine();
		ImGui::SliderInt("Area Size", &areaRadius, 200, 250);
		//slider throwTimeMs
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Determines how far the fishing rod will be thrown in milliseconds\nDefault value = 350");
		}
		ImGui::SameLine();
		ImGui::SliderInt("Throw time", &throwTimeMs, 250, 600);

		//slider throwCount
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			std::string throwTooltip = "Cleanup inventory from logs each " + std::to_string(cycles) + " fishing cycles \nif set to zero = cleanup disabled\nDefault value = 50";
			ImGui::SetTooltip(throwTooltip.c_str());
		}
		ImGui::SameLine();
		ImGui::SliderInt("Cleanup", &cycles, 0, 100);

		
		if (ImGui::Checkbox("Use Pie", &usePie)) {
			useSalad = false;
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Uses any tier pie");
		}
		ImGui::SameLine();
		if (ImGui::Checkbox("Use Salad", &useSalad)) {
			usePie = false;
		}
		ImGui::SameLine();
		ImGui::Checkbox("Use Bait", &useBait);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Uses any tier bait");
		}

		if (ImGui::Button("Save Settings")) { saveConfig(); }
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Saves binds too");
		}
		ImGui::End();
	}

private:
	void createDefaultCnf() {
		fihKey = VK_NUMPAD5;
		stopFihKey = VK_NUMPAD0;
		inventoryKey = 'I';
		foodKey = '2';
		areaRadius = 200;
		throwTimeMs = 350;
		cycles = 50;
		useSalad = false;
		usePie = false;
		useBait = false;
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