#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CDlgCannySobel dialog

class CDlgCannySobel : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCannySobel)

public:
	CDlgCannySobel(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgCannySobel();

    void InitDlg(int nCannyMin, int nCannyMax, int nCannyCur, int nSobelMin, int nSobelMax, int nSobelCur);

// Dialog Data
	enum { IDD = IDD_DLG_PARAMCANNY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnSave();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    CSliderCtrl m_sliderCanny;
    CSliderCtrl m_sliderSobel;
    CStatic m_staticCanny;
    CStatic m_staticSobel;
    afx_msg void OnClose();
    afx_msg void OnBnClickedBtnProccanny();
    afx_msg void OnBnClickedBtnProcsobel();
};
