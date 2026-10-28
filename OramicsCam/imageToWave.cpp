#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <algorithm> 

#include "SerialPort.h"
#include <chrono>
#include <thread>

using namespace cv;
using namespace std;


//Given a binary image mask, returns a vector storing the upper edge of the 1 pixels for each vertical column of the image. 
//Upper edge values are made proportional to given bounds.
//The image mask is also modified to show the vector.
std::vector<double> wavify(Mat& satImg, /*Mat& valImg,*/ double lBound = -1.0, double uBound = 1.0)
{
	int darkHitCount;
	bool darkened;

	std::vector<double> wave;

	for (int x = 0; x < satImg.cols; x++)
	{
		darkHitCount = 0;
		darkened = false;
		for (int y = 0; y < satImg.rows; y++)
		{
			//std::cout << (img.at<uchar>(y, x) < 255) << std::endl;


			if (darkened) //|| y < satImg.rows/15)
			{
				satImg.at<uchar>(y, x) = 255;
			}

			if (satImg.at<uchar>(y, x) < 255)// || valImg.at<uchar>(y, x) < 255)
			{
				darkHitCount++;
			}
			else
			{
				darkHitCount = 0;
			}
			if (darkHitCount > 0 && !darkened)
			{
				darkened = true;
				wave.push_back((satImg.rows * uBound / (uBound - lBound) - y) * ((uBound - lBound) / satImg.rows));

			}

			if (y == satImg.rows - 1 && !darkened)
			{
				wave.push_back((satImg.rows * uBound / (uBound - lBound) - y) * ((uBound - lBound) / satImg.rows));
			}

		}


	}
	return wave;
}

/////////////////  Images  //////////////////////
//For testing the wavify function with one image, and printing the generated vector.
//int main() {
//	Mat mask;
//	std::vector<double> wave;
//
//	int hmin = 0, smin = 125, vmin = 0,
//		hmax = 255, smax = 255, vmax = 255;
//	Scalar lower(hmin, smin, vmin);
//	Scalar upper(hmax, smax, vmax);
//
//	double lowerVoltageBound, upperVoltageBound;
//	std::cout << "Enter lower voltage bound:" << std::endl;
//	std::cin >> lowerVoltageBound;
//	std::cout << "Enter upper voltage bound:" << std::endl;
//	std::cin >> upperVoltageBound;
//
//	
//	string path = "Resources/orange.png";
//	Mat img = imread(path);
//	inRange(img, lower, upper, mask);
//	
//	
//
//	wave = wavify(mask, lowerVoltageBound, upperVoltageBound);
//
//	imshow("Image", mask);
//	for (int i = 0; i < wave.size(); i++)
//	{
//		std::cout << wave.at(i) << ", ";
//	}
//	waitKey(0);
//	
//
//}


/////////////////  Webcam  //////////////////////

void main() {

	VideoCapture cap(1);
	Mat img, imgHSV, satMask, valMask, waved;
	int hmin = 0, smin = 0, vmin = 120,
		hmax = 255, smax = 60, vmax = 255;

	double lowerVoltageBound, upperVoltageBound;

	std::vector<double> wave;

	std::cout << "Enter lower voltage bound:" << std::endl;
	std::cin >> lowerVoltageBound;
	std::cout << "Enter upper voltage bound:" << std::endl;
	std::cin >> upperVoltageBound;

	HANDLE esp = openSerial("COM7");   // use your actual COM number
	if (esp == INVALID_HANDLE_VALUE) return;
	std::this_thread::sleep_for(std::chrono::seconds(2));  // ESP32 reboots when the port opens
	auto lastSend = std::chrono::steady_clock::now();

	while (true) {

		cap.read(img);

		cvtColor(img, imgHSV, COLOR_BGR2HSV);

		imshow("Image0", img);

		Scalar lowerSat(hmin, smin, 0);
		Scalar upperSat(hmax, smax, 255);
		inRange(imgHSV, lowerSat, upperSat, satMask);

		Scalar lowerVal(hmin, 0, vmin);
		Scalar upperVal(hmax, 255, vmax);
		inRange(imgHSV, lowerVal, upperVal, valMask);

		imshow("Image1", satMask);

		wave = wavify(satMask, /*valMask,*/ lowerVoltageBound, upperVoltageBound);

		//sendVector(esp, wave);

		if (sendVector(esp, wave)) {
			std::string reply = readReply(esp);
			if (reply.empty()) std::cout << "No reply from ESP32\n";
			else               std::cout << "\nESP32: " << reply;
		}
		imshow("Image2", satMask);
		waitKey(1);
	}
}


