#include "StdAfx.h"
#include "Filter.h"
static char g_dbgBuf[500];

using namespace cv;

CFilter::CFilter(void)
{
}


CFilter::~CFilter(void)
{
}

BOOL CFilter::Ice(IplImage* pSrc)
{
    if(!pSrc)
        return FALSE;
    if(pSrc->nChannels != 3 && pSrc->nChannels != 4)
        return FALSE;
    unsigned char *pData = (unsigned char*)pSrc->imageData;
    int width = pSrc->width;
    int height = pSrc->height;
    int widthStep = pSrc->widthStep;
    int pixelsR, pixelsG, pixelsB, pixelsA;

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            unsigned char *pTemp = pData+i*widthStep+j*pSrc->nChannels;
            pixelsB = pTemp[0];
            pixelsG = pTemp[1];
            pixelsR = pTemp[2];
            //pixelsA = pTemp[3];

            //R
            int pixel = pixelsR - pixelsG - pixelsB;
            pixel = pixel * 3 / 2;
            if (pixel < 0) {
                pixel = -pixel;
            }
            if (pixel > 255){
                pixel = 255;
            }
            pixelsR = pixel; // 计算后重置R值，以下类同
            //G
            pixel = pixelsG - pixelsR - pixelsB;
            pixel = pixel * 3 / 2;
            if (pixel < 0) {
                pixel = -pixel;
            }
            if (pixel > 255){
                pixel = 255;
            }
            pixelsG = pixel;
            //B
            pixel = pixelsB - pixelsR - pixelsG;
            pixel = pixel * 3 / 2;
            if (pixel < 0) {
                pixel = -pixel;
            }
            if (pixel > 255){
                pixel = 255;
            }
            pixelsB = pixel;

            pTemp[0]=pixelsB;
            pTemp[1]=pixelsG;
            pTemp[2]=pixelsR;
        }
    }

    return TRUE;
}

BOOL CFilter::Old(IplImage *pSrc)
{
    if(!pSrc)
        return FALSE;
    if(pSrc->nChannels != 3 && pSrc->nChannels != 4)
        return FALSE;
    unsigned char *pData = (unsigned char*)pSrc->imageData;
    int width = pSrc->width;
    int height = pSrc->height;
    int widthStep = pSrc->widthStep;
    int pixelsR, pixelsG, pixelsB, pixelsA;
    int pixelsRN, pixelsGN, pixelsBN;

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            unsigned char *pTemp = pData+i*widthStep+j*pSrc->nChannels;
            pixelsB = pTemp[0];
            pixelsG = pTemp[1];
            pixelsR = pTemp[2];
            
            pixelsRN = (int) (0.393 * pixelsR + 0.769 * pixelsG + 0.189 * pixelsB);  
            pixelsGN = (int) (0.349 * pixelsR + 0.686 * pixelsG + 0.168 * pixelsB);  
            pixelsBN = (int) (0.272 * pixelsR + 0.534 * pixelsG + 0.131 * pixelsB); 
            if(pixelsRN<0)
                pixelsRN=0;
            if(pixelsRN>255)
                pixelsRN=255;
            if(pixelsGN<0)
                pixelsGN=0;
            if(pixelsGN>255)
                pixelsGN=255;
            if(pixelsBN<0)
                pixelsBN=0;
            if(pixelsBN>255)
                pixelsBN=255;
            pTemp[0]=pixelsBN;
            pTemp[1]=pixelsGN;
            pTemp[2]=pixelsRN;
        }
    }

    return TRUE;
}

BOOL CFilter::Comic(IplImage *pSrc)
{
    if(!pSrc)
        return FALSE;
    if(pSrc->nChannels != 3 && pSrc->nChannels != 4)
        return FALSE;
    unsigned char *pData = (unsigned char*)pSrc->imageData;
    int width = pSrc->width;
    int height = pSrc->height;
    int widthStep = pSrc->widthStep;
    int pixelsR, pixelsG, pixelsB, pixelsA;
    int pixelsRN, pixelsGN, pixelsBN;

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            unsigned char *pTemp = pData+i*widthStep+j*pSrc->nChannels;
            pixelsB = pTemp[0];
            pixelsG = pTemp[1];
            pixelsR = pTemp[2];

            pixelsBN = abs(pixelsB-pixelsG+pixelsB+pixelsR)*pixelsG/256; 
            pixelsGN = abs(pixelsB-pixelsG+pixelsB+pixelsR)*pixelsR/256;
            pixelsRN = abs(pixelsG-pixelsB+pixelsG+pixelsR)*pixelsR/256;
            
            if(pixelsRN<0)
                pixelsRN=0;
            if(pixelsRN>255)
                pixelsRN=255;
            if(pixelsGN<0)
                pixelsGN=0;
            if(pixelsGN>255)
                pixelsGN=255;
            if(pixelsBN<0)
                pixelsBN=0;
            if(pixelsBN>255)
                pixelsBN=255;
            //sprintf(g_dbgBuf, "%d %d %d", pixelsBN, pixelsGN, pixelsRN);
            //OutputDebugString(g_dbgBuf);

            pTemp[0]=pixelsBN;
            pTemp[1]=pixelsGN;
            pTemp[2]=pixelsRN;
        }
    }

    IplImage *pGrayImage = cvCreateImage(cvGetSize(pSrc), IPL_DEPTH_8U, 1);
    if(pGrayImage){
        cvCvtColor(pSrc, pGrayImage, CV_BGR2GRAY);
        cvNormalize(pGrayImage,pGrayImage,255,0,CV_MINMAX);
        
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                unsigned char *pDstData = pData+i*widthStep+j*pSrc->nChannels;
                unsigned char *pSrcData = (unsigned char*)pGrayImage->imageData+i*pGrayImage->widthStep+j;
                pDstData[0]=*pSrcData;
                pDstData[1]=*pSrcData;
                pDstData[2]=*pSrcData;
            }
        }
        cvReleaseImage(&pGrayImage);
        return TRUE;
    }else{
        return FALSE;
    }
}

BOOL CFilter::Spread(IplImage *pSrc)
{
    if(!pSrc)
        return FALSE;
    if(pSrc->nChannels != 3 && pSrc->nChannels != 4)
        return FALSE;
    unsigned char *pData = (unsigned char*)pSrc->imageData;
    int width = pSrc->width;
    int height = pSrc->height;
    int widthStep = pSrc->widthStep;
    Mat img(cvGetSize(pSrc),CV_8UC3);

    int nR = 5;
    int nGap = nR/2;
    RNG rng;
    for (int i=nGap; i<height-nGap; i++)
    { 
        uchar* P1 = img.ptr<uchar>(i);
        for (int j=nGap; j<width-nGap; j++)
        {
            int tmp=rng.uniform(0,nR*nR);
            int x = j-nGap+tmp%nR;
            int y = i-nGap+tmp/nR;
            P1[3*j]=*(pData+y*widthStep+pSrc->nChannels*x);
            P1[3*j+1]=*(pData+y*widthStep+pSrc->nChannels*x+1);
            P1[3*j+2]=*(pData+y*widthStep+pSrc->nChannels*x+2);
        }
    }

    for (int i=nGap; i<height-nGap; i++)
    { 
        uchar* P1 = img.ptr<uchar>(i);
        for (int j=nGap; j<width-nGap; j++)
        {
            unsigned char *pDstData = pData+i*widthStep+j*pSrc->nChannels;
            pDstData[0]=P1[3*j];
            pDstData[1]=P1[3*j+1];
            pDstData[2]=P1[3*j+2];
        }
    }

    return TRUE;
}

BOOL CFilter::Sin(IplImage *pSrc)
{
    if(!pSrc)
        return FALSE;
    if(pSrc->nChannels != 3 && pSrc->nChannels != 4)
        return FALSE;
    unsigned char *pData = (unsigned char*)pSrc->imageData;
    int width = pSrc->width;
    int height = pSrc->height;
    int widthStep = pSrc->widthStep;
    Mat img(cvGetSize(pSrc),CV_8UC3);

    int deltaI=10;	//波浪周期;
    int A=10;		//波浪振幅;
    double angle = 0.0;

    for (int i=0; i<height; i++)
    {
        int changeX = A*sin(angle);
        uchar *srcP = pData+i*widthStep;
        uchar *imgP = img.ptr<uchar>(i);
        for (int x=0; x<width; x++)
        {
            if(changeX+x<width && changeX+x>0)		//正弦分布（-1,1）
            {
                imgP[3*x]=srcP[pSrc->nChannels*(x+changeX)];
                imgP[3*x+1]=srcP[pSrc->nChannels*(x+changeX)+1];
                imgP[3*x+2]=srcP[pSrc->nChannels*(x+changeX)+2];
            }
            //每行开始和结束的空白区;
            else if(x+changeX<=0)       
            {
                imgP[3*x]=srcP[0];
                imgP[3*x+1]=srcP[1];
                imgP[3*x+2]=srcP[2];
            }
            else if (x+changeX>=width)
            {
                imgP[3*x]=srcP[pSrc->nChannels*(width-1)];
                imgP[3*x+1]=srcP[pSrc->nChannels*(width-1)+1];
                imgP[3*x+2]=srcP[pSrc->nChannels*(width-1)+2];
            }
        }
        angle += ((double)deltaI)/100;
    }
    
    for (int i=0; i<height; i++)
    { 
        uchar* P1 = img.ptr<uchar>(i);
        for (int j=0; j<width; j++)
        {
            unsigned char *pDstData = pData+i*widthStep+j*pSrc->nChannels;
            pDstData[0]=P1[3*j];
            pDstData[1]=P1[3*j+1];
            pDstData[2]=P1[3*j+2];
        }
    }

    return TRUE;
}

/*
* http://blog.csdn.net/jia20003/article/details/18850079
* 基于像素权重实现图像的像素模糊从而达到近似油画效果模糊
* 其实卷积的变种，不是基于Stroke维诺图算法与采样moment算法
*/
BOOL CFilter::Oil(IplImage *pSrc)
{
    if(!pSrc)
        return FALSE;
    if(pSrc->nChannels != 3 && pSrc->nChannels != 4)
        return FALSE;

    unsigned char *pData = (unsigned char*)pSrc->imageData;
    int width = pSrc->width;
    int height = pSrc->height;
    int widthStep = pSrc->widthStep;
    Mat img(cvGetSize(pSrc),CV_8UC3);

    int radius = 5;         // default value
    int intensity = 20;     // default value
    int subradius = radius / 2;
    int* intensityCount = new int[intensity+1];
    int* ravg = new int[intensity+1];
    int* gavg = new int[intensity+1];
    int* bavg = new int[intensity+1];
    for(int i=0; i<=intensity; i++)
    {
        intensityCount[i] = 0;
        ravg[i] = 0;
        gavg[i] = 0;
        bavg[i] = 0;
    }
    for(int row=0; row<height; row++) {
        uchar* P1 = img.ptr<uchar>(row);
        int ta = 0, tr = 0, tg = 0, tb = 0;
        for(int col=0; col<width; col++) {
            
            for(int subRow = -subradius; subRow <= subradius; subRow++)
            {
                for(int subCol = -subradius; subCol <= subradius; subCol++)
                {
                    int nrow = row + subRow;
                    int ncol = col + subCol;
                    if(nrow >=height || nrow < 0)
                    {
                        nrow = 0;
                    }
                    if(ncol >= width || ncol < 0)
                    {
                        ncol = 0;
                    }
                    unsigned char *pSrcData = pData+nrow*widthStep+ncol*pSrc->nChannels;
                    tr = pSrcData[2];
                    tg = pSrcData[1];
                    tb = pSrcData[0];
                    int curIntensity = (int)(((double)((tr+tg+tb)/3)*intensity)/255.0f);
                    intensityCount[curIntensity]++;
                    ravg[curIntensity] += tr;
                    gavg[curIntensity] += tg;
                    bavg[curIntensity] += tb;
                }
            }

            // find the max number of same gray level pixel
            int maxCount = 0, maxIndex = 0;
            for(int m=0; m<intensity+1; m++)
            {
                if(intensityCount[m] > maxCount)
                {
                    maxCount = intensityCount[m];
                    maxIndex = m;
                }
            }

            // get average value of the pixel
            int nr = ravg[maxIndex] / maxCount;
            int ng = gavg[maxIndex] / maxCount;
            int nb = bavg[maxIndex] / maxCount;

            P1[3*col]=nb;
            P1[3*col+1]=ng;
            P1[3*col+2]=nr;

            // post clear values for next pixel
            for(int i=0; i<=intensity; i++)
            {
                intensityCount[i] = 0;
                ravg[i] = 0;
                gavg[i] = 0;
                bavg[i] = 0;
            }

        }
    }

    for (int i=0; i<height; i++)
    { 
        uchar* P1 = img.ptr<uchar>(i);
        for (int j=0; j<width; j++)
        {
            unsigned char *pDstData = pData+i*widthStep+j*pSrc->nChannels;
            pDstData[0]=P1[3*j];
            pDstData[1]=P1[3*j+1];
            pDstData[2]=P1[3*j+2];
        }
    }

    delete[] intensityCount;
    delete[] ravg;
    delete[] gavg;
    delete[] bavg;

    return TRUE;
}


/*
* http://blog.csdn.net/yangtrees/article/details/7544481
* 
* 
*/
BOOL CFilter::Cartoon(IplImage *pSrc)
{
    if(!pSrc)
        return FALSE;
    if(pSrc->nChannels != 3 && pSrc->nChannels != 4)
        return FALSE;

	Mat src1(pSrc);
	Mat img;
	//双边滤波，第3个参数d可以说d>5时不能实时处理，最后两个参数是sigma参数，一般相同，
	//<10时基本没效果, >150时漫画效果
	bilateralFilter(src1,img,5,150,150);
	bilateralFilter(img,src1,5,150,150);
	//img.copyTo(src1);
		
	Mat src;
	cvtColor(src1,src,CV_BGR2GRAY);
	//粗线，越大越粗，但是会有大量噪点
	Mat imgL;
	//第三个参数ddepth表示目标图像的深度，ddepth=-1时，与原图像一致
	Laplacian(src,imgL,-1,3,1);
	//imshow("Laplacian",imgL);
	//waitKey(0);

	//细线
	Mat imgC;
	Canny(src,imgC,30,90);
	//imshow("Canny",imgC);
	//waitKey(0);

	Mat imgS,imgSx,imgSy,imgS0;
	Sobel(src,imgSx,-1,0,1);
	Sobel(src,imgSx,-1,1,0);
	imgS=imgSx+imgSy;
	Sobel(src,imgS0,-1,1,1);
	//imshow("Sobel0",imgS0);
	//imshow("Sobel",imgS);
	//waitKey(0);
	
	Mat imgTotal;
	imgTotal=imgC+imgS+imgL;
	//imgTotal.convertTo(imgTotal,CV_32FC1);
	normalize(imgTotal,imgTotal,255,0,CV_MINMAX);
	GaussianBlur(imgTotal,imgTotal,Size(3,3),3);
	threshold(imgTotal,imgTotal,100,255,THRESH_BINARY_INV);
	//imshow("Total",imgTotal);
	//waitKey(0);

	Mat imgTotalC3;
	cvtColor(imgTotal,imgTotalC3,CV_GRAY2BGR);
	bitwise_and(src1,imgTotalC3,src1);
	//imshow("Result",src1);
	//waitKey(0);

    return TRUE;
}

