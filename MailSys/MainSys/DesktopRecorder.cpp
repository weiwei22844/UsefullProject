#include "StdAfx.h"
#include "DesktopRecorder.h"
extern "C"
{
//#include "libavcodec\avcodec.h"
//#include "libavformat\avformat.h"
#include "libswscale\swscale.h"
};

#define BITSPERPIXEL		24
static LPVOID	pBits=NULL;
HBITMAP	hDesktopCompatibleBitmap=NULL;
HDC		hDesktopCompatibleDC=NULL;
HDC		hDesktopDC=NULL;
HWND	hDesktopWnd=NULL;

DesktopRecorder::DesktopRecorder(void)
{
    m_nWidth = 353;
    m_nHeight = 288;
    m_nFrameRate = 1;
    m_bRun = FALSE;
}

DesktopRecorder::~DesktopRecorder(void)
{
}

BOOL DesktopRecorder::init(int nWidth, int nHeight, int nFrameRate)
{
    // format width and height because x264 and sws_scale
    m_nWidth=nWidth/8>0?nWidth/8*8:8;
    m_nHeight=nHeight/16>0?nHeight/16*16:16;
    m_nFrameRate=nFrameRate;
    x264_param_default(&m_param);         //设置默认参数具体见common/common.c
    return TRUE;
}

#if 1
DWORD WINAPI DesktopRecdProc(LPVOID pParam)
{
    DesktopRecorder* pDesktopRecorder = (DesktopRecorder *)pParam;
    BITMAPINFO	bmpInfo;
    ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
    bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biBitCount=BITSPERPIXEL;
    bmpInfo.bmiHeader.biCompression = BI_RGB;
    bmpInfo.bmiHeader.biWidth=pDesktopRecorder->m_nWidth;
    bmpInfo.bmiHeader.biHeight=-pDesktopRecorder->m_nHeight;
    bmpInfo.bmiHeader.biPlanes=1;
    bmpInfo.bmiHeader.biSizeImage=(bmpInfo.bmiHeader.biWidth*bmpInfo.bmiHeader.biBitCount+31)/32*4*abs(bmpInfo.bmiHeader.biHeight);

    hDesktopWnd=GetDesktopWindow();
    hDesktopDC=GetDC(hDesktopWnd);
    hDesktopCompatibleDC=CreateCompatibleDC(hDesktopDC);
    hDesktopCompatibleBitmap=CreateDIBSection(hDesktopDC,&bmpInfo,DIB_RGB_COLORS,&pBits,NULL,0);
    if(hDesktopCompatibleDC==NULL || hDesktopCompatibleBitmap == NULL)
    {
        OutputDebugString(_T("Unable to Create Desktop Compatible DC/Bitmap"));
        return 0;
    }
    SelectObject(hDesktopCompatibleDC,hDesktopCompatibleBitmap);

    HANDLE hWaitEvent = NULL;
    x264_t *h264Encoder=NULL;
    FILE *f264=NULL;
    struct SwsContext* convertCtx=NULL;
    x264_picture_t picin;                                   //传入图像YUV
    x264_picture_t picout;                                  //输出图像RAW
    x264_picture_init(&picin);                              //初始化图片信息
    x264_picture_alloc(&picin,X264_CSP_I420,pDesktopRecorder->m_nWidth, pDesktopRecorder->m_nHeight);//图片按I420格式分配空间，最后要
    pDesktopRecorder->m_param.i_width=pDesktopRecorder->m_nWidth;
    pDesktopRecorder->m_param.i_height=pDesktopRecorder->m_nHeight;
    pDesktopRecorder->m_param.i_keyint_min=5;               //关键帧最小间隔
    pDesktopRecorder->m_param.i_keyint_max=250;             //关键帧最大间隔
    pDesktopRecorder->m_param.b_repeat_headers=0;           //关键帧前面是否放sps跟pps帧，0 否 1，放	
    //	param.analyse.i_subpel_refine=7;//压缩级别1~7
    pDesktopRecorder->m_param.i_log_level=X264_LOG_NONE;    //设置信息级别
    h264Encoder=x264_encoder_open(&(pDesktopRecorder->m_param));      //根据参数初始化X264级别
    if(!h264Encoder){
        MessageBox(NULL, _T("x264_encoder_open failed"), _T("Error"), MB_OK);
        goto End;
    }
    
    convertCtx = sws_getContext(pDesktopRecorder->m_nWidth, pDesktopRecorder->m_nHeight, PIX_FMT_BGR24, pDesktopRecorder->m_nWidth, pDesktopRecorder->m_nHeight, PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);

    f264=fopen("DesktopRecord.h264","wb");
    if(!f264){
        MessageBox(NULL, _T("Can not create DesktopRecord.h264"), _T("Error"), MB_OK);
        goto End;
    }
    x264_nal_t *nal_t;
    int i_nal;
    int ypos=0;
    int upos=pDesktopRecorder->m_nWidth*pDesktopRecorder->m_nHeight;
    int vpos=pDesktopRecorder->m_nWidth*pDesktopRecorder->m_nHeight/4*5;
    int ysize=pDesktopRecorder->m_nWidth*pDesktopRecorder->m_nHeight;
    int usize=pDesktopRecorder->m_nWidth*pDesktopRecorder->m_nHeight/4;
    int vsize=pDesktopRecorder->m_nWidth*pDesktopRecorder->m_nHeight/4;
    int tempsize=0;
    int i=0;
    tempsize=x264_encoder_headers(h264Encoder,&nal_t,&i_nal);
    if(tempsize>0)
    {
        for(i=0;i<i_nal;i++)
        {
            if (nal_t[i].i_type==NAL_SPS||nal_t[i].i_type==NAL_PPS)//获取SPS数据，PPS数据
            {	
                fwrite(nal_t[i].p_payload,1,nal_t[i].i_payload,f264);			
            }
        }
    }

    int nInterval = 1000/pDesktopRecorder->m_nFrameRate;
    hWaitEvent = CreateEvent(NULL, TRUE, FALSE, _T("DesktopDesktopWaitEvent"));
    while(pDesktopRecorder->m_bRun){
        LARGE_INTEGER t0, t1, f;
        TCHAR temp[100];
        QueryPerformanceFrequency(&f);
        QueryPerformanceCounter(&t0);
        BitBlt(hDesktopCompatibleDC,0,0,pDesktopRecorder->m_nWidth,pDesktopRecorder->m_nHeight,hDesktopDC,0,0,SRCCOPY); //SRCCOPY|CAPTUREBLT

        int srcstride = pDesktopRecorder->m_nWidth*3;       //RGB stride is just 3*width
        if(sws_scale(convertCtx, (const uint8_t* const *)&pBits, &srcstride, 0, pDesktopRecorder->m_nHeight, picin.img.plane, picin.img.i_stride)<0){
            OutputDebugString(_T("Error sws_scale please check!"));
            break;
        }
        if(0)
        {
            FILE *pF = fopen("test.yuv", "ab+");
            if(pF){
                fwrite(picin.img.plane[0],pDesktopRecorder->m_nWidth*pDesktopRecorder->m_nHeight,1,pF);
                fwrite(picin.img.plane[1],pDesktopRecorder->m_nWidth*pDesktopRecorder->m_nHeight/4,1,pF);
                fwrite(picin.img.plane[2],pDesktopRecorder->m_nWidth*pDesktopRecorder->m_nHeight/4,1,pF);
                fclose(pF);
            }
        }

        int frame_size=x264_encoder_encode(h264Encoder,&nal_t,&i_nal,&picin,&picout);
        if(frame_size>0)
        {
            for(i=0;i<i_nal;i++)
            {
                if (nal_t[i].i_type==NAL_SLICE||nal_t[i].i_type==NAL_SLICE_IDR)//获取帧数据
                {	
                    fwrite(nal_t[i].p_payload,1,nal_t[i].i_payload,f264);
                }
            }
        }
        QueryPerformanceCounter(&t1);
        _stprintf(temp, _T("time used: %fs\n"), (double)(t1.QuadPart - t0.QuadPart) / f.QuadPart);
        TRACE(temp);
        int nWaitTime = nInterval - (t1.QuadPart - t0.QuadPart)*1000 / f.QuadPart;
        if(nWaitTime > 0){
            TRACE(_T("Will wait %dms"), nWaitTime);
            WaitForSingleObject(hWaitEvent, nWaitTime);
        }
    }

End:
    if(hWaitEvent)
        CloseHandle(hWaitEvent);
    x264_picture_clean(&picin);
    if(h264Encoder)
        x264_encoder_close(h264Encoder);
    if(f264)
        fclose(f264);
    if(convertCtx)
        sws_freeContext(convertCtx);
    if(hDesktopCompatibleDC)
        DeleteDC(hDesktopCompatibleDC);
    if(hDesktopCompatibleBitmap)
        DeleteObject(hDesktopCompatibleBitmap);
    ReleaseDC(hDesktopWnd,hDesktopDC);
    hDesktopCompatibleDC = NULL;
    hDesktopCompatibleBitmap = NULL;
    return 1;
}
#else
DWORD WINAPI DesktopRecdProc(LPVOID pParam)
{
    x264_t*h;//对象句柄，
    x264_picture_t m_picin;//传入图像YUV
    x264_picture_t m_picout;//输出图像RAW
    x264_param_t param;//参数设置
    //int m_width=352;//宽，根据实际情况改
    //int m_height=288;//高	
    //FILE*fyuv=fopen("foreman_cif.yuv","rb");//文件下载地址：http://trace.eas.asu.edu/yuv/
    DesktopRecorder* pDesktopRecorder = (DesktopRecorder *)pParam;
    int m_width=pDesktopRecorder->m_nWidth;
    int m_height=pDesktopRecorder->m_nHeight;	
    FILE* fyuv=fopen("test.yuv","rb");
    FILE*f264=fopen("flower_352x288.h264","wb");
    if (fyuv==NULL||f264==NULL)
    {
        printf("file err\n");
        return 0;
    }
    x264_param_default(&param);//设置默认参数具体见common/common.c
    int yuvsize=m_height*m_width*3/2;
    char*pyuvbuff=(char*)malloc(yuvsize);
    char*ph264buff=(char*)malloc(yuvsize);
    param.i_width=m_width;
    param.i_height=m_height;
    param.i_keyint_min=5;//关键帧最小间隔
    param.i_keyint_max=250;//关键帧最大间隔
    param.b_repeat_headers=0;//关键帧前面是否放sps跟pps帧，0 否 1，放	
    //	param.analyse.i_subpel_refine=7;//压缩级别1~7
    param.i_log_level=X264_LOG_NONE;//设置信息级别
    h=x264_encoder_open(&param);//根据参数初始化X264级别
    x264_picture_init(&m_picin);//初始化图片信息
    x264_picture_alloc(&m_picin,X264_CSP_I420,m_width,m_height);//图片按I420格式分配空间，最后要
    x264_nal_t*nal_t;
    int i_nal;
    int ypos=0;
    int upos=m_height*m_width;
    int vpos=m_height*m_width/4*5;
    int ysize=m_height*m_width;
    int usize=m_height*m_width/4;
    int vsize=m_height*m_width/4;
    int tempsize=0;
    int i=0;
    tempsize=x264_encoder_headers(h,&nal_t,&i_nal);
    if(tempsize>0)
    {
        for(i=0;i<i_nal;i++)
        {	
            if (nal_t[i].i_type==NAL_SPS||nal_t[i].i_type==NAL_PPS)//获取SPS数据，PPS数据
            {	
                fwrite(nal_t[i].p_payload,1,nal_t[i].i_payload,f264);			
            }
        }
    }
    while(fread(pyuvbuff,1,yuvsize,fyuv)==yuvsize)
    {		
        memcpy(m_picin.img.plane[0],pyuvbuff+ypos,ysize);
        memcpy(m_picin.img.plane[1],pyuvbuff+upos,usize);
        memcpy(m_picin.img.plane[2],pyuvbuff+vpos,vsize);
        tempsize=0;
        tempsize=x264_encoder_encode(h,&nal_t,&i_nal,&m_picin,&m_picout);
        if(tempsize>0)
        {
            for(i=0;i<i_nal;i++)
            {
                if (nal_t[i].i_type==NAL_SLICE||nal_t[i].i_type==NAL_SLICE_IDR)//获取帧数据
                {	
                    fwrite(nal_t[i].p_payload,1,nal_t[i].i_payload,f264);
                }
            }
        }
    }

    x264_picture_clean(&m_picin);	
    x264_encoder_close(h);
    free(ph264buff);
    free(pyuvbuff);
    fclose(fyuv);
    fclose(f264);
    MessageBox(NULL, _T("shit"), _T("shit"), MB_OK);
    return 1;
}
#endif

BOOL DesktopRecorder::StartRec()
{
    if(m_bRun)
        return FALSE;
    m_bRun = TRUE;
    HANDLE handle = CreateThread(NULL, 0, DesktopRecdProc, this, 0, NULL);
    if(handle)
    {
        CloseHandle(handle);
    }else{
        m_bRun = FALSE;
    }

    return m_bRun;
}

BOOL DesktopRecorder::StopRec()
{
    m_bRun = FALSE;
    
    return TRUE;
}
