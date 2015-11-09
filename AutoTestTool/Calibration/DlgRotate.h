#pragma once
#include "afxwin.h"


// CDlgRotate dialog

class CDlgRotate : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgRotate)

public:
	CDlgRotate(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgRotate();

    ModuleData *m_pModuleData;

// Dialog Data
	enum { IDD = IDD_DLG_ROTATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
	CButton m_CkAntiwise;
};
