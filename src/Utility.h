#pragma once
#include <imgui/imgui_impl_win32.h>

#include"Config.h"
#include"Vision.h"
#include<keyauth/skStr.h>




struct AppState {
public:
	
	
	std::atomic<bool> shouldExit = false;
	std::atomic<bool> fihing = false;
	std::thread fishingThread;
	
	
private:
	Vision& vision;
	Config& config;

	const std::string compilation_date = (std::string)skCrypt(__DATE__);
	const std::string compilation_time = (std::string)skCrypt(__TIME__);
	std::string windowTitle = skCrypt("FihBot 1.0 - Built at:  ").decrypt() + compilation_date + " " + compilation_time;
	std::string guidelineRus;
	std::string guidelineEng;
	std::string author2 = skCrypt("Telegram @Sektor223").decrypt();

	bool showMainWin = true;
	bool showHint = true;
	bool debug = false;
	bool bindsOpen = false;
	bool settingsOpen = false;
	bool guideOpen = false;
	bool guideEng = true;
	
	
public:
	AppState(Config& cnf, Vision& vision) : config(cnf), vision(vision) {
		guidelineEng = readFileToString("src/eng.txt");
		guidelineRus = readFileToString("src/rus.txt");
	}
	void setMainTitle(std::string title) {
		windowTitle = "FihBot 1.0 " + title;
	}
	void manage() {
		
		if (!fihing.load()) {
			showMainWin = true;
		}
		else {
			showMainWin = false;
		}

		if (bindsOpen && showMainWin) {
			config.window(bindsOpen);
		}
		if (settingsOpen && showMainWin) {
			config.settingsWindow(settingsOpen);
		}

		if (guideOpen && showMainWin)
			GuideWindow();

		if (showMainWin)
			ShowMainWindow();


		if (debug)
			vision.debugWindow();
		
		
		
	}
	
private:
	void ShowMainWindow()
	{
		//ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::Begin(windowTitle.c_str(), 0, config.flazhoks);
		
		if (ImGui::Button("Guide", config.standartButton)) {

			guideOpen = !guideOpen;

		}
		
		ImGui::SameLine(config.ButtonX*3);
		if (ImGui::Button("Keybinds", config.standartButton)) {

			bindsOpen = !bindsOpen;
		}
		if (ImGui::Button("Start", ImVec2(config.ButtonX, config.ButtonY * 2)))
		{
			fihing.store(true);
		}
		ImGui::SameLine(config.ButtonX * 3);
		ImGui::BeginGroup();
		
		if (ImGui::Button("Settings", config.standartButton)) {

			settingsOpen = !settingsOpen;
		}
		if (ImGui::Button("Close", config.standartButton))
			shouldExit = true;
		ImGui::EndGroup();
		
		
		

		
		ImGui::Checkbox("View", &debug);
		ImGui::SameLine(config.ButtonX * 2.5);
		ImGui::Text(author2.c_str());
		
		ImGui::End();
	}
	void GuideWindow(){
		ImGui::Begin("Guide", NULL, config.flazhoks);

		if (guideEng) {
			ImGui::Text(guidelineEng.c_str());
		}
		else {
			ImGui::Text(guidelineRus.c_str());
		}

		if (ImGui::RadioButton("Eng", guideEng)) {
			guideEng = true;
		}
		if (ImGui::RadioButton("Ru", !guideEng)) {
			guideEng = false;
		}
		ImGui::End();
	}
	std::string readFileToString(const std::string& path) {
		std::ifstream file(path);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str(); 
	}

};


