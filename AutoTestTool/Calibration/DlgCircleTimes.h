#pragma once


// CDlgCircleTimes 对话框

class CDlgCircleTimes : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCircleTimes)

public:
	CDlgCircleTimes(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgCircleTimes();

	int m_nTimes;

// 对话框数据
	enum { IDD = IDD_DLG_CIRCLETIMES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
