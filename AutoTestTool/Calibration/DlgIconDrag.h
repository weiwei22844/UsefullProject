#pragma once
#include "afxwin.h"

// CDlgIconDrag dialog

class CDlgIconDrag : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgIconDrag)

public:
	CDlgIconDrag(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgIconDrag();
	void SetStartEndPos(LONG lStartX, LONG lStartY, LONG lEndX, LONG lEndY);
	void SetTargetPos(LONG x, LONG y);

    ModuleData *m_pModuleData;

// Dialog Data
	enum { IDD = IDD_DLG_ICONDRAG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
	CButton m_CkSpecifyPos;
	CEdit m_EditStartX;
	CEdit m_EditStartY;
	CEdit m_EditEndX;
	CEdit m_EditEndY;
	afx_msg void OnBnClickedCheckSpecifypos();
	CEdit m_EditTimeBefore;
	CEdit m_EditTimeAfter;
};
