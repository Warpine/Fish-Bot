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

	//передаются в конструктор
	Config& config;
	/*int& areaRadius; 
	int& fihKey;
	int& stopFih;*/
	ID3D11Device* g_pd3dDevice_;
	///////////////////////////
	
	HWND windowDesk = nullptr;
	//cv::Mat img = cv::Mat();
	cv::Mat fullScale = cv::Mat();
	bool init = false;
	bool areaSelected = false;
	Status status = STOPPED;
	cv::Point bestMatch = cv::Point(0);

	//вот эти два сбрасываются при выходе из мейн цикла
	cv::Rect cropRect = cv::Rect();
	cv::Rect scaleRect = cv::Rect(); 

	cv::Mat img = cv::Mat();
	//cv::Mat imgShow;
	cv::Mat	imgHSV, imgMask;
	std::vector<std::vector<cv::Point>> contours;
	::RECT selectedArea = { 0 };
	cv::Rect boundRect = cv::Rect();

	
	//add new objects here
	const std::vector<std::vector<int>> objHSV = {
		//hmin, hmax, smin, smax, vmin, vmax
			{0, 14, 80, 207, 124, 255} //BOBBER

	};
	//c высоты  550-800  ---- клюнуло  143-400
	//с средней 950-1200 ---- клюнуло  208-450
	//c низов   900-1200 ---- клюнуло  165-630
	const int inWaterSize = 530; 
	const int inScaleSize = 20;
	//const double threshold_value = 0.8;
	const cv::Mat templ = cv::imread("E:/IT/repos/imguiTest/src/scale.png", cv::IMREAD_COLOR);
	cv::Mat templ4chnl;

	
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
	
public:
	void debugWindow();

	std::atomic<time_t> timeNow;
	const std::string winName = "Debug Window";
	void startCapture(std::atomic<bool>& fihingState, std::atomic<bool>& shouldExit);
	Vision(Config& config, ID3D11Device* g_pd3dDevice);
	~Vision();
};

