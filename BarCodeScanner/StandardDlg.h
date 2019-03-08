#pragma once
#include "afxwin.h"
#include "ImageProcessing.h"


// StandardDlg 对话框

class StandardDlg : public CDialogEx
{
	DECLARE_DYNAMIC(StandardDlg)

public:
	StandardDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~StandardDlg();
	virtual BOOL OnInitDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STANDARD_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	CComboBox batteryColor;
	CEdit codeAngle;
	CEdit codeHeightStandard;
	CEdit codeHeightRange;
	CEdit codeWidthStandard;
	CEdit codeWidthRange;
	CEdit heightStandard;
	CEdit heightRange;
	CEdit widthStandard;
	CEdit widthRange;
	CEdit erosionSize;
	CEdit erosionTimes;
	CEdit morSize;
	CEdit morTimes;
	LPCTSTR parameter;
	configForCode code;
	codeStandardConfig standard;
	void initConfig();
	bool writeConfig();
public:
	afx_msg void OnBnClickedOk();
private:
	CEdit cycleSize;
	CEdit cycleTimes;
	CEdit codeHeightOffset;
	CEdit codeWidthOffset;
	CEdit heightOffset;
	CEdit widthOffset;
	configForOffset offset;
	CEdit codeOffset;
	CButton isSurf;
};
