#pragma once
#include "opencv2\opencv.hpp"

class CDetect
{
public:
    CDetect(void);
    ~CDetect(void);

    BOOL detectSkin(IplImage *pSrc);
    BOOL detectPedestrian(IplImage *pSrc);
};

