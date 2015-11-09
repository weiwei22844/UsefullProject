#if !defined(AFX_DLGTEMPLATE_H__07EB00A7_F91A_491E_8F40_B80DF0E16A93__INCLUDED_)
#define AFX_DLGTEMPLATE_H__07EB00A7_F91A_491E_8F40_B80DF0E16A93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgTemplate.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgTemplate dialog

class CDlgTemplate : public CDialog
{
// Construction
public:
	CDlgTemplate(CWnd* pParent = NULL);   // standard constructor

	BOOL m_bFirstDraw;
	HCURSOR m_hCursor;
	CRectTracker m_TemplateRectTracker;

	CRect m_Rect;
    BOOL m_bMakeTemplate;
	BOOL m_bCheckValid;
	CRect m_ValidRect;

// Dialog Data
	//{{AFX_DATA(CDlgTemplate)
	enum { IDD = IDD_TEMPLATE_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgTemplate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgTemplate)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGTEMPLATE_H__07EB00A7_F91A_491E_8F40_B80DF0E16A93__INCLUDED_)
