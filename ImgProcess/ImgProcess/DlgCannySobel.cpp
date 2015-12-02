// DlgCannySobel.cpp : implementation file
//

#include "stdafx.h"
#include "ImgProcess.h"
#include "DlgCannySobel.h"
#include "afxdialogex.h"
#include "ImgProcessDlg.h"

// CDlgCannySobel dialog

IMPLEMENT_DYNAMIC(CDlgCannySobel, CDialogEx)

CDlgCannySobel::CDlgCannySobel(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgCannySobel::IDD, pParent)
{

}

CDlgCannySobel::~CDlgCannySobel()
{
    OutputDebugString("~CDlgCannySobel");
}

void CDlgCannySobel::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SLIDER_BINVALUE, m_sliderCanny);
    DDX_Control(pDX, IDC_SLIDER_PENWIDTH, m_sliderSobel);
    DDX_Control(pDX, IDC_STATIC_CANNY, m_staticCanny);
    DDX_Control(pDX, IDC_STATIC_SOBEL, m_staticSobel);
}


BEGIN_MESSAGE_MAP(CDlgCannySobel, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_SAVE, &CDlgCannySobel::OnBnClickedBtnSave)
    ON_WM_HSCROLL()
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BTN_PROCCANNY, &CDlgCannySobel::OnBnClickedBtnProccanny)
    ON_BN_CLICKED(IDC_BTN_PROCSOBEL, &CDlgCannySobel::OnBnClickedBtnProcsobel)
END_MESSAGE_MAP()


// CDlgCannySobel message handlers

void CDlgCannySobel::InitDlg(int nCannyMin, int nCannyMax, int nCannyCur, int nSobelMin, int nSobelMax, int nSobelCur)
{
    CString str;
    m_sliderCanny.SetRange(nCannyMin, nCannyMax);
    m_sliderCanny.SetPos(nCannyCur);
    str.Format("%d", nCannyCur);
    m_staticCanny.SetWindowText(str);
    m_sliderSobel.SetRange(nSobelMin, nSobelMax);
    m_sliderSobel.SetPos(nSobelCur);
    str.Format("%d", nSobelCur);
    m_staticSobel.SetWindowText(str);

    HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    SetIcon(hIcon, TRUE);
    SetIcon(hIcon, FALSE);
}

void CDlgCannySobel::OnBnClickedBtnSave()
{
    // TODO: Add your control notification handler code here
    pMainDlg->OnSaveCannySobelResult(m_sliderCanny.GetPos(), m_sliderSobel.GetPos());
}


void CDlgCannySobel::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: Add your message handler code here and/or call default
    CString str;
    CSliderCtrl *pSlidCtrl = (CSliderCtrl*)pScrollBar;
    str.Format("%d", pSlidCtrl->GetPos());
    if(*pScrollBar == m_sliderCanny){
        m_staticCanny.SetWindowText(str);
        pMainDlg->OnChangeCanny(pSlidCtrl->GetPos());
    }else if(*pScrollBar == m_sliderSobel){
        m_staticSobel.SetWindowText(str);
        pMainDlg->OnChangeSobel(pSlidCtrl->GetPos());
    }
    
    CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CDlgCannySobel::OnClose()
{
    // TODO: Add your message handler code here and/or call default
    if(::MessageBox(m_hWnd, "关闭参数设置将会退出当前处理，是否确认关闭", "确认退出", MB_YESNO) == IDNO)
        return;
    pMainDlg->ExitCurProc();

    CDialogEx::OnClose();
}


void CDlgCannySobel::OnBnClickedBtnProccanny()
{
    // TODO: Add your control notification handler code here
    pMainDlg->ExitCurProc(1, m_sliderCanny.GetPos());
    CDialogEx::OnOK();
}


void CDlgCannySobel::OnBnClickedBtnProcsobel()
{
    // TODO: Add your control notification handler code here
    pMainDlg->ExitCurProc(2, m_sliderSobel.GetPos());
    CDialogEx::OnOK();
}
