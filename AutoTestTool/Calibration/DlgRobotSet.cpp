// DlgRobotSet.cpp : 实现文件
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgRobotSet.h"
#include "afxdialogex.h"

#include "CalibrationDlg.h"
#include "libControl.h"

static char g_dbgBuf[500];
// CDlgRobotSet 对话框

IMPLEMENT_DYNAMIC(CDlgRobotSet, CDialogEx)

CDlgRobotSet::CDlgRobotSet(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgRobotSet::IDD, pParent)
{
}

CDlgRobotSet::~CDlgRobotSet()
{
	OutputDebugString("CDlgRobotSet exit");
}

void CDlgRobotSet::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEV_IPADDR, m_RobotIpAddr);
	DDX_Control(pDX, IDC_EDIT_UPX, m_EditUpX);
	DDX_Control(pDX, IDC_EDIT_UPY, m_EditUpY);
	DDX_Control(pDX, IDC_EDIT_DOWNX, m_EditDownX);
	DDX_Control(pDX, IDC_EDIT_DOWNY, m_EditDownY);
	DDX_Control(pDX, IDC_EDIT_LEFTX, m_EditLeftX);
	DDX_Control(pDX, IDC_EDIT_LEFTY, m_EditLeftY);
	DDX_Control(pDX, IDC_EDIT_RIGHTX, m_EditRightX);
	DDX_Control(pDX, IDC_EDIT_RIGHTY, m_EditRightY);
	DDX_Control(pDX, IDC_EDIT_CLICESPEED, m_EditClkSpeed);
	DDX_Control(pDX, IDC_EDIT_CLICKTIMES, m_EditClkTimes);
	DDX_Control(pDX, IDC_EDIT_DBCLICKFREQ, m_EditDBClkFreq);
	DDX_Control(pDX, IDC_EDIT_DBCLKDURATION, m_EditDBClkDuration);
	DDX_Control(pDX, IDC_CK_LASER, m_CkOpenLaser);
	DDX_Control(pDX, IDC_EDIT_TIMEBEFORE, m_EditTimeBefore);
	DDX_Control(pDX, IDC_EDIT_TIMEAFTER, m_EditTimeAfter);
	DDX_Control(pDX, IDC_CK_DRAGICON, m_CkDragIcon);
}


BEGIN_MESSAGE_MAP(CDlgRobotSet, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgRobotSet::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CK_LASER, &CDlgRobotSet::OnBnClickedCkLaser)
	ON_BN_CLICKED(IDC_BTN_SETMARK, &CDlgRobotSet::OnBnClickedBtnSetmark)
	ON_BN_CLICKED(IDCANCEL, &CDlgRobotSet::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgRobotSet 消息处理程序


void CDlgRobotSet::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	m_RobotIpAddr.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "IPADDR", str.GetBuffer(0), theApp.m_szConfigFile);

	m_EditUpX.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "UPX", str.GetBuffer(0), theApp.m_szConfigFile);
	m_EditUpY.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "UPY", str.GetBuffer(0), theApp.m_szConfigFile);
	m_EditDownX.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "DOWNX", str.GetBuffer(0), theApp.m_szConfigFile);
	m_EditDownY.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "DOWNY", str.GetBuffer(0), theApp.m_szConfigFile);

	m_EditLeftX.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "LEFTX", str.GetBuffer(0), theApp.m_szConfigFile);
	m_EditLeftY.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "LEFTY", str.GetBuffer(0), theApp.m_szConfigFile);
	m_EditRightX.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "RIGHTX", str.GetBuffer(0), theApp.m_szConfigFile);
	m_EditRightY.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "RIGHTY", str.GetBuffer(0), theApp.m_szConfigFile);

	m_EditClkSpeed.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "CLICKSPEED", str.GetBuffer(0), theApp.m_szConfigFile);
	m_EditClkTimes.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "CLICKTIMES", str.GetBuffer(0), theApp.m_szConfigFile);
	m_EditDBClkFreq.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "DBCLICKFREQ", str.GetBuffer(0), theApp.m_szConfigFile);
	m_EditDBClkDuration.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "DBCLICKDURATION", str.GetBuffer(0), theApp.m_szConfigFile);

	if(m_CkOpenLaser.GetCheck()){
		WritePrivateProfileString("ROBOT", "LASEROPEN", "1", theApp.m_szConfigFile);
	}else{
		WritePrivateProfileString("ROBOT", "LASEROPEN", "0", theApp.m_szConfigFile);
	}

	if(m_CkDragIcon.GetCheck())
	{
		WritePrivateProfileString("ROBOT", "DRAGICON", "1", theApp.m_szConfigFile);
		m_EditTimeBefore.GetWindowText(str);
		WritePrivateProfileString("ROBOT", "TIMEBEFORE", str.GetBuffer(0), theApp.m_szConfigFile);
		m_EditTimeAfter.GetWindowText(str);
		WritePrivateProfileString("ROBOT", "TIMEAFTER", str.GetBuffer(0), theApp.m_szConfigFile);
	}else{
		WritePrivateProfileString("ROBOT", "DRAGICON", "0", theApp.m_szConfigFile);
	}

#if USE_MODALSETDLG
	CDialogEx::OnOK();
#else
	DestroyWindow();
	::SendMessage(g_pMainDlg->m_hWnd, ROBOTDLG_EXIT, 0, 0);
#endif
}


BOOL CDlgRobotSet::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	char chTemp[200];
	GetPrivateProfileString("ROBOT", "IPADDR", "192.168.1.144", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_RobotIpAddr.SetAddress(ntohl(inet_addr(chTemp)));

	GetPrivateProfileString("ROBOT", "UPX", "1000", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditUpX.SetWindowText(chTemp);
	GetPrivateProfileString("ROBOT", "UPY", "2000", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditUpY.SetWindowText(chTemp);
	GetPrivateProfileString("ROBOT", "DOWNX", "1000", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditDownX.SetWindowText(chTemp);
	GetPrivateProfileString("ROBOT", "DOWNY", "1000", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditDownY.SetWindowText(chTemp);

	GetPrivateProfileString("ROBOT", "LEFTX", "1000", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditLeftX.SetWindowText(chTemp);
	GetPrivateProfileString("ROBOT", "LEFTY", "1000", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditLeftY.SetWindowText(chTemp);
	GetPrivateProfileString("ROBOT", "RIGHTX", "2000", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditRightX.SetWindowText(chTemp);
	GetPrivateProfileString("ROBOT", "RIGHTY", "1000", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditRightY.SetWindowText(chTemp);

	GetPrivateProfileString("ROBOT", "CLICKSPEED", "5", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditClkSpeed.SetWindowText(chTemp);
	GetPrivateProfileString("ROBOT", "CLICKTIMES", "1", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditClkTimes.SetWindowText(chTemp);
	GetPrivateProfileString("ROBOT", "DBCLICKFREQ", "5", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditDBClkFreq.SetWindowText(chTemp);
	GetPrivateProfileString("ROBOT", "DBCLICKDURATION", "5", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditDBClkDuration.SetWindowText(chTemp);

	GetPrivateProfileString("ROBOT", "TIMEBEFORE", "5", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditTimeBefore.SetWindowText(chTemp);
	GetPrivateProfileString("ROBOT", "TIMEAFTER", "5", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_EditTimeAfter.SetWindowText(chTemp);

	int nOpenLaser = GetPrivateProfileInt("ROBOT", "LASEROPEN", 1, theApp.m_szConfigFile);
	m_CkOpenLaser.SetCheck(nOpenLaser);

#if 0
	int nDragIcon = GetPrivateProfileInt("ROBOT", "DRAGICON", 0, theApp.m_szConfigFile);
	m_CkDragIcon.SetCheck(nDragIcon);
	m_EditTimeBefore.EnableWindow(nDragIcon);
	m_EditTimeAfter.EnableWindow(nDragIcon);
#else
	m_CkDragIcon.SetCheck(1);
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CDlgRobotSet::OnBnClickedCkLaser()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_CkOpenLaser.GetCheck()){
		CONTROL_Extend(g_pMainDlg->m_lLoginID, LASER_OPEN, 0);
	}else{
		CONTROL_Extend(g_pMainDlg->m_lLoginID, LASER_CLOSE, 0);
	}
}


void CDlgRobotSet::OnBnClickedBtnSetmark()
{
	// TODO: 在此添加控件通知处理程序代码
	if(g_pMainDlg->m_lLoginID > 0){
		if(CONTROL_Extend(g_pMainDlg->m_lLoginID, SET_MARKPOINT, 0)){
			Sleep(1000);				// why there should be Sleep

			LONG lPosX;
			LONG lPosY;
			INT nStatus;
			BOOL bGetPos = CONTROL_QueryPos(g_pMainDlg->m_lLoginID, &lPosX, &lPosY, &nStatus);
			if(bGetPos){
				g_pMainDlg->m_lMarkPointX = lPosX;
				g_pMainDlg->m_lMarkPointY = lPosY;
				sprintf(g_dbgBuf, "%ld", lPosX);
				WritePrivateProfileString("COMMON", "MARKPOINTX", g_dbgBuf, theApp.m_szConfigFile);
				sprintf(g_dbgBuf, "%ld", lPosY);
				WritePrivateProfileString("COMMON", "MARKPOINTY", g_dbgBuf, theApp.m_szConfigFile);
				sprintf(g_dbgBuf, "设置标记点为(%ld, %ld)", lPosX, lPosY);
				MessageBox(g_dbgBuf);
			}
		}else{
			MessageBox("设置标记点失败！");
		}
	}else{
		MessageBox("请确定是否已连接机器人！");
	}
}


void CDlgRobotSet::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
#if USE_MODALSETDLG
	CDialogEx::OnCancel();
#else
	DestroyWindow();
	::SendMessage(g_pMainDlg->m_hWnd, ROBOTDLG_EXIT, 0, 0);
#endif
}

