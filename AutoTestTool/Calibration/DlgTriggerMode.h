#pragma once

// CDlgTriggerMode dialog

class CDlgTriggerMode : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTriggerMode)

public:
	CDlgTriggerMode(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgTriggerMode();

    ModuleData *m_pModuleData;

// Dialog Data
	enum { IDD = IDD_DLG_TRIGGERMODE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
};
