
// Calibration.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号
#define CAPTURE_NUM 1000
#define MODULENAMEMAXLEN 64


// Enumeration used for distinguishing different events.
enum MyEvents
{
	eMyExposureEndEvent,      // Triggered by a camera event.
	eMyFrameStartOvertrigger, // Triggered by a camera event.
	eMyImageReceivedEvent,    // Triggered by the receipt of an image.
	eMyMoveEvent,             // Triggered when the imaged item or the sensor head can be moved.
	eMyNoEvent                // Used as default setting.
};

typedef struct _BITMAPINFO_8bpp
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[256];
} BITMAPINFO_8bpp;

typedef enum TestModule{
    MODULE_INITSETTING = 0,
    MODULE_PM,
    MODULE_DRAG,
    MODULE_MOVE,
    MODULE_TRIGGER,
    MODULE_ROTATE,
    MODULE_DELAY,
}TestModule;

typedef struct ModuleInitSet{
    char cScriptFile[MAX_PATH];
}ModuleInitSet;

typedef struct ModulePM{
    char cPMTemplate[MAX_PATH];
}ModulePM;

typedef struct ModuleFingerMove{
    int nFingerType;                // 0 慢击手指 1 快击手指
    int nMode;                      // 0 根据识别匹配结果移动（忽略X、Y参数） 1 指定目标位置（利用X、Y参数）
    int nMoveToX;
    int nMoveToY;
	int nClick;						// 移动到指定位置后手指是否点击
	int nFrequency;
	int nTimes;
}ModuleFingerMove;

typedef struct ModuleIconDrag{
    int nMode;                      // 0 根据识别匹配结果确定图标初始位置 2 指定初始位置
    int nStartX;
    int nStartY;
    int nEndX;
    int nEndY;
	int nTimeBefore;
	int nTimeAfter;
}ModuleIconDrag;

typedef struct ModuleTriggerMode{
    int nTriggerIndex;              // 触发用例编号
	int nTriggerMode;				// 0 离屏触发 1 触屏触发
	int nTimeDelay;
	int nWaveFrq;
	int nWaveCnt;
	int nDBClkFrq;
	int nDBClkCnt;
}ModuleTriggerMode;

typedef struct ModuleDelay{
    int nDelayTime;                // 延时时间单位毫秒           
}ModuleDelay;

typedef struct ModuleRotate{
	int nAntiWise;					// 是否逆时针旋转
    int nRotateAngle;				// 0~1800
}ModuleRotate;

typedef struct ModuleData{
    int nType;
    char cName[MODULENAMEMAXLEN];
    int nSize;
    void *pData;
	ModuleData *pNext;
}ModuleData;

// CCalibrationApp:
// 有关此类的实现，请参阅 Calibration.cpp
//

class CCalibrationApp : public CWinApp
{
public:
	CCalibrationApp();

    BOOL DeleteDirectory(char* psDirName);

	char m_szConfigFile[MAX_PATH];
	char m_imageDirectory[MAX_PATH];
	char m_szUserCaseFile[MAX_PATH];

	int m_nUserCaseCount;
// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CCalibrationApp theApp;