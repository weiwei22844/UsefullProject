#include "StdAfx.h"
#include "Detect.h"

CDetect::CDetect(void)
{
}


CDetect::~CDetect(void)
{
}

/*
* »ùÓÚ²©ÎÄ http://blog.csdn.net/jia20003/article/details/7459534
* (R, G, B) is classified as skin if
* R > 95 and G > 40 and B > 20and max{R, G, B} ¨C min{R, G, B} > 15 and |R-G| > 15
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