// DlgTriggerSet.cpp : 实现文件
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgTriggerSet.h"
#include "afxdialogex.h"

#include "CalibrationDlg.h"
#include "DlgPreview.h"
#include "Redirect.h"

#define CASE_COUNT 7
static char* g_CaseDescribe[CASE_COUNT] = {
    "L2P 慢击相机图标启动相机，同时触发高速相机进行抓图，目的是测量相机的启动时间！",
    "S2S 启动相机，快击相机的拍照键进行拍照，目的是测量相机的拍照速度！",
    "ZSL 启动相机并同时触发2个LED灯，慢击拍照键，按下/抬起手指时停止1个LED灯，目的是测量快门正负延迟！",
    "VL 无需动作触发，手机处于拍照预览状态，下位机同时触发LED和高速相机，目的是测量预览窗口延迟！",
    "FPS 无需动作触发，手机处于拍照预览状态，下位机同时触发LED和高速相机，目的是测量帧率！",
    "SL 慢击拍照键（按下/抬起时只触发一个LED灯），只能测量正延迟",
    "CS continuous shooting fps连续拍照，触发LED，按下拍照键，延迟，抬起，停止LED",
};
// CDlgTriggerSet 对话框

IMPLEMENT_DYNAMIC(CDlgTriggerSet, CDialogEx)

CDlgTriggerSet::CDlgTriggerSet(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgTriggerSet::IDD, pParent)
{
    m_pModuleData = NULL;
}

CDlgTriggerSet::~CDlgTriggerSet()
{
}

void CDlgTriggerSet::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_CASE, m_ComboBoxCase);
    DDX_Control(pDX, IDC_STATIC_PICTURE, m_StaticPicture);
    DDX_Control(pDX, IDC_EDIT_FILTERLEVEL, m_EditFilterLevel);
    DDX_Control(pDX, IDC_CHECK_TRIGGERMODE, m_CkTriggerMode);
    DDX_Control(pDX, IDC_EDIT_TIMEDELAY, m_EditTimeDelay);
    DDX_Control(pDX, IDC_EDIT_WAVECNT, m_EditWaveCnt);
    DDX_Control(pDX, IDC_COMBO_WAVEFRQ, m_ComBoWaveFrq);
    DDX_Control(pDX, IDC_EDIT_INFO, m_EditInfo);
    DDX_Control(pDX, IDC_EDIT_DBCLICKFREQ, m_EditDBClkFreq);
    DDX_Control(pDX, IDC_EDIT_DBCLKTIMES, m_EditDBClkTimes);
}


BEGIN_MESSAGE_MAP(CDlgTriggerSet, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgTriggerSet::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO_CASE, &CDlgTriggerSet::OnCbnSelchangeComboCase)
	ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_BN_CLICKED(IDC_BTN_HSCPREVIEW, &CDlgTriggerSet::OnBnClickedBtnHscpreview)
    ON_BN_CLICKED(IDCANCEL, &CDlgTriggerSet::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgTriggerSet 消息处理程序


BOOL CDlgTriggerSet::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CString str;
	int nIndex = 0;
	for(int i = 1; i < 8; i++){
		str.Format("TriggerMode%d", i);
		m_ComboBoxCase.InsertString(nIndex++, str.GetBuffer(0));
	}
    m_StaticPicture.ShowWindow(SW_HIDE);
    // 频率设为5档
    for(int i = 0; i < 5; i++){
        str.Format("%dHz", (i+1)*100);
        m_ComBoWaveFrq.InsertString(i, str);
    }

    ModuleTriggerMode *pTriggerModeSet = (ModuleTriggerMode*)m_pModuleData->pData;
    m_ComboBoxCase.SetCurSel(pTriggerModeSet->nTriggerIndex);
    SelectCase(pTriggerModeSet->nTriggerIndex, FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CDlgTriggerSet::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str, strSection;
    UpdateData();
    if(m_EditDBClkFreq.IsWindowEnabled()){
        m_EditDBClkFreq.GetWindowText(str);
        int nQuickFreq = atoi(str.GetBuffer(0));
        if(nQuickFreq > 10){
            MessageBox("快击频率不能大于10");
            return;
        }
    }
    
	ModuleTriggerMode *pTriggerModeSet = (ModuleTriggerMode*)m_pModuleData->pData;

	//GetDlgItemText(IDC_EDIT_NAME, m_pModuleData->cName, MODULENAMEMAXLEN-1);
    pTriggerModeSet->nTriggerIndex = m_ComboBoxCase.GetCurSel();
	pTriggerModeSet->nTriggerMode = m_CkTriggerMode.GetCheck();

    int nValue = m_ComboBoxCase.GetCurSel();
    str.Format("%d", nValue);
    WritePrivateProfileString("TRIGGERSET", "CASEINDEX", str.GetBuffer(0), theApp.m_szConfigFile);
    strSection.Format("TRIGGERCASE%d", nValue);
    m_EditFilterLevel.GetWindowText(str);
    WritePrivateProfileString(strSection.GetBuffer(0), "FILTERLEVEL", str.GetBuffer(0), theApp.m_szConfigFile);
    if(m_CkTriggerMode.GetCheck()){
        WritePrivateProfileString(strSection.GetBuffer(0), "TRIGGERMODE", "1", theApp.m_szConfigFile);
    }else{
        WritePrivateProfileString(strSection.GetBuffer(0), "TRIGGERMODE", "0", theApp.m_szConfigFile);
    }
    m_EditTimeDelay.GetWindowText(str);
	pTriggerModeSet->nTimeDelay = atoi(str.GetBuffer(0));
    WritePrivateProfileString(strSection.GetBuffer(0), "TIMEDELAY", str.GetBuffer(0), theApp.m_szConfigFile);
    nValue = m_ComBoWaveFrq.GetCurSel();
	pTriggerModeSet->nWaveFrq = nValue;
    str.Format("%d", nValue);
    WritePrivateProfileString(strSection.GetBuffer(0), "WAVEFRQ", str.GetBuffer(0), theApp.m_szConfigFile);
    m_EditWaveCnt.GetWindowText(str);
	pTriggerModeSet->nWaveCnt = atoi(str.GetBuffer(0));
    WritePrivateProfileString(strSection.GetBuffer(0), "WAVECNT", str.GetBuffer(0), theApp.m_szConfigFile);
	m_EditDBClkFreq.GetWindowText(str);
	pTriggerModeSet->nDBClkFrq = atoi(str.GetBuffer(0));
	m_EditDBClkTimes.GetWindowText(str);
	pTriggerModeSet->nDBClkCnt = atoi(str.GetBuffer(0));

	CString Command;
	BOOL bSetSuccess = FALSE;
	char strResult[100];
	Command = "LED-Panel-cmd.exe setmode ExposTime";
	CRedirect *pRedirect = new CRedirect((LPTSTR)(LPCTSTR)Command, NULL, ".");
	do{
		/*pRedirect->m_pStrBuff = strResult;
		pRedirect->m_nBuffLen = sizeof(strResult);
		pRedirect->Run();

		pRedirect->m_szCommand = "LED-Panel-cmd.exe getmode";
		pRedirect->Run();
		int nValue = atoi(strResult);
		if(nValue != 2){
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

		bSetSuccess = TRUE;*/
	}while(FALSE);
	if(pRedirect){
		delete pRedirect;
		pRedirect = NULL;
	}

    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, TRIGGERMODE_EXIT, 0, 0);
    }else{
        CDialogEx::OnOK();
    }
}


void CDlgTriggerSet::OnCbnSelchangeComboCase()
{
	// TODO: 在此添加控件通知处理程序代码
	SelectCase(m_ComboBoxCase.GetCurSel());
}

void CDlgTriggerSet::SelectCase(int nIndex, BOOL bShowImage)
{
	CBitmap bitmap;  // CBitmap对象，用于加载位图   
	HBITMAP hBmp;    // 保存CBitmap加载的位图的句柄
    CString str;
    BOOL bUseHSC = FALSE;
	BOOL bTimeDelay = FALSE;
    m_CkTriggerMode.EnableWindow(TRUE);
    SetDlgItemText(IDC_STATIC_WAVECNT, "抓拍个数");
	switch(nIndex)
	{
	case 0:
		bitmap.LoadBitmap(IDB_BITMAP_CASE1);
        bUseHSC = TRUE;
		break;
	case 1:
		bitmap.LoadBitmap(IDB_BITMAP_CASE2);
		bTimeDelay = TRUE;
		break;
	case 2:
		bitmap.LoadBitmap(IDB_BITMAP_CASE3);
		bTimeDelay = TRUE;
		break;
	case 3:
		bitmap.LoadBitmap(IDB_BITMAP_CASE4);
        bUseHSC = TRUE;
		break;
	case 4:
		bitmap.LoadBitmap(IDB_BITMAP_CASE5);
        bUseHSC = TRUE;
		break;
	case 5:
		bitmap.LoadBitmap(IDB_BITMAP_CASE6);
		bTimeDelay = TRUE;
		break;
	case 6:
        bUseHSC = TRUE;
		bTimeDelay = TRUE;
		bitmap.LoadBitmap(IDB_BITMAP_CASE7);
		break;
	default:
        bUseHSC = TRUE;
		bitmap.LoadBitmap(IDB_BITMAP_CASE1);
		break;
	}
    m_EditInfo.SetWindowText(g_CaseDescribe[nIndex]);
    m_ComBoWaveFrq.EnableWindow(bUseHSC);
    m_EditWaveCnt.EnableWindow(bUseHSC);
	m_EditTimeDelay.EnableWindow(bTimeDelay);
	m_EditDBClkFreq.EnableWindow(nIndex == 1);
    m_EditDBClkTimes.EnableWindow(nIndex == 1);

    if(nIndex == 1 || nIndex == 3 || nIndex == 4){                            // 1 快击 3 预览延迟 4 帧率
        m_CkTriggerMode.EnableWindow(FALSE);
    }else if(nIndex == 6){                                                    // 6 连续拍照
        SetDlgItemText(IDC_STATIC_WAVECNT, "持续时间(x10ms)");
        m_CkTriggerMode.EnableWindow(FALSE);
        m_ComBoWaveFrq.EnableWindow(FALSE);
    }

	ModuleTriggerMode *pTriggerModeSet = (ModuleTriggerMode*)m_pModuleData->pData;
    char chTemp[100];
    int nValue;
    str.Format("TRIGGERCASE%d", nIndex);
    m_CkTriggerMode.SetCheck(pTriggerModeSet->nTriggerMode);
	sprintf(chTemp, "%d", pTriggerModeSet->nTimeDelay);
    m_EditTimeDelay.SetWindowText(chTemp);
    m_ComBoWaveFrq.SetCurSel(pTriggerModeSet->nWaveFrq);
    sprintf(chTemp, "%d", pTriggerModeSet->nWaveCnt);
    m_EditWaveCnt.SetWindowText(chTemp);
	sprintf(chTemp, "%d", pTriggerModeSet->nDBClkFrq);
    m_EditDBClkFreq.SetWindowText(chTemp);
	sprintf(chTemp, "%d", pTriggerModeSet->nDBClkCnt);
    m_EditDBClkTimes.SetWindowText(chTemp);

    if(bShowImage){
	    hBmp = (HBITMAP)bitmap.GetSafeHandle();
        BITMAP bmpInfo;
        bitmap.GetObject(sizeof(bmpInfo),&bmpInfo);
        int nWidthImg = bmpInfo.bmWidth;
        int nHeightImg = bmpInfo.bmHeight;

	    //CDC *pDC = m_StaticPicture.GetDC();
        CDC *pDC = GetDC();
	    CRect rect;
        CRect rectClient;
	    m_StaticPicture.GetWindowRect(rect);
        rectClient = rect;
        ScreenToClient(&rectClient);
	    CDC DCCompatible;
	    DCCompatible.CreateCompatibleDC(pDC);
	    CBitmap *pOldBmp = DCCompatible.SelectObject(&bitmap);
    
        int nWidthRc = rect.Width();
        int nHeightRc = rect.Height();
        int nShowWd, nShowHt, nPosX, nPosY;
        if((nWidthImg > nWidthRc ) || (nHeightImg > nHeightRc)){            // 需要伸缩
            // need strech
            if(nHeightImg*nWidthRc > nWidthImg*nHeightRc){                  // 水平方向伸缩
                nShowHt = nHeightRc;
                nShowWd = nShowHt*nWidthImg/nHeightImg;
                nPosX = (nWidthRc - nShowWd)/2;
                nPosY = 0;
            }else{
                nShowWd = nWidthRc;
                nShowHt = nShowWd*nHeightImg/nWidthImg;
                nPosX = 0;
                nPosY = (nHeightRc - nShowHt)/2;
            }
        }else{
            nPosX = (nWidthRc - nWidthImg)/2;
            nPosY = (nHeightRc - nHeightImg)/2;
            nShowWd = nWidthImg;
            nShowHt = nHeightImg;
        }

        pDC->FillSolidRect(rectClient.left, rectClient.top, rect.Width(), rect.Height(), RGB(105,105,105));
        //pDC->Rectangle(0, 0, rect.Width(), rect.Height());
        //DCCompatible.FillRect(&rect, &m_brHatch);
	    pDC->SetStretchBltMode( HALFTONE );
	    pDC->StretchBlt(rectClient.left+nPosX, rectClient.top+nPosY, nShowWd, nShowHt, &DCCompatible, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY );

	    DCCompatible.SelectObject(pOldBmp);
    }
}


void CDlgTriggerSet::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialogEx::OnPaint()
	SelectCase(m_ComboBoxCase.GetCurSel());
}


void CDlgTriggerSet::OnBnClickedBtnHscpreview()
{
    // TODO: Add your control notification handler code here
    CDlgPreview dlgPreview;
    if(g_pMainDlg->m_ulWidthMax > 0){
        dlgPreview.m_nMaxWidth = g_pMainDlg->m_ulWidthMax;
        dlgPreview.m_nMaxHeight = g_pMainDlg->m_ulHeightMax;
        dlgPreview.m_nFps = (m_ComBoWaveFrq.GetCurSel()+1)*100;
		dlgPreview.m_nCaseIndex = m_ComboBoxCase.GetCurSel()+1;
    }
    dlgPreview.DoModal();
}


void CDlgTriggerSet::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, TRIGGERMODE_EXIT, 0, 0);
    }else{
        CDialogEx::OnCancel();
    }
}

