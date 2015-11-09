// DlgTimeDelay.cpp : 实现文件
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgTimeDelay.h"
#include "afxdialogex.h"
#include "CalibrationDlg.h"

// CDlgTimeDelay 对话框

IMPLEMENT_DYNAMIC(CDlgTimeDelay, CDialogEx)

CDlgTimeDelay::CDlgTimeDelay(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgTimeDelay::IDD, pParent)
{

}

CDlgTimeDelay::~CDlgTimeDelay()
{
}

void CDlgTimeDelay::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgTimeDelay, CDialogEx)
    ON_BN_CLICKED(IDOK, &CDlgTimeDelay::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CDlgTimeDelay::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgTimeDelay 消息处理程序


BOOL CDlgTimeDelay::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    ModuleDelay *pTimeDelay = (ModuleDelay *)m_pModuleData->pData;
    SetDlgItemInt(IDC_EDIT_TIMEDELAY, pTimeDelay->nDelayTime);

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常: OCX 属性页应返回 FALSE
}


void CDlgTimeDelay::OnBnClickedOk()
{
    // TODO: 在此添加控件通知处理程序代码
    ModuleDelay *pTimeDelay = (ModuleDelay *)m_pModuleData->pData;
    pTimeDelay->nDelayTime = GetDlgItemInt(IDC_EDIT_TIMEDELAY);
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, TIMEDELAY_EXIT, 0, 0);
    }else{
        CDialogEx::OnOK();
    }
}


void CDlgTimeDelay::OnBnClickedCancel()
{
    // TODO: 在此添加控件通知处理程序代码
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, TIMEDELAY_EXIT, 0, 0);
    }else{
        CDialogEx::OnCancel();
    }
}
