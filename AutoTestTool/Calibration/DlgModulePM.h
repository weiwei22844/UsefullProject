#pragma once
#include "afxwin.h"

// CDlgModulePM dialog

class CDlgModulePM : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgModulePM)

public:
	CDlgModulePM(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgModulePM();

	void showBmp(CString path);

    ModuleData *m_pModuleData;
    CString m_TemplateFile;
	CString m_CurTemplate;

// Dialog Data
	enum { IDD = IDD_DLG_MODULEPM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedBtnMaketemplate();
	CStatic m_StaticPic;
	afx_msg void OnPaint();
};
