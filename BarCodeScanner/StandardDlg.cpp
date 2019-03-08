// StandardDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BarCodeScanner.h"
#include "StandardDlg.h"
#include "afxdialogex.h"


// StandardDlg 对话框

IMPLEMENT_DYNAMIC(StandardDlg, CDialogEx)

StandardDlg::StandardDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_STANDARD_DIALOG, pParent)
{

}

StandardDlg::~StandardDlg()
{
}

void StandardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_COLOR, batteryColor);
	DDX_Control(pDX, IDC_EDIT_ANGEL, codeAngle);
	DDX_Control(pDX, IDC_EDIT_CODE_HEIGHT, codeHeightStandard);
	DDX_Control(pDX, IDC_EDIT_CODE_HEIGHT_RANGE, codeHeightRange);
	DDX_Control(pDX, IDC_EDIT_CODE_WIDTH, codeWidthStandard);
	DDX_Control(pDX, IDC_EDIT_CODE_WIDTH_RANGE, codeWidthRange);
	DDX_Control(pDX, IDC_EDIT_HEIGHT, heightStandard);
	DDX_Control(pDX, IDC_EDIT_HEIGHT_RANGE, heightRange);
	DDX_Control(pDX, IDC_EDIT_WIDTH, widthStandard);
	DDX_Control(pDX, IDC_EDIT_WIDTH_RANGE, widthRange);
	DDX_Control(pDX, IDC_EROSION_SIZE, erosionSize);
	DDX_Control(pDX, IDC_EROSION_TIMES, erosionTimes);
	DDX_Control(pDX, IDC_MOR_SIZE, morSize);
	DDX_Control(pDX, IDC_MOR_TIMES, morTimes);
	DDX_Control(pDX, IDC_CYCLE_SIZE, cycleSize);
	DDX_Control(pDX, IDC_CYCLE_TIMES, cycleTimes);
	DDX_Control(pDX, IDC_EDIT_CODE_HEIGHT_OFFSET, codeHeightOffset);
	DDX_Control(pDX, IDC_EDIT_CODE_WIDTH_OFFSET, codeWidthOffset);
	DDX_Control(pDX, IDC_EDIT_HEIGHT_OFFSET, heightOffset);
	DDX_Control(pDX, IDC_EDIT_WIDTH_OFFSET, widthOffset);
	DDX_Control(pDX, IDC_EDIT_CODE_OFFSET, codeOffset);
	DDX_Control(pDX, IDC_CHECK_SURF, isSurf);
}


BEGIN_MESSAGE_MAP(StandardDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &StandardDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// StandardDlg 消息处理程序


BOOL StandardDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	parameter = _T(".\\Parameter.ini");
	// TODO:  在此添加额外的初始化
	CFileFind finder;   //查找是否存在ini文件，若不存在，则生成一个新的默认设置的ini文件，这样就保证了我们更改后的设置每次都可用   
	if (finder.FindFile(parameter))
	{
		GetPrivateProfileStruct(_T("code"), _T("codeConfig"), &code, sizeof(configForCode), parameter);
		GetPrivateProfileStruct(_T("project"), _T("codeStandardConfig"), &standard, sizeof(codeStandardConfig), parameter);
		GetPrivateProfileStruct(_T("project"), _T("offsetConfig"), &offset, sizeof(configForOffset), parameter);
	}
	initConfig();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void StandardDlg::initConfig()
{
	CString temp;
	temp.Format(_T("%.2lf"), (standard.highAngle - standard.lowAngle) / 2);
	codeAngle.SetWindowText(temp);
	temp.Format(_T("%.3lf"), (standard.highCodeWidth + standard.lowCodeWidth) / 2);
	codeWidthStandard.SetWindowText(temp);
	temp.Format(_T("%.3lf"), (standard.highCodeWidth - standard.lowCodeWidth) / 2);
	codeWidthRange.SetWindowText(temp);
	temp.Format(_T("%.3lf"), (standard.highCodeHeight + standard.lowCodeHeight) / 2);
	codeHeightStandard.SetWindowText(temp);
	temp.Format(_T("%.3lf"), (standard.highCodeHeight - standard.lowCodeHeight) / 2);
	codeHeightRange.SetWindowText(temp);
	temp.Format(_T("%.3lf"), (standard.highWidth + standard.lowWidth) / 2);
	widthStandard.SetWindowText(temp);
	temp.Format(_T("%.3lf"), (standard.highWidth - standard.lowWidth) / 2);
	widthRange.SetWindowText(temp);
	temp.Format(_T("%.3lf"), (standard.highHeight + standard.lowHeight) / 2);
	heightStandard.SetWindowText(temp);
	temp.Format(_T("%.3lf"), (standard.highHeight - standard.lowHeight) / 2);
	heightRange.SetWindowText(temp);
	temp.LoadStringW(IDS_STRING_WHITE_BATTERY);
	batteryColor.InsertString(0, temp);
	temp.LoadStringW(IDS_STRING_BLACK_BATTERY);
	batteryColor.InsertString(1, temp);
	batteryColor.SetCurSel(code.blackOrWhite);
	temp.Format(_T("%d"), code.ErosionTimes);
	erosionTimes.SetWindowText(temp);
	temp.Format(_T("%d"), code.ErosionSize);
	erosionSize.SetWindowText(temp);
	temp.Format(_T("%d"), code.MorphologyExTimes);
	morTimes.SetWindowText(temp);
	temp.Format(_T("%d"), code.MorphologyExSize);
	morSize.SetWindowText(temp);
	temp.Format(_T("%d"), code.cycleTimes);
	cycleTimes.SetWindowText(temp);
	temp.Format(_T("%d"), code.cycleSize);
	cycleSize.SetWindowText(temp);
	isSurf.SetCheck(code.isSurf);

	temp.Format(_T("%.3lf"), offset.codeHeightOffset);
	codeHeightOffset.SetWindowText(temp);
	temp.Format(_T("%.3lf"), offset.codeWidthOffset);
	codeWidthOffset.SetWindowText(temp);
	temp.Format(_T("%.3lf"), offset.heightOffset);
	heightOffset.SetWindowText(temp);
	temp.Format(_T("%.3lf"), offset.widthOffset);
	widthOffset.SetWindowText(temp);
	temp.Format(_T("%d"), offset.codeOffset);
	codeOffset.SetWindowText(temp);
}


void StandardDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!writeConfig())
	{
		CString str;
		str.LoadStringW(IDS_STRING_ERR_CONFIG_TIPS);
		MessageBox(str);
		return;
	}
	CDialogEx::OnOK();
}


bool StandardDlg::writeConfig()
{
	try
	{
		CString temp;
		codeAngle.GetWindowText(temp);
		standard.highAngle = 90.0 + _ttof(temp);
		standard.lowAngle = 90.0 - _ttof(temp);
		codeWidthStandard.GetWindowText(temp);
		double s = _ttof(temp);
		codeWidthRange.GetWindowText(temp);
		double r = _ttof(temp);
		standard.highCodeWidth = s + r;
		standard.lowCodeWidth = s - r;

		codeHeightStandard.GetWindowText(temp);
		s = _ttof(temp);
		codeHeightRange.GetWindowText(temp);
		r = _ttof(temp);
		standard.highCodeHeight = s + r;
		standard.lowCodeHeight = s - r;

		widthStandard.GetWindowText(temp);
		s = _ttof(temp);
		widthRange.GetWindowText(temp);
		r = _ttof(temp);
		standard.highWidth = s + r;
		standard.lowWidth = s - r;

		heightStandard.GetWindowText(temp);
		s = _ttof(temp);
		heightRange.GetWindowText(temp);
		r = _ttof(temp);
		standard.highHeight = s + r;
		standard.lowHeight = s - r;

		code.blackOrWhite = batteryColor.GetCurSel();
		erosionTimes.GetWindowText(temp);
		code.ErosionTimes = _ttoi(temp);
		erosionSize.GetWindowText(temp);
		code.ErosionSize = _ttoi(temp);
		morTimes.GetWindowText(temp);
		code.MorphologyExTimes = _ttoi(temp);
		morSize.GetWindowText(temp);
		code.MorphologyExSize = _ttoi(temp);
		cycleTimes.GetWindowText(temp);
		code.cycleTimes = _ttoi(temp);
		cycleSize.GetWindowText(temp);
		code.cycleSize = _ttoi(temp);
		code.isSurf = (isSurf.GetCheck() != 0);

		codeHeightOffset.GetWindowText(temp);
		offset.codeHeightOffset = _ttof(temp);
		codeWidthOffset.GetWindowText(temp);
		offset.codeWidthOffset = _ttof(temp);
		heightOffset.GetWindowText(temp);
		offset.heightOffset = _ttof(temp);
		widthOffset.GetWindowText(temp);
		offset.widthOffset = _ttof(temp);
		codeOffset.GetWindowText(temp);
		offset.codeOffset = _ttoi(temp);

		WritePrivateProfileStruct(_T("code"), _T("codeConfig"), &code, sizeof(configForCode), parameter);
		WritePrivateProfileStruct(_T("project"), _T("codeStandardConfig"), &standard, sizeof(codeStandardConfig), parameter);
		WritePrivateProfileStruct(_T("project"), _T("offsetConfig"), &offset, sizeof(configForOffset), parameter);
	}
	catch (...)
	{
		return false;
	}
	return true;
}
