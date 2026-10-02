#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <algorithm> 

using namespace cv;
using namespace std;


//Given a binary image mask, returns a vector storing the upper edge of the 1 pixels for each vertical column of the image. 
//Upper edge values are made proportional to given bounds.
//The image mask is also modified to show the vector.
std::vector<double> wavify(Mat &img, double lBound = -1.0, double uBound = 1.0)
{
	int darkHitCount;
	bool darkened;

	std::vector<double> wave;

	for (int x = 0; x < img.cols; x++)
	{
		darkHitCount = 0;
		darkened = false;
		for (int y = 0; y < img.rows; y++)
		{
			//std::cout << (img.at<uchar>(y, x) < 255) << std::endl;
			if (darkened)
			{
				img.at<uchar>(y, x) = 255;
			}

			if (img.at<uchar>(y, x) < 255)
			{
				darkHitCount++;
			}
			else
			{
				darkHitCount = 0;			
			}
			if (darkHitCount > 10 && !darkened)
			{
				darkened = true;
				wave.push_back((img.rows * uBound / (uBound - lBound) - y) * ((uBound - lBound) / img.rows));

			}

			if (y == img.rows - 1 && !darkened)
			{
				wave.push_back((img.rows * uBound / (uBound - lBound) - y) * ((uBound - lBound) / img.rows));
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
//	int hmin = 0, smin = 0, vmin = 120,
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
//	string path = "Resources/wave2.png";
//	Mat img = imread(path);
//	inRange(img, lower, upper, mask);
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
//}


/////////////////  Webcam  //////////////////////

void main() {

	VideoCapture cap(0);
	Mat img, imgHSV, mask, waved;
	int hmin = 0, smin = 0, vmin = 120,
		hmax = 255, smax = 255, vmax = 255;

	double lowerVoltageBound, upperVoltageBound;

	std::vector<double> wave;

	std::cout << "Enter lower voltage bound:" << std::endl;
	std::cin >> lowerVoltageBound;
	std::cout << "Enter upper voltage bound:" << std::endl;
	std::cin >> upperVoltageBound;



	while (true) {

		cap.read(img);

		cvtColor(img, imgHSV, COLOR_BGR2HSV);

		Scalar lower(hmin, smin, vmin);
		Scalar upper(hmax, smax, vmax);
		inRange(imgHSV, lower, upper, mask);

		wave = wavify(mask, lowerVoltageBound, upperVoltageBound);

		imshow("Image", mask);
		waitKey(1);
	}
}


