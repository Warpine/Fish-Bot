#pragma once
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include <Windows.h>
//#include<atomic>
#include"Utility.h"

class Vision
{
private:
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

	HDC deviceContext = nullptr;
	HDC memoryDeviceContext = nullptr;
	HBITMAP bitmap = nullptr;
	BITMAPINFOHEADER bi;
	bool gdiInitialized = false;

	int& areaRadius; //передаётся в конструктор
	int& fihKey;
	int& stopFih;

	HWND windowDesk = nullptr;
	cv::Mat img = cv::Mat();
	cv::Mat fullScale = cv::Mat();
	bool init = false;
	bool areaSelected = false;
	Status status = STOPPED;
	cv::Point bestMatch = cv::Point(0);

	//вот эти два сбрасываются при выходе из цикла
	cv::Rect cropRect = cv::Rect();
	cv::Rect scaleRect = cv::Rect(); 

	cv::Mat	imgHSV, imgMask;
	std::vector<std::vector<cv::Point>> contours;
	::RECT selectedArea = { 0 };
	cv::Rect boundRect = cv::Rect();

	//add new objects here
	const std::vector<std::vector<int>> objHSV = {
		//hmin, hmax, smin, smax, vmin, vmax
			{0, 14, 80, 207, 124, 255} //BOBBER

	};

	const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	const int inWaterSize = 400;
	const int inScaleSize = 20;
	const double threshold_value = 0.8;
	const cv::Mat templ = cv::imread("E:/IT/repos/imguiTest/src/scale.png", cv::IMREAD_COLOR);
	cv::Mat templ4chnl;

	//mouse related
	void CaptureFih();
	void stopCapture();
	void pressKeyMouseLeft(int KeyUpMillisec);

	//vision related
	bool initWindow();
	void getDesktopMat();
	void selectAreaWithMouse(std::atomic<bool>& fihingState); //idk
	cv::Mat cropMat();
	void getMaskColorBased(cv::Mat& imgMask);
	void getImage();
	void showImage();
	cv::Mat matchingMethod();
	void catchProcess();

public:
	static inline std::string statusMessage = "never started";
	std::atomic<time_t> timeNow;
	const std::string winName = "Debug Window";
	void startCapture(std::atomic<bool>& fihingState, std::atomic<bool>& shouldExit);
	Vision(int& areaRadius, int& fihkey, int& stopfih);
	~Vision();
};

