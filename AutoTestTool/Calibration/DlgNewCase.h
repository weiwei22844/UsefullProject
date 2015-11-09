#pragma once


// CDlgNewCase 对话框

class CDlgNewCase : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgNewCase)

public:
	CDlgNewCase(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgNewCase();

	CString m_UserCaseName;

// 对话框数据
	enum { IDD = IDD_DLG_NEWCASE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
