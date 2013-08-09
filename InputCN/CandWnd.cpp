#include "StdAfx.h"
#include "NewIme.h"
#include "Global.h"
#include "Assistance.h"
#include <stdio.h>
#include "InputWnd.h"
/************************************************************************/
/* 
/* CandWndProc()	:	候选窗口消息处理过程
/*                                                                      */
/************************************************************************/

LRESULT	CALLBACK	CandWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (uMsg) 
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
		return DefWindowProc(hWnd,uMsg, wParam, lParam);
	}
	return 0;
}

/************************************************************************/
/* 
/* CreateCandWnd() :	创建候选窗口
/*                                                                     */
/************************************************************************/

BOOL CreateCandWnd(HINSTANCE hInstance)
{

	RECT	rcInputWnd;
	int		nInputHeight;
	int		nInputWidth;

	GetClientRect(hInput,&rcInputWnd);				//获取输入窗口的大小

	nInputHeight	=rcInputWnd.bottom-rcInputWnd.top;
	nInputWidth		=rcInputWnd.right-rcInputWnd.left;
	nInputHeight	=nInputHeight/2;

	hCand=CreateWindowEx(
						WS_EX_TOPMOST|WS_EX_TOOLWINDOW,
						COMPCLASS,
						NULL,
						WS_POPUP|WS_DISABLED|WS_CHILD,
						0,
						nInputHeight,
						nInputWidth,
						nInputHeight,
						hInput,
						(HMENU)NULL,
						hInstance,
						NULL
						);
	if (!hCand)
	{
		return FALSE;
	}

	return TRUE;

}



/************************************************************************/
/* 
/* MakeCandStr()	: 生成候选列表串卡
/*                                                                     */
/************************************************************************/

void MakeCandStr()
{
	int nPageSize=CANDLIST.nPageSize;
	int nCandPos=0;
	int num=1;									//候选列表的选择数字

	// 1. 添加五笔候选列表.

	if (ResultMode & MODE_WUBI)
	{
		LPSTR	lpWB=WBINFO.strResult;			//指向五笔的候选列表

		CANDLIST.Buff[nCandPos++]	=(char)(48+num);
		CANDLIST.Buff[nCandPos++]	='.';

		while (*lpWB != '\0')
		{
			CANDLIST.Buff[nCandPos++]=*lpWB++;
		}

		CANDLIST.Buff[nCandPos++]	=' ';
		CANDLIST.Buff[nCandPos++]	=' ';

		num++;
	} //if

	// 2. 添加英语候选列表.
	// 如果单拼没有就添加所有英语，一般情问供下添加2个。谁
		
	if (ResultMode & MODE_ENGLIS )
	{
		LPSTR	lpEng=ENGINFO.strRestult;
		if (ResultMode & MODE_PINYIN)
		{
			CANDLIST.Buff[nCandPos++]=(char)(48+num);
			CANDLIST.Buff[nCandPos++]='.';
			while (*lpEng != ' ' && *lpEng !='\0')
			{
				CANDLIST.Buff[nCandPos++]=*lpEng++;
			}
			CANDLIST.Buff[nCandPos++]=' ';
			CANDLIST.Buff[nCandPos++]=' ';
			num++;
	
			if (*lpEng !='\0')
			{
				CANDLIST.Buff[nCandPos++]=(char)(48+num);
				CANDLIST.Buff[nCandPos++]='.';
				lpEng++;
				while (*lpEng != ' ' && *lpEng != '\0')
				{
					CANDLIST.Buff[nCandPos++]=*lpEng++;
				}
				CANDLIST.Buff[nCandPos++]=' ';
				CANDLIST.Buff[nCandPos++]=' ';
				num++;
			}//if
		}//if
		else	//添加全部英文
		{
			while (num <= nPageSize)
			{
				CANDLIST.Buff[nCandPos++]	=(char)(48+num);
				CANDLIST.Buff[nCandPos++]	='.';
				while (*lpEng!=' ' &&  *lpEng!='\0')
				{
					CANDLIST.Buff[nCandPos++]	=*lpEng++;
				}
				if (*lpEng++ == '\0')
				{
					break;
				}
				CANDLIST.Buff[nCandPos++]	=' ';
				CANDLIST.Buff[nCandPos++]	=' ';
				num++;
					
			}//while

		}//else
		
	}
	// 3.添加单拼音
	if (ResultMode & MODE_PINYIN)
	{
		LPSTR lpPY=PYINFO.lpStr;

		while (num <= nPageSize)
		{
			CANDLIST.Buff[nCandPos++]	=(char)(48+num);
			CANDLIST.Buff[nCandPos++]	='.';
			CANDLIST.Buff[nCandPos++]	=*lpPY++;
			CANDLIST.Buff[nCandPos++]	=*lpPY++;
			CANDLIST.Buff[nCandPos++]	=' ';
			CANDLIST.Buff[nCandPos++]	=' ';
				
			num++;

			if (*lpPY == '\0' || *(lpPY+1) =='\0')
			{
				break;
			}
		}//while	
		PYINFO.lpStr=lpPY;
	}

	// 4.添加拼音词组
	if(ResultMode & MODE_CIZU)
	{
		LPSTR	lpCZ=CIZUINFO.strResult;
		BOOL	bFalg=TRUE;
		while (num <= nPageSize && bFalg)
		{
			CANDLIST.Buff[nCandPos++]	=(char)(48+num);
			CANDLIST.Buff[nCandPos++]	='.';

			while(TRUE)
			{
				if (*lpCZ == ' ')
				{
					lpCZ++;

					if (*lpCZ == '\0')
					{
						bFalg=FALSE;
					}

					break;
				}

				if(*lpCZ =='\0')
				{
					bFalg=FALSE;
					break;
				}
				CANDLIST.Buff[nCandPos++] =*lpCZ++;
			}

			CANDLIST.Buff[nCandPos++]	=' ';
			CANDLIST.Buff[nCandPos++]	=' ';
			num++;
		} //while
	}//if

	CANDLIST.nLen=nCandPos;
}

/************************************************************************/
/* 
/*
/*                                                                      */
/************************************************************************/
void MakeCandBHStr(FILE *fp,LONG offest)
{
	if (!fp)
	{
		AssPrint("MakeCandBHStr: 打开文件失败!");
		return;
	}
	
	int CompLen=COMPLIST.nLen;
	
	int i=1;					//用于生成中文前的选择数字
	char hz[3];					//汉字
	char bihua[60];				//笔画编码
	
	fseek(fp,offest,SEEK_SET);	//移动到相应的偏移位置
	
	fpos_t pos=NULL;			//用于记录文件位置
	
	while (!feof(fp))
	{
		fgetpos(fp,&pos);			//保存当前位置
		fscanf(fp,"%s%s",hz,bihua);
		
		if (_mbsnbcmp((UCHAR *)COMPLIST.Buffer,(UCHAR *)bihua,CompLen) == 0)
		{
			MakeBHStr(fp,&pos);
			break;
		}
	}
	
}

/************************************************************************/
/* 
/*
/*                                                                      */
/************************************************************************/
void MakeBHStr(FILE *fp,fpos_t *fpos)
{
	int PageNum=CANDLIST.nPageSize;			//一页多少个候选字
	char hz[3];								//汉字
	char bihua[60];							//文件中的笔画编码
	
	int i=1;								//选择字前的数字
	int pos=0;

	fsetpos(fp,fpos);						//移到相应处

	while(PageNum -- )
	{
		fscanf(fp,"%s%s",hz,bihua);
		if (feof(fp))
		{
			return;
		}
		//放到缓冲区，方便翻页
		SAVE.szSave[pos++]=hz[0];
		SAVE.szSave[pos++]=hz[1];
		SAVE.szSave[pos++]=' ';				//汉字之间的分隔
		
	}


	SAVE.szSave[pos++]='|';
	SAVE.szSave[pos]='\0';

	SAVE.nPos=pos;


	//放到候选列表
	pos=0;
	int j=0;
	while (SAVE.szSave[j]!='|')
	{
		CANDLIST.Buff[pos++]=(char)(i+48);
		CANDLIST.Buff[pos++]='.';
		CANDLIST.Buff[pos++]=SAVE.szSave[j++];
		CANDLIST.Buff[pos++]=SAVE.szSave[j++];
		CANDLIST.Buff[pos++]=' ';
		CANDLIST.Buff[pos++]=' ';
		i++;
		j++;
	}
	CANDLIST.nLen=pos;
}
/************************************************************************/
/* 
/* 
/*                                                                      */
/************************************************************************/
void MakeWBCandStr()
{
	int pos=0;
	int i=0;
	int num=1;

	while (SAVE.szSave[i]!='|')
	{
		CANDLIST.Buff[pos++]=(char)(num+48);
		CANDLIST.Buff[pos++]='.';
		while ( (CANDLIST.Buff[pos++] = SAVE.szSave[i++] ) !=' ');

		CANDLIST.Buff[pos++]=' ';
		

		num++;
	}
	CANDLIST.Buff[pos]='\0';
	CANDLIST.nLen=pos;
}
/************************************************************************/
/* 
/* ClearCand()	: 清空候选列表
/*                                                                      */
/************************************************************************/
void ClearCand()
{
	CANDLIST.nLen=0;
}