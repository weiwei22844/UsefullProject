#pragma once


// CH264ToMP4Dlg 对话框

class CH264ToMP4Dlg : public CDialog
{
	DECLARE_DYNAMIC(CH264ToMP4Dlg)

public:
	CH264ToMP4Dlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CH264ToMP4Dlg();

    CString m_filePath;

// 对话框数据
	enum { IDD = IDD_DLG_H264TOMP4 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnSelectfile();
    afx_msg void OnBnClickedBtnTransform();
    DWORD m_dwWidth;
    DWORD m_dwHeight;
    float m_fFrameRate;
};
