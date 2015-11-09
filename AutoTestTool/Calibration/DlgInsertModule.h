#pragma once


// CDlgInsertModule 对话框

class CDlgInsertModule : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgInsertModule)

public:
	CDlgInsertModule(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgInsertModule();

    int m_nCheckRadio;

// 对话框数据
	enum { IDD = IDD_DLG_INSERTMODULE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
};
