// DlgNewCase.cpp : 实现文件
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgNewCase.h"
#include "afxdialogex.h"


// CDlgNewCase 对话框

IMPLEMENT_DYNAMIC(CDlgNewCase, CDialogEx)

CDlgNewCase::CDlgNewCase(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgNewCase::IDD, pParent)
{

}

CDlgNewCase::~CDlgNewCase()
{
}

void CDlgNewCase::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgNewCase, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgNewCase::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgNewCase 消息处理程序


BOOL CDlgNewCase::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CString str;
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	str.Format("UserCase%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	SetDlgItemText(IDC_EDIT_CASENAME, str);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CDlgNewCase::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItemText(IDC_EDIT_CASENAME, m_UserCaseName);
	if(m_UserCaseName.IsEmpty()){
		MessageBox("User case name should not be empty!", MB_OK);
		return;
	}else{
		// 判断该用例名是否存在 待完善
	}

	CDialogEx::OnOK();
}
