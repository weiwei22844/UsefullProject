
// CalibrationDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "C/FlyCapture2_C.h"
#include "CvvImage.h"
#include "btnst.h"
#include "XTreeCtrl.h"
#include "RichEditEx.h"

#define ROBOTDLG_EXIT       WM_USER+1001
#define PMSETDLG_EXIT		WM_USER+1002

#define INITSETTING_EXIT    WM_USER+1010
#define MODULEPM_EXIT       WM_USER+1011
#define FINGERMOVE_EXIT     WM_USER+1012
#define ICONDRAG_EXIT       WM_USER+1013
#define TRIGGERMODE_EXIT    WM_USER+1014
#define ROTATE_EXIT         WM_USER+1015
#define CIRCLETIMES_EXIT	WM_USER+1016
#define TIMEDELAY_EXIT      WM_USER+1017

class CDlgRobotSet;
class CDlgPMSet;
class CDlgInitSetting;
class CDlgModulePM;
class CDlgFingerMove;
class CDlgIconDrag;
class CDlgTriggerMode;
class CDlgTriggerSet;
class CDlgRotate;
class CDlgCircleTimes;
class CDlgTimeDelay;

// CCalibrationDlg 对话框
class CCalibrationDlg : public CDialogEx
{
// 构造
public:
	CCalibrationDlg(CWnd* pParent = NULL);	// 标准构造函数

	void EnableItem(int nType = 0, BOOL bEnable = TRUE);
	BOOL PointGreyCameraInit();
	BOOL PointGreyGrabImage();
	BOOL PointGreySaveImage(char* dst_file_name);
	BOOL PointGreyGetImageData(unsigned char ** target_image_memdata);
	BOOL PointGreyTerminate();
	int PointGreyGetImageWidth();
	int PointGreyGetImageHeight();

	BOOL MakeTemplate(CString fileName, BOOL bSpecifyRect = TRUE);

    UINT DoGrabLoop();
    static UINT ThreadGrabImage( void* pparam );

	BOOL BaslerCameraInit();
	BOOL startStorageThread();
	BOOL LedPannelInit();

    void AddItem(int nType, HTREEITEM hCurTreeItem = NULL);
    LRESULT ShowOutput(char *info);

	BOOL TemplatePM(char *pTempFile, LONG &lPMPosX, LONG &lPMPosY);
	BOOL WaitForPosition(LONG lPosX, LONG lPosY, int &nStatus, int nTimeOut = 5);
	BOOL RunModule(int nModuleType, char *strModule, char *chModuleName, CString strConfig);
	BOOL LoadModule(int nModuleType, char *strModule, ModuleData *pModuleData, CString strConfig);
	void CaseSerialize(CString caseName);
    BOOL InitUserCase();
    BOOL RunMatchThread();
    void StopMatchThread();

	BOOL m_bSdkInit;
	LONG m_lLoginID;
	DOUBLE m_dScale;
    LONG m_lPrePosX;
    LONG m_lPrePosY;
	LONG m_lPrePosZ;
    LONG m_lCurPosZ;
    LONG m_lMarkPointX;
    LONG m_lMarkPointY;

    LONG m_lCamLaserShiftX;
    LONG m_lCamLaserShiftY;
    LONG m_lFingerLaserShiftX;
    LONG m_lFingerLaserShiftY;
	LONG m_lQuickClickLaserShiftX;
	LONG m_lQuickClickLaserShiftY;

	BOOL m_bMoveIcon;

	// for pointgrey cam
	int m_Inited;
	char m_localCamIp[4];
	fc2Context m_Context;
	fc2PGRGuid m_Guid;
	fc2Image m_RawImage;
	fc2Image m_ConvertedImage;
	fc2Error m_Error;

	int m_nCamWidth;
	int m_nCamHeight;
	int m_nWidth;				// 识别摄像机缩放宽
	int m_nHeight;				// 识别摄像机缩放高

	BOOL m_bPreview;
	HANDLE hThreadCaptureDisplay;
	DWORD ThreadCaptureDisplayID;
    int m_nEnableROI;

	// PM
	BOOL m_bMatch;
    HANDLE m_hThreadMatch;
	BOOL m_bReloadTemplate;
	CString m_cCurrentMatchFile;
	BYTE* m_pYData;
	BOOL m_bPMProcessing;                       // 用于指示PM是否进行中
	float m_fMatchPosX[4];
	float m_fMatchPosY[4];
	INT m_nPMType;
	HANDLE m_MatchEvent;

	BOOL m_bDoSearch;
	BOOL m_bCalibration;

	DOUBLE m_dHorizontalRatio;
	DOUBLE m_dVerticalRatio;

	BOOL m_bLocateTemplate;

	// for C++ camera
    CCriticalSection m_csData;              // Critical section to protect access to the processed image
	FlyCapture2::CameraBase* m_pCamera;
	FlyCapture2::CameraInfo m_cameraInfo;
	FlyCapture2::EmbeddedImageInfo m_EmbeddedInfo;
	FlyCapture2::CameraControlDlg m_camCtlDlg;

    FlyCapture2::Image m_rawImage;
    FlyCapture2::Image m_processedImage;

    HANDLE m_heventThreadDone;

    CString m_ProgramInfo;

	CDlgRobotSet *m_pRobotSetDlg;
	CDlgPMSet	 *m_pPMSetDlg;
	CPoint m_downPoint;
	CPoint m_upPoint;

	// 测量相机
	BOOL m_bBaslerInit;
	ULONG m_ulWidthMax;
	ULONG m_ulHeightMax;
	BOOL m_bStorageThread;
	DWORD m_dwBaslerThreadId;
	int m_nTriggerFlag;						// 指示当前的触发模式是快击还是慢击
	HANDLE m_hEventCapReady;
    WORD m_waveFrequency;
    WORD m_waveTimes;
	BOOL m_bPicSave;

    //
    CImageList m_TreeImages;
	CString m_CurUserCase;
    CDlgInitSetting *m_pInitSetDlg;
    CDlgModulePM *m_pModulePMDlg;
    CDlgFingerMove *m_pFingerMoveDlg;
    CDlgIconDrag *m_pIconDragDlg;
    //CDlgTriggerMode *m_pTriggerModeDlg;
    CDlgTriggerSet *m_pTriggerSetDlg;
    CDlgRotate *m_pRotateDlg;
	CDlgCircleTimes *m_pCircleTimesDlg;
    CDlgTimeDelay *m_pTimeDelayDlg;

    char m_chUserCaseDir[MAX_PATH];
	int m_nCircleTimes;						// 循环测试列表循环次数

	BOOL m_bCaseRun;
	LONG m_lPMPosX;
	LONG m_lPMPosY;
	BOOL m_bIsCaseEdit;
    int m_nCurCircleIndex;

#ifdef USEMIL
	MIL_ID m_MilApplication;     /* Application identifier. */
	MIL_ID m_MilSystem;          /* System identifier.      */
	MIL_ID m_MilDisplay;         /* Display identifier.     */
#endif

// 对话框数据
	enum { IDD = IDD_CALIBRATION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	//FlyCapture2::CameraControlDlg m_camCtlDlg;

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnConnect();
	afx_msg void OnDestroy();
	CStatic m_VideoWnd;
	afx_msg void OnBnClickedBtnPreview();
	afx_msg void OnBnClickedBtnStoppreview();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnMaketemp();
	afx_msg void OnBnClickedBtnSearch();
    afx_msg void OnBnClickedBtnToprepos();
    afx_msg void OnBnClickedBtnQuerypos();
	afx_msg void OnBnClickedBtnCalibration();
	afx_msg void OnBnClickedBtnReset();
	afx_msg void OnBnClickedCancel();
	CButtonST m_BtnUp;
	CButtonST m_BtnRight;
	CButtonST m_BtnDown;
	CButtonST m_BtnLeft;
	afx_msg void OnBnClickedBtnRobotset();
	afx_msg void OnBnClickedBtnPmset();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedBtnTriggerset();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedBtnTest();
	afx_msg void OnBnClickedBtnTrigger();
	afx_msg void OnBnClickedBtnMoveicon();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    CXTreeCtrl m_TreeInit;
    CXTreeCtrl m_TreeCircle;
    afx_msg void OnBnClickedBtnNewcase();
    CButton m_RadioInit;
    CButton m_RadioCircle;
    afx_msg void OnBnClickedRadioInit();
    afx_msg void OnBnClickedRadioCircle();
    afx_msg void OnNMRClickTreeInit(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedBtnInitsetting();
    afx_msg void OnBnClickedBtnDragicon();
    afx_msg void OnBnClickedBtnPm();
    afx_msg void OnBnClickedBtnFingermove();
    afx_msg void OnBnClickedBtnTrrigermode();
    afx_msg void OnBnClickedBtnComplete();
	afx_msg void OnTreeCtrlRename();
	afx_msg void OnNMRClickTreeCircle(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDeleteTreeItem();
    afx_msg void OnSetTreeItem();
    CRichEditEx m_RichEditOutput;
    afx_msg void OnNMDblclkTreeInit(NMHDR *pNMHDR, LRESULT *pResult);
	CComboBox m_comboUsercase;
    afx_msg void OnNMDblclkTreeCircle(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedBtnRotate();
	afx_msg void OnSetCircleTimes();
	afx_msg void OnBnClickedBtnCasedetail();
	afx_msg void OnBnClickedBtnDelcase();
	afx_msg void OnBnClickedBtnRuncase();
	afx_msg void OnBnClickedBtnStopcase();;
	afx_msg void OnBnClickedBtnClick();
	afx_msg void OnBnClickedBntCanclenew();
	afx_msg void OnBnClickedBtnCasecomplete();
    afx_msg void OnBnClickedBtnCancleedit();
    afx_msg void OnBnClickedBtnRotatetoprepos();
    afx_msg void OnBnClickedBtnTimedelay();
    afx_msg void OnInsertTreeItem();
};

extern CCalibrationDlg *g_pMainDlg;
