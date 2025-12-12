#include "Vision.h"
#include <iostream>
#include <imgui/imgui.h>
#include<chrono>
Vision::Vision(Config& config, ID3D11Device* g_pd3dDevice) : config(config), g_pd3dDevice_(g_pd3dDevice) {
	if (!init) {
		initWindow();
	}
	
}
Vision::~Vision()
{
	if (bitmap) {
		DeleteObject(bitmap);
		bitmap = nullptr;
	}
	if (memoryDeviceContext) {
		DeleteDC(memoryDeviceContext);
		memoryDeviceContext = nullptr;
	}
	if (windowDesk && deviceContext) {
		ReleaseDC(windowDesk, deviceContext);
		deviceContext = nullptr;
	}

}
void Vision::startCapture(std::atomic<bool>& fihingState, std::atomic<bool>& shouldExit) {

	
	if (!isAreaSelected) {
		selectAreaWithMouse(fihingState);
	}
	
	clockStart = std::chrono::high_resolution_clock::now();
	if (config.usePie || config.useBait || config.useSalad) {
		buffsActive = true;
		if(config.usePie || config.useSalad)
		   timeFoodStart = clockStart;
		if(config.useBait)
		   timeBaitStart = clockStart;
	}
	//clockStart = std::chrono::high_resolution_clock::now();
	while (fihingState.load())
	{
		if (shouldExit.load()) { break; }

		auto clockEnd = std::chrono::high_resolution_clock::now();
			/*if (duration > std::chrono::seconds(360))
				fihingState = false;*/

			CaptureFih();
			/*auto clockEnd = std::chrono::high_resolution_clock::now();
			std::chrono::seconds duration = std::chrono::duration_cast<std::chrono::seconds>(clockEnd - clockStart);
			if (duration > std::chrono::seconds(360))
				fihingState = false;*/
				//сначала фикс логики, потом таймер
			
	}
	
	stopCapture();
	
}

void Vision::CaptureFih()
{   

	switch (status)
	{

	case STARTED:
		//std::cout << "status = STARTED" << std::endl;
		getDesktopMat();
		getImage();

		statusMessage = "thrown";
		pressKeyMouseLeft(config.throwTimeMs);
		status = LOOKING;
		break;

	case LOOKING:
		//std::cout << "status = LOOKING" << std::endl;
		statusMessage = "looking for bobber";
		std::this_thread::sleep_for(std::chrono::milliseconds(650));
		
		if (checkRestart()) { break; }
		getDesktopMat();
		getImage();
		

		
		if (boundRect.area() > inWaterSizeMin) {
			status = FOUND;
			
			break;
		}
		
		
		break;
	case FOUND:
		statusMessage = "found and watching";
		//std::cout << "status = FOUND " << std::endl;
		getDesktopMat();
		getImage();
        
		if (boundRect.area() < inWaterSizeMin) {
			status = CATCH;
			if (checkRestart()) { break; }
			pressKeyMouseLeft(10);
			
		}
		
		
		
		break;

	case CATCH:
		statusMessage = "catching";
		//std::cout << "status = CATCH" << std::endl;
		getDesktopMat();
		getImage();
		//catchProcess();
		
		//std::cout << "boundRect.area = " << boundRect.area() << std::endl; //debug
		if (boundRect.empty()) {
			emptyCounter++;

			if (emptyCounter >= 5) {
				inputCatch.mi.dwFlags = MOUSEEVENTF_LEFTUP;
				SendInput(1, &inputCatch, sizeof(INPUT));

				emptyCounter = 0;
				status = STARTED;
				break;
			}
		}
		else {
			catchProcess();
		}
		
		
		break;

	case FINISHED:
		clockStart = std::chrono::high_resolution_clock::now();

		status = STARTED;
		statusMessage = "Fihing end";
		scaleRect = cv::Rect();
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		if (checkRestart()) { break; } 
		if (config.cycles != NULL) {
			cleanCounter++;

			if (cleanCounter == config.cycles) {
				statusMessage = "cleaning";
				cleanInventory();
				cleanCounter = 0;
				
			}
		}
		if (buffsActive) {

			if (config.useBait) {
				auto endBait = std::chrono::high_resolution_clock::now();
				auto durationBait = std::chrono::duration_cast<std::chrono::minutes>(endBait - timeBaitStart);
				if (durationBait >= std::chrono::minutes(10)) {
					useBuff(BAIT);
					timeBaitStart = std::chrono::high_resolution_clock::now();
				}
			}
			//if(timelogic1 and config.useBait)
			
			if (config.usePie || config.useSalad) {
				auto endFood = std::chrono::high_resolution_clock::now();
				auto durationFood = std::chrono::duration_cast<std::chrono::minutes>(endFood - timeFoodStart);
				if (durationFood >= std::chrono::minutes(30)) {
					useBuff(SLOT);
					timeFoodStart = std::chrono::high_resolution_clock::now();
				}
			}
			//if(timelogic2 and config.usePie || config.useSalad)
			
		}
		
		
		
		break;
	case SERVER_RESTART:
		clockStart = std::chrono::high_resolution_clock::now();

		RestartingP1();
		clockStart = std::chrono::high_resolution_clock::now();
		if (okWindowCheck()) { break; }
		RestartingP2();
		clockStart = std::chrono::high_resolution_clock::now();

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		SetCursorPos(itThatRemember.x, itThatRemember.y);
		status = STARTED;
		break;
	default:
		break;
	}
	

}

void Vision::catchProcess() { 
	
	

	

	if (boundRect.area() <= inWaterSizeMin && boundRect.area() >= inScaleSize)
	{
		inputCatch.type = INPUT_MOUSE;
		inputCatch.mi.dx = 0;
		inputCatch.mi.dy = 0;
		cv::Point center(
			boundRect.x + boundRect.width / 2,
			boundRect.y + boundRect.height / 2
		);
		//std::cout << "center.x = "<< center.x << std::endl; //debug

		if (center.x <= scalePosDOWN && (inputCatch.mi.dwFlags != MOUSEEVENTF_LEFTDOWN)) {
			inputCatch.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
			SendInput(1, &inputCatch, sizeof(INPUT));
			std::cout << "Mouse DOWN" << std::endl;
		}
		if (center.x >= scalePosUP && (inputCatch.mi.dwFlags != MOUSEEVENTF_LEFTUP)) {
			inputCatch.mi.dwFlags = MOUSEEVENTF_LEFTUP;
			SendInput(1, &inputCatch, sizeof(INPUT));
			std::cout << "Mouse UP" << std::endl;
		}

	}
	else {
		if (inputCatch.mi.dwFlags != MOUSEEVENTF_LEFTUP) {
			inputCatch.mi.dwFlags = MOUSEEVENTF_LEFTUP;
			SendInput(1, &inputCatch, sizeof(INPUT));
			std::cout << "Mouse UP" << std::endl;
		}
	}
	
	}

void Vision::stopCapture()
{
	if (cv::getWindowProperty(winName, cv::WND_PROP_VISIBLE) > 0) {
		cv::destroyWindow(winName);
	}
	if (bitmap)              DeleteObject(bitmap);
	if (memoryDeviceContext) DeleteDC(memoryDeviceContext);
	if (deviceContext)       ReleaseDC(windowDesk, deviceContext);
	status = STARTED;
	statusMessage = "stopped";
	isAreaSelected = false;
	buffsActive = false;
	gdiInitialized = false;
	itThatRemember = POINT();
	cleanCounter = 0;
	cropRect = cv::Rect();
	scaleRect = cv::Rect();
}

void Vision::pressKeyMouseLeft(int KeyUpMillisec) {

	INPUT input = { 0 };

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	input.mi.dx = 0;
	input.mi.dy = 0;

	SendInput(1, &input, sizeof(input));
	std::this_thread::sleep_for(std::chrono::milliseconds(KeyUpMillisec));
	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(input));
}

void Vision::sendKeyPress(int keyCode) {
	INPUT input = { 0 };
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;

	// Нажатие клавиши
	input.ki.wVk = keyCode; // Код клавиши
	input.ki.dwFlags = 0; // 0 для нажатия клавиши
	SendInput(1, &input, sizeof(INPUT));

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	// Отпускание клавиши
	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
}

bool Vision::initWindow() {
	windowDesk = GetDesktopWindow();
	init = true;
	return init;
}

void Vision::getDesktopMat() {
	if (!gdiInitialized) {
		deviceContext = GetDC(windowDesk);
		if (!deviceContext) return;
		memoryDeviceContext = CreateCompatibleDC(deviceContext);

		
		BITMAPINFO bmi = { 0 }; //specify format by using bitmap info header
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = screenWidth;
		bmi.bmiHeader.biHeight = -screenHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		void* pBits = nullptr;
		bitmap = CreateDIBSection(memoryDeviceContext, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
		SelectObject(memoryDeviceContext, bitmap);

		//straight use of bitmap data
		fullScale = cv::Mat(screenHeight, screenWidth, CV_8UC4, pBits);
		gdiInitialized = true;
	}
	
	//copy data into the bitmap
	BitBlt(memoryDeviceContext, 0, 0, screenWidth, screenHeight, deviceContext, 0, 0, SRCCOPY);

	
}

void Vision::selectAreaWithMouse(std::atomic<bool>& fihingState) {

	POINT cursorPos;
	statusMessage = "waiting for area selection";
	
	while (fihingState.load()) {

		if (GetAsyncKeyState(config.fihKey) & 0x8000) {

			while (GetAsyncKeyState(config.fihKey) & 0x8000) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			if (!fihingState.load()) break;

			GetCursorPos(&cursorPos);
			ScreenToClient(windowDesk, &cursorPos);

			itThatRemember = cursorPos;

			
			selectedArea.left = cursorPos.x - config.areaRadius / 2;
			selectedArea.top = cursorPos.y - config.areaRadius / 2;
			selectedArea.right = cursorPos.x + config.areaRadius / 2;
			selectedArea.bottom = cursorPos.y + config.areaRadius / 2;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	isAreaSelected = fihingState.load();
}

cv::Mat Vision::cropMat() {
	if (!cropRect.empty())
	{
		return fullScale(cropRect).clone();
	}
	else {
		int x = max(0, selectedArea.left);
		int y = max(0, selectedArea.top);
		int width = min(fullScale.cols - x, selectedArea.right - selectedArea.left);
		int height = min(fullScale.rows - y, selectedArea.bottom - selectedArea.top);

		if (width <= 0 || height <= 0) {
			return cv::Mat();
		}

		cropRect = cv::Rect(x, y, width, height);
		return fullScale(cropRect).clone();
	}


}

void Vision::getMaskColorBased(cv::Mat& imgMask, objType type) {

	cv::Scalar Lower(objHSV[type][HMIN],
		             objHSV[type][SMIN],
		             objHSV[type][VMIN]);

	cv::Scalar Upper(objHSV[type][HMAX],
		             objHSV[type][SMAX],
		             objHSV[type][VMAX]);

	inRange(imgHSV, Lower, Upper, imgMask);
}

void Vision::getImage() {
	
	

	static double minVal = 30;

	if (status != CATCH) {
		img = cropMat();
	}
	else {
		img = matchingMethod(SCALE, scaleRect);
	}
	
	if (img.empty()) {
		
		return;
	}
	cvtColor(img, imgHSV, cv::COLOR_BGR2HSV); 

	getMaskColorBased(imgMask, BOBBER);

	
	cv::Mat blurred;
	GaussianBlur(imgMask, blurred, cv::Size(3, 3), 0);

	//find countour
	findContours(blurred, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	
	
	boundRect = cv::Rect();

	double maxArea = 0;
	int maxAreaIdx = -1;
	
	//find max area
	for (size_t i = 0; i < contours.size(); ++i) {

		double area = cv::contourArea(contours[i]);

		if (area < minVal) {
			continue;
		}
		if (area > maxArea) {
			std::cout << "area = " << area << std::endl;
			maxArea = area;
			maxAreaIdx = i;
		}
	}

	//find rect for bobber
	if (maxAreaIdx >= 0) {
		boundRect = cv::boundingRect(contours[maxAreaIdx]);
		//std::cout << boundRect.area() << std::endl;
		
		if (status != CATCH) {
			if (boundRect.area() >= inWaterSizeMin && boundRect.area() <= inWaterSizeMax) {
				cv::rectangle(img, boundRect.tl(), boundRect.br(), cv::Scalar(0, 0, 255, 255), 3);
			}

		}
		else {
			if (boundRect.area() < inWaterSizeMin && boundRect.area() >= inScaleSize) {
				cv::rectangle(img, boundRect.tl(), boundRect.br(), cv::Scalar(0, 0, 255, 255), 3);
			}
		}
	}
	

	
}

/** @brief Matches the given template, no mask.

@param type of object see matchingEnum
@return  fullScale(storedRect).clone() 

 */
cv::Mat Vision::matchingMethod(matchingEnum type, cv::Rect& storedRect) //надо сделать так чтобы принимал и рект, чтобы не ебаться каждый раз с опустошением сторедРекта и опустошать только скейлРект
{
	
	if(!storedRect.empty())
	{ 
		return fullScale(storedRect).clone();
	
	}

	else {
		
		int startRow;
		cv::Mat cropped = fullScale;
		
		if (status == CATCH) {
			startRow = fullScale.rows * 0.45;
			cropped = fullScale(cv::Range(startRow, fullScale.rows), cv::Range::all());
		}

		cv::Mat templ4chnl;
		cv::cvtColor(matchingTempl[type], templ4chnl, cv::COLOR_BGR2BGRA);
	    
		if (templ4chnl.empty()) {
			statusMessage = "Template Not Found";
			storedRect = cv::Rect();
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			return cv::Mat();
		}

		

		cv::Mat result;
		int result_cols = cropped.cols - templ4chnl.cols + 1;
		int result_rows = cropped.rows - templ4chnl.rows + 1;

		result.create(result_rows, result_cols, CV_32FC1);

		
		try {
			cv::matchTemplate(cropped, templ4chnl, result, matchingModes[type]);
		}
		catch (const cv::Exception& e) {
			if (cropped.type() != templ4chnl.type())
			{
				statusMessage = "types of image not matching";
			}
			else {
				statusMessage = "idk exeption lol";
			}
			storedRect = cv::Rect();
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			return cv::Mat();
		}
			

			
			double minVal; double maxVal;
			cv::Point minLoc, maxLoc, matchLoc;

			if  (  matchingModes[type] != cv::TM_SQDIFF_NORMED
				&& matchingModes[type] != cv::TM_CCOEFF_NORMED
				&& matchingModes[type] != cv::TM_CCORR_NORMED
				) 
			{
				normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
			}
			
			

			double thresholdValue = (double)matchingThresholds[type] / 255.0;
			threshold(result, result, thresholdValue, 1.0, cv::THRESH_BINARY);

			minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

			
			

			if (matchingModes[type] == cv::TM_SQDIFF || matchingModes[type] == cv::TM_SQDIFF_NORMED) {
				if (minLoc == cv::Point()) {
					std::cout << "MINLOC EMPTY" << std::endl;

					return cv::Mat();
					
				}
				matchLoc = minLoc;
			}
			else {
				if (maxLoc == cv::Point()) {
					std::cout << "MAXLOC EMPTY" << std::endl;

				    return cv::Mat();
				
				}
				matchLoc = maxLoc;
			}
			
			
			
			

			if (status == CATCH) {
				storedRect = cv::Rect(matchLoc, cv::Point(matchLoc.x + templ4chnl.cols, matchLoc.y + templ4chnl.rows));
				storedRect.y += startRow;
				storedRect.height -= 24;
				
			}
			else { 
				storedRect = cv::Rect(matchLoc, cv::Point(matchLoc.x + templ4chnl.cols, matchLoc.y + templ4chnl.rows));
				itThatPOINT = POINT(matchLoc.x + templ4chnl.cols / 2, matchLoc.y + templ4chnl.rows / 2);

			}
			
			return fullScale(storedRect).clone();
			//работает по такому же принципу как кроп мат
			



		
		
	}
	
}

void Vision::TextureForDebug()
{
	if (img.empty()) {
		return;
	}
	
	// Создаём текстуру
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = img.cols;
	desc.Height = img.rows;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = img.data;
	initData.SysMemPitch = img.cols * 4;

	ID3D11Texture2D* texture = nullptr;
	g_pd3dDevice_->CreateTexture2D(&desc, &initData, &texture);

	// Создаём шейдерный ресурс
	g_pd3dDevice_->CreateShaderResourceView(texture, nullptr, &textureSRV);
	
	texture->Release();
}

void Vision::cleanInventory()
{
	//start
	sendKeyPress(config.inventoryKey);
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
	getDesktopMat();
	//
	cv::Rect cleanRect = cv::Rect();
	cv::Mat cleanImage = matchingMethod(LOGS, cleanRect);
	if (cleanImage.empty()) {
		statusMessage = "no trash found";
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		return;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	SetCursorPos(itThatPOINT.x, itThatPOINT.y); 

	INPUT input = { 0 };

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	input.mi.dx = 0;
	input.mi.dy = 0;

	std::this_thread::sleep_for(std::chrono::milliseconds(300));

	SendInput(1, &input, sizeof(input));
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	SetCursorPos(fullScale.cols / 2, fullScale.rows / 2);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(input));
	std::this_thread::sleep_for(std::chrono::milliseconds(300));

	SetCursorPos(fullScale.cols / 2 - 100, fullScale.rows / 2);
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
	pressKeyMouseLeft(10);

	//end
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	sendKeyPress(config.inventoryKey);
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
	SetCursorPos(itThatRemember.x, itThatRemember.y);
	//
}


void Vision::useBuff(matchingEnum type)
{
	
	cv::Mat buffImage;
	if (type == SLOT) {
		getDesktopMat();
		cv::Rect slotRect = cv::Rect();
		cv::Mat slotImage = matchingMethod(type, slotRect);
		if (!slotImage.empty()) { // this condition == if slotImage found
			statusMessage = "trying to find food";
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			//scaleRect = cv::Rect();

			sendKeyPress(config.inventoryKey);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			getDesktopMat();

			if (config.usePie) {
				cv::Rect pieRect = cv::Rect();
				buffImage = matchingMethod(PIE, pieRect);
				if (buffImage.empty()) {
					sendKeyPress(config.inventoryKey);
					return;
				}

			}
			else {
				cv::Rect saladRect = cv::Rect();
				buffImage = matchingMethod(SALAD, saladRect);
				if (buffImage.empty()) {
					sendKeyPress(config.inventoryKey);
					return;
				}

			}

			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			SetCursorPos(itThatPOINT.x, itThatPOINT.y);
			INPUT inputs = { 0 };
			

			inputs.type = INPUT_KEYBOARD;
			inputs.ki.wVk = VK_SHIFT;

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			SendInput(1, &inputs, sizeof(INPUT));
			
			ZeroMemory(&inputs, sizeof(INPUT));

			inputs.type = INPUT_MOUSE;
			inputs.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));

			SendInput(1, &inputs, sizeof(INPUT));

			std::this_thread::sleep_for(std::chrono::milliseconds(10));

			ZeroMemory(&inputs, sizeof(INPUT));
			inputs.type = INPUT_MOUSE;
			inputs.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
			SendInput(1, &inputs, sizeof(INPUT));

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			ZeroMemory(&inputs, sizeof(INPUT));
			inputs.type = INPUT_KEYBOARD;
			inputs.ki.dwFlags = KEYEVENTF_KEYUP;
			inputs.ki.wVk = VK_SHIFT;
			SendInput(1, &inputs, sizeof(INPUT));

			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			sendKeyPress(config.inventoryKey);
			std::this_thread::sleep_for(std::chrono::seconds(12));
			sendKeyPress(config.foodKey);

	
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			SetCursorPos(itThatRemember.x, itThatRemember.y);

			return;
		}

		else { //if not found just press food
			
			sendKeyPress(config.foodKey);
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			return;
		}
	}

	else { //logic for bait here
		sendKeyPress(config.inventoryKey);
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		getDesktopMat();
		cv::Rect baitRect = cv::Rect();
		buffImage = matchingMethod(BAIT, baitRect);
		if (buffImage.empty()) {
			return;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(700));
		SetCursorPos(itThatPOINT.x, itThatPOINT.y);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		pressKeyMouseLeft(10);
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		getDesktopMat();

		//scaleRect = cv::Rect();
		cv::Rect usebRect = cv::Rect();
		buffImage = matchingMethod(USEBUTTON, usebRect);
		if (buffImage.empty()) {
			sendKeyPress(config.inventoryKey);
			return;
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(700));
		SetCursorPos(itThatPOINT.x, itThatPOINT.y);

		std::this_thread::sleep_for(std::chrono::milliseconds(700));
		pressKeyMouseLeft(10);
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		sendKeyPress(config.inventoryKey);

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		SetCursorPos(itThatRemember.x, itThatRemember.y);

	}

}

bool Vision::checkRestart() { 
	
	cv::Rect logoRect = cv::Rect();
	cv::Mat logoImage = matchingMethod(MAINLOGO, logoRect);
	if (logoImage.empty()) {
		
		return false;
	}
	else {
		status = SERVER_RESTART;
		
		return true;
	}
}

bool Vision::getSelectAreaState()
{
	return isAreaSelected;
}

void Vision::debugWindow() {
	
	if (isAreaSelected) {
		TextureForDebug();
		imguiTexture = (ImTextureID)textureSRV.Get();
	}
	else {
		imguiTexture = NULL;
	}
	
	ImGui::Begin("debug");
	ImGui::Text(statusMessage.c_str());
	
	if (status != CATCH) {
		ImGui::Image(imguiTexture, ImVec2(config.areaRadius, config.areaRadius));
	}
	else {
		ImGui::Image(imguiTexture, ImVec2(scaleRect.width, scaleRect.height));
	}
	std::string errorTimer = "error timer: " + std::to_string(duration);
	ImGui::Text(errorTimer.c_str());
	
	
	ImGui::End();
}

void Vision::RestartingP1() { 
	cv::Rect logoRect = cv::Rect();
	cv::Mat logoMat = matchingMethod(MAINLOGO, logoRect);
	if (!logoMat.empty()) {
		cv::Rect noticeRect = cv::Rect();
		cv::Mat noticeMat = matchingMethod(SERVERNOTICE, noticeRect);
		if (!noticeMat.empty()) {
			statusMessage = "notice found, sleeping for 4 minutes";
			std::this_thread::sleep_for(std::chrono::minutes(4));
		}
		cv::Rect loginRect = cv::Rect();
		cv::Mat loginMat = matchingMethod(LOGINBUTTON, loginRect);
		if (!loginMat.empty()) {
			SetCursorPos(itThatPOINT.x, itThatPOINT.y);
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			pressKeyMouseLeft(12);
			std::this_thread::sleep_for(std::chrono::seconds(20)); //after login button clicked
		}

	}
}

bool Vision::okWindowCheck() 
{
	cv::Rect okRect = cv::Rect();
	cv::Mat okMat = matchingMethod(OKBUTTON, okRect);
	if (!okMat.empty()) {
		SetCursorPos(itThatPOINT.x, itThatPOINT.y);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		pressKeyMouseLeft(12);
		return true;
	}
	else {
		return false;
	}
}

void Vision::RestartingP2() 
{
	cv::Rect enterWorldRect = cv::Rect();
	cv::Mat enterWorldMat = matchingMethod(ENTERWORLDBUTTON, enterWorldRect);
	if (!enterWorldMat.empty()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		SetCursorPos(itThatPOINT.x, itThatPOINT.y);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		pressKeyMouseLeft(12);
		std::this_thread::sleep_for(std::chrono::seconds(10));
		INPUT mouseWheelUP = { 0 };
		mouseWheelUP.type = INPUT_MOUSE;
		mouseWheelUP.mi.dwFlags = MOUSEEVENTF_WHEEL;
		mouseWheelUP.mi.mouseData = WHEEL_DELTA;
		for (size_t i = 0; i < 13; i++)
		{
			SendInput(1, &mouseWheelUP, sizeof(INPUT));
			std::this_thread::sleep_for(std::chrono::milliseconds(65));
		}

	}
}
