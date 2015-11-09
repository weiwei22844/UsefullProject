// DlgModulePM.cpp : implementation file
//

#include "stdafx.h"
#include "Calibration.h"
#include "DlgModulePM.h"
#include "afxdialogex.h"
#include "CalibrationDlg.h"


// CDlgModulePM dialog

IMPLEMENT_DYNAMIC(CDlgModulePM, CDialogEx)

CDlgModulePM::CDlgModulePM(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgModulePM::IDD, pParent)
{
    m_pModuleData = NULL;
    m_TemplateFile = "";
}

CDlgModulePM::~CDlgModulePM()
{
}

void CDlgModulePM::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PICTURE, m_StaticPic);
}


BEGIN_MESSAGE_MAP(CDlgModulePM, CDialogEx)
    ON_BN_CLICKED(IDOK, &CDlgModulePM::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CDlgModulePM::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BTN_MAKETEMPLATE, &CDlgModulePM::OnBnClickedBtnMaketemplate)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CDlgModulePM message handlers


BOOL CDlgModulePM::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here
    ModulePM *pModulePM = (ModulePM *)m_pModuleData->pData;
    SetDlgItemText(IDC_EDIT_NAME, m_pModuleData->cName);
	m_CurTemplate = pModulePM->cPMTemplate;
	//m_CurTemplate = "D:\\ZWW\\yihongyu\\AutoTestTool\\Calibration\\preTemp.bmp";
    char chFileName[MAX_PATH];
    char *pPos = strrchr(pModulePM->cPMTemplate, '\\');
    if(pPos){
        strcpy(chFileName, pPos+1);
        SetDlgItemText(IDC_EDIT_TEMPLATE, chFileName);          // 只显示文件名不显示路径
    }
    m_StaticPic.ShowWindow(SW_HIDE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgModulePM::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    GetDlgItemText(IDC_EDIT_NAME, m_pModuleData->cName, MODULENAMEMAXLEN-1);
    ModulePM *pModulePMSet = (ModulePM *)m_pModuleData->pData;
    if(strlen(pModulePMSet->cPMTemplate) > 1)
        DeleteFile(pModulePMSet->cPMTemplate);
    strcpy(pModulePMSet->cPMTemplate, m_TemplateFile.GetBuffer(0));
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, MODULEPM_EXIT, 0, 0);
    }else{
        CDialogEx::OnOK();
    }
}


void CDlgModulePM::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    if(!m_TemplateFile.IsEmpty())
        DeleteFile(m_TemplateFile);
    if(g_pMainDlg->IsWindowEnabled()){
        DestroyWindow();
        ::SendMessage(g_pMainDlg->m_hWnd, MODULEPM_EXIT, 0, 0);
    }else{
        CDialogEx::OnCancel();
    }
}


void CDlgModulePM::OnBnClickedBtnMaketemplate()
{
    // TODO: Add your control notification handler code here
    CString str;
    CString fileName;
    SYSTEMTIME st = {0};
    GetLocalTime(&st);
    fileName.Format("Template%04d%02d%02d%02d%02d%02d.bmp", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    str.Format("%s%s\\%s", g_pMainDlg->m_chUserCaseDir, g_pMainDlg->m_CurUserCase.GetBuffer(0), fileName.GetBuffer(0));
    if(g_pMainDlg->MakeTemplate(str)){
        m_TemplateFile = str;
		m_CurTemplate = m_TemplateFile;
		//m_CurTemplate = "D:\\ZWW\\yihongyu\\AutoTestTool\\Calibration\\Grab.bmp";
		showBmp(m_CurTemplate);
        str.Format("创建模板【%s】成功\r\n", fileName.GetBuffer(0));
        g_pMainDlg->ShowOutput(str.GetBuffer(0));
        SetDlgItemText(IDC_EDIT_TEMPLATE, fileName);
    }else{
        g_pMainDlg->ShowOutput("制作模板失败\r\n");
    }
}

bool LoadBmpPic(const CString strBmpPath, CBitmap &bmp)
{
	HBITMAP bitmap = NULL;
	try
	{
		bmp.Detach();
		//从文件路径加载图片
		bitmap =(HBITMAP)::LoadImage(NULL, strBmpPath, IMAGE_BITMAP, 0, 0,
			LR_CREATEDIBSECTION|LR_LOADFROMFILE|LR_DEFAULTSIZE);
		if(!bitmap)
			return false;
		if(bmp.Attach(bitmap)){
			return true;
		}
	}
	catch(...)
	{
		return false;
	}

	return true;
}

void CDlgModulePM::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialogEx::OnPaint()
	showBmp(m_CurTemplate);
}

void CDlgModulePM::showBmp(CString path)
{
	CBitmap bitmap;  // CBitmap对象，用于加载位图   
	HBITMAP hBmp;    // 保存CBitmap加载的位图的句柄
	CString str;
	BOOL bUseHSC = FALSE;
	BOOL bTimeDelay = FALSE;
	if(!LoadBmpPic(path, bitmap))
		return;

	char chTemp[100];
	int nValue;
	hBmp = (HBITMAP)bitmap.GetSafeHandle();
	BITMAP bmpInfo;
	bitmap.GetObject(sizeof(bmpInfo),&bmpInfo);
	int nWidthImg = bmpInfo.bmWidth;
	int nHeightImg = bmpInfo.bmHeight;

	CDC *pDC = GetDC();
	CRect rect;
	CRect rectClient;
	m_StaticPic.GetWindowRect(rect);
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
	pDC->SetStretchBltMode( HALFTONE );
	pDC->StretchBlt(rectClient.left+nPosX, rectClient.top+nPosY, nShowWd, nShowHt, &DCCompatible, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, SRCCOPY );
	DCCompatible.SelectObject(pOldBmp);
}
