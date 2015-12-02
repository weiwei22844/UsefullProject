#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CDlgBinaryParam dialog

class CDlgBinaryParam : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgBinaryParam)

public:
	CDlgBinaryParam(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgBinaryParam();

// Dialog Data
	enum { IDD = IDD_DLG_PARAMSET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CSliderCtrl m_sliderBinValue;
    CSliderCtrl m_sliderPenWidth;

    void InitDlg(int nThresholdMin, int nThresholdMax, int nThresholdCur, int nPenWdMin, int nPenWdMax, int nPenWdCur);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    CStatic m_thresholdValue;
    CStatic m_staticPenWd;
    afx_msg void OnBnClickedBtnSave();
    afx_msg void OnBnClickedRadioBlack();
    afx_msg void OnBnClickedRadioWhite();
    afx_msg void OnClose();
    afx_msg void OnBnClickedBtnVectorize();
};
