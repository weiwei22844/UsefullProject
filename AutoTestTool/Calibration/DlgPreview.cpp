// DlgPreview.cpp : 实现文件
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgPreview.h"
#include "afxdialogex.h"
#include "DlgTemplate.h"
#include "DlgTriggerSet.h"

#include <pylon/PylonIncludes.h>
#include <pylon/PylonGUI.h>

// Include files used by samples.
#include "include/ConfigurationEventPrinter.h"
#include <pylon/PylonImage.h>
#include <pylon/Pixel.h>
#include <pylon/ImageFormatConverter.h>
using namespace Pylon;

#include <pylon/usb/BaslerUsbInstantCamera.h>
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
typedef Pylon::CBaslerUsbCameraEventHandler CameraEventHandler_t;	// Or use Camera_t::CameraEventHandler_t
typedef Pylon::CBaslerUsbImageEventHandler ImageEventHandler_t;		// Or use Camera_t::ImageEventHandler_t
typedef Pylon::CBaslerUsbGrabResultPtr GrabResultPtr_t;				// Or use Camera_t::GrabResultPtr_t
using namespace Basler_UsbCameraParams;

static CDlgPreview *g_pPreviewDlg = NULL;
Camera_t *g_pCamera = NULL;
static char g_dbgStr[500];

static BITMAPINFO_8bpp g_bmp;
CRect g_prevWndRect;
HDC g_hDC = NULL;
HDC g_hMemDC = NULL;
int g_nEnableROI = FALSE;
CString g_CaseSection;
CImageFormatConverter g_Converter;
CPylonImage g_targetImage;
// added by ZWW for double buffer
CDC g_SecondDC;
CBitmap g_bmpTemp;


// Example handler for GigE camera events.
// Additional handling is required for GigE camera events because the event network packets can be lost, doubled or delayed on the network.
class CPreEventHandler : public CameraEventHandler_t, public ImageEventHandler_t
{
public:
	CPreEventHandler()
		: m_nextExpectedFrameNumberImage(0)
		, m_nextExpectedFrameNumberExposureEnd(0)
		, m_nextFrameNumberForMove(0)
		, m_frameIDsInitialized(false)
	{
		// Reserve space to log camera, image and move events.
		m_lastTimeStamp = 0;
	}

	// This method is called when a camera event has been received.
	virtual void OnCameraEvent( Camera_t& camera, intptr_t userProvidedId, GenApi::INode* /* pNode */)
	{
		if ( userProvidedId == eMyExposureEndEvent)
		{
			// An Exposure End event has been received.
			uint16_t frameNumber = (uint16_t)camera.EventExposureEndFrameID.GetValue();

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
					//throw RUNTIME_EXCEPTION( "An Exposure End event has been lost. Expected frame number is %d but got frame number %d.", m_nextExpectedFrameNumberExposureEnd, frameNumber);
				}
				IncrementFrameNumber( m_nextExpectedFrameNumberExposureEnd);
			}
		}
		else if ( userProvidedId == eMyFrameStartOvertrigger)
		{
			// The camera has been overtriggered.
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

		// Check whether the imaged item or the sensor head can be moved.
		// This will be the case if the Exposure End has been lost or if the Exposure End is received later than the image.
		if ( frameNumber == m_nextFrameNumberForMove)
		{
			MoveImagedItemOrSensorHead();
		}

		// Check for missing images.
		if ( frameNumber != m_nextExpectedFrameNumberImage)
		{
			//throw RUNTIME_EXCEPTION( "An image has been lost. Expected frame number is %d but got frame number %d.", m_nextExpectedFrameNumberImage, frameNumber);
		}
		IncrementFrameNumber( m_nextExpectedFrameNumberImage);

		// 默认PixelType为：PixelType_BayerRG8
		TRACE("GetTimeStamp: %I64u width: %d height: %d imageSize: %d PixelType: %d tick count diff: %I64u\n", 
			ptrGrabResult->GetTimeStamp(), ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight(), ptrGrabResult->GetImageSize(), ptrGrabResult->GetPixelType(), ptrGrabResult->GetTimeStamp()-m_lastTimeStamp);
		m_lastTimeStamp = ptrGrabResult->GetTimeStamp();

#if 0
		Pylon::DisplayImage(1, ptrGrabResult);
#else
		CPylonImage image;
		image.CopyImage(ptrGrabResult->GetBuffer(), ptrGrabResult->GetImageSize(), ptrGrabResult->GetPixelType(), ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight(), ptrGrabResult->GetPaddingX());
		//image.Reset(PixelType_Mono8, ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight());
		//int nsize = image.GetImageSize();	//9362368
		//Pylon::DisplayImage(1, image);


		//image.ChangePixelType(PixelType_Mono8);
		g_Converter.Convert( g_targetImage, image);

		StretchDIBits(g_hDC, g_pPreviewDlg->m_nOffsetX*g_pPreviewDlg->m_dPrevScale, g_pPreviewDlg->m_nOffsetY*g_pPreviewDlg->m_dPrevScale, g_pPreviewDlg->m_nWidth*g_pPreviewDlg->m_dPrevScale, 
			 g_pPreviewDlg->m_nHeight*g_pPreviewDlg->m_dPrevScale, 0, 0, g_pPreviewDlg->m_nWidth, g_pPreviewDlg->m_nHeight, g_targetImage.GetBuffer(), (BITMAPINFO *)&g_bmp, DIB_RGB_COLORS, SRCCOPY);
		CString str;
		str.Format("%d.bmp", frameNumber);
		//image.Save(ImageFileFormat_Bmp, str.GetBuffer(0));
		//Pylon::DisplayImage(1, g_targetImage);
#endif
		//g_Images[g_nCapIndex++].CopyImage( ptrGrabResult->GetBuffer(), ptrGrabResult->GetImageSize(), ptrGrabResult->GetPixelType(), ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight(), ptrGrabResult->GetPaddingX());

		if(g_nEnableROI){
			int nROILeft = GetPrivateProfileInt(g_CaseSection.GetBuffer(0), "ROILEFT", 0, theApp.m_szConfigFile);
			int nROIWidth = GetPrivateProfileInt(g_CaseSection.GetBuffer(0), "ROIWIDTH", g_pPreviewDlg->m_nWidth, theApp.m_szConfigFile);
			int nROITop = GetPrivateProfileInt(g_CaseSection.GetBuffer(0), "ROITOP", 0, theApp.m_szConfigFile);
			int nROIHeight = GetPrivateProfileInt(g_CaseSection.GetBuffer(0), "ROIHEIGHT", g_pPreviewDlg->m_nHeight, theApp.m_szConfigFile);
			if(nROILeft < 0)
				nROILeft = 0;
			if(nROILeft%2)					// 不知道为什么不能为奇数
				nROILeft -= 1;
			if(nROIWidth > g_pPreviewDlg->m_nWidth)
				nROIWidth = g_pPreviewDlg->m_nWidth;
			if(nROITop < 0)
				nROITop = 0;
			if(nROITop%2)
				nROITop -= 1;
			if(nROIHeight > g_pPreviewDlg->m_nHeight)
				nROIHeight = g_pPreviewDlg->m_nHeight;
			if(nROILeft+nROIWidth > g_pPreviewDlg->m_nWidth || nROITop+nROIHeight > g_pPreviewDlg->m_nHeight){
				g_pPreviewDlg->m_StaticInfo.SetWindowTextA("ROI设置异常，请重新设置ROI区域！\r\n");
			}else{
				HGDIOBJ hOldPen = SelectObject(g_hDC, g_pPreviewDlg->m_ROIPen);
				HGDIOBJ hOldBrush = SelectObject(g_hDC, GetStockObject(NULL_BRUSH));
				Rectangle(g_hDC, (g_pPreviewDlg->m_nOffsetX+nROILeft)*g_pPreviewDlg->m_dPrevScale,  (g_pPreviewDlg->m_nOffsetY+nROITop)*g_pPreviewDlg->m_dPrevScale,
					 (g_pPreviewDlg->m_nOffsetX+nROILeft+nROIWidth)*g_pPreviewDlg->m_dPrevScale, (g_pPreviewDlg->m_nOffsetY+nROITop+nROIHeight)*g_pPreviewDlg->m_dPrevScale);
				SelectObject(g_hDC, hOldPen);
				SelectObject(g_hDC, hOldBrush);
			}
		}
	}

	void MoveImagedItemOrSensorHead()
	{
		// The imaged item or the sensor head can be moved now...
		// The camera may not be ready for a trigger at this point yet because the sensor is still being read out.
		// See the documentation of the CInstantCamera::WaitForFrameTriggerReady() method for more information.
		IncrementFrameNumber( m_nextFrameNumberForMove);
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

	uint64_t m_lastTimeStamp;
};

// CDlgPreview 对话框

IMPLEMENT_DYNAMIC(CDlgPreview, CDialogEx)

CDlgPreview::CDlgPreview(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgPreview::IDD, pParent)
{
	m_nMaxWidth = 1280;
	m_nMaxHeight = 720;

	int xScreen = GetSystemMetrics(SM_CXSCREEN);
	int yScreen = GetSystemMetrics(SM_CYSCREEN);
	int nShowType = GetPrivateProfileInt(_T("COMMON"), _T("SHOWREALRESOLUTION"), 0, theApp.m_szConfigFile);
	if(nShowType){
		m_dPrevScale = 1;
	}else{                  // 根据屏幕分辨率动态调整主程序窗口大小
		if(xScreen > 1920){
			m_dPrevScale = 0.8;
		}else if(xScreen > 1600){
			m_dPrevScale = 0.7;
		}else if(xScreen > 1400){
			m_dPrevScale = 0.6;
		}else if(xScreen > 1280){
			m_dPrevScale = 0.5;
		}else{
			m_dPrevScale = 0.4;
		}
	}

	m_bCapture = FALSE;
    m_nFps = 100;
}

CDlgPreview::~CDlgPreview()
{
}

void CDlgPreview::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PREVIEW_WND, m_PreviewWnd);
	DDX_Control(pDX, IDC_COMBO_EXPOSURE, m_ComBoxExposureAuto);
	DDX_Control(pDX, IDC_EDIT_EXPOSURETIME, m_EditExposureTime);
	DDX_Control(pDX, IDC_EDIT_WIDTH, m_EditWidth);
	DDX_Control(pDX, IDC_EDIT_HEIGHT, m_EditHeight);
	DDX_Control(pDX, IDC_STATIC_RESOLUTION, m_StaticResolution);
	DDX_Control(pDX, IDC_EDIT_OFFSETX, m_EditOffsetX);
	DDX_Control(pDX, IDC_EDIT_OFFSETY, m_EditOffsetY);
	DDX_Control(pDX, IDC_STATIC_INFO, m_StaticInfo);
	DDX_Control(pDX, IDC_SLIDER_GAIN, m_SliderGain);
	DDX_Control(pDX, IDC_CHECK_ROI, m_CkEnableROI);
}


BEGIN_MESSAGE_MAP(CDlgPreview, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_START, &CDlgPreview::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CDlgPreview::OnBnClickedBtnStop)
	ON_BN_CLICKED(IDOK, &CDlgPreview::OnBnClickedOk)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BTN_APPLYSET, &CDlgPreview::OnBnClickedBtnApplyset)
	ON_CBN_SELCHANGE(IDC_COMBO_EXPOSURE, &CDlgPreview::OnCbnSelchangeComboExposure)
	ON_BN_CLICKED(IDC_BTN_SETROI, &CDlgPreview::OnBnClickedBtnSetroi)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_ROI, &CDlgPreview::OnBnClickedCheckRoi)
END_MESSAGE_MAP()


// CDlgPreview 消息处理程序


BOOL CDlgPreview::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CString str;
	CRect rect, rectNew;
	int nBottomMargin = 40;
	int nPosX;
	int nPosY;
	int nWd, nHt;
	int nValue;
	CButton *pBtn = (CButton*)GetDlgItem(IDOK);
	pBtn->GetWindowRect(rect);
	ScreenToClient(&rect);
	nPosX = rect.right+10;
	nPosY = 10;

	nWd = m_nMaxWidth*m_dPrevScale;
	nHt = m_nMaxHeight*m_dPrevScale;
	m_PreviewWnd.SetWindowPos(NULL, nPosX, nPosY, nWd, nHt, SWP_NOZORDER);
	m_PreviewWnd.GetWindowRect(&rect);
	ScreenToClient(&rect);
	rectNew.top=rectNew.left=0;
	rectNew.right=rect.left+nWd+10;
	rectNew.bottom=nHt+20+nBottomMargin;
	AdjustWindowRectEx(&rectNew, GetWindowLong(m_hWnd, GWL_STYLE), ::GetMenu(m_hWnd) != NULL, GetWindowLong(m_hWnd, GWL_EXSTYLE));
	::MoveWindow(m_hWnd, 20, 20, rectNew.Width(), rectNew.Height(), TRUE);
	m_StaticInfo.SetWindowPos(NULL, nPosX, nPosY+nHt+10, nWd, 50, SWP_NOZORDER);
	g_pPreviewDlg= this;
	
	str.Format("Max Resolution %dx%d", m_nMaxWidth, m_nMaxHeight);
	m_StaticResolution.SetWindowText(str);
#if 0
	m_nOffsetX = GetPrivateProfileInt("BASLER", "OFFSETX", 0, theApp.m_szConfigFile);
	m_nOffsetY = GetPrivateProfileInt("BASLER", "OFFSETY", 0, theApp.m_szConfigFile);
	m_nWidth = GetPrivateProfileInt("BASLER", "WIDTH", m_nMaxWidth, theApp.m_szConfigFile);
	m_nHeight = GetPrivateProfileInt("BASLER", "HEIGHT", m_nMaxHeight, theApp.m_szConfigFile);
#else
	m_nWidth = m_nMaxWidth;
    switch(m_nFps)
    {
    case 100:
        m_nHeight = m_nMaxHeight;
        break;
	case 200:
		m_nHeight = 800;
		break;
	case 300:
		m_nHeight = 500;
		break;
	case 400:
		m_nHeight = 400;
		break;
	case 500:
		m_nHeight = 300;
		break;
    default:
        m_nWidth = m_nMaxWidth;
        break;
    }
    m_nOffsetX = 0;
    m_nOffsetY = (m_nMaxHeight - m_nHeight)/2;
	if(m_nOffsetY%2)
		m_nOffsetY -= 1;
#endif
	str.Format("%d", m_nOffsetX);
	m_EditOffsetX.SetWindowText(str);
	str.Format("%d", m_nOffsetY);
	m_EditOffsetY.SetWindowText(str);
	str.Format("%d", m_nWidth);
	m_EditWidth.SetWindowText(str);
	str.Format("%d", m_nHeight);
	m_EditHeight.SetWindowText(str);

	g_Converter.OutputPixelFormat = PixelType_BGR8packed;//PixelType_RGB8packed;
	g_Converter.OutputBitAlignment = OutputBitAlignment_MsbAligned;
	m_SliderGain.SetRange(0, 24);

	if(m_nCaseIndex == TRIGGERMODE_L2P || m_nCaseIndex == TRIGGERMODE_VL || m_nCaseIndex == TRIGGERMODE_FPS){
		str.Format("TRIGGERCASE%d", m_nCaseIndex-1);
		nValue = GetPrivateProfileInt(str.GetBuffer(0), "SAVEROI", 0, theApp.m_szConfigFile);
		m_CkEnableROI.SetCheck(nValue);
		GetDlgItem(IDC_BTN_SETROI)->EnableWindow(nValue);
		g_nEnableROI = nValue;
	}else{
		m_CkEnableROI.EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_SETROI)->EnableWindow(FALSE);
	}
	g_CaseSection.Format("TRIGGERCASE%d", m_nCaseIndex-1);
	
	m_ROIPen.CreatePen(PS_SOLID,1,RGB(0,0,255));

	OnBnClickedBtnStart();

	m_ComBoxExposureAuto.InsertString(0, "Off");
	m_ComBoxExposureAuto.InsertString(1, "Once");
	m_ComBoxExposureAuto.InsertString(2, "Continuous");

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

DWORD WINAPI ThreadCap(LPVOID lpParam)
{
	// Exit code of the sample application.
	int exitCode = 0;

	// Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
	// is initialized during the lifetime of this object.
	Pylon::PylonAutoInitTerm autoInitTerm;

	try
	{
		// Create the event handler.
		CPreEventHandler eventHandler;

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

		g_pCamera = &camera;
		camera.ExposureAuto.SetValue(ExposureAuto_Off);
		if(g_pPreviewDlg->m_nWidth > camera.Width.GetValue()){				// 重置宽大于原来的宽度则需先调整offset
			if (IsWritable(camera.OffsetX)){
				//camera.OffsetX.SetValue(camera.OffsetX.GetMin());
				camera.OffsetX.SetValue(g_pPreviewDlg->m_nOffsetX);
			}
			camera.Width = g_pPreviewDlg->m_nWidth;
		}else{
			camera.Width = g_pPreviewDlg->m_nWidth;
			if (IsWritable(camera.OffsetX)){
				//camera.OffsetX.SetValue(camera.OffsetX.GetMin());
				camera.OffsetX.SetValue(g_pPreviewDlg->m_nOffsetX);
			}
		}
		
		if(g_pPreviewDlg->m_nHeight > camera.Height.GetValue()){
			if (IsWritable(camera.OffsetY))
			{
				//camera.OffsetY.SetValue(camera.OffsetY.GetMin());
				camera.OffsetY.SetValue(g_pPreviewDlg->m_nOffsetY);
			}
			camera.Height = g_pPreviewDlg->m_nHeight;
		}else{
			camera.Height = g_pPreviewDlg->m_nHeight;
			if (IsWritable(camera.OffsetY))
			{
				//camera.OffsetY.SetValue(camera.OffsetY.GetMin());
				camera.OffsetY.SetValue(g_pPreviewDlg->m_nOffsetY);
			}
		}
		camera.ExposureTime = 1000*1000/g_pPreviewDlg->m_nFps;

		camera.AcquisitionMode.SetValue( AcquisitionMode_Continuous );
		// Select the frame burst start trigger
		camera.TriggerSelector.SetValue(TriggerSelector_FrameBurstStart);
		// Set the mode for the selected trigger
		camera.TriggerMode.SetValue( TriggerMode_Off );
		// Set the source for the selected trigger
		camera.TriggerSource.SetValue ( TriggerSource_Line1 );

		camera.TriggerSelector.SetValue(TriggerSelector_FrameStart);
		//camera.TriggerMode.SetValue( TriggerMode_On);

		camera.AcquisitionFrameRateEnable = true;
		camera.AcquisitionFrameRate = 10;
		camera.DeviceLinkThroughputLimitMode = DeviceLinkThroughputLimitMode_Off;
		g_pPreviewDlg->UpdateCameraSetting();

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
		camera.StartGrabbing( GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);

		// This smart pointer will receive the grab result data.
		CGrabResultPtr ptrGrabResult;

		// Camera.StopGrabbing() is called automatically by the RetrieveResult() method
		// when c_countOfImagesToGrab images have been retrieved.
		while ( camera.IsGrabbing())
		{
			// Retrieve grab results and notify the camera event and image event handlers.
			if(!g_pPreviewDlg->m_bCapture)
				break;
			Sleep(10);
			//camera.RetrieveResult( 50000, ptrGrabResult, TimeoutHandling_ThrowException);
			// Nothing to do here with the grab result, the grab results are handled by the registered event handlers.
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
		//eventHandler.PrintLog();
	}
	catch (GenICam::GenericException &e)
	{
		sprintf(g_dbgStr, "An exception occurred. %s\r\n", e.GetDescription());
		g_pPreviewDlg->m_StaticInfo.SetWindowText(g_dbgStr);
		exitCode = 1;
	}

	g_pPreviewDlg->m_PreviewWnd.Invalidate();
	if(g_pPreviewDlg->m_bCapture){					// Exception happen
		g_pPreviewDlg->OnBnClickedBtnStop();
	}
	g_pCamera = NULL;
	g_pPreviewDlg->m_StaticInfo.SetWindowText("线程退出！\n");
	if(exitCode == 0){
	}

	if(g_hDC){
		::ReleaseDC(g_pPreviewDlg->m_PreviewWnd.GetSafeHwnd(), g_hDC);
		g_hDC = NULL;
	}

	return exitCode;
}


void CDlgPreview::OnBnClickedBtnStart()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_bCapture){
		MessageBox("正在预览中！");
		return;
	}

	g_hDC = ::GetDC(m_PreviewWnd.GetSafeHwnd());//显示图像的窗口HWND.
	//g_hMemDC = ::CreateCompatibleDC(g_hDC);
	SetStretchBltMode(g_hDC, HALFTONE);

	/*g_SecondDC.CreateCompatibleDC(m_PreviewWnd.GetDC());
	g_bmpTemp.CreateCompatibleBitmap(m_PreviewWnd.GetDC(), m_nMaxWidth, m_nMaxHeight);
	g_SecondDC.SelectObject(&g_bmpTemp);
	g_SecondDC.SetStretchBltMode(HALFTONE);*/

	int nBitCount = 24;			//24;
	memset(&g_bmp, 0, sizeof(BITMAPINFO_8bpp));
	DWORD dwPaletteSize = 256*sizeof(RGBQUAD);
	UINT uLineByte = ((m_nWidth * nBitCount+31)/32) * 4;
	DWORD dwBmBitsSize = uLineByte * m_nHeight;
	memset(&g_bmp, 0, sizeof(LPBITMAPINFOHEADER));

	g_bmp.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	g_bmp.bmiHeader.biWidth  = m_nWidth;
	g_bmp.bmiHeader.biHeight = -m_nHeight;               // 摄像头采集图像数据是从上到下的，所以这里用负数
	g_bmp.bmiHeader.biPlanes = 1;
	g_bmp.bmiHeader.biBitCount = nBitCount;
	g_bmp.bmiHeader.biCompression = BI_RGB;
	g_bmp.bmiHeader.biSizeImage = dwBmBitsSize;
	g_bmp.bmiHeader.biXPelsPerMeter	= 0;
	g_bmp.bmiHeader.biYPelsPerMeter	= 0;
	if(nBitCount == 8){
		g_bmp.bmiHeader.biClrUsed = 256;
		g_bmp.bmiHeader.biClrImportant = 256;
	}
	memset(g_bmp.bmiColors, 0, sizeof(g_bmp.bmiColors));
	for (int i=0; i < 256; i++) {
		g_bmp.bmiColors[ i ].rgbRed = i;
		g_bmp.bmiColors[ i ].rgbGreen = i;
		g_bmp.bmiColors[ i ].rgbBlue = i;
		g_bmp.bmiColors[ i ].rgbReserved = 0;
	}

	m_PreviewWnd.GetClientRect(&g_prevWndRect);

	m_bCapture = TRUE;
	HANDLE  hThreadCaptureDisplay = CreateThread(NULL, 0, ThreadCap, (LPVOID)this, 0, NULL);
	if(hThreadCaptureDisplay){
		m_StaticInfo.SetWindowText("开始预览");
		m_EditOffsetX.EnableWindow(FALSE);
		m_EditOffsetY.EnableWindow(FALSE);
		m_EditWidth.EnableWindow(FALSE);
		m_EditHeight.EnableWindow(FALSE);
		CloseHandle(hThreadCaptureDisplay);
	}else{
		m_bCapture = FALSE;
	}
}


void CDlgPreview::OnBnClickedBtnStop()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bCapture = FALSE;
	/*m_EditOffsetX.EnableWindow(TRUE);
	m_EditOffsetY.EnableWindow(TRUE);
	m_EditWidth.EnableWindow(TRUE);
	m_EditHeight.EnableWindow(TRUE);*/
}


void CDlgPreview::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_bCapture){
		m_bCapture = FALSE;
		Sleep(1000);
	}

	CDialogEx::OnOK();
}

void CDlgPreview::UpdateCameraSetting()
{
	if(g_pCamera){
		CString str;
		m_ComBoxExposureAuto.EnableWindow(TRUE);
		//m_EditExposureTime.EnableWindow(TRUE);
		switch(g_pCamera->ExposureAuto.GetValue())
		{
		case ExposureAuto_Off:
			m_ComBoxExposureAuto.SetCurSel(0);
			break;
		case ExposureAuto_Once:
			m_ComBoxExposureAuto.SetCurSel(1);
			break;
		case ExposureAuto_Continuous:
			m_ComBoxExposureAuto.SetCurSel(2);
			break;
		default:
			m_ComBoxExposureAuto.SetCurSel(0);
			break;
		}
		str.Format("%f", g_pCamera->ExposureTime.GetValue());
		m_EditExposureTime.SetWindowText(str);

		m_SliderGain.SetPos(g_pCamera->Gain.GetValue());
	}
}


void CDlgPreview::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_bCapture){
		m_bCapture = FALSE;
		Sleep(1000);
	}
	CDialogEx::OnClose();
}


void CDlgPreview::OnBnClickedBtnApplyset()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	CString str;
	if(g_pCamera){
		m_EditExposureTime.GetWindowText(str);
		g_pCamera->ExposureTime.SetValue(atof(str.GetBuffer(0)));
	}
	int nOffsetX, nOffsetY;
	int nWidth, nHeight;

	m_EditOffsetX.GetWindowText(str);
	nOffsetX = atoi(str.GetBuffer(0));
	if(nOffsetX < 0 || nOffsetX > m_nMaxWidth){
		MessageBox("Offset X超出有效范围，请检查！");
		return;
	}
	m_EditOffsetY.GetWindowText(str);
	nOffsetY = atoi(str.GetBuffer(0));
	if(nOffsetY < 0 || nOffsetY > m_nMaxHeight){
		MessageBox("Offset Y超出有效范围，请检查！");
		return;
	}
	m_EditWidth.GetWindowText(str);
	nWidth = atoi(str.GetBuffer(0));
	if(nWidth < 0 || nWidth > m_nMaxWidth){
		MessageBox("Width超出有效范围，请检查！");
		return;
	}
	m_EditHeight.GetWindowText(str);
	nHeight = atoi(str.GetBuffer(0));
	if(nHeight < 0 || nHeight > m_nMaxHeight){
		MessageBox("Height超出有效范围，请检查！");
		return;
	}
	if(nOffsetX+nWidth>m_nMaxWidth){
		MessageBox("Offset X和Width与最大分辨率冲突，请检查！");
		return;
	}
	if(nOffsetY+nHeight>m_nMaxHeight){
		MessageBox("Offset Y和Height与最大分辨率冲突，请检查！");
		return;
	}
	str.Format("%d", nOffsetX);
	WritePrivateProfileString("BASLER", "OFFSETX", str.GetBuffer(0), theApp.m_szConfigFile);
	m_nOffsetX = nOffsetX;
	str.Format("%d", nOffsetY);
	WritePrivateProfileString("BASLER", "OFFSETY", str.GetBuffer(0), theApp.m_szConfigFile);
	m_nOffsetY = nOffsetY;
	str.Format("%d", nWidth);
	WritePrivateProfileString("BASLER", "WIDTH", str.GetBuffer(0), theApp.m_szConfigFile);
	m_nWidth = nWidth;
	str.Format("%d", nHeight);
	WritePrivateProfileString("BASLER", "HEIGHT", str.GetBuffer(0), theApp.m_szConfigFile);
	m_nHeight = nHeight;
}


void CDlgPreview::OnCbnSelchangeComboExposure()
{
	// TODO: 在此添加控件通知处理程序代码
	if(g_pCamera){
		CString str;
		if(m_ComBoxExposureAuto.GetCurSel() == ExposureAuto_Continuous){
			m_EditExposureTime.EnableWindow(FALSE);
			g_pCamera->ExposureAuto.SetValue(ExposureAuto_Continuous);
			str.Format("%f", g_pCamera->ExposureTime.GetValue());
			m_EditExposureTime.SetWindowText(str);
		}else if(m_ComBoxExposureAuto.GetCurSel() == ExposureAuto_Once){
			//m_EditExposureTime.EnableWindow(TRUE);
			g_pCamera->ExposureAuto.SetValue(ExposureAuto_Once);
		}else{
			//m_EditExposureTime.EnableWindow(TRUE);
			g_pCamera->ExposureAuto.SetValue(ExposureAuto_Off);
		}
	}
}


void CDlgPreview::OnBnClickedBtnSetroi()
{
	// TODO: 在此添加控件通知处理程序代码
	CRect rect;
	m_PreviewWnd.GetWindowRect(&rect);
	CDlgTemplate templateDlg;
	templateDlg.m_Rect = rect;
	templateDlg.m_bCheckValid = TRUE;
	templateDlg.m_ValidRect.left = m_nOffsetX*m_dPrevScale;
	templateDlg.m_ValidRect.right = (m_nOffsetX+m_nWidth)*m_dPrevScale;
	templateDlg.m_ValidRect.top = m_nOffsetY*m_dPrevScale;
	templateDlg.m_ValidRect.bottom = (m_nOffsetY+m_nHeight)*m_dPrevScale;
	templateDlg.DoModal();

	if(!templateDlg.m_bMakeTemplate)
		return;
	templateDlg.m_TemplateRectTracker.GetTrueRect(&rect);

	CString str;
	int nROILeft, nROIWidth, nROITop, nROIHeight;
	nROILeft = rect.left/m_dPrevScale - m_nOffsetX;
	if(nROILeft < 0)
		nROILeft = 0;
	if(nROILeft%2)					// 不知道为什么不能为奇数
		nROILeft -= 1;
	//nROIWidth = rect.right/m_dPrevScale - nROILeft;
	nROIWidth = (rect.right-rect.left)/m_dPrevScale;
	if(nROIWidth > m_nWidth)
		nROIWidth = m_nWidth;
	nROITop = rect.top/m_dPrevScale - m_nOffsetY;
	if(nROITop < 0)
		nROITop = 0;
	if(nROITop%2)
		nROITop -= 1;
	//nROIHeight = rect.bottom/m_dPrevScale - nROITop;
	nROIHeight = (rect.bottom-rect.top)/m_dPrevScale;
	if(nROIHeight > m_nHeight)
		nROIHeight = m_nHeight;

#if 0
	str.Format("%d", nROILeft);
	WritePrivateProfileString("BASLER", "ROILEFT", str.GetBuffer(0), theApp.m_szConfigFile);
	str.Format("%d", nROIWidth);
	WritePrivateProfileString("BASLER", "ROIWIDTH", str.GetBuffer(0), theApp.m_szConfigFile);
	str.Format("%d", nROITop);
	WritePrivateProfileString("BASLER", "ROITOP", str.GetBuffer(0), theApp.m_szConfigFile);
	str.Format("%d", nROIHeight);
	WritePrivateProfileString("BASLER", "ROIHEIGHT", str.GetBuffer(0), theApp.m_szConfigFile);
#else
	CString strSection;
	strSection.Format("TRIGGERCASE%d", m_nCaseIndex-1);
	str.Format("%d", nROILeft);
	WritePrivateProfileString(strSection.GetBuffer(0), "ROILEFT", str.GetBuffer(0), theApp.m_szConfigFile);
	str.Format("%d", nROIWidth);
	WritePrivateProfileString(strSection.GetBuffer(0), "ROIWIDTH", str.GetBuffer(0), theApp.m_szConfigFile);
	str.Format("%d", nROITop);
	WritePrivateProfileString(strSection.GetBuffer(0), "ROITOP", str.GetBuffer(0), theApp.m_szConfigFile);
	str.Format("%d", nROIHeight);
	WritePrivateProfileString(strSection.GetBuffer(0), "ROIHEIGHT", str.GetBuffer(0), theApp.m_szConfigFile);
#endif
}


void CDlgPreview::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(pScrollBar->m_hWnd == m_SliderGain.m_hWnd){
		int nValue = m_SliderGain.GetPos();
		if(g_pCamera){
			g_pCamera->Gain = nValue;
		}
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CDlgPreview::OnBnClickedCheckRoi()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BTN_SETROI)->EnableWindow(m_CkEnableROI.GetCheck());
	CString str;
	str.Format("TRIGGERCASE%d", m_nCaseIndex-1);
	if(m_CkEnableROI.GetCheck()){
		g_nEnableROI = 1;
		WritePrivateProfileString(str.GetBuffer(0), "SAVEROI", "1", theApp.m_szConfigFile);
	}else{
		g_nEnableROI = 0;
		WritePrivateProfileString(str.GetBuffer(0), "SAVEROI", "0", theApp.m_szConfigFile);
	}
}
