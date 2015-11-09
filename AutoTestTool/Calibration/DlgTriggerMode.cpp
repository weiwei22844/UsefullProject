// DlgTriggerMode.cpp : implementation file
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgTriggerMode.h"
#include "afxdialogex.h"
#include "CalibrationDlg.h"

// CDlgTriggerMode dialog

IMPLEMENT_DYNAMIC(CDlgTriggerMode, CDialogEx)

CDlgTriggerMode::CDlgTriggerMode(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgTriggerMode::IDD, pParent)
{
    m_pModuleData = NULL;
}

CDlgTriggerMode::~CDlgTriggerMode()
{
}

void CDlgTriggerMode::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgTriggerMode, CDialogEx)
END_MESSAGE_MAP()


// CDlgTriggerMode message handlers


BOOL CDlgTriggerMode::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}
