// LoginDialog.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "BarCodeScanner.h"
#include "LoginDialog.h"
#include "afxdialogex.h"


// LoginDialog �Ի���

IMPLEMENT_DYNAMIC(LoginDialog, CDialogEx)

LoginDialog::LoginDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_LOGIN_DIALOG, pParent)
{

	userLevel = 0;
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_USER, userName);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, passWord);
}


BEGIN_MESSAGE_MAP(LoginDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &LoginDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// LoginDialog ��Ϣ�������


void LoginDialog::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString pass;
	passWord.GetWindowTextW(pass);
	bool isPass = false;
	userLevel = userName.GetCurSel();
	switch (userLevel)
	{
	case 0:
		isPass = true;
		break;
	case 1:
		if (pass == _T("xtd10086"))
			isPass = true;
		break;
	case 2:
		if (pass == _T("nvt10001"))
			isPass = true;
		break;
	default:
		break;
	}
	if (isPass)
		CDialogEx::OnOK();
	else
	{
		CString str;
		str.LoadStringW(IDS_STRING_USER_ERR);
		MessageBox(str);
	}
}


BOOL LoginDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	userName.InsertString(0, _T("admin"));
	userName.InsertString(1, _T("xtd"));
	userName.InsertString(2, _T("nvt"));
	userName.SetCurSel(0);

	passWord.SetWindowTextW(_T("12345"));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


int LoginDialog::GetUserLevel()
{
	return userLevel;
}
