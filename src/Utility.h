#pragma once
#include <imgui/imgui_impl_win32.h>

#include<thread>
#include<keyauth/skStr.h>
#include<fstream>



struct AppState {
public:
	
	
	std::atomic<bool> shouldExit = false;
	std::atomic<bool> fihing = false;
	//std::atomic<bool> botAwareMessage = false;
	std::thread fishingThread;
	std::thread tgBotThread;
	
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
	
	bool bindsOpen = false;
	bool settingsOpen = false;
	bool guideOpen = false;
	bool guideEng = true;
	bool botMenu = false;
	bool tgRequest = false;

public:

	AppState(Config& cnf, Vision& vision) : config(cnf), vision(vision) {
		guidelineEng = readFileToString(skCrypt("src/eng.txt").decrypt());
		guidelineRus = readFileToString(skCrypt("src/rus.txt").decrypt());
	}
	void setMainTitle(std::string title) {
		windowTitle = skCrypt("FihBot ").decrypt() + title;
	}
	
	void manage() {
		
		if (!fihing.load()) {
			showMainWin = true;
		}
		else {
			showMainWin = false;
		}

		if (bindsOpen && showMainWin) {
			config.keybindsWindow(bindsOpen);
		}
		if (settingsOpen && showMainWin) {
			config.settingsWindow(settingsOpen);
		}

		if (guideOpen && showMainWin)
			GuideWindow();

		if (showMainWin)
			ShowMainWindow();


		if (config.viewOpen)
			vision.viewWindow();
		
		
		
	}
	
private:
	void ShowMainWindow()
	{
		
		ImGui::Begin(windowTitle.c_str(), 0, config.flazhoks);
		
		if (ImGui::Button(skCrypt("Guide"), config.standartButton)) {

			guideOpen = !guideOpen;

		}
		
		ImGui::SameLine(config.ButtonX*3);
		if (ImGui::Button(skCrypt("Keybinds"), config.standartButton)) {

			bindsOpen = !bindsOpen;
		}
		if (ImGui::Button(skCrypt("Start"), ImVec2(config.ButtonX, config.ButtonY * 2)))
		{
			fihing.store(true);
		}
		ImGui::SameLine(config.ButtonX * 3);
		ImGui::BeginGroup();
		
		if (ImGui::Button(skCrypt("Settings"), config.standartButton)) {

			settingsOpen = !settingsOpen;
		}
		if (ImGui::Button(skCrypt("Close"), config.standartButton))
			shouldExit = true;
		ImGui::EndGroup();
		
		
		

		
		ImGui::Checkbox(skCrypt("View"), &config.viewOpen);
		ImGui::Checkbox(skCrypt("TgBot menu"), &botMenu);
		
		ImGui::SameLine(config.ButtonX * 2.5);
		ImGui::Text(author2.c_str());
		if (botMenu) {
			ImGui::Text("Id: "); ImGui::SameLine();
			ImGui::PushItemWidth(100.0f);
			ImGui::InputText(" ", config.idBuff, IM_ARRAYSIZE(config.idBuff));
			ImGui::TextLinkOpenURL(skCrypt("TgBot URL"), skCrypt("https://t.me/KabaniyPromisel_bot"));

			ImGui::TextColored(ImVec4(255, 0, 0, 255), skCrypt("WARNING that may freeze menu for about 20 seconds"));
			if (ImGui::RadioButton(skCrypt("Start Request"), tgRequest)) {
				tgRequest = true;
				if (!tgBotThread.joinable()) {
					tgBotThread = std::thread(Notifier::sendLongPollEvent, std::ref(bot));
				}
			}
			ImGui::SameLine();
			if (ImGui::RadioButton(skCrypt("Stop Request"), !tgRequest)) {
				tgRequest = false;
				if (tgBotThread.joinable()) {
					tgBotThread.join();
				}
			}
			
		}

		ImGui::End();
	}
	void GuideWindow(){
		ImGui::Begin(skCrypt("Guide"), NULL, config.flazhoks);

		if (guideEng) {
			ImGui::Text(guidelineEng.c_str());
		}
		else {
			ImGui::Text(guidelineRus.c_str());
		}

		if (ImGui::RadioButton(skCrypt("Eng"), guideEng)) {
			guideEng = true;
		}
		if (ImGui::RadioButton(skCrypt("Ru"), !guideEng)) {
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


