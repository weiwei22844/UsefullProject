#pragma once
#include "opencv2\opencv.hpp"

class Filter
{
public:
    Filter(void);
    ~Filter(void);

    BOOL Ice(IplImage *pSrc);
    BOOL Old(IplImage *pSrc);
    BOOL Comic(IplImage *pSrc);
    BOOL Spread(IplImage *pSrc);
    BOOL Sin(IplImage *pSrc);
    BOOL Oil(IplImage *pSrc);
};

