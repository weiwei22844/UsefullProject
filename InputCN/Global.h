#ifndef GLOBAL_H
#define GLOBAL_H
#include <stdio.h>
#include <commctrl.h>
#include <Commdlg.h>
#include <vector>
#include<string>
//////////////////////////////////////////////////////////////////////////
//全局结构体
//////////////////////////////////////////////////////////////////////////

//输入窗口
typedef	struct _tagInputStruct
{
	SIZE	sz;
	POINT	pos;
	BOOL	bShow;
	
}INPUTSTRUCT;
//////////////////////////////////////////////////////////////////////////
//候选列表
typedef struct _tagCANDLIST 
{
	char	Buff[200];					//候选字列表串
	int		nPageSize;					//一页存放多少个
	int		nLen;						//串长度
	DWORD	dwPageUp;					//向上翻页
	DWORD	dwPageDown;					//向下翻页

}CANDLISTSTRUCT;

//编码列表
typedef struct _tagCOMPLIST 
{
	char	Buffer[200];				//输入的编码
	int		nCurPos;					//当前当标在编码串的位置
	int		nLen;						//编码的长度
}COMPLISTSTRUCT;

//////////////////////////////////////////////////////////////////////////
//
//拼音映射文件结构
typedef struct _tagMapPY 
{
	char PinYin[8];
	long Offest;

}MapPY;

typedef struct _tagPYINFO 
{
	MapPY	MapFile[417];
	FILE	*fp;
	CHAR	strResult[2000];
	LPSTR	lpStr;

}PYINFOSTRUCT,*LPPYINFOSTRUCT;
//======================================
//五笔文件信息头
typedef struct _tagWBINFO 
{
	char FirstChar[2];
	long Offest;
	
}WBINFOSTRUCT,LPWBINFOSTRUCT;

//五笔结构
typedef struct _tagWB 
{
	WBINFOSTRUCT	MapWB[25];
	FILE			*fp;
	CHAR			strResult[200];

}WBINFOST;

//=====================================
//词组
typedef struct _tagPYCZ 
{
	char	FirstChar[2];
	long	Offest;
}CZSTRUCT;

typedef struct _tagCIZU 
{
	CZSTRUCT	MapCZ[23];
	FILE		*fp;
	CHAR		strResult[300];
}CIZUINFOSTRUCT;


//========================================
//英语
typedef struct _tagEng 
{
	LONG		Offest[26];
	FILE		*fp;
	CHAR		strRestult[100];

	char		strList[10][25];		//英文输入时的候选列表
	int			nListLen;				//列表长度
}ENGSTRUCTINFO;


//壁画文件头信息
typedef struct _tagBHTABLE
{
	int		nLen;
	long	lOffest;
}BHTABLESTRUCT,* LPBHTABLESTRUCT;

//笔画信息
typedef struct _tagBHINFO 
{
	BHTABLESTRUCT	BHTABLE1[36];		//用于存放bh.ini文件信息的内存区
	BHTABLESTRUCT	BHTABLE2[36];
	BHTABLESTRUCT	BHTABLE3[36];
	BHTABLESTRUCT	BHTABLE4[36];
	BHTABLESTRUCT	BHTABLE5[36];
	LPBHTABLESTRUCT	LPBHTABLE[6];		//指向前面5个结构的指针
	
	FILE			*fpBHDic[6];		//指向笔画字库的文件指针
}BHINFOSTRUCT,*LPBHINFOSTRUCT;

//////////////////////////////////////////////////////////////////////////
typedef struct  _tagSave 
{
	CHAR	szSave[1000];				//保存读出的文字
	int		nLen;						//长度
	int		nPos;						//当前位置
	fpos_t	fPos;						//文件位置
}SAVESTRUCT;
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
// 全局变量

extern	HINSTANCE		hInst;			//当前线程实例
extern	HWND			hStatus;		//状态窗口句柄
extern	HWND			hInput;			//输入窗口句柄
extern	HWND			hComp;			//编码窗口
extern	HWND			hCand;			//候选列表窗口
extern	HWND			hForeWnd;		//当前拥有光标的窗口
//////////////////////////////////////////////////////////////////////////

extern	DWORD			KeyState;		//当前消息状态
extern	DWORD			CurMode;		//当前输入模式
extern	DWORD			ResultMode;		//结果模式
extern	DWORD			dwType;

extern	PYINFOSTRUCT	PYINFO;			//拼音结构
extern	WBINFOST		WBINFO;			//五笔结构
extern	CIZUINFOSTRUCT	CIZUINFO;		//词组结构
extern	ENGSTRUCTINFO	ENGINFO;		//英语结构
extern	BHINFOSTRUCT	BHINFO;	


extern	INPUTSTRUCT		INPUTWNDSTRUCT;	//输入窗口属性
extern	CANDLISTSTRUCT	CANDLIST;		//候选窗口列表
extern	COMPLISTSTRUCT	COMPLIST;		//编码结构

extern	SAVESTRUCT		SAVE;	

extern	CHOOSEFONT		cf; 
extern	LOGFONT			lf; 		

		
//////////////////////////////////////////////////////////////////////////
		
#define			HANDLE_CHAR			0x01		//普通字符状态
#define			HANDLE_NUM			0x02		//数字状态
#define			HANDLE_SPACE		0x04		//空格
#define			HANDLE_BACK			0x08		//删除
#define			HANDLE_UP			0x010		//向上
#define			HANDLE_DOWN			0x040		//向下
#define			HANDLE_LEFT			0x080		//向左
#define			HANDLE_RIGHT		0x0100		//向右
#define			HANDLE_SMALL_NUM	0x0400		//小键盘

#define			MODE_PINYIN			0x01		//拼音模式
#define			MODE_BIHUA			0x02		//笔画
#define			MODE_WUBI			0x04		//五笔
#define			MODE_ENGLIS			0x08		//英语
#define			MODE_CIZU			0x010		//拼音词组
#define			MODE_FULL			0x020		//混合模式

#endif

