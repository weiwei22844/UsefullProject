// DlgIconDrag.cpp : implementation file
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgIconDrag.h"
#include "afxdialogex.h"
#include "CalibrationDlg.h"

// CDlgIconDrag dialog

IMPLEMENT_DYNAMIC(CDlgIconDrag, CDialogEx)

CDlgIconDrag::CDlgIconDrag(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgIconDrag::IDD, pParent)
{
    m_pModuleData = NULL;
}

CDlgIconDrag::~CDlgIconDrag()
{
}

void CDlgIconDrag::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_SPECIFYPOS, m_CkSpecifyPos);
	DDX_Control(pDX, IDC_EDIT_STARTX, m_EditStartX);
	DDX_Control(pDX, IDC_EDIT_STARTY, m_EditStartY);
	DDX_Control(pDX, IDC_EDIT_ENDX, m_EditEndX);
	DDX_Control(pDX, IDC_EDIT_ENDY, m_EditEndY);
	DDX_Control(pDX, IDC_EDIT_TIMEBEFORE, m_EditTimeBefore);
	DDX_Control(pDX, IDC_EDIT_TIMEAFTER, m_EditTimeAfter);
}


BEGIN_MESSAGE_MAP(CDlgIconDrag, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgIconDrag::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CDlgIconDrag::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK_SPECIFYPOS, &CDlgIconDrag::OnBnClickedCheckSpecifypos)
END_MESSAGE_MAP()


// CDlgIconDrag message handlers


BOOL CDlgIconDrag::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here
	ModuleIconDrag *pIconDrag = (ModuleIconDrag *)m_pModuleData->pData;
	m_CkSpecifyPos.SetCheck(pIconDrag->nMode);
	m_EditStartX.EnableWindow(pIconDrag->nMode);
	m_EditStartY.EnableWindow(pIconDrag->nMode);
	if(m_CkSpecifyPos.GetCheck()){
		SetDlgItemText(IDC_STATIC_INFO, "指定拖动起始位置，需设置目标位置及目的位置XY值，请在视频窗口左键按下拖动示范拖动轨迹");
	}else{
		SetDlgItemText(IDC_STATIC_INFO, "默认情况下基于识别匹配进行图标拖动，无需设置图标初始位置XY值，只需设置目的位置XY值，鼠标右击单击可自动更新");
	}
	SetDlgItemInt(IDC_EDIT_STARTX, pIconDrag->nStartX);
	SetDlgItemInt(IDC_EDIT_STARTY, pIconDrag->nStartY);
	SetDlgItemInt(IDC_EDIT_ENDX, pIconDrag->nEndX);
	SetDlgItemInt(IDC_EDIT_ENDY, pIconDrag->nEndY);
	SetDlgItemInt(IDC_EDIT_TIMEBEFORE, pIconDrag->nTimeBefore);
	SetDlgItemInt(IDC_EDIT_TIMEAFTER, pIconDrag->nTimeAfter);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgIconDrag::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
    //GetDlgItemText(IDC_EDIT_NAME, m_pModuleData->cName, MODULENAMEMAXLEN-1);
    ModuleIconDrag *pIconDrag = (ModuleIconDrag *)m_pModuleData->pData;
	if(m_CkSpecifyPos.GetCheck()){
		pIconDrag->nMode = 1;
	}else{
		pIconDrag->nMode = 0;
	}
	pIconDrag->nStartX = GetDlgItemInt(IDC_EDIT_STARTX);
	pIconDrag->nStartY = GetDlgItemInt(IDC_EDIT_STARTY);
	pIconDrag->nEndX = GetDlgItemInt(IDC_EDIT_ENDX);
	pIconDrag->nEndY = GetDlgItemInt(IDC_EDIT_ENDY);
	pIconDrag->nTimeBefore = GetDlgItemInt(IDC_EDIT_TIMEBEFORE);
	pIconDrag->nTimeAfter = GetDlgItemInt(IDC_EDIT_TIMEAFTER);
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, ICONDRAG_EXIT, 0, 0);
    }else{
        CDialogEx::OnOK();
    }
}


void CDlgIconDrag::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, ICONDRAG_EXIT, 0, 0);
    }else{
        CDialogEx::OnCancel();
    }
}


void CDlgIconDrag::OnBnClickedCheckSpecifypos()
{
	// TODO: 在此添加控件通知处理程序代码
	m_EditStartX.EnableWindow(m_CkSpecifyPos.GetCheck());
	m_EditStartY.EnableWindow(m_CkSpecifyPos.GetCheck());
	if(m_CkSpecifyPos.GetCheck()){
		SetDlgItemText(IDC_STATIC_INFO, "指定拖动起始位置，需设置目标位置及目的位置XY值，请在视频窗口左键按下拖动示范拖动轨迹");
	}else{
		SetDlgItemText(IDC_STATIC_INFO, "默认情况下基于识别匹配进行图标拖动，无需设置图标初始位置XY值，只需设置目的位置XY值，鼠标右击单击可自动更新");
	}
}

void CDlgIconDrag::SetStartEndPos(LONG lStartX, LONG lStartY, LONG lEndX, LONG lEndY)
{
	CString str;
	str.Format("%ld", lStartX);
	m_EditStartX.SetWindowText(str);
	str.Format("%ld", lStartY);
	m_EditStartY.SetWindowTextA(str);
	str.Format("%ld", lEndX);
	m_EditEndX.SetWindowText(str);
	str.Format("%ld", lEndY);
	m_EditEndY.SetWindowTextA(str);
}

void CDlgIconDrag::SetTargetPos(LONG x, LONG y)
{
	CString str;
	str.Format("%ld", x);
	m_EditEndX.SetWindowText(str);
	str.Format("%ld", y);
	m_EditEndY.SetWindowTextA(str);
}