#pragma once

#include "CvvImage.h"
#include "sharedmatting.h"
// CDlgSharedMatting dialog

class CDlgSharedMatting : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSharedMatting)

public:
    CString m_fileName;

    CvvImage m_cimg;
    SharedMatting sm;
    int m_penWidth;
    std::vector<CvPoint> m_polyLineVector;

    void showSrc();

public:
	CDlgSharedMatting(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSharedMatting();

// Dialog Data
	enum { IDD = IDD_DLG_SHAREDMATTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBtnSelect();
    afx_msg void OnBnClickedBtnSave();
    afx_msg void OnBnClickedOk();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    virtual BOOL DestroyWindow();
};
