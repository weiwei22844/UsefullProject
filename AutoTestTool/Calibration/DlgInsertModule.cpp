// DlgInsertModule.cpp : 实现文件
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgInsertModule.h"
#include "afxdialogex.h"


// CDlgInsertModule 对话框

IMPLEMENT_DYNAMIC(CDlgInsertModule, CDialogEx)

CDlgInsertModule::CDlgInsertModule(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgInsertModule::IDD, pParent)
{

}

CDlgInsertModule::~CDlgInsertModule()
{
}

void CDlgInsertModule::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgInsertModule, CDialogEx)
    ON_BN_CLICKED(IDOK, &CDlgInsertModule::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgInsertModule 消息处理程序


BOOL CDlgInsertModule::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    CheckRadioButton(IDC_RADIO1, IDC_RADIO7, IDC_RADIO1);
    m_nCheckRadio = 0;

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常: OCX 属性页应返回 FALSE
}


void CDlgInsertModule::OnBnClickedOk()
{
    // TODO: 在此添加控件通知处理程序代码
    m_nCheckRadio = GetCheckedRadioButton(IDC_RADIO1, IDC_RADIO7);
    m_nCheckRadio -= IDC_RADIO1;
    CDialogEx::OnOK();
}
