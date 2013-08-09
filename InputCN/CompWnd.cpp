#include "StdAfx.h"
#include "Global.h"
#include "NewIme.h"
/************************************************************************/
/* 
/*
/*                                                                      */
/************************************************************************/

LRESULT CALLBACK CompWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message) 
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		RECT rt;
		GetClientRect(hWnd, &rt);
		DrawText(hdc, "CompWnd", strlen("CompWndPro"), &rt, DT_CENTER);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

/************************************************************************/
/* 
/*
/*                                                                     */
/************************************************************************/

BOOL CreateCompWnd(HINSTANCE hInstance)
{

	RECT	rcInputWnd;
	int		nInputHeight;
	int		nInputWidth;

	GetClientRect(hInput,&rcInputWnd);				//获取输入窗口的大小

	nInputHeight	=rcInputWnd.bottom-rcInputWnd.top;
	nInputWidth		=rcInputWnd.right-rcInputWnd.left;
	nInputHeight	=nInputHeight/2;

	hComp=CreateWindowEx(
						WS_EX_TOPMOST|WS_EX_TOOLWINDOW,
						COMPCLASS,
						NULL,
						WS_POPUP|WS_DISABLED|WS_CHILD,
						0,
						0,
						nInputWidth,
						nInputHeight,
						hInput,
						(HMENU)NULL,
						hInstance,
						NULL
						);
	if (!hComp)
	{
		return FALSE;
	}
	return TRUE;

}

/************************************************************************/
/* 
/*
/*                                                                     */
/************************************************************************/

void ClearComp()
{
	COMPLIST.nCurPos=0;
}