
// ImgProcessDlg.h : header file
//

#pragma once
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include "CvvImage.h"
#include "DlgBinaryParam.h"
#include "DlgCannySobel.h"

#define WM_VECTORCURBIN WM_USER+100

// CImgProcessDlg dialog
class CImgProcessDlg : public CDialogEx
{
// Construction
public:
	CImgProcessDlg(CWnd* pParent = NULL);	// standard constructor

    void ShowFile(BOOL bLoad = FALSE);
    void OnChangeBinThresHold(int nValue);
    void OnChangeBinPenWd(int nValue);
    void OnChangeBinPenColor(int nValue);
    void OnSaveBinResult();
    void OnVectorizeCurBin();

    void OnChangeCanny(int nValue);
    void OnChangeSobel(int nValue);
    void OnSaveCannySobelResult(int nValueCanny, int nValueSobel);

    void ExitCurProc(int nFlag = 0, int nValue = 1);

// Dialog Data
	enum { IDD = IDD_IMGPROCESS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

    CString m_filePath;
    CvvImage m_cimg;

    CDlgBinaryParam* m_pDlgBinParam;
    CDlgCannySobel* m_pDlgCannySobel;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnFileOpen();
    afx_msg void OnHelpAbout();
    afx_msg void OnFileExit();
    afx_msg void OnEditOutline();
    afx_msg void OnBnClickedRadioBlack();
    afx_msg void OnBnClickedRadioWhite();
    afx_msg void OnEditCanny();
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnClose();
    afx_msg void OnEditVectorize();
    afx_msg void OnBnClickedOk();
    afx_msg void OnEditShowvectoer();
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    afx_msg void OnEditPotrace();
    afx_msg void OnEditSharedmatting();
    afx_msg void OnEditGrowth();
    afx_msg void OnFileSavegray();
    afx_msg void OnFileSaveas();
    afx_msg void OnEditRotae();
    afx_msg void OnEditRotateclockwise();
    afx_msg void OnFilterIce();
    afx_msg void OnFilterOld();
    afx_msg void OnFilterComic();
    afx_msg void OnFilterSpread();
    afx_msg void OnFilterSin();
    afx_msg void OnFilterOil();
    afx_msg void OnFilterCartoon();
    afx_msg void OnDetectSkin();
    afx_msg void OnDetectPedestrian();
};

extern CImgProcessDlg* pMainDlg;
