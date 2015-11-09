// DlgTemplate.cpp : implementation file
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgTemplate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef WS_EX_LAYERED
#define   WS_EX_LAYERED 0x00080000
#endif
typedef BOOL (WINAPI *MYFUNC) (HWND, COLORREF, BYTE, DWORD);

/////////////////////////////////////////////////////////////////////////////
// CDlgTemplate dialog


CDlgTemplate::CDlgTemplate(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgTemplate::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgTemplate)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	//CRect rect(0,0,m_xScreen,m_yScreen);
	//m_pBitmap=CBitmap::FromHandle(CopyScreenToBitmap(&rect));
	m_TemplateRectTracker.m_nStyle = CRectTracker::solidLine | CRectTracker::resizeInside;
	m_bFirstDraw = TRUE;
    m_bMakeTemplate = FALSE;
	m_bCheckValid = FALSE;
}


void CDlgTemplate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgTemplate)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgTemplate, CDialog)
	//{{AFX_MSG_MAP(CDlgTemplate)
	ON_WM_SETCURSOR()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgTemplate message handlers

BOOL CDlgTemplate::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	int xScreen = GetSystemMetrics(SM_CXSCREEN);
	int yScreen = GetSystemMetrics(SM_CYSCREEN);
	//SetWindowPos(&wndTopMost,0,0,xScreen,yScreen,SWP_SHOWWINDOW);
	SetWindowPos(&wndTopMost, m_Rect.left, m_Rect.top, m_Rect.Width(), m_Rect.Height(), SWP_SHOWWINDOW);

	//设置窗口的透明度
	SetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE,GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE)^WS_EX_LAYERED);
	//SetLayeredWindowAttributes(RGB(0,0,0),255,2);
	HINSTANCE hInst = LoadLibrary(_T("User32.DLL"));
	if(hInst)	
	{
		MYFUNC fun = NULL; 
		fun = (MYFUNC) GetProcAddress(hInst, _T("SetLayeredWindowAttributes"));
		if(fun)
			fun(this->GetSafeHwnd(), RGB(0,0,0), 50, 2);		// 最后一个参数 =1: 仅颜色 col 透明，=2: 窗口按照bAlpha变量进行透明处理
		FreeLibrary(hInst);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgTemplate::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// TODO: Add your message handler code here and/or call default
	if(pWnd == this&&m_TemplateRectTracker.SetCursor(this,nHitTest))
	{
		/*CPoint current;
		GetCursorPos(&current);
		int i = m_TemplateRectTracker.HitTest(current);
		TRACE("HitTest: %d\n", i);
		switch(i)
		{
		case 0:
		case 2:
			m_hCursor = AfxGetApp()->LoadCursor(IDC_CURSOR4);
			SetCursor(m_hCursor);
			break;
		case 1:
		case 3:
			m_hCursor = AfxGetApp()->LoadCursor(IDC_CURSOR3);
			SetCursor(m_hCursor);
			break;
		case 4:
		case 6:
			m_hCursor = AfxGetApp()->LoadCursor(IDC_CURSOR6);
			SetCursor(m_hCursor);
			break;
		case 5:
		case 7:
			m_hCursor = AfxGetApp()->LoadCursor(IDC_CURSOR5);
			SetCursor(m_hCursor);
			break;
		case 8:
			m_hCursor = AfxGetApp()->LoadCursor(IDC_CURSOR2);
			SetCursor(m_hCursor);
			break;
		default:
			break;
		}*/
		return TRUE;
	}
	SetCursor(AfxGetApp()->LoadCursor(IDC_CURSOR1));
	return TRUE;

	//return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CDlgTemplate::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	/*CRect   rect;
	GetClientRect(rect);    
	dc.FillSolidRect(rect,RGB(0,255,0));     //设置为绿色背景*/

	m_TemplateRectTracker.Draw(&dc);
	// Do not call CDialog::OnPaint() for painting messages
}

void CDlgTemplate::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	int iIn;
	
	iIn = m_TemplateRectTracker.HitTest(point);
	if(iIn<0&&m_bFirstDraw)
	{
		m_TemplateRectTracker.TrackRubberBand(this,point,TRUE);
		m_bFirstDraw = FALSE;
		Invalidate();
	}
	else
	{
		CClientDC dc(this);
		m_TemplateRectTracker.Draw(&dc);
		m_TemplateRectTracker.Track(this,point,TRUE);
		Invalidate();
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CDlgTemplate::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(!m_bFirstDraw)
	{
		if(m_bCheckValid){
			CRect rect;
			m_TemplateRectTracker.GetTrueRect(&rect);
			if(rect.left < m_ValidRect.left || rect.right > m_ValidRect.right 
				|| rect.top < m_ValidRect.top || rect.bottom > m_ValidRect.bottom){
					OutputDebugString("ROI区域超范围");
			}
		}
		//CopyScreenToBitmap(&(m_rtMyRectTracker.m_rect),TRUE);
        m_bMakeTemplate = TRUE;
		OnCancel();
	}

	CDialog::OnLButtonDblClk(nFlags, point);
}

void CDlgTemplate::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(!m_bFirstDraw)
	{
		m_TemplateRectTracker.m_rect.SetRect(0,0,0,0);
		m_bFirstDraw = TRUE;
		Invalidate();
	}
	else
	{
		OnCancel();
	}

	CDialog::OnRButtonUp(nFlags, point);
}
