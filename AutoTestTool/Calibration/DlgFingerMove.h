#pragma once
#include "afxwin.h"

// CDlgFingerMove dialog

class CDlgFingerMove : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgFingerMove)

public:
	CDlgFingerMove(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgFingerMove();

    ModuleData *m_pModuleData;
	void SetTargetPos(LONG x, LONG y);

// Dialog Data
	enum { IDD = IDD_DLG_FINGERMOVE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCheckFixedpos();
    CButton m_CkQuickFinger;
    CButton m_CkFixedPos;
    CEdit m_EditMoveToX;
    CEdit m_EditMoveToY;
	afx_msg void OnBnClickedCheckQuickfinger();
	CEdit m_EditQuickFreq;
	CEdit m_EditQuickTimes;
	CButton m_CkClick;
	afx_msg void OnBnClickedCheckIsclick();
};
