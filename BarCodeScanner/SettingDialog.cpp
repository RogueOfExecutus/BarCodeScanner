// SettingDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "BarCodeScanner.h"
#include "SettingDialog.h"
#include "afxdialogex.h"
#include "BarCodeScannerDlg.h"

using namespace std;
// SettingDialog 对话框

IMPLEMENT_DYNAMIC(SettingDialog, CDialogEx)

SettingDialog::SettingDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SET_DIALOG, pParent)
{
	VersionStr = _T("");
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CODE_TYPE, codeTypeList);
	DDX_Control(pDX, IDC_COMBO_CORNER, cornerList);
	DDX_Control(pDX, IDC_COMBO_LOG, logLevel);
	DDX_Control(pDX, IDC_COMBO_MSCOMM, comList);
	DDX_Control(pDX, IDC_DEBUG, debugCheck);
	DDX_Control(pDX, IDC_EDIT_CODE_RECT, codeRect);
	DDX_Control(pDX, IDC_CODE_THRESHOLD, codeThreshold);
	DDX_Control(pDX, IDC_EDIT_RLINE_RECT, RLlineRect);
	DDX_Control(pDX, IDC_EDIT_ULINE_RECT, UDlineRect);
	DDX_Control(pDX, IDC_RLINE_DIRECTION, RLlineDirection);
	DDX_Control(pDX, IDC_RLINE_OFFSET, RLlineOffset);
	DDX_Control(pDX, IDC_RLINE_SAME, RLlineSame);
	DDX_Control(pDX, IDC_RLINE_THRESHOLD, RLlineThreshold);
	DDX_Control(pDX, IDC_ULINE_DIRECTION, UDlineDirection);
	DDX_Control(pDX, IDC_ULINE_OFFSET, UDlineOffset);
	DDX_Control(pDX, IDC_ULINE_SAME, UDlineSame);
	DDX_Control(pDX, IDC_ULINE_THRESHOLD, UDlineThreshold);
	DDX_Control(pDX, IDC_X_PIXEL, xPixel);
	DDX_Control(pDX, IDC_Y_PIXEL, yPixel);
	//  DDX_Control(pDX, IDC_RLINE_ONLY_CANNY, useThreshR);
	DDX_Control(pDX, IDC_RLINE_THRESHOLD1, RLlineThreshold1);
	DDX_Control(pDX, IDC_RLINE_THRESHOLD2, RLlineThreshold2);
	//  DDX_Control(pDX, IDC_ULINE_ONLY_CANNY, useThreshU);
	DDX_Control(pDX, IDC_ULINE_THRESHOLD1, UDlineThreshold1);
	DDX_Control(pDX, IDC_ULINE_THRESHOLD2, UDlineThreshold2);
	DDX_Control(pDX, IDC_ULINE_ADAPTIVE_KERNEL, UDlineAdapKernel);
	DDX_Control(pDX, IDC_RLINE_ADAPTIVE_KERNEL, RLlineAdapKernel);
	DDX_Control(pDX, IDC_COMBO_RLINE_METHOD, RLlineMethod);
	DDX_Control(pDX, IDC_COMBO_ULINE_METHOD, UDlineMethod);
	DDX_Control(pDX, IDC_RLINE_ADAPTIVE_OFFSET, RLlineAdapOffset);
	DDX_Control(pDX, IDC_ULINE_ADAPTIVE_OFFSET, UDlineAdapOffset);
	DDX_Control(pDX, IDC_COMBO_VERSION_ANGLE, versionAngle);
	DDX_Control(pDX, IDC_EDIT_VERSION_RECT, versionRect);
	DDX_Control(pDX, IDC_EDIT_VERSION_STR, versionString);
	DDX_Control(pDX, IDC_VERSION_ENABLE, versionEnable);
}


BEGIN_MESSAGE_MAP(SettingDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &SettingDialog::OnBnClickedOk)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &SettingDialog::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_LEARN, &SettingDialog::OnBnClickedButtonLearn)
END_MESSAGE_MAP()


// SettingDialog 消息处理程序


BOOL SettingDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	parameter = _T(".\\Parameter.ini");
	// TODO:  在此添加额外的初始化
	CFileFind finder;   //查找是否存在ini文件，若不存在，则生成一个新的默认设置的ini文件，这样就保证了我们更改后的设置每次都可用   
	if (finder.FindFile(parameter))
	{
		GetPrivateProfileStruct(_T("line"), _T("RLLineConfig"), &rll, sizeof(configForLine), parameter);
		GetPrivateProfileStruct(_T("line"), _T("UDLineConfig"), &udl, sizeof(configForLine), parameter);
		GetPrivateProfileStruct(_T("code"), _T("codeConfig"), &code, sizeof(configForCode), parameter);
		GetPrivateProfileStruct(_T("project"), _T("projectConfig"), &proj, sizeof(configForProject), parameter);
		GetPrivateProfileStruct(_T("project"), _T("codeStandardConfig"), &standard, sizeof(codeStandardConfig), parameter);
		GetPrivateProfileStruct(_T("project"), _T("versionConfig"), &version, sizeof(configForVersion), parameter);
		GetPrivateProfileString(_T("communication"), _T("port"), _T("COM1"), portName.GetBuffer(7), 7, parameter);
		GetPrivateProfileString(_T("version"), _T("string"), _T(""), VersionStr.GetBuffer(30), 30, parameter);
		//cameraName.ReleaseBuffer();
		portName.ReleaseBuffer();
		VersionStr.ReleaseBuffer();
	}
	else
	{
		CString str;
		str.LoadStringW(IDS_STRING_CONFIG_FILE_ERR);
		MessageBox(str);
		return TRUE;
	}
	initMSComm();
	initConfig();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void SettingDialog::initConfig()
{
	CString temp;
	codeTypeList.InsertString(0, _T("QR Code"));
	codeTypeList.InsertString(1, _T("Data Matrix"));
	codeTypeList.SetCurSel(proj.codeType);
	debugCheck.SetCheck(proj.isDebug);
	temp.Format(_T("%lf"), proj.pixel_scale_x);
	xPixel.SetWindowText(temp);
	temp.Format(_T("%lf"), proj.pixel_scale_y);
	yPixel.SetWindowText(temp);

	logLevel.InsertString(0, _T("TRACE"));
	logLevel.InsertString(1, _T("DEBUG"));
	logLevel.InsertString(2, _T("INFO"));
	logLevel.InsertString(3, _T("WARN"));
	logLevel.InsertString(4, _T("ERROR"));
	logLevel.InsertString(5, _T("FATAL"));
	logLevel.SetCurSel(proj.logLevel);

	temp.Format(_T("%d,%d,%d,%d"), proj.rect.x, proj.rect.y, proj.rect.width, proj.rect.height);
	codeRect.SetWindowText(temp);
	temp.Format(_T("%d"), code.threshold);
	codeThreshold.SetWindowText(temp);
	temp.LoadStringW(IDS_STRING_LEFT_BOTTOM);
	cornerList.InsertString(0, temp);
	temp.LoadStringW(IDS_STRING_RIGHT_BOTTOM);
	cornerList.InsertString(1, temp);
	temp.LoadStringW(IDS_STRING_RIGHT_TOP);
	cornerList.InsertString(2, temp);
	temp.LoadStringW(IDS_STRING_LEFT_TOP);
	cornerList.InsertString(3, temp);
	cornerList.SetCurSel(proj.corner);

	temp.Format(_T("%d,%d,%d,%d"), rll.rect.x, rll.rect.y, rll.rect.width, rll.rect.height);
	RLlineRect.SetWindowText(temp);
	RLlineDirection.SetCheck(rll.line_direction);
	temp.Format(_T("%d"), rll.threshold);
	RLlineThreshold.SetWindowText(temp);
	temp.Format(_T("%d"), rll.offset);
	RLlineOffset.SetWindowText(temp);
	temp.Format(_T("%.1lf"), rll.threshold1);
	RLlineThreshold1.SetWindowText(temp);
	temp.Format(_T("%.1lf"), rll.threshold2);
	RLlineThreshold2.SetWindowText(temp);
	RLlineSame.SetCheck(rll.same);
	temp.LoadStringW(IDS_STRING_METHOD_THRESHOLD);
	RLlineMethod.InsertString(0, temp);
	temp.LoadStringW(IDS_STRING_METHOD_CANNY);
	RLlineMethod.InsertString(1, temp);
	temp.LoadStringW(IDS_STRING_METHOD_ADAPTIVETHRESHOLD);
	RLlineMethod.InsertString(2, temp);
	RLlineMethod.SetCurSel(rll.method);
	temp.Format(_T("%d"), rll.adaptiveKernel);
	RLlineAdapKernel.SetWindowText(temp);
	temp.Format(_T("%d"), rll.adaptiveOffset);
	RLlineAdapOffset.SetWindowText(temp);

	//useThreshR.SetCheck(rll.threshold);

	temp.Format(_T("%d,%d,%d,%d"), udl.rect.x, udl.rect.y, udl.rect.width, udl.rect.height);
	UDlineRect.SetWindowText(temp);
	UDlineDirection.SetCheck(udl.line_direction);
	temp.Format(_T("%d"), udl.threshold);
	UDlineThreshold.SetWindowText(temp);
	temp.Format(_T("%d"), udl.offset);
	UDlineOffset.SetWindowText(temp);
	temp.Format(_T("%.1lf"), udl.threshold1);
	UDlineThreshold1.SetWindowText(temp);
	temp.Format(_T("%.1lf"), udl.threshold2);
	UDlineThreshold2.SetWindowText(temp);
	UDlineSame.SetCheck(udl.same);
	temp.LoadStringW(IDS_STRING_METHOD_THRESHOLD);
	UDlineMethod.InsertString(0, temp);
	temp.LoadStringW(IDS_STRING_METHOD_CANNY);
	UDlineMethod.InsertString(1, temp);
	temp.LoadStringW(IDS_STRING_METHOD_ADAPTIVETHRESHOLD);
	UDlineMethod.InsertString(2, temp);
	UDlineMethod.SetCurSel(udl.method);
	temp.Format(_T("%d"), udl.adaptiveKernel);
	UDlineAdapKernel.SetWindowText(temp);
	temp.Format(_T("%d"), udl.adaptiveOffset);
	UDlineAdapOffset.SetWindowText(temp);
	//useThreshU.SetCheck(udl.threshold);

	versionEnable.SetCheck(version.enable);
	temp.Format(_T("%d,%d,%d,%d"), version.rect.x, version.rect.y, version.rect.width, version.rect.height);
	versionRect.SetWindowText(temp);
	versionAngle.InsertString(0, _T("0"));
	versionAngle.InsertString(1, _T("90"));
	versionAngle.InsertString(2, _T("180"));
	versionAngle.InsertString(3, _T("270"));
	versionAngle.SetCurSel(version.rotateAngle);

	versionString.SetWindowText(VersionStr);
}


void SettingDialog::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!writeConfig())
	{
		CString str;
		str.LoadStringW(IDS_STRING_ERR_CONFIG_TIPS);
		MessageBox(str);
		return;
	}
	GetParent()->GetDlgItem(IDC_BUTTON_CONFIG)->EnableWindow();
	CDialogEx::OnOK();
}


bool SettingDialog::writeConfig()
{
	try
	{
		CString temp;
		codeRect.GetWindowText(temp);
		vector<CString> coder = SplitCString(temp, _T(","));
		if (coder.size() != 4)
			return false;
		proj.rect.x = _ttoi(coder[0]);
		proj.rect.y = _ttoi(coder[1]);
		proj.rect.width = _ttoi(coder[2]);
		proj.rect.height = _ttoi(coder[3]);

		RLlineRect.GetWindowText(temp);
		vector<CString> rlr = SplitCString(temp, _T(","));
		if (rlr.size() != 4)
			return false;
		rll.rect.x = _ttoi(rlr[0]);
		rll.rect.y = _ttoi(rlr[1]);
		rll.rect.width = _ttoi(rlr[2]);
		rll.rect.height = _ttoi(rlr[3]);

		UDlineRect.GetWindowText(temp);
		vector<CString> udr = SplitCString(temp, _T(","));
		if (udr.size() != 4)
			return false;
		udl.rect.x = _ttoi(udr[0]);
		udl.rect.y = _ttoi(udr[1]);
		udl.rect.width = _ttoi(udr[2]);
		udl.rect.height = _ttoi(udr[3]);

		versionRect.GetWindowText(temp);
		vector<CString> ver = SplitCString(temp, _T(","));
		if (ver.size() != 4)
			return false;
		version.rect.x = _ttoi(ver[0]);
		version.rect.y = _ttoi(ver[1]);
		version.rect.width = _ttoi(ver[2]);
		version.rect.height = _ttoi(ver[3]);

		proj.isDebug = (debugCheck.GetCheck() != 0);
		comList.GetWindowText(portName);
		proj.codeType = codeTypeList.GetCurSel();
		xPixel.GetWindowText(temp);
		proj.pixel_scale_x = _ttof(temp);
		yPixel.GetWindowText(temp);
		proj.pixel_scale_y = _ttof(temp);

		proj.logLevel = logLevel.GetCurSel();

		codeThreshold.GetWindowText(temp);
		code.threshold = _ttoi(temp);
		proj.corner = cornerList.GetCurSel();

		rll.line_direction = (RLlineDirection.GetCheck() != 0);
		
		RLlineThreshold.GetWindowText(temp);
		rll.threshold = _ttoi(temp);
		RLlineOffset.GetWindowText(temp);
		rll.offset = _ttoi(temp);
		RLlineThreshold1.GetWindowText(temp);
		rll.threshold1 = _ttof(temp);
		RLlineThreshold2.GetWindowText(temp);
		rll.threshold2 = _ttof(temp);
		rll.same = (RLlineSame.GetCheck() != 0);
		rll.method = RLlineMethod.GetCurSel();
		RLlineAdapKernel.GetWindowText(temp);
		rll.adaptiveKernel = _ttoi(temp);
		RLlineAdapOffset.GetWindowText(temp);
		rll.adaptiveOffset = _ttoi(temp);

		udl.line_direction = (UDlineDirection.GetCheck() != 0);
		
		UDlineThreshold.GetWindowText(temp);
		udl.threshold = _ttoi(temp);
		UDlineOffset.GetWindowText(temp);
		udl.offset = _ttoi(temp);
		UDlineThreshold1.GetWindowText(temp);
		udl.threshold1 = _ttof(temp);
		UDlineThreshold2.GetWindowText(temp);
		udl.threshold2 = _ttof(temp);
		udl.same = (UDlineSame.GetCheck() != 0);
		udl.method = UDlineMethod.GetCurSel();
		UDlineAdapKernel.GetWindowText(temp);
		udl.adaptiveKernel = _ttoi(temp);
		UDlineAdapOffset.GetWindowText(temp);
		udl.adaptiveOffset = _ttoi(temp);

		version.enable = (versionEnable.GetCheck() != 0);
		version.rotateAngle = versionAngle.GetCurSel();

		versionString.GetWindowText(VersionStr);
		version.strLen = VersionStr.GetLength();

		WritePrivateProfileStruct(_T("line"), _T("RLLineConfig"), &rll, sizeof(configForLine), parameter);
		WritePrivateProfileStruct(_T("line"), _T("UDLineConfig"), &udl, sizeof(configForLine), parameter);
		WritePrivateProfileStruct(_T("code"), _T("codeConfig"), &code, sizeof(configForCode), parameter);
		WritePrivateProfileStruct(_T("project"), _T("projectConfig"), &proj, sizeof(configForProject), parameter);
		WritePrivateProfileStruct(_T("project"), _T("codeStandardConfig"), &standard, sizeof(codeStandardConfig), parameter);
		WritePrivateProfileStruct(_T("project"), _T("versionConfig"), &version, sizeof(configForVersion), parameter);
		WritePrivateProfileString(_T("communication"), _T("port"), portName, parameter);
		WritePrivateProfileString(_T("version"), _T("string"), VersionStr, parameter);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

// 分割CString
vector<CString> SettingDialog::SplitCString(CString strSource, CString ch)
{
	vector <CString> vecString;

	int iPos = 0;

	CString strTmp = strSource.Tokenize(ch, iPos);

	while (strTmp.Trim() != _T(""))
	{
		vecString.push_back(strTmp);
		strTmp = strSource.Tokenize(ch, iPos);
	}
	return vecString;
}

int SettingDialog::getPCPort(vector<CString>& Coms)
{
	long lReg;
	HKEY hKey;
	DWORD MaxValueLength;
	DWORD dwValueNumber;

	lReg = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
		0, KEY_QUERY_VALUE, &hKey);

	if (lReg != ERROR_SUCCESS) //成功时返回ERROR_SUCCESS，
	{
		MessageBox(_T("Open Registry Error!"));
		return 0;
	}

	lReg = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL,
		&dwValueNumber, &MaxValueLength, NULL, NULL, NULL);

	if (lReg != ERROR_SUCCESS) //没有成功
	{
		MessageBox(_T("Getting Info Error!"));
		return 0;
	}

	TCHAR *pValueName, *pCOMNumber;
	DWORD cchValueName, dwValueSize = 254;

	Coms = vector<CString>(dwValueNumber);

	for (DWORD i = 0; i < dwValueNumber; i++)
	{
		cchValueName = MaxValueLength + 1;
		dwValueSize = 254;  //端口数
		pValueName = (TCHAR*)VirtualAlloc(NULL, cchValueName, MEM_COMMIT, PAGE_READWRITE);
		lReg = RegEnumValue(hKey, i, pValueName,
			&cchValueName, NULL, NULL, NULL, NULL);

		if ((lReg != ERROR_SUCCESS) && (lReg != ERROR_NO_MORE_ITEMS))
		{
			MessageBox(_T("Enum Registry Error or No More Items!"));
			return FALSE;
		}

		pCOMNumber = (TCHAR*)VirtualAlloc(NULL, 6, MEM_COMMIT, PAGE_READWRITE);
		lReg = RegQueryValueEx(hKey, pValueName, NULL,
			NULL, (LPBYTE)pCOMNumber, &dwValueSize);

		if (lReg != ERROR_SUCCESS)
		{
			MessageBox(_T("Can not get the name of the port"));
			return FALSE;
		}

		CString str(pCOMNumber);
		Coms[i] = str;
		VirtualFree(pValueName, 0, MEM_RELEASE);
		VirtualFree(pCOMNumber, 0, MEM_RELEASE);
	}

	return dwValueNumber;
}

// 串口初始化
void SettingDialog::initMSComm()
{
	vector<CString> coms;
	if (getPCPort(coms))
		for (size_t i = 0; i < coms.size(); i++)
			comList.AddString(coms[i]);
	comList.SetCurSel(comList.FindStringExact(0, portName));
}


void SettingDialog::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
}


void SettingDialog::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnClose();
}


void SettingDialog::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	GetParent()->GetDlgItem(IDC_BUTTON_CONFIG)->EnableWindow();
	CDialogEx::OnCancel();
}


void SettingDialog::OnBnClickedButtonLearn()
{
	// TODO: 在此添加控件通知处理程序代码
	processing.findAllWords(version, code, VersionStr);
	if (version.hasTemplate)
	{
		MessageBox(_T("success"));
	}
	else
	{
		MessageBox(_T("failed"));
	}
}
