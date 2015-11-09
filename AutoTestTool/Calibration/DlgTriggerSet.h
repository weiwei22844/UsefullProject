#pragma once
#include "afxwin.h"

typedef enum _TRIGGERMODE{
	TRIGGERMODE_NONE = 0,
	TRIGGERMODE_L2P,
	TRIGGERMODE_S2S,
	TRIGGERMODE_ZSL,
	TRIGGERMODE_VL,
	TRIGGERMODE_FPS,
	TRIGGERMODE_SL,
	TRIGGERMODE_CS,
}TRIGGERMODE;

class CCalibrationDlg;
// CDlgTriggerSet 对话框

class CDlgTriggerSet : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTriggerSet)

public:
	CDlgTriggerSet(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgTriggerSet();

	void SelectCase(int nIndex, BOOL bShowImage = TRUE);

    ModuleData *m_pModuleData;
// 对话框数据
	enum { IDD = IDD_DLG_TRIGGERSET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CComboBox m_ComboBoxCase;
	afx_msg void OnCbnSelchangeComboCase();
	CStatic m_StaticPicture;
	afx_msg void OnPaint();
    CEdit m_EditFilterLevel;
    CButton m_CkTriggerMode;
    CEdit m_EditTimeDelay;
    CEdit m_EditWaveCnt;
    CComboBox m_ComBoWaveFrq;
    CEdit m_EditInfo;
    afx_msg void OnBnClickedBtnHscpreview();
    afx_msg void OnBnClickedCancel();
	CEdit m_EditDBClkFreq;
	CEdit m_EditDBClkTimes;
};
