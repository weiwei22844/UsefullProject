#pragma once


// CDlgTimeDelay 对话框

class CDlgTimeDelay : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTimeDelay)

public:
	CDlgTimeDelay(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgTimeDelay();

    ModuleData *m_pModuleData;

// 对话框数据
	enum { IDD = IDD_DLG_TIMEDELAY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
};
