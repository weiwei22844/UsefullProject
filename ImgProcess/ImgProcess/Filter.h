#pragma once
#include "opencv2\opencv.hpp"

class CFilter
{
public:
    CFilter(void);
    ~CFilter(void);

    BOOL Ice(IplImage *pSrc);
    BOOL Old(IplImage *pSrc);
    BOOL Comic(IplImage *pSrc);
    BOOL Spread(IplImage *pSrc);
    BOOL Sin(IplImage *pSrc);
    BOOL Oil(IplImage *pSrc);
    BOOL Cartoon(IplImage *pSrc);
};

