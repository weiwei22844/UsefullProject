#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CDlgPreview 对话框

class CDlgPreview : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPreview)

public:
	CDlgPreview(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgPreview();
	void UpdateCameraSetting();

	int m_nMaxWidth;
	int m_nMaxHeight;
	int m_nOffsetX;
	int m_nOffsetY;
	int m_nWidth;
	int m_nHeight;
	DOUBLE m_dPrevScale;
	BOOL m_bCapture;

	int m_nCaseIndex;
    int m_nFps;

	CPen m_ROIPen;

// 对话框数据
	enum { IDD = IDD_DLG_PREVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CStatic m_PreviewWnd;
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnBnClickedOk();
	CComboBox m_ComBoxExposureAuto;
	CEdit m_EditExposureTime;
	afx_msg void OnClose();
	afx_msg void OnBnClickedBtnApplyset();
	afx_msg void OnCbnSelchangeComboExposure();
	CEdit m_EditWidth;
	CEdit m_EditHeight;
	afx_msg void OnBnClickedBtnSetroi();
	CStatic m_StaticResolution;
	CEdit m_EditOffsetX;
	CEdit m_EditOffsetY;
	CStatic m_StaticInfo;
	CSliderCtrl m_SliderGain;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CButton m_CkEnableROI;
	afx_msg void OnBnClickedCheckRoi();
};
