// DlgFingerMove.cpp : implementation file
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgFingerMove.h"
#include "afxdialogex.h"
#include "CalibrationDlg.h"

// CDlgFingerMove dialog

IMPLEMENT_DYNAMIC(CDlgFingerMove, CDialogEx)

CDlgFingerMove::CDlgFingerMove(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgFingerMove::IDD, pParent)
{
    m_pModuleData = NULL;
}

CDlgFingerMove::~CDlgFingerMove()
{
}

void CDlgFingerMove::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_QUICKFINGER, m_CkQuickFinger);
	DDX_Control(pDX, IDC_CHECK_FIXEDPOS, m_CkFixedPos);
	DDX_Control(pDX, IDC_EDIT_MOVETOX, m_EditMoveToX);
	DDX_Control(pDX, IDC_EDIT_MOVETOY, m_EditMoveToY);
	DDX_Control(pDX, IDC_EDIT_QUICKFREQ, m_EditQuickFreq);
	DDX_Control(pDX, IDC_EDIT_QUICKTIMES, m_EditQuickTimes);
	DDX_Control(pDX, IDC_CHECK_ISCLICK, m_CkClick);
}


BEGIN_MESSAGE_MAP(CDlgFingerMove, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &CDlgFingerMove::OnBnClickedCancel)
    ON_BN_CLICKED(IDOK, &CDlgFingerMove::OnBnClickedOk)
    ON_BN_CLICKED(IDC_CHECK_FIXEDPOS, &CDlgFingerMove::OnBnClickedCheckFixedpos)
	ON_BN_CLICKED(IDC_CHECK_QUICKFINGER, &CDlgFingerMove::OnBnClickedCheckQuickfinger)
	ON_BN_CLICKED(IDC_CHECK_ISCLICK, &CDlgFingerMove::OnBnClickedCheckIsclick)
END_MESSAGE_MAP()


// CDlgFingerMove message handlers


BOOL CDlgFingerMove::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here
    ModuleFingerMove *pFingerMove = (ModuleFingerMove *)m_pModuleData->pData;
    if(pFingerMove->nFingerType){
        m_CkQuickFinger.SetCheck(1);
    }
	m_EditQuickFreq.EnableWindow(m_CkQuickFinger.GetCheck());
	m_EditQuickTimes.EnableWindow(m_CkQuickFinger.GetCheck());
    if(pFingerMove->nMode){
        m_CkFixedPos.SetCheck(1);
        SetDlgItemText(IDC_STATIC_INFO, "右击视频窗口会自动更新目标位置XY值");
    }else{
		SetDlgItemText(IDC_STATIC_INFO, "默认情况下基于识别匹配进行手指移动，无需设置目标位置XY值");
    }
    m_EditMoveToX.EnableWindow(m_CkFixedPos.GetCheck());
    m_EditMoveToY.EnableWindow(m_CkFixedPos.GetCheck());
    SetDlgItemInt(IDC_EDIT_MOVETOX, pFingerMove->nMoveToX);
    SetDlgItemInt(IDC_EDIT_MOVETOY, pFingerMove->nMoveToY);
	SetDlgItemInt(IDC_EDIT_QUICKFREQ, pFingerMove->nFrequency);
	SetDlgItemInt(IDC_EDIT_QUICKTIMES, pFingerMove->nTimes);
	m_CkClick.SetCheck(pFingerMove->nClick);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgFingerMove::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, FINGERMOVE_EXIT, 0, 0);
    }else{
        CDialogEx::OnCancel();
    }
}


void CDlgFingerMove::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
	UpdateData();
    ModuleFingerMove *pFingerMove = (ModuleFingerMove *)m_pModuleData->pData;
    if(m_CkQuickFinger.GetCheck()){
        pFingerMove->nFingerType = 1;
		pFingerMove->nFrequency = GetDlgItemInt(IDC_EDIT_QUICKFREQ);
		pFingerMove->nTimes = GetDlgItemInt(IDC_EDIT_QUICKTIMES);
	}else{
        pFingerMove->nFingerType = 0;
		pFingerMove->nFrequency = 1;
		pFingerMove->nTimes = 1;
	}
    if(m_CkFixedPos.GetCheck()){
        pFingerMove->nMode = 1;
		pFingerMove->nMoveToX = GetDlgItemInt(IDC_EDIT_MOVETOX);
		pFingerMove->nMoveToY = GetDlgItemInt(IDC_EDIT_MOVETOY);
    }else{
        pFingerMove->nMode = 0;
		pFingerMove->nMoveToX = 0;
		pFingerMove->nMoveToY = 0;
    }
	if(m_CkClick.GetCheck()){
		pFingerMove->nClick = 1;
	}else{
		pFingerMove->nClick = 0;
	}
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, FINGERMOVE_EXIT, 0, 0);
    }else{
        CDialogEx::OnOK();
    }
}


void CDlgFingerMove::OnBnClickedCheckFixedpos()
{
    // TODO: Add your control notification handler code here
    m_EditMoveToX.EnableWindow(m_CkFixedPos.GetCheck());
    m_EditMoveToY.EnableWindow(m_CkFixedPos.GetCheck());
    if(m_CkFixedPos.GetCheck()){
        SetDlgItemText(IDC_STATIC_INFO, "右击视频窗口会自动更新目标位置XY值");
    }else{
		SetDlgItemText(IDC_STATIC_INFO, "默认情况下基于识别匹配进行手指移动，无需设置目标位置XY值");
    }
}


void CDlgFingerMove::OnBnClickedCheckQuickfinger()
{
	// TODO: 在此添加控件通知处理程序代码
	m_EditQuickFreq.EnableWindow(m_CkQuickFinger.GetCheck()&&m_CkClick.GetCheck());
	m_EditQuickTimes.EnableWindow(m_CkQuickFinger.GetCheck()&&m_CkClick.GetCheck());
}


void CDlgFingerMove::OnBnClickedCheckIsclick()
{
	// TODO: 在此添加控件通知处理程序代码
	m_EditQuickFreq.EnableWindow(m_CkQuickFinger.GetCheck()&&m_CkClick.GetCheck());
	m_EditQuickTimes.EnableWindow(m_CkQuickFinger.GetCheck()&&m_CkClick.GetCheck());
}

void CDlgFingerMove::SetTargetPos(LONG x, LONG y)
{
	CString str;
	str.Format("%ld", x);
	m_EditMoveToX.SetWindowText(str);
	str.Format("%ld", y);
	m_EditMoveToY.SetWindowText(str);
}
