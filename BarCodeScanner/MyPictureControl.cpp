// MyPictureControl.cpp : ʵ���ļ�
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



// MyPictureControl ��Ϣ�������




void MyPictureControl::OnDropFiles(HDROP hDropInfo)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	int DropCount = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);//ȡ�ñ��϶��ļ�����Ŀ

	for (int i = 0; i< DropCount; i++)
	{
		WCHAR wcStr[MAX_PATH];
		DragQueryFile(hDropInfo, i, wcStr, MAX_PATH);//�����ҷ�ĵ�i���ļ����ļ���
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
