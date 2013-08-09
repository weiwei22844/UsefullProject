#include<stdio.h>


//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK	InputWndProc(HWND,UINT,WPARAM,LPARAM);
BOOL				CreateInputWnd(HINSTANCE hInstance);
BOOL				CreateCompWnd(HINSTANCE hInstance);
BOOL				CreateCandWnd(HINSTANCE hInstance);
//////////////////////////////////////////////////////////////////////////
void				MakeCandStr();
void				MakeBHStr(FILE *fp,fpos_t *fpos);
void				MakeCandBHStr(FILE *fp,LONG offest);
void				MakeWBCandStr();
//////////////////////////////////////////////////////////////////////////
void				UpdateInputWnd();
void				Input_OnPaint(HWND hWnd,HDC hdc);
void				Input_OnCreate(HWND hWnd);
void				ClearComp();
void				ClearCand();
//////////////////////////////////////////////////////////////////////////
void				TranspanentWindow(int nOptVal);
void				MakeCiZuCand();