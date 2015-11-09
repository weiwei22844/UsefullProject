// DlgRotate.cpp : implementation file
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgRotate.h"
#include "afxdialogex.h"
#include "CalibrationDlg.h"


// CDlgRotate dialog

IMPLEMENT_DYNAMIC(CDlgRotate, CDialogEx)

CDlgRotate::CDlgRotate(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgRotate::IDD, pParent)
{

}

CDlgRotate::~CDlgRotate()
{
}

void CDlgRotate::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_ANTIWISE, m_CkAntiwise);
}


BEGIN_MESSAGE_MAP(CDlgRotate, CDialogEx)
    ON_BN_CLICKED(IDOK, &CDlgRotate::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CDlgRotate::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgRotate message handlers


BOOL CDlgRotate::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here
	ModuleRotate *pRotate = (ModuleRotate *)m_pModuleData->pData;
	m_CkAntiwise.SetCheck(pRotate->nAntiWise);
	SetDlgItemInt(IDC_EDIT_ROTATEANGLE, pRotate->nRotateAngle/10);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgRotate::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
	ModuleRotate *pRotate = (ModuleRotate *)m_pModuleData->pData;
	if(m_CkAntiwise.GetCheck()){
		pRotate->nAntiWise = 1;
	}else{
		pRotate->nAntiWise = 0;
	}
	pRotate->nRotateAngle = GetDlgItemInt(IDC_EDIT_ROTATEANGLE)*10;
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, ROTATE_EXIT, 0, 0);
    }else{
        CDialogEx::OnOK();
    }
}


void CDlgRotate::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, ROTATE_EXIT, 0, 0);
    }else{
        CDialogEx::OnCancel();
    }
}
