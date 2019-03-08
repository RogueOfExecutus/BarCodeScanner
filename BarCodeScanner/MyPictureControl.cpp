// MyPictureControl.cpp : 实现文件
//

#include "stdafx.h"
#include "BarCodeScanner.h"
#include "MyPictureControl.h"
#include <opencv2/opencv.hpp> 


using namespace cv;
using namespace std;

// MyPictureControl

IMPLEMENT_DYNAMIC(MyPictureControl, CStatic)

MyPictureControl::MyPictureControl()
{

}

MyPictureControl::~MyPictureControl()
{
}


BEGIN_MESSAGE_MAP(MyPictureControl, CStatic)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()



// MyPictureControl 消息处理程序




void MyPictureControl::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int DropCount = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);//取得被拖动文件的数目

	for (int i = 0; i< DropCount; i++)
	{
		WCHAR wcStr[MAX_PATH];
		DragQueryFile(hDropInfo, i, wcStr, MAX_PATH);//获得拖曳的第i个文件的文件名
		USES_CONVERSION;
		Mat J = imread(string(W2A(wcStr)), IMREAD_ANYDEPTH | IMREAD_ANYCOLOR);
		if (J.data)
		{
			GetParent()->SendMessage(WM_UPDATA_UI, 0, (LPARAM)&J);
			break;
		}
	}

	DragFinish(hDropInfo);
	CStatic::OnDropFiles(hDropInfo);
}
