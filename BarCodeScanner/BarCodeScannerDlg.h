
// BarCodeScannerDlg.h : ͷ�ļ�
//

#pragma once

#include "ImageProcessing.h"
#include <mutex>
#include <condition_variable>
#include "afxwin.h"
#include "CMSComm.h"
#include "MyPictureControl.h"
#include "MyLogger.h"

// CBarCodeScannerDlg �Ի���
class CBarCodeScannerDlg : public CDialogEx
{
// ����
public:
	CBarCodeScannerDlg(CWnd* pParent = NULL);	// ��׼���캯��x

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BARCODESCANNER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnUpdataUi(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonConfig();
	afx_msg void OnBnClickedButtonCodeStandard();
	afx_msg void OnBnClickedButtonLog();
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnBnClickedButtonAbout();
	afx_msg void OnBnClickedButtonManualGrab();
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint point);
	afx_msg void OnCopyCoordinate();
private:
	std::string OpenImageFile();
	CString PickImageFile();
	void recursionImageFolder(CString folder, std::vector<CString>& paths);
	void initLog();
	void initConfig();
	void initSurf();
	int CodeJudgement(const cv::Mat& image_r, cv::Mat& J, std::string& data);
	void handleCodeLine(const cv::Mat & I, cv::Mat & J, cv::Point cp1, cv::Point cp2, bool isRight, double & Len, double & CLen);
	cv::Point FindMidPoint(cv::Point p1, cv::Point p2, int x_offset, int y_offset);
	double GetLineLength(cv::Point p1, cv::Point p2, double pixel_scale_x, double pixel_scale_y);
	bool JudgeAndDrawLength(cv::Mat & J, double len, double high, double low, std::string start, int offset);
	double GetTheta(cv::Point p1, cv::Point p2, cv::Point p3, cv::Point p4);
	cv::Point PointToLineDist(cv::Point p, cv::Point p1, cv::Point p2);
	void DeleteLogAndImageThread();
	void multipleTestThread();
	void SaveImageThread();
	void OmronThread();
	void CameraThread();
	// �汾ʶ���߳�
	void VersionThread();
	cv::String imageWindow;
	cv::String codeWindow;
	ImageProcessing imageProcessing;
	configForLine RLLineConfig;
	configForLine UDLineConfig;
	configForCode codeConfig;
	configForProject projectConfig;
	codeStandardConfig standardConfig;
	configForClear clearConfig;
	configForCount countRecord;
	configForOffset offsetConfig;
	configForVersion versionConfig;
	LPCTSTR checkTrigger;
	LPCTSTR resetTrigger;
	//LPCTSTR sendReset;
	LPCTSTR sendOK;
	LPCTSTR sendNG;
	LPCTSTR versionNG;
	CString portName;
	CString cameraName;
	CString versionSTR;
	std::condition_variable openCamera;
	std::condition_variable grabImage;
	std::condition_variable saveImage;
	std::condition_variable testImage;
	std::condition_variable procesImage;
	std::mutex mtxCamera;
	std::mutex mtxImage;
	std::mutex mtxSaveImage;
	bool cameraRunning = false;
	bool cameraIsOpen = false;
	bool plcGrabFlag = false;
	bool savingImage = false;
	// �������Ա��
	bool testingImage = false;
	// �������Ա���ͼƬ���
	bool testingFlag = false;
	int imageResType;
	std::string newBarcode;
	CComboBox cameraList;
	LPCTSTR imageParameter;
	void ShowForStop();
	void HideForStart();
	CButton codeStandard;
	CButton configButton;
	CButton manualGrab;
	CButton startOrStop;
	CButton testButton;
	CMSComm omronPlc;
	// �򿪴���
	bool openMSComm();
	cv::Scalar red;
	cv::Scalar green;
	cv::Point s_pt;
	cv::Point e_pt;
	CEdit showMSG;
	std::vector<cv::Mat> imageList;
	std::vector<CString> versions;
	CRect buttonRect[31];
	void ReSizeAll();
	void ReSizeImage(int right_x, int y_offset);
	void RecordSize();
	CButton aboutBtn;
	CButton logBtn;
	MyPictureControl imageShowWindow;
	CStatic staticOne;
	CStatic staticTwo;
	CStatic staticThree;
	CStatic staticFour;
	CStatic staticFive;
	CStatic staticSix;
	CEdit countAll;
	CEdit countNG;
	CEdit countOK;
	CEdit countVersionNG;
	CEdit countYield;
	CStatic staticEight;
	CStatic staticSeven;
	CEdit sizeNG;
	CEdit positionNG;
	CEdit angleNG;
	CEdit otherNG;
	CStatic staticNine;
	CStatic staticTen;
	POINT Oldp;
	float r_image_msg;
	void AddLogMSG(CString msg, CString file);
	void AddLogMSG(std::string msg, std::string file);
	void checkAppFile();
	CButton resetCounting;
	void updateCounting();
	void counting(int res);
	int userLevel;
	void HideForLevel();
	bool isCtrl;
	void RedrawAll();
	void checkFile(const char* fileName);
	// �ݹ�ɾ���ļ���
	void DeleteDirectory(CString directoryPath);
	// ����ļ��д���ʱ��
	bool CheckFolderCreateTime(CString FolderPath, CTime deletetime);
	// ɾ����־��ͼƬ��¼
	void CheckAndDeleteDirectory(LPCTSTR path);
	int omronFlag;
	bool recTrigger = false;
	cv::Mat image_r;
	std::vector<CString> allImagePath;
	MyLogger *myLogger;
	log4cplus::Initializer initializer;
public:
	cv::Mat image_j;
	void readConfig();
	void OnCommOmronPlc();
	void DrawRectangle();
	void SetSPt(int x, int y);
	cv::Point GetSPt();
	void SetEPt(int x, int y);
	cv::Point GetEPt();
	void AddShowMSG(CString msg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedCountReset();
	void SetUserLevel(int level);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTestThreshold();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnTestCanny();
	afx_msg void OnTestAdaptiveThreshold();
	afx_msg void OnCutMat();
	afx_msg void OnSaveImage();
	afx_msg void OnRotate90();
	afx_msg void OnRotate270();
	afx_msg void OnRotate180();
};
