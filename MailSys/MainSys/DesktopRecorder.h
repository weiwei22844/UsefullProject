#pragma once
#include "stdint.h"
extern "C"
{
#include "x264_config.h"
#include "x264.h"
}

class DesktopRecorder
{
public:
    DesktopRecorder(void);
    virtual ~DesktopRecorder(void);

    BOOL init(int nWidth, int nHeight, int nFrameRate);
    BOOL StartRec();
    BOOL StopRec();

    int m_nWidth;
    int m_nHeight;
    int m_nFrameRate;
    x264_param_t m_param;

    BOOL m_bRun;
};
