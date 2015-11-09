
// CalibrationDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Calibration.h"
#include "CalibrationDlg.h"
#include "afxdialogex.h"

#include "libControl.h"
#include "DlgTemplate.h"

#include "DlgRobotSet.h"
#include "DlgPMSet.h"
#include "DlgTriggerSet.h"
#include "DlgPreview.h"
#include "Redirect.h"

#include "DlgNewCase.h"
#include "DlgInitSetting.h"
#include "DlgModulePM.h"
#include "DlgFingerMove.h"
#include "DlgIconDrag.h"
#include "DlgTriggerMode.h"
#include "DlgRotate.h"
#include "DlgCircleTimes.h"
#include "DlgTimeDelay.h"
#include "DlgInsertModule.h"

#include <pylon/PylonIncludes.h>
#include <pylon/PylonGUI.h>
#include <pylon/PylonImage.h>
#include <pylon/Pixel.h>
#include <pylon/ImageFormatConverter.h>

// Include files used by samples.
#include "include/ConfigurationEventPrinter.h"
#include <iomanip>

#define PM_SHOWTIME  500               // 用于显示匹配成功后的结果

static char g_dbgStr[500];
CString g_strPMSuccess;
using namespace Pylon;

static CPylonImage g_Images[CAPTURE_NUM];
static uint64_t g_timeStamp[CAPTURE_NUM];
int g_nCapIndex = 0;

CCalibrationDlg *g_pMainDlg = NULL;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define  INIT_TIMER 100
#define LP_PI 3.1415926535897932384626433832795
#define SMALLZERO 1.0e-6
#define UPDATEINFO_MSG (WM_USER+100)

static char g_dbgBuf[500];
unsigned char * target_image_memdata = NULL;
unsigned int  target_image_width = 0;
unsigned int  target_image_height = 0;

unsigned char * template_image_memdata = NULL;
unsigned int  template_image_width = 0;
unsigned int  template_image_height = 0;

char g_strResult[1000];

using namespace FlyCapture2;

//*********************************************************************
/* Image rotation values. */
#define ROTATED_FIND_ROTATION_DELTA_ANGLE  10
#define ROTATED_FIND_ROTATION_ANGLE_STEP   1
#define ROTATED_FIND_RAD_PER_DEG           0.01745329251
/* Minimum accuracy for the search position. */
#define ROTATED_FIND_MIN_POSITION_ACCURACY     0.10
/* Minimum accuracy for the search angle. */
#define ROTATED_FIND_MIN_ANGLE_ACCURACY        0.25
/* Angle range to search. */
#define ROTATED_FIND_ANGLE_DELTA_POS           ROTATED_FIND_ROTATION_DELTA_ANGLE
#define ROTATED_FIND_ANGLE_DELTA_NEG           ROTATED_FIND_ROTATION_DELTA_ANGLE
//*********************************************************************

// Settings for using Basler USB cameras.
#include <pylon/usb/BaslerUsbInstantCamera.h>
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
typedef Pylon::CBaslerUsbCameraEventHandler CameraEventHandler_t;	// Or use Camera_t::CameraEventHandler_t
typedef Pylon::CBaslerUsbImageEventHandler ImageEventHandler_t;		// Or use Camera_t::ImageEventHandler_t
typedef Pylon::CBaslerUsbGrabResultPtr GrabResultPtr_t;				// Or use Camera_t::GrabResultPtr_t
using namespace Basler_UsbCameraParams;

// Namespace for using cout.
using namespace std;

// Names of possible events for a printed output.
const char* MyEventNames[] =
{
    "ExposureEndEvent     ",
    "FrameStartOvertrigger",
    "ImageReceived        ",
    "Move                 ",
    "NoEvent              "
};

// Used for logging received events without outputting the information on the screen
// because outputting will change the timing.
// This class is used for demonstration purposes only.
struct LogItem
{
    LogItem()
        : eventType( eMyNoEvent)
        , frameNumber(0)
    {
    }

    LogItem( MyEvents event, uint16_t frameNr)
        : eventType(event)
        , frameNumber(frameNr)
    {
        //Warning, measured values can be wrong on older PC hardware.
        QueryPerformanceCounter(&time);
    }


    LARGE_INTEGER time; // Recorded time stamps.
    MyEvents eventType; // Type of the received event.
    uint16_t frameNumber; // Frame number of the received event.
};


// Helper function for printing a log.
// This function is used for demonstration purposes only.
void PrintLog( const std::vector<LogItem>& aLog)
{
    // Get the PC timer frequency.
    LARGE_INTEGER timerFrequency;
    QueryPerformanceFrequency(&timerFrequency);

    /*sprintf(g_dbgStr, "\nWarning, the printed time values can be wrong on older PC hardware.\r\n");
    g_pMainDlg->ShowTestInfo(g_dbgStr);
    sprintf(g_dbgStr, "Time [ms]      Event                 FrameNumber\r\n");
    g_pMainDlg->ShowTestInfo(g_dbgStr);
    sprintf(g_dbgStr, "------------ --------------------- -----------\r\n");
    g_pMainDlg->ShowTestInfo(g_dbgStr);
    */

    // Print the logged information.
    size_t logSize = aLog.size();
    for ( size_t i = 0; i < logSize; ++i)
    {
        // Calculate the elapsed time between the events.
        double time_ms = 0;
        if ( i)
        {
            __int64 oldTicks = ((__int64)aLog[i-1].time.HighPart << 32) + (__int64)aLog[i-1].time.LowPart;
            __int64 newTicks = ((__int64)aLog[i].time.HighPart << 32) + (__int64)aLog[i].time.LowPart;
            long double timeDifference = (long double) (newTicks - oldTicks);
            long double ticksPerSecond = (long double) (((__int64)timerFrequency.HighPart << 32) + (__int64)timerFrequency.LowPart);
            time_ms = (timeDifference / ticksPerSecond) * 1000;
        }

        // Print the event information.
        //cout << setw(12) << fixed << setprecision(4) << time_ms <<" "<< MyEventNames[ aLog[i].eventType ] <<" "<< aLog[i].frameNumber << std::endl;
        sprintf(g_dbgStr, "%lf %s %d\r\n", time_ms, MyEventNames[ aLog[i].eventType ], aLog[i].frameNumber);
		OutputDebugString(g_dbgStr);
        //g_pMainDlg->ShowTestInfo(g_dbgStr);
    }
}


// Number of images to be grabbed.
static const uint32_t c_countOfImagesToGrab = CAPTURE_NUM;


// Example handler for GigE camera events.
// Additional handling is required for GigE camera events because the event network packets can be lost, doubled or delayed on the network.
class CEventHandler : public CameraEventHandler_t, public ImageEventHandler_t
{
public:
    CEventHandler()
        : m_nextExpectedFrameNumberImage(0)
        , m_nextExpectedFrameNumberExposureEnd(0)
        , m_nextFrameNumberForMove(0)
        , m_frameIDsInitialized(false)
    {
        // Reserve space to log camera, image and move events.
        m_log.reserve( c_countOfImagesToGrab * 3);
        m_lastTimeStamp = 0;
    }

    // This method is called when a camera event has been received.
    virtual void OnCameraEvent( Camera_t& camera, intptr_t userProvidedId, GenApi::INode* /* pNode */)
    {
        if ( userProvidedId == eMyExposureEndEvent)
        {
            // An Exposure End event has been received.
            uint16_t frameNumber = (uint16_t)camera.EventExposureEndFrameID.GetValue();
            m_log.push_back( LogItem( eMyExposureEndEvent, frameNumber));

            // If Exposure End event is not doubled.
            if ( GetIncrementedFrameNumber( frameNumber) != m_nextExpectedFrameNumberExposureEnd)
            {
                // Check whether the imaged item or the sensor head can be moved.
                if ( frameNumber == m_nextFrameNumberForMove)
                {
                    MoveImagedItemOrSensorHead();
                }

                // Check for missing Exposure End events.
                if ( frameNumber != m_nextExpectedFrameNumberExposureEnd)
                {
                    throw RUNTIME_EXCEPTION( "An Exposure End event has been lost. Expected frame number is %d but got frame number %d.", m_nextExpectedFrameNumberExposureEnd, frameNumber);
                }
                IncrementFrameNumber( m_nextExpectedFrameNumberExposureEnd);
            }
        }
        else if ( userProvidedId == eMyFrameStartOvertrigger)
        {
            // The camera has been overtriggered.
            m_log.push_back( LogItem( eMyFrameStartOvertrigger, 0));

            // Handle this error...
        }
        else
        {
            PYLON_ASSERT2(false, "The sample has been modified and a new event has been registered. Add handler code above.");
        }
    }

    // This method is called when an image has been grabbed.
    virtual void OnImageGrabbed( Camera_t& camera, const GrabResultPtr_t& ptrGrabResult)
    {   
        // An image has been received.
        uint16_t frameNumber = (uint16_t)ptrGrabResult->GetBlockID();
        m_log.push_back( LogItem( eMyImageReceivedEvent, frameNumber));

        // Check whether the imaged item or the sensor head can be moved.
        // This will be the case if the Exposure End has been lost or if the Exposure End is received later than the image.
        if ( frameNumber == m_nextFrameNumberForMove)
        {
            MoveImagedItemOrSensorHead();
        }

        // Check for missing images.
        if ( frameNumber != m_nextExpectedFrameNumberImage)
        {
            throw RUNTIME_EXCEPTION( "An image has been lost. Expected frame number is %d but got frame number %d.", m_nextExpectedFrameNumberImage, frameNumber);
        }
        IncrementFrameNumber( m_nextExpectedFrameNumberImage);

        // 默认PixelType为：PixelType_BayerRG8
        TRACE("GetTimeStamp: %I64u width: %d height: %d imageSize: %d PixelType: %d tick count diff: %I64u\n", 
            ptrGrabResult->GetTimeStamp(), ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight(), ptrGrabResult->GetImageSize(), ptrGrabResult->GetPixelType(), ptrGrabResult->GetTimeStamp()-m_lastTimeStamp);
        m_lastTimeStamp = ptrGrabResult->GetTimeStamp();
		g_timeStamp[g_nCapIndex] = m_lastTimeStamp;
        g_Images[g_nCapIndex++].CopyImage( ptrGrabResult->GetBuffer(), ptrGrabResult->GetImageSize(), ptrGrabResult->GetPixelType(), ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight(), ptrGrabResult->GetPaddingX());
    }

    void MoveImagedItemOrSensorHead()
    {
        // The imaged item or the sensor head can be moved now...
        // The camera may not be ready for a trigger at this point yet because the sensor is still being read out.
        // See the documentation of the CInstantCamera::WaitForFrameTriggerReady() method for more information.
        m_log.push_back( LogItem( eMyMoveEvent, m_nextFrameNumberForMove));
        IncrementFrameNumber( m_nextFrameNumberForMove);
    }

    void PrintLog()
    {
        ::PrintLog( m_log);
    }

private:
    void IncrementFrameNumber( uint16_t& frameNumber)
    {
        frameNumber = GetIncrementedFrameNumber( frameNumber);
    }

    uint16_t GetIncrementedFrameNumber( uint16_t frameNumber)
    {
        ++frameNumber;
        return frameNumber;
    }

    uint16_t m_nextExpectedFrameNumberImage;
    uint16_t m_nextExpectedFrameNumberExposureEnd;
    uint16_t m_nextFrameNumberForMove;

    bool m_frameIDsInitialized;

    std::vector<LogItem> m_log;
    uint64_t m_lastTimeStamp;
};

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

void SetGray(BITMAPINFO_8bpp & bmi)
{
	memset(bmi.bmiColors, 0, sizeof(bmi.bmiColors));
	for (int i=0; i < 256; i++) {
		bmi.bmiColors[ i ].rgbRed = i;
		bmi.bmiColors[ i ].rgbGreen = i;
		bmi.bmiColors[ i ].rgbBlue = i;
		bmi.bmiColors[ i ].rgbReserved = 0;
	}
}

// CCalibrationDlg 对话框
CCalibrationDlg::CCalibrationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCalibrationDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nCamWidth = 1280;
	m_nCamHeight = 1024;
	m_Context = NULL;
	m_bSdkInit = FALSE;
	m_lLoginID = 0;
	m_dScale = 0.5;
	m_Inited = -1;

    m_lPrePosX = 0;
    m_lPrePosY = 0;
	m_lPrePosZ = 0;
    m_lCurPosZ = 0;
    m_lMarkPointX = 0;
    m_lMarkPointY = 0;

    m_lCamLaserShiftX = 0;
    m_lCamLaserShiftY = 0;
    m_lFingerLaserShiftX = 0;
    m_lFingerLaserShiftY = 0;
	m_lQuickClickLaserShiftX = 0;
	m_lQuickClickLaserShiftY = 0;

	m_bMoveIcon = FALSE;

	m_bMatch = FALSE;
    m_hThreadMatch = INVALID_HANDLE_VALUE;
	m_bReloadTemplate = FALSE;
	m_pYData = NULL;
	m_bPMProcessing = FALSE;
	for(int i = 0; i < 4; i++){
		m_fMatchPosX[i] = 0;
		m_fMatchPosY[i] = 0;
	}
	m_localCamIp[0] = m_localCamIp[1] = m_localCamIp[2] = m_localCamIp[3] = 0;

	m_bPreview = FALSE;
	hThreadCaptureDisplay = NULL;
	ThreadCaptureDisplayID = 0;
	m_bMatch = FALSE;

	/*
	* 默认取方法5（归一化相关系数匹配法），初步验证该方法有最高的匹配度，且
	* 当max_val取值大于0.85时可认为匹配成功，当小于该值时可认为匹配失败
	*/
	m_nPMType = GetPrivateProfileInt(_T("COMMON"), _T("PMTYPE"), 5, theApp.m_szConfigFile);
	m_MatchEvent = CreateEvent(NULL, FALSE, FALSE, "MatchEvent");
	if(!m_MatchEvent){
		OutputDebugString("CreateEvent failed!");
	}
	m_bDoSearch = FALSE;
	m_bCalibration = FALSE;
	m_bLocateTemplate = FALSE;

	m_pCamera = NULL;
    m_heventThreadDone = CreateEvent( NULL, FALSE, FALSE, NULL );

	m_pRobotSetDlg = NULL;
	m_pPMSetDlg = NULL;
	m_downPoint.x = m_downPoint.y = -1;
	m_upPoint.x = m_upPoint.y = -1;

	m_bBaslerInit = FALSE;
	m_ulWidthMax = 0;
	m_ulHeightMax = 0;
	m_bStorageThread = FALSE;
	m_nTriggerFlag = -1;
	m_hEventCapReady = CreateEvent(NULL, FALSE, FALSE, NULL);

    m_CurUserCase = "";
    m_pInitSetDlg = NULL;
    m_pModulePMDlg = NULL;
    m_pFingerMoveDlg = NULL;
    m_pIconDragDlg = NULL;
    m_pTriggerSetDlg = NULL;
    m_pRotateDlg = NULL;
	m_pCircleTimesDlg = NULL;
    m_pTimeDelayDlg = NULL;

    m_chUserCaseDir[0] = 0;
	m_nCircleTimes = 1;
	m_bCaseRun = FALSE;
	m_bIsCaseEdit = FALSE;
}

void CCalibrationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIDEO_WND, m_VideoWnd);
	DDX_Control(pDX, IDC_BTN_UP, m_BtnUp);
	DDX_Control(pDX, IDC_BTN_RIGHT, m_BtnRight);
	DDX_Control(pDX, IDC_BTN_DOWN, m_BtnDown);
	DDX_Control(pDX, IDC_BTN_LEFT, m_BtnLeft);
	DDX_Control(pDX, IDC_TREE_INIT, m_TreeInit);
	DDX_Control(pDX, IDC_TREE_CIRCLE, m_TreeCircle);
	DDX_Control(pDX, IDC_RADIO_INIT, m_RadioInit);
	DDX_Control(pDX, IDCANCEL, m_RadioCircle);
	DDX_Control(pDX, IDC_RICHEDIT_OUTPUT, m_RichEditOutput);
	DDX_Control(pDX, IDC_COMBO_USERCASE, m_comboUsercase);
}

BEGIN_MESSAGE_MAP(CCalibrationDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CCalibrationDlg::OnBnClickedBtnConnect)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_PREVIEW, &CCalibrationDlg::OnBnClickedBtnPreview)
	ON_BN_CLICKED(IDC_BTN_STOPPREVIEW, &CCalibrationDlg::OnBnClickedBtnStoppreview)
	ON_BN_CLICKED(IDOK, &CCalibrationDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_MAKETEMP, &CCalibrationDlg::OnBnClickedBtnMaketemp)
	ON_BN_CLICKED(IDC_BTN_SEARCH, &CCalibrationDlg::OnBnClickedBtnSearch)
    ON_BN_CLICKED(IDC_BTN_TOPREPOS, &CCalibrationDlg::OnBnClickedBtnToprepos)
    ON_BN_CLICKED(IDC_BTN_QUERYPOS, &CCalibrationDlg::OnBnClickedBtnQuerypos)
	ON_BN_CLICKED(IDC_BTN_CALIBRATION, &CCalibrationDlg::OnBnClickedBtnCalibration)
	ON_BN_CLICKED(IDC_BTN_RESET, &CCalibrationDlg::OnBnClickedBtnReset)
	ON_BN_CLICKED(IDCANCEL, &CCalibrationDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_ROBOTSET, &CCalibrationDlg::OnBnClickedBtnRobotset)
	ON_BN_CLICKED(IDC_BTN_PMSET, &CCalibrationDlg::OnBnClickedBtnPmset)
	ON_WM_RBUTTONDOWN()
	ON_BN_CLICKED(IDC_BTN_TRIGGERSET, &CCalibrationDlg::OnBnClickedBtnTriggerset)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BTN_TEST, &CCalibrationDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_TRIGGER, &CCalibrationDlg::OnBnClickedBtnTrigger)
	ON_BN_CLICKED(IDC_BTN_MOVEICON, &CCalibrationDlg::OnBnClickedBtnMoveicon)
	ON_WM_SETCURSOR()
    ON_BN_CLICKED(IDC_BTN_NEWCASE, &CCalibrationDlg::OnBnClickedBtnNewcase)
    ON_BN_CLICKED(IDC_RADIO_INIT, &CCalibrationDlg::OnBnClickedRadioInit)
    ON_BN_CLICKED(IDC_RADIO_CIRCLE, &CCalibrationDlg::OnBnClickedRadioCircle)
    ON_NOTIFY(NM_RCLICK, IDC_TREE_INIT, &CCalibrationDlg::OnNMRClickTreeInit)
    ON_BN_CLICKED(IDC_BTN_INITSETTING, &CCalibrationDlg::OnBnClickedBtnInitsetting)
    ON_BN_CLICKED(IDC_BTN_DRAGICON, &CCalibrationDlg::OnBnClickedBtnDragicon)
    ON_BN_CLICKED(IDC_BTN_PM, &CCalibrationDlg::OnBnClickedBtnPm)
    ON_BN_CLICKED(IDC_BTN_FINGERMOVE, &CCalibrationDlg::OnBnClickedBtnFingermove)
    ON_BN_CLICKED(IDC_BTN_TRRIGERMODE, &CCalibrationDlg::OnBnClickedBtnTrrigermode)
    ON_BN_CLICKED(IDC_BTN_COMPLETE, &CCalibrationDlg::OnBnClickedBtnComplete)
	ON_COMMAND(ID__RENAME, &CCalibrationDlg::OnTreeCtrlRename)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_CIRCLE, &CCalibrationDlg::OnNMRClickTreeCircle)
    ON_COMMAND(ID__DELETE, &CCalibrationDlg::OnDeleteTreeItem)
    ON_COMMAND(ID__SETTING, &CCalibrationDlg::OnSetTreeItem)
    ON_NOTIFY(NM_DBLCLK, IDC_TREE_INIT, &CCalibrationDlg::OnNMDblclkTreeInit)
    ON_NOTIFY(NM_DBLCLK, IDC_TREE_CIRCLE, &CCalibrationDlg::OnNMDblclkTreeCircle)
    ON_BN_CLICKED(IDC_BTN_ROTATE, &CCalibrationDlg::OnBnClickedBtnRotate)
	ON_COMMAND(ID__CIRCLETIMES, &CCalibrationDlg::OnSetCircleTimes)
	ON_BN_CLICKED(IDC_BTN_CASEDETAIL, &CCalibrationDlg::OnBnClickedBtnCasedetail)
	ON_BN_CLICKED(IDC_BTN_DELCASE, &CCalibrationDlg::OnBnClickedBtnDelcase)
	ON_BN_CLICKED(IDC_BTN_RUNCASE, &CCalibrationDlg::OnBnClickedBtnRuncase)
	ON_BN_CLICKED(IDC_BTN_STOPCASE, &CCalibrationDlg::OnBnClickedBtnStopcase)
	ON_BN_CLICKED(IDC_BTN_CLICK, &CCalibrationDlg::OnBnClickedBtnClick)
	ON_BN_CLICKED(IDC_BNT_CANCLENEW, &CCalibrationDlg::OnBnClickedBntCanclenew)
	ON_BN_CLICKED(IDC_BTN_CASECOMPLETE, &CCalibrationDlg::OnBnClickedBtnCasecomplete)
    ON_BN_CLICKED(IDC_BTN_CANCLEEDIT, &CCalibrationDlg::OnBnClickedBtnCancleedit)
    ON_BN_CLICKED(IDC_BTN_ROTATETOPREPOS, &CCalibrationDlg::OnBnClickedBtnRotatetoprepos)
    ON_BN_CLICKED(IDC_BTN_TIMEDELAY, &CCalibrationDlg::OnBnClickedBtnTimedelay)
    ON_COMMAND(ID__INSERT, &CCalibrationDlg::OnInsertTreeItem)
END_MESSAGE_MAP()


// CCalibrationDlg 消息处理程序

BOOL CCalibrationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	CRect rect, rectNew;
	int nPosX;
	int nPosY;
	int nWd, nHt;
	CButton *pBtn = (CButton*)GetDlgItem(IDC_BTN_RESET);
	//pBtn->GetWindowRect(rect);
    m_TreeInit.GetWindowRect(rect);
	nPosX = rect.right;
	nPosY = 10;

	nWd = m_nCamWidth*m_dScale;
	nHt = m_nCamHeight*m_dScale;
	m_VideoWnd.SetWindowPos(NULL, nPosX, nPosY, nWd, nHt, SWP_NOZORDER);
	m_VideoWnd.GetWindowRect(&rect);
	rectNew.top=rectNew.left=0;
	rectNew.right=rect.left+nWd;
    m_TreeCircle.GetWindowRect(&rect);
    TRACE("%d %d %d %d\n", rect.left, rect.right, rect.top, rect.bottom);
    rectNew.bottom=rect.bottom-GetSystemMetrics(SM_CYCAPTION);
	AdjustWindowRectEx(&rectNew, GetWindowLong(m_hWnd, GWL_STYLE), ::GetMenu(m_hWnd) != NULL, GetWindowLong(m_hWnd, GWL_EXSTYLE));
	::MoveWindow(m_hWnd, 0, 0, rectNew.Width(), rectNew.Height(), TRUE);

	nPosY += nHt+10;
    m_RichEditOutput.MoveWindow(nPosX, nPosY, nWd, rect.bottom-GetSystemMetrics(SM_CYCAPTION)-GetSystemMetrics(SM_CYFRAME)-nPosY);

	SetTimer(INIT_TIMER, 0, NULL);
	//EnableItem(0, FALSE);
#ifndef _DEBUG
	EnableItem(1, FALSE);
#endif
    EnableItem(2, FALSE);
	
	m_lPrePosX = GetPrivateProfileInt("COMMON", "PREPOSX", 0, theApp.m_szConfigFile);
	m_lPrePosY = GetPrivateProfileInt("COMMON", "PREPOSY", 0, theApp.m_szConfigFile);
	m_lPrePosZ = GetPrivateProfileInt("COMMON", "PREPOSZ", 0, theApp.m_szConfigFile);
    m_lMarkPointX = GetPrivateProfileInt("COMMON", "MARKPOINTX", -1, theApp.m_szConfigFile);
    m_lMarkPointY = GetPrivateProfileInt("COMMON", "MARKPOINTY", -1, theApp.m_szConfigFile);

    m_lCamLaserShiftX = GetPrivateProfileInt("CALIBRATION", "CAMLASERSHIFTX", -1, theApp.m_szConfigFile);
    m_lCamLaserShiftY = GetPrivateProfileInt("CALIBRATION", "CAMLASERSHIFTY", -1, theApp.m_szConfigFile);
    m_lFingerLaserShiftX = GetPrivateProfileInt("CALIBRATION", "FINGERLASERSHIFTX", -1, theApp.m_szConfigFile);
    m_lFingerLaserShiftY = GetPrivateProfileInt("CALIBRATION", "FINGERLASERSHIFTY", -1, theApp.m_szConfigFile);
	m_lQuickClickLaserShiftX = GetPrivateProfileInt("CALIBRATION", "QUICKCLICKLASERSHIFTX", -1, theApp.m_szConfigFile);
	m_lQuickClickLaserShiftY = GetPrivateProfileInt("CALIBRATION", "QUICKCLICKLASERSHIFTY", -1, theApp.m_szConfigFile);

	SetDlgItemText(IDC_EDIT_MOVESTEP, "5");

	char chTemp[200];
	GetPrivateProfileString("CALIBRATION", "HORIZONTALRATIO", "-1", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_dHorizontalRatio = atof(chTemp);
	GetPrivateProfileString("CALIBRATION", "VERTICALRATIO", "-1", chTemp, sizeof(chTemp), theApp.m_szConfigFile);
	m_dVerticalRatio = atof(chTemp);

    m_nEnableROI = GetPrivateProfileInt("POINTGREY", "ENABLEROI", 0, theApp.m_szConfigFile);

	//m_BtnUp.SetBitmaps(IDB_BMP_UP, RGB(0, 255, 0));
	m_BtnUp.SetIcon(IDI_ICON_UP, (int)BTNST_AUTO_GRAY);
	m_BtnRight.SetIcon(IDI_ICON_RIGHT, (int)BTNST_AUTO_GRAY);
	m_BtnDown.SetIcon(IDI_ICON_DOWN, (int)BTNST_AUTO_GRAY);
	m_BtnLeft.SetIcon(IDI_ICON_LEFT, (int)BTNST_AUTO_GRAY);
	g_pMainDlg = this;

    CheckRadioButton(IDC_RADIO_INIT, IDC_RADIO_CIRCLE, IDC_RADIO_INIT);
    m_TreeInit.EnableWindow(FALSE);
    m_TreeCircle.EnableWindow(FALSE);

    m_TreeImages.Create (IDB_TREEIMAGE,16,1,RGB(255,255,255) );
    m_TreeInit.SetImageList ( &m_TreeImages,TVSIL_NORMAL );
    m_TreeCircle.SetImageList ( &m_TreeImages,TVSIL_NORMAL );
    /*HTREEITEM  hti1 = m_TreeInit.InsertItem ( _T("唐詩"),0,1 );
    HTREEITEM  hti2 = m_TreeInit.InsertItem ( _T("宋詞"),0,1 );
    HTREEITEM  hti3 = m_TreeInit.InsertItem ( _T("元曲"),0,1 );
    HTREEITEM  hti4 = m_TreeInit.InsertItem ( _T("李白"),0,1,hti1 );*/

    ShowOutput(m_ProgramInfo.GetBuffer(0));

    InitUserCase();

#ifdef USEMIL
	/* Allocate defaults. */
	MappAllocDefault(M_DEFAULT, &m_MilApplication, &m_MilSystem, &m_MilDisplay, M_NULL, M_NULL);
#endif

    OnBnClickedBtnPreview();
	GetDlgItem(IDC_BTN_CASECOMPLETE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_CANCLEEDIT)->EnableWindow(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCalibrationDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCalibrationDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCalibrationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCalibrationDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(nIDEvent == INIT_TIMER){
		KillTimer(INIT_TIMER);
		m_bSdkInit = CONTROL_Init();
		if(!m_bSdkInit){
			OutputDebugString("CONTROL_Init failed!\n");
		}

        // Connect the camera control dialog to the camera object
        if(m_pCamera)
            m_camCtlDlg.Connect( m_pCamera );
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CCalibrationDlg::EnableItem(int nType, BOOL bEnable)
{
	if(nType == 0){
		GetDlgItem(IDC_BTN_MAKETEMP)->EnableWindow(bEnable);
	}else if(nType == 1){									// 动作相关按钮
		GetDlgItem(IDC_BTN_RESET)->EnableWindow(bEnable);
		GetDlgItem(IDC_BTN_UP)->EnableWindow(bEnable);
		GetDlgItem(IDC_BTN_DOWN)->EnableWindow(bEnable);
		GetDlgItem(IDC_BTN_LEFT)->EnableWindow(bEnable);
		GetDlgItem(IDC_BTN_RIGHT)->EnableWindow(bEnable);
		GetDlgItem(IDC_BTN_QUERYPOS)->EnableWindow(bEnable);
		GetDlgItem(IDC_BTN_MOVEICON)->EnableWindow(bEnable);
		GetDlgItem(IDC_BTN_CLOCKWISE)->EnableWindow(bEnable);
		GetDlgItem(IDC_BTN_ANTICLOCKWISE)->EnableWindow(bEnable);
		GetDlgItem(IDC_BTN_TOPREPOS)->EnableWindow(bEnable);
        GetDlgItem(IDC_BTN_TRIGGER)->EnableWindow(bEnable);
	}else if(nType == 2){									// 新建测试用例相关按钮
        GetDlgItem(IDC_BTN_INITSETTING)->EnableWindow(bEnable);
        GetDlgItem(IDC_BTN_DRAGICON)->EnableWindow(bEnable);
        GetDlgItem(IDC_BTN_PM)->EnableWindow(bEnable);
        GetDlgItem(IDC_BTN_FINGERMOVE)->EnableWindow(bEnable);
        GetDlgItem(IDC_BTN_TRRIGERMODE)->EnableWindow(bEnable);
        GetDlgItem(IDC_BTN_COMPLETE)->EnableWindow(bEnable);
        GetDlgItem(IDC_BTN_ROTATE)->EnableWindow(bEnable);
        GetDlgItem(IDC_BTN_TIMEDELAY)->EnableWindow(bEnable);
		GetDlgItem(IDC_BNT_CANCLENEW)->EnableWindow(bEnable);
    }else if(nType == 3){									// 执行测试用例相关按钮
		GetDlgItem(IDC_BTN_RUNCASE)->EnableWindow(bEnable);
		GetDlgItem(IDC_BTN_STOPCASE)->EnableWindow(bEnable);
		GetDlgItem(IDC_BTN_DELCASE)->EnableWindow(bEnable);
		GetDlgItem(IDC_BTN_CASEDETAIL)->EnableWindow(bEnable);
	}
}

void CCalibrationDlg::OnBnClickedBtnConnect()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();

	char chIpAddr[30];
	GetPrivateProfileString("ROBOT", "IPADDR", "192.168.1.144", chIpAddr, sizeof(chIpAddr), theApp.m_szConfigFile);
	if(m_lLoginID > 0){
		CONTROL_Disconnect(m_lLoginID);
	}
	m_lLoginID =  CONTROL_Connect(chIpAddr, 34567, "admin", "123456");
	if (0 == m_lLoginID){
		int err = GetLastError();
		sprintf_s(g_dbgBuf, "connect device %s failed!\r\n", chIpAddr);
		MessageBox(g_dbgBuf);
        ShowOutput(g_dbgBuf);
		return;
	}else{
        ShowOutput("连接下位机成功\r\n");
		int nOpenLaser = GetPrivateProfileInt("ROBOT", "LASEROPEN", 1, theApp.m_szConfigFile);
		if(nOpenLaser){
			CONTROL_Extend(m_lLoginID, LASER_OPEN, 0);
		}else{
			CONTROL_Extend(m_lLoginID, LASER_CLOSE, 0);
		}
		EnableItem(1);
	}
}


void CCalibrationDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	CONTROL_Cleanup();
	OnBnClickedBtnStoppreview();
	PointGreyTerminate();

	CString str;
    if(!m_CurUserCase.IsEmpty() && !m_bIsCaseEdit){
		HTREEITEM hItem = m_TreeInit.GetRootItem();
		ModuleData *pModuleData = NULL;
		while(hItem){
			pModuleData = (ModuleData*)m_TreeInit.GetItemData(hItem);
			if(pModuleData){
				if(pModuleData->pData)
					free(pModuleData->pData);
				free(pModuleData);
			}
			hItem = m_TreeInit.GetNextSiblingItem(hItem);
		}
		hItem = m_TreeCircle.GetRootItem();
		pModuleData = NULL;
		while(hItem){
			pModuleData = (ModuleData*)m_TreeCircle.GetItemData(hItem);
			if(pModuleData){
				if(pModuleData->pData)
					free(pModuleData->pData);
				free(pModuleData);
			}
			hItem = m_TreeCircle.GetNextSiblingItem(hItem);
		}

        //RemoveDirectory(m_chCurrWorkDir);
		str = m_chUserCaseDir;
		str += m_CurUserCase;
        theApp.DeleteDirectory(str.GetBuffer(0));
    }
    CloseHandle( m_heventThreadDone );

	if (m_pCamera)
	{
		delete m_pCamera;
		m_pCamera = NULL;
	}

	CloseHandle(m_hEventCapReady);
	Sleep(200);
#ifdef USEMIL
	MappFreeDefault(m_MilApplication, m_MilSystem, m_MilDisplay, M_NULL, M_NULL);
#endif
}

int splitString(CString str, char split, CStringArray& strArray) 
{
	strArray.RemoveAll();
	CString strTemp = str; //此赋值不能少
	strTemp.TrimLeft();
	strTemp.TrimRight();
	int nIndex = 0;
	while(1)
	{
		nIndex = strTemp.Find(split); 
		if( nIndex >= 0 )
		{
			strArray.Add(strTemp.Left(nIndex)); 
			strTemp = strTemp.Right( strTemp.GetLength() - nIndex - 1 );
			strTemp.TrimLeft();
		}
		else
			break;
	} 
	strArray.Add(strTemp);
	return strArray.GetSize();
} 

BOOL CCalibrationDlg::PointGreyCameraInit()
{
    if(m_Inited >= 2)
        return TRUE;
#ifdef OLDMETHOD
	fc2Error error;
	unsigned int numCameras;

	// 开始捕获视频，先探测设备
	error = fc2CreateContext( &m_Context );
	if ( error != FC2_ERROR_OK ){
		sprintf(g_dbgBuf, "Error in fc2CreateContext: %s\n", fc2ErrorToDescription(error) );
		MessageBox(g_dbgBuf);
		return FALSE;
	}

	error = fc2GetNumOfCameras( m_Context, &numCameras );
	if ( error != FC2_ERROR_OK ){
		fc2DestroyContext(m_Context);
		m_Context = NULL;
		sprintf(g_dbgBuf, "Error in fc2GetNumOfCameras: %s\n", fc2ErrorToDescription(error) );
		MessageBox(g_dbgBuf);
		return FALSE;
	}
	if ( numCameras == 0 ){
		fc2DestroyContext(m_Context);
		m_Context = NULL;
		// No cameras detected
		MessageBox( "No PointGrey cameras detected.\n");
		return FALSE;
	}

	//然后打开设备从索引为0的摄像头开始
	int nIndex = 0;
	BOOL bConnect = FALSE;
	do{
		error = fc2GetCameraFromIndex( m_Context, nIndex++, &m_Guid );
		if ( error != FC2_ERROR_OK ){
			sprintf( g_dbgBuf, "Error in fc2GetCameraFromIndex: %s\n", fc2ErrorToDescription(error) );
			OutputDebugString(g_dbgBuf);
			continue;
		}

		error = fc2Connect( m_Context, &m_Guid );
		if ( error != FC2_ERROR_OK ){
			sprintf( g_dbgBuf, "Error in fc2Connect: %s\n", fc2ErrorToDescription(error) );
			OutputDebugString(g_dbgBuf);
			continue;
		}else{
			bConnect = TRUE;
			break;
		}
	}while(nIndex<numCameras);

	if(!bConnect){
		fc2DestroyContext(m_Context);
		m_Context = NULL;
		return FALSE;
	}

	fc2CameraInfo camInfo;
	error = fc2GetCameraInfo( m_Context, &camInfo );
	if ( error != FC2_ERROR_OK )
	{
		//m_CameraInfo.Format("Get camera info failed: %s", fc2ErrorToDescription(error));
		m_ProgramInfo.Format("Get camera info failed: %s\r\n", fc2ErrorToDescription(error));
		ShowOutput(g_dbgBuf);
	}else{
		//m_CameraInfo.Format("Camera model - %s, Resolution - %s", camInfo.modelName, camInfo.sensorResolution);
		sprintf(g_dbgBuf, "Get Camera model - %s, Resolution - %s\r\n", camInfo.modelName, camInfo.sensorResolution);
		ShowOutput(g_dbgBuf);
		memcpy(m_localCamIp, camInfo.ipAddress.octets, sizeof(m_localCamIp));
		CStringArray strArr;
		CString strResolution;
		strResolution.Format("%s", camInfo.sensorResolution);
		splitString(strResolution, 'x', strArr);
		int nSubStr = strArr.GetCount();
		if(nSubStr > 1){
			m_nCamWidth = atoi(strArr.GetAt(0));
			m_nCamHeight = atoi(strArr.GetAt(1));
		}else{
			m_nCamWidth = 1280;
			m_nCamHeight = 1024;
		}
		m_nWidth = m_nCamWidth*m_dScale;
		m_nHeight = m_nCamHeight*m_dScale;
	}

	// SetTimeStamping
	fc2EmbeddedImageInfo embeddedInfo;
	error = fc2GetEmbeddedImageInfo( m_Context, &embeddedInfo );
	if ( error != FC2_ERROR_OK ){
		sprintf(g_dbgBuf, "Error in fc2GetEmbeddedImageInfo: %s\n", fc2ErrorToDescription(error) );
		MessageBox(g_dbgBuf);
		//return;
	}
	if ( embeddedInfo.timestamp.available != 0 ){       
		embeddedInfo.timestamp.onOff = TRUE;
	}
	error = fc2SetEmbeddedImageInfo( m_Context, &embeddedInfo );
	if ( error != FC2_ERROR_OK ){
		printf( "Error in fc2SetEmbeddedImageInfo: %d\n", error );
		//return;
	}
	// end SetTimeStamping

#if 0                       // by ZWW we do not StartCapture there, StartCapture when begin preview
	//开始捕获视频
	error = fc2StartCapture( m_Context );
	if ( error != FC2_ERROR_OK )
	{
		printf( "Error in fc2StartCapture: %d\n", error );
		fc2Disconnect(m_Context);
		fc2DestroyContext(m_Context);
		m_Context = NULL;
		return FALSE;
	}
#endif
	m_Inited = 1;

	//申请两个buffer，一个用于存储捕获到的图像，另一个用于存储对捕获的图像进行转换后的图像
	error = fc2CreateImage( &m_RawImage );
	if ( error != FC2_ERROR_OK )
	{
		sprintf( g_dbgBuf, "Error in fc2CreateImage: %d\n", error );
		MessageBox(g_dbgBuf);
		goto InitFailed;
	}

	error = fc2CreateImage( &m_ConvertedImage );
	if ( error != FC2_ERROR_OK )
	{
		sprintf( g_dbgBuf, "Error in fc2CreateImage: %d\n", error );
		MessageBox(g_dbgBuf);
		goto InitFailed;
	}

	m_Inited = 2;
	return TRUE;

InitFailed:
	fc2StopCapture(m_Context);
	fc2Disconnect(m_Context);
	fc2DestroyContext(m_Context);
	m_Context = NULL;
	return FALSE;
#else
	Error error;
	PGRGuid arGuid[64];
	BusManager busMgr;
	InterfaceType ifType;
	unsigned int uCamNum;
	error = busMgr.GetNumOfCameras(&uCamNum);
	if( error != PGRERROR_OK ){
		sprintf(g_dbgBuf, "GetNumOfCameras failed cause:%s\n", error.GetDescription());
		MessageBox(g_dbgBuf);
		return FALSE;
	}
	if( uCamNum == 0 ){
		MessageBox("No pointgrey camera detected!");
		return FALSE;
	}
	error = busMgr.GetCameraFromIndex( 0, arGuid );
	if( error != PGRERROR_OK){
		sprintf(g_dbgBuf, "GetCameraFromIndex failed cause:%s\n", error.GetDescription());
		MessageBox(g_dbgBuf);
		return FALSE;
	}
	error = busMgr.GetInterfaceTypeFromGuid( &arGuid[0], &ifType );
	if ( error != PGRERROR_OK )
	{
		sprintf(g_dbgBuf, "GetInterfaceTypeFromGuid failed cause:%s\n", error.GetDescription());
		MessageBox(g_dbgBuf);
		return FALSE;
	} 

	if ( ifType == INTERFACE_GIGE )
	{
		m_pCamera = new GigECamera;
	}
	else
	{
		m_pCamera = new Camera;
	}

	// Connect to the 0th selected camera
	error = m_pCamera->Connect( &arGuid[0] );
	if( error != PGRERROR_OK )
	{
		CString csMessage;
		csMessage.Format( "Connect Failure: %s", error.GetDescription() );
		AfxMessageBox( csMessage, MB_ICONSTOP );

		return FALSE;
	}

	error = m_pCamera->GetCameraInfo( &m_cameraInfo );
	if( error != PGRERROR_OK )
	{
		CString csMessage;
		csMessage.Format("CameraInfo Failure: %s", error.GetDescription() );
		AfxMessageBox( csMessage, MB_ICONSTOP );

		return FALSE;
	}else{
		m_ProgramInfo.Format("Get Camera model - %s, Resolution - %s\r\n", m_cameraInfo.modelName, m_cameraInfo.sensorResolution);
		memcpy(m_localCamIp, m_cameraInfo.ipAddress.octets, sizeof(m_localCamIp));
		CStringArray strArr;
		CString strResolution;
		strResolution.Format("%s", m_cameraInfo.sensorResolution);
		splitString(strResolution, 'x', strArr);
		int nSubStr = strArr.GetCount();
		if(nSubStr > 1){
			m_nCamWidth = atoi(strArr.GetAt(0));
			m_nCamHeight = atoi(strArr.GetAt(1));
		}else{
			m_nCamWidth = 1280;
			m_nCamHeight = 1024;
		}
		m_nWidth = m_nCamWidth*m_dScale;
		m_nHeight = m_nCamHeight*m_dScale;
	}

	error = m_pCamera->GetEmbeddedImageInfo(&m_EmbeddedInfo);
	if( error != PGRERROR_OK){
		CString csMessage;
		csMessage.Format("CameraInfo Failure: %s", error.GetDescription() );
		OutputDebugString(csMessage.GetBuffer(0));
	}else{
		m_EmbeddedInfo.timestamp.onOff = true;
		m_pCamera->SetEmbeddedImageInfo(&m_EmbeddedInfo);
	}

    m_Inited = 2;

	return TRUE;
#endif
}

DWORD WINAPI ThreadCaptureDisplay(LPVOID lpParam)
{
	BOOL ret_err = FALSE;
	char converted_file_name[256];
	CCalibrationDlg *pDlg = (CCalibrationDlg *)lpParam;

	//绘制模板方框: 初始化工作
	//矩形的中心在绘图窗口的中心：获得cstatic左角坐标，再分别用1/2图像高宽相加即可
	CRect cstatic_rect;
	pDlg->m_VideoWnd.GetClientRect(&cstatic_rect); 
	CClientDC dc(&pDlg->m_VideoWnd); //获取设备句柄 
	CPen MatchPen;
    CPen CrossPen;
	MatchPen.CreatePen(PS_SOLID,1,RGB(255,0,0));
    CrossPen.CreatePen(PS_SOLID,1,RGB(0,0,255));

	BITMAPINFO_8bpp bi;
	memset(&bi, 0, sizeof(BITMAPINFO_8bpp));
	DWORD dwPaletteSize = 256*sizeof(RGBQUAD);
	UINT uLineByte = ((pDlg->m_nCamWidth * 8+31)/32) * 4;
	DWORD dwBmBitsSize = uLineByte * pDlg->m_nCamHeight;
	memset(&bi, 0, sizeof(LPBITMAPINFOHEADER));

	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth  = pDlg->m_nCamWidth;
	bi.bmiHeader.biHeight = -pDlg->m_nCamHeight;               // 摄像头采集图像数据是从上到下的，所以这里用负数
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 8;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = dwBmBitsSize;
	bi.bmiHeader.biXPelsPerMeter	= 0;
	bi.bmiHeader.biYPelsPerMeter	= 0;
	bi.bmiHeader.biClrUsed = 256;
	bi.bmiHeader.biClrImportant = 256;
	SetGray(bi);
	dc.SetStretchBltMode(COLORONCOLOR);
	dc.SelectStockObject(NULL_BRUSH);
	HGDIOBJ hOldPen = dc.SelectObject(MatchPen);

	BOOL bGrabOk = TRUE;
	do{
		bGrabOk = pDlg->PointGreyGrabImage();
		if(!bGrabOk){
			Sleep(10);
			continue;
		}
		pDlg->PointGreyGetImageData(&target_image_memdata);
		if(pDlg->m_pYData && !pDlg->m_bPMProcessing){
			memcpy(pDlg->m_pYData, target_image_memdata, uLineByte*pDlg->m_ConvertedImage.rows);
			pDlg->m_bPMProcessing = TRUE;
		}

		StretchDIBits(dc.GetSafeHdc(), 0, 0, cstatic_rect.Width(), cstatic_rect.Height(),
			0, 0, pDlg->m_ConvertedImage.cols, pDlg->m_ConvertedImage.rows, target_image_memdata, (BITMAPINFO *)&bi, DIB_RGB_COLORS, SRCCOPY);

		if(pDlg->m_bMatch){
			CRect rect;
			rect.left = pDlg->m_fMatchPosX[0]*pDlg->m_dScale;
			rect.right = pDlg->m_fMatchPosX[2]*pDlg->m_dScale;
			rect.top = pDlg->m_fMatchPosY[0]*pDlg->m_dScale;
			rect.bottom = pDlg->m_fMatchPosY[2]*pDlg->m_dScale;
			sprintf(g_dbgBuf, "%d %d %d %d!!!!!!!!!\n", rect.left, rect.right, rect.top, rect.bottom);
			//OutputDebugString(g_dbgBuf);
			dc.Rectangle(rect);
		}
	}while(pDlg->m_bPreview);
	dc.SelectObject(hOldPen);

	pDlg->m_VideoWnd.Invalidate();
	fc2Error error = fc2StopCapture( pDlg->m_Context );
	if ( error != FC2_ERROR_OK )
	{
		sprintf(g_dbgBuf, "Error in fc2StopCapture: %d\n", error);
		::AfxMessageBox(g_dbgBuf);
	}else{
		OutputDebugString("fcStopCapture success\n");
	}

	return 0;
}

UINT CCalibrationDlg::DoGrabLoop()
{
    Error error;
    CString csMessage;

    error = m_pCamera->StartCapture();
    if( error != PGRERROR_OK )
    {
        csMessage.Format("StartCapture Failure: %s", error.GetDescription());
        AfxMessageBox( csMessage, MB_ICONSTOP );
        return -1;
    }

    EnableItem();
    CRect WndRect;
    m_VideoWnd.GetClientRect(&WndRect); 
    CClientDC dc(&m_VideoWnd); //获取设备句柄 
    CPen MatchPen;
    CPen CrossPen;
    MatchPen.CreatePen(PS_SOLID,1,RGB(255,0,0));
    CrossPen.CreatePen(PS_SOLID,1,RGB(0,0,255));

    // ROI时启用双缓冲绘制
    CDC dcmem;                      //创建一个内存DC
    CBitmap bmp;                    //创建一个内存位图
    dcmem.CreateCompatibleDC(&dc);  //创建一个内存DC
    bmp.CreateCompatibleBitmap(&dc,WndRect.Width(),WndRect.Height()); //创建一个内存位图
    dcmem.SelectObject(&bmp);       //关联内存DC和内存位图

    BITMAPINFO_8bpp bi;
    memset(&bi, 0, sizeof(BITMAPINFO_8bpp));
    DWORD dwPaletteSize = 256*sizeof(RGBQUAD);
    UINT uLineByte = ((m_nCamWidth * 8+31)/32) * 4;
    DWORD dwBmBitsSize = uLineByte * m_nCamHeight;
    memset(&bi, 0, sizeof(LPBITMAPINFOHEADER));

    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth  = m_nCamWidth;
    bi.bmiHeader.biHeight = -m_nCamHeight;               // 摄像头采集图像数据是从上到下的，所以这里用负数
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 8;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = dwBmBitsSize;
    bi.bmiHeader.biXPelsPerMeter	= 0;
    bi.bmiHeader.biYPelsPerMeter	= 0;
    bi.bmiHeader.biClrUsed = 256;
    bi.bmiHeader.biClrImportant = 256;
    SetGray(bi);
    dc.SetStretchBltMode(COLORONCOLOR);
    dc.SelectStockObject(NULL_BRUSH);
    HGDIOBJ hOldPen = dc.SelectObject(MatchPen);
    dcmem.SetStretchBltMode(COLORONCOLOR);

    // Start of main grab loop
    while( m_bPreview )
    {
        error = m_pCamera->RetrieveBuffer( &m_rawImage );
        if( error != PGRERROR_OK )
        {
            csMessage.Format("RetrieveBuffer Failure: %s", error.GetDescription());
            continue;
        }

        // Check to see if the thread should die.
        if( !m_bPreview ){
            break;
        }

        // Update current framerate.
        //m_processedFrameRate.NewFrame();

        // Do post processing on the image.
        unsigned int rows, cols, stride;
        PixelFormat format;
        m_rawImage.GetDimensions( &rows, &cols, &stride, &format );    

        CSingleLock dataLock( &m_csData );
        dataLock.Lock();
        error = m_rawImage.Convert( PIXEL_FORMAT_MONO8, &m_processedImage );
        dataLock.Unlock();
        if( error != PGRERROR_OK )
        {
            csMessage.Format("Convert Failure: %s", error.GetDescription() );
            continue;
        }

        if(m_pYData && !m_bPMProcessing){
            memcpy(m_pYData, m_processedImage.GetData(), uLineByte*rows);
            m_bPMProcessing = TRUE;
        }

        if(g_pMainDlg->m_nEnableROI){
            dcmem.FillSolidRect(0, 0, WndRect.Width(), WndRect.Height(), RGB(105,105,105));
            int nLeft = GetPrivateProfileInt("POINTGREY", "ROILEFT", 0, theApp.m_szConfigFile);
            int nTop = GetPrivateProfileInt("POINTGREY", "ROITOP", 0, theApp.m_szConfigFile);
            int nWd = GetPrivateProfileInt("POINTGREY", "ROIWIDTH", g_pMainDlg->m_nCamWidth, theApp.m_szConfigFile);
            int nHt = GetPrivateProfileInt("POINTGREY", "ROIHEIGHT", g_pMainDlg->m_nCamHeight, theApp.m_szConfigFile);
            StretchDIBits(dcmem.GetSafeHdc(), nLeft*g_pMainDlg->m_dScale, nTop*g_pMainDlg->m_dScale, nWd*g_pMainDlg->m_dScale, nHt*g_pMainDlg->m_dScale,
                nLeft, rows - nTop - nHt, nWd, nHt, m_processedImage.GetData(), (BITMAPINFO *)&bi, DIB_RGB_COLORS, SRCCOPY);
            dc.BitBlt(0,0,WndRect.Width(),WndRect.Height(),&dcmem,0,0,SRCCOPY);
        }else{
            StretchDIBits(dc.GetSafeHdc(), 0, 0, WndRect.Width(), WndRect.Height(),
                0, 0, cols, rows, m_processedImage.GetData(), (BITMAPINFO *)&bi, DIB_RGB_COLORS, SRCCOPY);
        }

        if(m_bMatch)
        {
            CRect rect;
            rect.left = m_fMatchPosX[0]*m_dScale;
            rect.right = m_fMatchPosX[2]*m_dScale;
            rect.top = m_fMatchPosY[0]*m_dScale;
            rect.bottom = m_fMatchPosY[2]*m_dScale;
            sprintf(g_dbgBuf, "%d %d %d %d!!!!!!!!!\n", rect.left, rect.right, rect.top, rect.bottom);
            //OutputDebugString(g_dbgBuf);
            dc.Rectangle(rect);
        }/*else{
            CRect rect;
            rect.left = WndRect.Width()/2-100;
            rect.right = WndRect.Width()/2+100;
            rect.top = WndRect.Height()/2-100;
            rect.bottom = WndRect.Height()/2+100;
            sprintf(g_dbgBuf, "%d %d %d %d!!!!!!!!!\n", rect.left, rect.right, rect.top, rect.bottom);
            OutputDebugString(g_dbgBuf);
            dc.Rectangle(rect);
        }*/

        /*if(g_pMainDlg->m_nEnableROI){
            int nLeft = GetPrivateProfileInt("POINTGREY", "ROILEFT", 0, theApp.m_szConfigFile)*g_pMainDlg->m_dScale;
            int nTop = GetPrivateProfileInt("POINTGREY", "ROITOP", 0, theApp.m_szConfigFile)*g_pMainDlg->m_dScale;
            int nWd = GetPrivateProfileInt("POINTGREY", "ROIWIDTH", g_pMainDlg->m_nCamWidth, theApp.m_szConfigFile)*g_pMainDlg->m_dScale;
            int nHt = GetPrivateProfileInt("POINTGREY", "ROIHEIGHT", g_pMainDlg->m_nCamHeight, theApp.m_szConfigFile)*g_pMainDlg->m_dScale;
            HGDIOBJ hTmpPen = dc.SelectObject(CrossPen);
            dc.Rectangle(nLeft, nTop, nLeft+nWd, nTop+nHt);
			dc.SelectObject(hTmpPen);
        }*/
    }

    dc.SelectObject(hOldPen);
    m_VideoWnd.Invalidate();
    error = m_pCamera->StopCapture();
    if( error != PGRERROR_OK ){
        csMessage.Format( "Stop Failure: %s", error.GetDescription() );
        AfxMessageBox( csMessage, MB_ICONSTOP );
    }else{
        OutputDebugString("相机停止预览\n");
    }
    dcmem.DeleteDC();
    bmp.DeleteObject();

    // End of main grab loop
    SetEvent( m_heventThreadDone );

    return 0;
}

UINT CCalibrationDlg::ThreadGrabImage( void* pparam )
{
    CCalibrationDlg *pDlg = (CCalibrationDlg *)pparam;
    UINT uiRetval = pDlg->DoGrabLoop();   
    if( uiRetval != 0 )
    {
        CString csMessage;
        csMessage.Format("The grab thread has encountered a problem and had to terminate.");
        AfxMessageBox( csMessage, MB_ICONSTOP );

        // Signal that the thread has died.
        SetEvent( pDlg->m_heventThreadDone );
    }
    pDlg->m_bPreview = FALSE;

    return uiRetval;
}

DWORD WINAPI ThreadMatch(LPVOID lpParam)
{
	BOOL ret_err = FALSE;
	char converted_file_name[256];
	CCalibrationDlg *pDlg = (CCalibrationDlg *)lpParam;
	BOOL bTemplateLoad = FALSE;                                 // 模板文件是否加载成功
	DWORD dwLineSize;

    OutputDebugString("begin ThreadMatch\n");
	if(!pDlg->m_pYData){
		dwLineSize = ((pDlg->m_nCamWidth * 8+31)/32) * 4;
		DWORD dwBmBitsSize = dwLineSize * pDlg->m_nCamHeight; 
		pDlg->m_pYData = (BYTE*)malloc(dwBmBitsSize);
		if(!pDlg->m_pYData)
			return 0;
		else
			memset(pDlg->m_pYData, 0, dwBmBitsSize);
	}

	if(pDlg->m_nPMType == 6){
#ifdef NIENABLE
		OutputDebugString("NI Enabled Version");
		Image* templateImage = imaqCreateImage(IMAQ_IMAGE_U8, 0);
		Image* searchImage = imaqCreateImage(IMAQ_IMAGE_U8, 0);

		if(templateImage && searchImage){
			MatchPatternOptions matchOptions;
			matchOptions.mode = IMAQ_MATCH_ROTATION_INVARIANT;      //IMAQ_MATCH_SHIFT_INVARIANT;
			matchOptions.minContrast = 10;
			matchOptions.subpixelAccuracy = FALSE;
			matchOptions.numMatchesRequested = 2;
			matchOptions.minMatchScore = 500;
			matchOptions.numRanges = 0;
			matchOptions.angleRanges = NULL;
			matchOptions.matchFactor = 0;
			int numMatches;
			while(pDlg->m_bMatch){
				if(pDlg->m_bReloadTemplate){
					if (!imaqReadFile(templateImage, pDlg->m_cCurrentMatchFile.GetBuffer(0), NULL, NULL)) {
						sprintf(g_dbgBuf, "Unable to Read the Template File: %s by ZWW\n", pDlg->m_cCurrentMatchFile.GetBuffer(0));
						OutputDebugString(g_dbgBuf);
						Sleep(10);
						bTemplateLoad = FALSE;
					}else{
						imaqLearnPattern(templateImage, IMAQ_LEARN_ALL);
						bTemplateLoad = TRUE;
						pDlg->m_bReloadTemplate = FALSE;
					}
				}

				if(bTemplateLoad){
					if(!pDlg->m_bPMProcessing){                         // 搜索图片数据未拷贝
						Sleep(5);
						continue;
					}else{
						sprintf(g_dbgBuf, "imaqArrayToImage %d %d by ZWW\n", pDlg->PointGreyGetImageWidth(), pDlg->PointGreyGetImageHeight());
						OutputDebugString(g_dbgBuf);
						imaqArrayToImage(searchImage, pDlg->m_pYData, pDlg->PointGreyGetImageWidth(), pDlg->PointGreyGetImageHeight());
						//CStopwatch sw;      // added by ZWW
						PatternMatch* report = imaqMatchPattern(searchImage, templateImage, &matchOptions, IMAQ_NO_RECT, &numMatches);
						if (!report) {
							char* err = imaqGetErrorText(imaqGetLastError());
							sprintf(g_dbgBuf, "模式匹配失败: %s by ZWW\n", err);
							OutputDebugString(g_dbgBuf);
							imaqDispose(err);
						}
						//-----------------------------------------------------------------------
						//  Put the number of matches found on the dialog & draw the matches on
						//  the image.  After that, we are done with the report so dispose it.
						//-----------------------------------------------------------------------
						//m_matchesText.Format("Matches Found: %d, time: %lds", numMatches, sw.Now());
						if(numMatches > 0){
							for (int j = 0; j < 4; ++j) {
								pDlg->m_fMatchPosX[j] = (report[0].corner[j].x + 0.5);
								pDlg->m_fMatchPosY[j] = (report[0].corner[j].y + 0.5);
							}
							sprintf(g_dbgBuf, "Matches Found: %d, Position: %f %f %f %f %f %f %f %f", numMatches, report[0].corner[0].x, report[0].corner[0].y, 
								report[0].corner[1].x, report[0].corner[1].y, report[0].corner[2].x, report[0].corner[2].y, report[0].corner[3].x, report[0].corner[3].y);
							SetEvent(pDlg->m_MatchEvent);
						}else{
							sprintf(g_dbgBuf, "Matches no Found");
							for (int j = 0; j < 4; ++j) {
								pDlg->m_fMatchPosX[j] = 0;
								pDlg->m_fMatchPosY[j] = 0;
							}
						}
						OutputDebugString(g_dbgBuf);

						RGBValue red = {0x20, 0x20, 0xff, 0};
						//DrawBoxAroundMatches(report, numMatches, red, SEARCH_WINDOW);
						imaqDispose(report);

						pDlg->m_bPMProcessing = FALSE;
					}
				}else{
					Sleep(40);
					continue;
				}
			}
		}

		for (int j = 0; j < 4; ++j) {
			pDlg->m_fMatchPosX[j] = 0;
			pDlg->m_fMatchPosY[j] = 0;
		}
		if(templateImage){
			imaqDispose(templateImage);
			templateImage = NULL;
		}
		if(searchImage){
			imaqDispose(searchImage);
			searchImage = NULL;
		}
#endif
    }
#ifdef USEMIL
	else if(TRUE){
		MIL_ID MilSourceImage,           /* Model  image buffer identifier.   */
			MilTargetImage,              /* Target image buffer identifier.   */
			MilModel,                    /* Model identifier.                 */
			MilResult;                   /* Result identifier.                */
		MIL_DOUBLE   RealX       = 0.0,           /* Model real position in x.         */
			RealY       = 0.0,           /* Model real position in y.         */
			RealAngle   = 0.0,           /* Model real angle.                 */
			X           = 0.0,           /* Model position in x found.        */
			Y           = 0.0,           /* Model position in y found.        */
			Angle       = 0.0,           /* Model angle found.                */
			Score       = 0.0,           /* Model correlation score.          */
			Time        = 0.0,           /* Model search time.                */
			ErrX        = 0.0,           /* Model error position in x.        */
			ErrY        = 0.0,           /* Model error position in y.        */
			ErrAngle    = 0.0,           /* Model error angle.                */
			SumErrX     = 0.0,           /* Model total error position in x.  */
			SumErrY     = 0.0,           /* Model total error position in y.  */
			SumErrAngle = 0.0,           /* Model total error angle.          */
			SumTime     = 0.0;           /* Model total search time.          */
		MIL_INT      NbFound     = 0;             /* Number of models found.           */
		MIL_DOUBLE   AnnotationColor = M_COLOR_GREEN; /* Drawing color.                */
		int nModleWidth, nModleHeight;
		MbufAlloc2d(g_pMainDlg->m_MilSystem, g_pMainDlg->m_nCamWidth, g_pMainDlg->m_nCamHeight, 8, M_IMAGE+M_PROC, &MilSourceImage);
        PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)"使用MIL进行模式匹配！\r\n", 0);
		while(g_pMainDlg->m_bMatch){
			if(g_pMainDlg->m_bReloadTemplate){
				MbufRestore(g_pMainDlg->m_cCurrentMatchFile.GetBuffer(0), g_pMainDlg->m_MilSystem, &MilTargetImage);
				bTemplateLoad = TRUE;
				g_pMainDlg->m_bReloadTemplate = FALSE;
				nModleWidth = MbufInquire(MilTargetImage, M_SIZE_X, M_NULL);
				nModleHeight = MbufInquire(MilTargetImage, M_SIZE_Y, M_NULL);
			}
			if(bTemplateLoad){
				if(!pDlg->m_bPMProcessing){                         // 搜索图片数据未拷贝
					Sleep(5);
					continue;
				}else{
					MbufPut2d(MilSourceImage, 0, 0, g_pMainDlg->m_nCamWidth, g_pMainDlg->m_nCamHeight, pDlg->m_pYData);
					//MbufSaveA("shit.bmp", MilSourceImage);

					MpatAllocModel(g_pMainDlg->m_MilSystem, MilTargetImage, 0, 0, nModleWidth, nModleHeight, M_NORMALIZED, &MilModel);
					MpatSetSpeed(MilModel, M_MEDIUM);

					/* Set the position search accuracy. */
					MpatSetAccuracy(MilModel, M_HIGH);

					/* Activate the search model angle mode. */
					MpatSetAngle(MilModel, M_SEARCH_ANGLE_MODE, M_ENABLE);

					/* Set the search model range angle. */
					MpatSetAngle(MilModel, M_SEARCH_ANGLE_DELTA_NEG, ROTATED_FIND_ANGLE_DELTA_NEG);
					MpatSetAngle(MilModel, M_SEARCH_ANGLE_DELTA_POS, ROTATED_FIND_ANGLE_DELTA_POS);

					/* Set the search model angle accuracy. */
					MpatSetAngle(MilModel, M_SEARCH_ANGLE_ACCURACY, ROTATED_FIND_MIN_ANGLE_ACCURACY);

					/* Set the search model angle interpolation mode to bilinear. */
					MpatSetAngle(MilModel, M_SEARCH_ANGLE_INTERPOLATION_MODE, M_BILINEAR);

					/* Preprocess the model. */
					MpatPreprocModel(MilSourceImage, MilModel, M_DEFAULT);

					/* Allocate a result buffer. */
					MpatAllocResult(g_pMainDlg->m_MilSystem, 1L, &MilResult);

					MpatFindModel(MilSourceImage, MilModel, MilResult);

					/* If the model was found above the acceptance threshold. */
					if (MpatGetNumber(MilResult, M_NULL) == 1L)
					{
						MpatGetResult(MilResult, M_POSITION_X, &X);
						MpatGetResult(MilResult, M_POSITION_Y, &Y);
						MpatGetResult(MilResult, M_SCORE, &Score);
						MpatGetResult(MilResult, M_ANGLE, &Angle);
						TRACE("x: %f y:%f Score: %f Angle: %f\n", X, Y, Score, Angle);
						if(Score > 0.9){
							g_pMainDlg->m_fMatchPosX[0] = X - nModleWidth/2;
							g_pMainDlg->m_fMatchPosY[0] = Y - nModleHeight/2;
							g_pMainDlg->m_fMatchPosX[1] = pDlg->m_fMatchPosX[0]+nModleWidth;
							g_pMainDlg->m_fMatchPosY[1] = pDlg->m_fMatchPosY[0];
							g_pMainDlg->m_fMatchPosX[2] = pDlg->m_fMatchPosX[0]+nModleWidth;
							g_pMainDlg->m_fMatchPosY[2] = pDlg->m_fMatchPosY[0]+nModleHeight;
							g_pMainDlg->m_fMatchPosX[3] = pDlg->m_fMatchPosX[0];
							g_pMainDlg->m_fMatchPosY[3] = pDlg->m_fMatchPosY[0]+nModleHeight;
							PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)"匹配成功！\r\n", 0);
							SetEvent(pDlg->m_MatchEvent);
						}else{
							PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)"得分低匹配失败！\r\n", 0);
						}
					}else{
						PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)"匹配失败！\r\n", 0);
					}

					bTemplateLoad = FALSE;
					pDlg->m_bPMProcessing = FALSE;
					MpatFree(MilResult);
					MpatFree(MilModel);
					MbufFree(MilTargetImage);
				}
			}else{
				Sleep(40);
				continue;
			}
		}
		/* Free all allocations. */
		MbufFree(MilSourceImage);
	}
#endif
	else{
		CvvImage imgTemplate;
		IplImage* src, *temp, *ftmp;
		double min_val;
		double max_val;
		CvPoint min_loc;
		CvPoint max_loc;
		int nTypes[6]={CV_TM_SQDIFF, CV_TM_SQDIFF_NORMED, CV_TM_CCORR, CV_TM_CCORR_NORMED, CV_TM_CCOEFF, CV_TM_CCOEFF_NORMED};
        
        PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)"使用OpenCV进行模式匹配！\r\n", 0);
		src = cvCreateImage(cvSize(pDlg->m_nCamWidth, pDlg->m_nCamHeight), IPL_DEPTH_8U, 1);
		ftmp = NULL;
		if(src){
			while(pDlg->m_bMatch){
				if(pDlg->m_bReloadTemplate){
					if(!imgTemplate.Load(pDlg->m_cCurrentMatchFile.GetBuffer(0), CV_LOAD_IMAGE_GRAYSCALE)){
						sprintf(g_dbgBuf, "Unable to Read the Template File: %s by ZWW\n", pDlg->m_cCurrentMatchFile.GetBuffer(0));
						OutputDebugString(g_dbgBuf);
						Sleep(10);
						bTemplateLoad = FALSE;
					}else{
						temp = imgTemplate.GetImage();

						bTemplateLoad = TRUE;
						pDlg->m_bReloadTemplate = FALSE;
						if(ftmp)
							cvReleaseImage( &ftmp );
						ftmp = cvCreateImage(cvSize(pDlg->m_nCamWidth-temp->width+1, pDlg->m_nCamHeight-temp->height+1), IPL_DEPTH_32F, 1);
					}
				}

				if(bTemplateLoad && ftmp){
					if(!pDlg->m_bPMProcessing){                         // 搜索图片数据未拷贝
						Sleep(5);
						continue;
					}else{
#if 1
						memcpy(src->imageData, pDlg->m_pYData, src->imageSize);
#else
						for(int i = 0; i < pDlg->m_nHeightRes; i++){
							memcpy(src->imageData+i*dwLineSize, pDlg->m_pYData+(pDlg->m_nHeightRes-1-i)*dwLineSize, dwLineSize);
						}
#endif

						cvMatchTemplate(src, temp, ftmp, nTypes[pDlg->m_nPMType]);
						//cvNormalize( ftmp, ftmp, 1, 0, CV_MINMAX);
						cvMinMaxLoc(ftmp, &min_val, &max_val, &min_loc, &max_loc, NULL);

						if (pDlg->m_nPMType == CV_TM_SQDIFF || pDlg->m_nPMType == CV_TM_SQDIFF_NORMED){
							pDlg->m_fMatchPosX[0] = min_loc.x;
							pDlg->m_fMatchPosY[0] = min_loc.y;
						}else{
							pDlg->m_fMatchPosX[0] = max_loc.x;
							pDlg->m_fMatchPosY[0] = max_loc.y;
						}
						pDlg->m_fMatchPosX[1] = pDlg->m_fMatchPosX[0]+temp->width;
						pDlg->m_fMatchPosY[1] = pDlg->m_fMatchPosY[0];
						pDlg->m_fMatchPosX[2] = pDlg->m_fMatchPosX[0]+temp->width;
						pDlg->m_fMatchPosY[2] = pDlg->m_fMatchPosY[0]+temp->height;
						pDlg->m_fMatchPosX[3] = pDlg->m_fMatchPosX[0];
						pDlg->m_fMatchPosY[3] = pDlg->m_fMatchPosY[0]+temp->height;

						sprintf(g_dbgBuf, "min_val: %lf max_val: %lf Position: %f %f %f %f %f %f %f %f\n", min_val, max_val, pDlg->m_fMatchPosX[0], pDlg->m_fMatchPosY[0], 
							pDlg->m_fMatchPosX[1], pDlg->m_fMatchPosY[1], pDlg->m_fMatchPosX[2], pDlg->m_fMatchPosY[2], pDlg->m_fMatchPosX[3], pDlg->m_fMatchPosY[3]);
						TRACE(g_dbgBuf);
						if(pDlg->m_nPMType == CV_TM_CCOEFF_NORMED){
							if(max_val < 0.85){				// 对于归一化相关系数匹配法来说小于该值认为匹配失败
								PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)"得分太低匹配失败！\r\n", 0);
							}else{
                                g_strPMSuccess.Format("匹配成功 topleft: %f %f bottomright: %f %f\r\n", pDlg->m_fMatchPosX[0], pDlg->m_fMatchPosY[0], pDlg->m_fMatchPosX[2], pDlg->m_fMatchPosY[2]);
								PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)g_strPMSuccess.GetBuffer(0), 0);
								SetEvent(pDlg->m_MatchEvent);
								bTemplateLoad = FALSE;
                                break;
							}
						}else{
							SetEvent(pDlg->m_MatchEvent);
						}

						pDlg->m_bPMProcessing = FALSE;
					}
				}else{
					Sleep(30);
					continue;
				}
			}// end while(pDlg->m_bMatch)
			cvReleaseImage(&src);
			cvReleaseImage(&ftmp);
		}
	}

	if(pDlg->m_pYData){
		free(pDlg->m_pYData);
		pDlg->m_pYData = NULL;
	}
    PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)"模式匹配线程退出\r\n", 0);

    Sleep(PM_SHOWTIME);                        // 让匹配结果显示一下
    for (int j = 0; j < 4; ++j) {
        pDlg->m_fMatchPosX[j] = 0;
        pDlg->m_fMatchPosY[j] = 0;
    }

    pDlg->m_bPMProcessing = FALSE;
	pDlg->m_bMatch = FALSE;
	return 0;
}

void CCalibrationDlg::OnBnClickedBtnPreview()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_Inited <= 0){
        BOOL bInitOk = PointGreyCameraInit();
        if(!bInitOk){
            MessageBox("相机初始化失败，请重启相机后再重试！");
            return;
        }else{
            /*if(!PointGreyGrabImage()){
                MessageBox("抓取图像失败，请重启局部相机后在重试！");
                return;
            }*/
        }
    }
	if(m_bPreview){
		MessageBox("正在预览中");
		return;
	}

#ifdef OLDMETHOD
    fc2Error error = fc2StartCapture( m_Context );
    if ( error != FC2_ERROR_OK )
    {
        sprintf( g_dbgBuf, "Error in fc2StartCapture: %s\n", fc2ErrorToDescription(error) );
        MessageBox(g_dbgBuf, MB_OK);
        return;
    }

	m_bPreview = TRUE;
	hThreadCaptureDisplay = CreateThread(NULL, 0, ThreadCaptureDisplay, (LPVOID)this, 0, &ThreadCaptureDisplayID);
	if(hThreadCaptureDisplay){
		CloseHandle(hThreadCaptureDisplay);
		hThreadCaptureDisplay = NULL;
		EnableItem();
	}else{
		m_bPreview = FALSE;
	}
#else
    m_bPreview = TRUE;   
    AfxBeginThread( ThreadGrabImage, this );
#endif

#if 0
    /*m_bMatch = TRUE;
    HANDLE hThreadMatch = CreateThread(NULL, 0, ThreadMatch, (LPVOID)this, 0, &ThreadCaptureDisplayID);
    if(hThreadMatch){
        CloseHandle(hThreadMatch);
    }else{
        m_bMatch = FALSE;
    }*/
    RunMatchThread();
#endif
}

void CCalibrationDlg::OnBnClickedBtnStoppreview()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bPreview = FALSE;
	m_bMatch = FALSE;
	EnableItem(0, FALSE);
}

BOOL CCalibrationDlg::PointGreyGrabImage()
{
	fc2Error error;
	// Retrieve the image
	error = fc2RetrieveBuffer(m_Context, &m_RawImage);
	if ( error != FC2_ERROR_OK )
	{
		sprintf(g_dbgBuf, "Error in retrieveBuffer: %s\n", fc2ErrorToDescription(error));
		OutputDebugString(g_dbgBuf);
		//MessageBox(g_dbgBuf);
		return FALSE;
	}

	// Convert the final image to mono8
	error = fc2ConvertImageTo(FC2_PIXEL_FORMAT_MONO8, &m_RawImage, &m_ConvertedImage);
	if ( error != FC2_ERROR_OK ) {
		sprintf(g_dbgBuf, "Error in fc2ConvertImageTo: %s\n", fc2ErrorToDescription(error));
		OutputDebugString(g_dbgBuf);
		//MessageBox(g_dbgBuf);
		return FALSE;
	}
	return TRUE;
}

BOOL CCalibrationDlg::PointGreySaveImage(char* dst_file_name)
{
	fc2Error error;
	if(m_Inited < 2)
		return FALSE;
#ifdef OLDMETHOD
	error = fc2SaveImage( &m_ConvertedImage, dst_file_name, FC2_BMP);
	if ( error != FC2_ERROR_OK )
	{
		sprintf(g_dbgBuf, "Error in fc2SaveImage: %s\n", fc2ErrorToDescription(error));
		MessageBox(g_dbgBuf);
		return FALSE;
	}
#else
    CSingleLock dataLock( &m_csData );
    dataLock.Lock();
    m_processedImage.Save(dst_file_name, BMP);
    dataLock.Unlock();
#endif

	return TRUE;
}

BOOL CCalibrationDlg::PointGreyGetImageData(unsigned char ** target_image_memdata)
{
	fc2Error fc2err = fc2GetImageData(&m_ConvertedImage, target_image_memdata);
	return TRUE;
}

BOOL CCalibrationDlg::PointGreyTerminate()
{
#ifdef OLDMETHOD
	fc2Context context = m_Context;
	fc2Image rawImage = m_RawImage;
	fc2Image convertedImage = m_ConvertedImage;
	fc2Error error;
	if(m_Inited >= 1)
	{
		error = fc2Disconnect(context);
		if ( error != FC2_ERROR_OK )
		{
			sprintf(g_dbgBuf, "Error in fc2Disconnect: %s\n", fc2ErrorToDescription(error) );
			::AfxMessageBox(g_dbgBuf);
		}

		error = fc2DestroyContext( context );
		if ( error != FC2_ERROR_OK )
		{
			sprintf(g_dbgBuf, "Error in fc2DestroyContext: %s\n", fc2ErrorToDescription(error) );
			::AfxMessageBox(g_dbgBuf);
		}
		if(m_Inited >= 2){
			error = fc2DestroyImage( &rawImage );
			if ( error != FC2_ERROR_OK ){
				sprintf(g_dbgBuf, "Error in first fc2DestroyImage: %s\n", fc2ErrorToDescription(error) );
				::AfxMessageBox(g_dbgBuf);
			}

			error = fc2DestroyImage( &convertedImage );
			if ( error != FC2_ERROR_OK ){
				sprintf(g_dbgBuf, "Error in second fc2DestroyImage: %s\n", fc2ErrorToDescription(error) );
				::AfxMessageBox(g_dbgBuf);
			}
		}
		m_Inited = 0;
	}
#else
    m_bPreview = FALSE;      
    DWORD dwRet = WaitForSingleObject( m_heventThreadDone, 5000 );
    if ( dwRet == WAIT_TIMEOUT )
    {
        // Timed out while waiting for thread to exit
    }

    m_camCtlDlg.Hide();
    m_camCtlDlg.Disconnect();

    if (m_pCamera != NULL)
    {
        m_pCamera->Disconnect();
    }
#endif

	return TRUE;
}

int CCalibrationDlg::PointGreyGetImageWidth()
{
#ifdef OLDMETHOD
	int width = m_ConvertedImage.cols;
#else
    int width = m_processedImage.GetCols();
#endif
	return width;
}

int CCalibrationDlg::PointGreyGetImageHeight()
{
#ifdef OLDMETHOD
	int height = m_ConvertedImage.rows;
#else
    int height = m_processedImage.GetRows();
#endif
	return height;
}

BOOL CCalibrationDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	int nStep = 0;
	nStep = GetDlgItemInt(IDC_EDIT_MOVESTEP);
	if(nStep < 0){
		nStep = 0;
	}else if(nStep > 255){
		nStep = 255;
	}
	if(pMsg->message == WM_LBUTTONDOWN){
		if(pMsg->hwnd == GetDlgItem(IDC_BTN_UP)->m_hWnd){
			CONTROL_Move(m_lLoginID, 0, nStep, FALSE);
		}else if(pMsg->hwnd == GetDlgItem(IDC_BTN_DOWN)->m_hWnd){
			CONTROL_Move(m_lLoginID, 1, nStep, FALSE);
		}else if(pMsg->hwnd == GetDlgItem(IDC_BTN_LEFT)->m_hWnd){
			CONTROL_Move(m_lLoginID, 3, nStep, FALSE);
		}else if(pMsg->hwnd == GetDlgItem(IDC_BTN_RIGHT)->m_hWnd){
			CONTROL_Move(m_lLoginID, 2, nStep, FALSE);
		}else if(pMsg->hwnd == GetDlgItem(IDC_BTN_CLOCKWISE)->m_hWnd){
			CONTROL_PlatformRotate(m_lLoginID, TRUE, nStep, FALSE);
		}else if(pMsg->hwnd == GetDlgItem(IDC_BTN_ANTICLOCKWISE)->m_hWnd){
			CONTROL_PlatformRotate(m_lLoginID, FALSE, nStep, FALSE);
		}
	}else if(pMsg->message == WM_LBUTTONUP){
		if(pMsg->hwnd == GetDlgItem(IDC_BTN_UP)->m_hWnd){
			CONTROL_Move(m_lLoginID, 0, nStep, TRUE);
		}else if(pMsg->hwnd == GetDlgItem(IDC_BTN_DOWN)->m_hWnd){
			CONTROL_Move(m_lLoginID, 1, nStep, TRUE);
		}else if(pMsg->hwnd == GetDlgItem(IDC_BTN_LEFT)->m_hWnd){
			CONTROL_Move(m_lLoginID, 3, nStep, TRUE);
		}else if(pMsg->hwnd == GetDlgItem(IDC_BTN_RIGHT)->m_hWnd){
			CONTROL_Move(m_lLoginID, 2, nStep, TRUE);
		}else if(pMsg->hwnd == GetDlgItem(IDC_BTN_CLOCKWISE)->m_hWnd){
			CONTROL_PlatformRotate(m_lLoginID, TRUE, nStep, TRUE);
		}else if(pMsg->hwnd == GetDlgItem(IDC_BTN_ANTICLOCKWISE)->m_hWnd){
			CONTROL_PlatformRotate(m_lLoginID, FALSE, nStep, TRUE);
		}
	}else if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN){
		 CWnd* focus = GetFocus();
		 CEdit *pEdit = m_TreeInit.GetEditControl();
		 if(pEdit){
			 pEdit->SendMessage(WM_KEYDOWN,VK_RETURN);
			 return TRUE;
		 }else{
			 pEdit = m_TreeCircle.GetEditControl();
			 if(pEdit){
				 pEdit->SendMessage(WM_KEYDOWN,VK_RETURN);
				 return TRUE;
			 }
		 }
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

DWORD WINAPI LocateTemplate(LPVOID lpParam)
{
	CCalibrationDlg *pMainDlg = (CCalibrationDlg *)lpParam;

	CFile dibFile;
	char chTemp[200];
	sprintf(chTemp, "temp.bmp");
	// 先回到预置位
	CONTROL_MoveTo(pMainDlg->m_lLoginID, pMainDlg->m_lPrePosX, pMainDlg->m_lPrePosY, pMainDlg->m_lPrePosZ);
	int nQueryTimes = 0;
	while(nQueryTimes < 10){
		LONG lPosX;
		LONG lPosY;
		INT nStatus;
		BOOL bGetPos = CONTROL_QueryPos(pMainDlg->m_lLoginID, &lPosX, &lPosY, &nStatus);
		if(bGetPos){
			/*if(lPosX == pMainDlg->m_lPrePosX && lPosY == pMainDlg->m_lPrePosY){
				break;
			}*/
            if(abs(lPosX - pMainDlg->m_lPrePosX) <= 10 && abs(lPosY - pMainDlg->m_lPrePosY) <= 10 ){
                break;
            }
		}
		Sleep(1000);
	}
	if(nQueryTimes >= 10){
		g_pMainDlg->ShowOutput("移动至预置位过程中出错，退出定位流程！\r\n");
		goto EndLocate;
	}

	if(!dibFile.Open(chTemp, CFile::modeRead | CFile::shareDenyWrite)) {
		sprintf(g_dbgBuf, "文件 %s 不存在跳过匹配\r\n", chTemp);
		g_pMainDlg->ShowOutput(g_dbgBuf);
		goto EndLocate;
	}else{
		dibFile.Close();
		pMainDlg->m_bReloadTemplate = TRUE;
		pMainDlg->m_cCurrentMatchFile = chTemp;
	}

	Sleep(1000);		// 这里需完善模式匹配的可靠性，即真正的找到了模板才计算位置并移动

	sprintf(g_dbgBuf, "匹配文件 %s\r\n", chTemp);
	g_pMainDlg->ShowOutput(g_dbgBuf);
	DWORD result = WaitForSingleObject(pMainDlg->m_MatchEvent, 5000);
	if(WAIT_OBJECT_0 == result){
		LONG lPosX = (pMainDlg->m_fMatchPosX[0]+pMainDlg->m_fMatchPosX[2])/2;
		LONG lPosY = (pMainDlg->m_fMatchPosY[0]+pMainDlg->m_fMatchPosY[2])/2;

		sprintf(g_dbgBuf, "匹配结果中心：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);
		lPosX -= pMainDlg->m_nCamWidth/2;
		lPosY -= pMainDlg->m_nCamHeight/2;
		sprintf(g_dbgBuf, "距离图像中心偏移：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

		lPosX *= pMainDlg->m_dHorizontalRatio;
		lPosY *= pMainDlg->m_dVerticalRatio;
		sprintf(g_dbgBuf, "转换为机械偏移：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

		lPosX = pMainDlg->m_lPrePosX-lPosX;
		lPosY = pMainDlg->m_lPrePosY-lPosY;

		/*CONTROL_MoveTo(pMainDlg->m_lLoginID, lPosX, lPosY, 0);
		sprintf(g_dbgBuf, "将要移动至：%ld %ld\r\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);
		g_pMainDlg->ShowOutput(g_dbgBuf);

        // 查询是否移动到指定位置
        nQueryTimes = 0;
        while(nQueryTimes < 10){
            LONG lPosX2;
            LONG lPosY2;
            INT nStatus2;
            BOOL bGetPos = CONTROL_QueryPos(pMainDlg->m_lLoginID, &lPosX2, &lPosY2, &nStatus2);
            if(bGetPos){
                if(lPosX2 == lPosX && lPosY2 == lPosY){
                    break;
                }
            }
            Sleep(1000);
			OutputDebugString("haha");
        }
        if(nQueryTimes >= 10){
            sprintf(g_dbgBuf, "移动至模板定位位置(%ld, %ld)过程中出错，退出定位流程！\r\n", lPosX, lPosY);
            g_pMainDlg->ShowOutput(g_dbgBuf);
            goto EndLocate;
        }*/

		int rt;
		LONG lPosX2, lPosY2;
        /*rt = pMainDlg->MessageBox("是否移动光标到模板中心位置？", "提示", MB_OKCANCEL);
        if(IDOK == rt){
            // 转换为十字光标位置
            lPosX2 = lPosX + pMainDlg->m_lCamLaserShiftX;
            lPosY2 = lPosY + pMainDlg->m_lCamLaserShiftY;

            CONTROL_MoveTo(pMainDlg->m_lLoginID, lPosX2, lPosY2, 0);
            sprintf(g_dbgBuf, "将要移动至：%ld %ld\r\n", lPosX2, lPosY2);
            OutputDebugString(g_dbgBuf);
            g_pMainDlg->ShowOutput(g_dbgBuf);

            Sleep(3000);
        }*/

        rt = IDOK;// pMainDlg->MessageBox("是否移动手指到模板中心位置？", "提示", MB_OKCANCEL);
        if(IDOK == rt){
            // 转换为十字光标位置
            lPosX2 = lPosX + pMainDlg->m_lCamLaserShiftX;
            lPosY2 = lPosY + pMainDlg->m_lCamLaserShiftY;
            // 转换为手指位置
            lPosX2 = lPosX2 - pMainDlg->m_lFingerLaserShiftX;
            lPosY2 = lPosY2 - pMainDlg->m_lFingerLaserShiftY;

            CONTROL_MoveTo(pMainDlg->m_lLoginID, lPosX2, lPosY2, 0);
            sprintf(g_dbgBuf, "将要移动至：%ld %ld\r\n", lPosX2, lPosY2);
            pMainDlg->ShowOutput(g_dbgBuf);

            Sleep(3000);
        }
	}else{
		g_pMainDlg->ShowOutput("匹配失败！\r\n");
	}

EndLocate:
	pMainDlg->m_bLocateTemplate = FALSE;
	return 1;
}

void CCalibrationDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	if(m_lLoginID <= 0){
		MessageBox("请先连接机器人");
		return;
	}
	if(m_bLocateTemplate){
		MessageBox("正在定位模板请稍后");
		return;
	}
	m_bLocateTemplate = TRUE;
	HANDLE hThread = CreateThread(NULL, 0, LocateTemplate, (LPVOID)this, 0, NULL);
	if(hThread){
		CloseHandle(hThread);
	}else{
		m_bLocateTemplate = FALSE;
		MessageBox("启动定位模板线程失败，请重试！");
	}
}

BOOL CCalibrationDlg::MakeTemplate(CString fileName, BOOL bSpecifyRect)
{
	CRect rect;
	BOOL bMakeOK = FALSE;
	m_VideoWnd.GetWindowRect(&rect);
	if(bSpecifyRect){
		CDlgTemplate templateDlg;
		templateDlg.m_Rect = rect;
		templateDlg.DoModal();

		if(!templateDlg.m_bMakeTemplate)
			return FALSE;
		templateDlg.m_TemplateRectTracker.GetTrueRect(&rect);
	}else{
		int nTempWd = GetPrivateProfileInt(_T("COMMON"), _T("PRETEMPWIDTH"), 100, theApp.m_szConfigFile)*m_dScale;
		int nTempHt = GetPrivateProfileInt(_T("COMMON"), _T("PRETEMPHEIGHT"), 100, theApp.m_szConfigFile)*m_dScale;
		int nCenterX = rect.Width()/2;
		int nCenterY = rect.Height()/2;
		rect.left = nCenterX - nTempWd/2;
		rect.right = nCenterX + nTempWd/2;
		rect.top = nCenterY - nTempHt/2;
		rect.bottom = nCenterY + nTempHt/2;
	}

	LPBITMAPFILEHEADER pBmpFileHeader = NULL;
	LPBYTE pBufSrc = NULL;
	LPBYTE pBufDes = NULL;
	BOOL bRs = PointGreySaveImage("Grab.bmp");
	if(bRs){
		CFile dibFile;
		if(!dibFile.Open("Grab.bmp", CFile::modeRead | CFile::shareDenyWrite)) {
			return FALSE;
		}
		pBmpFileHeader = (LPBITMAPFILEHEADER)new BYTE[sizeof(BITMAPFILEHEADER)];
		if(!pBmpFileHeader){
			dibFile.Close();
			goto End;
		}
		int nCount = dibFile.Read((void *)pBmpFileHeader, sizeof(BITMAPFILEHEADER));
		if(nCount != sizeof(BITMAPFILEHEADER)) {
			dibFile.Close();
			goto End;
		} 

		// 判断此文件是不是位图文件（“0x4d42”代表“BM”）
		if(pBmpFileHeader->bfType == 0x4d42) {
			// 计算除位图文件头的空间大小，并分配空间
			DWORD dwDibSize = dibFile.GetLength() - sizeof(BITMAPFILEHEADER);

			LPBYTE pBufSrc = (LPBYTE)malloc(dwDibSize);
			if(pBufSrc){
				dibFile.Read(pBufSrc, dwDibSize);
				// 关闭位图文件
				dibFile.Close();
				PBITMAPINFOHEADER  pBmpHeader;			// LPBITMAPINFO
				pBmpHeader = (PBITMAPINFOHEADER)pBufSrc;
				rect.left/=m_dScale;
				rect.top/=m_dScale;
				rect.right/=m_dScale;
				rect.bottom/=m_dScale;
				if(rect.left < 0)
					rect.left = 0;
				if(rect.top < 0)
					rect.top = 0;
				if(rect.right > pBmpHeader->biWidth)
					rect.right = pBmpHeader->biWidth;
				if(rect.bottom > pBmpHeader->biHeight)
					rect.bottom = pBmpHeader->biHeight;

				TRACE("left %d right %d top %d bottom %d\n", rect.left, rect.right, rect.top, rect.bottom);
				UINT uBitCount = pBmpHeader->biBitCount;
				UINT uLineByteSrc, uLineByteDes;
				uLineByteSrc = (pBmpHeader->biWidth * uBitCount / 8 + 3) / 4 * 4;
				uLineByteDes = (rect.Width() * uBitCount / 8 + 3) / 4 * 4;

				// 从原始图片中扣出模板文件
				BITMAPFILEHEADER bmpFileHd;
				BITMAPINFOHEADER bmpInfoHd;
				memset(&bmpFileHd, 0, sizeof(BITMAPFILEHEADER));
				memset(&bmpInfoHd, 0, sizeof(BITMAPINFOHEADER));
				bmpFileHd.bfType = 0x4d42;
				bmpInfoHd.biSize = sizeof(BITMAPINFOHEADER);
				bmpInfoHd.biWidth = rect.Width();
				bmpInfoHd.biHeight = rect.Height();
				bmpInfoHd.biPlanes = 1;
				bmpInfoHd.biBitCount = pBmpHeader->biBitCount;
				bmpInfoHd.biCompression = BI_RGB;
				bmpInfoHd.biSizeImage = 0;          // biCompression为BI_RGB时可设置为0
				bmpInfoHd.biClrUsed = pBmpHeader->biClrUsed;
				bmpInfoHd.biClrImportant = pBmpHeader->biClrImportant;
				PBYTE pBufData = pBufSrc+sizeof(BITMAPINFOHEADER)+pBmpHeader->biClrUsed*sizeof(RGBQUAD);
				pBufDes = (LPBYTE)malloc(uLineByteDes*rect.Height());
				bmpFileHd.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+pBmpHeader->biClrUsed*sizeof(RGBQUAD);
				bmpFileHd.bfSize = bmpFileHd.bfOffBits + uLineByteDes*rect.Height();
				if(pBufDes){
					for(int i = 0; i < rect.Height(); i++){
						PBYTE pSrc = pBufData+(pBmpHeader->biHeight - rect.top - i - 1)*uLineByteSrc+rect.left*bmpInfoHd.biBitCount/8;
						PBYTE pDes = pBufDes+(rect.Height() - i -1)*uLineByteDes;
						memcpy(pDes, pSrc, uLineByteDes);
					}

					RGBQUAD *pPalette = (RGBQUAD *)(pBufSrc+sizeof(BITMAPINFOHEADER));
					for (int i=0; i < pBmpHeader->biClrUsed; i++)
					{
						pPalette[i].rgbRed = pPalette[i].rgbGreen = pPalette[i].rgbBlue = i;
						pPalette[i].rgbReserved = 0;
					}

					CFile dibFile;
					CString tempName;
					tempName.Format("%s", fileName.GetBuffer(0));
					if(dibFile.Open(tempName.GetBuffer(0), CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive))
					{
						dibFile.Write(&bmpFileHd, sizeof(BITMAPFILEHEADER));

						// 将文件信息头结构写进文件
						dibFile.Write(&bmpInfoHd, sizeof(BITMAPINFOHEADER));

						// 如果有颜色表的话，将颜色表写进文件
						if(pBmpHeader->biClrUsed != 0)
						{
							dibFile.Write(pPalette, pBmpHeader->biClrUsed*sizeof(RGBQUAD));
						}

						UINT uDataSize = uLineByteDes * rect.Height();
						dibFile.Write(pBufDes, uDataSize);
						// 关闭文件
						dibFile.Close();
#if 0						// 1 by ZWW for testing
						m_bReloadTemplate = TRUE;                 // 不再进行匹配
#endif
						m_cCurrentMatchFile = tempName;
						bMakeOK = TRUE;
					}
				}
			}	
		}
	}

End:
	if(pBmpFileHeader)
		delete []pBmpFileHeader;
	if(pBufSrc)
		free(pBufSrc);
	if(pBufDes)
		free(pBufDes);

	return bMakeOK;
}

void CCalibrationDlg::OnBnClickedBtnMaketemp()
{
	// TODO: 在此添加控件通知处理程序代码
	//MakeTemplate("temp.bmp");
    RunMatchThread();
    StopMatchThread();
}


DWORD WINAPI ThreadSearch(LPVOID lpParam)
{
	CCalibrationDlg *pMainDlg = (CCalibrationDlg *)lpParam;
	CString str;
	char chTemp[MAX_PATH];
	CFile dibFile;

	BOOL bMatchAll = TRUE;
	// 匹配并执行
	int nMatchTimes = 5;
	for(int i = 0; i < nMatchTimes; i++){
		if(!pMainDlg->m_bDoSearch){
			bMatchAll = FALSE;
			break;
		}
		if(i == 0){
			sprintf(chTemp, "center.bmp");
		}else if(i == 1){
			sprintf(chTemp, "topleft.bmp");
		}else if(i == 2){
			sprintf(chTemp, "topright.bmp");
		}else if(i == 3){
			sprintf(chTemp, "downleft.bmp");
		}else if(i == 4){
			sprintf(chTemp, "downright.bmp");
		}
		
		if(!dibFile.Open(chTemp, CFile::modeRead | CFile::shareDenyWrite)) {
			sprintf(g_dbgBuf, "文件 %s 不存在跳过匹配\r\n", chTemp);
			pMainDlg->ShowOutput(g_dbgBuf);
			Sleep(2000);
			continue;
		}else{
			dibFile.Close();
			pMainDlg->m_bReloadTemplate = TRUE;
			pMainDlg->m_cCurrentMatchFile = chTemp;
		}

		sprintf(g_dbgBuf, "匹配文件 %s\r\n", chTemp);
		pMainDlg->ShowOutput(g_dbgBuf);

		while(pMainDlg->m_bDoSearch){
			// 等待匹配成功信号
			DWORD result = WaitForSingleObject(pMainDlg->m_MatchEvent, 1000);
			if(WAIT_OBJECT_0 == result){
				sprintf(g_dbgBuf, "第%d个匹配成功\r\n", i);
				pMainDlg->ShowOutput(g_dbgBuf);
				Sleep(2000);
				break;
			} else if(WAIT_TIMEOUT == result){
				OutputDebugString("匹配超时\n");
			}else {
				TRACE("WaitForSingleObject failed %d\n", result);
			}
		}
	}

ExitProcess:
	sprintf(g_dbgBuf, "四角搜索完毕！\r\n", str.GetBuffer(0));
	pMainDlg->ShowOutput(g_dbgBuf);
	pMainDlg->m_bDoSearch = FALSE;
	return 1;
}

void CCalibrationDlg::OnBnClickedBtnSearch()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_bDoSearch){
		MessageBox("测试用例正在执行，请稍后重试！");
		return;
	}

	m_bDoSearch = TRUE;
	HANDLE hThread = CreateThread(NULL, 0, ThreadSearch, (LPVOID)this, 0, NULL);
	if(hThread){
		CloseHandle(hThread);
	}else{
		m_bDoSearch = FALSE;
		MessageBox("启动查找四角及中心失败，请重试！");
	}
}


void CCalibrationDlg::OnBnClickedBtnToprepos()
{
    // TODO: Add your control notification handler code here
    LONG lPosX;
    LONG lPosY;
    LONG lPosZ;
    INT nStatus;
    if(g_pMainDlg->m_lLoginID > 0){
        BOOL bGetPos = CONTROL_QueryPos(g_pMainDlg->m_lLoginID, &lPosX, &lPosY, &nStatus, &lPosZ);
        if(bGetPos){
            CONTROL_MoveTo(m_lLoginID, m_lPrePosX, m_lPrePosY, lPosZ);
        }else{
            MessageBox("获取机器人当前坐标失败！");
        }
    }
}


void CCalibrationDlg::OnBnClickedBtnQuerypos()
{
    // TODO: Add your control notification handler code here
    CString str;
    LONG lPosX;
    LONG lPosY;
    LONG lPosZ;
    INT nStatus;
    BOOL bGetPos = CONTROL_QueryPos(m_lLoginID, &lPosX, &lPosY, &nStatus, &lPosZ);
    if(bGetPos){
        str.Format("RobotErr: %d RobotState: %d lPosX %ld lPosY %ld lPosZ %ld", (nStatus>>8)&0xFF, nStatus&0xFF, lPosX, lPosY, lPosZ);
        MessageBox(str);
    }else{
        MessageBox("查询当前位置失败");
    }
}

DWORD WINAPI CalibrationThread(LPVOID lpParam)
{
	//CCalibrationDlg *pMainDlg = (CCalibrationDlg *)lpParam;
	CString str;
	CString preTemp = "preTemp.bmp";
	// 移动到预置位
	CONTROL_MoveTo(g_pMainDlg->m_lLoginID, g_pMainDlg->m_lPrePosX, g_pMainDlg->m_lPrePosY, g_pMainDlg->m_lPrePosZ);

	Sleep(2000);

	// 制作中心模板
	g_pMainDlg->MakeTemplate(preTemp, FALSE);
	DWORD result = WaitForSingleObject(g_pMainDlg->m_MatchEvent, 2000);
	if(WAIT_OBJECT_0 == result){
		sprintf(g_dbgBuf, "中心模板匹配成功！\r\n");
        g_pMainDlg->ShowOutput(g_dbgBuf);
		Sleep(1000);
	} else if(WAIT_TIMEOUT == result){
		g_pMainDlg->ShowOutput("中心模板匹配超时，退出校准！\r\n");
		goto EndProcess;
	} else {
		g_pMainDlg->ShowOutput("中心模板匹配等待异常，退出校准！\r\n");
		goto EndProcess;
	}

	Sleep(2000);

	int nQueryTimes = 0;
	// 控制下位机水平方向上移动，并进行匹配
	int nMoveX = GetPrivateProfileInt(_T("CALIBRATION"), _T("MOVEXDIS"), 500, theApp.m_szConfigFile);
	CONTROL_MoveTo(g_pMainDlg->m_lLoginID, g_pMainDlg->m_lPrePosX+nMoveX, g_pMainDlg->m_lPrePosY, g_pMainDlg->m_lPrePosZ);
	while(nQueryTimes < 10){
		LONG lPosX;
		LONG lPosY;
		INT nStatus;
		BOOL bGetPos = CONTROL_QueryPos(g_pMainDlg->m_lLoginID, &lPosX, &lPosY, &nStatus);
		if(bGetPos){
			if(lPosX == g_pMainDlg->m_lPrePosX+nMoveX){
				break;
			}
		}
		Sleep(1000);
	}
	if(nQueryTimes >= 10){
		g_pMainDlg->ShowOutput("校准中水平移动出错，退出校准！\r\n");
		goto EndProcess;
	}
	sprintf(g_dbgBuf, "已移动到指定点 %ld %ld\r\n", g_pMainDlg->m_lPrePosX+nMoveX, g_pMainDlg->m_lPrePosY);
	g_pMainDlg->ShowOutput(g_dbgBuf);
	Sleep(2000);

	g_pMainDlg->m_bReloadTemplate = TRUE;
	g_pMainDlg->m_cCurrentMatchFile = preTemp;
	g_pMainDlg->m_dHorizontalRatio = nMoveX;
	result = WaitForSingleObject(g_pMainDlg->m_MatchEvent, 2000);
	if(WAIT_OBJECT_0 == result){
		sprintf(g_dbgBuf, "水平机械位移：%d 像素位移：%f\n", nMoveX, (g_pMainDlg->m_fMatchPosX[2] + g_pMainDlg->m_fMatchPosX[0] - g_pMainDlg->m_nCamWidth)/2);
		OutputDebugString(g_dbgBuf);
		g_pMainDlg->m_dHorizontalRatio = g_pMainDlg->m_dHorizontalRatio/((g_pMainDlg->m_fMatchPosX[2] + g_pMainDlg->m_fMatchPosX[0] - g_pMainDlg->m_nCamWidth)/2);
		sprintf(g_dbgBuf, "水平移动后中心模板匹配成功，水平距离像素比：%f\r\n", g_pMainDlg->m_dHorizontalRatio);
		g_pMainDlg->ShowOutput(g_dbgBuf);
		str.Format("%f", g_pMainDlg->m_dHorizontalRatio);
		WritePrivateProfileString("CALIBRATION", "HORIZONTALRATIO", str.GetBuffer(0), theApp.m_szConfigFile);
		Sleep(2000);
	} else if(WAIT_TIMEOUT == result){
		g_pMainDlg->ShowOutput("水平移动后中心模板匹配超时，退出校准！\r\n");
		goto EndProcess;
	} else {
		g_pMainDlg->ShowOutput("水平移动后中心模板匹配等待异常，退出校准！\r\n");
		goto EndProcess;
	}

	//goto EndProcess;		// by ZWW for testing

	// 控制下位机垂直方向上移动，并进行匹配
	nQueryTimes = 0;
	int nMoveY = GetPrivateProfileInt(_T("CALIBRATION"), _T("MOVEYDIS"), 500, theApp.m_szConfigFile);
	CONTROL_MoveTo(g_pMainDlg->m_lLoginID, g_pMainDlg->m_lPrePosX, g_pMainDlg->m_lPrePosY+nMoveY, g_pMainDlg->m_lPrePosZ);
	while(nQueryTimes < 10){
		LONG lPosX;
		LONG lPosY;
		INT nStatus;
		BOOL bGetPos = CONTROL_QueryPos(g_pMainDlg->m_lLoginID, &lPosX, &lPosY, &nStatus);
		if(bGetPos){
			if(lPosY == g_pMainDlg->m_lPrePosY+nMoveX){
				break;
			}
		}
		Sleep(1000);
	}
	if(nQueryTimes >= 10){
		g_pMainDlg->ShowOutput("校准中垂直移动出错，退出校准！\r\n");
		goto EndProcess;
	}
	sprintf(g_dbgBuf, "已移动到指定点 %ld %ld\r\n", g_pMainDlg->m_lPrePosX+nMoveX, g_pMainDlg->m_lPrePosY);
	g_pMainDlg->ShowOutput(g_dbgBuf);
	Sleep(2000);

	g_pMainDlg->m_bReloadTemplate = TRUE;
	g_pMainDlg->m_cCurrentMatchFile = preTemp;
	g_pMainDlg->m_dVerticalRatio = nMoveY;
	result = WaitForSingleObject(g_pMainDlg->m_MatchEvent, 2000);
	if(WAIT_OBJECT_0 == result){
		sprintf(g_dbgBuf, "垂直机械位移：%d 像素位移：%f\n", nMoveY, (g_pMainDlg->m_fMatchPosY[2] + g_pMainDlg->m_fMatchPosY[0] - g_pMainDlg->m_nCamHeight)/2);
		OutputDebugString(g_dbgBuf);
		g_pMainDlg->m_dVerticalRatio = g_pMainDlg->m_dVerticalRatio/((g_pMainDlg->m_fMatchPosY[2] + g_pMainDlg->m_fMatchPosY[0] - g_pMainDlg->m_nCamHeight)/2);
		sprintf(g_dbgBuf, "垂直移动后中心模板匹配成功，垂直距离像素比：%f\r\n", g_pMainDlg->m_dVerticalRatio);
		g_pMainDlg->ShowOutput(g_dbgBuf);
		str.Format("%f", g_pMainDlg->m_dVerticalRatio);
		WritePrivateProfileString("CALIBRATION", "VERTICALRATIO", str.GetBuffer(0), theApp.m_szConfigFile);
		Sleep(2000);
	} else if(WAIT_TIMEOUT == result){
		g_pMainDlg->ShowOutput("垂直移动后中心模板匹配超时，退出校准！\r\n");
		goto EndProcess;
	} else {
		g_pMainDlg->ShowOutput("垂直移动后中心模板匹配等待异常，退出校准！\r\n");
		goto EndProcess;
	}

EndProcess:
	g_pMainDlg->m_bCalibration = FALSE;
	return 1;
}

void CCalibrationDlg::OnBnClickedBtnCalibration()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_bCalibration){
		MessageBox("校准执行中，请稍后重试！");
		return;
	}

	m_bCalibration = TRUE;
	HANDLE hThread = CreateThread(NULL, 0, CalibrationThread, (LPVOID)this, 0, NULL);
	if(hThread){
		CloseHandle(hThread);
	}else{
		m_bCalibration = FALSE;
		MessageBox("启动校准失败，请重试！");
	}
}


void CCalibrationDlg::OnBnClickedBtnReset()
{
	// TODO: 在此添加控件通知处理程序代码
	if(IDYES != MessageBox("是否复位机器人？", "提示", MB_YESNO)){
		return;
	}
	if(CONTROL_Extend(m_lLoginID, RESET_ROBOT, 0)){
		ShowOutput("下位机复位\r\n");
	}
}

void CCalibrationDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}

#pragma comment(lib, "Version.lib")
BOOL CAboutDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here
    TCHAR szFullPath[MAX_PATH]; 
    DWORD dwVerInfoSize = 0; 
    DWORD dwVerHnd; 
    VS_FIXEDFILEINFO * pFileInfo; 

    if(GetModuleFileName(NULL, szFullPath, sizeof(szFullPath))){
        dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd); 
        if (dwVerInfoSize) 
        { 
            // If we were able to get the information, process it: 
            HANDLE hMem; 
            LPVOID lpvMem; 
            unsigned int uInfoSize = 0; 

            hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize); 
            lpvMem = GlobalLock(hMem); 
            GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpvMem); 

            ::VerQueryValue(lpvMem, (LPTSTR)_T( "\\"), (void**)&pFileInfo, &uInfoSize); 

            WORD m_nProdVersion[4]; 

            // Product version from the FILEVERSION of the version info resource 
            m_nProdVersion[0] = HIWORD(pFileInfo-> dwProductVersionMS); 
            m_nProdVersion[1] = LOWORD(pFileInfo-> dwProductVersionMS); 
            m_nProdVersion[2] = HIWORD(pFileInfo-> dwProductVersionLS); 
            m_nProdVersion[3] = LOWORD(pFileInfo-> dwProductVersionLS); 

            CString strVersion ; 
            //strVersion.Format(_T( "The file 's version : %d.%d.%d.%d "),m_nProdVersion[0], m_nProdVersion[1],m_nProdVersion[2],m_nProdVersion[3]);
            strVersion.Format(_T("测试工具 %d.%d.%d.%d"), m_nProdVersion[0], m_nProdVersion[1],m_nProdVersion[2],m_nProdVersion[3]);
            ((CWnd*)GetDlgItem(IDC_STATIC_VERSION))->SetWindowText(strVersion);

            GlobalUnlock(hMem);
            GlobalFree(hMem);
            //AfxMessageBox(strVersion);
        }
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CCalibrationDlg::OnBnClickedBtnRobotset()
{
	// TODO: 在此添加控件通知处理程序代码
#if USE_MODALSETDLG
	CDlgRobotSet robotSet;
	robotSet.m_pMainDlg = this;
	robotSet.DoModal();
#else
	if(!m_pRobotSetDlg){
		m_pRobotSetDlg = new CDlgRobotSet(this);
		m_pRobotSetDlg->Create(IDD_DLG_ROBOTSET, this);
		m_pRobotSetDlg->ShowWindow(SW_SHOW);
	}else{
		m_pRobotSetDlg->SetActiveWindow();
	}
#endif
}


void CCalibrationDlg::OnBnClickedBtnPmset()
{
	// TODO: 在此添加控件通知处理程序代码
    if(m_bCaseRun){
        MessageBox("测试用例正在执行，不能进行全局参数设置");
        return;
    }
#if USE_MODALSETDLG
	CDlgPMSet pmSet;
	pmSet.DoModal();
#else
	if(!m_pPMSetDlg){
		m_pPMSetDlg = new CDlgPMSet(this);
		m_pPMSetDlg->Create(IDD_DLG_PMSET, this);
		m_pPMSetDlg->ShowWindow(SW_SHOW);
	}else{
		m_pPMSetDlg->SetActiveWindow();
	}
#endif
}

void CCalibrationDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ClientToScreen(&point);
	m_VideoWnd.ScreenToClient(&point);
	sprintf(g_dbgBuf, "Click position: %d %d", point.x, point.y);
	OutputDebugString(g_dbgBuf);
	CRect rect;
	m_VideoWnd.GetClientRect(rect);
	m_bMoveIcon = FALSE;
	if(point.x < 0 || point.x > rect.right || point.y < 0 || point.y > rect.bottom || m_lLoginID == 0)
		return;

	LONG lPosX, lPosY, lPosX2, lPosY2;
	lPosX = point.x/m_dScale;
	lPosY = point.y/m_dScale;
	sprintf(g_dbgBuf, "转换为像素位置：%ld %ld\n", lPosX, lPosY);
	OutputDebugString(g_dbgBuf);

	lPosX -= m_nCamWidth/2;
	lPosY -= m_nCamHeight/2;
	sprintf(g_dbgBuf, "距离图像中心偏移：%ld %ld\n", lPosX, lPosY);
	OutputDebugString(g_dbgBuf);

	lPosX *= m_dHorizontalRatio;
	lPosY *= m_dVerticalRatio;
	sprintf(g_dbgBuf, "转换为机械偏移：%ld %ld\n", lPosX, lPosY);
	OutputDebugString(g_dbgBuf);

	// 测试是否需要移动到预置位
#if 0
	lPosX = m_lPrePosX-lPosX;
	lPosY = m_lPrePosY-lPosY;
#else
	LONG lCurPosX;
	LONG lCurPosY;
	INT nStatus;
	BOOL bGetPos = CONTROL_QueryPos(m_lLoginID, &lCurPosX, &lCurPosY, &nStatus);
	if(bGetPos){
		lPosX = lCurPosX-lPosX;
		lPosY = lCurPosY-lPosY;
	}else{
		MessageBox("查询当前位置失败，请检查！");
		return;
	}
#endif

	if(m_pFingerMoveDlg){
		// 转换为十字光标位置
		lPosX2 = lPosX + m_lCamLaserShiftX;
		lPosY2 = lPosY + m_lCamLaserShiftY;
		if(m_pFingerMoveDlg->m_CkQuickFinger.GetCheck()){
			lPosX2 = lPosX2 - m_lQuickClickLaserShiftX;
			lPosY2 = lPosY2 - m_lQuickClickLaserShiftY;
		}else{
			lPosX2 = lPosX2 - m_lFingerLaserShiftX;
			lPosY2 = lPosY2 - m_lFingerLaserShiftY;
		}
		m_pFingerMoveDlg->SetTargetPos(lPosX2, lPosY2);
	}else if(m_pIconDragDlg){
		// 转换为十字光标位置
		lPosX2 = lPosX + m_lCamLaserShiftX;
		lPosY2 = lPosY + m_lCamLaserShiftY;
		lPosX2 = lPosX2 - m_lFingerLaserShiftX;
		lPosY2 = lPosY2 - m_lFingerLaserShiftY;
		m_pIconDragDlg->SetTargetPos(lPosX2, lPosY2);
	}else{
		int rt = MessageBox("是否移动慢击手指到点击位置？", "提示", MB_OKCANCEL);
		if(IDOK == rt){
			// 转换为十字光标位置
			lPosX2 = lPosX + m_lCamLaserShiftX;
			lPosY2 = lPosY + m_lCamLaserShiftY;
			// 转换为手指位置
			lPosX2 = lPosX2 - m_lFingerLaserShiftX;
			lPosY2 = lPosY2 - m_lFingerLaserShiftY;

			CONTROL_MoveTo(m_lLoginID, lPosX2, lPosY2, 0);
			sprintf(g_dbgBuf, "将要移动慢击手指至：%ld %ld\r\n", lPosX2, lPosY2);
			g_pMainDlg->ShowOutput(g_dbgBuf);
		}
	}

	CDialogEx::OnRButtonDown(nFlags, point);
}


void CCalibrationDlg::OnBnClickedBtnTriggerset()
{
	// TODO: 在此添加控件通知处理程序代码
	CDlgTriggerSet triggerSet;
	triggerSet.DoModal();
}


LRESULT CCalibrationDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
    BOOL bResetName = FALSE;
    switch(message)
    {
    case ROBOTDLG_EXIT:
        if(m_pRobotSetDlg){
            delete m_pRobotSetDlg;
            m_pRobotSetDlg = NULL;
        }
        break;
	case PMSETDLG_EXIT:
		if(m_pPMSetDlg){
            delete m_pPMSetDlg;
            m_pPMSetDlg = NULL;
        }
		break;
    case UPDATEINFO_MSG:
        ShowOutput((char*)wParam);
        break;
    case INITSETTING_EXIT:
        if(m_pInitSetDlg){
            bResetName = TRUE;
            delete m_pInitSetDlg;
            m_pInitSetDlg = NULL;
        }
        break;
    case MODULEPM_EXIT:
        if(m_pModulePMDlg){
            bResetName = TRUE;
            delete m_pModulePMDlg;
            m_pModulePMDlg = NULL;
        }
        break;
    case FINGERMOVE_EXIT:
        if(m_pFingerMoveDlg){
            delete m_pFingerMoveDlg;
            m_pFingerMoveDlg = NULL;
        }
        break;
    case ICONDRAG_EXIT:
        if(m_pIconDragDlg){
            delete m_pIconDragDlg;
            m_pIconDragDlg = NULL;
        }
        break;
    case TRIGGERMODE_EXIT:
        if(m_pTriggerSetDlg){
            delete m_pTriggerSetDlg;
            m_pTriggerSetDlg = NULL;
        }
        break;
    case ROTATE_EXIT:
        if(m_pRotateDlg){
            delete m_pRotateDlg;
            m_pRotateDlg = NULL;
        }
        break;
	case CIRCLETIMES_EXIT:
		if(m_pCircleTimesDlg){
			m_nCircleTimes = m_pCircleTimesDlg->m_nTimes;
			delete m_pCircleTimesDlg;
			m_pCircleTimesDlg = NULL;
		}
		break;
    case TIMEDELAY_EXIT:
        if(m_pTimeDelayDlg){
            delete m_pTimeDelayDlg;
            m_pTimeDelayDlg = NULL;
        }
        break;
    default:
        break;
    }
    if(bResetName){
        ModuleData *pModuleData = NULL;
        HTREEITEM hItem = NULL;
        CTreeCtrl *pTreeCtrl = NULL;
        if(m_TreeInit.IsWindowEnabled()){
            hItem = m_TreeInit.GetSelectedItem();
            pTreeCtrl = &m_TreeInit;
        }else if(m_TreeCircle.IsWindowEnabled()){
            hItem = m_TreeCircle.GetSelectedItem();
            pTreeCtrl = &m_TreeCircle;
        }
        if(hItem){
            pModuleData = (ModuleData *)m_TreeInit.GetItemData(hItem);
            if(pModuleData){
                pTreeCtrl->SetItemText(hItem, pModuleData->cName);
            }
        }
    }
	return CDialogEx::WindowProc(message, wParam, lParam);
}


void CCalibrationDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ClientToScreen(&point);
	m_VideoWnd.ScreenToClient(&point);
	sprintf(g_dbgBuf, "double click position: %d %d", point.x, point.y);
	OutputDebugString(g_dbgBuf);
	CRect rect;
	m_VideoWnd.GetClientRect(rect);
	if(point.x < 0 || point.x > rect.right || point.y < 0 || point.y > rect.bottom || m_lLoginID == 0)
		return;

	LONG lPosX, lPosY, lPosX2, lPosY2;
	lPosX = point.x/m_dScale;
	lPosY = point.y/m_dScale;
	sprintf(g_dbgBuf, "转换为像素位置：%ld %ld\n", lPosX, lPosY);
	OutputDebugString(g_dbgBuf);

	lPosX -= m_nCamWidth/2;
	lPosY -= m_nCamHeight/2;
	sprintf(g_dbgBuf, "距离图像中心偏移：%ld %ld\n", lPosX, lPosY);
	OutputDebugString(g_dbgBuf);

	lPosX *= m_dHorizontalRatio;
	lPosY *= m_dVerticalRatio;
	sprintf(g_dbgBuf, "转换为机械偏移：%ld %ld\n", lPosX, lPosY);
	OutputDebugString(g_dbgBuf);

	lPosX = m_lPrePosX-lPosX;
	lPosY = m_lPrePosY-lPosY;

	int rt = MessageBox("是否移动快击手指到点击位置？", "提示", MB_OKCANCEL);
	if(IDOK == rt){
		// 转换为十字光标位置
		lPosX2 = lPosX + m_lCamLaserShiftX;
		lPosY2 = lPosY + m_lCamLaserShiftY;
		// 转换为手指位置
		lPosX2 = lPosX2 - m_lQuickClickLaserShiftX;
		lPosY2 = lPosY2 - m_lQuickClickLaserShiftY;

		CONTROL_MoveTo(m_lLoginID, lPosX2, lPosY2, 0);
		sprintf(g_dbgBuf, "将要移动快击手指至：%ld %ld\r\n", lPosX2, lPosY2);
		ShowOutput(g_dbgBuf);
	}

	CDialogEx::OnLButtonDblClk(nFlags, point);
}


void CCalibrationDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rect;
	m_VideoWnd.GetClientRect(&rect);
	ClientToScreen(&point);
	m_VideoWnd.ScreenToClient(&point);
	if(point.x < 0 || point.x > rect.right || point.y < 0 || point.y > rect.bottom)
		return;
	m_downPoint = point;
	TRACE("down point x: %d y: %d", point.x, point.y);

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CCalibrationDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rect;
	CString str;
	m_VideoWnd.GetClientRect(&rect);
	ClientToScreen(&point);
	m_VideoWnd.ScreenToClient(&point);
	if(point.x < 0 || point.x > rect.right || point.y < 0 || point.y > rect.bottom){
		m_downPoint.x = m_downPoint.y = -1;
		m_bMoveIcon = FALSE;
		return;
	}
	if(m_downPoint.x < 0 || m_downPoint.y < 0){
		m_bMoveIcon = FALSE;
		return;
	}
	m_upPoint = point;
	TRACE("up point x: %d y: %d", point.x, point.y);

	BOOL bSaveParam = FALSE;
	double dAngle = 0;
	double dLen = sqrt((double)(m_downPoint.x-m_upPoint.x)*(m_downPoint.x-m_upPoint.x) + (m_downPoint.y-m_upPoint.y)*(m_downPoint.y-m_upPoint.y));
	if(dLen < 10){			// 小于10像素则不处理
		m_bMoveIcon = FALSE;
		return;
	}

	CClientDC dc(&m_VideoWnd); //获取设备句柄
	dc.MoveTo(m_downPoint.x, m_downPoint.y);
	dc.LineTo(m_upPoint.x, m_upPoint.y);

	if(m_downPoint.x == m_upPoint.x){
		dAngle = LP_PI/2;
	}else{
		dAngle = atan(double(m_downPoint.y - m_upPoint.y)/(m_downPoint.x-m_upPoint.x));
	}

	LONG lPosX, lPosY, lPosX1, lPosY1, lPosX2, lPosY2;
	lPosX = m_downPoint.x/m_dScale;
	lPosY = m_downPoint.y/m_dScale;
	sprintf(g_dbgBuf, "转换为像素位置：%ld %ld\n", lPosX, lPosY);
	OutputDebugString(g_dbgBuf);

	lPosX -= m_nCamWidth/2;
	lPosY -= m_nCamHeight/2;
	sprintf(g_dbgBuf, "距离图像中心偏移：%ld %ld\n", lPosX, lPosY);
	OutputDebugString(g_dbgBuf);

	lPosX *= m_dHorizontalRatio;
	lPosY *= m_dVerticalRatio;
	sprintf(g_dbgBuf, "转换为机械偏移：%ld %ld\n", lPosX, lPosY);
	OutputDebugString(g_dbgBuf);

#if 0
	lPosX = m_lPrePosX-lPosX;
	lPosY = m_lPrePosY-lPosY;
#else
	LONG lCurPosX;
	LONG lCurPosY;
	INT nStatus;
	BOOL bGetPos = CONTROL_QueryPos(m_lLoginID, &lCurPosX, &lCurPosY, &nStatus);
	if(bGetPos){
		lPosX = lCurPosX-lPosX;
		lPosY = lCurPosY-lPosY;
	}else{
		MessageBox("查询当前位置失败，请检查！");
		return;
	}
#endif

	// 移动图标模块
	if(m_pIconDragDlg){
		if(m_lLoginID > 0){
			// 转换为十字光标位置
			lPosX1 = lPosX + m_lCamLaserShiftX;
			lPosY1 = lPosY + m_lCamLaserShiftY;
			// 转换为手指位置
			lPosX1 = lPosX1 - m_lFingerLaserShiftX;
			lPosY1 = lPosY1 - m_lFingerLaserShiftY;
		}

		lPosX = m_upPoint.x/m_dScale;
		lPosY = m_upPoint.y/m_dScale;
		sprintf(g_dbgBuf, "转换为像素位置：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

		lPosX -= m_nCamWidth/2;
		lPosY -= m_nCamHeight/2;
		sprintf(g_dbgBuf, "距离图像中心偏移：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

		lPosX *= m_dHorizontalRatio;
		lPosY *= m_dVerticalRatio;
		sprintf(g_dbgBuf, "转换为机械偏移：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

#if 0
		lPosX = m_lPrePosX-lPosX;
		lPosY = m_lPrePosY-lPosY;
#else
		lPosX = lCurPosX-lPosX;
		lPosY = lCurPosY-lPosY;
#endif

		if(m_lLoginID > 0){
			// 转换为十字光标位置
			lPosX2 = lPosX + m_lCamLaserShiftX;
			lPosY2 = lPosY + m_lCamLaserShiftY;
			// 转换为手指位置
			lPosX2 = lPosX2 - m_lFingerLaserShiftX;
			lPosY2 = lPosY2 - m_lFingerLaserShiftY;
			m_pIconDragDlg->SetStartEndPos(lPosX1, lPosY1, lPosX2, lPosY2);
		}
		return;
	}

	int rt;
	if(m_bMoveIcon)
	{
		if(m_lLoginID > 0){
			// 转换为十字光标位置
			lPosX1 = lPosX + m_lCamLaserShiftX;
			lPosY1 = lPosY + m_lCamLaserShiftY;
			// 转换为手指位置
			lPosX1 = lPosX1 - m_lFingerLaserShiftX;
			lPosY1 = lPosY1 - m_lFingerLaserShiftY;

			//CONTROL_MoveTo(m_lLoginID, lPosX1, lPosY1, 0);
		}

		lPosX = m_upPoint.x/m_dScale;
		lPosY = m_upPoint.y/m_dScale;
		sprintf(g_dbgBuf, "转换为像素位置：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

		lPosX -= m_nCamWidth/2;
		lPosY -= m_nCamHeight/2;
		sprintf(g_dbgBuf, "距离图像中心偏移：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

		lPosX *= m_dHorizontalRatio;
		lPosY *= m_dVerticalRatio;
		sprintf(g_dbgBuf, "转换为机械偏移：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

#if 0
		lPosX = m_lPrePosX-lPosX;
		lPosY = m_lPrePosY-lPosY;
#else
		lPosX = lCurPosX-lPosX;
		lPosY = lCurPosY-lPosY;
#endif

		if(m_lLoginID > 0){
			// 转换为十字光标位置
			lPosX2 = lPosX + m_lCamLaserShiftX;
			lPosY2 = lPosY + m_lCamLaserShiftY;
			// 转换为手指位置
			lPosX2 = lPosX2 - m_lFingerLaserShiftX;
			lPosY2 = lPosY2 - m_lFingerLaserShiftY;

			//CONTROL_MoveTo(m_lLoginID, lPosX2, lPosY2, 0);
			int nSpeed = GetDlgItemInt(IDC_EDIT_MOVESTEP);

			BYTE timeBefore = GetPrivateProfileInt("ROBOT", "TIMEBEFORE", 0, theApp.m_szConfigFile);
			BYTE timeAfter = GetPrivateProfileInt("ROBOT", "TIMEAFTER", 0, theApp.m_szConfigFile);

			CONTROL_Slide(m_lLoginID, lPosX1, lPosY1, lPosX2, lPosY2, timeBefore, timeAfter, nSpeed, 400);
		}
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		m_bMoveIcon = FALSE;
	}else{
		if(m_lLoginID > 0){
			rt = MessageBox("是否移动慢击手指到第一个点位置？", "提示", MB_OKCANCEL);
			if(IDOK == rt){
				// 转换为十字光标位置
				lPosX1 = lPosX + m_lCamLaserShiftX;
				lPosY1 = lPosY + m_lCamLaserShiftY;
				// 转换为手指位置
				lPosX1 = lPosX1 - m_lFingerLaserShiftX;
				lPosY1 = lPosY1 - m_lFingerLaserShiftY;

				CONTROL_MoveTo(m_lLoginID, lPosX1, lPosY1, 0);
				sprintf(g_dbgBuf, "将要移动慢击手指至：%ld %ld\r\n", lPosX1, lPosY1);
				ShowOutput(g_dbgBuf);
				bSaveParam = TRUE;
			}
		}

		lPosX = m_upPoint.x/m_dScale;
		lPosY = m_upPoint.y/m_dScale;
		sprintf(g_dbgBuf, "转换为像素位置：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

		lPosX -= m_nCamWidth/2;
		lPosY -= m_nCamHeight/2;
		sprintf(g_dbgBuf, "距离图像中心偏移：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

		lPosX *= m_dHorizontalRatio;
		lPosY *= m_dVerticalRatio;
		sprintf(g_dbgBuf, "转换为机械偏移：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

#if 0
		lPosX = m_lPrePosX-lPosX;
		lPosY = m_lPrePosY-lPosY;
#else
		lPosX = lCurPosX-lPosX;
		lPosY = lCurPosY-lPosY;
#endif

		if(m_lLoginID > 0){
			rt = MessageBox("是否移动慢击手指到第二个点位置？", "提示", MB_OKCANCEL);
			if(IDOK == rt){
				// 转换为十字光标位置
				lPosX2 = lPosX + m_lCamLaserShiftX;
				lPosY2 = lPosY + m_lCamLaserShiftY;
				// 转换为手指位置
				lPosX2 = lPosX2 - m_lFingerLaserShiftX;
				lPosY2 = lPosY2 - m_lFingerLaserShiftY;

				CONTROL_MoveTo(m_lLoginID, lPosX2, lPosY2, 0);
				sprintf(g_dbgBuf, "将要移动慢击手指至：%ld %ld\r\n", lPosX2, lPosY2);
				ShowOutput(g_dbgBuf);
				bSaveParam = TRUE;
			}
		}
		if(!bSaveParam){
			ShowOutput("不保存手指位置\r\n");
			return;
		}

		if(dAngle < LP_PI/4 && dAngle > -LP_PI/4){
			OutputDebugString("水平方向");

			if(lPosX1 > lPosX2){
				str.Format("%d", lPosX1);
				WritePrivateProfileString("ROBOT", "LEFTX", str.GetBuffer(0), theApp.m_szConfigFile);
				str.Format("%d", lPosY1);
				WritePrivateProfileString("ROBOT", "LEFTY", str.GetBuffer(0), theApp.m_szConfigFile);
				str.Format("%d", lPosX2);
				WritePrivateProfileString("ROBOT", "RIGHTX", str.GetBuffer(0), theApp.m_szConfigFile);
				str.Format("%d", lPosY2);
				WritePrivateProfileString("ROBOT", "RIGHTY", str.GetBuffer(0), theApp.m_szConfigFile);
			}else{
				str.Format("%d", lPosX2);
				WritePrivateProfileString("ROBOT", "LEFTX", str.GetBuffer(0), theApp.m_szConfigFile);
				str.Format("%d", lPosY2);
				WritePrivateProfileString("ROBOT", "LEFTY", str.GetBuffer(0), theApp.m_szConfigFile);
				str.Format("%d", lPosX1);
				WritePrivateProfileString("ROBOT", "RIGHTX", str.GetBuffer(0), theApp.m_szConfigFile);
				str.Format("%d", lPosY1);
				WritePrivateProfileString("ROBOT", "RIGHTY", str.GetBuffer(0), theApp.m_szConfigFile);
			}
		}else{
			OutputDebugString("垂直方向");

			if(lPosY1 > lPosY2){
				str.Format("%d", lPosX1);
				WritePrivateProfileString("ROBOT", "UPX", str.GetBuffer(0), theApp.m_szConfigFile);
				str.Format("%d", lPosY1);
				WritePrivateProfileString("ROBOT", "UPY", str.GetBuffer(0), theApp.m_szConfigFile);
				str.Format("%d", lPosX2);
				WritePrivateProfileString("ROBOT", "DOWNX", str.GetBuffer(0), theApp.m_szConfigFile);
				str.Format("%d", lPosY2);
				WritePrivateProfileString("ROBOT", "DOWNY", str.GetBuffer(0), theApp.m_szConfigFile);
			}else{
				str.Format("%d", lPosX2);
				WritePrivateProfileString("ROBOT", "UPX", str.GetBuffer(0), theApp.m_szConfigFile);
				str.Format("%d", lPosY2);
				WritePrivateProfileString("ROBOT", "UPY", str.GetBuffer(0), theApp.m_szConfigFile);
				str.Format("%d", lPosX1);
				WritePrivateProfileString("ROBOT", "DOWNX", str.GetBuffer(0), theApp.m_szConfigFile);
				str.Format("%d", lPosY1);
				WritePrivateProfileString("ROBOT", "DOWNY", str.GetBuffer(0), theApp.m_szConfigFile);
			}
		}
	}

	m_downPoint.x = m_downPoint.y = -1;
	CDialogEx::OnLButtonUp(nFlags, point);
}

BOOL CCalibrationDlg::BaslerCameraInit()
{
	Pylon::PylonAutoInitTerm autoInitTerm;
	try
	{
		// Create the event handler.
		CEventHandler eventHandler;

		// Only look for cameras supported by Camera_t. 
		CDeviceInfo info;
		info.SetDeviceClass( Camera_t::DeviceClass());

		// Create an instant camera object with the first found camera device matching the specified device class.
		Camera_t camera( CTlFactory::GetInstance().CreateFirstDevice( info));

		// For demonstration purposes only, add sample configuration event handlers to print out information
		// about camera use and image grabbing.
		camera.RegisterConfiguration( new CConfigurationEventPrinter, RegistrationMode_Append, Cleanup_Delete); // Camera use.

		// Register the event handler.
		camera.RegisterImageEventHandler( &eventHandler, RegistrationMode_Append, Cleanup_None);
		camera.RegisterCameraEventHandler( &eventHandler, "EventExposureEndData", eMyExposureEndEvent, RegistrationMode_ReplaceAll, Cleanup_None);
		camera.RegisterCameraEventHandler( &eventHandler, "EventFrameStartOvertriggerData", eMyFrameStartOvertrigger, RegistrationMode_Append, Cleanup_None);

		// Camera event processing must be activated first, the default is off.
		camera.GrabCameraEvents = true;

		// Open the camera for setting parameters.
		camera.Open();

		if (IsWritable(camera.OffsetX))
		{
			camera.OffsetX.SetValue(camera.OffsetX.GetMin());
		}
		if (IsWritable(camera.OffsetY))
		{
			camera.OffsetY.SetValue(camera.OffsetY.GetMin());
		}
		m_ulWidthMax = camera.Width.GetMax();
		m_ulHeightMax = camera.Height.GetMax();
		sprintf(g_dbgStr, "相机最大分辨率%lldx%lld\r\n", camera.Width.GetMax(), camera.Height.GetMax());
		//ShowTestInfo(g_dbgStr);
		m_bBaslerInit = TRUE;
		camera.Close();
	}catch (GenICam::GenericException &e)
	{
		sprintf(g_dbgStr, "Basler相机初始化失败 %s\r\n", e.GetDescription());
		//g_pMainDlg->ShowTestInfo(g_dbgStr);
		MessageBox(g_dbgStr);
		m_bBaslerInit = FALSE;
	}

	return m_bBaslerInit;
}

BOOL CCalibrationDlg::LedPannelInit()
{
	CString Command;
	BOOL bRt = FALSE;
	char strResult[100];
	Command = "LED-Panel-cmd.exe setmode RespTime";
	CRedirect *pRedirect = new CRedirect((LPTSTR)(LPCTSTR)Command, NULL, ".");
	do{
		pRedirect->m_pStrBuff = strResult;
		pRedirect->m_nBuffLen = sizeof(strResult);
		pRedirect->Run();

		pRedirect->m_szCommand = "LED-Panel-cmd.exe getmode";
		pRedirect->Run();
		int nValue = atoi(strResult);
		if(nValue != 1){
			OutputDebugString("LED set operation mode failed!");
			break;
		}

		pRedirect->m_szCommand = "LED-Panel-cmd.exe settrigger Single";
		pRedirect->Run();

		pRedirect->m_szCommand = "LED-Panel-cmd.exe gettrigger";
		pRedirect->Run();
		nValue = atoi(strResult);
		if(nValue != 2){
			OutputDebugString("LED set trigger mode failed!");
			break;
		}

		bRt = TRUE;
	}while(FALSE);

	if(pRedirect){
		delete pRedirect;
		pRedirect = NULL;
	}
	return bRt;
}

DWORD WINAPI StorageThread(LPVOID lpParam)
{
	// Exit code of the sample application.
	int exitCode = 0;
	int nWidth;
	int nHeight;
	int nSaveCount = 30;
	int nWaitTime = 20000;
	CString str;

	// Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
	// is initialized during the lifetime of this object.
	Pylon::PylonAutoInitTerm autoInitTerm;
	try
	{
		// Create the event handler.
		CEventHandler eventHandler;

		// Only look for cameras supported by Camera_t. 
		CDeviceInfo info;
		info.SetDeviceClass( Camera_t::DeviceClass());

		// Create an instant camera object with the first found camera device matching the specified device class.
		Camera_t camera( CTlFactory::GetInstance().CreateFirstDevice( info));

		// For demonstration purposes only, add sample configuration event handlers to print out information
		// about camera use and image grabbing.
		camera.RegisterConfiguration( new CConfigurationEventPrinter, RegistrationMode_Append, Cleanup_Delete); // Camera use.

		// Register the event handler.
		camera.RegisterImageEventHandler( &eventHandler, RegistrationMode_Append, Cleanup_None);
		camera.RegisterCameraEventHandler( &eventHandler, "EventExposureEndData", eMyExposureEndEvent, RegistrationMode_ReplaceAll, Cleanup_None);
		camera.RegisterCameraEventHandler( &eventHandler, "EventFrameStartOvertriggerData", eMyFrameStartOvertrigger, RegistrationMode_Append, Cleanup_None);

		// Camera event processing must be activated first, the default is off.
		camera.GrabCameraEvents = true;

		//sprintf(g_dbgStr, "使用设备: %s", camera.GetDeviceInfo().GetModelName().c_str());
		//g_pMainDlg->m_StaticInfo.SetWindowText(g_dbgStr);
		
		// Open the camera for setting parameters.
		camera.Open();
		camera.OffsetX = 0;
		camera.OffsetY = 0;
		camera.Width = g_pMainDlg->m_ulWidthMax;
		camera.Height = g_pMainDlg->m_ulHeightMax;

		int nOffsetX = GetPrivateProfileInt("BASLER", "OFFSETX", 0, theApp.m_szConfigFile);
		int nOffsetY = GetPrivateProfileInt("BASLER", "OFFSETY", 0, theApp.m_szConfigFile);
		nWidth = GetPrivateProfileInt("BASLER", "WIDTH", g_pMainDlg->m_ulWidthMax, theApp.m_szConfigFile);
		nHeight = GetPrivateProfileInt("BASLER", "HEIGHT", g_pMainDlg->m_ulHeightMax, theApp.m_szConfigFile);
#if 0
		camera.Width = nWidth;
		camera.Height = nHeight;
		camera.OffsetX = nOffsetX;
		camera.OffsetY = nOffsetY;
#endif

		camera.DeviceLinkThroughputLimitMode = DeviceLinkThroughputLimitMode_Off;
		camera.ExposureAuto.SetValue(ExposureAuto_Off);
		//camera.ExposureTime = 1000;
		int nFrameRate;
		if(g_pMainDlg->m_nTriggerFlag != -1){
			// 相机外部触发时设置帧率会导致捕获图像数减半
			nFrameRate = g_pMainDlg->m_waveFrequency;
			nSaveCount = g_pMainDlg->m_waveTimes;
			camera.AcquisitionFrameRateEnable = false;
			nWaitTime = nSaveCount*1000/nFrameRate+2000;		// 2000 表示预留的冗余时间

			nWidth = g_pMainDlg->m_ulWidthMax;
			nOffsetX = 0;
			switch(nFrameRate)
			{
			case 100:
				nHeight = g_pMainDlg->m_ulHeightMax;
				break;
			case 200:
				nHeight = 800;
				break;
			case 300:
				nHeight = 500;
				break;
			case 400:
				nHeight = 400;
				break;
			case 500:
				nHeight = 300;
				break;
			default:
				nHeight = g_pMainDlg->m_ulHeightMax;
				break;
			}
			nOffsetY = (g_pMainDlg->m_ulHeightMax - nHeight)/2;
			if(nOffsetY%2)
				nOffsetY -= 1;
			camera.Width = nWidth;
			camera.Height = nHeight;
			camera.OffsetX = nOffsetX;
			camera.OffsetY = nOffsetY;
			camera.ExposureTime = 1000*1000/nFrameRate*0.8;
		}else{
			nFrameRate = 100;
			camera.AcquisitionFrameRateEnable = true;
			camera.AcquisitionFrameRate = nFrameRate;
		}
		str.Format("使用设备: %s 捕获帧率:%d 捕获张数：%d\n", camera.GetDeviceInfo().GetModelName().c_str(), nFrameRate, nSaveCount);
		PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)str.GetBuffer(0), 0);
		
		camera.AcquisitionMode.SetValue( AcquisitionMode_Continuous );
		// Select the frame burst start trigger
		camera.TriggerSelector.SetValue(TriggerSelector_FrameBurstStart);
		// Set the mode for the selected trigger
		camera.TriggerMode.SetValue( TriggerMode_Off );
		// Set the source for the selected trigger
		camera.TriggerSource.SetValue ( TriggerSource_Line1 );

		camera.TriggerSelector.SetValue(TriggerSelector_FrameStart);
		if(g_pMainDlg->m_nTriggerFlag == 0 || g_pMainDlg->m_nTriggerFlag == 1)
			camera.TriggerMode.SetValue( TriggerMode_On);
		else
			camera.TriggerMode.SetValue( TriggerMode_Off);

		// Check if the device supports events.
		if ( !IsAvailable( camera.EventSelector))
		{
			throw RUNTIME_EXCEPTION( "The device doesn't support events.");
		}

		// Enable the sending of Exposure End events.
		// Select the event to be received.
		camera.EventSelector.SetValue(EventSelector_ExposureEnd);
		// Enable it.
		camera.EventNotification.SetValue(EventNotification_On);

		// Enable the sending of Frame Start Overtrigger events.
		if ( IsAvailable( camera.EventSelector.GetEntry(EventSelector_FrameStartOvertrigger)))
		{
			camera.EventSelector.SetValue(EventSelector_FrameStartOvertrigger);
			camera.EventNotification.SetValue(EventNotification_On);
		}

		// Start the grabbing of c_countOfImagesToGrab images.
		// The camera device is parameterized with a default configuration which
		// sets up free-running continuous acquisition.
		//camera.StartGrabbing( c_countOfImagesToGrab);
		camera.StartGrabbing(nSaveCount);

		if(g_pMainDlg->m_nTriggerFlag == 0 || g_pMainDlg->m_nTriggerFlag == 1){
			SetEvent( g_pMainDlg->m_hEventCapReady );
		}
		// This smart pointer will receive the grab result data.
		CGrabResultPtr ptrGrabResult;

		// Camera.StopGrabbing() is called automatically by the RetrieveResult() method
		// when c_countOfImagesToGrab images have been retrieved.
		while ( camera.IsGrabbing())
		{
			// Retrieve grab results and notify the camera event and image event handlers.
			camera.RetrieveResult( nWaitTime, ptrGrabResult, TimeoutHandling_ThrowException);
			// Nothing to do here with the grab result, the grab results are handled by the registered event handlers.
			if(!g_pMainDlg->m_bStorageThread)
				break;
		}

		// Disable the sending of Exposure End events.
		camera.EventSelector = EventSelector_ExposureEnd;
		camera.EventNotification.SetValue(EventNotification_Off);

		// Disable the sending of Frame Start Overtrigger events.
		if ( IsAvailable( camera.EventSelector.GetEntry(EventSelector_FrameStartOvertrigger)))
		{
			camera.EventSelector.SetValue(EventSelector_FrameStartOvertrigger);
			camera.EventNotification.SetValue(EventNotification_Off);
		}

		// Print the recorded log showing the timing of events and images.
		eventHandler.PrintLog();

		camera.Close();
	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		sprintf(g_dbgStr, "An exception occurred. %s\r\n", e.GetDescription());
		g_pMainDlg->ShowOutput(g_dbgStr);

		exitCode = 1;
	}

	// Comment the following two lines to disable waiting on exit.
	//cerr << endl << "Press Enter to exit." << endl;
	//while( cin.get() != '\n');
	g_pMainDlg->m_bStorageThread = FALSE;
	PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)"捕获存储线程退出！\r\n", 0);
	if(exitCode == 0)
	{
		int nCaseIndex = GetPrivateProfileInt("TRIGGERSET", "CASEINDEX", 0, theApp.m_szConfigFile);
		CPylonImageWindow imageWindows;
		BOOL bSaveROI = TRUE;
		int nROILeft;
		int nROIWidth;
		int nROITop;
		int nROIHeight;
		imageWindows.Create(0, 0, 0, 400, 400);
		str.Format("TRIGGERCASE%d", nCaseIndex);
		int nValue = GetPrivateProfileInt(str.GetBuffer(0), "SAVEROI", 0, theApp.m_szConfigFile);
		if(nValue){
			nROILeft = GetPrivateProfileInt(str.GetBuffer(0), "ROILEFT", 0, theApp.m_szConfigFile);
			nROIWidth = GetPrivateProfileInt(str.GetBuffer(0), "ROIWIDTH", nWidth, theApp.m_szConfigFile);
			nROITop = GetPrivateProfileInt(str.GetBuffer(0), "ROITOP", 0, theApp.m_szConfigFile);
			nROIHeight = GetPrivateProfileInt(str.GetBuffer(0), "ROIHEIGHT", nHeight, theApp.m_szConfigFile);
			if(nROILeft < 0)
				nROILeft = 0;
			if(nROILeft%2)					// 不知道为什么不能为奇数
				nROILeft -= 1;
			if(nROIWidth > nWidth)
				nROIWidth = nWidth;
			if(nROITop < 0)
				nROITop = 0;
			if(nROITop%2)
				nROITop -= 1;
			if(nROIHeight > nHeight)
				nROIHeight = nHeight;
			if(nROILeft+nROIWidth > nWidth || nROITop+nROIHeight > nHeight){
				bSaveROI = FALSE;
				g_pMainDlg->ShowOutput("ROI设置异常，不再保存ROI图片！\r\n");
			}
		}
		
		sprintf(g_dbgStr, "bSaveROI: %d nValue: %d", bSaveROI, nValue);
		OutputDebugString(g_dbgStr);

		SYSTEMTIME st;
		CString path;
		//GetSystemTime(&st);
		GetLocalTime(&st);
		if(g_pMainDlg->m_nTriggerFlag == -1){
			path.Format("\\测试_%02dH%02dM%02dS", st.wHour, st.wMinute, st.wSecond);
		}else{
			//path.Format("\\Circle%d\\%02dH%02dM%02dS", g_pMainDlg->m_nCurCircleIndex, st.wHour, st.wMinute, st.wSecond);
            path.Format("\\%02dH%02dM%02dS", st.wHour, st.wMinute, st.wSecond);
		}
#if 0
		path = theApp.m_imageDirectory+path;
#else
		CString strCaseName;
		g_pMainDlg->m_comboUsercase.GetLBText(g_pMainDlg->m_comboUsercase.GetCurSel(), strCaseName);
		path = g_pMainDlg->m_chUserCaseDir+strCaseName+path;
#endif

		int nCreatDir = 0;
		DWORD dwAttr = GetFileAttributes(path);
		if(dwAttr == 0xFFFFFFFF){						//若文件夹不存在，创建文件夹
			nCreatDir = CreateDirectory(path, NULL);
		}
		
		if(nCreatDir){
			//for(int i = 0; i < nSaveCount; i++){
			for(int i = 0; i < g_nCapIndex; i++){
				str.Format("正在保存第%d张图片，请等待...\r\n", i+1);
				PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)str.GetBuffer(0), 0);
				//sprintf(g_dbgStr, "%s\\%d_%I64u.png", theApp.m_imageDirectory, i, g_timeStamp[i]);
				sprintf(g_dbgStr, "%s\\%d_%I64u.png", path, i, (g_timeStamp[i]-g_timeStamp[0])/1000000);
				g_Images[i].Save(ImageFileFormat_Png, g_dbgStr);
				//imageWindows.SetImage(g_Images[i]);
				//imageWindows.Show();
				if(bSaveROI&&nValue){
					CPylonImage roiImage = g_Images[i].GetAoi(nROILeft, nROITop, nROIWidth, nROIHeight);
					//sprintf(g_dbgStr, "%s\\ROI%d.png", theApp.m_imageDirectory, i);
					sprintf(g_dbgStr, "%s\\ROI%d.png", path, i);
					roiImage.Save(ImageFileFormat_Png, g_dbgStr);
				}
			}
			PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)"图片保存完毕，请继续后续操作！\r\n", 0);
		}else{
			sprintf(g_dbgStr, "创建图片文件夹(%s)失败\r\n", path.GetBuffer(0));
			PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)g_dbgStr, 0);
		}
	}else{
        PostMessage(g_pMainDlg->m_hWnd, UPDATEINFO_MSG, (WPARAM)g_dbgStr, 0);
    }

	g_pMainDlg->m_bPicSave = FALSE;

	return 1;
}

void CCalibrationDlg::OnBnClickedBtnTest()
{
	// TODO: 在此添加控件通知处理程序代码
	m_nTriggerFlag = -1;
	startStorageThread();
}

BOOL CCalibrationDlg::startStorageThread()
{
	if(m_bStorageThread){
		MessageBox("捕获存储线程已启动，请触发相机！\r\n");
		return FALSE;
	}

	g_nCapIndex = 0;
	m_bStorageThread = TRUE;
	HANDLE  hThreadCaptureDisplay = CreateThread(NULL, 0, StorageThread, (LPVOID)this, 0, NULL);
	if(hThreadCaptureDisplay){
		ShowOutput("测量相机捕获线程已开启！\r\n");
		CloseHandle(hThreadCaptureDisplay);
		return TRUE;
	}else{
		m_bStorageThread = FALSE;
		return FALSE;
	}
}


void CCalibrationDlg::OnBnClickedBtnTrigger()
{
	// TODO: 在此添加控件通知处理程序代码
    CString str;
	int nCaseIndex = GetPrivateProfileInt("TRIGGERSET", "CASEINDEX", 0, theApp.m_szConfigFile)+1;
    BYTE type = 0;
    BYTE clkFrequency = 0;
    BYTE clkTimes = 1;
    if(nCaseIndex == TRIGGERMODE_S2S){
        type = 1;
        clkFrequency = GetPrivateProfileInt("ROBOT", "DBCLICKFREQ", 5, theApp.m_szConfigFile);
        clkTimes = GetPrivateProfileInt("ROBOT", "DBCLICKDURATION", 5, theApp.m_szConfigFile);
    }
    str.Format("TRIGGERCASE%d", nCaseIndex-1);
	BYTE triggerMode = GetPrivateProfileInt(str.GetBuffer(0), "TRIGGERMODE", 0, theApp.m_szConfigFile);
	BYTE filterLevel = GetPrivateProfileInt(str.GetBuffer(0), "FILTERLEVEL", 0, theApp.m_szConfigFile);
	BYTE delay = GetPrivateProfileInt(str.GetBuffer(0), "TIMEDELAY", 0, theApp.m_szConfigFile);
	m_waveFrequency = (GetPrivateProfileInt(str.GetBuffer(0), "WAVEFRQ", 0, theApp.m_szConfigFile)+1)*100;
	m_waveTimes = GetPrivateProfileInt(str.GetBuffer(0), "WAVECNT", 0, theApp.m_szConfigFile);
	if(nCaseIndex == TRIGGERMODE_L2P || nCaseIndex == TRIGGERMODE_VL || nCaseIndex == TRIGGERMODE_FPS){
		m_nTriggerFlag = 0;
		if(!startStorageThread()){
			MessageBox("启动相机捕获线程失败，请检查！");
			return;
		}
		DWORD dwRet = WaitForSingleObject( m_hEventCapReady, 3000 );
		if ( dwRet == WAIT_TIMEOUT ){
			MessageBox("测量相机互斥等待异常，照片保存可能会发生异常！");
		}
	}else{
		m_nTriggerFlag = -1;
	}
	CONTROL_Trigger(m_lLoginID, type, triggerMode, filterLevel, nCaseIndex, clkFrequency, delay, clkTimes, m_waveFrequency*10, m_waveTimes);
}


void CCalibrationDlg::OnBnClickedBtnMoveicon()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bMoveIcon = TRUE;
}


BOOL CCalibrationDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_bMoveIcon){
		SetCursor(AfxGetApp()->LoadCursor(IDC_GRAB));
		//SetCursor(LoadCursor(NULL, IDC_HAND));
		return TRUE;
	}
	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}


void CCalibrationDlg::OnBnClickedBtnNewcase()
{
    // TODO: Add your control notification handler code here
    if(!m_CurUserCase.IsEmpty()){
        MessageBox("请先完成当前测试用例");
        return;
    }

	CDlgNewCase newCaseDlg;
	if(IDOK == newCaseDlg.DoModal()){
        char chCreateTime[200] = {0};
        GetPrivateProfileString(newCaseDlg.m_UserCaseName, "CREATETIME", "", chCreateTime, sizeof(chCreateTime), theApp.m_szUserCaseFile);
        if(strlen(chCreateTime) > 0){
            MessageBox("当前名称测试用例已存在，请用其他名称新建测试用例");
            return;
        }

		m_TreeInit.DeleteAllItems();
		m_TreeCircle.DeleteAllItems();

		m_CurUserCase = newCaseDlg.m_UserCaseName;
		m_TreeInit.EnableWindow(m_RadioInit.GetCheck());
		m_TreeCircle.EnableWindow(!m_RadioInit.GetCheck());
		EnableItem(2, TRUE);
		EnableItem(3, FALSE);
		sprintf(g_dbgBuf, "创建新的测试用例【%s】！\r\n", m_CurUserCase.GetBuffer(0));
		ShowOutput(g_dbgBuf);
		CString str;
        str = m_chUserCaseDir+m_CurUserCase;
        DWORD dwAttr = GetFileAttributes(str);
        if(dwAttr == 0xFFFFFFFF){
            CreateDirectory(str, NULL);
        }
		m_nCircleTimes = 1;
	}
}


void CCalibrationDlg::OnBnClickedRadioInit()
{
    // TODO: Add your control notification handler code here
    if(m_RadioInit.GetCheck()){
        m_TreeInit.EnableWindow(TRUE);
        m_TreeCircle.EnableWindow(FALSE);
    }else{
        m_TreeInit.EnableWindow(FALSE);
        m_TreeCircle.EnableWindow(TRUE);
    }
}


void CCalibrationDlg::OnBnClickedRadioCircle()
{
    // TODO: Add your control notification handler code here
    if(m_RadioInit.GetCheck()){
        m_TreeInit.EnableWindow(TRUE);
        m_TreeCircle.EnableWindow(FALSE);
    }else{
        m_TreeInit.EnableWindow(FALSE);
        m_TreeCircle.EnableWindow(TRUE);
    }
}

void CCalibrationDlg::AddItem(int nType, HTREEITEM hCurTreeItem)
{
    CTreeCtrl *pTreeCtrl = NULL;
    if(m_RadioInit.GetCheck()){
        pTreeCtrl = &m_TreeInit;
    }else{
        pTreeCtrl = &m_TreeCircle;
    }
    ModuleData *pModuleData = NULL;
    pModuleData = (ModuleData*)malloc(sizeof(ModuleData));
    if(!pModuleData){
        MessageBox("分配模块数据内存失败");
        return;
    }
    memset(pModuleData, 0, sizeof(ModuleData));
    pModuleData->nType = nType;
    HTREEITEM hTreeItem = NULL;
    switch(nType)
    {
    case MODULE_INITSETTING:
        {
            ModuleInitSet *pInitSetData = (ModuleInitSet*)malloc(sizeof(ModuleInitSet));
            if(!pInitSetData){
                MessageBox("分配脚本执行数据内存块失败");
                break;
            }
            memset(pInitSetData, 0, sizeof(ModuleInitSet));
            strcpy(pModuleData->cName, "脚本执行");
            pModuleData->nSize = sizeof(ModuleInitSet);
            pModuleData->pData = pInitSetData;
            if(hCurTreeItem)
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1,NULL,hCurTreeItem);
            else
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1);
        }
        break;
    case MODULE_PM:
        {
            ModulePM *pPMData = (ModulePM*)malloc(sizeof(ModulePM));
            if(!pPMData){
                MessageBox("分配识别匹配数据内存块失败");
                break;
            }
            memset(pPMData, 0, sizeof(ModulePM));
            strcpy(pModuleData->cName, "识别匹配");
            pModuleData->nSize = sizeof(ModulePM);
            pModuleData->pData = pPMData;
            if(hCurTreeItem)
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1,NULL,hCurTreeItem);
            else
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1);
        }
        break;
    case MODULE_MOVE:
        {
            ModuleFingerMove *pFingerMoveData = (ModuleFingerMove*)malloc(sizeof(ModuleFingerMove));
            if(!pFingerMoveData){
                MessageBox("分配手指移动数据内存块失败");
                break;
            }
            memset(pFingerMoveData, 0, sizeof(ModuleFingerMove));
            strcpy(pModuleData->cName, "手指移动");
            pModuleData->nSize = sizeof(ModuleFingerMove);
            pModuleData->pData = pFingerMoveData;
            if(hCurTreeItem)
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1,NULL,hCurTreeItem);
            else
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1);
        }
        break;
    case MODULE_DRAG:
        {
            ModuleIconDrag *pIconDragData = (ModuleIconDrag*)malloc(sizeof(ModuleIconDrag));
            if(!pIconDragData){
                MessageBox("分配图标拖动数据内存块失败");
                break;
            }
            memset(pIconDragData, 0, sizeof(ModuleIconDrag));
            strcpy(pModuleData->cName, "图标拖动");
            pModuleData->nSize = sizeof(ModuleIconDrag);
            pModuleData->pData = pIconDragData;
			pIconDragData->nTimeBefore = pIconDragData->nTimeAfter = 10;
            if(hCurTreeItem)
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1,NULL,hCurTreeItem);
            else
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1);
        }
        break;
    case MODULE_TRIGGER:
        {
            ModuleTriggerMode *pTriggerModeData = (ModuleTriggerMode*)malloc(sizeof(ModuleTriggerMode));
            if(!pTriggerModeData){
                MessageBox("分配测量触发数据内存块失败");
                break;
            }
            memset(pTriggerModeData, 0, sizeof(ModuleTriggerMode));
            strcpy(pModuleData->cName, "测量触发");
            pModuleData->nSize = sizeof(ModuleTriggerMode);
            pModuleData->pData = pTriggerModeData;
            if(hCurTreeItem)
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1,NULL,hCurTreeItem);
            else
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1);
        }
        break;
    case MODULE_ROTATE:
        {
            ModuleRotate *pRotateData = (ModuleRotate *)malloc(sizeof(ModuleRotate));
            if(!pRotateData){
                MessageBox("分配腰部转动数据内存块失败");
                break;
            }
            memset(pRotateData, 0, sizeof(ModuleRotate));
            strcpy(pModuleData->cName, "腰部转动");
            pModuleData->nSize = sizeof(ModuleRotate);
            pModuleData->pData = pRotateData;
            if(hCurTreeItem)
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1,NULL,hCurTreeItem);
            else
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1);
        }
        break;
    case MODULE_DELAY:
        {
            ModuleDelay *pTimeDelay = (ModuleDelay *)malloc(sizeof(ModuleDelay));
            if(!pTimeDelay){
                MessageBox("分配时间延迟数据内存失败");
                break;
            }
            memset(pTimeDelay, 0, sizeof(ModuleDelay));
            strcpy(pModuleData->cName, "时间延迟");
            pModuleData->nSize = sizeof(ModuleDelay);
            pModuleData->pData = pTimeDelay;
            if(hCurTreeItem)
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1,NULL,hCurTreeItem);
            else
                hTreeItem = pTreeCtrl->InsertItem(pModuleData->cName,0,1);
        }
        break;
    default:
        break;
    }

    if(hTreeItem){
        pTreeCtrl->SetItemData( hTreeItem, (DWORD_PTR)pModuleData);
    }else{
        free(pModuleData);
    }
}


void CCalibrationDlg::OnNMRClickTreeInit(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: Add your control notification handler code here
    CMenu obMenu;
    obMenu.LoadMenu(IDR_MENU_POPUP);
    CMenu* pPopupMenu = NULL;
    if(m_pInitSetDlg || m_pModulePMDlg || m_pFingerMoveDlg || m_pIconDragDlg || m_pTriggerSetDlg || m_pRotateDlg || m_pCircleTimesDlg || m_pTimeDelayDlg)       // 正在进行参数设置不允许再进行其他菜单操作
        return;

    // Get the cursor position
    CPoint obCursorPoint = (0, 0);
    GetCursorPos(&obCursorPoint);
    /*if(0 >= m_List.GetSelectedCount()){
        pPopupMenu->EnableMenuItem(ID_LST_DELETE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED); 
    }else{
        pPopupMenu->EnableMenuItem(ID_LST_DELETE, MF_BYCOMMAND | MF_ENABLED);
    }*/

    CPoint pt;
    GetCursorPos(&pt);
    m_TreeInit.ScreenToClient(&pt);
	UINT nFlag = 0;
    HTREEITEM item = m_TreeInit.HitTest(pt, &nFlag);
    if(!item){
        m_TreeInit.SelectItem(NULL);
    }else{
        pPopupMenu = obMenu.GetSubMenu(0);
		m_TreeInit.SelectItem(item);
	}

    // Track the popup menu
    if(pPopupMenu)
        pPopupMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, obCursorPoint.x, obCursorPoint.y, this);

    *pResult = 0;
}


void CCalibrationDlg::OnBnClickedBtnInitsetting()
{
    // TODO: Add your control notification handler code here
    AddItem(MODULE_INITSETTING);
}


void CCalibrationDlg::OnBnClickedBtnDragicon()
{
    // TODO: Add your control notification handler code here
    AddItem(MODULE_DRAG);
}


void CCalibrationDlg::OnBnClickedBtnPm()
{
    // TODO: Add your control notification handler code here
    AddItem(MODULE_PM);
}


void CCalibrationDlg::OnBnClickedBtnFingermove()
{
    // TODO: Add your control notification handler code here
    AddItem(MODULE_MOVE);
}


void CCalibrationDlg::OnBnClickedBtnTrrigermode()
{
    // TODO: Add your control notification handler code here
    AddItem(MODULE_TRIGGER);
}

void CCalibrationDlg::OnBnClickedBtnRotate()
{
    // TODO: Add your control notification handler code here
    AddItem(MODULE_ROTATE);
}

void CCalibrationDlg::CaseSerialize(CString caseName)
{
	CString str;
	CString strCurrConfig;
	strCurrConfig.Format("%s%s\\config.ini", m_chUserCaseDir, caseName.GetBuffer(0));

	int nModuleCnt = 0;
	CString strModule;
	// 保存初始化列表各个功能模块参数
	ModuleData *pModuleData = NULL;
	HTREEITEM hItem = m_TreeInit.GetRootItem();
	while(hItem){
		pModuleData = (ModuleData*)m_TreeInit.GetItemData(hItem);
		if(pModuleData){
			strModule.Format("INITMODULE%d", nModuleCnt);
			str.Format("%d", pModuleData->nType);
			WritePrivateProfileString(strModule, "TYPE", str, strCurrConfig);
			WritePrivateProfileString(strModule, "MODULENAME", pModuleData->cName, strCurrConfig);
			switch(pModuleData->nType)
			{
			case MODULE_INITSETTING:
				{
					ModuleInitSet *pInitSet = (ModuleInitSet *)pModuleData->pData;
					if(pInitSet){
						WritePrivateProfileString(strModule, "SCRIPTFILE", pInitSet->cScriptFile, strCurrConfig);
					}
				}
				break;
			case MODULE_PM:
				{
					ModulePM *pPMData = (ModulePM *)pModuleData->pData;
					if(pPMData){
						WritePrivateProfileString(strModule, "PMTEMPLATE", pPMData->cPMTemplate, strCurrConfig);
					}
				}
				break;
			case MODULE_MOVE:
				{
					ModuleFingerMove *pFingerMove = (ModuleFingerMove *)pModuleData->pData;
					if(pFingerMove){
						str.Format("%d", pFingerMove->nFingerType);
						WritePrivateProfileString(strModule, "FINGERTYPE", str, strCurrConfig);
						str.Format("%d", pFingerMove->nMode);
						WritePrivateProfileString(strModule, "MODE", str, strCurrConfig);
						str.Format("%d", pFingerMove->nMoveToX);
						WritePrivateProfileString(strModule, "MOVETOX", str, strCurrConfig);
						str.Format("%d", pFingerMove->nMoveToY);
						WritePrivateProfileString(strModule, "MOVETOY", str, strCurrConfig);
						str.Format("%d", pFingerMove->nClick);
						WritePrivateProfileString(strModule, "CLICK", str, strCurrConfig);
						str.Format("%d", pFingerMove->nFrequency);
						WritePrivateProfileString(strModule, "FREQUENCY", str, strCurrConfig);
						str.Format("%d", pFingerMove->nTimes);
						WritePrivateProfileString(strModule, "TIMES", str, strCurrConfig);
					}
				}
				break;
			case MODULE_DRAG:
				{
					ModuleIconDrag *pIconDrag = (ModuleIconDrag *)pModuleData->pData;
					if(pIconDrag){
						str.Format("%d", pIconDrag->nMode);
						WritePrivateProfileString(strModule, "MODE", str, strCurrConfig);
						str.Format("%d", pIconDrag->nStartX);
						WritePrivateProfileString(strModule, "STARTX", str, strCurrConfig);
						str.Format("%d", pIconDrag->nStartY);
						WritePrivateProfileString(strModule, "STARTY", str, strCurrConfig);
						str.Format("%d", pIconDrag->nEndX);
						WritePrivateProfileString(strModule, "ENDX", str, strCurrConfig);
						str.Format("%d", pIconDrag->nEndY);
						WritePrivateProfileString(strModule, "ENDY", str, strCurrConfig);
						str.Format("%d", pIconDrag->nTimeBefore);
						WritePrivateProfileString(strModule, "TIMEBEFORE", str, strCurrConfig);
						str.Format("%d", pIconDrag->nTimeAfter);
						WritePrivateProfileString(strModule, "TIMEAFTER", str, strCurrConfig);
					}
				}
				break;
			case MODULE_TRIGGER:
				{
					ModuleTriggerMode *pTriggerMode = (ModuleTriggerMode *)pModuleData->pData;
					if(pTriggerMode){
						str.Format("%d", pTriggerMode->nTriggerIndex);
						WritePrivateProfileString(strModule, "TRIGGERINDEX", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nTriggerMode);
						WritePrivateProfileString(strModule, "TRIGGERMODE", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nTimeDelay);
						WritePrivateProfileString(strModule, "TIMEDELAY", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nWaveFrq);
						WritePrivateProfileString(strModule, "WAVEFRQ", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nWaveCnt);
						WritePrivateProfileString(strModule, "WAVECNT", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nDBClkFrq);
						WritePrivateProfileString(strModule, "DBCLKFRQ", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nDBClkCnt);
						WritePrivateProfileString(strModule, "DBCLKCNT", str, strCurrConfig);
					}
				}
				break;
			case MODULE_ROTATE:
				{
					ModuleRotate *pRotate = (ModuleRotate *)pModuleData->pData;
					if(pRotate){
						str.Format("%d", pRotate->nAntiWise);
						WritePrivateProfileString(strModule, "ANTIWISE", str, strCurrConfig);
						str.Format("%d", pRotate->nRotateAngle);
						WritePrivateProfileString(strModule, "ROTATEANGLE", str, strCurrConfig);
					}
				}
				break;
            case MODULE_DELAY:
                {
                    ModuleDelay *pTimeDelay = (ModuleDelay *)pModuleData->pData;
                    if(pTimeDelay){
                        str.Format("%d", pTimeDelay->nDelayTime);
                        WritePrivateProfileString(strModule, "TIMEDELAY", str, strCurrConfig);
                    }
                }
                break;
			default:
				break;
			}
			if(pModuleData->pData)
				free(pModuleData->pData);
			free(pModuleData);
		}

		hItem = m_TreeInit.GetNextSiblingItem(hItem);
		nModuleCnt++;
	}
	str.Format("%d", nModuleCnt);
	WritePrivateProfileString("COMMON", "INITMODULECNT", str.GetBuffer(0), strCurrConfig);

	nModuleCnt = 0;
	// 保存循环列表各个功能模块参数
	hItem = m_TreeCircle.GetRootItem();
	while(hItem){
		pModuleData = (ModuleData*)m_TreeCircle.GetItemData(hItem);
		if(pModuleData){
			strModule.Format("CIRCLEMODULE%d", nModuleCnt);
			str.Format("%d", pModuleData->nType);
			WritePrivateProfileString(strModule, "TYPE", str, strCurrConfig);
			WritePrivateProfileString(strModule, "MODULENAME", pModuleData->cName, strCurrConfig);
			switch(pModuleData->nType)
			{
			case MODULE_INITSETTING:
				{
					ModuleInitSet *pInitSet = (ModuleInitSet *)pModuleData->pData;
					if(pInitSet){
						WritePrivateProfileString(strModule, "SCRIPTFILE", pInitSet->cScriptFile, strCurrConfig);
					}
				}
				break;
			case MODULE_PM:
				{
					ModulePM *pPMData = (ModulePM *)pModuleData->pData;
					if(pPMData){
						WritePrivateProfileString(strModule, "PMTEMPLATE", pPMData->cPMTemplate, strCurrConfig);
					}
				}
				break;
			case MODULE_MOVE:
				{
					ModuleFingerMove *pFingerMove = (ModuleFingerMove *)pModuleData->pData;
					if(pFingerMove){
						str.Format("%d", pFingerMove->nFingerType);
						WritePrivateProfileString(strModule, "FINGERTYPE", str, strCurrConfig);
						str.Format("%d", pFingerMove->nMode);
						WritePrivateProfileString(strModule, "MODE", str, strCurrConfig);
						str.Format("%d", pFingerMove->nMoveToX);
						WritePrivateProfileString(strModule, "MOVETOX", str, strCurrConfig);
						str.Format("%d", pFingerMove->nMoveToY);
						WritePrivateProfileString(strModule, "MOVETOY", str, strCurrConfig);
						str.Format("%d", pFingerMove->nClick);
						WritePrivateProfileString(strModule, "CLICK", str, strCurrConfig);
						str.Format("%d", pFingerMove->nFrequency);
						WritePrivateProfileString(strModule, "FREQUENCY", str, strCurrConfig);
						str.Format("%d", pFingerMove->nTimes);
						WritePrivateProfileString(strModule, "TIMES", str, strCurrConfig);
					}
				}
				break;
			case MODULE_DRAG:
				{
					ModuleIconDrag *pIconDrag = (ModuleIconDrag *)pModuleData->pData;
					if(pIconDrag){
						str.Format("%d", pIconDrag->nMode);
						WritePrivateProfileString(strModule, "MODE", str, strCurrConfig);
						str.Format("%d", pIconDrag->nStartX);
						WritePrivateProfileString(strModule, "STARTX", str, strCurrConfig);
						str.Format("%d", pIconDrag->nStartY);
						WritePrivateProfileString(strModule, "STARTY", str, strCurrConfig);
						str.Format("%d", pIconDrag->nEndX);
						WritePrivateProfileString(strModule, "ENDX", str, strCurrConfig);
						str.Format("%d", pIconDrag->nEndY);
						WritePrivateProfileString(strModule, "ENDY", str, strCurrConfig);
						str.Format("%d", pIconDrag->nTimeBefore);
						WritePrivateProfileString(strModule, "TIMEBEFORE", str, strCurrConfig);
						str.Format("%d", pIconDrag->nTimeAfter);
						WritePrivateProfileString(strModule, "TIMEAFTER", str, strCurrConfig);
					}
				}
				break;
			case MODULE_TRIGGER:
				{
					ModuleTriggerMode *pTriggerMode = (ModuleTriggerMode *)pModuleData->pData;
					if(pTriggerMode){
						str.Format("%d", pTriggerMode->nTriggerIndex);
						WritePrivateProfileString(strModule, "TRIGGERINDEX", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nTriggerMode);
						WritePrivateProfileString(strModule, "TRIGGERMODE", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nTimeDelay);
						WritePrivateProfileString(strModule, "TIMEDELAY", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nWaveFrq);
						WritePrivateProfileString(strModule, "WAVEFRQ", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nWaveCnt);
						WritePrivateProfileString(strModule, "WAVECNT", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nDBClkFrq);
						WritePrivateProfileString(strModule, "DBCLKFRQ", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nDBClkCnt);
						WritePrivateProfileString(strModule, "DBCLKCNT", str, strCurrConfig);
					}
				}
				break;
			case MODULE_ROTATE:
				{
					ModuleRotate *pRotate = (ModuleRotate *)pModuleData->pData;
					if(pRotate){
						str.Format("%d", pRotate->nAntiWise);
						WritePrivateProfileString(strModule, "ANTIWISE", str, strCurrConfig);
						str.Format("%d", pRotate->nRotateAngle);
						WritePrivateProfileString(strModule, "ROTATEANGLE", str, strCurrConfig);
					}
				}
				break;
            case MODULE_DELAY:
                {
                    ModuleDelay *pTimeDelay = (ModuleDelay *)pModuleData->pData;
                    if(pTimeDelay){
                        str.Format("%d", pTimeDelay->nDelayTime);
                        WritePrivateProfileString(strModule, "TIMEDELAY", str, strCurrConfig);
                    }
                }
                break;
			default:
				break;
			}
			if(pModuleData->pData)
				free(pModuleData->pData);
			free(pModuleData);
		}

		hItem = m_TreeInit.GetNextSiblingItem(hItem);
		nModuleCnt++;
	}
	str.Format("%d", nModuleCnt);
	WritePrivateProfileString("COMMON", "CIRCLEMODULECNT", str.GetBuffer(0), strCurrConfig);
	str.Format("%d", m_nCircleTimes);
	WritePrivateProfileString("COMMON", "CIRCLETIMES", str.GetBuffer(0), strCurrConfig);
}

void CCalibrationDlg::OnBnClickedBtnComplete()
{
    // TODO: Add your control notification handler code here
	// 有效性检查，即检查当前测试用例模块等相关内容是否合理，待完善
	HTREEITEM hItem = m_TreeInit.GetRootItem();
	if(!hItem){
		if(IDYES != MessageBox("预设定列表为空确定创建？", "Info", MB_YESNO))
			return;
	}
	hItem = m_TreeCircle.GetRootItem();
	if(!hItem){
		if(IDYES != MessageBox("循环测试列表为空确定创建？", "Info", MB_YESNO))
			return;
	}

	// 向usercase.ini增加记录
	CString str;
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	str.Format("%04d%02d%02d%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	WritePrivateProfileString(m_CurUserCase.GetBuffer(0), "CREATETIME", str.GetBuffer(0), theApp.m_szUserCaseFile);

	CString strCurrConfig;
	strCurrConfig.Format("%s%s//config.ini", m_chUserCaseDir, m_CurUserCase.GetBuffer(0));
	
	int nModuleCnt = 0;
	CString strModule;
	// 保存初始化列表各个功能模块参数
	ModuleData *pModuleData = NULL;
	hItem = m_TreeInit.GetRootItem();
	while(hItem){
		pModuleData = (ModuleData*)m_TreeInit.GetItemData(hItem);
		if(pModuleData){
			strModule.Format("INITMODULE%d", nModuleCnt);
			str.Format("%d", pModuleData->nType);
			WritePrivateProfileString(strModule, "TYPE", str, strCurrConfig);
			WritePrivateProfileString(strModule, "MODULENAME", pModuleData->cName, strCurrConfig);
			switch(pModuleData->nType)
			{
			case MODULE_INITSETTING:
				{
					ModuleInitSet *pInitSet = (ModuleInitSet *)pModuleData->pData;
					if(pInitSet){
						WritePrivateProfileString(strModule, "SCRIPTFILE", pInitSet->cScriptFile, strCurrConfig);
					}
				}
				break;
			case MODULE_PM:
				{
					ModulePM *pPMData = (ModulePM *)pModuleData->pData;
					if(pPMData){
						WritePrivateProfileString(strModule, "PMTEMPLATE", pPMData->cPMTemplate, strCurrConfig);
					}
				}
				break;
			case MODULE_MOVE:
				{
					ModuleFingerMove *pFingerMove = (ModuleFingerMove *)pModuleData->pData;
					if(pFingerMove){
						str.Format("%d", pFingerMove->nFingerType);
						WritePrivateProfileString(strModule, "FINGERTYPE", str, strCurrConfig);
						str.Format("%d", pFingerMove->nMode);
						WritePrivateProfileString(strModule, "MODE", str, strCurrConfig);
						str.Format("%d", pFingerMove->nMoveToX);
						WritePrivateProfileString(strModule, "MOVETOX", str, strCurrConfig);
						str.Format("%d", pFingerMove->nMoveToY);
						WritePrivateProfileString(strModule, "MOVETOY", str, strCurrConfig);
						str.Format("%d", pFingerMove->nClick);
						WritePrivateProfileString(strModule, "CLICK", str, strCurrConfig);
						str.Format("%d", pFingerMove->nFrequency);
						WritePrivateProfileString(strModule, "FREQUENCY", str, strCurrConfig);
						str.Format("%d", pFingerMove->nTimes);
						WritePrivateProfileString(strModule, "TIMES", str, strCurrConfig);
					}
				}
				break;
			case MODULE_DRAG:
				{
					ModuleIconDrag *pIconDrag = (ModuleIconDrag *)pModuleData->pData;
					if(pIconDrag){
						str.Format("%d", pIconDrag->nMode);
						WritePrivateProfileString(strModule, "MODE", str, strCurrConfig);
						str.Format("%d", pIconDrag->nStartX);
						WritePrivateProfileString(strModule, "STARTX", str, strCurrConfig);
						str.Format("%d", pIconDrag->nStartY);
						WritePrivateProfileString(strModule, "STARTY", str, strCurrConfig);
						str.Format("%d", pIconDrag->nEndX);
						WritePrivateProfileString(strModule, "ENDX", str, strCurrConfig);
						str.Format("%d", pIconDrag->nEndY);
						WritePrivateProfileString(strModule, "ENDY", str, strCurrConfig);
						str.Format("%d", pIconDrag->nTimeBefore);
						WritePrivateProfileString(strModule, "TIMEBEFORE", str, strCurrConfig);
						str.Format("%d", pIconDrag->nTimeAfter);
						WritePrivateProfileString(strModule, "TIMEAFTER", str, strCurrConfig);
					}
				}
				break;
			case MODULE_TRIGGER:
				{
					ModuleTriggerMode *pTriggerMode = (ModuleTriggerMode *)pModuleData->pData;
					if(pTriggerMode){
						str.Format("%d", pTriggerMode->nTriggerIndex);
						WritePrivateProfileString(strModule, "TRIGGERINDEX", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nTriggerMode);
						WritePrivateProfileString(strModule, "TRIGGERMODE", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nTimeDelay);
						WritePrivateProfileString(strModule, "TIMEDELAY", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nWaveFrq);
						WritePrivateProfileString(strModule, "WAVEFRQ", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nWaveCnt);
						WritePrivateProfileString(strModule, "WAVECNT", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nDBClkFrq);
						WritePrivateProfileString(strModule, "DBCLKFRQ", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nDBClkCnt);
						WritePrivateProfileString(strModule, "DBCLKCNT", str, strCurrConfig);
					}
				}
				break;
			case MODULE_ROTATE:
				{
					ModuleRotate *pRotate = (ModuleRotate *)pModuleData->pData;
					if(pRotate){
						str.Format("%d", pRotate->nAntiWise);
						WritePrivateProfileString(strModule, "ANTIWISE", str, strCurrConfig);
						str.Format("%d", pRotate->nRotateAngle);
						WritePrivateProfileString(strModule, "ROTATEANGLE", str, strCurrConfig);
					}
				}
				break;
            case MODULE_DELAY:
                {
                    ModuleDelay *pTimeDelay = (ModuleDelay *)pModuleData->pData;
                    if(pTimeDelay){
                        str.Format("%d", pTimeDelay->nDelayTime);
                        WritePrivateProfileString(strModule, "TIMEDELAY", str, strCurrConfig);
                    }
                }
                break;
			default:
				break;
			}
			if(pModuleData->pData)
				free(pModuleData->pData);
			free(pModuleData);
		}

		hItem = m_TreeInit.GetNextSiblingItem(hItem);
		nModuleCnt++;
	}
	str.Format("%d", nModuleCnt);
	WritePrivateProfileString("COMMON", "INITMODULECNT", str.GetBuffer(0), strCurrConfig);

	nModuleCnt = 0;
	// 保存循环列表各个功能模块参数
	hItem = m_TreeCircle.GetRootItem();
	while(hItem){
		pModuleData = (ModuleData*)m_TreeCircle.GetItemData(hItem);
		if(pModuleData){
			strModule.Format("CIRCLEMODULE%d", nModuleCnt);
			str.Format("%d", pModuleData->nType);
			WritePrivateProfileString(strModule, "TYPE", str, strCurrConfig);
			WritePrivateProfileString(strModule, "MODULENAME", pModuleData->cName, strCurrConfig);
			switch(pModuleData->nType)
			{
			case MODULE_INITSETTING:
				{
					ModuleInitSet *pInitSet = (ModuleInitSet *)pModuleData->pData;
					if(pInitSet){
						WritePrivateProfileString(strModule, "SCRIPTFILE", pInitSet->cScriptFile, strCurrConfig);
					}
				}
				break;
			case MODULE_PM:
				{
					ModulePM *pPMData = (ModulePM *)pModuleData->pData;
					if(pPMData){
						WritePrivateProfileString(strModule, "PMTEMPLATE", pPMData->cPMTemplate, strCurrConfig);
					}
				}
				break;
			case MODULE_MOVE:
				{
					ModuleFingerMove *pFingerMove = (ModuleFingerMove *)pModuleData->pData;
					if(pFingerMove){
						str.Format("%d", pFingerMove->nFingerType);
						WritePrivateProfileString(strModule, "FINGERTYPE", str, strCurrConfig);
						str.Format("%d", pFingerMove->nMode);
						WritePrivateProfileString(strModule, "MODE", str, strCurrConfig);
						str.Format("%d", pFingerMove->nMoveToX);
						WritePrivateProfileString(strModule, "MOVETOX", str, strCurrConfig);
						str.Format("%d", pFingerMove->nMoveToY);
						WritePrivateProfileString(strModule, "MOVETOY", str, strCurrConfig);
						str.Format("%d", pFingerMove->nClick);
						WritePrivateProfileString(strModule, "CLICK", str, strCurrConfig);
						str.Format("%d", pFingerMove->nFrequency);
						WritePrivateProfileString(strModule, "FREQUENCY", str, strCurrConfig);
						str.Format("%d", pFingerMove->nTimes);
						WritePrivateProfileString(strModule, "TIMES", str, strCurrConfig);
					}
				}
				break;
			case MODULE_DRAG:
				{
					ModuleIconDrag *pIconDrag = (ModuleIconDrag *)pModuleData->pData;
					if(pIconDrag){
						str.Format("%d", pIconDrag->nMode);
						WritePrivateProfileString(strModule, "MODE", str, strCurrConfig);
						str.Format("%d", pIconDrag->nStartX);
						WritePrivateProfileString(strModule, "STARTX", str, strCurrConfig);
						str.Format("%d", pIconDrag->nStartY);
						WritePrivateProfileString(strModule, "STARTY", str, strCurrConfig);
						str.Format("%d", pIconDrag->nEndX);
						WritePrivateProfileString(strModule, "ENDX", str, strCurrConfig);
						str.Format("%d", pIconDrag->nEndY);
						WritePrivateProfileString(strModule, "ENDY", str, strCurrConfig);
						str.Format("%d", pIconDrag->nTimeBefore);
						WritePrivateProfileString(strModule, "TIMEBEFORE", str, strCurrConfig);
						str.Format("%d", pIconDrag->nTimeAfter);
						WritePrivateProfileString(strModule, "TIMEAFTER", str, strCurrConfig);
					}
				}
				break;
			case MODULE_TRIGGER:
				{
					ModuleTriggerMode *pTriggerMode = (ModuleTriggerMode *)pModuleData->pData;
					if(pTriggerMode){
						str.Format("%d", pTriggerMode->nTriggerIndex);
						WritePrivateProfileString(strModule, "TRIGGERINDEX", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nTriggerMode);
						WritePrivateProfileString(strModule, "TRIGGERMODE", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nTimeDelay);
						WritePrivateProfileString(strModule, "TIMEDELAY", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nWaveFrq);
						WritePrivateProfileString(strModule, "WAVEFRQ", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nWaveCnt);
						WritePrivateProfileString(strModule, "WAVECNT", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nDBClkFrq);
						WritePrivateProfileString(strModule, "DBCLKFRQ", str, strCurrConfig);
						str.Format("%d", pTriggerMode->nDBClkCnt);
						WritePrivateProfileString(strModule, "DBCLKCNT", str, strCurrConfig);
					}
				}
				break;
			case MODULE_ROTATE:
				{
					ModuleRotate *pRotate = (ModuleRotate *)pModuleData->pData;
					if(pRotate){
						str.Format("%d", pRotate->nAntiWise);
						WritePrivateProfileString(strModule, "ANTIWISE", str, strCurrConfig);
						str.Format("%d", pRotate->nRotateAngle);
						WritePrivateProfileString(strModule, "ROTATEANGLE", str, strCurrConfig);
					}
				}
				break;
            case MODULE_DELAY:
                {
                    ModuleDelay *pTimeDelay = (ModuleDelay *)pModuleData->pData;
                    if(pTimeDelay){
                        str.Format("%d", pTimeDelay->nDelayTime);
                        WritePrivateProfileString(strModule, "TIMEDELAY", str, strCurrConfig);
                    }
                }
                break;
			default:
				break;
			}
			if(pModuleData->pData)
				free(pModuleData->pData);
			free(pModuleData);
		}

		hItem = m_TreeInit.GetNextSiblingItem(hItem);
		nModuleCnt++;
	}
	str.Format("%d", nModuleCnt);
	WritePrivateProfileString("COMMON", "CIRCLEMODULECNT", str.GetBuffer(0), strCurrConfig);
	str.Format("%d", m_nCircleTimes);
	WritePrivateProfileString("COMMON", "CIRCLETIMES", str.GetBuffer(0), strCurrConfig);

	//m_TreeInit.DeleteAllItems();
	//m_TreeCircle.DeleteAllItems();

	m_comboUsercase.InsertString(theApp.m_nUserCaseCount++, m_CurUserCase.GetBuffer(0));
	m_comboUsercase.SetCurSel(theApp.m_nUserCaseCount-1);
	sprintf(g_dbgBuf, "测试用例【%s】创建完成！\r\n", m_CurUserCase.GetBuffer(0));
	ShowOutput(g_dbgBuf);

    m_TreeInit.EnableWindow(FALSE);
    m_TreeCircle.EnableWindow(FALSE);
    EnableItem(2, FALSE);
	EnableItem(3, TRUE);
	m_CurUserCase = "";
}


void CCalibrationDlg::OnTreeCtrlRename()
{
	// TODO: 在此添加命令处理程序代码
	if(m_TreeInit.IsWindowEnabled()){
		m_TreeInit.EditLabel(m_TreeInit.GetSelectedItem());
	}else if(m_TreeCircle.IsWindowEnabled()){
		m_TreeCircle.EditLabel(m_TreeCircle.GetSelectedItem());
	}
}


void CCalibrationDlg::OnNMRClickTreeCircle(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	CMenu obMenu;
    obMenu.LoadMenu(IDR_MENU_POPUP);
    CMenu* pPopupMenu = NULL;

    if(m_pInitSetDlg || m_pModulePMDlg || m_pFingerMoveDlg || m_pIconDragDlg || m_pTriggerSetDlg || m_pRotateDlg || m_pCircleTimesDlg || m_pTimeDelayDlg)
        return;

    // Get the cursor position
    CPoint obCursorPoint = (0, 0);
    GetCursorPos(&obCursorPoint);

    CPoint pt;
    GetCursorPos(&pt);
    m_TreeCircle.ScreenToClient(&pt);
	UINT nFlag = 0;
    HTREEITEM item = m_TreeCircle.HitTest(pt, &nFlag);
    if(!item || nFlag&TVHT_ONITEMRIGHT){
        m_TreeCircle.SelectItem(NULL);
        pPopupMenu = obMenu.GetSubMenu(1);
    }else{
        pPopupMenu = obMenu.GetSubMenu(0);
		m_TreeCircle.SelectItem(item);
	}

    // Track the popup menu
    if(pPopupMenu)
        pPopupMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, obCursorPoint.x, obCursorPoint.y, this);

    *pResult = 0;
}


void CCalibrationDlg::OnDeleteTreeItem()
{
    // TODO: Add your command handler code here
    CString str;
    ModuleData *pModuleData = NULL;
	CTreeCtrl *pTreeCtrl = NULL;
	HTREEITEM hItem = NULL;
    if(m_TreeInit.IsWindowEnabled()){
        //m_TreeInit.EditLabel(m_TreeInit.GetSelectedItem());
		pTreeCtrl = &m_TreeInit;
        hItem = m_TreeInit.GetSelectedItem();
    }else if(m_TreeCircle.IsWindowEnabled()){
        pTreeCtrl = &m_TreeCircle;
		hItem = m_TreeCircle.GetSelectedItem();
    }

	if(hItem){
		CString name = pTreeCtrl->GetItemText(hItem);
		str.Format("是否删除%s？", name.GetBuffer(0));
		if(MessageBox(str, "Info", MB_YESNO) == IDYES){
			pModuleData = (ModuleData*)pTreeCtrl->GetItemData(hItem);
			if(pModuleData){
				if(pModuleData->pData)
					free(pModuleData->pData);
				free(pModuleData);
			}
			pTreeCtrl->DeleteItem(hItem);
		}
	}
}


void CCalibrationDlg::OnSetTreeItem()
{
    // TODO: Add your command handler code here
    ModuleData *pModuleData = NULL;
    HTREEITEM hItem = NULL;
    if(m_TreeInit.IsWindowEnabled()){
        hItem = m_TreeInit.GetSelectedItem();
    }else if(m_TreeCircle.IsWindowEnabled()){
        hItem = m_TreeCircle.GetSelectedItem();
    }

    if(hItem){
        pModuleData = (ModuleData *)m_TreeInit.GetItemData(hItem);
        if(!pModuleData)
            return;
        switch(pModuleData->nType)
        {
        case MODULE_INITSETTING:
            {
#if 1
                if(!m_pInitSetDlg){
                    m_pInitSetDlg = new CDlgInitSetting(this);
                    m_pInitSetDlg->m_pModuleData = pModuleData;
                    m_pInitSetDlg->Create(IDD_DLG_INITSETTING, this);
                    m_pInitSetDlg->ShowWindow(SW_SHOW);
                }else{
                    m_pInitSetDlg->SetActiveWindow();
                }
#else
                CDlgInitSetting initSetDlg;
                initSetDlg.m_pMainDlg = this;
                initSetDlg.DoModal();
#endif
            }
            break;
        case MODULE_PM:
            if(!m_pModulePMDlg){
                m_pModulePMDlg = new CDlgModulePM(this);
                m_pModulePMDlg->m_pModuleData = pModuleData;
                m_pModulePMDlg->Create(IDD_DLG_MODULEPM, this);
                m_pModulePMDlg->ShowWindow(SW_SHOW);
            }else{
                m_pModulePMDlg->SetActiveWindow();
            }
            break;
        case MODULE_MOVE:
            if(!m_pFingerMoveDlg){
                m_pFingerMoveDlg = new CDlgFingerMove(this);
                m_pFingerMoveDlg->m_pModuleData = pModuleData;
                m_pFingerMoveDlg->Create(IDD_DLG_FINGERMOVE, this);
                m_pFingerMoveDlg->ShowWindow(SW_SHOW);
            }else{
                m_pFingerMoveDlg->SetActiveWindow();
            }
            break;
        case MODULE_DRAG:
            if(!m_pIconDragDlg){
                m_pIconDragDlg = new CDlgIconDrag(this);
                m_pIconDragDlg->m_pModuleData = pModuleData;
                m_pIconDragDlg->Create(IDD_DLG_ICONDRAG, this);
                m_pIconDragDlg->ShowWindow(SW_SHOW);
            }else{
                m_pIconDragDlg->SetActiveWindow();
            }
            break;
        case MODULE_TRIGGER:
            if(!m_pTriggerSetDlg){
                m_pTriggerSetDlg = new CDlgTriggerSet(this);
                m_pTriggerSetDlg->m_pModuleData = pModuleData;
                m_pTriggerSetDlg->Create(IDD_DLG_TRIGGERSET, this);
                m_pTriggerSetDlg->ShowWindow(SW_SHOW);
            }else{
                m_pTriggerSetDlg->SetActiveWindow();
            }
            break;
        case MODULE_ROTATE:
            if(!m_pRotateDlg){
                m_pRotateDlg = new CDlgRotate(this);
                m_pRotateDlg->m_pModuleData = pModuleData;
                m_pRotateDlg->Create(IDD_DLG_ROTATE, this);
                m_pRotateDlg->ShowWindow(SW_SHOW);
            }else{
                m_pRotateDlg->SetActiveWindow();
            }
            break;
        case MODULE_DELAY:
            if(!m_pTimeDelayDlg){
                m_pTimeDelayDlg = new CDlgTimeDelay(this);
                m_pTimeDelayDlg->m_pModuleData = pModuleData;
                m_pTimeDelayDlg->Create(IDD_DLG_TIMEDELAY, this);
                m_pTimeDelayDlg->ShowWindow(SW_SHOW);
            }else{
                m_pTimeDelayDlg->SetActiveWindow();
            }
        default:
            break;
        }
    }
}

LRESULT CCalibrationDlg::ShowOutput(char *info)
{
    CString strLoad;
    //m_RichEditOutput.SetBackgroundColor(TRUE, RGB(255, 255, 255));
    CString str(info);
#if 1
    int cntStr = m_RichEditOutput.GetWindowTextLength();  
    m_RichEditOutput.SetSel(cntStr,cntStr);
    m_RichEditOutput.ReplaceSel(str);
    m_RichEditOutput.HideSelection(FALSE, FALSE);
#else
    CString strCurr;
    m_RichEditOutput.GetWindowText(strCurr);
    strCurr+=str;
    m_RichEditOutput.SetWindowText(strCurr);
#endif

    return 1;
}

void CCalibrationDlg::OnNMDblclkTreeInit(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: Add your control notification handler code here
    HTREEITEM hItem = m_TreeInit.GetSelectedItem();	
    if (hItem){
        OnSetTreeItem();
    }
    *pResult = 0;
}


void CCalibrationDlg::OnNMDblclkTreeCircle(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: Add your control notification handler code here
    HTREEITEM hItem = m_TreeCircle.GetSelectedItem();	
    if (hItem){
        OnSetTreeItem();
    }
    *pResult = 0;
}



void CCalibrationDlg::OnSetCircleTimes()
{
	// TODO: 在此添加命令处理程序代码
	if(!m_pCircleTimesDlg){
		m_pCircleTimesDlg = new CDlgCircleTimes(this);
		m_pCircleTimesDlg->m_nTimes = m_nCircleTimes;
		m_pCircleTimesDlg->Create(IDD_DLG_CIRCLETIMES, this);
		m_pCircleTimesDlg->ShowWindow(SW_SHOW);
	}else{
		m_pCircleTimesDlg->SetActiveWindow();
	}
}


void CCalibrationDlg::OnBnClickedBtnCasedetail()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	
	char chTemp[MAX_PATH];
	if(m_bIsCaseEdit){
		MessageBox("测试用例编辑中！");
		return;
	}

	HTREEITEM  hTreeItem = NULL;
	ModuleData *pModuleData = NULL;
	if(m_comboUsercase.GetCount() > 0){
        m_comboUsercase.GetLBText(m_comboUsercase.GetCurSel(), m_CurUserCase);
        str = m_chUserCaseDir+m_CurUserCase;
        str += "\\config.ini";

		m_TreeInit.DeleteAllItems();
		m_TreeCircle.DeleteAllItems();

		EnableItem(2, TRUE);

		CString strModule;
		int nModuleCnt = 0;
		int nModuleType;
		char chModuleName[200];
		nModuleCnt = GetPrivateProfileInt("COMMON", "INITMODULECNT", 0, str);
		if(nModuleCnt > 0){
			for(int i = 0; i < nModuleCnt; i++){
				strModule.Format("INITMODULE%d", i);
				nModuleType = GetPrivateProfileInt(strModule, "TYPE", MODULE_INITSETTING, str);
				GetPrivateProfileString(strModule, "MODULENAME", "NONE", chModuleName, sizeof(chModuleName), str);
				pModuleData = (ModuleData*)malloc(sizeof(ModuleData));
				if(!pModuleData){
					MessageBox("分配模块数据内存失败");
					goto ErrProc;
				}
				hTreeItem = m_TreeInit.InsertItem(chModuleName,0,1);
				m_TreeInit.SetItemData(hTreeItem, (DWORD_PTR)pModuleData);
				memset(pModuleData, 0, sizeof(ModuleData));
				pModuleData->nType = nModuleType;
				strcpy(pModuleData->cName, chModuleName);
				if(!LoadModule(nModuleType, strModule.GetBuffer(0), pModuleData, str))
					goto ErrProc;
			}
		}

		nModuleCnt = GetPrivateProfileInt("COMMON", "CIRCLEMODULECNT", 0, str.GetBuffer(0));
		if(nModuleCnt > 0){
			m_nCircleTimes = GetPrivateProfileInt("COMMON", "CIRCLETIMES", 1, str);
			for(int i = 0; i < nModuleCnt; i++){
				strModule.Format("CIRCLEMODULE%d", i);
				nModuleType = GetPrivateProfileInt(strModule, "TYPE", MODULE_INITSETTING, str.GetBuffer(0));
				GetPrivateProfileString(strModule, "MODULENAME", "NONE", chModuleName, sizeof(chModuleName), str.GetBuffer(0));
				pModuleData = (ModuleData*)malloc(sizeof(ModuleData));
				if(!pModuleData){
					MessageBox("分配模块数据内存失败");
					goto ErrProc;
				}
				hTreeItem = m_TreeCircle.InsertItem(chModuleName,0,1);
				m_TreeCircle.SetItemData( hTreeItem, (DWORD_PTR)pModuleData);
				memset(pModuleData, 0, sizeof(ModuleData));
				pModuleData->nType = nModuleType;
				strcpy(pModuleData->cName, chModuleName);
				if(!LoadModule(nModuleType, strModule.GetBuffer(0), pModuleData, str))
					goto ErrProc;
			}
		}

		
		m_TreeInit.EnableWindow(m_RadioInit.GetCheck());
		m_TreeCircle.EnableWindow(!m_RadioInit.GetCheck());
		GetDlgItem(IDC_BTN_NEWCASE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_RUNCASE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_CASEDETAIL)->EnableWindow(FALSE);
        GetDlgItem(IDC_BNT_CANCLENEW)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_COMPLETE)->EnableWindow(FALSE);
		m_comboUsercase.EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_STOPCASE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_CASECOMPLETE)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_CANCLEEDIT)->EnableWindow(TRUE);
		m_bIsCaseEdit = TRUE;
	}

	return;

ErrProc:
	hTreeItem = m_TreeInit.GetRootItem();
	while(hTreeItem){
		pModuleData = (ModuleData*)m_TreeInit.GetItemData(hTreeItem);
		if(pModuleData){
			if(pModuleData->pData)
				free(pModuleData->pData);
			free(pModuleData);
		}
	}

	// 保存循环列表各个功能模块参数
	hTreeItem = m_TreeCircle.GetRootItem();
	while(hTreeItem){
		pModuleData = (ModuleData*)m_TreeCircle.GetItemData(hTreeItem);
		if(pModuleData){
			if(pModuleData->pData)
				free(pModuleData->pData);
			free(pModuleData);
		}
	}

	m_TreeInit.DeleteAllItems();
	m_TreeCircle.DeleteAllItems();
	GetDlgItem(IDC_BTN_NEWCASE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_RUNCASE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_CASEDETAIL)->EnableWindow(TRUE);
	m_comboUsercase.EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_CASECOMPLETE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_CANCLEEDIT)->EnableWindow(FALSE);
}


void CCalibrationDlg::OnBnClickedBtnDelcase()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_bIsCaseEdit){
		MessageBox("用例正在编辑中，不能删除！");
		return;
	}

	CString str;
	m_comboUsercase.GetLBText(m_comboUsercase.GetCurSel(), str);
	WritePrivateProfileString(str, NULL, NULL, theApp.m_szUserCaseFile);
	str = m_chUserCaseDir+str;
	theApp.DeleteDirectory(str.GetBuffer(0));
	m_comboUsercase.DeleteString(m_comboUsercase.GetCurSel());
	m_comboUsercase.SetCurSel(0);
	theApp.m_nUserCaseCount--;
}

BOOL CCalibrationDlg::RunModule(int nModuleType, char *strModule, char *chModuleName, CString strConfig)
{
	char chFilePath[MAX_PATH] = { 0 };
	int nStatus;
    int nFailedTimes = 0;
    LONG lPosX, lPosY, lPosZ;
    
	switch(nModuleType)
	{
	case MODULE_INITSETTING:
		{
			GetPrivateProfileString(strModule, "SCRIPTFILE", "", chFilePath, sizeof(chFilePath), strConfig);
			BOOL bRt = FALSE;
			if(strlen(chFilePath) > 0){
				CString cmd;
				cmd.Format("sh %s", chFilePath);
				CRedirect *pRedirect = new CRedirect((LPTSTR)(LPCTSTR)cmd, NULL, "D:\\ZWW\\PnP_E2E\\");
				do{
					pRedirect->m_pStrBuff = g_strResult;
					pRedirect->m_nBuffLen = sizeof(g_strResult);
					pRedirect->Run();
					bRt = TRUE;
				}while(FALSE);
				delete pRedirect;
			}else{
				MessageBox("初始化脚本未指定，请检查！");
				goto RunFailed;
			}
			sprintf(g_dbgBuf, "%s模块执行完成，结果：\r\n", chModuleName);
			ShowOutput(g_dbgBuf);
			ShowOutput(g_strResult);
			if(strstr(g_strResult, "success")){
				;
			}else{
				;
			}
		}
		break;
	case MODULE_PM:
		{
			GetPrivateProfileString(strModule, "PMTEMPLATE", "", chFilePath, sizeof(chFilePath), strConfig);
			if(strlen(chFilePath) > 0){
				if(!TemplatePM(chFilePath, m_lPMPosX, m_lPMPosY)){
					goto RunFailed;
                }
			}else{
				MessageBox("模式匹配文件未指定，请检查！");
				goto RunFailed;
			}
		}
		break;
	case MODULE_MOVE:
		{
			LONG lMoveToX, lMoveToY;
			int nFrequency = 1;
			int nTimes = 1;
			int nFingerType = GetPrivateProfileInt(strModule, "FINGERTYPE", 0, strConfig);
			int nMode = GetPrivateProfileInt(strModule, "MODE", 0, strConfig);
			int nClick = GetPrivateProfileInt(strModule, "CLICK", 0, strConfig);
			if(nFingerType == 1){					// 快击手指
				if(nMode == 0){						// 
					lMoveToX = m_lPMPosX;
					lMoveToY = m_lPMPosY;
					lMoveToX = lMoveToX - m_lQuickClickLaserShiftX;					// 模式匹配结果已转化为十字光标坐标
					lMoveToY = lMoveToY - m_lQuickClickLaserShiftY;
				}else{
					lMoveToX = GetPrivateProfileInt(strModule, "MOVETOX", -1, strConfig);
					lMoveToY = GetPrivateProfileInt(strModule, "MOVETOY", -1, strConfig);
				}
				if(lMoveToX < 0 || lMoveToY < 0){
					MessageBox("快击手指移动目标位置未设置，请检查！");
					goto RunFailed;
				}
				nFrequency = GetPrivateProfileInt(strModule, "FREQUENCY", 1, strConfig);
				nTimes = GetPrivateProfileInt(strModule, "TIMES", 1, strConfig);
			}else{									// 慢击手指
				if(nMode == 0){						// 
					lMoveToX = m_lPMPosX;
					lMoveToY = m_lPMPosY;
					lMoveToX = lMoveToX - m_lFingerLaserShiftX;
					lMoveToY = lMoveToY - m_lFingerLaserShiftY;
				}else{
					lMoveToX = GetPrivateProfileInt(strModule, "MOVETOX", -1, strConfig);
					lMoveToY = GetPrivateProfileInt(strModule, "MOVETOY", -1, strConfig);
				}
				if(lMoveToX < 0 || lMoveToY<0){
					MessageBox("慢击手指移动目标位置未设置，请检查！");
					goto RunFailed;
				}
			}
			CONTROL_MoveTo(m_lLoginID, lMoveToX, lMoveToY, g_pMainDlg->m_lCurPosZ);
			// 是否移动到指定位置
			if(!WaitForPosition(lMoveToX, lMoveToY, nStatus)){
				MessageBox("手指移动到目标位置异常，请检查！");
				goto RunFailed;
			}
			if(nClick){
				int nCaseIndex = 0;
				BYTE triggerMode = 0;
				BYTE step = GetPrivateProfileInt("ROBOT", "CLICKSPEED", 5, theApp.m_szConfigFile);;
				BYTE delay = 0;
				//CONTROL_ClickEx(m_lLoginID, nFingerType, triggerMode, step, nCaseIndex, 0, delay, 1,  nFrequency, nTimes);
                CONTROL_Trigger(m_lLoginID, nFingerType, triggerMode, step, nCaseIndex, nFrequency, delay, nTimes, 0, 0);
			}
		}
		break;
	case MODULE_DRAG:
		{
			int nMode;                      // 0 根据识别匹配结果确定图标初始位置 2 指定初始位置
			LONG lStartX, lStartY, lEndX, lEndY;
			nMode = GetPrivateProfileInt(strModule, "MODE", 0, strConfig);
			if(nMode == 0){
				lStartX = m_lPMPosX;
				lStartY = m_lPMPosY;
				lStartX = lStartX - m_lFingerLaserShiftX;
				lStartY = lStartY - m_lFingerLaserShiftY;
			}else{
				lStartX = GetPrivateProfileInt(strModule, "STARTX", -1, strConfig);
				lStartY = GetPrivateProfileInt(strModule, "STARTY", -1, strConfig);
			}
			if(lStartX < 0 || lStartY < 0){
				MessageBox("图标拖动起始位置异常，请检查！");
				goto RunFailed;
			}
			lEndX = GetPrivateProfileInt(strModule, "ENDX", -1, strConfig);
			lEndY = GetPrivateProfileInt(strModule, "ENDY", -1, strConfig);
			if(lStartX < 0 || lStartY < 0){
				MessageBox("图标拖动结束位置异常，请检查！");
				goto RunFailed;
			}
			BYTE timeBefore = GetPrivateProfileInt(strModule, "TIMEBEFORE", 10, theApp.m_szConfigFile);
			BYTE timeAfter = GetPrivateProfileInt(strModule, "TIMEAFTER", 10, theApp.m_szConfigFile);
			int nSpeed = 5;
			CONTROL_Slide(m_lLoginID, lStartX, lStartY, lEndX, lEndY, timeBefore, timeAfter, nSpeed, 400);
			if(!WaitForPosition(lEndX, lEndY, nStatus)){
				MessageBox("图标拖动异常，请检查！");
				goto RunFailed;
			}
		}
		break;
	case MODULE_TRIGGER:
		{
			CString str;
			int nCaseIndex = GetPrivateProfileInt(strModule, "TRIGGERINDEX", 0, strConfig)+1;
			BYTE triggerMode = GetPrivateProfileInt(strModule, "TRIGGERMODE", 0, strConfig);
			BYTE delay = GetPrivateProfileInt(strModule, "TIMEDELAY", 0, strConfig);
			WORD waveFrequency = m_waveFrequency = (GetPrivateProfileInt(strModule, "WAVEFRQ", 0, strConfig)+1)*100;
			WORD waveCnt = m_waveTimes = GetPrivateProfileInt(strModule, "WAVECNT", 0, strConfig);
			BYTE type = 0;
			BYTE clkFrequency = 0;
			BYTE clkTimes = 1;
			if(nCaseIndex == TRIGGERMODE_S2S){
				type = 1;
				clkFrequency = GetPrivateProfileInt(strModule, "DBCLKFRQ", 5, strConfig);
				clkTimes = GetPrivateProfileInt(strModule, "DBCLKCNT", 5, strConfig);
			}
			BYTE filterLevel = GetPrivateProfileInt("ROBOT", "CLICKSPEED", 5, theApp.m_szConfigFile);
			m_bPicSave = FALSE;
			if(nCaseIndex == TRIGGERMODE_L2P || nCaseIndex == TRIGGERMODE_VL || nCaseIndex == TRIGGERMODE_FPS){
				m_nTriggerFlag = 0;
				if(!startStorageThread()){
					MessageBox("启动相机捕获线程失败，请检查！");
					goto RunFailed;
				}else{
					m_bPicSave = TRUE;
				}
				DWORD dwRet = WaitForSingleObject( m_hEventCapReady, 3000 );
				if ( dwRet == WAIT_TIMEOUT ){
					MessageBox("测量相机互斥等待异常，照片保存可能会发生异常！");
					m_bPicSave = FALSE;
				}
			}else{
				m_nTriggerFlag = -1;
			}
			CONTROL_Trigger(m_lLoginID, type, triggerMode, filterLevel, nCaseIndex, clkFrequency, delay, clkTimes, waveFrequency*10, waveCnt);
			while(m_bPicSave){
				Sleep(1000);
			}
            nFailedTimes = 0;
            while(TRUE && g_pMainDlg->m_bCaseRun){
                BOOL bGetPos = CONTROL_QueryPos(m_lLoginID, &lPosX, &lPosY, &nStatus);
                if(bGetPos){
                    if(nStatus == 1)
                        break;
                }else{
                    Sleep(1000);
                    nFailedTimes++;
                }
                if(nFailedTimes > 5)
                    break;
            }
		}
		break;
	case MODULE_ROTATE:
		{
			int nRotateAngle = GetPrivateProfileInt(strModule, "ROTATEANGLE", 0, strConfig);
			LONG lCurPosX, lCurPosY;
			BOOL bGetPos = CONTROL_QueryPos(m_lLoginID, &lCurPosX, &lCurPosY, &nStatus);
			if(!bGetPos){
				MessageBox("转台转动查询位置失败，请检查！");
				goto RunFailed;
			}
			CONTROL_MoveTo(m_lLoginID, lCurPosX, lCurPosY, nRotateAngle);
            nFailedTimes = 0;
            while(TRUE && g_pMainDlg->m_bCaseRun){
                BOOL bGetPos = CONTROL_QueryPos(m_lLoginID, &lPosX, &lPosY, &nStatus, &lPosZ);
                if(bGetPos){
                    if(nStatus == 1){
                        g_pMainDlg->m_lCurPosZ = lPosZ;
                        break;
                    }
                }else{
                    Sleep(1000);
                    nFailedTimes++;
                }
                if(nFailedTimes > 5)
                    break;
            }
		}
		break;
    case MODULE_DELAY:
        {
            int nDelayTime = GetPrivateProfileInt(strModule, "TIMEDELAY", 0, strConfig);
            while(nDelayTime > 0 && g_pMainDlg->m_bCaseRun){
                sprintf(g_dbgBuf, "剩余等待时间%dms\r\n", nDelayTime);
                g_pMainDlg->ShowOutput(g_dbgBuf);
                Sleep(1000);
                nDelayTime -= 1000;
            }
        }
        break;
	default:
		break;
	}

	return TRUE;

RunFailed:
	return FALSE;
}

BOOL CCalibrationDlg::LoadModule(int nModuleType, char *strModule, ModuleData *pModuleData, CString strConfig)
{
	if(!pModuleData)
		return FALSE;

	char chTemp[MAX_PATH];
	switch(nModuleType)
	{
	case MODULE_INITSETTING:
		{
			ModuleInitSet *pInitSetData = (ModuleInitSet*)malloc(sizeof(ModuleInitSet));
			if(!pInitSetData){
				MessageBox("分配脚本执行数据内存块失败");
				return FALSE;
			}
			memset(pInitSetData, 0, sizeof(ModuleInitSet));
			pModuleData->nSize = sizeof(ModuleInitSet);
			pModuleData->pData = pInitSetData;
			GetPrivateProfileString(strModule, "SCRIPTFILE", "", chTemp, sizeof(chTemp), strConfig);
			strcpy(pInitSetData->cScriptFile, chTemp);
		}
		break;
	case MODULE_PM:
		{
			ModulePM *pPMData = (ModulePM*)malloc(sizeof(ModulePM));
			if(!pPMData){
				MessageBox("分配识别匹配数据内存块失败");
				return FALSE;
			}
			memset(pPMData, 0, sizeof(ModulePM));
			pModuleData->nSize = sizeof(ModulePM);
			pModuleData->pData = pPMData;
			GetPrivateProfileString(strModule, "PMTEMPLATE", "", chTemp, sizeof(chTemp), strConfig);
			strcpy(pPMData->cPMTemplate, chTemp);
		}
		break;
	case MODULE_MOVE:
		{
			ModuleFingerMove *pFingerMoveData = (ModuleFingerMove*)malloc(sizeof(ModuleFingerMove));
			if(!pFingerMoveData){
				MessageBox("分配手指移动数据内存块失败");
				return FALSE;
			}
			memset(pFingerMoveData, 0, sizeof(ModuleFingerMove));
			pModuleData->nSize = sizeof(ModuleFingerMove);
			pModuleData->pData = pFingerMoveData;
			pFingerMoveData->nFingerType = GetPrivateProfileInt(strModule, "FINGERTYPE", 0, strConfig);
			pFingerMoveData->nMode = GetPrivateProfileInt(strModule, "MODE", 0, strConfig);
			pFingerMoveData->nMoveToX = GetPrivateProfileInt(strModule, "MOVETOX", -1, strConfig);
			pFingerMoveData->nMoveToY = GetPrivateProfileInt(strModule, "MOVETOY", -1, strConfig);
			pFingerMoveData->nClick = GetPrivateProfileInt(strModule, "CLICK", 0, strConfig);
			pFingerMoveData->nFrequency = GetPrivateProfileInt(strModule, "FREQUENCY", 1, strConfig);
			pFingerMoveData->nTimes = GetPrivateProfileInt(strModule, "TIMES", 1, strConfig);
		}
		break;
	case MODULE_DRAG:
		{
			ModuleIconDrag *pIconDragData = (ModuleIconDrag*)malloc(sizeof(ModuleIconDrag));
			if(!pIconDragData){
				MessageBox("分配图标拖动数据内存块失败");
				return FALSE;
			}
			memset(pIconDragData, 0, sizeof(ModuleIconDrag));
			pModuleData->nSize = sizeof(ModuleIconDrag);
			pModuleData->pData = pIconDragData;
			pIconDragData->nMode = GetPrivateProfileInt(strModule, "MODE", 0, strConfig);
			pIconDragData->nStartX = GetPrivateProfileInt(strModule, "STARTX", 0, strConfig);
			pIconDragData->nStartY = GetPrivateProfileInt(strModule, "STARTY", 0, strConfig);
			pIconDragData->nEndX = GetPrivateProfileInt(strModule, "ENDX", 0, strConfig);
			pIconDragData->nEndY = GetPrivateProfileInt(strModule, "ENDY", 0, strConfig);
			pIconDragData->nTimeBefore = GetPrivateProfileInt(strModule, "TIMEBEFORE", 0, strConfig);
			pIconDragData->nTimeAfter = GetPrivateProfileInt(strModule, "TIMEAFTER", 0, strConfig);
		}
		break;
	case MODULE_TRIGGER:
		{
			ModuleTriggerMode *pTriggerModeData = (ModuleTriggerMode*)malloc(sizeof(ModuleTriggerMode));
			if(!pTriggerModeData){
				MessageBox("分配测量触发数据内存块失败");
				return FALSE;
			}
			memset(pTriggerModeData, 0, sizeof(ModuleTriggerMode));
			pModuleData->nSize = sizeof(ModuleTriggerMode);
			pModuleData->pData = pTriggerModeData;
			pTriggerModeData->nTriggerIndex = GetPrivateProfileInt(strModule, "TRIGGERINDEX", 0, strConfig);
			pTriggerModeData->nTriggerMode = GetPrivateProfileInt(strModule, "TRIGGERMODE", 0, strConfig);
			pTriggerModeData->nTimeDelay = GetPrivateProfileInt(strModule, "TIMEDELAY", 0, strConfig);
			pTriggerModeData->nWaveFrq = GetPrivateProfileInt(strModule, "WAVEFRQ", 0, strConfig);
			pTriggerModeData->nWaveCnt = GetPrivateProfileInt(strModule, "WAVECNT", 0, strConfig);
			pTriggerModeData->nDBClkFrq = GetPrivateProfileInt(strModule, "DBCLKFRQ", 0, strConfig);
			pTriggerModeData->nDBClkCnt = GetPrivateProfileInt(strModule, "DBCLKCNT", 0, strConfig);
		}
		break;
	case MODULE_ROTATE:
		{
			ModuleRotate *pRotateData = (ModuleRotate *)malloc(sizeof(ModuleRotate));
			if(!pRotateData){
				MessageBox("分配腰部转动数据内存块失败");
				return FALSE;
			}
			memset(pRotateData, 0, sizeof(ModuleRotate));
			pModuleData->nSize = sizeof(ModuleRotate);
			pModuleData->pData = pRotateData;
			pRotateData->nAntiWise = GetPrivateProfileInt(strModule, "ANTIWISE", 0, strConfig);
			pRotateData->nRotateAngle = GetPrivateProfileInt(strModule, "ROTATEANGLE", 0, strConfig);
		}
		break;
    case MODULE_DELAY:
        {
            ModuleDelay *pTimeDelay = (ModuleDelay *)malloc(sizeof(ModuleDelay));
            if(!pTimeDelay){
                MessageBox("分配时间延迟数据内存块失败");
                return FALSE;
            }
            memset(pTimeDelay, 0, sizeof(ModuleDelay));
            pModuleData->nSize = sizeof(ModuleDelay);
            pModuleData->pData = pTimeDelay;
            pTimeDelay->nDelayTime = GetPrivateProfileInt(strModule, "TIMEDELAY", 0, strConfig);
        }
        break;
	default:
		break;
	}

	return TRUE;
}

DWORD WINAPI CaseRunThread(LPVOID lpParam)
{
	CString str;
	g_pMainDlg->m_comboUsercase.GetLBText(g_pMainDlg->m_comboUsercase.GetCurSel(), str);
	sprintf(g_dbgBuf, "将要执行测试用例[%s]\r\n", str.GetBuffer(0));
	g_pMainDlg->ShowOutput(g_dbgBuf);
	str = g_pMainDlg->m_chUserCaseDir+str;
	str += "\\config.ini";
	BOOL bException = TRUE;
    CString strModule;
    int nModuleCnt = 0;
    int nModuleType;
    char chModuleName[200];

    LONG lPosX;
    LONG lPosY;
    LONG lPosZ;
    INT nStatus;
    if(g_pMainDlg->m_lLoginID > 0){
        BOOL bGetPos = CONTROL_QueryPos(g_pMainDlg->m_lLoginID, &lPosX, &lPosY, &nStatus, &g_pMainDlg->m_lCurPosZ);
        if(!bGetPos){
            g_pMainDlg->MessageBox("获取机器人当前坐标失败！");
            goto ThreadEnd;
        }
    }
    
	g_pMainDlg->m_lPMPosX = g_pMainDlg->m_lPMPosY = -1;
	nModuleCnt = GetPrivateProfileInt("COMMON", "INITMODULECNT", 0, str);
	if(nModuleCnt > 0){
        g_pMainDlg->m_nCurCircleIndex = 0;
		for(int i = 0; i < nModuleCnt && g_pMainDlg->m_bCaseRun; i++){
			strModule.Format("INITMODULE%d", i);
			nModuleType = GetPrivateProfileInt(strModule, "TYPE", MODULE_INITSETTING, str);
			GetPrivateProfileString(strModule, "MODULENAME", "NONE", chModuleName, sizeof(chModuleName), str);
			//HTREEITEM  hTreeItem = g_pMainDlg->m_TreeInit.InsertItem(chModuleName, 0, 1);
			sprintf(g_dbgBuf, "执行%s模块\r\n", chModuleName);
			g_pMainDlg->ShowOutput(g_dbgBuf);
			if(!g_pMainDlg->RunModule(nModuleType, strModule.GetBuffer(0), chModuleName, str))
				goto ThreadEnd;
		}
	}

	nModuleCnt = GetPrivateProfileInt("COMMON", "CIRCLEMODULECNT", 0, str);
	int nCircleTime = GetPrivateProfileInt("COMMON", "CIRCLETIMES", 1, str);
	if(nModuleCnt > 0){
		for(int j = 0; j < nCircleTime; j++){
			for(int i = 0; i < nModuleCnt && g_pMainDlg->m_bCaseRun; i++){
                g_pMainDlg->m_nCurCircleIndex = i+1;
				strModule.Format("CIRCLEMODULE%d", i);
				nModuleType = GetPrivateProfileInt(strModule, "TYPE", MODULE_INITSETTING, str);
				GetPrivateProfileString(strModule, "MODULENAME", "NONE", chModuleName, sizeof(chModuleName), str);
				//HTREEITEM  hTreeItem = g_pMainDlg->m_TreeCircle.InsertItem(chModuleName,0,1);
				sprintf(g_dbgBuf, "执行%s模块\r\n", chModuleName);
				g_pMainDlg->ShowOutput(g_dbgBuf);
				if(!g_pMainDlg->RunModule(nModuleType, strModule.GetBuffer(0), chModuleName, str))
					goto ThreadEnd;
			}
		}
	}
	bException = FALSE;
	
ThreadEnd:
	if(bException){
		g_pMainDlg->ShowOutput("异常，测试用例退出执行\r\n\r\n");
	}else{
		g_pMainDlg->ShowOutput("测试用例执行完成\r\n\r\n");
	}
	g_pMainDlg->m_bCaseRun = FALSE;
	g_pMainDlg->GetDlgItem(IDC_BTN_RUNCASE)->EnableWindow(TRUE);
	g_pMainDlg->GetDlgItem(IDC_BTN_NEWCASE)->EnableWindow(TRUE);
	g_pMainDlg->GetDlgItem(IDC_BTN_CASEDETAIL)->EnableWindow(TRUE);
    g_pMainDlg->GetDlgItem(IDC_BTN_DELCASE)->EnableWindow(TRUE);

	return 0;
}

void CCalibrationDlg::OnBnClickedBtnRuncase()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_bCaseRun){
		MessageBox("测试用例执行中，请等待当前测试用例执行完成！");
		return;
	}

	if(m_comboUsercase.GetCount() > 0){
		m_bCaseRun = TRUE;
		GetDlgItem(IDC_BTN_RUNCASE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_NEWCASE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_DELCASE)->EnableWindow(FALSE);
		HANDLE hThread = CreateThread(NULL, 0, CaseRunThread, (LPVOID)this, 0, NULL);
		if(hThread){
			EnableItem(2, FALSE);
			GetDlgItem(IDC_BTN_CASEDETAIL)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_CASECOMPLETE)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_CANCLEEDIT)->EnableWindow(FALSE);
            GetDlgItem(IDC_BTN_STOPCASE)->EnableWindow(TRUE);
			CloseHandle(hThread);
		}else{
			m_bCaseRun = FALSE;
			GetDlgItem(IDC_BTN_RUNCASE)->EnableWindow(TRUE);
			MessageBox("执行测试用例线程创建失败，请重试！");
		}
	}else{
		MessageBox("已有测试用例列表为空，请先新建测试用例！");
	}
}

void CCalibrationDlg::OnBnClickedBtnStopcase()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bCaseRun = FALSE;
}

/*
 * 返回十字光标位置
*/
BOOL CCalibrationDlg::TemplatePM(char *pTempFile, LONG &lPMPosX, LONG &lPMPosY)
{
    int bRt = FALSE;
	lPMPosX = lPMPosY = -1;

	// 移动到预置位
	CONTROL_MoveTo(m_lLoginID, m_lPrePosX, m_lPrePosY, m_lPrePosZ);
#if 0           // 0 by ZWW for testing
	int nQueryTimes = 0;
    LONG lPosX, lPosY;
    LONG lPosXLast, lPosYLast;
    lPosXLast = lPosYLast = 0;
	while(nQueryTimes < 20){
		Sleep(500);
		INT nStatus;
		BOOL bGetPos = CONTROL_QueryPos(m_lLoginID, &lPosX, &lPosY, &nStatus);
		if(bGetPos){
            if((lPosXLast == lPosX) && (lPosY == lPosYLast)){
				break;
			}else{
                sprintf(g_dbgBuf, "当前位置X %ld Y %ld\r\n", lPosX, lPosY);
                g_pMainDlg->ShowOutput(g_dbgBuf);
            }
            lPosXLast = lPosX;
            lPosYLast = lPosY;
		}else{
			g_pMainDlg->ShowOutput("模板匹配时获取当前位置失败！\r\n");
		}
		nQueryTimes++;
	}
	if(nQueryTimes >= 20 || abs(lPosXLast == lPosX) > 10 || abs(lPosY - m_lPrePosY) > 10){
		ShowOutput("移动至预置位过程中出错，退出模式匹配！\r\n");
		return FALSE;
	}
#endif

	CFile dibFile;
	if(!dibFile.Open(pTempFile, CFile::modeRead | CFile::shareDenyWrite)) {
		sprintf(g_dbgBuf, "模板文件 %s 不存在，匹配失败\r\n", pTempFile);
		ShowOutput(g_dbgBuf);
		return FALSE;
	}else{
		dibFile.Close();
		m_bReloadTemplate = TRUE;
		m_cCurrentMatchFile = pTempFile;
	}
    sprintf(g_dbgBuf, "匹配文件 %s\r\n", pTempFile);
    ShowOutput(g_dbgBuf);

	//Sleep(1000);		// 这里需完善模式匹配的可靠性，即真正的找到了模板才计算位置并移动
    if(!RunMatchThread()){
        MessageBox("启动匹配线程失败，请检查！");
        return FALSE;
    }

	DWORD result = WaitForSingleObject(m_MatchEvent, 5000);
	if(WAIT_OBJECT_0 == result){
		LONG lPosX = (m_fMatchPosX[0]+m_fMatchPosX[2])/2;
		LONG lPosY = (m_fMatchPosY[0]+m_fMatchPosY[2])/2;

		sprintf(g_dbgBuf, "匹配结果中心：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);
		lPosX -= m_nCamWidth/2;
		lPosY -= m_nCamHeight/2;
		sprintf(g_dbgBuf, "距离图像中心偏移：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

		lPosX *= m_dHorizontalRatio;
		lPosY *= m_dVerticalRatio;
		sprintf(g_dbgBuf, "转换为机械偏移：%ld %ld\n", lPosX, lPosY);
		OutputDebugString(g_dbgBuf);

		lPosX = m_lPrePosX-lPosX;
		lPosY = m_lPrePosY-lPosY;

        // 转换为十字光标位置
        lPMPosX = lPosX + m_lCamLaserShiftX;
        lPMPosY = lPosY + m_lCamLaserShiftY;
        // 转换为手指位置
        /*lPMPosX = lPMPosX - m_lFingerLaserShiftX;
        lPMPosY = lPMPosY - m_lFingerLaserShiftY;

        CONTROL_MoveTo(m_lLoginID, lPMPosX, lPMPosY, 0);
        sprintf(g_dbgBuf, "将要移动至：%ld %ld\r\n", lPMPosX, lPMPosY);
        ShowOutput(g_dbgBuf);

        Sleep(3000);*/
        Sleep(PM_SHOWTIME);
        bRt = TRUE;
	}else{
		ShowOutput("等待匹配结果超时，匹配失败！\r\n");
	}

    StopMatchThread();
	return bRt;
}

// 查询是否移动到指定位置
BOOL CCalibrationDlg::WaitForPosition(LONG lPosX, LONG lPosY, int &nStatus, int nTimeOut)
{
	int nQueryTimes = 0;
	while(nQueryTimes < nTimeOut){
		Sleep(1000);
		LONG lCurPosX;
		LONG lCurPosY;
		INT nStatus;
		BOOL bGetPos = CONTROL_QueryPos(m_lLoginID, &lCurPosX, &lCurPosY, &nStatus);
		if(bGetPos){
			sprintf(g_dbgBuf, "等待位置(%ld, %ld)当前位置(%ld, %ld) nStatus: %d\r\n", lPosX, lPosY, lCurPosX, lCurPosY, nStatus);
			ShowOutput(g_dbgBuf);
			if((abs(lPosX - lCurPosX) <= 10 && abs(lPosY - lCurPosY) <= 10) || nStatus == 1 ){
				break;
			}
		}else{
			ShowOutput("获取当前位置失败！\r\n");
		}
		nQueryTimes++;
	}
	if(nQueryTimes >= nTimeOut){
		return FALSE;
	}else{
		return TRUE;
	}
}


void CCalibrationDlg::OnBnClickedBtnClick()
{
	// TODO: 在此添加控件通知处理程序代码
	int nCaseIndex = 0;
	BYTE triggerMode = 0;
	//BYTE step = GetPrivateProfileInt("TRIGGERSET", "SLOWSTEP", 0, theApp.m_szConfigFile);
    BYTE step = GetPrivateProfileInt("ROBOT", "CLICKSPEED", 5, theApp.m_szConfigFile);
	BYTE delay = GetPrivateProfileInt("TRIGGERSET", "SLOWDELAY", 0, theApp.m_szConfigFile);
	WORD waveFrequency = GetPrivateProfileInt("TRIGGERSET", "SLOWWAVEFREQ", 0, theApp.m_szConfigFile);
	WORD waveTimes = GetPrivateProfileInt("TRIGGERSET", "SLOWWAVECNT", 0, theApp.m_szConfigFile);
	m_nTriggerFlag = -1;
    //CONTROL_ClickEx(m_lLoginID, 0, triggerMode, step, nCaseIndex, 0, delay, 1,  waveFrequency, waveTimes);
    CONTROL_Trigger(m_lLoginID, 0, 0, step, 0, 0, 0, 1, 1, 0);
}


void CCalibrationDlg::OnBnClickedBntCanclenew()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	str = m_chUserCaseDir+m_CurUserCase;
	theApp.DeleteDirectory(str.GetBuffer(0));

	HTREEITEM hItem;
	CString strModule;
	// 保存初始化列表各个功能模块参数
	ModuleData *pModuleData = NULL;
	hItem = m_TreeInit.GetRootItem();
	while(hItem){
		pModuleData = (ModuleData*)m_TreeInit.GetItemData(hItem);
		if(pModuleData){
			if(pModuleData->pData)
				free(pModuleData->pData);
			free(pModuleData);
		}
		hItem = m_TreeInit.GetNextSiblingItem(hItem);
	}

	// 保存循环列表各个功能模块参数
	hItem = m_TreeCircle.GetRootItem();
	while(hItem){
		pModuleData = (ModuleData*)m_TreeCircle.GetItemData(hItem);
		if(pModuleData){
			if(pModuleData->pData)
				free(pModuleData->pData);
			free(pModuleData);
		}
		hItem = m_TreeCircle.GetNextSiblingItem(hItem);
	}

	m_TreeInit.DeleteAllItems();
	m_TreeCircle.DeleteAllItems();
	m_TreeInit.EnableWindow(FALSE);
	m_TreeCircle.EnableWindow(FALSE);

	sprintf(g_dbgBuf, "测试用例【%s】创建取消！\r\n", m_CurUserCase.GetBuffer(0));
	ShowOutput(g_dbgBuf);

	EnableItem(2, FALSE);
	EnableItem(3, TRUE);
	m_CurUserCase = "";
}


void CCalibrationDlg::OnBnClickedBtnCasecomplete()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	m_comboUsercase.GetLBText(m_comboUsercase.GetCurSel(), str);
	CaseSerialize(str);

#if 0
	HTREEITEM  hTreeItem = NULL;
	ModuleData *pModuleData = NULL;
	hTreeItem = m_TreeInit.GetRootItem();
	while(hTreeItem){
		pModuleData = (ModuleData*)m_TreeInit.GetItemData(hTreeItem);
		if(pModuleData){
			if(pModuleData->pData)
				free(pModuleData->pData);
			free(pModuleData);
		}
		hTreeItem = m_TreeInit.GetNextSiblingItem(hTreeItem);
	}

	// 保存循环列表各个功能模块参数
	hTreeItem = m_TreeCircle.GetRootItem();
	while(hTreeItem){
		pModuleData = (ModuleData*)m_TreeCircle.GetItemData(hTreeItem);
		if(pModuleData){
			if(pModuleData->pData)
				free(pModuleData->pData);
			free(pModuleData);
		}
		hTreeItem = m_TreeCircle.GetNextSiblingItem(hTreeItem);
	}
#endif

	//m_TreeInit.DeleteAllItems();
	//m_TreeCircle.DeleteAllItems();
	m_TreeInit.EnableWindow(FALSE);
	m_TreeCircle.EnableWindow(FALSE);
	EnableItem(2, FALSE);
	GetDlgItem(IDC_BTN_NEWCASE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_RUNCASE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_CASEDETAIL)->EnableWindow(TRUE);
	m_comboUsercase.EnableWindow(TRUE);
	GetDlgItem(IDC_BTN_CASECOMPLETE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_CANCLEEDIT)->EnableWindow(FALSE);
	m_bIsCaseEdit = FALSE;
    m_CurUserCase = "";
}



void CCalibrationDlg::OnBnClickedBtnCancleedit()
{
    // TODO: 在此添加控件通知处理程序代码
    CString str;
    m_comboUsercase.GetLBText(m_comboUsercase.GetCurSel(), str);
    
    m_TreeInit.EnableWindow(FALSE);
    m_TreeCircle.EnableWindow(FALSE);
    EnableItem(2, FALSE);
    GetDlgItem(IDC_BTN_NEWCASE)->EnableWindow(TRUE);
    GetDlgItem(IDC_BTN_RUNCASE)->EnableWindow(TRUE);
    GetDlgItem(IDC_BTN_CASEDETAIL)->EnableWindow(TRUE);
    m_comboUsercase.EnableWindow(TRUE);
    GetDlgItem(IDC_BTN_CASECOMPLETE)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_CANCLEEDIT)->EnableWindow(FALSE);
    m_bIsCaseEdit = FALSE;
    m_CurUserCase = "";
}


void CCalibrationDlg::OnBnClickedBtnRotatetoprepos()
{
    // TODO: 在此添加控件通知处理程序代码
    LONG lPosX;
    LONG lPosY;
    LONG lPosZ;
    INT nStatus;
    if(g_pMainDlg->m_lLoginID > 0){
        BOOL bGetPos = CONTROL_QueryPos(g_pMainDlg->m_lLoginID, &lPosX, &lPosY, &nStatus, &lPosZ);
        if(bGetPos){
            CONTROL_MoveTo(m_lLoginID, lPosX, lPosY, m_lPrePosZ);
        }else{
            MessageBox("获取机器人当前坐标失败！");
        }
    }
}


void CCalibrationDlg::OnBnClickedBtnTimedelay()
{
    // TODO: 在此添加控件通知处理程序代码
    AddItem(MODULE_DELAY);
}

BOOL CCalibrationDlg::InitUserCase()
{
    // 初始化用户测试用例
    m_comboUsercase.ResetContent();
    FILE *pFile = fopen(theApp.m_szUserCaseFile, "rb");
    LONG lFileLen = 0;
    if(pFile){
        fseek(pFile,0,SEEK_END);
        lFileLen = ftell(pFile);
        fclose(pFile);
    }else{
        return TRUE;
    }

    //GetModuleFileName(NULL, m_chUserCaseDir, MAX_PATH);
    strcpy(m_chUserCaseDir, theApp.m_szUserCaseFile);
    (strrchr(m_chUserCaseDir, '\\'))[1] = 0;

    char* chSectionNames = NULL;
    char *pSectionName;                       //保存找到的某个节名字符串的首地址
    theApp.m_nUserCaseCount = 0;                     //统计节的个数
    chSectionNames = (char*)malloc(lFileLen);
    if(chSectionNames){
        DWORD dwRt = GetPrivateProfileSectionNames(chSectionNames, lFileLen, theApp.m_szUserCaseFile);
        if(dwRt > 0){
            pSectionName = chSectionNames;
            char chCreateTime[200];
            for(int i = 0; i < lFileLen; i++){
                GetPrivateProfileString(pSectionName, "CREATETIME", "", chCreateTime, sizeof(chCreateTime), theApp.m_szUserCaseFile);
                if(strlen(chCreateTime) > 0){
                    m_comboUsercase.InsertString(theApp.m_nUserCaseCount++, pSectionName);
                }else{
                    WritePrivateProfileString(pSectionName, NULL, NULL, theApp.m_szUserCaseFile);
                }
                i+=strlen(pSectionName);
                pSectionName += strlen(pSectionName)+1;
                if(!(*pSectionName))
                    break;
            }
            m_comboUsercase.SetCurSel(0);
        }
        TRACE("Usercase total number: %d\n", theApp.m_nUserCaseCount);
    }
    if(chSectionNames){
        free(chSectionNames);
        chSectionNames = NULL;
    }

    return TRUE;
}


void CCalibrationDlg::OnInsertTreeItem()
{
    // TODO: 在此添加命令处理程序代码
    CDlgInsertModule dlgInsertModule;
    if(dlgInsertModule.DoModal() == IDOK){
        HTREEITEM hItem = NULL;
        if(m_RadioInit.GetCheck()){
            hItem = m_TreeInit.GetSelectedItem();
        }else{
            hItem = m_TreeCircle.GetSelectedItem();
        }
        AddItem(dlgInsertModule.m_nCheckRadio, hItem);
    }
}

BOOL CCalibrationDlg::RunMatchThread()
{
    m_bMatch = TRUE;
    m_hThreadMatch = CreateThread(NULL, 0, ThreadMatch, (LPVOID)this, 0, &ThreadCaptureDisplayID);
    if(m_hThreadMatch != INVALID_HANDLE_VALUE){
        //CloseHandle(hThreadMatch);
    }else{
        m_bMatch = FALSE;
    }
    return m_bMatch;
}

void CCalibrationDlg::StopMatchThread()
{
    m_bMatch = FALSE;
    if(m_hThreadMatch != INVALID_HANDLE_VALUE){
        WaitForSingleObject(m_hThreadMatch, 2000);
        CloseHandle(m_hThreadMatch);
        m_hThreadMatch = INVALID_HANDLE_VALUE;
    }
}


