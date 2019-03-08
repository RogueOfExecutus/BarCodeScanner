// BarCodeScannerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BarCodeScanner.h"
#include "BarCodeScannerDlg.h"
#include "afxdialogex.h"
#include "pylon/PylonIncludes.h"
#include <condition_variable>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <opencv2/opencv.hpp> 
#include "opencv2/highgui/highgui_c.h"
#include "ImageProcessing.h"
#include "SettingDialog.h"
#include "StandardDlg.h"
#include <iostream>
#include <fstream>  //ofstream类的头文件
#include <io.h>
#include <direct.h>
#include "afxwin.h"
#include "afxdtctl.h"

using namespace cv; 
using namespace std;
using namespace Pylon;
using namespace GenApi;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void on_mouse_l(int mouse_event, int x, int y, int flags, void *ustc);

void TestThreshold(int, void*);
void TestCanny(int, void*);
void TestAdativeThreshold(int, void*);

int thresh = 40;
int threshLow = 50;
int threshHigh = 180;
int kernel_size = 3;
int offset_C = 10;

CBarCodeScannerDlg *thisDlg;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
private:
	CEdit saveDays;
	CDateTimeCtrl timePicker;
	configForClear clear;
	LPCTSTR parameter;
	void initConfig();
	bool writeConfig();
	CComboBox language_combo;
	bool changeLanguage = false;
public:
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	bool GetChangeLanguage();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DAYS, saveDays);
	DDX_Control(pDX, IDC_TIMEPICKER, timePicker);
	DDX_Control(pDX, IDC_COMBO_LANGUAGE, language_combo);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
END_MESSAGE_MAP()



void CAboutDlg::initConfig()
{
	CString temp;
	temp.Format(_T("%d"), clear.days);
	saveDays.SetWindowTextW(temp);
	temp.Format(_T("%d:%d:00"), clear.hours, clear.minutes);
	COleVariant vtime(temp);
	COleDateTime time1 = vtime;
	timePicker.SetTime(time1);
	temp.LoadStringW(IDS_STRING_CHINESE);
	language_combo.InsertString(0, temp);
	temp.LoadStringW(IDS_STRING_ENGLISH);
	language_combo.InsertString(1, temp);
	language_combo.SetCurSel(clear.lang);
}

bool CAboutDlg::writeConfig()
{
	try
	{
		CString temp;
		saveDays.GetWindowTextW(temp);
		clear.days = _ttoi(temp);
		COleDateTime time1;
		timePicker.GetTime(time1);
		clear.hours = time1.GetHour();
		clear.minutes = time1.GetMinute();
		if (clear.lang != language_combo.GetCurSel())
		{
			clear.lang = language_combo.GetCurSel();
			changeLanguage = true;
		}
		WritePrivateProfileStruct(_T("project"), _T("clearConfig"), &clear, sizeof(configForClear), parameter);
	}
	catch (...)
	{
		return false;
	}
	return true;
}

void CAboutDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	if (!writeConfig())
	{
		str.LoadStringW(IDS_STRING_ERR_CONFIG_TIPS);
		MessageBox(str);
		return;
	}
	if (changeLanguage)
	{
		str.LoadStringW(IDS_STRING_REBOOT_TIPS);
		if (MessageBox(str, _T("change language"), MB_ICONINFORMATION | MB_YESNO) == IDNO)
			changeLanguage = false;
	}
	CDialogEx::OnOK();
}


BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	parameter = _T(".\\Parameter.ini");
	// TODO:  在此添加额外的初始化
	CFileFind finder;   //查找是否存在ini文件，若不存在，则生成一个新的默认设置的ini文件，这样就保证了我们更改后的设置每次都可用   
	if (finder.FindFile(parameter))
		GetPrivateProfileStruct(_T("project"), _T("clearConfig"), &clear, sizeof(configForClear), parameter);
	initConfig();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

bool CAboutDlg::GetChangeLanguage()
{
	return changeLanguage;
}

// CBarCodeScannerDlg 对话框



CBarCodeScannerDlg::CBarCodeScannerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_BARCODESCANNER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	userLevel = 0;
	omronFlag = 0;
	imageResType = 0;
}

void CBarCodeScannerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CAMERA, cameraList);
	DDX_Control(pDX, IDC_BUTTON_CODE_STANDARD, codeStandard);
	DDX_Control(pDX, IDC_BUTTON_CONFIG, configButton);
	DDX_Control(pDX, IDC_BUTTON_MANUAL_GRAB, manualGrab);
	DDX_Control(pDX, IDC_BUTTON_START, startOrStop);
	DDX_Control(pDX, IDC_BUTTON_TEST, testButton);
	DDX_Control(pDX, IDC_OMRON_PLC, omronPlc);
	DDX_Control(pDX, IDC_MSG_SHOW, showMSG);
	DDX_Control(pDX, IDC_BUTTON_ABOUT, aboutBtn);
	DDX_Control(pDX, IDC_BUTTON_LOG, logBtn);
	DDX_Control(pDX, IDC_IMAGE_SHOW, imageShowWindow);
	DDX_Control(pDX, IDC_STATIC_ONE, staticOne);
	DDX_Control(pDX, IDC_STATIC_TWO, staticTwo);
	DDX_Control(pDX, IDC_STATIC_THREE, staticThree);
	DDX_Control(pDX, IDC_STATIC_FOUR, staticFour);
	DDX_Control(pDX, IDC_STATIC_FIVE, staticFive);
	DDX_Control(pDX, IDC_STATIC_SIX, staticSix);
	DDX_Control(pDX, IDC_ALL, countAll);
	DDX_Control(pDX, IDC_CODE_NG, countNG);
	DDX_Control(pDX, IDC_CODE_OK, countOK);
	DDX_Control(pDX, IDC_VERSION_NG, countVersionNG);
	DDX_Control(pDX, IDC_YIELD, countYield);
	DDX_Control(pDX, IDC_COUNT_RESET, resetCounting);
	DDX_Control(pDX, IDC_STATIC_EIGHT, staticEight);
	DDX_Control(pDX, IDC_STATIC_SEVEN, staticSeven);
	DDX_Control(pDX, IDC_SIZE_NG, sizeNG);
	DDX_Control(pDX, IDC_POSITION_NG, positionNG);
	DDX_Control(pDX, IDC_ANGLE_NG, angleNG);
	DDX_Control(pDX, IDC_OTHER_NG, otherNG);
	DDX_Control(pDX, IDC_STATIC_NINE, staticNine);
	DDX_Control(pDX, IDC_STATIC_TEN, staticTen);
}

BEGIN_MESSAGE_MAP(CBarCodeScannerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CBarCodeScannerDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG, &CBarCodeScannerDlg::OnBnClickedButtonConfig)
	ON_BN_CLICKED(IDC_BUTTON_CODE_STANDARD, &CBarCodeScannerDlg::OnBnClickedButtonCodeStandard)
	ON_BN_CLICKED(IDC_BUTTON_LOG, &CBarCodeScannerDlg::OnBnClickedButtonLog)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CBarCodeScannerDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON_ABOUT, &CBarCodeScannerDlg::OnBnClickedButtonAbout)
	ON_BN_CLICKED(IDC_BUTTON_MANUAL_GRAB, &CBarCodeScannerDlg::OnBnClickedButtonManualGrab)
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_COPY_COORDINATE, &CBarCodeScannerDlg::OnCopyCoordinate)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_COUNT_RESET, &CBarCodeScannerDlg::OnBnClickedCountReset)
	ON_COMMAND(ID_TEST_THRESHOLD, &CBarCodeScannerDlg::OnTestThreshold)
	ON_MESSAGE(WM_UPDATA_UI, &CBarCodeScannerDlg::OnUpdataUi)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_COMMAND(ID_TEST_CANNY, &CBarCodeScannerDlg::OnTestCanny)
	ON_COMMAND(ID_TEST_ADAPTIVE_THRESHOLD, &CBarCodeScannerDlg::OnTestAdaptiveThreshold)
	ON_COMMAND(ID_CUT_MAT, &CBarCodeScannerDlg::OnCutMat)
	ON_COMMAND(ID_SAVE_IMAGE, &CBarCodeScannerDlg::OnSaveImage)
	ON_COMMAND(ID_ROTATE_90, &CBarCodeScannerDlg::OnRotate90)
	ON_COMMAND(ID_ROTATE_270, &CBarCodeScannerDlg::OnRotate270)
	ON_COMMAND(ID_ROTATE_180, &CBarCodeScannerDlg::OnRotate180)
END_MESSAGE_MAP()


// CBarCodeScannerDlg 消息处理程序

BOOL CBarCodeScannerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	checkTrigger = _T("@00RD0970000159*\r");
	resetTrigger = _T("@00WD0970000000005D*\r");
	//sendReset = _T("@00WD097100005C*\r");
	sendOK = _T("@00WD097100015D*\r");
	sendNG = _T("@00WD097100025E*\r");
	versionNG = _T("@00WD0981000253*\r");
	omronFlag = 0;

	red = Scalar(0, 0, 255);
	green = Scalar(0, 255, 0);

	isCtrl = false;

	imageParameter = _T(".\\Parameter.ini");

	thisDlg = this;

	imageWindow = "image";
	codeWindow = "BarCodeImage";

	//创建处理图片窗口
	namedWindow(imageWindow, WINDOW_NORMAL);

	setMouseCallback(imageWindow, on_mouse_l, 0);//调用回调函数 

	CRect rect_s;
	imageShowWindow.GetClientRect(rect_s);
	resizeWindow(imageWindow, rect_s.Width(), rect_s.Height());

	HWND hWnd_s = (HWND)cvGetWindowHandle(imageWindow.c_str());
	HWND hParent_s = ::GetParent(hWnd_s);
	::SetParent(hWnd_s, imageShowWindow.m_hWnd);
	::ShowWindow(hParent_s, SW_HIDE);

	if(!userLevel)
		HideForLevel();
	checkAppFile();
	RecordSize();
	initConfig();
	initLog();
	initSurf();
	GetPrivateProfileStruct(_T("project"), _T("count"), &countRecord, sizeof(configForCount), imageParameter);
	updateCounting();

	// 定时删除图片与日志
	SetTimer(1, 1000 * 60 * 10, NULL);

	thread ths(&CBarCodeScannerDlg::SaveImageThread, this);
	ths.detach();
	if (!cameraRunning)
	{
		cameraRunning = true;
		thread thc(&CBarCodeScannerDlg::CameraThread, this);
		thc.detach();
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CBarCodeScannerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CBarCodeScannerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CBarCodeScannerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// 启动和暂停相机工作线程
void CBarCodeScannerDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码
	if (testingImage)
		return;
	CString tempStr;
	initConfig();
	if (cameraRunning)
	{
		if (cameraIsOpen) 
		{
			cameraIsOpen = false;
			openCamera.notify_all();
			ShowForStop();
			omronPlc.put_PortOpen(FALSE);
			tempStr.LoadStringW(IDS_STRING_STOP_RUNNING);
			AddShowMSG(tempStr);
			if(projectConfig.isDebug)
				destroyWindow(codeWindow);
			WritePrivateProfileStruct(_T("project"), _T("count"), &countRecord, sizeof(configForCount), imageParameter);
		}
		else
		{
			if (!openMSComm())
			{
				tempStr.LoadStringW(IDS_STRING_OPEN_COM_FAILED);
				MessageBox(tempStr);
				return;
			}
			if (cameraList.GetCurSel() == -1)
			{
				tempStr.LoadStringW(IDS_STRING_SELECT_CAMERA_TIPS);
				MessageBox(tempStr);
				return;
			}
			cameraIsOpen = true;
			thread tho(&CBarCodeScannerDlg::OmronThread, this);
			tho.detach();
			openCamera.notify_all();
			HideForStart();
			tempStr.LoadStringW(IDS_STRING_START_RUNNING);
			AddShowMSG(tempStr);
			if (projectConfig.isDebug)
				namedWindow(codeWindow, WINDOW_AUTOSIZE);
		}
	}
	else
	{
		tempStr.LoadStringW(IDS_STRING_CAMERA_THREAD_ERR);
		MessageBox(tempStr);
	}
}

// 图像处理参数页面
void CBarCodeScannerDlg::OnBnClickedButtonConfig()
{
	// TODO: 在此添加控件通知处理程序代码
	if (testingImage)
		return;
	CString str;
	str.LoadStringW(IDS_STRING_OPEN_SETTING_DLG);
	AddShowMSG(str);
	SettingDialog *setDlg = new SettingDialog(this);
	setDlg->Create(IDD_SET_DIALOG, this);
	setDlg->ShowWindow(SW_SHOW);
	configButton.EnableWindow(false);
}

// 二维码规格参数
void CBarCodeScannerDlg::OnBnClickedButtonCodeStandard()
{
	// TODO: 在此添加控件通知处理程序代码
	if (testingImage)
		return;
	CString str;
	str.LoadStringW(IDS_STRING_OPEN_CODE_STANDARD_DLG);
	AddShowMSG(str);
	StandardDlg sDlg;
	sDlg.DoModal();
	readConfig();
	initSurf();
}

// 日志文件
void CBarCodeScannerDlg::OnBnClickedButtonLog()
{
	// TODO: 在此添加控件通知处理程序代码
	ShellExecute(NULL, _T("open"), _T("log"), NULL, NULL, SW_SHOW);
}

// 测试本地图片
void CBarCodeScannerDlg::OnBnClickedButtonTest()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	if (testingImage)
		testingImage = false;
	else
	{
		str.LoadStringW(IDS_STRING_SELECT_TEST_TYPE);
		int result = MessageBox(str, _T("test image"), MB_ICONINFORMATION | MB_YESNO);
		if (result == IDYES)
		{
			String img = OpenImageFile();
			if (!img.empty())
			{
				str.LoadStringW(IDS_STRING_TEST_ONE);
				AddShowMSG(str);
				Mat J = imread(img, IMREAD_ANYDEPTH | IMREAD_ANYCOLOR);
				if (J.data)
				{
					image_j = J;
					//J.copyTo(image_j);
					imageResType = CodeJudgement(J, image_r, newBarcode);
					imshow(imageWindow, image_r);
					savingImage = true;
					saveImage.notify_all();
				}
			}
		}
		else if (result == IDNO)
		{
			CString folderPath = PickImageFile();
			if (folderPath.IsEmpty())
				return;
			recursionImageFolder(folderPath, allImagePath);
			thread thc(&CBarCodeScannerDlg::multipleTestThread, this);
			thc.detach();
		}
	}
}

// 关于软件
void CBarCodeScannerDlg::OnBnClickedButtonAbout()
{
	// TODO: 在此添加控件通知处理程序代码
	if (testingImage)
		return;
	CString str;
	str.LoadStringW(IDS_STRING_OPEN_ABOUT_DIALOG);
	AddShowMSG(str);
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
	if(aboutDlg.GetChangeLanguage())
		PostQuitMessage(0);
	checkAppFile();
	RedrawAll();
}

// 手动触发
void CBarCodeScannerDlg::OnBnClickedButtonManualGrab()
{
	// TODO: 在此添加控件通知处理程序代码
	if (testingImage)
		return;
	if (cameraIsOpen)
	{
		grabImage.notify_all();
	}
}

// 打开图片
string CBarCodeScannerDlg::OpenImageFile()
{
	CString fileType;
	fileType.LoadStringW(IDS_STRING_OPEN_IMAGE_SELECTER);
	CFileDialog findFileDlg(
		TRUE,  // TRUE是创建打开文件对话框，FALSE则创建的是保存文件对话框
		_T(".png"),  // 默认的打开文件的类型
		NULL,  // 默认打开的文件名
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,  // 打开只读文件
		fileType  // 所有可以打开的文件类型
	);
	//图像名称
	string imgFile;
	if (IDOK == findFileDlg.DoModal())
	{
		imgFile = (LPCSTR)(CStringA)(findFileDlg.GetPathName());
	}
	return imgFile;
}

CString CBarCodeScannerDlg::PickImageFile()
{
	CFolderPickerDialog findFileDlg(NULL, 0, this, 0);
	CString imgFile;
	if (IDOK == findFileDlg.DoModal())
	{
		imgFile = findFileDlg.GetPathName();
	}
	return imgFile;
}

void CBarCodeScannerDlg::recursionImageFolder(CString folder, vector<CString>& paths)
{
	CFileFind finder;
	CString folderPath;
	folderPath.Format(_T("%s\\*.*"), folder);
	BOOL bWorking = finder.FindFile(folderPath);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsSystem() || finder.IsDots())
			continue;
		CString pathname = finder.GetFilePath();
		if (finder.IsDirectory())//处理文件夹
			recursionImageFolder(pathname, paths); //递归文件夹
		else //处理文件
			paths.push_back(pathname);
	}
	finder.Close();
	//return paths;
}

// log初始化
void CBarCodeScannerDlg::initLog()
{
	switch (projectConfig.logLevel)
	{
	case 0:
		myLogger = &MyLogger("log", "log\\log.log", log4cplus::TRACE_LOG_LEVEL);
		break;
	case 1:
		myLogger = &MyLogger("log", "log\\log.log", log4cplus::DEBUG_LOG_LEVEL);
		break;
	case 2:
		myLogger = &MyLogger("log", "log\\log.log", log4cplus::INFO_LOG_LEVEL);
		break;
	case 3:
		myLogger = &MyLogger("log", "log\\log.log", log4cplus::WARN_LOG_LEVEL);
		break;
	case 4:
		myLogger = &MyLogger("log", "log\\log.log", log4cplus::ERROR_LOG_LEVEL);
		break;
	case 5:
		myLogger = &MyLogger("log", "log\\log.log", log4cplus::FATAL_LOG_LEVEL);
		break;
	default:
		break;
	}
}

// 参数初始化
void CBarCodeScannerDlg::initConfig()
{
	//写入程序数据库连接ini文件信息，默认设置如下   
	CFileFind finder;   //查找是否存在ini文件，若不存在，则生成一个新的默认设置的ini文件，这样就保证了我们更改后的设置每次都可用   
	if (finder.FindFile(imageParameter))
	{
		readConfig();
	}
	else
	{
		RLLineConfig.rect = Rect(0, 0, 100, 100);
		RLLineConfig.canny_size = 3;
		RLLineConfig.line_direction = true;
		RLLineConfig.maxLineGap = 20;
		RLLineConfig.minLinLength = RLLineConfig.rect.height * 2 / 3;
		RLLineConfig.threshold = 40;
		RLLineConfig.threshold1 = 80;
		RLLineConfig.threshold2 = 200;
		RLLineConfig.same = true;
		RLLineConfig.offset = 0;
		RLLineConfig.adaptiveKernel = 3;
		RLLineConfig.adaptiveOffset = 10;
		RLLineConfig.method = 0;

		UDLineConfig.rect = Rect(0, 0, 100, 100);
		UDLineConfig.canny_size = 3;
		UDLineConfig.line_direction = false;
		UDLineConfig.maxLineGap = 20;
		UDLineConfig.minLinLength = UDLineConfig.rect.width * 2 / 3;
		UDLineConfig.threshold = 35;
		UDLineConfig.threshold1 = 80;
		UDLineConfig.threshold2 = 200;
		UDLineConfig.same = true;
		UDLineConfig.offset = 0;
		UDLineConfig.adaptiveKernel = 3;
		UDLineConfig.adaptiveOffset = 10;
		UDLineConfig.method = 0;

		projectConfig.codeType = 0;
		projectConfig.rect = Rect(0, 0, 100, 100);
		projectConfig.pixel_scale_x = 37.0;
		projectConfig.pixel_scale_y = 37.0;
		projectConfig.corner = 3;
		projectConfig.isDebug = false;
		projectConfig.pencilSize = 3;
		projectConfig.fontSize = 3;
		projectConfig.recLen = 1;
		projectConfig.imageWidth = 1080;
		projectConfig.imageHeight = 1920;
		projectConfig.logLevel = 2;

		codeConfig.blackOrWhite = 0;
		codeConfig.ErosionSize = 1;
		codeConfig.ErosionTimes = 0;
		codeConfig.MorphologyExSize = 1;
		codeConfig.MorphologyExTimes = 1;
		codeConfig.threshold = 100;
		codeConfig.BlurSize = 3;
		codeConfig.cycleTimes = 4;
		codeConfig.cycleSize = 5;
		codeConfig.isSurf = false;

		standardConfig.highAngle = 93.0;
		standardConfig.lowAngle = 87.0;
		standardConfig.highCodeWidth = 5.5;
		standardConfig.lowCodeWidth = 4.5;
		standardConfig.highCodeHeight = 5.5;
		standardConfig.lowCodeHeight = 4.5;
		standardConfig.highWidth = 18;
		standardConfig.lowWidth = 16;
		standardConfig.highHeight = 32;
		standardConfig.lowHeight = 30;

		countRecord.codeNG = 0;
		countRecord.sizeNG = 0;
		countRecord.positionNG = 0;
		countRecord.angleNG = 0;
		countRecord.otherNG = 0;
		countRecord.All = 0;
		countRecord.VersionNG = 0;

		clearConfig.days = 7;
		clearConfig.hours = 19;
		clearConfig.minutes = 40;

		offsetConfig.codeHeightOffset = 0.0;
		offsetConfig.codeWidthOffset = 0.0;
		offsetConfig.heightOffset = 0.0;
		offsetConfig.widthOffset = 0.0;
		offsetConfig.codeOffset = 10;

		versionConfig.enable = false;
		versionConfig.hasTemplate = false;
		versionConfig.rect = Rect(0, 0, 100, 100);
		versionConfig.rotateAngle = 0;
		versionConfig.strLen = 0;

		versionSTR = _T("");

		cameraName = _T("1");
		portName = _T("COM1");

		WritePrivateProfileStruct(_T("line"), _T("RLLineConfig"), &RLLineConfig, sizeof(configForLine), imageParameter);
		WritePrivateProfileStruct(_T("line"), _T("UDLineConfig"), &UDLineConfig, sizeof(configForLine), imageParameter);
		WritePrivateProfileStruct(_T("code"), _T("codeConfig"), &codeConfig, sizeof(configForCode), imageParameter);
		WritePrivateProfileStruct(_T("project"), _T("projectConfig"), &projectConfig, sizeof(configForProject), imageParameter);
		WritePrivateProfileStruct(_T("project"), _T("codeStandardConfig"), &standardConfig, sizeof(codeStandardConfig), imageParameter);
		WritePrivateProfileStruct(_T("project"), _T("count"), &countRecord, sizeof(configForCount), imageParameter);
		WritePrivateProfileStruct(_T("project"), _T("clearConfig"), &clearConfig, sizeof(configForClear), imageParameter);
		WritePrivateProfileStruct(_T("project"), _T("offsetConfig"), &offsetConfig, sizeof(configForOffset), imageParameter);
		WritePrivateProfileStruct(_T("project"), _T("versionConfig"), &versionConfig, sizeof(configForVersion), imageParameter);
		WritePrivateProfileString(_T("camera"), _T("cameraName"), cameraName, imageParameter);
		WritePrivateProfileString(_T("communication"), _T("port"), portName, imageParameter);
		WritePrivateProfileString(_T("version"), _T("string"), versionSTR, imageParameter);
	}

	if (versionConfig.enable)
	{
		CString temp;
		if (!versionConfig.hasTemplate)
		{
			temp.LoadStringW(IDS_STRING_VERSION_NO_LEARN);
			AddShowMSG(temp);
			versionConfig.enable = false;
		}
		else if (!imageProcessing.initModel())
		{
			temp.LoadStringW(IDS_STRING_VERSION_INIT_ERROR);
			AddShowMSG(temp);
			versionConfig.enable = false;
		}
		else
		{
			versions = SettingDialog::SplitCString(versionSTR, _T(","));
			versionConfig.strLen = versions[0].GetLength();
		}
	}
}

void CBarCodeScannerDlg::initSurf()
{
	// 初始化SURF寻找QR-Code
	if (codeConfig.isSurf)
	{
		CString tips;
		if (imageProcessing.PrepareForSURF("QRCodeImage\\qrcode.png", codeConfig))
		{
			tips.LoadStringW(IDS_STRING_INIT_SURF_SUCCEED);
			AddShowMSG(tips);
		}
		else
		{
			codeConfig.isSurf = false;
			tips.LoadStringW(IDS_STRING_INIT_SURF_FAILED);
			AddShowMSG(tips);
		}
	}
}

void CBarCodeScannerDlg::readConfig()
{
	GetPrivateProfileStruct(_T("line"), _T("RLLineConfig"), &RLLineConfig, sizeof(configForLine), imageParameter);
	GetPrivateProfileStruct(_T("line"), _T("UDLineConfig"), &UDLineConfig, sizeof(configForLine), imageParameter);
	GetPrivateProfileStruct(_T("code"), _T("codeConfig"), &codeConfig, sizeof(configForCode), imageParameter);
	GetPrivateProfileStruct(_T("project"), _T("projectConfig"), &projectConfig, sizeof(configForProject), imageParameter);
	GetPrivateProfileStruct(_T("project"), _T("codeStandardConfig"), &standardConfig, sizeof(codeStandardConfig), imageParameter);
	GetPrivateProfileStruct(_T("project"), _T("clearConfig"), &clearConfig, sizeof(configForClear), imageParameter);
	GetPrivateProfileStruct(_T("project"), _T("offsetConfig"), &offsetConfig, sizeof(configForOffset), imageParameter);
	GetPrivateProfileStruct(_T("project"), _T("versionConfig"), &versionConfig, sizeof(configForVersion), imageParameter);
	GetPrivateProfileString(_T("camera"), _T("cameraName"), _T("camera"), cameraName.GetBuffer(20), 20, imageParameter);
	GetPrivateProfileString(_T("communication"), _T("port"), _T("COM1"), portName.GetBuffer(7), 7, imageParameter);
	GetPrivateProfileString(_T("version"), _T("string"), _T(""), versionSTR.GetBuffer(50), 50, imageParameter);
	cameraName.ReleaseBuffer();
	portName.ReleaseBuffer();
	versionSTR.ReleaseBuffer();
	RLLineConfig.minLinLength = RLLineConfig.rect.height * 2 / 3;
	UDLineConfig.minLinLength = UDLineConfig.rect.width * 2 / 3;
}

// 二维码判定
int CBarCodeScannerDlg::CodeJudgement(const Mat& image, Mat& J, string& data)
{
	//计时
	DWORD t;
	if (projectConfig.isDebug)
		t = GetTickCount();
	CString tempStr;
	CString tempFormat;
	int res = 0;
	if (!image.data)
	{
		tempFormat.LoadStringW(IDS_STRING_ERR_IMAGE);
		AddShowMSG(tempFormat);
		res = 10;
	}
	else
	{
		J = image.clone();
		if (J.channels() == 1)
			cvtColor(J, J, COLOR_GRAY2BGR);

		//版本识别
		if (versionConfig.enable)
		{
			string resChar;
			imageProcessing.VersionCheck(J(versionConfig.rect), versionConfig, resChar);
			bool f = false;
			res = 5;
			for (size_t i = 0; i < versions.size(); i++)
			{
				if ((LPCSTR)(CStringA)versions[i] == resChar)
				{
					f = true;
					res = 0;
					break;
				}
			}
			putText(J, "version:"+resChar, Point(projectConfig.imageWidth / 8, projectConfig.imageHeight / 8 + 600),
				FONT_HERSHEY_COMPLEX, 2, f ? green : red, projectConfig.fontSize, 8);
			if (!f)
				return res;
		}

		RotatedRect rotatedRect;
		Mat code(image, projectConfig.rect);
		rectangle(J, projectConfig.rect, Scalar(0, 0, 255), projectConfig.pencilSize);
		Mat temp = code.clone();

		Point2f fourPoint2f[4];
		float angle;
		vector<Point2f> codePoint;

		if (projectConfig.codeType != 0 && projectConfig.codeType != 1)
			res = 11;
		else
		{
			Mat ScanRect;
			bool scanResult;
			//zxing
			for (int i = 0; i < codeConfig.cycleTimes; i++)
			{
				scanResult = imageProcessing.ScanBarCodeForZxing(temp, ScanRect, projectConfig.codeType, data, codePoint, codeConfig, i);
				if (scanResult)
				{
					if (projectConfig.isDebug)
					{
						tempFormat.LoadStringW(IDS_STRING_DECODE_TIMES);
						tempStr.Format(tempFormat, i);
						AddShowMSG(tempStr);
					}
					break;
				}
			}
			if (!scanResult)
			{
				if (projectConfig.codeType == 0)
				{
					bool findResult;
					//寻找QR Code
					for (int i = 0; i < codeConfig.cycleTimes; i++)
					{
						findResult = imageProcessing.FindCodeCoutours(code, temp, codeConfig, rotatedRect, i);
						if (findResult)
						{
							if (projectConfig.isDebug)
							{
								tempFormat.LoadStringW(IDS_STRING_FIND_QRCODE_TIMES);
								tempStr.Format(tempFormat, i);
								AddShowMSG(tempStr);
							}
							break;
						}
					}
					if (findResult)
					{
						Rect tempRect = rotatedRect.boundingRect();
						int maxR = max(tempRect.width * 4 / 3, tempRect.height * 4 / 3);
						Rect scanRect = Rect(tempRect.x + projectConfig.rect.x - tempRect.width / 6,
							tempRect.y + projectConfig.rect.y - tempRect.height / 6, maxR, maxR);
						Mat codeTemp = Mat(image, scanRect);
						//Zbar
						for (int i = 0; i < codeConfig.cycleTimes; i++)
						{
							scanResult = imageProcessing.ScanBarCodeForZbar(codeTemp, ScanRect, data, codePoint, codeConfig, i);
							if (scanResult)
							{
								if (!codePoint.empty())
									for (int i = 0; i < 3; i++)
									{
										codePoint[i].x += tempRect.x - tempRect.width / 6;
										codePoint[i].y += tempRect.y - tempRect.height / 6;
									}
								if (projectConfig.isDebug)
								{
									tempFormat.LoadStringW(IDS_STRING_ZBAR_DECODE_TIMES);
									tempStr.Format(tempFormat, i);
									AddShowMSG(tempStr);
								}
								break;
							}
						}
					}
				}
			}
			if (!scanResult)
				res = 1;
			if (projectConfig.isDebug)
				imshow(codeWindow, ScanRect);
		}

		/*switch (projectConfig.codeType)
		{
		case 0:
		{
			bool findResult;
			for (int i = 0; i < codeConfig.cycleTimes; i++)
			{
				findResult = imageProcessing.FindCodeCoutours(code, temp, codeConfig, rotatedRect, i);
				if (findResult)
				{
					if (projectConfig.isDebug)
					{
						tempFormat.LoadStringW(IDS_STRING_FIND_QRCODE_TIMES);
						tempStr.Format(tempFormat, i);
						AddShowMSG(tempStr);
					}
					break;
				}
			}
			if (findResult)
			{
				Rect tempRect = rotatedRect.boundingRect();
				int maxR = max(tempRect.width * 4 / 3, tempRect.height * 4 / 3);
				Rect scanRect = Rect(tempRect.x + projectConfig.rect.x - tempRect.width / 6,
					tempRect.y + projectConfig.rect.y - tempRect.height / 6,
					maxR, maxR);
				Mat codeTemp = Mat(image, scanRect);
				Mat ScanRect;

				bool scanResult = false;
				for (int i = 0; i < codeConfig.cycleTimes; i++)
				{
					scanResult = imageProcessing.ScanBarCodeForZxing(codeTemp, ScanRect, 0, data, codePoint, codeConfig, i);
					if (scanResult)
					{
						for (int i = 0; i < 3; i++)
						{
							codePoint[i].x += tempRect.x - tempRect.width / 6;
							codePoint[i].y += tempRect.y - tempRect.height / 6;
						}
						if (projectConfig.isDebug)
						{
							tempFormat.LoadStringW(IDS_STRING_DECODE_TIMES);
							tempStr.Format(tempFormat, i);
							AddShowMSG(tempStr);
						}
						break;
					}
				}

				if (!scanResult)
				{
					for (int i = 0; i < codeConfig.cycleTimes; i++)
					{
						scanResult = imageProcessing.ScanBarCodeForZbar(codeTemp, ScanRect, data, codePoint, codeConfig, i);
						if (scanResult)
						{
							if (projectConfig.isDebug)
							{
								tempFormat.LoadStringW(IDS_STRING_ZBAR_DECODE_TIMES);
								tempStr.Format(tempFormat, i);
								AddShowMSG(tempStr);
							}
							break;
						}
					}

					if (!scanResult)
					{
						tempFormat.LoadStringW(IDS_STRING_DECODE_FAILED);
						AddShowMSG(tempFormat);
						putText(J, "error code", Point(projectConfig.imageWidth / 8, projectConfig.imageHeight / 8),
							CV_FONT_HERSHEY_COMPLEX, 2, red, projectConfig.fontSize, 8);
						res = 1;
					}
				}
				if (projectConfig.isDebug)
					imshow(codeWindow, ScanRect);
			}
			else
			{
				Mat ScanRect;
				bool scanResult;
				for (int i = 0; i < codeConfig.cycleTimes; i++)
				{
					scanResult = imageProcessing.ScanBarCodeForZxing(code, ScanRect, 0, data, codePoint, codeConfig, i);
					if (scanResult)
					{
						if (projectConfig.isDebug)
						{
							tempFormat.LoadStringW(IDS_STRING_DECODE_TIMES);
							tempStr.Format(tempFormat, i);
							AddShowMSG(tempStr);
						}
						break;
					}
				}
				if (!scanResult)
				{
					tempFormat.LoadStringW(IDS_STRING_DECODE_FAILED);
					AddShowMSG(tempFormat);
					putText(J, "error code", Point(projectConfig.imageWidth / 8, projectConfig.imageHeight / 8),
						CV_FONT_HERSHEY_COMPLEX, 2, red, projectConfig.fontSize, 8);
					res = 1;
				}
			}
		}
		break;
		case 1:
		{
			bool scanResult;
			for (int i = 0; i < codeConfig.cycleTimes; i++)
			{
				//imageProcessing.PretreatmentForScanCode(code, temp, codeConfig, i);
				scanResult = imageProcessing.ScanBarCodeForZxing(code, temp, 1, data, codePoint, codeConfig, i);
				if (scanResult)
				{
					if (projectConfig.isDebug)
					{
						tempFormat.LoadStringW(IDS_STRING_DECODE_TIMES);
						tempStr.Format(tempFormat, i);
						AddShowMSG(tempStr);
					}
					break;
				}
			}
			if (projectConfig.isDebug)
				imshow(codeWindow, temp);
			if (!scanResult)
			{
				tempFormat.LoadStringW(IDS_STRING_DECODE_FAILED);
				AddShowMSG(tempFormat);
				putText(J, "error code", Point(projectConfig.imageWidth / 8, projectConfig.imageHeight / 8),
					CV_FONT_HERSHEY_COMPLEX, 2, red, projectConfig.fontSize, 8);
				res = 1;
			}
		}
		break;
		default:
			res = 11;
			break;
		}*/
		if (!res)
		{
			if (codePoint.empty())
			{
				Point2f Pointf[4];
				rotatedRect.points(Pointf);
				for (int i = 0; i < 4; i++)
				{
					if (Pointf[i].x > rotatedRect.center.x)
					{
						if (Pointf[i].y > rotatedRect.center.y)
							fourPoint2f[3] = Pointf[i];
						else
							fourPoint2f[2] = Pointf[i];
					}
					else
					{
						if (Pointf[i].y > rotatedRect.center.y)
							fourPoint2f[0] = Pointf[i];
						else
							fourPoint2f[1] = Pointf[i];
					}
				}
				//angle = GetTheta(fourPoint2f[0], fourPoint2f[1], fourPoint2f[0], fourPoint2f[3]);
			}
			else
			{
				int offset;
				if (projectConfig.codeType == 0)
				{
					codePoint[3] = Point(codePoint[0].x + codePoint[2].x - codePoint[1].x,
						codePoint[0].y + codePoint[2].y - codePoint[1].y);
					offset = offsetConfig.codeOffset;
				}
				else
					offset = 0;
				Point center = Point((codePoint[0].x + codePoint[1].x + codePoint[2].x + codePoint[3].x) / 4,
					(codePoint[0].y + codePoint[1].y + codePoint[2].y + codePoint[3].y) / 4);
				/*Point2f Pointf[4];
				Pointf[0] = codePoint[0];
				Pointf[1] = codePoint[1];
				Pointf[2] = codePoint[2];
				Pointf[3] = p4;*/
				for (int i = 0; i < 4; i++)
				{
					if (codePoint[i].x > center.x)
					{
						if (codePoint[i].y > center.y)
							fourPoint2f[3] = Point2f(codePoint[i].x + offset, codePoint[i].y + offset);
						else
							fourPoint2f[2] = Point2f(codePoint[i].x + offset, codePoint[i].y - offset);
					}
					else
					{
						if (codePoint[i].y > center.y)
							fourPoint2f[0] = Point2f(codePoint[i].x - offset, codePoint[i].y + offset);
						else
							fourPoint2f[1] = Point2f(codePoint[i].x - offset, codePoint[i].y - offset);
					}
				}
			}
			angle = GetTheta(fourPoint2f[0], fourPoint2f[1], fourPoint2f[0], fourPoint2f[3]);

			CString showCode(data.c_str());
			showCode = _T("BarCode：") + showCode;
			AddShowMSG(showCode);
			putText(J, "code:" + data, Point(projectConfig.imageWidth / 8, projectConfig.imageHeight / 8),
				FONT_HERSHEY_COMPLEX, 2, green, projectConfig.fontSize, 8);

			double Rlen;
			double CRlen;
			if (RLLineConfig.same)
				handleCodeLine(image, J, fourPoint2f[2], fourPoint2f[3], true, Rlen, CRlen);
			else
				handleCodeLine(image, J, fourPoint2f[0], fourPoint2f[1], true, Rlen, CRlen);
			bool a = JudgeAndDrawLength(J, Rlen, standardConfig.highWidth, standardConfig.lowWidth, "codeRightLine:", 100);
			bool b = JudgeAndDrawLength(J, CRlen, standardConfig.highCodeWidth, standardConfig.lowCodeWidth, "codeHeight:", 300);

			double Dlen;
			double CDlen;
			if (UDLineConfig.same)
				handleCodeLine(image, J, fourPoint2f[0], fourPoint2f[3], false, Dlen, CDlen);
			else
				handleCodeLine(image, J, fourPoint2f[1], fourPoint2f[2], false, Dlen, CDlen);
			bool c = JudgeAndDrawLength(J, Dlen, standardConfig.highHeight, standardConfig.lowHeight, "codeTopLine:", 200);
			bool d = JudgeAndDrawLength(J, CDlen, standardConfig.highCodeHeight, standardConfig.lowCodeHeight, "codeWidth:", 400);

			bool e = JudgeAndDrawLength(J, angle, standardConfig.highAngle, standardConfig.lowAngle, "codeAngle:", 500);

			if (!(a && c))
				res = 2;
			else if (!(b && d))
				res = 3;
			else if (!e)
				res = 4;
		}
		else
		{
			tempFormat.LoadStringW(IDS_STRING_DECODE_FAILED);
			AddShowMSG(tempFormat);
			putText(J, "error code", Point(projectConfig.imageWidth / 8, projectConfig.imageHeight / 8),
				FONT_HERSHEY_COMPLEX, 2, red, projectConfig.fontSize, 8);
		}
	}
	putText(J, res ? "NG":"OK", Point(J.size().width * 3 / 4, J.size().height / 4),
			FONT_HERSHEY_COMPLEX, 12, res ? red : green, 20, 8);
	if (projectConfig.isDebug)
	{
		t = GetTickCount() - t;
		tempFormat.LoadStringW(IDS_STRING_TIMEFORTASK);
		tempStr.Format(tempFormat, t);
		AddShowMSG(tempStr);
	}
	return res;
}

// 处理二维码到边缘距离
void CBarCodeScannerDlg::handleCodeLine(const Mat& I, Mat& J, Point cp1, Point cp2, bool isRight, double& Len, double& CLen)
{
	Point mid1 = FindMidPoint(cp1, cp2, projectConfig.rect.x, projectConfig.rect.y);
	Mat lineRect;
	if (isRight)
	{
		lineRect = Mat(I, RLLineConfig.rect);
		rectangle(J, RLLineConfig.rect, Scalar(0, 0, 255), projectConfig.pencilSize);
	}
	else
	{
		lineRect = Mat(I, UDLineConfig.rect);
		rectangle(J, UDLineConfig.rect, Scalar(0, 0, 255), projectConfig.pencilSize);
	}
	Vec4i l;
	CLen = GetLineLength(cp1, cp2, projectConfig.pixel_scale_x, projectConfig.pixel_scale_y);
	CLen += isRight ? offsetConfig.codeHeightOffset : offsetConfig.codeWidthOffset;
	Point cp1_os = Point(cp1.x + projectConfig.rect.x, cp1.y + projectConfig.rect.y);
	Point cp2_os = Point(cp2.x + projectConfig.rect.x, cp2.y + projectConfig.rect.y);
	line(J, cp1_os, cp2_os, Scalar(0, 0, 255), projectConfig.pencilSize);
	bool result;
	if (isRight)
		result = imageProcessing.PretreatmentForFindLine(lineRect, RLLineConfig, l, isRight);
	else
		result = imageProcessing.PretreatmentForFindLine(lineRect, UDLineConfig, l, isRight);
	if (result)
	{
		Point R1;
		Point R2;
		if (isRight)
		{
			R1 = Point(l[0] + RLLineConfig.rect.x + RLLineConfig.offset, l[1] + RLLineConfig.rect.y);
			R2 = Point(l[2] + RLLineConfig.rect.x + RLLineConfig.offset, l[3] + RLLineConfig.rect.y);
		}
		else
		{
			R1 = Point(l[0] + UDLineConfig.rect.x, l[1] + UDLineConfig.rect.y + UDLineConfig.offset);
			R2 = Point(l[2] + UDLineConfig.rect.x, l[3] + UDLineConfig.rect.y + UDLineConfig.offset);
		}
		Point mid2 = PointToLineDist(mid1, R1, R2);
		line(J, R1, R2, Scalar(0, 0, 255), projectConfig.pencilSize);
		//line(J, mid1, mid2, Scalar(0, 0, 255), 5);
		Len = GetLineLength(mid1, mid2, projectConfig.pixel_scale_x, projectConfig.pixel_scale_y);
		Len += isRight ? offsetConfig.widthOffset : offsetConfig.heightOffset;
	}
	else
	{
		Len = 0.0;
	}
}

Point CBarCodeScannerDlg::FindMidPoint(Point p1, Point p2, int x_offset, int y_offset)
{
	return Point((p1.x + p2.x) / 2 + x_offset, (p1.y + p2.y) / 2 + y_offset);
}

double CBarCodeScannerDlg::GetLineLength(Point p1, Point p2, double pixel_scale_x, double pixel_scale_y)
{
	double x1 = p1.x / pixel_scale_x;
	double x2 = p2.x / pixel_scale_x;
	double y1 = p1.y / pixel_scale_y;
	double y2 = p2.y / pixel_scale_y;
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

// 判断尺寸范围并画在图片上
bool CBarCodeScannerDlg::JudgeAndDrawLength(Mat& J, double len, double high, double low, string start, int offset)
{
	bool result = false;
	if (len >= low && len <= high)
		result = true;
	char num[10];
	sprintf_s(num, "%.3lf", len);
	putText(J, start + num, Point(projectConfig.imageWidth / 8, projectConfig.imageHeight / 8 + offset),
		FONT_HERSHEY_COMPLEX, 2, result?green:red, projectConfig.fontSize, 8);
	return result;
}

// 计算两线角度
double CBarCodeScannerDlg::GetTheta(Point p1, Point p2, Point p3, Point p4)
{
	double thetaA;
	double thetaB;
	if (p2.x == p1.x)
		thetaA = CV_PI / 2;
	else
		thetaA = atan((double)(p1.y - p2.y) / (p1.x - p2.x));
	if (p3.x == p4.x)
		thetaB = CV_PI / 2;
	else
		thetaB = atan((double)(p3.y - p4.y) / (p3.x - p4.x));

	return fabs(thetaA - thetaB) / CV_PI * 180.0;
}

// 点到线的最短距离
Point CBarCodeScannerDlg::PointToLineDist(Point p, Point p1, Point p2)
{
	Point result;
	//向量内积，向量1为点1到点p，向量2为点1到点2
	double cross = (p2.x - p1.x) * (p.x - p1.x) + (p2.y - p1.y) * (p.y - p1.y);
	//向量2的长度的平方
	double d2 = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
	//double d2 = norm((p2.x - p1.x), (p2.y - p1.y));

	//r为向量1在向量2上投影向量与向量2的比值，方向相反则为负数
	double r = cross / d2;
	result.x = p1.x + (p2.x - p1.x) * r;
	result.y = p1.y + (p2.y - p1.y) * r;
	return result;
}

// 清理日志与图片线程
void CBarCodeScannerDlg::DeleteLogAndImageThread()
{
	if (clearConfig.lang == 1)
		SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
	else
		SetThreadUILanguage(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED));
	CString str;
	str.LoadStringW(IDS_STRING_EXECUTE_CLEAR_TASK);
	AddShowMSG(str);
	CheckAndDeleteDirectory(_T("IMGBackUp\\*.*"));
	CheckAndDeleteDirectory(_T("log\\*.*"));
}

void CBarCodeScannerDlg::multipleTestThread()
{
	if (clearConfig.lang == 1)
		SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
	else
		SetThreadUILanguage(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED));
	if (_access("IMGBackUp\\test", 0) != -1)
		DeleteDirectory(_T("IMGBackUp\\test"));
	if (_access("IMGBackUp\\testr", 0) != -1)
		DeleteDirectory(_T("IMGBackUp\\testr"));
	testingImage = true;
	testingFlag = true;
	CString temp;
	temp.LoadStringW(IDS_STRING_STOP);
	testButton.SetWindowTextW(temp);
	temp.LoadStringW(IDS_STRING_TEST_START);
	AddShowMSG(temp);
	Mat R;
	string B;
	int res;
	unique_lock<mutex> lock(mtxSaveImage);
	for (size_t i = 0; i < allImagePath.size(); i++)
	{
		if (!testingImage)
			break;
		Mat J = imread((LPCSTR)(CStringA)allImagePath[i], IMREAD_ANYDEPTH | IMREAD_ANYCOLOR);
		if (!J.data)
			continue;
		res = CodeJudgement(J, R, B);
		if (savingImage)
			testImage.wait(lock);
		image_j = J;
		image_r = R;
		newBarcode = B;
		imageResType = res;
		savingImage = true;
		saveImage.notify_all();
	}
	lock.unlock();
	temp.LoadStringW(IDS_STRING_TEST_DONE);
	AddShowMSG(temp);
	temp.LoadStringW(IDS_STRING_TEST);
	testButton.SetWindowTextW(temp);
	testingImage = false;
	testingFlag = false;
	allImagePath.clear();
	ShellExecute(NULL, _T("open"), _T("IMGBackUp"), NULL, NULL, SW_SHOW);
}

//保存图片线程
void CBarCodeScannerDlg::SaveImageThread()
{
	if (clearConfig.lang == 1)
		SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
	else
		SetThreadUILanguage(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED));
	unique_lock<mutex> lock(mtxSaveImage);
	while (true)
	{
			saveImage.wait(lock);
			if (!savingImage)
				continue;
			SYSTEMTIME st;
			GetLocalTime(&st);
			CString srcFile, resFile, lastName;

			if (testingFlag)
				srcFile = _T("IMGBackUp\\test");
			else
				srcFile.Format(_T("IMGBackUp\\%u_%02u_%02u"), st.wYear, st.wMonth, st.wDay);
			resFile = srcFile + _T("r");
			lastName.Format(_T("_%02u%02u%02u_%03u.png"), st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

			string srcpath(CW2A(srcFile.GetString()));
			string respath(CW2A(resFile.GetString()));
			string lastpath(CW2A(lastName.GetString()));

			checkFile(srcpath.c_str());
			checkFile((srcpath + "\\OK").c_str());

			checkFile(respath.c_str());
			checkFile((respath + "\\OK").c_str());

			if (imageResType)
			{
				srcpath += "\\NG";
				respath += "\\NG";
				checkFile(respath.c_str());
				checkFile(srcpath.c_str());
				if (imageResType == 1)
				{
					srcpath += "\\ERR01";
					respath += "\\ERR01";
					checkFile(respath.c_str());
					checkFile(srcpath.c_str());
					CString cstrTime;
					cstrTime.Format(_T("\\%u_%02u_%02u"), st.wYear, st.wMonth, st.wDay);
					string name1(CW2A(cstrTime.GetString()));
					srcpath += name1;
					respath += name1;
				}
				else
				{
					switch (imageResType)
					{
					case 2:
						srcpath += "\\ERR02";
						respath += "\\ERR02";
						break;
					case 3:
						srcpath += "\\ERR03";
						respath += "\\ERR03";
						break;
					case 10:
						srcpath += "\\ERR10";
						respath += "\\ERR10";
						break;
					case 11:
						srcpath += "\\ERR11";
						respath += "\\ERR11";
						break;
					default:
						break;
					}
					checkFile(respath.c_str());
					checkFile(srcpath.c_str());
					respath += "\\";
					respath += newBarcode;
					srcpath += "\\";
					srcpath += newBarcode;
				}
			}
			else
			{
				respath += "\\OK\\";
				respath += newBarcode;
				srcpath += "\\OK\\";
				srcpath += newBarcode;
			}
			respath += lastpath;
			srcpath += lastpath;
			Mat image_save_r;
			resize(image_r, image_save_r, Size(600, 450));
			imwrite(respath, image_save_r);
			imwrite(srcpath, image_j);

			// 执行完成
			savingImage = false;
			if (testingFlag)
				testImage.notify_all();
		}
	lock.unlock();
}

// omron线程
void CBarCodeScannerDlg::OmronThread()
{
	while (cameraIsOpen)
	{
		if (omronFlag)
		{
			switch (omronFlag)
			{
			case 1:
				omronPlc.put_Output(COleVariant(sendOK));
				break;
			case 2:
				omronPlc.put_Output(COleVariant(sendNG));
				break;
			default:
				break;
			}
			plcGrabFlag = false;
			omronFlag = 0;
		}
		else if (recTrigger)
		{
			/*omronPlc.put_Output(COleVariant(sendReset));
			Sleep(50);*/
			omronPlc.put_Output(COleVariant(resetTrigger));
			recTrigger = false;
		}
		else
			omronPlc.put_Output(COleVariant(checkTrigger));
		Sleep(80);
	}
}

// 相机线程
void CBarCodeScannerDlg::CameraThread()
{
	if (clearConfig.lang == 1)
		SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
	else
		SetThreadUILanguage(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED));
	PylonInitialize();
	CString tempTips;
	try
	{
		CTlFactory& tlFactory = CTlFactory::GetInstance();

		DeviceInfoList_t devices;

		if (tlFactory.EnumerateDevices(devices) == 0)
		{
			//throw RUNTIME_EXCEPTION("No camera present.");
			PylonTerminate();
			cameraRunning = false;
			//MessageBox(_T("basler"));
		}
		else
		{
			for (size_t i = 0; i < devices.size(); i++)
				cameraList.InsertString(i, CStringW(devices[i].GetUserDefinedName().c_str()));
			//devices[0].GetUserDefinedName
			cameraList.SetCurSel(max(0, cameraList.FindStringExact(0, cameraName)));
		}
		while (cameraRunning)
		{
			try
			{
				unique_lock<mutex> lock(mtxCamera);
				openCamera.wait(lock);
				lock.unlock();
				if (!cameraRunning)
					break;
				CInstantCamera camera(tlFactory.CreateDevice(devices[cameraList.GetCurSel()]));
				//获取相机节点映射以获得相机参数
				INodeMap& nodemap = camera.GetNodeMap();
				//打开相机
				camera.Open();
				tempTips.LoadStringW(IDS_STRING_OPEN_CAMERA);
				AddShowMSG(tempTips);
				//获取相机成像宽度和高度
				CIntegerPtr width = nodemap.GetNode("Width");
				projectConfig.imageWidth = width->GetValue();
				CIntegerPtr height = nodemap.GetNode("Height");
				projectConfig.imageHeight = height->GetValue();
				//设置相机最大缓冲区,默认为10
				camera.MaxNumBuffer = 5;

				while (cameraIsOpen)
				{
					try
					{
						unique_lock<mutex> lock2(mtxImage);
						grabImage.wait(lock2);
						lock2.unlock();
						if (!cameraIsOpen)
						{
							camera.Close();
							tempTips.LoadStringW(IDS_STRING_CLOSE_CAMERA);
							AddShowMSG(tempTips);
							break;
						}

						DWORD t;
						if (projectConfig.isDebug)
							t = GetTickCount();
						//抓取结果数据指针
						CGrabResultPtr ptrGrabResult;

						if (camera.GrabOne(5000, ptrGrabResult, TimeoutHandling_ThrowException))
						//while (camera.IsGrabbing())
						{
							// 等待接收和恢复图像，超时时间设置为5000 ms.
							if (ptrGrabResult->GrabSucceeded())
							{
								int nPixelType = ptrGrabResult->GetPixelType();
								unique_lock<mutex> lock3(mtxSaveImage);
								tempTips.LoadStringW(IDS_STRING_PIXEL_TYPE);
								switch (nPixelType)
								{
								case PixelType_RGB8packed:
								{
									AddShowMSG(tempTips + _T(":RGB8Packed"));
									cvtColor(Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, ptrGrabResult->GetBuffer()), image_j, COLOR_RGB2BGR);
									break;
								}
								case PixelType_BGR8packed:
								{
									AddShowMSG(tempTips + _T(":BGR8packed"));
									Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, ptrGrabResult->GetBuffer()).copyTo(image_j);
									break;
								}
								case PixelType_Mono8:
								{
									AddShowMSG(tempTips + _T(":Mono8"));
									cvtColor(Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC1, ptrGrabResult->GetBuffer()), image_j, COLOR_GRAY2BGR);
									break;
								}
								case PixelType_YUV422packed:
								{
									AddShowMSG(tempTips + _T(":YUV422packed"));
									cvtColor(Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC2, ptrGrabResult->GetBuffer()), image_j, COLOR_YUV2BGR_Y422);
									break;
								}
								case PixelType_BayerGB8:
								{
									AddShowMSG(tempTips + _T(":BayerGB8"));
									cvtColor(Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC1, ptrGrabResult->GetBuffer()), image_j, COLOR_BayerGB2BGR);
									break;
								}
								case PixelType_BayerBG8:
								{
									AddShowMSG(tempTips + _T(":BayerBG8"));
									cvtColor(Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC1, ptrGrabResult->GetBuffer()), image_j, COLOR_BayerRG2BGR);
									break;
								}
								default:
								{
									tempTips.LoadStringW(IDS_STRING_PIXEL_TYPE_ERR);
									AddShowMSG(tempTips); 
									continue; 
								}
								}
								if (projectConfig.isDebug)
								{
									CString tempStr;
									t = GetTickCount() - t;
									tempTips.LoadStringW(IDS_STRING_TIMEFORGRAB);
									tempStr.Format(tempTips, t);
									AddShowMSG(tempStr);
								}

								imageResType = CodeJudgement(image_j, image_r, newBarcode);

								savingImage = true;
								lock3.unlock();
								saveImage.notify_all();

								counting(imageResType);
								updateCounting();
								imshow(imageWindow, image_r);

								tempTips.LoadStringW(IDS_STRING_CODE_JUDGE);
								if (imageResType)
									AddShowMSG(tempTips + _T("NG"));
								else
									AddShowMSG(tempTips + _T("OK"));

								if (plcGrabFlag)
								{
									//处理信号
									//plcGrabFlag = false;
									//omronPlc.put_Output(COleVariant(resetTrigger));
									if (imageResType)
									{
										tempTips.LoadStringW(IDS_STRING_SEND_NG);
										AddShowMSG(tempTips);
										omronFlag = 2;
										//omronPlc.put_Output(COleVariant(sendNG));
									}
									else
									{
										tempTips.LoadStringW(IDS_STRING_SEND_OK);
										AddShowMSG(tempTips);
										omronFlag = 1;
										//omronPlc.put_Output(COleVariant(sendOK));
									}
								}
							}
						}
						else
						{
							// 拍照失败
							tempTips.LoadStringW(IDS_STRING_GRAB_TIME_OUT);
							AddShowMSG(tempTips);
						}
					}
					catch (...)
					{
						//抓取图片失败
						tempTips.LoadStringW(IDS_STRING_GRAB_ERR);
						AddShowMSG(tempTips);
					}

				}
			}
			catch (...)
			{
				//相机异常断开连接
				tempTips.LoadStringW(IDS_STRING_OPEN_CAMERA_FAILED);
				AddShowMSG(tempTips);
			}
		}
	}
	catch (...)
	{

	}
	// Releases all pylon resources.
	PylonTerminate();
}

// 版本识别线程
void CBarCodeScannerDlg::VersionThread()
{
}

void CBarCodeScannerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	KillTimer(1);
	WritePrivateProfileStruct(_T("project"), _T("count"), &countRecord, sizeof(configForCount), imageParameter);
	cameraRunning = false;
	cameraIsOpen = false;
	grabImage.notify_all();
	openCamera.notify_all();
	destroyAllWindows();
}


void CBarCodeScannerDlg::ShowForStop()
{
	//manualGrab.EnableWindow();
	testButton.EnableWindow();
	aboutBtn.EnableWindow();
	if (userLevel) 
	{
		configButton.EnableWindow();
		codeStandard.EnableWindow();
		cameraList.EnableWindow();
	}
	CString start;
	start.LoadStringW(IDS_STRING_START);
	startOrStop.SetWindowTextW(start);
}


void CBarCodeScannerDlg::HideForStart()
{
	//manualGrab.EnableWindow(false);
	testButton.EnableWindow(FALSE);
	aboutBtn.EnableWindow(FALSE);
	configButton.EnableWindow(FALSE);
	codeStandard.EnableWindow(FALSE);
	cameraList.EnableWindow(FALSE);
	CString stop;
	stop.LoadStringW(IDS_STRING_STOP);
	startOrStop.SetWindowTextW(stop);
}
BEGIN_EVENTSINK_MAP(CBarCodeScannerDlg, CDialogEx)
	ON_EVENT(CBarCodeScannerDlg, IDC_OMRON_PLC, 1, CBarCodeScannerDlg::OnCommOmronPlc, VTS_NONE)
END_EVENTSINK_MAP()


void CBarCodeScannerDlg::OnCommOmronPlc()
{
	// TODO: 在此处添加消息处理程序代码
	COleSafeArray safearray_inp;
	long len, k;
	if (omronPlc.get_CommEvent() == 2) //值为 2 表示接收缓冲区内有字符
	{
		safearray_inp = omronPlc.get_Input();; ///变量转换
		len = safearray_inp.GetOneDimSize(); //得到有效的数据长度
		byte *rxdata; //设置 BYTE 数组
		rxdata = new byte[len];
		for (k = 0; k<len; k++)
			safearray_inp.GetElement(&k, rxdata + k);

		/*if (projectConfig.isDebug)
		{
			CString temp;
			temp.Format(_T("长度：%d，内容：%s"), len, rxdata);
			AddShowMSG(temp);
		}*/
		
		if (len == 15) 
		{
			// 判断信号
			if (!plcGrabFlag && rxdata[9] == '1' && rxdata[10] == '1' && rxdata[7] == '0' && rxdata[8] == '0')
			{
				CString triggerOn;
				triggerOn.LoadStringW(IDS_STRING_TRIGGER_ON);
				AddShowMSG(triggerOn);
				recTrigger = true;
				//omronPlc.put_Output(COleVariant(resetTrigger));
				//omronPlc.put_Output(COleVariant(sendReset));
				plcGrabFlag = true;
				grabImage.notify_all();
			}
		}

		delete[] rxdata;
		/*else if (len == 11)
		{

		}*/
	}
}

// 打开串口
bool CBarCodeScannerDlg::openMSComm()
{
	CString portnums(portName);
	portnums.Delete(0, 3);
	int flag = _ttoi(portnums);//将其转化为数值类型
	if (flag)//如果输入的串口编号为正整数，才执行下列命令
	{
		if (omronPlc.get_PortOpen())
		{
			omronPlc.put_PortOpen(FALSE);
		}
		omronPlc.put_CommPort(flag); //选择COM
		omronPlc.put_InBufferSize(1024); //接收缓冲区
		omronPlc.put_OutBufferSize(1024);//发送缓冲区
		omronPlc.put_InputLen(0);//设置当前接收区数据长度为0,表示全部读取
		omronPlc.put_InputMode(1);//以二进制方式读写数据
		omronPlc.put_RThreshold(projectConfig.recLen);//接收缓冲区有1个及1个以上字符时，将引发接收数据的OnComm
		omronPlc.put_Settings(_T("115200,e,7,2"));//波特率，检验位，数据位，停止位

		if (!omronPlc.get_PortOpen())//如果串口没有打开则打开
		{
			try
			{
				omronPlc.put_PortOpen(TRUE);//打开串口
				CString openTips;
				openTips.LoadStringW(IDS_STRING_OPEN_COM);
				AddShowMSG(openTips);
				return true;
			}
			catch (...)
			{

			}
		}
		else
		{
			omronPlc.put_OutBufferCount(0);
		}
	}
	else
	{
		CString str;
		str.LoadStringW(IDS_STRING_SELECT_COM);
		MessageBox(str);
	}
	return false;
}

// 鼠标响应函数
void on_mouse_l(int mouse_event, int x, int y, int flags, void *ustc)
{
	if (mouse_event == EVENT_LBUTTONDOWN)//左键按下，读取初始坐标
	{
		thisDlg->SetSPt(x, y);
	}
	if (mouse_event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
	{
		if (x != thisDlg->GetSPt().x && y != thisDlg->GetSPt().y)
		{
			thisDlg->SetEPt(x, y);
			thisDlg->DrawRectangle();
		}
	}
	if (mouse_event == EVENT_LBUTTONUP)
	{
		if (x != thisDlg->GetSPt().x && y != thisDlg->GetSPt().y)
		{
			thisDlg->SetEPt(x, y);
			thisDlg->DrawRectangle();
		}
	}
}


void CBarCodeScannerDlg::DrawRectangle()
{
	Mat image_temp;
	image_r.copyTo(image_temp);
	rectangle(image_temp, Rect(s_pt,e_pt), Scalar(255, 255, 0), projectConfig.pencilSize);
	imshow(imageWindow, image_temp);
}

// 右键处理函数
void CBarCodeScannerDlg::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	// TODO: 在此处添加消息处理程序代码
	CRect rect;//定义矩形区域
	GetDlgItem(IDC_IMAGE_SHOW)->GetWindowRect(&rect);//获取控件区域的矩形
	if (rect.PtInRect(point))
	{
		CMenu   menu;
		VERIFY(menu.LoadMenu(IDR_MENU_RIGHT_CLICK));

		CMenu*   pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, point.x, point.y, this); //在鼠标当前位置显示指定菜单
	}
}

// 复制坐标
void CBarCodeScannerDlg::OnCopyCoordinate()
{
	// TODO: 在此添加命令处理程序代码
	if (OpenClipboard())
	{
		EmptyClipboard();
		CString str;
		str.Format(_T("%d,%d,%d,%d"), min(s_pt.x, e_pt.x), min(s_pt.y, e_pt.y),
			abs(s_pt.x - e_pt.x), abs(s_pt.y - e_pt.y));
		HANDLE hClip = GlobalAlloc(GMEM_MOVEABLE, str.GetLength() + 1);
		USES_CONVERSION;
		strcpy_s((char *)GlobalLock(hClip), str.GetLength() + 1, T2A(str));
		GlobalUnlock(hClip);//解锁  
		SetClipboardData(CF_TEXT, hClip);//设置格式
		CloseClipboard();
	}
}


void CBarCodeScannerDlg::SetSPt(int x, int y)
{
	s_pt = Point(min(image_j.size().width, max(x, 0)), min(image_j.size().height, max(y, 0)));
}


Point CBarCodeScannerDlg::GetSPt()
{
	return s_pt;
}


void CBarCodeScannerDlg::SetEPt(int x, int y)
{
	e_pt = Point(min(image_j.size().width, max(x, 0)), min(image_j.size().height, max(y, 0)));
}


Point CBarCodeScannerDlg::GetEPt()
{
	return e_pt;
}


void CBarCodeScannerDlg::AddShowMSG(CString msg)
{
	if (showMSG.GetLineCount() > 50)
	{
		showMSG.SetSel(0, -1);
		showMSG.ReplaceSel(_T(""));
	}
	CTime time1 = CTime::GetCurrentTime();
	CString cstrTime = time1.Format("[%Y/%m/%d %H:%M:%S] ");
	CString logFile = time1.Format("%Y-%m-%d.log");

	//showMSG.SetSel(showMSG.GetWindowTextLengthW());
	int nlen = showMSG.GetWindowTextLengthW();
	showMSG.SetSel(nlen, nlen);
	showMSG.ReplaceSel(cstrTime + msg + _T("\r\n"));
	AddLogMSG(msg, logFile);
}


void CBarCodeScannerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (nType != SIZE_MINIMIZED)  //判断窗口是不是最小化了，因为窗口最小化之后 ，窗口的长和宽会变成0，当前一次变化的时就会出现除以0的错误操作
		ReSizeAll();
}



void CBarCodeScannerDlg::ReSizeAll()
{
	POINT Newp; //获取现在对话框的大小  
	CRect recta;
	GetWindowRect(&recta);     //取客户区大小    
	Newp.x = recta.Width();
	Newp.y = recta.Height();
	int x_offset = Newp.x - Oldp.x;
	int y_offset = Newp.y - Oldp.y;
	if (aboutBtn)
		aboutBtn.MoveWindow(buttonRect[0].left + x_offset, buttonRect[0].top + y_offset,
			buttonRect[0].Width(), buttonRect[0].Height());
	if (logBtn)
		logBtn.MoveWindow(buttonRect[1].left + x_offset, buttonRect[1].top + y_offset,
			buttonRect[1].Width(), buttonRect[1].Height());
	if (codeStandard)
		codeStandard.MoveWindow(buttonRect[2].left + x_offset, buttonRect[2].top + y_offset,
			buttonRect[2].Width(), buttonRect[2].Height());
	if (configButton)
		configButton.MoveWindow(buttonRect[3].left + x_offset, buttonRect[3].top + y_offset,
			buttonRect[3].Width(), buttonRect[3].Height());
	if (startOrStop)
		startOrStop.MoveWindow(buttonRect[4].left + x_offset, buttonRect[4].top + y_offset,
			buttonRect[4].Width(), buttonRect[4].Height());
	if (countAll)
		countAll.MoveWindow(buttonRect[5].left + x_offset, buttonRect[5].top + y_offset,
			buttonRect[5].Width(), buttonRect[5].Height());
	if (countNG)
		countNG.MoveWindow(buttonRect[6].left + x_offset, buttonRect[6].top + y_offset,
			buttonRect[6].Width(), buttonRect[6].Height());
	if (countOK)
		countOK.MoveWindow(buttonRect[7].left + x_offset, buttonRect[7].top + y_offset,
			buttonRect[7].Width(), buttonRect[7].Height());
	if (countVersionNG)
		countVersionNG.MoveWindow(buttonRect[8].left + x_offset, buttonRect[8].top + y_offset,
			buttonRect[8].Width(), buttonRect[8].Height());
	if (countYield)
		countYield.MoveWindow(buttonRect[9].left + x_offset, buttonRect[9].top + y_offset,
			buttonRect[9].Width(), buttonRect[9].Height());
	if (staticOne)
		staticOne.MoveWindow(buttonRect[10].left + x_offset, buttonRect[10].top + y_offset,
			buttonRect[10].Width(), buttonRect[10].Height());
	if (staticTwo)
		staticTwo.MoveWindow(buttonRect[11].left + x_offset, buttonRect[11].top + y_offset,
			buttonRect[11].Width(), buttonRect[11].Height());
	if (staticThree)
		staticThree.MoveWindow(buttonRect[12].left + x_offset, buttonRect[12].top + y_offset,
			buttonRect[12].Width(), buttonRect[12].Height());
	if (staticFour)
		staticFour.MoveWindow(buttonRect[13].left + x_offset, buttonRect[13].top + y_offset,
			buttonRect[13].Width(), buttonRect[13].Height());
	if (staticFive)
		staticFive.MoveWindow(buttonRect[14].left + x_offset, buttonRect[14].top + y_offset,
			buttonRect[14].Width(), buttonRect[14].Height());
	if (resetCounting)
		resetCounting.MoveWindow(buttonRect[22].left + x_offset, buttonRect[22].top + y_offset,
			buttonRect[22].Width(), buttonRect[22].Height());

	if (staticSeven)
		staticSeven.MoveWindow(buttonRect[23].left + x_offset, buttonRect[23].top + y_offset,
			buttonRect[23].Width(), buttonRect[23].Height());
	if (staticEight)
		staticEight.MoveWindow(buttonRect[24].left + x_offset, buttonRect[24].top + y_offset,
			buttonRect[24].Width(), buttonRect[24].Height());
	if (staticNine)
		staticNine.MoveWindow(buttonRect[25].left + x_offset, buttonRect[25].top + y_offset,
			buttonRect[25].Width(), buttonRect[25].Height());
	if (staticTen)
		staticTen.MoveWindow(buttonRect[26].left + x_offset, buttonRect[26].top + y_offset,
			buttonRect[26].Width(), buttonRect[26].Height());
	if (positionNG)
		positionNG.MoveWindow(buttonRect[27].left + x_offset, buttonRect[27].top + y_offset,
			buttonRect[27].Width(), buttonRect[27].Height());
	if (sizeNG)
		sizeNG.MoveWindow(buttonRect[28].left + x_offset, buttonRect[28].top + y_offset,
			buttonRect[28].Width(), buttonRect[28].Height());
	if (angleNG)
		angleNG.MoveWindow(buttonRect[29].left + x_offset, buttonRect[29].top + y_offset,
			buttonRect[29].Width(), buttonRect[29].Height());
	if (otherNG)
		otherNG.MoveWindow(buttonRect[30].left + x_offset, buttonRect[30].top + y_offset,
			buttonRect[30].Width(), buttonRect[30].Height());

	if (staticSix)
		staticSix.SetWindowPos(NULL, buttonRect[15].left, buttonRect[15].top + y_offset,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
	if (cameraList)
		cameraList.SetWindowPos(NULL, buttonRect[16].left, buttonRect[16].top + y_offset,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
	if (manualGrab)
		manualGrab.SetWindowPos(NULL, buttonRect[17].left, buttonRect[17].top + y_offset,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
	if (testButton)
		testButton.SetWindowPos(NULL, buttonRect[18].left, buttonRect[18].top + y_offset,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);

	ReSizeImage(x_offset, y_offset);
}
void CBarCodeScannerDlg::ReSizeImage(int x_offset, int y_offset)
{
	int right_x = x_offset * r_image_msg;
	int left_x = x_offset - right_x;
	int x = buttonRect[20].Width() + left_x;
	int y = buttonRect[20].Height() + y_offset;
	int msg_x = 0;
	if (x * 3 >= y * 4)
	{
		x = y * 4 / 3;
		msg_x = buttonRect[20].Width() + left_x - x;
	}
	else
		y = x * 3 / 4;
	if (showMSG)
		showMSG.SetWindowPos(NULL, buttonRect[19].left + left_x - msg_x, buttonRect[19].top,
			buttonRect[19].Width() + right_x + msg_x, buttonRect[19].Height() + y_offset, SWP_NOZORDER);
	//保持图片显示比例
	if (imageShowWindow)
	{
		imageShowWindow.SetWindowPos(NULL, 0, 0, x, y, SWP_NOZORDER | SWP_NOMOVE);
		resizeWindow(imageWindow, x, y);
	}
}


void CBarCodeScannerDlg::RecordSize()
{
	GetWindowRect(buttonRect[21]);
	Oldp.x = buttonRect[21].Width();
	Oldp.y = buttonRect[21].Height();

	aboutBtn.GetWindowRect(buttonRect[0]);
	logBtn.GetWindowRect(buttonRect[1]);
	codeStandard.GetWindowRect(buttonRect[2]);
	configButton.GetWindowRect(buttonRect[3]);
	startOrStop.GetWindowRect(buttonRect[4]);
	countAll.GetWindowRect(buttonRect[5]);
	countNG.GetWindowRect(buttonRect[6]);
	countOK.GetWindowRect(buttonRect[7]);
	countVersionNG.GetWindowRect(buttonRect[8]);
	countYield.GetWindowRect(buttonRect[9]);
	staticOne.GetWindowRect(buttonRect[10]);
	staticTwo.GetWindowRect(buttonRect[11]);
	staticThree.GetWindowRect(buttonRect[12]);
	staticFour.GetWindowRect(buttonRect[13]);
	staticFive.GetWindowRect(buttonRect[14]);
	resetCounting.GetWindowRect(buttonRect[22]);
	staticSeven.GetWindowRect(buttonRect[23]);
	staticEight.GetWindowRect(buttonRect[24]);
	staticNine.GetWindowRect(buttonRect[25]);
	staticTen.GetWindowRect(buttonRect[26]);
	positionNG.GetWindowRect(buttonRect[27]);
	sizeNG.GetWindowRect(buttonRect[28]);
	angleNG.GetWindowRect(buttonRect[29]);
	otherNG.GetWindowRect(buttonRect[30]);

	staticSix.GetWindowRect(buttonRect[15]);
	cameraList.GetWindowRect(buttonRect[16]);
	manualGrab.GetWindowRect(buttonRect[17]);
	testButton.GetWindowRect(buttonRect[18]);

	showMSG.GetWindowRect(buttonRect[19]);
	int msgWidth = buttonRect[19].Width();
	imageShowWindow.GetWindowRect(buttonRect[20]);
	r_image_msg = (float)msgWidth / (msgWidth + buttonRect[20].Width());
	//ReSizeImage(0, 0);
	for (int i = 0; i < 31; i++)
		ScreenToClient(buttonRect[i]);
}


void CBarCodeScannerDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	lpMMI->ptMinTrackSize.x = 942;   //最小宽度    
	lpMMI->ptMinTrackSize.y = 569;   //最小高度
	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CBarCodeScannerDlg::AddLogMSG(CString msg, CString file)
{
	AddLogMSG(string(CW2A(msg.GetString())), string(CW2A(file.GetString())));
}

void CBarCodeScannerDlg::AddLogMSG(string msg, string file)
{
	/*CTime time1 = CTime::GetCurrentTime();
	CString cstrTime = time1.Format("%Y/%m/%d %H:%M:%S ");
	msg = string(CW2A(cstrTime.GetString())) + msg + "\r\n";
	ofstream mycout("log\\" + file, ios::app);
	mycout << msg;
	mycout.close();*/
	myLogger->writeLog(log4cplus::INFO_LOG_LEVEL, msg, "log");
}


void CBarCodeScannerDlg::checkAppFile()
{
	checkFile("log");
	checkFile("IMGBackUp");
	checkFile("matchTemplateImage");
	checkFile("knnData");
}


void CBarCodeScannerDlg::OnBnClickedCountReset()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!isCtrl)
		countRecord.All = 0;
	countRecord.codeNG = 0;
	countRecord.positionNG = 0;
	countRecord.sizeNG = 0;
	countRecord.angleNG = 0;
	countRecord.otherNG = 0;
	countRecord.VersionNG = 0;
	WritePrivateProfileStruct(_T("project"), _T("count"), &countRecord, sizeof(configForCount), imageParameter);
	updateCounting();
}


void CBarCodeScannerDlg::updateCounting()
{
	int ng = countRecord.codeNG + countRecord.positionNG + countRecord.sizeNG 
		+ countRecord.angleNG + countRecord.otherNG + countRecord.VersionNG;
	CString temp;
	temp.Format(_T("%d"), countRecord.VersionNG);
	countVersionNG.SetWindowTextW(temp);
	temp.Format(_T("%d"), countRecord.codeNG);
	countNG.SetWindowTextW(temp);

	temp.Format(_T("%d"), countRecord.positionNG);
	positionNG.SetWindowTextW(temp);
	temp.Format(_T("%d"), countRecord.sizeNG);
	sizeNG.SetWindowTextW(temp);
	temp.Format(_T("%d"), countRecord.angleNG);
	angleNG.SetWindowTextW(temp);
	temp.Format(_T("%d"), countRecord.otherNG);
	otherNG.SetWindowTextW(temp);

	temp.Format(_T("%d"), countRecord.All - ng);
	countOK.SetWindowTextW(temp);
	temp.Format(_T("%d"), countRecord.All);
	countAll.SetWindowTextW(temp);
	if (countRecord.All)
		temp.Format(_T("%.2lf%c"), (double)(countRecord.All - ng) / countRecord.All * 100, '%');
	else
		temp = _T("0.00%");
	countYield.SetWindowTextW(temp);
}


void CBarCodeScannerDlg::counting(int res)
{
	if (res)
	{
		if (res == 1)
			countRecord.codeNG++;
		else if (res == 2)
			countRecord.positionNG++;
		else if (res == 3)
			countRecord.sizeNG++;
		else if (res == 4)
			countRecord.angleNG++;
		else if (res == 5)
			countRecord.VersionNG++;
		else
			countRecord.otherNG++;
	}
	countRecord.All++;
}


void CBarCodeScannerDlg::SetUserLevel(int level)
{
	userLevel = level;
}


void CBarCodeScannerDlg::HideForLevel()
{
	manualGrab.EnableWindow(FALSE);
	//testButton.EnableWindow(FALSE);
	configButton.EnableWindow(FALSE);
	codeStandard.EnableWindow(FALSE);
}


BOOL CBarCodeScannerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	// 键盘事件
	if (pMsg->message == WM_KEYDOWN)
		if (pMsg->wParam == VK_CONTROL)
			isCtrl = true;
	if (pMsg->message == WM_KEYUP)
		if (pMsg->wParam == VK_CONTROL)
			isCtrl = false;

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CBarCodeScannerDlg::RedrawAll()
{
	staticOne.RedrawWindow();
	staticTwo.RedrawWindow();
	staticThree.RedrawWindow();
	staticFour.RedrawWindow();
	staticFive.RedrawWindow();
	staticSix.RedrawWindow();
	staticSeven.RedrawWindow();
	staticEight.RedrawWindow();
	staticNine.RedrawWindow();
	staticTen.RedrawWindow();
	codeStandard.RedrawWindow();
	configButton.RedrawWindow();
}


void CBarCodeScannerDlg::checkFile(const char* fileName)
{
	if (_access(fileName, 0) == -1)
		_mkdir(fileName);
}


void CBarCodeScannerDlg::OnTestThreshold()
{
	// TODO: 在此添加命令处理程序代码
	namedWindow("ThresholdTest", WINDOW_NORMAL);
	createTrackbar("Threshold:", "ThresholdTest",
		&thresh, 255, TestThreshold);
	TestThreshold(0, 0);
}

void TestThreshold(int, void*) 
{
	Mat temp(thisDlg->image_j, Rect(thisDlg->GetSPt(), thisDlg->GetEPt()));
	Mat testMat = temp.clone();
	if (testMat.channels() != 1)
		cvtColor(testMat, testMat, COLOR_BGR2GRAY);
	threshold(testMat, testMat, thresh, 255, 0);
	imshow("ThresholdTest", testMat);
}


afx_msg LRESULT CBarCodeScannerDlg::OnUpdataUi(WPARAM wParam, LPARAM lParam)
{
	//暂不用
	//updateCounting();
	image_j = image_r = *(Mat*)lParam;
	imshow(imageWindow, image_j);
	//CodeJudgement(image_j, image_r, string());
	return 0;
}


// 递归删除文件夹
void CBarCodeScannerDlg::DeleteDirectory(CString directoryPath)
{
	// 非文件夹直接删除
	if (GetFileAttributes(directoryPath) != FILE_ATTRIBUTE_DIRECTORY)
	{
		DeleteFile(directoryPath);
		return;
	}
	CFileFind finder;
	CString path;
	path.Format(_T("%s\\*.*"), directoryPath);
	BOOL bWorking = finder.FindFile(path);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsSystem() || finder.IsDots())
			continue;
		CString pathname = finder.GetFilePath();
		if (finder.IsDirectory())//处理文件夹
			DeleteDirectory(pathname); //递归删除文件夹
		else //处理文件
			DeleteFile(pathname);
	}
	finder.Close();
	RemoveDirectory(directoryPath);
}


// 检查文件夹创建时间
bool CBarCodeScannerDlg::CheckFolderCreateTime(CString FolderPath, CTime deletetime)
{
	HANDLE hDir = CreateFile(FolderPath,
		GENERIC_READ,//只读方式打开即可
		FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,//打开现存目录
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);
	FILETIME lpCreateTime, lpLastWriteTime;
	if (GetFileTime(hDir, &lpCreateTime, NULL, &lpLastWriteTime))
	{
		CTime ccreatetime(lpCreateTime);
		CTime clastwritetime(lpLastWriteTime);
		CTime temp = ccreatetime < clastwritetime ? ccreatetime : clastwritetime;
		CloseHandle(hDir);
		return temp < deletetime;
	}
	CloseHandle(hDir);
	return false;
}


// 删除日志与图片记录
void CBarCodeScannerDlg::CheckAndDeleteDirectory(LPCTSTR path)
{
	CFileFind finder;
	BOOL isFind = finder.FindFile(path);
	CTime deletetime = CTime::GetCurrentTime() - CTimeSpan(clearConfig.days, 0, 0, 0);
	while (isFind)
	{
		isFind = finder.FindNextFile();
		if (finder.IsSystem())
			continue;
		CString pathname = finder.GetFilePath();
		if (!finder.IsDots())
			if (CheckFolderCreateTime(pathname, deletetime))
				DeleteDirectory(pathname);
	}
	finder.Close();
}


void CBarCodeScannerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 1://定时检查删除图片与日志
	{
		CTime now = CTime::GetCurrentTime();
		int minutes = now.GetHour() * 60 + now.GetMinute();
		int deleteminutes = clearConfig.hours * 60 + clearConfig.minutes;
		if (minutes > deleteminutes && minutes <= deleteminutes + 10)
		{
			thread thc(&CBarCodeScannerDlg::DeleteLogAndImageThread, this);
			thc.detach();
		}
	}
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CBarCodeScannerDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (testingImage)
		return;
	CString exitText;
	CString caption;
	exitText.LoadStringW(IDS_STRING_EXIT_TEXT);
	caption.LoadStringW(IDS_STRING_EXIT_CAPTION);
	if (MessageBox(exitText, caption, MB_ICONINFORMATION | MB_YESNO) != IDYES)
		return;
	CDialogEx::OnClose();
}

void CBarCodeScannerDlg::OnTestCanny()
{
	// TODO: 在此添加命令处理程序代码
	namedWindow("CannyTest", WINDOW_NORMAL);
	createTrackbar("ThresholdLow:", "CannyTest",
		&threshLow, 180, TestCanny);
	createTrackbar("ThresholdHigh:", "CannyTest",
		&threshHigh, 360, TestCanny);
	TestCanny(0, 0);
}
void TestCanny(int, void*)
{
	Mat temp(thisDlg->image_j, Rect(thisDlg->GetSPt(), thisDlg->GetEPt()));
	Mat testMat = temp.clone();
	if (testMat.channels() != 1)
		cvtColor(testMat, testMat, COLOR_BGR2GRAY);
	Canny(testMat, testMat, threshLow, threshHigh);
	imshow("CannyTest", testMat);
}


void CBarCodeScannerDlg::OnTestAdaptiveThreshold()
{
	// TODO: 在此添加命令处理程序代码
	namedWindow("AdaptiveThresholdTest", WINDOW_AUTOSIZE);
	createTrackbar("offset_C: ", "AdaptiveThresholdTest",
		&offset_C, 100, TestAdativeThreshold);

	createTrackbar("Kernel size:\n 2n +1", "AdaptiveThresholdTest",
		&kernel_size, 30, TestAdativeThreshold);
	TestAdativeThreshold(0, 0);
}


void TestAdativeThreshold(int, void *)
{
	Mat temp(thisDlg->image_j, Rect(thisDlg->GetSPt(), thisDlg->GetEPt()));
	Mat testMat = temp.clone();
	if (testMat.channels() != 1)
		cvtColor(testMat, testMat, COLOR_BGR2GRAY);
	adaptiveThreshold(testMat, testMat, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, kernel_size * 2 + 3, offset_C);
	imshow("AdaptiveThresholdTest", testMat);
}


void CBarCodeScannerDlg::OnCutMat()
{
	// TODO: 在此添加命令处理程序代码
	Mat temp(image_j, Rect(min(s_pt.x, e_pt.x), min(s_pt.y, e_pt.y), abs(e_pt.x - s_pt.x), abs(e_pt.y - s_pt.y)));
	image_j = image_r = temp;
	imshow(imageWindow, image_j);
}


void CBarCodeScannerDlg::OnSaveImage()
{
	// TODO: 在此添加命令处理程序代码
	if (!image_j.data)
	{
		MessageBox(_T("no image"));
		return;
	}

	CString fileType;
	fileType.LoadStringW(IDS_STRING_OPEN_IMAGE_SELECTER);
	CFileDialog saveFileDlg(
		FALSE,  // TRUE是创建打开文件对话框，FALSE则创建的是保存文件对话框
		NULL,  // 默认的打开文件的类型
		_T("version_template"),  // 默认打开的文件名
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,  // 打开只读文件
		fileType  // 所有可以打开的文件类型
	);
	if (IDOK == saveFileDlg.DoModal())
	{
		CString savePath = saveFileDlg.GetPathName();
		if (saveFileDlg.m_ofn.nFilterIndex == 1)
			savePath += _T(".png");
		else 
			savePath += _T(".jpg");
		imwrite(string(CW2A(savePath.GetString())), image_j);
	}
}


void CBarCodeScannerDlg::OnRotate90()
{
	// TODO: 在此添加命令处理程序代码
	Mat temp;
	imageProcessing.RotateImage(image_j, temp, 1);
	image_j = image_r = temp;
	imshow(imageWindow, image_j);
}


void CBarCodeScannerDlg::OnRotate270()
{
	// TODO: 在此添加命令处理程序代码
	Mat temp;
	imageProcessing.RotateImage(image_j, temp, 3);
	image_j = image_r = temp;
	imshow(imageWindow, image_j);
}


void CBarCodeScannerDlg::OnRotate180()
{
	// TODO: 在此添加命令处理程序代码
	Mat temp;
	imageProcessing.RotateImage(image_j, temp, 2);
	image_j = image_r = temp;
	imshow(imageWindow, image_j);
}


