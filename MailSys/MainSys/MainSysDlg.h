
// MainSysDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "DesktopSender.h"
#include "DesktopRecorder.h"

// CMainSysDlg 对话框
class CMainSysDlg : public CDialog
{
// 构造
public:
	CMainSysDlg(CWnd* pParent = NULL);	// 标准构造函数

    void ProcessMail(_bstr_t mailBody);
    int startServer();

    BOOL m_bListening;
    SOCKET m_sListen;
    BOOL m_bProcessCur;

// 对话框数据
	enum { IDD = IDD_MAINSYS_DIALOG };

    CStringArray m_strBodyArray;
    int m_nMailCount;
    DesktopSender* m_pDesktopSender;
    DesktopRecorder* m_pDesktopRecorder;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	void OnHotKey(MSG* pMsg);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnSend();
    afx_msg void OnBnClickedBtnTest();
    afx_msg void OnNcPaint();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnReceive();
	CListCtrl m_ctlMessagesList;
	CString m_strMailServer;
	CString m_strUserid;
	CString m_strPassword;
	CString m_strBody;
    afx_msg void OnLvnItemchangedListMails(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedBtnTest2();
    afx_msg void OnBnClickedBtnStartcapdesktop();
    afx_msg void OnBnClickedBtnStopcapdesktop();
    afx_msg void OnBnClickedBtnStartrecdesktop();
    afx_msg void OnBnClickedBtnStoprecdesktop();
    afx_msg void OnBnClickedBtnTransformh264();
};
