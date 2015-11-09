// DlgCircleTimes.cpp : 实现文件
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgCircleTimes.h"
#include "afxdialogex.h"
#include "CalibrationDlg.h"

// CDlgCircleTimes 对话框

IMPLEMENT_DYNAMIC(CDlgCircleTimes, CDialogEx)

CDlgCircleTimes::CDlgCircleTimes(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgCircleTimes::IDD, pParent)
{

}

CDlgCircleTimes::~CDlgCircleTimes()
{
}

void CDlgCircleTimes::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgCircleTimes, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgCircleTimes::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgCircleTimes::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgCircleTimes 消息处理程序


BOOL CDlgCircleTimes::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetDlgItemInt(IDC_EDIT_CIRCLETIMES, m_nTimes);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CDlgCircleTimes::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if(g_pMainDlg->IsWindowEnabled()){
		m_nTimes = GetDlgItemInt(IDC_EDIT_CIRCLETIMES);
		DestroyWindow();
		::SendMessage(g_pMainDlg->m_hWnd, CIRCLETIMES_EXIT, 0, 0);
	}else{
		CDialogEx::OnOK();
	}
}


void CDlgCircleTimes::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	if(g_pMainDlg->IsWindowEnabled()){
		DestroyWindow();
		::SendMessage(g_pMainDlg->m_hWnd, CIRCLETIMES_EXIT, 0, 0);
	}else{
		CDialogEx::OnCancel();
	}
}
