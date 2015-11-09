
// Calibration.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "Calibration.h"
#include "CalibrationDlg.h"
#include "Splash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCalibrationApp

BEGIN_MESSAGE_MAP(CCalibrationApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCalibrationApp 构造

CCalibrationApp::CCalibrationApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CCalibrationApp 对象

CCalibrationApp theApp;


// CCalibrationApp 初始化

BOOL CCalibrationApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
    AfxInitRichEdit2();

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	GetModuleFileName(NULL, m_szConfigFile, MAX_PATH);
	(strrchr(m_szConfigFile, '\\'))[1] = 0;         //删除文件名，只获得路径
	strcpy(m_szUserCaseFile, m_szConfigFile);
	strcat(m_szConfigFile, "config.ini");
    TCHAR chTemp[MAX_PATH];
    GetPrivateProfileString("COMMON", "CASEPATH", "", chTemp, MAX_PATH-1, m_szConfigFile);
    DWORD dwAttr = GetFileAttributes(chTemp);
    if(dwAttr == 0xFFFFFFFF){
        strcat(m_szUserCaseFile, "UserCase");
        DWORD dwAttr = GetFileAttributes(m_szUserCaseFile);
        if(dwAttr == 0xFFFFFFFF){						//若文件夹不存在，创建文件夹
            CreateDirectory(m_szUserCaseFile, NULL);
        }else if(dwAttr & FILE_ATTRIBUTE_DIRECTORY){	//文件夹存在
            //do something
        }
    }else{
	    strcpy(m_szUserCaseFile, chTemp);
    }
    if(theApp.m_szUserCaseFile[strlen(theApp.m_szUserCaseFile)-1] == '\\')
        strcat(theApp.m_szUserCaseFile, "usercase.ini");
    else
        strcat(theApp.m_szUserCaseFile, "\\usercase.ini");

	/*CString strPath;
	GetCurrentDirectory(MAX_PATH,strPath.GetBuffer(MAX_PATH));
	strPath.ReleaseBuffer();*/
	GetCurrentDirectory(MAX_PATH, m_imageDirectory);
	strcat(m_imageDirectory, "\\images");
	dwAttr = GetFileAttributes(m_imageDirectory);
	if(dwAttr == 0xFFFFFFFF){						//若文件夹不存在，创建文件夹
		CreateDirectory(m_imageDirectory, NULL);
	}else if(dwAttr & FILE_ATTRIBUTE_DIRECTORY){	//文件夹存在
		//do something
	}

    CSplashWnd::EnableSplashScreen(TRUE);
    CSplashWnd::ShowSplashScreen();
    CCalibrationDlg dlg;
    dlg.PointGreyCameraInit();
	dlg.BaslerCameraInit();
	/*if(!dlg.LedPannelInit())
		AfxMessageBox("LED pannel init failed");*/
    CSplashWnd::HideSplashScreen();

	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

BOOL CCalibrationApp::DeleteDirectory(char* psDirName) 
{ 
    CFileFind tempFind; 
    char sTempFileFind[ _MAX_PATH ] = { 0 }; 
    sprintf(sTempFileFind,"%s//*.*",psDirName); 
    BOOL IsFinded = tempFind.FindFile(sTempFileFind); 
    while (IsFinded) 
    { 
        IsFinded = tempFind.FindNextFile(); 
        if (!tempFind.IsDots())
        { 
            char sFoundFileName[ _MAX_PATH ] = { 0 }; 
            strcpy(sFoundFileName,tempFind.GetFileName().GetBuffer(200)); 
            if (tempFind.IsDirectory()) 
            { 
                char sTempDir[ _MAX_PATH ] = { 0 }; 
                sprintf(sTempDir,"%s//%s",psDirName,sFoundFileName); 
                DeleteDirectory(sTempDir); 
            } 
            else 
            { 
                char sTempFileName[ _MAX_PATH ] = { 0 };
                sprintf(sTempFileName,"%s//%s",psDirName,sFoundFileName); 
                DeleteFile(sTempFileName); 
            } 
        } 
    } 
    tempFind.Close(); 
    if(!RemoveDirectory(psDirName)) 
    { 
        return FALSE; 
    } 
    return TRUE; 
}