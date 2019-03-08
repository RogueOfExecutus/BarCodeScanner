#pragma once
#include "afxwin.h"


// LoginDialog �Ի���

class LoginDialog : public CDialogEx
{
	DECLARE_DYNAMIC(LoginDialog)

public:
	LoginDialog(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~LoginDialog();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
