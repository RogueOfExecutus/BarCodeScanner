#pragma once
#include "afxwin.h"


// LoginDialog 对话框

class LoginDialog : public CDialogEx
{
	DECLARE_DYNAMIC(LoginDialog)

public:
	LoginDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~LoginDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
private:
	CComboBox userName;
	CEdit passWord;
public:
	virtual BOOL OnInitDialog();
private:
	int userLevel;
public:
	int GetUserLevel();
};
