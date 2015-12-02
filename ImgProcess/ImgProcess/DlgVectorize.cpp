// DlgVectorize.cpp : implementation file
//

#include "stdafx.h"
#include "ImgProcess.h"
#include "DlgVectorize.h"
#include "afxdialogex.h"
#include "Redirect.h"
#include "drawVector.h"

// CDlgVectorize dialog

IMPLEMENT_DYNAMIC(CDlgVectorize, CDialogEx)

CDlgVectorize::CDlgVectorize(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgVectorize::IDD, pParent)
{
    m_nFlag = 0;        // 0 ras2vec 1 potrace
    m_runParam = "ras2vec.exe -p -X 1 -Y 1 ";     //  -x 
    m_file = "binaryResult.bmp";
}

CDlgVectorize::~CDlgVectorize()
{
}

void CDlgVectorize::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_RUN, m_EditRun);
    DDX_Control(pDX, IDC_EDIT_OUTPUT, m_EditOutput);
    DDX_Control(pDX, IDC_BTN_RUN, m_BtnRun);
    DDX_Control(pDX, IDC_BTN_STOP, m_BtnStop);
    DDX_Control(pDX, IDC_BTN_EXIT, m_BtnExit);
    DDX_Control(pDX, IDC_BTN_SHOW, m_BtnShow);
    DDX_Control(pDX, IDC_STATIC_FILE, m_StaticFile);
}


BEGIN_MESSAGE_MAP(CDlgVectorize, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_RUN, &CDlgVectorize::OnBnClickedBtnRun)
    ON_BN_CLICKED(IDC_BTN_STOP, &CDlgVectorize::OnBnClickedBtnStop)
    ON_BN_CLICKED(IDC_BTN_EXIT, &CDlgVectorize::OnBnClickedBtnExit)
    ON_BN_CLICKED(IDC_BTN_SHOW, &CDlgVectorize::OnBnClickedBtnShow)
    ON_BN_CLICKED(IDC_BTN_SELECTOUTLINE, &CDlgVectorize::OnBnClickedBtnSelectoutline)
END_MESSAGE_MAP()


// CDlgVectorize message handlers


void CDlgVectorize::OnBnClickedBtnRun()
{
    // TODO: Add your control notification handler code here
    CString Command;
    CString file;

    UpdateData();

    //if ( m_bClearOutput )
    {
        m_EditOutput.SetWindowText(_T(""));
    }

    m_EditRun.GetWindowText(Command);
    m_StaticFile.GetWindowText(file);
    m_file = file;
    Command += file;

    m_pRedirect = new CRedirect((LPTSTR)(LPCTSTR)Command,  &m_EditOutput);

    UpdateControls();

    m_pRedirect->Run();

    delete m_pRedirect;

    m_pRedirect = 0;

    UpdateControls();

    m_EditRun.SetFocus();
    m_EditRun.SetSel(0, -1);
}


void CDlgVectorize::OnBnClickedBtnStop()
{
    // TODO: Add your control notification handler code here
    if ( m_pRedirect )
    {
        m_pRedirect->Stop();
    }
}


void CDlgVectorize::OnBnClickedBtnExit()
{
    // TODO: Add your control notification handler code here
    CDialogEx::OnOK();
}

void CDlgVectorize::UpdateControls()
{
    CMenu* pSysMenu = GetSystemMenu(FALSE);

    if ( !pSysMenu )
    {
        return;
    }

    if ( m_pRedirect )
    {
        m_EditRun.EnableWindow(FALSE);
        //m_BtnBrowse.EnableWindow(FALSE);
        m_BtnRun.EnableWindow(FALSE);
        m_BtnStop.EnableWindow(TRUE);
        m_BtnShow.EnableWindow(FALSE);
        m_BtnExit.EnableWindow(FALSE);

        //----------------------------------------------------------------------
        //	Disable Close in System Menu (this also disables close box).
        //----------------------------------------------------------------------
        if (pSysMenu != NULL)
        {
            pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
        }
    }
    else 
    {
        m_EditRun.EnableWindow(TRUE);
        //m_BtnBrowse.EnableWindow(TRUE);
        m_BtnRun.EnableWindow(m_EditRun.GetWindowTextLength() > 0);
        m_BtnStop.EnableWindow(FALSE);
        m_BtnShow.EnableWindow(TRUE);
        m_BtnExit.EnableWindow(TRUE);

        //----------------------------------------------------------------------
        //	Enable Close in System Menu (this also enables close box).
        //----------------------------------------------------------------------
        if (pSysMenu != NULL)
        {
            pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
        }
    }
}


BOOL CDlgVectorize::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here
    if(m_nFlag == 0){
        m_EditRun.SetWindowText(m_runParam);
        m_StaticFile.SetWindowText(m_file);
    }else if(m_nFlag == 1){
        m_EditRun.SetWindowText("potrace.exe -b dxf --progress -L 0 -B 0 -x 1x1 ");
    }
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgVectorize::OnBnClickedBtnShow()
{
    // TODO: Add your control notification handler code here
    //showVectorWnd("binaryResult.ply");
    int nPos= m_file.ReverseFind('\\');
    CString fileName = m_file.Right(m_file.GetLength() - nPos - 1 );
    nPos= fileName.ReverseFind('.');
    //fileName = fileName.Left(fileName.GetLength() - nPos -1);
    fileName = fileName.Left(nPos);
    if(m_nFlag == 0){
        fileName += ".ply";
    }else{
        fileName += ".json";
    }
    showVectorWnd(fileName.GetBuffer(0));
}


void CDlgVectorize::OnBnClickedBtnSelectoutline()
{
    // TODO: Add your control notification handler code here
    CFileDialog fileOpenDlg(TRUE, _T("*.bmp"), NULL, OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
        _T("image files (*.jpg;*.bmp) |*.jpg;*.bmp|image file(*.bmp)|*.bmp;|image file(*.png)|*.png;|All Files (*.*)|*.*||"), NULL);
    fileOpenDlg.m_ofn.lpstrTitle = _T("Open File");
    if (fileOpenDlg.DoModal()==IDOK)
    {    
        m_file = fileOpenDlg.GetPathName();
        m_StaticFile.SetWindowText(m_file);
    }
}
