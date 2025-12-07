#pragma once
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include <Windows.h>
//#include<atomic>
//#include"Utility.h"
#include <wrl/client.h>
#include <d3d11.h>

#include"Config.h"

 //mb need to create globalVar header
const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

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

	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> textureSRV = nullptr;
	static inline ImTextureID imguiTexture = NULL;
	
	std::string statusMessage = "never started";

	HDC deviceContext = nullptr;
	HDC memoryDeviceContext = nullptr;
	HBITMAP bitmap = nullptr;
	BITMAPINFOHEADER bi;
	bool gdiInitialized = false;

	//in vision class constructor
	Config& config;
	//in vision class constructor
	ID3D11Device* g_pd3dDevice_;
	///////////////////////////
	
	
	//must be reset when status == FINISHED
	static inline INPUT inputCatch = { 0 };
	
	//used in catchProcess
	const int scalePosDOWN = 70;
	//used in catchProcess
	const int scalePosUP = 140;

	//used in initWindow
	bool init = false;
	//used in initWindow
	HWND windowDesk = nullptr;

	//used in getDesktopMat
	cv::Mat fullScale = cv::Mat();
	//used in selectAreaWithMouse
	::RECT selectedArea = { 0 };
	
	bool areaSelected = false;

	//for overal logic control
	Status status = STOPPED;

	cv::Point bestMatch = cv::Point(0);

	//reset when fishing cycle end
	cv::Rect cropRect = cv::Rect();
	//reset when fishing cycle end
	cv::Rect scaleRect = cv::Rect(); 

	//used in getImage
	cv::Mat	img, imgHSV, imgMask;
	std::vector<std::vector<cv::Point>> contours;
	//used in getImage and area comparsion for logic control
	cv::Rect boundRect = cv::Rect();
	//c высоты  800-875   ---- клюнуло  209-450
	//с средней 1500-1700 ---- клюнуло  252-644
	//c низов   2400-2900 ---- клюнуло  546-975
	const int inWaterSize = 470;
	const int inScaleSize = 80;
	
	//add new objects here
	const std::vector<std::vector<int>> objHSV = {
		//hmin, hmax, smin, smax, vmin, vmax
			{0, 14, 80, 207, 124, 255} //BOBBER

	};
	
	//template for matching
	const cv::Mat templ = cv::imread("E:/IT/repos/imguiTest/src/scale.png", cv::IMREAD_COLOR);
	cv::Mat templ4chnl;

	const std::string winName = "Debug Window";

	
	void CaptureFih();
	void stopCapture();
	void pressKeyMouseLeft(int KeyUpMillisec);

	
	bool initWindow();
	void getDesktopMat();
	void selectAreaWithMouse(std::atomic<bool>& fihingState); //idk
	cv::Mat cropMat();
	void getMaskColorBased(cv::Mat& imgMask, objType type);
	void getImage();
	
	cv::Mat matchingMethod();
	void catchProcess();
	void TextureForDebug();

	~Vision();
public:
	void debugWindow();

	std::atomic<time_t> timeNow;
	
	void startCapture(std::atomic<bool>& fihingState, std::atomic<bool>& shouldExit);
	Vision(Config& config, ID3D11Device* g_pd3dDevice);
	
};

