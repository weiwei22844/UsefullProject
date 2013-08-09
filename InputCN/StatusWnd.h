#ifndef STATUS_H
#define STATUS_H

#pragma comment(lib,"HOOKDLL.lib")
#pragma comment(lib,"User32.lib")

#define DllImport __declspec(dllimport)

//////////////////////////////////////////////////////////////////////////
//
// 导入函数
DllImport void		InstallHook();
DllImport void		UnInstallHook();

//////////////////////////////////////////////////////////////////////////
//
// 常量定义

#define		BORDER_WIDTH	4

#define		TYPE_ENG		0x01
#define		TYPE_WUBI		0x02
#define		TYPE_PINYIN		0x04
#define		TYPE_BIHUA		0x08
#define		TYPE_FULL		0x010
#define		TYPE_ZHONGWEN	0x020
#define		TYPE_YINGYU		0x040
#define		TYPE_CHUANGJIAO	0x080
#define		TYPE_BANJIAO	0x0100
#define		TYPE_BIAO1		0x0200
#define		TYPE_BIAO2		0x0400
//////////////////////////////////////////////////////////////////////////
//
// 函数声明
LRESULT CALLBACK	StatusWndProc	(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL				CreateStatus	(HINSTANCE hInst);
void	Status_OnPaint(HWND hWnd,HDC hdc);

void	Status_OnCreate();
#endif
