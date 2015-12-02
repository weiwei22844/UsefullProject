// DlgGrowth.cpp : implementation file
//

#include "stdafx.h"
#include "ImgProcess.h"
#include "DlgGrowth.h"
#include "afxdialogex.h"
#include "RegionGrowth.h"

// CDlgGrowth dialog

IMPLEMENT_DYNAMIC(CDlgGrowth, CDialogEx)

CDlgGrowth::CDlgGrowth(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgGrowth::IDD, pParent)
{
    m_fileName = _T("");
    m_nThreshold = 20;
}

CDlgGrowth::~CDlgGrowth()
{
}

void CDlgGrowth::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SCROLLBAR_THRESHOLD, m_ScrollBarThreshold);
}


BEGIN_MESSAGE_MAP(CDlgGrowth, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_SELECT, &CDlgGrowth::OnBnClickedBtnSelect)
    ON_BN_CLICKED(IDOK, &CDlgGrowth::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BTN_SAVE, &CDlgGrowth::OnBnClickedBtnSave)
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDCANCEL, &CDlgGrowth::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgGrowth message handlers


BOOL CDlgGrowth::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here
    m_ScrollBarThreshold.SetScrollRange(1, 100);
    m_ScrollBarThreshold.SetScrollPos(m_nThreshold);
    SetDlgItemInt(IDC_STATIC_THRESHOLD, m_nThreshold);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgGrowth::OnBnClickedBtnSelect()
{
    // TODO: Add your control notification handler code here
    CFileDialog fileOpenDlg(TRUE, _T("*.bmp"), NULL, OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
        _T("image files (*.jpg;*.bmp) |*.jpg;*.bmp|image file(*.bmp)|*.bmp;|image file(*.png)|*.png;|All Files (*.*)|*.*||"), NULL);
    fileOpenDlg.m_ofn.lpstrTitle = _T("Open File");
    if (fileOpenDlg.DoModal()==IDOK){    
        m_fileName = fileOpenDlg.GetPathName();
        m_cimg.Load(m_fileName);
        CRect rect;
        GetDlgItem(IDC_STATIC_SRC)->GetClientRect(&rect);
        CDC *pDC = GetDlgItem(IDC_STATIC_SRC)->GetDC();
        pDC->FillSolidRect(&rect,RGB(105,105,105));
        HDC hDC=  pDC->GetSafeHdc();
        m_cimg.ShowEx(hDC, 0, 0, rect.Width(), rect.Height(), 0, 0, true);
        ReleaseDC(pDC);
    }
}


void CDlgGrowth::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    //CDialogEx::OnOK();
    if(!m_fileName.IsEmpty()){
        IplImage* img;
        img = m_cimg.GetImage();
        //img = cvLoadImage(m_fileName.GetBuffer(0), CV_LOAD_IMAGE_ANYCOLOR);
        if(img){
            //建立需要使用的单通道8位深图像
            IplImage* img_gray;
            img_gray = cvCreateImage(cvGetSize(img), img->depth, 1);
            if(img_gray){
                cvSplit(img, NULL, img_gray, NULL, NULL);
#if 0
                cvNamedWindow("gray image", -1);
                cvShowImage("gray image", img_gray);
#endif

                //建立所需的与图像同样宽高的整型数矩阵并赋初值
                int * pres_mat;
                pres_mat = new int[img_gray->width * img_gray->height];
                if(pres_mat){
                    for(int i = 0; i < img_gray->width * img_gray->height; i++){
                        pres_mat[i] = INIT;
                    }
                    m_matImg.Create(img_gray->width, img_gray->height, 24);
                    IplImage * pMatImg = m_matImg.GetImage();
                    //开始进行区域生长
                    regionGrowth_pro(img_gray, pres_mat, img_gray->width, img_gray->height, 1, 1, m_nThreshold, pMatImg);

                    CRect rect;
                    GetDlgItem(IDC_STATIC_MAT)->GetClientRect(&rect);
                    CDC *pDC = GetDlgItem(IDC_STATIC_MAT)->GetDC();
                    pDC->FillSolidRect(&rect,RGB(105,105,105));
                    HDC hDC=  pDC->GetSafeHdc();
                    m_matImg.ShowEx(hDC, 0, 0, rect.Width(), rect.Height(), 0, 0, true);
                    ReleaseDC(pDC);

                    delete []pres_mat;
                }
                
                cvReleaseImage(&img_gray);
            }
        }
    }
}


void CDlgGrowth::OnBnClickedBtnSave()
{
    // TODO: Add your control notification handler code here
    BOOL bGenSuccess = FALSE;
    TCHAR szFilters[]= _T("bmg file(*.bmp)|*.bmp|jpg file(*.jpg)|*.jpg|png file(*.png)|*.png|All Files (*.*)|*.*||");
    CFileDialog dlgFile(FALSE, _T("Picture"), _T("*.bmp"),
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilters);
    CString fileName = _T("outline.bmp");
    dlgFile.GetOFN().lpstrFile = fileName.GetBuffer(MAX_PATH);
    dlgFile.GetOFN().nMaxFile = 100;
    if(dlgFile.DoModal()==IDOK)
    {
        fileName = dlgFile.GetOFN().lpstrFile;
        m_matImg.Save(fileName.GetBuffer(0));
    }
    fileName.ReleaseBuffer();
}


void CDlgGrowth::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: Add your message handler code here and/or call default
    int pos = m_ScrollBarThreshold.GetScrollPos();    // 获取水平滚动条当前位置
    switch (nSBCode)    
    {    
        // 如果向左滚动一列，则pos减1   
    case SB_LINEUP:    
        pos -= 1;    
        break;    
        // 如果向右滚动一列，则pos加1   
    case SB_LINEDOWN:    
        pos  += 1;    
        break;    
        // 如果向左滚动一页，则pos减10   
    case SB_PAGEUP:    
        pos -= 10;    
        break;    
        // 如果向右滚动一页，则pos加10   
    case SB_PAGEDOWN:    
        pos  += 10;    
        break;    
        // 如果滚动到最左端，则pos为1   
    case SB_TOP:    
        pos = 1;    
        break;    
        // 如果滚动到最右端，则pos为100   
    case SB_BOTTOM:    
        pos = 100;    
        break;      
        // 如果拖动滚动块滚动到指定位置，则pos赋值为nPos的值   
    case SB_THUMBPOSITION:    
        pos = nPos;    
        break;    
        // 下面的m_horiScrollbar.SetScrollPos(pos);执行时会第二次进入此函数，最终确定滚动块位置，并且会直接到default分支，所以在此处设置编辑框中显示数值  
    case SB_THUMBTRACK:
        pos = nPos;
        //TRACE(_T("%d\n"), nPos);
        break;
    default:
        return;
    }

    // 设置滚动块位置   
    m_ScrollBarThreshold.SetScrollPos(pos);
    SetDlgItemInt(IDC_STATIC_THRESHOLD, pos);
    m_nThreshold = pos;

    CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CDlgGrowth::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    CDialogEx::OnCancel();
}


BOOL CDlgGrowth::DestroyWindow()
{
    // TODO: Add your specialized code here and/or call the base class


    return CDialogEx::DestroyWindow();
}
