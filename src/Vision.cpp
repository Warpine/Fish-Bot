#include "Vision.h"
#include <iostream>
#include <imgui/imgui.h>

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

	if (!areaSelected) {
		selectAreaWithMouse(fihingState);
	}
	//auto clockStart = std::chrono::high_resolution_clock::now();
	while (fihingState.load())
	{
		if (shouldExit.load()) { break; }
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
	//uses when CATCH for the first time in fishing cycle
	//and became true when FINISHED
	static bool firstTimeSleep = true;

	switch (status)
	{
	case STOPPED:
		statusMessage = "stopped";

		status = STARTED;

		break;

	case STARTED: //тут  должно быть закидывание удочки
		getDesktopMat();
		getImage();
		
		statusMessage = "thrown";
		pressKeyMouseLeft(500);
		status = LOOKING;
		break;

	case LOOKING:
		statusMessage = "looking for bobber";
		std::this_thread::sleep_for(std::chrono::milliseconds(650));
		getDesktopMat();
		getImage();
		if (boundRect.area() >= inWaterSize) {
			status = FOUND;
			break;
		}
		
		
		break;
	case FOUND:
		statusMessage = "found and watching";
	
		
		getDesktopMat();
		getImage();
        
		if (boundRect.area() < inWaterSize && boundRect.area() >= inScaleSize) {
			pressKeyMouseLeft(15);
			status = CATCH;
		}
		//std::cout << currentY << std::endl;
		
		/*if (abs(currentY - prevY) < 10) {
			prevY = 0;
			status = CATCH;
			pressKeyMouseLeft(15);
		}
		else {
			prevY = currentY;
		}*/
		//if (boundRect.area() < inWaterSize) //скорее всего не работает
		//{
		//	status = CATCH;
		//	pressKeyMouseLeft(15);
		//	break;
		//}
		
		
		break;

	case CATCH:
		statusMessage = "catching";
		
		if (firstTimeSleep) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			firstTimeSleep = false;
		}
		getDesktopMat();
		getImage();
		catchProcess();
		
		std::cout << boundRect.area() << std::endl; //debug
		if (boundRect.area() <= inScaleSize) { //как срать сука какая тут логика нужна
			status = FINISHED;
		}
		

		break;

	case PULL:
		statusMessage = "pull";

		status = FINISHED;
		break;

	case RELEASE:
		statusMessage = "Release";

		status = FINISHED;
		break;

	case FINISHED:
		statusMessage = "Fihing end";
		
		inputCatch = { 0 };
		firstTimeSleep = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		status = STARTED;
		break;
	default:
		break;
	}


	std::this_thread::sleep_for(std::chrono::milliseconds(10));


}

void Vision::catchProcess() { 
	
	inputCatch.type = INPUT_MOUSE;
	inputCatch.mi.dx = 0;
	inputCatch.mi.dy = 0;

	if (boundRect.area() < inWaterSize && boundRect.area() >= inScaleSize)
	{
		cv::Point center(
			boundRect.x + boundRect.width / 2,
			boundRect.y + boundRect.height / 2
		);
		//std::cout << center.x << std::endl; //debug

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
	
	}



void Vision::stopCapture()
{
	if (cv::getWindowProperty(winName, cv::WND_PROP_VISIBLE) > 0) {
		cv::destroyWindow(winName);
	}
	if (bitmap)              DeleteObject(bitmap);
	if (memoryDeviceContext) DeleteDC(memoryDeviceContext);
	if (deviceContext)       ReleaseDC(windowDesk, deviceContext);
	gdiInitialized = false;
	status = STOPPED;
	statusMessage = "Not watching";
	areaSelected = false;
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

			
			selectedArea.left = cursorPos.x - config.areaRadius / 2;
			selectedArea.top = cursorPos.y - config.areaRadius / 2;
			selectedArea.right = cursorPos.x + config.areaRadius / 2;
			selectedArea.bottom = cursorPos.y + config.areaRadius / 2;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	areaSelected = fihingState.load();
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
	static const int MIN_CONTOUR_AREA = 50;

	if (status != CATCH) {
		img = cropMat();
	}
	else {
		img = matchingMethod();
	}
	
	if (img.empty()) {
		statusMessage = "img empty restarting...";
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		status = STOPPED;
		return;
	}
	cvtColor(img, imgHSV, cv::COLOR_BGR2HSV); 

	getMaskColorBased(imgMask, BOBBER);

	//сюда нужен гаусиан блюр и временно убрать переход в катч, замерить inWaterSize
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

		if (area < MIN_CONTOUR_AREA) {
			continue;
		}
		if (area > maxArea) {
			maxArea = area;
			maxAreaIdx = i;
		}
	}

	//find rect for bobber
	if (maxAreaIdx >= 0) {
		boundRect = cv::boundingRect(contours[maxAreaIdx]);
		//std::cout << boundRect.area() << std::endl;
		
		if (status != CATCH) {
			if (boundRect.area() >= inWaterSize) {
				cv::rectangle(img, boundRect.tl(), boundRect.br(), cv::Scalar(0, 0, 255, 255), 3);
			}

		}
		else {
			if (boundRect.area() < inWaterSize && boundRect.area() >= inScaleSize) {
				cv::rectangle(img, boundRect.tl(), boundRect.br(), cv::Scalar(0, 0, 255, 255), 3);
			}
		}
	}
	

	
}

cv::Mat Vision::matchingMethod()
{
	if(!scaleRect.empty())
	{ 
		return fullScale(scaleRect).clone();
	
	}

	else {
		int match_method = cv::TM_SQDIFF_NORMED;

		cv::cvtColor(templ, templ4chnl, cv::COLOR_BGR2BGRA);

		if (templ4chnl.empty()) {
			statusMessage = "Template Not Found";
			scaleRect = cv::Rect();
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			return cv::Mat();
		}

		

		cv::Mat result;
		int result_cols = fullScale.cols - templ4chnl.cols + 1;
		int result_rows = fullScale.rows - templ4chnl.rows + 1;

		result.create(result_rows, result_cols, CV_32FC1);

		
		try {
			cv::matchTemplate(fullScale, templ4chnl, result, match_method); 
		}
		catch (const cv::Exception& e) {
			if (fullScale.type() != templ4chnl.type())
			{
				statusMessage = "types of image not matching";
			}
			else {
				statusMessage = "idk exeption lol";
			}
			scaleRect = cv::Rect();
			std::this_thread::sleep_for(std::chrono::milliseconds(5000));
			return cv::Mat();
		}
			

			
			double minVal; double maxVal;
			cv::Point minLoc, maxLoc, matchLoc;

			if (match_method != cv::TM_SQDIFF_NORMED && match_method != cv::TM_CCOEFF_NORMED && match_method != cv::TM_CCORR_NORMED) {
				normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
			}
			
			

			double thresholdValue = 12 / 255.0;
			threshold(result, result, thresholdValue, 1.0, cv::THRESH_BINARY);

			minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

			
			

			if (match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED) {
				matchLoc = minLoc;
			}
			else {
				statusMessage = "cringe";
				return cv::Mat();
			}
			
			
			scaleRect = cv::Rect(matchLoc, cv::Point(matchLoc.x + templ4chnl.cols, matchLoc.y + templ4chnl.rows));
			scaleRect.height -= 24;
			
			//std::this_thread::sleep_for(std::chrono::months(2));
			return fullScale(scaleRect).clone();
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

void Vision::debugWindow() {
	
	if (areaSelected) {
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
	
	ImGui::End();
}

