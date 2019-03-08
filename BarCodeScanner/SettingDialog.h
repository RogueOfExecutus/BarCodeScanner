#pragma once

#include "BarCodeScannerDlg.h"
#include "afxwin.h"

// SettingDialog 对话框

class SettingDialog : public CDialogEx
{
	DECLARE_DYNAMIC(SettingDialog)

public:
	SettingDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~SettingDialog();

	virtual BOOL OnInitDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SET_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	configForLine rll;
	configForLine udl;
	configForCode code;
	configForProject proj;
	codeStandardConfig standard;
	CComboBox codeTypeList;
	CComboBox cornerList;
	CComboBox logLevel;
	CComboBox comList;
	CButton debugCheck;
	CEdit codeRect;
	CEdit codeThreshold;
	CEdit RLlineRect;
	CEdit UDlineRect;
	CButton RLlineDirection;
	CEdit RLlineOffset;
	CButton RLlineSame;
	CEdit RLlineThreshold;
	CButton UDlineDirection;
	CEdit UDlineOffset;
	CButton UDlineSame;
	CEdit UDlineThreshold;
	CEdit xPixel;
	CEdit yPixel;
	void initConfig();
	LPCTSTR parameter;
	CString portName;
	bool writeConfig();
	int getPCPort(std::vector<CString>& Coms);
	void initMSComm();
	CEdit RLlineThreshold1;
	CEdit RLlineThreshold2;
	CEdit UDlineThreshold1;
	CEdit UDlineThreshold2;
	CEdit UDlineAdapKernel;
	CEdit RLlineAdapKernel;
	CComboBox RLlineMethod;
	CComboBox UDlineMethod;
	CEdit RLlineAdapOffset;
	CEdit UDlineAdapOffset;
	CComboBox versionAngle;
	CEdit versionRect;
	CEdit versionString;
	CButton versionEnable;
	configForVersion version;
	CString VersionStr;
	ImageProcessing processing;
public:
	static std::vector<CString> SplitCString(CString strSource, CString ch);
	afx_msg void OnBnClickedOk();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonLearn();
};
