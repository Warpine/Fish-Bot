#pragma once

#include<Windows.h>
#include<string>
#include<atomic>
enum objType
{
	BOBBER = 0
};
enum HSV
{
	HMIN = 0, HMAX = 1,
	SMIN = 2, SMAX = 3,
	VMIN = 4, VMAX = 5
};
enum Status
{
	STOPPED,
	STARTED,
	LOOKING,
	FOUND,
	CATCH,
	PULL,
	RELEASE,
	FINISHED
};

struct AppState {
private:
	//cv::Mat getMat();

public:
	//numbers
	float f = 0.0f;
	float mainScale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0,0 }, MONITOR_DEFAULTTOPRIMARY));
	int counter = 0;
	int areaRadius = 200;
	int mainWinSize = (int)(500 * mainScale);
	int hostWinSize = (int)(1 * mainScale);

	

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	std::string visionWinName = "Window";

	//keys
	int fihKey = VK_NUMPAD5;
	int stopFih = VK_ESCAPE;

	//bool
	bool showDemoWindow = false;
	bool showAnotherWindow = false;
	bool showMainWindow = true;
	bool showHint = true;
	bool exit = false;
	std::atomic<bool> fihing = false;
	bool areaSelected = false;
	bool debug = false;

	//window flags
	ImGuiWindowFlags flazhoks = 
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;
	
	//arrays or sth
	std::vector<std::vector<int>> objHSV = {
		//hmin, hmax, smin, smax, vmin, vmax
			{0, 14, 80, 207, 124, 255} //BOBBER

	};
	std::string statusMessage = "stopped";

	ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
	cv::Mat fullScale; 
	//пустая матрица для захвата картинки
	cv::Mat img = cv::Mat();
	std::atomic<cv::Rect> boundRect = cv::Rect(0,0,0,0);

	//xyi znaet handles?
	HWND windowDesk = GetDesktopWindow();
	RECT selectedArea = { 0 };

};

struct Vision
{
private:
	
	cv::Mat	imgHSV, imgMask, imgBack;
	std::vector<std::vector<cv::Point>> contours;

	

public: 
	

	
	/*void work(AppState& state) {

		if (!state.areaSelected) {
			selectAreaWithMouse(state);
		}

		getDesktopMat(state);
		getImage(state);
		showImage(state);

	}*/

	void showImage(AppState& state)
	{
		cv::namedWindow(state.visionWinName);
		cv::imshow(state.visionWinName, state.img);
		cv::waitKey(5);
	};

	void selectAreaWithMouse(AppState& state) {

		POINT cursorPos;

		// Ожидаем нажатие Num5 
		while (true) {

			if (GetAsyncKeyState(state.fihKey) & 0x8000) {
				// Ждем отпускания, чтобы избежать повторного срабатывания
				while (GetAsyncKeyState(state.fihKey) & 0x8000) {
					Sleep(10);
				}


				GetCursorPos(&cursorPos);
				ScreenToClient(state.windowDesk, &cursorPos);

				// Вычисляем область вокруг курсора
				state.selectedArea.left = cursorPos.x - state.areaRadius / 2;
				state.selectedArea.top = cursorPos.y - state.areaRadius / 2;
				state.selectedArea.right = cursorPos.x + state.areaRadius / 2;
				state.selectedArea.bottom = cursorPos.y + state.areaRadius / 2;
				break;
			}
			Sleep(10);
		}
		state.areaSelected = true;
	};

	void getDesktopMat(AppState& state) {
		HDC deviceContext = GetDC(state.windowDesk);
		HDC memoryDeviceContext = CreateCompatibleDC(deviceContext);

		HBITMAP bitmap = CreateCompatibleBitmap(deviceContext, state.screenWidth, state.screenHeight);

		SelectObject(memoryDeviceContext, bitmap);

		//copy data into the bitmap
		BitBlt(memoryDeviceContext, 0, 0, state.screenWidth, state.screenHeight, deviceContext, 0, 0, SRCCOPY);

		//specify format by using bitmap info header
		BITMAPINFOHEADER bi;
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = state.screenWidth;
		bi.biHeight = -state.screenHeight;
		bi.biPlanes = 1;
		bi.biBitCount = 32;
		bi.biCompression = BI_RGB;
		bi.biSizeImage = 0; //cause no compression
		bi.biXPelsPerMeter = 1;
		bi.biYPelsPerMeter = 2;
		bi.biClrUsed = 3;
		bi.biClrImportant = 4;

		state.fullScale = cv::Mat(state.screenHeight, state.screenWidth, CV_8UC4);//rgba = 8bit per value

		//copy and transform data
		GetDIBits(memoryDeviceContext, bitmap, 0, state.screenHeight, state.fullScale.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

		DeleteObject(bitmap);
		DeleteDC(memoryDeviceContext);
		ReleaseDC(state.windowDesk, deviceContext);

	};

	void getImage(AppState& state)
	{
		state.img = cropMat(state.fullScale, state.selectedArea);

		cvtColor(state.img, imgHSV, cv::COLOR_BGR2HSV);



		imgMask = getMaskColorBased(imgHSV, imgMask, BOBBER, state.objHSV);

		//find countour
		findContours(imgMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		//drawContours(bobberBack, contours, -1, (0, 255, 0), 3);


		//find rectangle for bobber
		for (size_t i = 0; i < contours.size(); ++i) {
			state.boundRect = cv::boundingRect(contours[i]);
			cv::Rect boundRect = state.boundRect.load();
			if (boundRect.area() > 400)
			{                                                                //&& (state.boundRect.width < 70 || state.boundRect.height < 70)
				cv::rectangle(state.img, boundRect.tl(), boundRect.br(), cv::Scalar(0, 0, 255), 3);
			}

		}
	};

	void destroyImage(AppState& state)
	{
		cv::destroyWindow(state.visionWinName);
		state.areaSelected = false;
	}

private:


	cv::Mat cropMat(const cv::Mat& fullFrame, const RECT& area) {
		int x = max(0, area.left);
		int y = max(0, area.top);
		int width = min(fullFrame.cols - x, area.right - area.left);
		int height = min(fullFrame.rows - y, area.bottom - area.top);

		if (width <= 0 || height <= 0) {
			return cv::Mat(); // Пустая матрица при невалидной области
		}

		return fullFrame(cv::Rect(x, y, width, height)).clone();
	}

	cv::Mat getMaskColorBased(cv::Mat& imgHsv, cv::Mat& imgMask, objType type, std::vector<std::vector<int>>& objHSV)
	{
		cv::Scalar Lower(objHSV[type][HMIN],
			             objHSV[type][SMIN],
			             objHSV[type][VMIN]);

		cv::Scalar Upper(objHSV[type][HMAX],
			             objHSV[type][SMAX],
			             objHSV[type][VMAX]);

		inRange(imgHsv, Lower, Upper, imgMask);

		return imgMask;
	}

	
};