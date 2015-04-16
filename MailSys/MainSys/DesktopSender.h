#pragma once

class DesktopSender
{
public:
    DesktopSender(void);
    virtual ~DesktopSender(void);

    BOOL Init(BOOL bIsSend, BOOL bDeleteAfterSend, int nFrameRate);
    BOOL Start();
    BOOL Stop();
    BOOL IsRun(){return m_bRun;}

    BOOL m_bRun;
    DWORD m_dwImageNum;
    HANDLE m_hEvent;

private:
    BOOL m_bIsSend;
    BOOL m_bDeleteAfterSend;
    int m_nFrameRate;
};
