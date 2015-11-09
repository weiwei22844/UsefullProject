#pragma once

// CDlgInitSetting dialog

class CDlgInitSetting : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgInitSetting)

public:
	CDlgInitSetting(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgInitSetting();

    ModuleData *m_pModuleData;
    CString m_ScriptFile;

// Dialog Data
	enum { IDD = IDD_DLG_INITSETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedBtnInitscript();
};
