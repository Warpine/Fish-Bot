#pragma once
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include <Windows.h>

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
		STARTED,
		LOOKING,
		FOUND,
		CATCH,
		FINISHED,
		CLEAN,
		WORMS,
		FOOD
	};

	//for debug window display
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> textureSRV = nullptr;
	static inline ImTextureID imguiTexture = NULL;
	std::string statusMessage = "never started";
	const std::string winName = "Debug Window";


	//for getDesktopMat
	HDC deviceContext = nullptr;
	HDC memoryDeviceContext = nullptr;
	HBITMAP bitmap = nullptr;
	BITMAPINFOHEADER bi;
	bool gdiInitialized = false;
	
	//used in getDesktopMat
	cv::Mat fullScale = cv::Mat();
	/////////////////////////


	//in vision class constructor
	Config& config;
	//in vision class constructor
	ID3D11Device* g_pd3dDevice_;
	///////////////////////////
	
	
	//must be reset when status == FINISHED
	static inline INPUT inputCatch = { 0 };
	
	//used in catchProcess
	const int scalePosDOWN = 140;
	//used in catchProcess
	const int scalePosUP = 150;

	//used in initWindow
	bool init = false;
	//used in initWindow
	HWND windowDesk = nullptr;

	//used in selectAreaWithMouse
	::RECT selectedArea = { 0 };
	bool areaSelected = false;

	//for overal logic control
	Status status = STARTED;
	int cyclesCounter = 0;
	int emptyCounter = 0;
	POINT itThatRemember;

	//reset when fishing cycle end
	cv::Rect cropRect = cv::Rect();
	//reset when fishing cycle end
	cv::Rect storedRect = cv::Rect();
	POINT itThatPOINT = POINT();

	//used in getImage
	cv::Mat	img, imgHSV, imgMask;
	std::vector<std::vector<cv::Point>> contours;
	//used in getImage and area comparsion for logic control
	cv::Rect boundRect = cv::Rect();
	//c высоты  800-875   ---- клюнуло  209-450
	//с средней 1500-1700 ---- клюнуло  252-644
	//c низов   2400-2900 ---- клюнуло  546-975
	const int inWaterSize = 640;
	const int inScaleSize = 60;
	

	//add new objects here
	const std::vector<std::vector<int>> objHSV = {
		//hmin, hmax, smin, smax, vmin, vmax
			{0, 14, 80, 207, 124, 255} //BOBBER

	};
	
	//template for matching/////////////////////////////////////////////////////////
	enum matchingEnum {
		SCALE,
		PIE,
		SALAD,
		BAIT,
		LOGS,
		SLOT,
		MAINLOGO,
		BUTTON,
		BUTTON2,
		USEBUTTON,
		YESBUTTON
	};
	std::vector<int> matchingThresholds = {
		10,
		151,
		249,
		151,
		247,
		200,
		30,
		25,
		240,
		224,
		221
	};
    
	const std::vector<cv::Mat> matchingTempl ={   
		cv::imread("src/scale.png", cv::IMREAD_COLOR),
		cv::imread("src/chickenPie.png", cv::IMREAD_COLOR),
		cv::imread("src/salad.png", cv::IMREAD_COLOR),
		cv::imread("src/t1bait.png", cv::IMREAD_COLOR),
		cv::imread("src/logs.png", cv::IMREAD_COLOR),
		cv::imread("src/slot.png", cv::IMREAD_COLOR),
		cv::imread("src/mainLogo.png", cv::IMREAD_COLOR),
		cv::imread("src/enterButton.png", cv::IMREAD_COLOR),
		cv::imread("src/enterButton2.png", cv::IMREAD_COLOR),
		cv::imread("src/useButton.png", cv::IMREAD_COLOR),
		cv::imread("src/yesButton.png", cv::IMREAD_COLOR)

	};

	const std::vector<cv::TemplateMatchModes> matchingModes = {
		cv::TM_SQDIFF_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCORR_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_CCOEFF_NORMED,
		cv::TM_SQDIFF_NORMED,
		cv::TM_SQDIFF,
		cv::TM_CCORR_NORMED,
		cv::TM_CCOEFF,
		cv::TM_CCOEFF_NORMED
	};
	//all this related with matchingEnum
	//////////////////////////////////////////////////////////////////////////
	

	void CaptureFih();
	void stopCapture();
	void pressKeyMouseLeft(int KeyUpMillisec);
	void sendKeyPress(int keyCode);
	
	bool initWindow();
	void getDesktopMat();
	void selectAreaWithMouse(std::atomic<bool>& fihingState); //idk
	cv::Mat cropMat();
	void getMaskColorBased(cv::Mat& imgMask, objType type);
	void getImage();
	
	cv::Mat matchingMethod(matchingEnum type);
	void catchProcess();
	void TextureForDebug();
	void cleanInventory();
	void useBuff(matchingEnum type);

	
public:
	void debugWindow();

	std::atomic<time_t> startTime;
	
	void startCapture(std::atomic<bool>& fihingState, std::atomic<bool>& shouldExit);
	Vision(Config& config, ID3D11Device* g_pd3dDevice);
	~Vision();
	
};

