// DlgSharedMatting.cpp : implementation file
//

#include "stdafx.h"
#include "ImgProcess.h"
#include "DlgSharedMatting.h"
#include "afxdialogex.h"
#include "RegionGrowth.h"

const char *pstrMattingSrc = "SharedMatting原图";
const char *pstrMaskWnd = "SharedMattingMask";
static CvScalar g_scalarBinPen;
static CvPoint prev_pt = {-1,-1};
static CvPoint leftTop_pt = {-1,-1};
static CvPoint rightBottom_pt = {-1,-1};
IplImage *g_pSrcImage = NULL;
IplImage *g_pSrcClone = NULL;
IplImage *g_pMaskImage = NULL;
// CDlgSharedMatting dialog

IMPLEMENT_DYNAMIC(CDlgSharedMatting, CDialogEx)

CDlgSharedMatting::CDlgSharedMatting(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgSharedMatting::IDD, pParent)
{
    m_fileName = _T("");
    m_penWidth = 20;
}

CDlgSharedMatting::~CDlgSharedMatting()
{
    
}

void CDlgSharedMatting::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSharedMatting, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_SELECT, &CDlgSharedMatting::OnBnClickedBtnSelect)
    ON_BN_CLICKED(IDC_BTN_SAVE, &CDlgSharedMatting::OnBnClickedBtnSave)
    ON_BN_CLICKED(IDOK, &CDlgSharedMatting::OnBnClickedOk)
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CDlgSharedMatting message handlers

#define PART 4
BOOL CDlgSharedMatting::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here
    showSrc();

    IplImage *src1, *src2;
    CvFont font;
    float alpha_value;
    char alpha_str[10];
    char wnd_title[100];

#if 0
    src1 = cvLoadImage("F:\\software\\openCV\\2.3.1\\opencv\\vs2010\\bin\\Debug\\beauty.jpg", 1);
    src2 = cvCreateImage(cvSize(src1->width / PART, src1->height / PART),
        src1->depth, src1->nChannels);
    cvResize(src1, src2);

    //初始化字体，为以后的绘制文字做准备
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5);
    for (int x = 0; x < PART; x+=1){
        for (int y = 0; y < PART; y+=1){
            //计算alpha值
            alpha_value = (float)(x + y * PART) / (PART * PART -1);
            sprintf(alpha_str, "%.3f", alpha_value);

            //设定RIO并进行该区域的Alpha blending
            cvSetImageROI(src1, cvRect(x * (src1->width / PART), y * (src1->height / PART),
                src1->width / PART, src1->height / PART));
            cvAddWeighted(src2, alpha_value, src1, 1 - alpha_value, 0, src1);

            //绘制图框和文字
            cvRectangle(src1, cvPoint(0, 0),
                cvPoint(src1->width / PART, src1->height / PART),
                cvScalar(0, 64, 238), 1);
            cvPutText(src1, alpha_str, cvPoint(10, 20), &font, cvScalar(0, 0, 0));

            cvResetImageROI(src1);
        }
    }

    //显示混合结果
    strcpy(wnd_title, "Alpha_blending by Afu 2010/7/26");
    cvNamedWindow(wnd_title);
    cvShowImage(wnd_title, src1);
#endif


    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSharedMatting::OnBnClickedBtnSelect()
{
    // TODO: Add your control notification handler code here
    CFileDialog fileOpenDlg(TRUE, _T("*.bmp"), NULL, OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY,
        _T("image files (*.jpg;*.bmp) |*.jpg;*.bmp|image file(*.bmp)|*.bmp;|image file(*.png)|*.png;|All Files (*.*)|*.*||"), NULL);
    fileOpenDlg.m_ofn.lpstrTitle = _T("Open File");
    if (fileOpenDlg.DoModal()==IDOK)
    {    
        m_fileName = fileOpenDlg.GetPathName();
#if 0
        m_cimg.Load(m_fileName);
        CRect rect;
        GetDlgItem(IDC_STATIC_SRC)->GetClientRect(&rect);
        CDC *pDC = GetDlgItem(IDC_STATIC_SRC)->GetDC();
        pDC->FillSolidRect(&rect,RGB(105,105,105));
        HDC hDC=  pDC->GetSafeHdc();
        m_cimg.ShowEx(hDC, 0, 0, rect.Width(), rect.Height(), 0, 0, true);
        ReleaseDC(pDC);
#else
        showSrc();
#endif
    }
}


void CDlgSharedMatting::OnBnClickedBtnSave()
{
    // TODO: Add your control notification handler code here
    if(g_pMaskImage){
        cvSaveImage("mask.jpg", g_pMaskImage);
    }
}


void CDlgSharedMatting::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    //CDialogEx::OnOK();

}


void CDlgSharedMatting::OnMouseMove(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default

    CDialogEx::OnMouseMove(nFlags, point);
}

void DrawTransRec(IplImage* img,int x,int y,int width,int height,CvScalar color,double alpha)
{
    IplImage * rec=cvCreateImage(cvSize(width,height),img->depth,img->nChannels);
    cvRectangle(rec,cvPoint(0,0),cvPoint(width,height),color,-1);
    cvSetImageROI(img,cvRect(x,y,width,height));
    cvAddWeighted(img,alpha,rec,1-alpha,0.0,img);
    cvResetImageROI(img);
}

void OnMouse(int event, int x, int y, int flags, void* param)
{
    HWND hWnd = (HWND)cvGetWindowHandle(pstrMattingSrc);
    CDlgSharedMatting *pDlg = (CDlgSharedMatting*)param;
    CRect rc;
    ::GetWindowRect(hWnd, rc);
    if(hWnd){
        int r = pDlg->m_penWidth/2;
        HCURSOR myCursor;
        myCursor=(HCURSOR)LoadImage(NULL,"res/circle.cur",IMAGE_CURSOR, pDlg->m_penWidth, pDlg->m_penWidth,LR_LOADFROMFILE);
        //SetCursor(myCursor);
        //SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
        if(event == CV_EVENT_LBUTTONUP || !(flags & CV_EVENT_FLAG_LBUTTON)){
            if(prev_pt.x == x && prev_pt.y == y){
                cvShowImage("test", g_pMaskImage);
                return;
                int arr[1];
                arr[0] = pDlg->m_polyLineVector.size();//+2;
                CvPoint *pts[1];//单轮廓
                pts[0] = new CvPoint[arr[0]];
                int i;
                for (i = 0;i < pDlg->m_polyLineVector.size();i++)
                {
                    pts[0][i].x = pDlg->m_polyLineVector[i].x;
                    pts[0][i].y = pDlg->m_polyLineVector[i].y;
                }
                cvFillPoly(g_pMaskImage,pts,arr,1,CV_RGB(255,255,255));
                cvPolyLine(g_pMaskImage,pts,arr,1,1,CV_RGB(128,128,128), pDlg->m_penWidth);
                delete[] pts[0];
                //建立所需的与图像同样宽高的整型数矩阵并赋初值
                int * pres_mat;
                pres_mat = new int[g_pMaskImage->width * g_pMaskImage->height];
                for(int i = 0; i < g_pMaskImage->width * g_pMaskImage->height; i++)
                {
                    pres_mat[i] = 0;
                }

                //开始进行区域生长
                //regionGrowth_pro(g_pMaskImage, pres_mat, g_pMaskImage->width, g_pMaskImage->height, 1, 1, 50);

                if(leftTop_pt.y != -1 && rightBottom_pt.y != -1){
#if 1
                    pts[0] = new CvPoint[4];
                    pts[0][0].x=0;
                    pts[0][0].y=leftTop_pt.y;
                    pts[0][1].x=g_pMaskImage->width;
                    pts[0][1].y=rightBottom_pt.y;
                    pts[0][2].x=g_pMaskImage->width;
                    pts[0][2].y=g_pMaskImage->height;
                    pts[0][3].x=0;
                    pts[0][3].y=g_pMaskImage->height;
                    arr[0] = 4;
#else
                    pts[0][i].x=g_pMaskImage->width;
                    pts[0][i].y=g_pMaskImage->height;
                    i++;
                    pts[0][i].x=0;
                    pts[0][i].y=g_pMaskImage->height;
#endif
                    //cvFillPoly(g_pMaskImage,pts,arr,1,CV_RGB(255,255,255));
                    //cvPolyLine(g_pMaskImage,pts,arr,1,0,CV_RGB(128,128,128), pDlg->m_penWidth);
                    delete[] pts[0];
                }else if(leftTop_pt.x != -1 && rightBottom_pt.x != -1){
                    ;
                }
                cvShowImage("test", g_pMaskImage);
                pDlg->m_polyLineVector.clear();
                leftTop_pt.x=leftTop_pt.y=-1;
                rightBottom_pt.x=rightBottom_pt.y=-1;
            }
            
            prev_pt = cvPoint(-1,-1);
            CvPoint pt = cvPoint(x,y);
            CString str;
            str.Format("%d %d %d %d\n", pt.x, pt.y, prev_pt.x, prev_pt.y);
            OutputDebugString(str);
        }else if( event == CV_EVENT_LBUTTONDOWN ){
            prev_pt = cvPoint(x,y);
            /*CClientDC dc(CWnd::FromHandle(hWnd));
            dc.SetROP2(R2_NOTXORPEN);
            dc.SelectStockObject(NULL_BRUSH);
            CPen pen;
            pen.CreatePen(PS_SOLID,1,RGB(255, 0 , 0));  
            dc.SelectObject(&pen);
            int rectX, rectY, rX, rY;
            RECT rect;
            GetClientRect( hWnd, &rect );
            rectX = x*(rect.right-rect.left)/g_pSrcImage->width;
            rectY = y*(rect.bottom-rect.top)/g_pSrcImage->height;
            rX = r*(rect.right-rect.left)/g_pSrcImage->width;
            rY = r*(rect.bottom-rect.top)/g_pSrcImage->height;
            dc.Ellipse(rectX-rX, rectY-rY, rectX+rX, rectY+rY);
            CString str;
            str.Format("x %d y %d r %d\n", x, y, r);*/
        }else if( event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON) )
        {
            CvPoint pt = cvPoint(x,y);
            if( prev_pt.x < 0 )
                prev_pt = pt;

            CString str;
            str.Format("MOVE %d %d %d %d\n", pt.x, pt.y, prev_pt.x, prev_pt.y);
            OutputDebugString(str);

            if(!g_pSrcClone){
                g_pSrcClone=(IplImage*)cvClone(g_pSrcImage);
            }
            if(!g_pMaskImage){
                g_pMaskImage=cvCreateImage(cvGetSize(g_pSrcImage), IPL_DEPTH_8U, 1);
                cvZero(g_pMaskImage);
            }
            cvLine( g_pSrcClone, prev_pt, pt, g_scalarBinPen, pDlg->m_penWidth, 8, 0 );
            cvLine( g_pMaskImage, prev_pt, pt, CV_RGB(128,128,128), pDlg->m_penWidth, 8, 0 );
            IplImage * pTemp=(IplImage*)cvClone(g_pSrcImage);
            cvAddWeighted(g_pSrcImage,0.5,g_pSrcClone,0.5,0.0,pTemp);
            prev_pt = pt;
            cvShowImage(pstrMattingSrc, pTemp);
            //cvShowImage("test", g_pSrcClone);
            cvReleaseImage(&pTemp);
            pDlg->m_polyLineVector.push_back(pt);

            if(x <= 3){
                leftTop_pt.y=y;
            }else if(x >= g_pSrcImage->width - 3){
                rightBottom_pt.y=y;
            }else if(y <= 3){
                leftTop_pt.x=x;
            }else if(y >= g_pSrcImage->height - 3){
                rightBottom_pt.x=x;
            }
#if 0
            CClientDC dc(CWnd::FromHandle(hWnd));
            dc.SetROP2(R2_NOTXORPEN);
            //CBrush *pbrush=CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)   );
            //dc.SelectObject(pbrush);
            dc.SelectStockObject(NULL_BRUSH);
            CPen pen;
            pen.CreatePen(PS_SOLID,1,RGB(255, 0 , 0));  
            dc.SelectObject(&pen);
            int rectX, rectY, rX, rY;
            RECT rect;
            GetClientRect( hWnd, &rect );
            rectX = x*(rect.right-rect.left)/g_pSrcImage->width;
            rectY = y*(rect.bottom-rect.top)/g_pSrcImage->height;
            rX = r*(rect.right-rect.left)/g_pSrcImage->width;
            rY = r*(rect.bottom-rect.top)/g_pSrcImage->height;
            dc.Ellipse(rectX-rX, rectY-rY, rectX+rX, rectY+rY);
#endif
        }else if( event == CV_EVENT_MOUSEMOVE ){
            CvPoint pt = cvPoint(x,y);
            CString str;
            str.Format("%d %d %d %d\n", pt.x, pt.y, prev_pt.x, prev_pt.y);
            OutputDebugString(str);

            /*fillContSingle.push_back(cvPoint(1,1));
            fillContSingle.push_back(cvPoint(1,2));
            fillContSingle.push_back(cvPoint(2,2));
            fillContSingle.push_back(cvPoint(2,1));
            fillContSingle.push_back(cvPoint(1,1));

            std::vector<std::vector<CvPoint>> fillContAll;
            //fill the single contour 
            //(one could add multiple other similar contours to the vector)
            fillContAll.push_back(fillContSingle);
            if(g_pMaskImage){
                cvFillPoly(g_pMaskImage, fillContAll, fillContSingle.size(), 0,  CV_RGB(255,128,128));
            }*/
            /*
            cvFillPoly( CvArr* img, CvPoint** pts, const int* npts,
                int contours, CvScalar color,
                int line_type CV_DEFAULT(8), int shift CV_DEFAULT(0) );
                */
        }
    }
}

void CDlgSharedMatting::showSrc()
{
    if(m_fileName.IsEmpty())
        return;
    sm.loadImage(m_fileName.GetBuffer(0));
    g_scalarBinPen = cvScalar(0,0,255);
    g_pSrcImage = cvLoadImage(m_fileName, CV_LOAD_IMAGE_UNCHANGED);

    /*IplImage * pTemp=(IplImage*)cvClone(g_pSrcImage);
    cvCircle(pTemp,cvPoint(200,200),100,CV_RGB(255,0,0),-1);
    cvAddWeighted(g_pSrcImage,0.4,pTemp,0.6,0.0,g_pSrcImage);
    cvReleaseImage(&pTemp);*/

    int wd = g_pSrcImage->width;
    int ht = g_pSrcImage->height;
    int widthScr = GetSystemMetrics( SM_CXSCREEN ) / 2;
    int heightScr = GetSystemMetrics( SM_CYSCREEN ) / 2;
    int widthShow, heightShow;

    if(wd > widthScr || ht > heightScr){        // need stretch
        if(heightScr*wd < widthScr*ht){
            heightShow = heightScr;
            widthShow = heightShow*wd/ht;
        }else{
            widthShow = widthScr;
            heightShow = heightScr*ht/wd;
        }
    }else{
        widthShow = wd;
        heightShow = ht;
    }


    int arr[1];
    CvPoint *pts[1];
    pts[0] = new CvPoint[5];
    pts[0][0].x=0;
    pts[0][0].y=g_pSrcImage->height/2;
    pts[0][1].x=g_pSrcImage->width;
    pts[0][1].y=g_pSrcImage->height*2/3;
    pts[0][2].x=g_pSrcImage->width;
    pts[0][2].y=g_pSrcImage->height;
    pts[0][3].x=0;
    pts[0][3].y=g_pSrcImage->height;
#if 0
    pts[0][0].x=1;
    pts[0][0].y=1;
    pts[0][1].x=201;
    pts[0][1].y=1;
    pts[0][2].x=201;
    pts[0][2].y=201;
    pts[0][3].x=1;
    pts[0][3].y=201;
    pts[0][4].x=1;
    pts[0][4].y=1;
#endif
    arr[0] = 4;
    //cvFillPoly(g_pSrcImage,pts,arr,1,CV_RGB(255,255,255));
    //cvPolyLine(g_pSrcImage,pts,arr,1,1,CV_RGB(128,128,128), m_penWidth);
    delete[] pts[0];

    cvNamedWindow(pstrMattingSrc, CV_WINDOW_NORMAL);
    cvResizeWindow(pstrMattingSrc, widthShow, heightShow);
    cvShowImage(pstrMattingSrc, g_pSrcImage);

    CString str;
    str.Format("widthShow: %d heightShow: %d", widthShow, heightShow);
    //OutputDebugString(str);

    cvSetMouseCallback(pstrMattingSrc, OnMouse, this);
#if 0
    cvWaitKey(0);

    if(g_pSrcImage)
        cvReleaseImage(&g_pSrcImage);
    if(g_pSrcClone)
        cvReleaseImage(&g_pSrcClone);
    if(g_pMaskImage)
        cvReleaseImage(&g_pMaskImage);
    g_pSrcImage = NULL;
    g_pSrcClone = NULL;
    g_pMaskImage = NULL;
#endif
}


BOOL CDlgSharedMatting::DestroyWindow()
{
    // TODO: Add your specialized code here and/or call the base class
    cvDestroyWindow(pstrMattingSrc);
    if(g_pSrcImage)
        cvReleaseImage(&g_pSrcImage);
    if(g_pSrcClone)
        cvReleaseImage(&g_pSrcClone);
    if(g_pMaskImage)
        cvReleaseImage(&g_pMaskImage);
    g_pSrcImage = NULL;
    g_pSrcClone = NULL;
    g_pMaskImage = NULL;

    return CDialogEx::DestroyWindow();
}
