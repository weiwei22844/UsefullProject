#include "StdAfx.h"
#include "Global.h"
#include "HandleDic.h"
#include "InputWnd.h"
#include "Assistance.h"
#include "HandleMsg.h"
#include "StatusWnd.h"
/************************************************************************
/* 
/* HandleMsg ()	: 判断该消息是否需要处理，返回真要处理，返回假不处理
/*                                                                      */
/************************************************************************/
BOOL HandleMsg(WPARAM wParam,LPARAM lParam)
{
	if (wParam == VK_BACK)						//删除键
	{
		if(KeyState & HANDLE_BACK)
		{
			return TRUE;
		}
		else
		{
			if (GetFrontWnd(&hForeWnd))
			{
				PostMessage(hForeWnd,WM_KEYDOWN,wParam,lParam);
				PostMessage(hForeWnd,WM_CHAR,wParam,lParam);
				PostMessage(hForeWnd,WM_KEYUP,wParam,lParam);
			}
			return FALSE;
		}
	}
	else if(wParam == VK_SPACE)				   	//空格键
	{
		if (KeyState & HANDLE_SPACE)
		{
			return TRUE;
		}
		else
		{
			if (GetFrontWnd(&hForeWnd))
			{
				PostMessage(hForeWnd,WM_CHAR,wParam,lParam);
			}

			return FALSE;
		}
	}

	else if (wParam >= 0x30 && wParam <= 0x39 )	// 0 - 9
	{
		if (KeyState & HANDLE_NUM)
		{
			return TRUE;
		}
		else
		{
			if (GetFrontWnd(&hForeWnd))
			{
				PostMessage(hForeWnd,WM_CHAR,wParam,lParam);
			}
			return FALSE;
		}
	}

	else if (wParam == VK_LEFT)					//LEFT
	{
		if (KeyState & HANDLE_LEFT)
		{
			return TRUE;
		}
		else
		{
			if (GetFrontWnd(&hForeWnd))
			{
				PostMessage(hForeWnd,WM_KEYDOWN,wParam,lParam);
				PostMessage(hForeWnd,WM_KEYUP,wParam,lParam);
			}
			return FALSE;
		}
	}

	else if (wParam == VK_RIGHT)				//RIGHT
	{
		if (KeyState & HANDLE_RIGHT)
		{
			return TRUE;
		}
		else
		{
			if (GetFrontWnd(&hForeWnd))
			{
				PostMessage(hForeWnd,WM_KEYDOWN,wParam,lParam);
				PostMessage(hForeWnd,WM_KEYUP,wParam,lParam);
			}
			return FALSE;
		}
	}

	else if (wParam == VK_DOWN)					//DOWN
	{
		if (KeyState & HANDLE_DOWN)
		{
			return TRUE;
		}
		else
		{
			if (GetFrontWnd(&hForeWnd))
			{
				PostMessage(hForeWnd,WM_KEYDOWN,wParam,lParam);
				PostMessage(hForeWnd,WM_KEYUP,wParam,lParam);
			}
			return FALSE;
		}
	}

	else if (wParam == VK_UP)					//UP
	{
		if (KeyState & HANDLE_UP)
		{
			return TRUE;
		}
		else
		{
			if (GetFrontWnd(&hForeWnd))
			{
				PostMessage(hForeWnd,WM_KEYDOWN,wParam,lParam);
				PostMessage(hForeWnd,WM_KEYUP,wParam,lParam);
			}
			return FALSE;
		}
	}
	else if (wParam >= VK_NUMPAD4 && wParam<= VK_NUMPAD9)
	{
		if (KeyState & HANDLE_SMALL_NUM)
		{
			return TRUE;
		}
		else
		{
			if (GetFrontWnd(&hForeWnd))
			{
				PostMessage(hForeWnd,WM_KEYDOWN,wParam,lParam);
				PostMessage(hForeWnd,WM_KEYUP,wParam,lParam);
			}
			return FALSE;
		}
	}
	else if (wParam >= 0x41 && wParam <= 0x5A )	// A - Z
	{
		if (KeyState & HANDLE_CHAR)
		{
			return TRUE;
		}
		else
		{
			if (GetFrontWnd(&hForeWnd))
			{
				PostMessage(hForeWnd,WM_CHAR,wParam,lParam);
				//PostMessage(hForeWnd,WM_KEYUP,wParam,lParam);
			}
			return FALSE;
		}
	}
	else if (wParam == 0xBC )//逗号
	{
		if (GetFrontWnd(&hForeWnd))
		{
			PostMessage(hForeWnd,WM_CHAR,(WPARAM)44,lParam);
		}
		return FALSE;
	}
	else if (wParam == 0xBE)//句号
	{
		if (GetFrontWnd(&hForeWnd))
		{
			PostMessage(hForeWnd,WM_CHAR,(WPARAM)46,lParam);
		}
		return FALSE;
	}
	else if (wParam == 0xBF) //  /?
	{
		if (GetFrontWnd(&hForeWnd))
		{
			PostMessage(hForeWnd,WM_KEYDOWN,wParam,lParam);
			PostMessage(hForeWnd,WM_CHAR,(WPARAM)44,lParam);
			PostMessage(hForeWnd,WM_KEYUP,wParam,lParam);
		}
		return FALSE;
	}
	else if (wParam == VK_CONTROL)
		return TRUE;

	return FALSE;
}

/************************************************************************/
/* 
/*	HandleChar()	: 处理 A - Z
/*	wParam			: 虚拟键
/*	lParam			: 辅助消息
/*	                                                                    */
/************************************************************************/

void HandleChar(WPARAM wParam,LPARAM lParam)
{

	CHAR		ReadAChar;								//读入的字符
	ReadAChar=(CHAR)(wParam+32);
	COMPLIST.Buffer[COMPLIST.nCurPos++]	=ReadAChar;		//向编码串加入一个字符
	COMPLIST.Buffer[COMPLIST.nCurPos]='\0';
	COMPLIST.nLen=COMPLIST.nCurPos;						//设置长度


	if (CurMode & MODE_FULL)
	{
		/*  分析已输入的英文  词组输入*/
		ResultMode=0;										//清空所有状态
		if(SearchWB())
		{
			ResultMode|=MODE_WUBI;
		}
		if (SearchEng())
		{
			ResultMode|=MODE_ENGLIS;
		}
		if (SearchPY())
		{
			ResultMode|=MODE_PINYIN;
		}
		else if( SearchPYCZ() )
		{
			ResultMode|=MODE_CIZU;
		}

		MakeCandStr();	//生成候选列表
	}

	else if (CurMode & MODE_ENGLIS)					//单英语模式
	{
		SearchEnglish();
	}

	else if (CurMode & MODE_PINYIN)					//单拼音模式
	{
		ResultMode=0;
		if(SearchPY())
		{
			ResultMode|=MODE_PINYIN;
		}
		else if (SearchPYCZ())
		{
			ResultMode|=MODE_CIZU;
		}
		MakeCandStr();
	}

	else if (CurMode & MODE_WUBI)					//单五笔模式
	{
		if (SearchSingleWB())
		{
			MakeWBCandStr();
		}
	}

	//按键状态转换
	KeyState=0;
	KeyState|=HANDLE_NUM;
	KeyState|=HANDLE_BACK;
	KeyState|=HANDLE_SPACE;
	KeyState|=HANDLE_LEFT;
	KeyState|=HANDLE_RIGHT;
	KeyState|=HANDLE_UP;
	KeyState|=HANDLE_CHAR;
	KeyState|=HANDLE_DOWN;


	//更改输入窗口状态
	INPUTWNDSTRUCT.bShow=TRUE;
	//更新显示
	UpdateInputWnd();
}

/************************************************************************/
/* 
/*	HandleBack()	: 处理删除键
/*                                                                      */
/************************************************************************/
void HandleBack()
{
	if (COMPLIST.nLen > 0 )
	{
		COMPLIST.nLen -- ;
		COMPLIST.nCurPos--;
		COMPLIST.Buffer[COMPLIST.nLen]='\0';
	}

	if (COMPLIST.nLen == 0)
	{
		KeyState=0;						//清空所有状态
		KeyState|=HANDLE_CHAR;
		KeyState|=HANDLE_SMALL_NUM;
		
		INPUTWNDSTRUCT.bShow=FALSE;
		UpdateInputWnd();
		return;
	}
	//////////////////////////////////////////////////////////////////////////
	
	KeyState	=0;				//清空按键状态

	if (CurMode & MODE_FULL)
	{
		ResultMode	=0;
		if (SearchWB())	
		{
			ResultMode|=MODE_WUBI;
		}

		if (SearchPY())
		{
			ResultMode|=MODE_PINYIN;
		}
		
		MakeCandStr();
	}


	else if (CurMode & MODE_BIHUA)
	{
		FILE *fp;

		LONG offest=SearchBH(&fp);
		
		if (offest == -1) //找不到相应的字
		{
			return ;
		}

		MakeCandBHStr(fp,offest);
	}

	else if (CurMode & MODE_PINYIN)
	{
		ResultMode =0;
		if (SearchPY())
		{
			ResultMode|=MODE_PINYIN;
		}
		else if (SearchPYCZ())
		{
			ResultMode|=MODE_CIZU;
		}

		MakeCandStr();
	}


	//更新按键盘状态
	KeyState|=HANDLE_CHAR;
	KeyState|=HANDLE_BACK;
	KeyState|=HANDLE_SPACE;
	KeyState|=HANDLE_NUM;
	KeyState|=HANDLE_LEFT;
	KeyState|=HANDLE_RIGHT;

	//更改输入窗口状态
	INPUTWNDSTRUCT.bShow=TRUE;
	//更新显示
	UpdateInputWnd();
}

/************************************************************************/
/* 
/* HandleSpace()	: 处理空格键盘      
/*                                                                      */
/************************************************************************/
void HandleSpace()
{
	if (CurMode&MODE_ENGLIS)
	{
		TranslateEng(1);
	}
	else
	{
		TranslateResult(1);
	}

	KeyState=0;						//清楚所有状态
	KeyState|=HANDLE_CHAR;			//设置普通字母输入
	KeyState|=HANDLE_SMALL_NUM;

	INPUTWNDSTRUCT.bShow=FALSE;

	ClearCand();
	ClearComp();

	UpdateInputWnd();				//更新输入窗口
}

/************************************************************************/
/* 
/* 处理输入的数字
/*                                                                     */
/************************************************************************/

void HandleNum(WPARAM wParam,LPARAM lParam)
{
	int SelectNum=wParam-0x30;

	if (SelectNum <=0 || SelectNum > CANDLIST.nPageSize )
	{
		return;
	}

	if (CurMode&MODE_ENGLIS)
	{
		TranslateEng(SelectNum);
	}
	else
	{
		TranslateResult(SelectNum);
	}


	KeyState=0;						//清楚所有状态
	KeyState|=HANDLE_CHAR;			//设置普通字母输入
	KeyState|=HANDLE_SMALL_NUM;		//设置笔画输入

	INPUTWNDSTRUCT.bShow=FALSE;
	ClearCand();
	ClearComp();
	UpdateInputWnd();				//更新输入窗口
}

/************************************************************************/
/* 
/* 几种输入法的切换
/*                                                                     */
/************************************************************************/

void HandleControl()
{
	ClearComp();
	ShowWindow(hInput,SW_HIDE);

	if (CurMode>= MODE_FULL)
	{
		CurMode=MODE_PINYIN;

	}
	else
	{
		CurMode<<=1;
		if (CurMode == MODE_CIZU)
		{
			CurMode<<=1;
		}
	}

	dwType=0;
	dwType|=TYPE_ZHONGWEN;		//中文 
	dwType|=TYPE_CHUANGJIAO;	//全角
	dwType|=TYPE_BIAO1;			//标点1
	
	switch(CurMode)			//转换模式
	{
	case MODE_FULL:
		{
			dwType|=TYPE_FULL;
			KeyState=0;
			KeyState|=HANDLE_CHAR;
			KeyState|=HANDLE_SMALL_NUM;

		}
		break;
	case MODE_ENGLIS:
		{
			dwType|=TYPE_ENG;
			KeyState=0;
			KeyState|=HANDLE_CHAR;
		}
		break;
	case MODE_PINYIN:
		{
			dwType|=TYPE_PINYIN;
			KeyState=0;
			KeyState|=HANDLE_CHAR;
		}
		break;
	case MODE_WUBI:
		{
			dwType|=TYPE_WUBI;
			KeyState=0;
			KeyState|=HANDLE_CHAR;
		}
		break;
	case MODE_BIHUA:
		{
			dwType|=TYPE_BIHUA;
			KeyState=0;
			KeyState|=HANDLE_SMALL_NUM;
		}
		break;
	default:
		break;
	}
	if (CurMode & MODE_ENGLIS)
	{
		INPUTWNDSTRUCT.sz.cx=150;
		INPUTWNDSTRUCT.sz.cy=200;
	}
	else
	{
		INPUTWNDSTRUCT.sz.cx=400;
		INPUTWNDSTRUCT.sz.cy=42;
	}

	HDC hdc=GetDC(hStatus);
	Status_OnPaint(hStatus,hdc);
	ReleaseDC(hStatus,hdc);
}


/************************************************************************/
/* 
/*
/*                                                                      */
/************************************************************************/

void HandleRight()
{
}

/************************************************************************/
/* 
/*
/*                                                                     */
/************************************************************************/
void HandLeft()
{
}

/************************************************************************/
/* 
/*
/*                                                                     */
/************************************************************************/
void Handle_Small_Num(WPARAM wParam)
{
	FILE *fp;
	
	int num=wParam - VK_NUMPAD0;
	
	if (num >=7 && num <=9)
	{
		num -= 6;
	}
	
	COMPLIST.Buffer[COMPLIST.nCurPos++]=(char)(num+48);
	COMPLIST.nLen=COMPLIST.nCurPos;
	COMPLIST.Buffer[COMPLIST.nCurPos]='\0';

	
	LONG offest=SearchBH(&fp);
	
	if (offest == -1) //找不到相应的字
	{
		return ;
	}
	
	MakeCandBHStr(fp,offest);
	
	//更新按键盘状态
	
	KeyState|=HANDLE_BACK;
	KeyState|=HANDLE_SPACE;
	KeyState|=HANDLE_NUM;
	KeyState|=HANDLE_LEFT;
	KeyState|=HANDLE_RIGHT;
	
	INPUTWNDSTRUCT.bShow=TRUE;
	
	UpdateInputWnd();
}

/************************************************************************/
/* 
/* AddBHtoComp() : 添加一笔到编码表
/*                                                                     */
/************************************************************************/
void AddBHtoComp(char ch)
{
	COMPLIST.Buffer[COMPLIST.nCurPos++]=ch;
}

/************************************************************************/
/* 
/* 发送结果串到目标程序
/*                                                                      */
/************************************************************************/

void TranslateResult(int num)
{
	
	if (!GetFrontWnd(&hForeWnd))
	{
		return;
	}
	
	int pos=0;

	if (num == 1)				//输入出第一个
	{
		pos+=2;
	}
	else if(num > 1 )			//输出下面几个
	{
		int FindNum=num-1;

		//找到对应的开始位置
		while (TRUE)
		{
			if (CANDLIST.Buff[pos] == ' ')
			{
				pos+=2;
				FindNum--;
				if(!FindNum)
					break;
			}
			else
			{
				pos++;
			}
		}
		pos+=2;				//指向中文字开始位置g
	}
	
	DWORD dwTransMsg;		//中文字的编码

	LPCSTR lpCandStr=CANDLIST.Buff;

	while( *(lpCandStr+pos) != ' ')
	{
		dwTransMsg=(unsigned char)*(lpCandStr+pos);
		dwTransMsg<<=8;
		dwTransMsg+=(unsigned char)*(lpCandStr+pos+1);
		PostMessage(hForeWnd,WM_IME_CHAR,dwTransMsg,1);		//发送中文字到程序
		pos+=2;
	}
}


/************************************************************************/
/* 
/* 发送英文串到目标程序
/*                                                                     */
/************************************************************************/

void TranslateEng(int num)
{

	if (!GetFrontWnd(&hForeWnd))
	{
		return;
	}

	LPSTR lpResult=&ENGINFO.strList[num-1][2];

	while (*lpResult != ' ')
	{
		PostMessage(hForeWnd,WM_CHAR,(WPARAM)*lpResult,1);
		lpResult++;
	}
}



/************************************************************************/
/* 
/*  拼音模式下向上翻页
/*                                                                      */
/************************************************************************/
void PYPageUp()
{
}

/************************************************************************/
/* 
/* 拼音模式下向下翻页
/*                                                                     */
/************************************************************************/

void PYPageDown()
{
}
