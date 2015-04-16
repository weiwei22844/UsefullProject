// H264ToMP4Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MainSys.h"
#include "H264ToMP4Dlg.h"
#include "MP4Encoder.h"


// CH264ToMP4Dlg 对话框

IMPLEMENT_DYNAMIC(CH264ToMP4Dlg, CDialog)

CH264ToMP4Dlg::CH264ToMP4Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CH264ToMP4Dlg::IDD, pParent)
    , m_dwWidth(352)
    , m_dwHeight(288)
    , m_fFrameRate(25)
{
    m_filePath=_T("");
}

CH264ToMP4Dlg::~CH264ToMP4Dlg()
{
}

void CH264ToMP4Dlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_WIDTH, m_dwWidth);
    DDX_Text(pDX, IDC_EDIT_HEIGHT, m_dwHeight);
    DDX_Text(pDX, IDC_EDIT_FRAMERATE, m_fFrameRate);
}


BEGIN_MESSAGE_MAP(CH264ToMP4Dlg, CDialog)
    ON_BN_CLICKED(IDC_BTN_SELECTFILE, &CH264ToMP4Dlg::OnBnClickedBtnSelectfile)
    ON_BN_CLICKED(IDC_BTN_TRANSFORM, &CH264ToMP4Dlg::OnBnClickedBtnTransform)
END_MESSAGE_MAP()


// CH264ToMP4Dlg 消息处理程序

void CH264ToMP4Dlg::OnBnClickedBtnSelectfile()
{
    // TODO: 在此添加控件通知处理程序代码
    CString filter = _T("h264 File (*.h264;*.264)|*.h264;*.264|All File (*.*)|*.*||");

    CFileDialog fileDlg(TRUE, NULL, NULL, NULL, filter, this);
    // Specifies that the user can type only names of existing files in the File Name entry field. If this flag 
    // is specified and the user enters an invalid name, the dialog box procedure displays a warning in a message box
    fileDlg.m_ofn.Flags |= OFN_FILEMUSTEXIST;
    fileDlg.m_ofn.lpstrTitle = _T("Loading file...");
    if (fileDlg.DoModal() == IDOK) {
        AfxGetApp()->BeginWaitCursor();
        m_filePath = fileDlg.GetPathName();	///Returns the full path of the selected file
        GetDlgItem(IDC_STATIC_PATH)->SetWindowText(m_filePath);
        //m_tt.AddTool(GetDlgItem(IDC_STATIC_FILE1), m_fileName1);
    }
}

void CH264ToMP4Dlg::OnBnClickedBtnTransform()
{
    // TODO: 在此添加控件通知处理程序代码
    if(!m_filePath.IsEmpty()){
        UpdateData();

        GetDlgItem(IDC_BTN_TRANSFORM)->EnableWindow(FALSE);
        int nPos= m_filePath.ReverseFind('\\'); 
        CString fileName = m_filePath.Right(m_filePath.GetLength() - nPos - 1 );
        nPos= fileName.ReverseFind('.'); 
        fileName = fileName.Left(fileName.GetLength() - nPos -1);
        fileName += _T(".mp4");

        /*_splitpath(
            const char *path,
            char *drive,
            char *dir,
            char *fname,
            char *ext 
            );
        CString 
        _tsplitpath();*/
        MP4Encoder mp4Encoder;
        // convert H264 file to mp4 file
        //声明标识符
        USES_CONVERSION;
        mp4Encoder.WriteH264File(T2A(m_filePath), T2A(fileName), m_dwWidth, m_dwHeight, m_fFrameRate);
        GetDlgItem(IDC_BTN_TRANSFORM)->EnableWindow(TRUE);
    }
}
