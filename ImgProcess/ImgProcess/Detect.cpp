#include "StdAfx.h"
#include "Detect.h"

using namespace cv;

CDetect::CDetect(void)
{
}


CDetect::~CDetect(void)
{
}

/*
* 基于博文 http://blog.csdn.net/jia20003/article/details/7459534
* (R, G, B) is classified as skin if
* R > 95 and G > 40 and B > 20and max{R, G, B} – min{R, G, B} > 15 and |R-G| > 15
* and R > G and R > B
*/
BOOL CDetect::detectSkin(IplImage *pSrc)
{
    if(!pSrc)
        return FALSE;
    if(pSrc->nChannels != 3 && pSrc->nChannels != 4)
        return FALSE;
    unsigned char *pData = (unsigned char*)pSrc->imageData;
    int width = pSrc->width;
    int height = pSrc->height;
    int widthStep = pSrc->widthStep;
    int pixelsR, pixelsG, pixelsB;

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            unsigned char *pTemp = pData+i*widthStep+j*pSrc->nChannels;
            pixelsB = pTemp[0];
            pixelsG = pTemp[1];
            pixelsR = pTemp[2];

            int max = MAX(pixelsR, MAX(pixelsG, pixelsB));
            int min = MIN(pixelsR, MIN(pixelsG, pixelsB));
            int rg = abs(pixelsR - pixelsG);
            if(pixelsR > 95 && pixelsG > 40 && pixelsB > 20 && pixelsG > 15 && 
                (max - min) > 15 && pixelsR > pixelsG && pixelsR > pixelsB) {
                    ;
            } else {
                pTemp[0]=0;
                pTemp[1]=0;
                pTemp[2]=0;
            }
        }
    }


    return TRUE;
}

/*
* http://www.cnblogs.com/tornadomeet/archive/2012/08/03/2621814.html http://blog.csdn.net/icvpr/article/details/8454439
* detectMultiScale(const Mat& img, CV_OUT vector<Rect>& foundLocations, double hitThreshold=0, Size winStride=Size(), 
*     Size padding=Size(), double scale=1.05, double finalThreshold=2.0, bool useMeanshiftGrouping = false)
* img: 输入图片 foundLocations: 检测到目标区域列表 hitThreshold: 程序内部计算为行人目标的阈值，也就是检测到的特征到SVM分类超平面的距离
* winStride: 滑动窗口每次移动的距离，必须是块移动的整数倍 padding: 图像扩充的大小 scale: 比例系数，滑动窗口每次增加的比例
* finalThreshold: 组阈值，即校正系数，当一个目标被多个窗口检测出来时，该参数此时就起了调节作用，为0时表示不起调节作用
*/
BOOL CDetect::detectPedestrian(IplImage *pSrc)
{
    if(!pSrc)
        return FALSE;

    Mat mtx(pSrc);
    // 1. 定义HOG对象
    HOGDescriptor hog; // 采用默认参数

    // 2. 设置SVM分类器
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());   // 采用已经训练好的行人检测分类器

    // 3. 在测试图像上检测行人区域
    std::vector<Rect> regions, found_filtered;
    hog.detectMultiScale(mtx, regions, 0, Size(8,8), Size(32,32), 1.05, 1);

#if 0
    // 显示
    for (size_t i = 0; i < regions.size(); i++)
    {
        if(i%3==0)
            rectangle(mtx, regions[i], Scalar(0,0,255), 2);
        else if(i%3==1)
            rectangle(mtx, regions[i], Scalar(0,255,0), 2);
        else
            rectangle(mtx, regions[i], Scalar(255,0,0), 2);
    }
#else
    size_t i, j;
    for (i = 0; i < regions.size(); i++ )
    {
        Rect r = regions[i];

        //下面的这个for语句是找出所有没有嵌套的矩形框r,并放入found_filtered中,如果有嵌套的
        //话,则取外面最大的那个矩形框放入found_filtered中
        for(j = 0; j <regions.size(); j++)
            if(j != i && (r&regions[j])==r)
                break;
        if(j == regions.size())
            found_filtered.push_back(r);
    }

    //在图片img上画出矩形框,因为hog检测出的矩形框比实际人体框要稍微大些,所以这里需要
    //做一些调整
    for(i = 0; i <found_filtered.size(); i++)
    {
        Rect r = found_filtered[i];
        r.x += cvRound(r.width*0.1);
        r.width = cvRound(r.width*0.8);
        r.y += cvRound(r.height*0.07);
        r.height = cvRound(r.height*0.8);
        rectangle(mtx, r.tl(), r.br(), Scalar(0, 255, 0), 3);
    }
#endif

    //imshow("hog", mtx);

    return TRUE;
}

