// DlgPMSet.cpp : 实现文件
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgPMSet.h"
#include "afxdialogex.h"

#include "CalibrationDlg.h"
#include "libControl.h"

#include "DlgTemplate.h"

// CDlgPMSet 对话框

IMPLEMENT_DYNAMIC(CDlgPMSet, CDialogEx)

CDlgPMSet::CDlgPMSet(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgPMSet::IDD, pParent)
{
}

CDlgPMSet::~CDlgPMSet()
{
}

void CDlgPMSet::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHECK_ENABLEROI, m_CkEnableROI);
    DDX_Control(pDX, IDC_DEV_IPADDR, m_DevIpAddr);
    DDX_Control(pDX, IDC_CK_LASER, m_CkOpenLaser);
    DDX_Control(pDX, IDC_EDIT_CLICESPEED, m_EditClkSpeed);
}


BEGIN_MESSAGE_MAP(CDlgPMSet, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SETPREPOS, &CDlgPMSet::OnBnClickedBtnSetprepos)
	ON_BN_CLICKED(IDC_BTN_CAMASET, &CDlgPMSet::OnBnClickedBtnCamaset)
    ON_BN_CLICKED(IDC_BTN_SETROI, &CDlgPMSet::OnBnClickedBtnSetroi)
    ON_BN_CLICKED(IDC_CHECK_ENABLEROI, &CDlgPMSet::OnBnClickedCheckEnableroi)
    ON_BN_CLICKED(IDOK, &CDlgPMSet::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CK_LASER, &CDlgPMSet::OnBnClickedCkLaser)
	ON_BN_CLICKED(IDCANCEL, &CDlgPMSet::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BTN_SETPATH, &CDlgPMSet::OnBnClickedBtnSetpath)
END_MESSAGE_MAP()


// CDlgPMSet 消息处理程序


BOOL CDlgPMSet::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	TCHAR chTemp[MAX_PATH];
	GetPrivateProfileString("ROBOT", "IPADDR", "192.168.1.144", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_DevIpAddr.SetAddress(ntohl(inet_addr(chTemp)));

	LONG lPrePosX = GetPrivateProfileInt("COMMON", "PREPOSX", -1, theApp.m_szConfigFile);
	LONG lPrePosY = GetPrivateProfileInt("COMMON", "PREPOSY", -1, theApp.m_szConfigFile);
    LONG lPrePosZ = GetPrivateProfileInt("COMMON", "PREPOSZ", -1, theApp.m_szConfigFile);
	SetDlgItemInt(IDC_EDIT_PREPOSX, lPrePosX);
	SetDlgItemInt(IDC_EDIT_PREPOSY, lPrePosY);
    SetDlgItemInt(IDC_EDIT_PREPOSZ, lPrePosZ);

    int nValue = GetPrivateProfileInt("POINTGREY", "ENABLEROI", 0, theApp.m_szConfigFile);
    m_CkEnableROI.SetCheck(nValue);
    GetDlgItem(IDC_BTN_SETROI)->EnableWindow(nValue);

    nValue = GetPrivateProfileInt("ROBOT", "CLICKSPEED", 5, theApp.m_szConfigFile);
    SetDlgItemInt(IDC_EDIT_CLICESPEED, nValue);

    strcpy(m_pszPath, theApp.m_szUserCaseFile);
    (strrchr(m_pszPath, '\\'))[1] = 0;         //删除文件名，只获得路径
    SetDlgItemText(IDC_EDIT_CASEPATH, m_pszPath);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CDlgPMSet::OnBnClickedBtnSetprepos()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	LONG lPosX;
	LONG lPosY;
	LONG lPosZ;
	INT nStatus;
	if(g_pMainDlg->m_lLoginID > 0){
		BOOL bGetPos = CONTROL_QueryPos(g_pMainDlg->m_lLoginID, &lPosX, &lPosY, &nStatus, &lPosZ);
		if(bGetPos){
			g_pMainDlg->m_lPrePosX = lPosX;
			str.Format("%d", lPosX);
			WritePrivateProfileString("COMMON", "PREPOSX", str.GetBuffer(0), theApp.m_szConfigFile);
			g_pMainDlg->m_lPrePosY = lPosY;
			str.Format("%d", lPosY);
			WritePrivateProfileString("COMMON", "PREPOSY", str.GetBuffer(0), theApp.m_szConfigFile);
			g_pMainDlg->m_lPrePosZ = lPosZ;
			str.Format("%d", lPosZ);
			WritePrivateProfileString("COMMON", "PREPOSZ", str.GetBuffer(0), theApp.m_szConfigFile);

			SetDlgItemInt(IDC_EDIT_PREPOSX, lPosX);
			SetDlgItemInt(IDC_EDIT_PREPOSY, lPosY);
            SetDlgItemInt(IDC_EDIT_PREPOSZ, lPosZ);
		}else{
			MessageBox("设置预置位失败");
		}
	}else{
		MessageBox("请先连接机器人");
	}
}


void CDlgPMSet::OnBnClickedBtnCamaset()
{
	// TODO: 在此添加控件通知处理程序代码
	if(g_pMainDlg){
		if(g_pMainDlg->m_Inited >= 2){
			if( g_pMainDlg->m_camCtlDlg.IsVisible() == true )
			{
				g_pMainDlg->m_camCtlDlg.Hide();
			}
			else
			{
				g_pMainDlg->m_camCtlDlg.Show();
			}
		}else{
			MessageBox("识别相机未初始化！");
		}
	}
}

void CDlgPMSet::OnBnClickedBtnSetroi()
{
    // TODO: Add your control notification handler code here
    CRect rect;
    g_pMainDlg->m_VideoWnd.GetWindowRect(&rect);
    CString str;
    int nROILeft, nROIWidth, nROITop, nROIHeight;
    CDlgTemplate templateDlg;
    templateDlg.m_Rect = rect;
    templateDlg.DoModal();

    if(!templateDlg.m_bMakeTemplate)
        return;
    
    templateDlg.m_TemplateRectTracker.GetTrueRect(&rect);
    if(rect.left < 0)
        rect.left = 0;
    nROILeft = rect.left/g_pMainDlg->m_dScale;
    nROIWidth = rect.Width()/g_pMainDlg->m_dScale;
    if(nROIWidth > g_pMainDlg->m_nCamWidth)
        nROIWidth = g_pMainDlg->m_nCamWidth;
    if(rect.top < 0)
        rect.top = 0;
    nROITop = rect.top/g_pMainDlg->m_dScale;
    nROIHeight = rect.Height()/g_pMainDlg->m_dScale;
    if(nROIHeight > g_pMainDlg->m_nCamHeight)
        nROIHeight = g_pMainDlg->m_nCamHeight;
    TRACE("left %d width %d top %d height %d", nROILeft, nROIWidth, nROITop, nROIHeight);
    str.Format("%d", nROILeft);
    WritePrivateProfileString("POINTGREY", "ROILEFT", str.GetBuffer(0), theApp.m_szConfigFile);
    str.Format("%d", nROIWidth);
    WritePrivateProfileString("POINTGREY", "ROIWIDTH", str.GetBuffer(0), theApp.m_szConfigFile);
    str.Format("%d", nROITop);
    WritePrivateProfileString("POINTGREY", "ROITOP", str.GetBuffer(0), theApp.m_szConfigFile);
    str.Format("%d", nROIHeight);
    WritePrivateProfileString("POINTGREY", "ROIHEIGHT", str.GetBuffer(0), theApp.m_szConfigFile);
}


void CDlgPMSet::OnBnClickedCheckEnableroi()
{
    // TODO: Add your control notification handler code here
    GetDlgItem(IDC_BTN_SETROI)->EnableWindow(m_CkEnableROI.GetCheck());
}


void CDlgPMSet::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
	CString str;
	m_DevIpAddr.GetWindowText(str);
	WritePrivateProfileString("ROBOT", "IPADDR", str.GetBuffer(0), theApp.m_szConfigFile);

    if(m_CkEnableROI.GetCheck()){
        WritePrivateProfileString("POINTGREY", "ENABLEROI", "1", theApp.m_szConfigFile);
        g_pMainDlg->m_nEnableROI = 1;
    }else{
        WritePrivateProfileString("POINTGREY", "ENABLEROI", "0", theApp.m_szConfigFile);
        g_pMainDlg->m_nEnableROI = 0;
    }

	if(m_CkOpenLaser.GetCheck()){
		WritePrivateProfileString("ROBOT", "LASEROPEN", "1", theApp.m_szConfigFile);
	}else{
		WritePrivateProfileString("ROBOT", "LASEROPEN", "0", theApp.m_szConfigFile);
	}

    m_EditClkSpeed.GetWindowText(str);
    WritePrivateProfileString("ROBOT", "CLICKSPEED", str.GetBuffer(0), theApp.m_szConfigFile);

    strcpy(theApp.m_szUserCaseFile, m_pszPath);
    WritePrivateProfileString("COMMON", "CASEPATH", m_pszPath, theApp.m_szConfigFile);
    if(theApp.m_szUserCaseFile[strlen(theApp.m_szUserCaseFile)-1] == '\\')
        strcat(theApp.m_szUserCaseFile, "usercase.ini");
    else
        strcat(theApp.m_szUserCaseFile, "\\usercase.ini");
    g_pMainDlg->InitUserCase();

	if(g_pMainDlg->IsWindowEnabled()){
		DestroyWindow();
		::SendMessage(g_pMainDlg->m_hWnd, PMSETDLG_EXIT, 0, 0);
	}else{
		CDialogEx::OnOK();
	}
}


void CDlgPMSet::OnBnClickedCkLaser()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_CkOpenLaser.GetCheck()){
		CONTROL_Extend(g_pMainDlg->m_lLoginID, LASER_OPEN, 0);
	}else{
		CONTROL_Extend(g_pMainDlg->m_lLoginID, LASER_CLOSE, 0);
	}
}


void CDlgPMSet::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	if(g_pMainDlg->IsWindowEnabled()){
		DestroyWindow();
		::SendMessage(g_pMainDlg->m_hWnd, PMSETDLG_EXIT, 0, 0);
	}else{
		CDialogEx::OnCancel();
	}
}

//选择文件夹对话框回调函数  
int CALLBACK BrowseCallBackFun(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)  
{
    switch(uMsg)
    {
    case BFFM_INITIALIZED:  //选择文件夹对话框初始化
        //设置默认路径为lpData即'D:\'
        ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
        //在STATUSTEXT区域显示当前路径
        ::SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, lpData);
        //设置选择文件夹对话框的标题  
        ::SetWindowText(hwnd, TEXT("请设置测试用例工作目录"));
        break;
    case BFFM_SELCHANGED:   //选择文件夹变更时
        {
            TCHAR pszPath[MAX_PATH];
            //获取当前选择路径
            SHGetPathFromIDList((LPCITEMIDLIST)lParam, pszPath);
            //在STATUSTEXT区域显示当前路径
            ::SendMessage(hwnd, BFFM_SETSTATUSTEXT, TRUE, (LPARAM)pszPath);
        }
        break;
    }
    return 0;
} 

void CDlgPMSet::OnBnClickedBtnSetpath()
{
    // TODO: 在此添加控件通知处理程序代码
    BROWSEINFO bi; 
    bi.hwndOwner      = this->GetSafeHwnd();
    bi.pidlRoot       = NULL;
    bi.pszDisplayName = NULL; 
    bi.lpszTitle      = TEXT("请选择文件夹"); 
    bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;//BIF_USENEWUI;//
    bi.lpfn           = BrowseCallBackFun; 
    bi.lParam         = 0;          // (LPARAM)TEXT("D:\\");  //传给回调函数的参数,设置默认路径
    bi.iImage         = 0; 

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl == NULL)
    {
        return;
    }

    if (SHGetPathFromIDList(pidl, m_pszPath))
    {
        //AfxMessageBox(pszPath);
        SetDlgItemText(IDC_EDIT_CASEPATH, m_pszPath);
    }
}
