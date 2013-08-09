
#pragma once

#define WIN32_LEAN_AND_MEAN		

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <crtdbg.h>
 
#include "resource.h"
#include <commctrl.h>
#include <Commdlg.h>

//////////////////////////////////////////////////////////////////////////
#pragma comment (lib,"comctl32.lib")
#pragma  comment (lib,"Gdi32.lib")
//////////////////////////////////////////////////////////////////////////
 
#define LEFT_VALUE	0x01
#define RIGHT_VALUE	0x02
#define PAGE1		0x01
#define PAGE2		0x02
#define PAGE3		0x04
#define PAGE4		0x08


//外观设置页
typedef struct _tagSetView
{
	HFONT		hFont;					//字体信息
	COLORREF	rgbFontColor;			//背景颜色
	COLORREF	regBackColor;			//字体颜色
	int			nTransparent;			//透明度
	BOOL		bSkin;					//使用皮肤
	
	char	bmpStatus[80];
	char	bmpChar[80];
	char	bmpInput[80];
	char	bmpEng[80];
	
}SETVIEWSTRUCT,*LPSETVIEWSTRUCT;

//热键设置页
typedef struct _tagSetHotKey
{
	DWORD	dwControl;
	DWORD	dwShift;
	DWORD	dwPage;

}SETHOTKEYSTRUCT;
//////////////////////////////////////////////////////////////////////////
//	SETTING
extern	SETVIEWSTRUCT	SETVIEW;		//设置外观	
extern	SETHOTKEYSTRUCT	SETKEY;
//////////////////////////////////////////////////////////////////////////
int DoPropertySheet(HWND hWnd);
BOOL WINAPI Config_setkey(HWND hWnd,UINT uMsg, WPARAM wParam,LPARAM lParam);	// 后台设置，设置热键的函数
BOOL WINAPI Config_setusual(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);	//后台设置，设置常规功能的函数
BOOL WINAPI Config_setview(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);	// 后台设置，设置外观的功能函数
BOOL WINAPI Config_setciku(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);	//后台设置， 设置词库功能的函数

void LoadConfig();
void InitConfig();
