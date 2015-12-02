// DlgBinaryParam.cpp : implementation file
//

#include "stdafx.h"
#include "ImgProcess.h"
#include "DlgBinaryParam.h"
#include "afxdialogex.h"
#include "ImgProcessDlg.h"

// CDlgBinaryParam dialog

IMPLEMENT_DYNAMIC(CDlgBinaryParam, CDialogEx)

CDlgBinaryParam::CDlgBinaryParam(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgBinaryParam::IDD, pParent)
{

}

CDlgBinaryParam::~CDlgBinaryParam()
{
    OutputDebugString("CDlgBinaryParam exit");
}

void CDlgBinaryParam::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SLIDER_BINVALUE, m_sliderBinValue);
    DDX_Control(pDX, IDC_SLIDER_PENWIDTH, m_sliderPenWidth);
    DDX_Control(pDX, IDC_STATIC_THRESHOLD, m_thresholdValue);
    DDX_Control(pDX, IDC_STATIC_PENWIDTH, m_staticPenWd);
}


BEGIN_MESSAGE_MAP(CDlgBinaryParam, CDialogEx)
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDC_BTN_SAVE, &CDlgBinaryParam::OnBnClickedBtnSave)
    ON_BN_CLICKED(IDC_RADIO_BLACK, &CDlgBinaryParam::OnBnClickedRadioBlack)
    ON_BN_CLICKED(IDC_RADIO_WHITE, &CDlgBinaryParam::OnBnClickedRadioWhite)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BTN_VECTORIZE, &CDlgBinaryParam::OnBnClickedBtnVectorize)
END_MESSAGE_MAP()


// CDlgBinaryParam message handlers


void CDlgBinaryParam::InitDlg(int nThresholdMin, int nThresholdMax, int nThresholdCur, int nPenWdMin, int nPenWdMax, int nPenWdCur)
{
    CString str;
    m_sliderBinValue.SetRange(nThresholdMin, nThresholdMax);
    m_sliderBinValue.SetPos(nThresholdCur);
    str.Format("%d", nThresholdCur);
    m_thresholdValue.SetWindowText(str);
    m_sliderPenWidth.SetRange(nPenWdMin, nPenWdMax);
    m_sliderPenWidth.SetPos(nPenWdCur);
    str.Format("%d", nPenWdCur);
    m_staticPenWd.SetWindowText(str);
    ((CButton *)GetDlgItem(IDC_RADIO_BLACK))->SetCheck(TRUE);

    HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    SetIcon(hIcon, TRUE);
    SetIcon(hIcon, FALSE);
}

void CDlgBinaryParam::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CString str;
    CSliderCtrl *pSlidCtrl = (CSliderCtrl*)pScrollBar;
    str.Format("%d", pSlidCtrl->GetPos());
    if(*pScrollBar == m_sliderBinValue){
        m_thresholdValue.SetWindowText(str);
        pMainDlg->OnChangeBinThresHold(pSlidCtrl->GetPos());
    }else if(*pScrollBar == m_sliderPenWidth){
        m_staticPenWd.SetWindowText(str);
        pMainDlg->OnChangeBinPenWd(pSlidCtrl->GetPos());
    }
    CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CDlgBinaryParam::OnBnClickedBtnSave()
{
    // TODO: Add your control notification handler code here
    pMainDlg->OnSaveBinResult();
}


void CDlgBinaryParam::OnBnClickedRadioBlack()
{
    // TODO: Add your control notification handler code here
    pMainDlg->OnChangeBinPenColor(0);
}


void CDlgBinaryParam::OnBnClickedRadioWhite()
{
    // TODO: Add your control notification handler code here
    pMainDlg->OnChangeBinPenColor(1);
}


void CDlgBinaryParam::OnClose()
{
    // TODO: Add your message handler code here and/or call default
    if(::MessageBox(m_hWnd, "关闭参数设置将会退出当前处理，是否确认关闭", "确认退出", MB_YESNO) == IDNO)
        return;
    pMainDlg->ExitCurProc();
    CDialogEx::OnClose();
}


void CDlgBinaryParam::OnBnClickedBtnVectorize()
{
    // TODO: Add your control notification handler code here
    //pMainDlg->OnVectorizeCurBin();
    ::PostMessage(pMainDlg->m_hWnd, WM_VECTORCURBIN, 0, 0);
    CDialogEx::OnOK();
}
