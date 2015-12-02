#pragma once
#include "afxwin.h"

class CRedirect;

// CDlgVectorize dialog

class CDlgVectorize : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgVectorize)

public:
	CDlgVectorize(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgVectorize();

    CString m_runParam;
    CString m_file;
    int m_nFlag;

// Dialog Data
	enum { IDD = IDD_DLG_VECTORIZE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    CRedirect	*m_pRedirect;


    void		UpdateControls();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnRun();
    afx_msg void OnBnClickedBtnStop();
    afx_msg void OnBnClickedBtnExit();
    CEdit m_EditRun;
    CEdit m_EditOutput;
    CButton m_BtnRun;
    CButton m_BtnStop;
    CButton m_BtnExit;
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBtnShow();
    CButton m_BtnShow;
    afx_msg void OnBnClickedBtnSelectoutline();
    CStatic m_StaticFile;
};
