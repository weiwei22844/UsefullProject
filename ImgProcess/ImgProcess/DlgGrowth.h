#pragma once
#include "afxwin.h"
#include "CvvImage.h"

// CDlgGrowth dialog

class CDlgGrowth : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgGrowth)

public:
	CDlgGrowth(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgGrowth();

    CvvImage m_cimg;
    CvvImage m_matImg;
    CString m_fileName;
    int m_nThreshold;

// Dialog Data
	enum { IDD = IDD_DLG_GROWTH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBtnSelect();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedBtnSave();
    CScrollBar m_ScrollBarThreshold;
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnBnClickedCancel();
    virtual BOOL DestroyWindow();
};
