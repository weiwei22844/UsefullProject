// DlgInitSetting.cpp : implementation file
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgInitSetting.h"
#include "afxdialogex.h"
#include "CalibrationDlg.h"

// CDlgInitSetting dialog

IMPLEMENT_DYNAMIC(CDlgInitSetting, CDialogEx)

CDlgInitSetting::CDlgInitSetting(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgInitSetting::IDD, pParent)
{
    m_pModuleData = NULL;
    m_ScriptFile = "";
}

CDlgInitSetting::~CDlgInitSetting()
{
}

void CDlgInitSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgInitSetting, CDialogEx)
    ON_BN_CLICKED(IDOK, &CDlgInitSetting::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CDlgInitSetting::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BTN_INITSCRIPT, &CDlgInitSetting::OnBnClickedBtnInitscript)
END_MESSAGE_MAP()


// CDlgInitSetting message handlers


BOOL CDlgInitSetting::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here
    ModuleInitSet *pInitSet = (ModuleInitSet*)m_pModuleData->pData;
    SetDlgItemText(IDC_EDIT_NAME, m_pModuleData->cName);
    SetDlgItemText(IDC_STATIC_SCRIPTPATH, pInitSet->cScriptFile);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgInitSetting::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    GetDlgItemText(IDC_EDIT_NAME, m_pModuleData->cName, MODULENAMEMAXLEN-1);
    ModuleInitSet *pInitSet = (ModuleInitSet *)m_pModuleData->pData;
    strcpy(pInitSet->cScriptFile, m_ScriptFile.GetBuffer(0));
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, INITSETTING_EXIT, 0, 0);
    }else{
        CDialogEx::OnOK();
    }
}


void CDlgInitSetting::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, INITSETTING_EXIT, 0, 0);
    }else{
        CDialogEx::OnCancel();
    }
}


void CDlgInitSetting::OnBnClickedBtnInitscript()
{
    // TODO: Add your control notification handler code here
    BOOL bGenSuccess = FALSE;
    TCHAR szFilters[]= _T("All Files (*.*)|*.*||");
    CFileDialog dlgFile(TRUE, _T("All files"), _T("*.*"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilters);
    if(dlgFile.DoModal()==IDOK){
        m_ScriptFile = dlgFile.GetPathName();
        SetDlgItemText(IDC_STATIC_SCRIPTPATH, m_ScriptFile);
    }
}
