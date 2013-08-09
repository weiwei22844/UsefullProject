#include "stdafx.h"
#include "resource.h"
#include "NewIme.h"
#include "InputWnd.h"
#include "StatusWnd.h"
#include "Global.h"
#include "HandleDic.h"
#include "Assistance.h"
#include "config.h"
/************************************************************************/
/*
/* 函数声明
/*                                                                      */
/************************************************************************/
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	StatusWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT	CALLBACK	CompWndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT CALLBACK	CandWndProc(HWND,UINT,WPARAM,LPARAM);

//////////////////////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	RegisterHotKey(hStatus,GlobalAddAtom("SHIFT"),MOD_SHIFT,NULL);

	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//释放钩子
	UnInstallHook();
	return msg.wParam;
}

/************************************************************************/
/* 
/*  MyRegisterClass()	: 注册窗口类
/*                                                                     */
/************************************************************************/

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	//
	// 注册状态窗口
	//
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)StatusWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= STATUSCLASS;
	wcex.hIconSm		= NULL;

	if(!RegisterClassEx(&wcex))
		return FALSE;

	//
	// 注册输入窗口
	//
	WNDCLASSEX wcex2;
	
	wcex2.cbSize = sizeof(WNDCLASSEX); 
	
	wcex2.style			= CS_HREDRAW | CS_VREDRAW;
	wcex2.lpfnWndProc	= (WNDPROC)InputWndProc;
	wcex2.cbClsExtra		= 0;
	wcex2.cbWndExtra		= 0;
	wcex2.hInstance		= hInstance;
	wcex2.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_NEWIME);
	wcex2.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex2.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex2.lpszMenuName	= NULL;
	wcex2.lpszClassName	= INPUTCLASS;
	wcex2.hIconSm		= NULL;

	if(!RegisterClassEx(&wcex2))
		return FALSE;

	return TRUE;
}

/************************************************************************/
/* 
/*
/*                                                                     */
/************************************************************************/
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;


   //读入配置文件
   LoadConfig();
 
  
   if( !CreateStatus(hInstance))			//创建状态窗口
   {
	   return FALSE;
   }
   
   if (!CreateInputWnd(hInstance))			//创建输入窗口
   {
		return FALSE;
   }
   ShowWindow(hInput,SW_HIDE);				//初始化隐藏输入窗口

   InitConfig();
   //////////////////////////////////////////////////////////////////////////
   //装载拼音信息
   if (!InitPYInfo())
   {
	   return FALSE;
   }

   // 五笔
   if (!InitWBInfo())
   {
	   return FALSE;
   }

   // 词组
   if (!InitCZInfo())
   {
	   return FALSE;
   }

   //英语
   if (!InitEngInfo())
   {
	   return FALSE;
   }

   //笔画
   if (!InitBHInfo())
   {
	   return FALSE;
   }

   //////////////////////////////////////////////////////////////////////////
   //安装钩子
   InstallHook();

   
   
   CurMode=MODE_FULL;

   INPUTWNDSTRUCT.sz.cx=400;
   INPUTWNDSTRUCT.sz.cy=42;

   CANDLIST.nPageSize=7;

   //初始化键盘状态
   KeyState|=HANDLE_CHAR;
   KeyState|=HANDLE_SMALL_NUM;
   //初始化编码列表
   COMPLIST.nCurPos=0;
   COMPLIST.nLen=0;

   AssPrint("初始化成功!");

   ShowWindow(hStatus,SW_SHOWNA);			//显示状态窗口	
   UpdateWindow(hStatus);

   return TRUE;
}