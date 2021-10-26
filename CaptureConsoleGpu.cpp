#include "pch.h"
#include "windows.h"
#include "tchar.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

#include "yolo_v2_class.hpp"    // imported functions from DLL


using namespace std;
using namespace cv;
#include <iostream>

#define OPENCV

Mat hwnd2mat(HWND hwnd, int x, int y, int w, int h) {

	HDC hwindowDC, hwindowCompatibleDC;

	int height, width, srcheight, srcwidth;
	HBITMAP hbwindow;
	Mat src;
	BITMAPINFOHEADER  bi;

	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;    // get the height and width of the screen
	GetClientRect(hwnd, &windowsize);

	srcheight = windowsize.bottom;
	srcwidth = windowsize.right;
	height = windowsize.bottom;  //change this to whatever size you want to resize to
	width = windowsize.right;

	src.create(height, width, CV_8UC4);

	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

																									   // avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);

	Rect roi;
	roi.x = x;
	roi.y = y;
	roi.width = w;
	roi.height = h;

	src = src(roi);

	return src;
}

void print_detection(Mat& mat, bbox_t bbox) {
	Scalar color;
	if (bbox.obj_id == 0) {
		color = Scalar(0, 255, 0);
	}
	else if (bbox.obj_id == 1) {
		color = Scalar(255, 0, 0);
	}
	else {
		color = Scalar(0, 0, 255);
	}
	rectangle(mat, Point(bbox.x, bbox.y), Point(bbox.x + bbox.w, bbox.y + bbox.h), color, 1, 0);
}

void main(int argc, char** argv)
{

	if (argc < 2) {
		fprintf(stderr, "usage: config weights  \n");
		return;
	}

	std::string cfg_s(argv[1]);
	std::string weights_s(argv[2]);

	Detector detector(cfg_s, weights_s);

	HWND desk = GetDesktopWindow();
	LPCWSTR lpwstr = L"Test String";

	bbox_t_container container;

	Mat mat = hwnd2mat(desk, 0, 0, 720, 576);
	
	
	while (desk != 0)
	{
		Mat mat = hwnd2mat(desk, 0, 0, 720, 576);
		waitKey(50); // need after imshow you can change wait time
		std::vector<bbox_t>  bboxes;
		imshow("Capture", mat);
		//waitKey(50000); // need after imshow you can change wait time
		int result = -1;
		try {
			bboxes = detector.detect(mat);
		}
		catch (const std::exception& e) {
			cerr << e.what();
		}
		if (result > 0) { cout << "\n"; }
		for (int i = 0; i < bboxes.size(); i++) {
			if (bboxes[i].prob > 0.5) {
				print_detection(mat, bboxes[i]);
			/*	string setStr = "obj: "
					+ to_string(bboxes[i].obj_id)
					+ ", conf: " + to_string(bboxes[i].prob)
					+ ", x: " + to_string(bboxes[i].x)
					+ ", y: " + to_string(bboxes[i].y)
					+ ", w: " + to_string(bboxes[i].w)
					+ ", h: " + to_string(bboxes[i].h)
					+ "\n";
				cout << setStr;*/
			}
		}
		imshow("Capture", mat);
	}
	
}
