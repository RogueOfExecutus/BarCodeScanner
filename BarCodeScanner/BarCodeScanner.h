
// BarCodeScanner.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CBarCodeScannerApp: 
// �йش����ʵ�֣������ BarCodeScanner.cpp
//

class CBarCodeScannerApp : public CWinApp
{
public:
	CBarCodeScannerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CBarCodeScannerApp theApp;