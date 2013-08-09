#include "StdAfx.h"
#include "HandleDic.h"
#include <stdio.h>
#include "Global.h"
#include "Assistance.h"

//////////////////////////////////////////////////////////////////////////
BOOL MapBHFile();

/************************************************************************/
/* 
/*	InitPyInfo()	:读取拼音文件信息
/*                                                                      */
/************************************************************************/

BOOL InitPYInfo()
{
	
	PYINFO.fp=fopen("DIC\\DicInfo.ini","r");	//打开信息文件
	if (!PYINFO.fp)
	{
		AssPrint("打开拼音信息出错!");
		return FALSE;
	}

	int i=0;
	
	while (!feof(PYINFO.fp))					//载入信息
	{
		fscanf(PYINFO.fp,"%s %ld",PYINFO.MapFile[i].PinYin,&PYINFO.MapFile[i].Offest);
		i++;
	}
	fclose(PYINFO.fp);
	
	
	PYINFO.fp=fopen("DIC\\PYDic.txt","r");		//找开拼音字库
	if (!PYINFO.fp)
	{
		AssPrint("打开拼音字库出错!");
		return FALSE;
	}

	PYINFO.lpStr=NULL;

	return TRUE;
};

/************************************************************************/
/* 
/* SearchPY()	: 查找相应的拼音，找不到返回 FALSE
/*                                                                     */
/************************************************************************/

BOOL SearchPY()
{
	int i;									//计数器
	LPSTR lpCompStr=COMPLIST.Buffer;		//指向输入的编码


	for (i=0;i<417;i++)						//在映射区找到相应的偏移
	{
		if (_mbsnbcmp((UCHAR*)lpCompStr,(UCHAR *) PYINFO.MapFile[i].PinYin,strlen(lpCompStr)) == 0)
		{
			fseek(PYINFO.fp,PYINFO.MapFile[i].Offest,SEEK_SET);
			MakePYStr();					//生成拼音的候选列表
			return TRUE;
		}
	}

	//找不到
	return FALSE;
}

/************************************************************************/
/* 
/* MakePYStr()	:生成拼音的候选列表，保存在ResultStr
/*                                                                      */
/************************************************************************/
void MakePYStr()
{
	fgets(PYINFO.strResult,2000,PYINFO.fp);

	LPSTR lpRead=PYINFO.strResult;

	if (lpRead != NULL)	//清空上次记录
	{
		*lpRead='\0';
	}

	while (*lpRead++ !='\t');

	PYINFO.lpStr=lpRead;				//指向中文
}


/************************************************************************/
/* 
/* 读取五笔信息文件，并打开五笔字库
/*                                                                      */
/************************************************************************/
BOOL InitWBInfo()
{
	WBINFO.fp=fopen("DIC\\WBINI.ini","r");
	if(!WBINFO.fp)
	{
		AssPrint("读取五笔信息文件出错!");
		return FALSE;
	}

	//读取信息到数组

	int i=0;
	while (!feof(WBINFO.fp))
	{
		fscanf(WBINFO.fp,"%s %ld",WBINFO.MapWB[i].FirstChar,&WBINFO.MapWB[i].Offest);
		i++;
	}
	fclose(WBINFO.fp);

	WBINFO.fp=fopen("DIC\\WB.txt","r");
	if (!WBINFO.fp)
	{
		AssPrint("找开五笔字库文件出错!");
		return FALSE;
	}

	return TRUE;
}
/************************************************************************/
/* 
/*  检索五笔
/*                                                                     */
/************************************************************************/
BOOL SearchWB()
{
	WBINFO.strResult[0]='\0';

	CHAR FirstChar=COMPLIST.Buffer[0];
	LONG Offest=WBINFO.MapWB[FirstChar-97].Offest;
	
	//移到相应文件处
	fseek(WBINFO.fp,Offest,SEEK_SET);
	
	CHAR strFile[10];
	CHAR strHZ[30];
	while (!feof(WBINFO.fp))
	{
		fscanf(WBINFO.fp,"%s%s",strFile,strHZ);
		if(strFile[0] != FirstChar )
		{
			return FALSE;
		}
		if ( strcmp(strFile,COMPLIST.Buffer) == 0 )
		{
			strcpy(WBINFO.strResult,strHZ);
			return TRUE;
		}
	}

	return FALSE;
}

/************************************************************************/
/* 
/* 检索单模式下的五笔
/*                                                                      */
/************************************************************************/
BOOL SearchSingleWB()
{
	WBINFO.strResult[0]='\0';			//清空五笔结果的缓冲区
	CHAR FirstChar=COMPLIST.Buffer[0];	//指空第一个字符
	LONG Offest=WBINFO.MapWB[FirstChar-97].Offest; //找到对应的偏移量

	int nLenComp=0;
	int	nLenFile=0;
	int nLenHZ=0;

	LPSTR pFileComp;					//指向文件中的编码

	SAVE.nPos=0;

	//移到相应文件处
	fseek(WBINFO.fp,Offest,SEEK_SET);
	
	CHAR strFile[10];					//文件中的五笔编码
	CHAR strHZ[30];						//文件中的五笔字

	while (!feof(WBINFO.fp))
	{
		fscanf(WBINFO.fp,"%s%s",strFile,strHZ);
		if(strFile[0] != FirstChar )
		{
			return FALSE;
		}
		if ( _mbsnbcmp((unsigned char *)strFile,(unsigned char *)COMPLIST.Buffer,COMPLIST.nLen) == 0 )
		{
			nLenComp=strlen(COMPLIST.Buffer);		//文件中编码的长度
			nLenFile=strlen(strFile);		//输入编码的长度
			nLenHZ=strlen(strHZ);

			SAVE.nPos+=nLenHZ;

			strcpy(SAVE.szSave,strHZ);		//复制汉字到缓冲区

			if (nLenComp <nLenFile)			//把还没输入的编码添加到汉字后面
			{
				pFileComp=strFile;
				pFileComp+=nLenComp;		//指向未输入的编码

				strcpy(&SAVE.szSave[SAVE.nPos],pFileComp);

				SAVE.nPos+=nLenFile-nLenComp;
	
			}

			SAVE.szSave[SAVE.nPos++]=' ';

			int nPageNumber=CANDLIST.nPageSize-1;

			while (nPageNumber -- )
			{
				fscanf(WBINFO.fp,"%s%s",strFile,strHZ);

				nLenFile	= strlen(strFile);
				nLenHZ		= strlen(strHZ);
				
				strcpy(&SAVE.szSave[SAVE.nPos],strHZ);

				SAVE.nPos+=nLenHZ;

				if (nLenComp < nLenFile)
				{
					pFileComp=strFile;
					pFileComp+=nLenComp;
					strcpy(&SAVE.szSave[SAVE.nPos],pFileComp);
					SAVE.nPos+=nLenFile-nLenComp;
				}
				SAVE.szSave[SAVE.nPos++]=' ';

				if (feof(WBINFO.fp))
				{
					break;
				}	
			}

			SAVE.szSave[SAVE.nPos++]='|';
			SAVE.szSave[SAVE.nPos]='\0';
			SAVE.nLen=SAVE.nPos;

			return TRUE;
		}
	}
	
	return FALSE;
}

/************************************************************************/
/* 
/* 关闭五笔字库文件
/*                                                                     */
/************************************************************************/
void CloseWBFile()
{
	fclose(WBINFO.fp);
}

/************************************************************************/
/* 
/* 初始化词组信息文件
/*                                                                     */
/************************************************************************/
BOOL InitCZInfo()
{
	CIZUINFO.fp=fopen("DIC\\PYCZ.ini","r");
	if (!CIZUINFO.fp)
	{
		AssPrint("读取拼音词组信息文件出错!");
		return FALSE;
	}

	int i=0;
	while (!feof(CIZUINFO.fp))
	{
		fscanf(CIZUINFO.fp,"%s%ld",CIZUINFO.MapCZ[i].FirstChar,&CIZUINFO.MapCZ[i].Offest);
		i++;
	}

	fclose(CIZUINFO.fp);

	CIZUINFO.fp=fopen("DIC\\PYCZ.txt","r");

	if (!CIZUINFO.fp)
	{
		AssPrint("读取拼音词库文件出错!");
		return FALSE;
	}
	return TRUE;
}

/************************************************************************/
/* 
/* 检索拼音词组 
/*                                                                      */
/************************************************************************/
BOOL SearchPYCZ()
{
	char	ReadLine[80];					//从文件读入的行
	char	PinYin[20];						//读入的拼音
	char	hz[50];							//读入的汉字
	
	char	FirstChar;

	int		nPos=0;							//词组信息结构中的结果串的当前位置

	int		nBuffLen=300;					//最大容量

	BOOL	bRet=FALSE;

	FirstChar = COMPLIST.Buffer[0];
	
	if (FirstChar<= 'h')
	{
		FirstChar-=97;
	}
	else if(FirstChar >= 'j' && FirstChar <= 't')
	{
		FirstChar-=98;
	}
	else if (FirstChar >= 'w' && FirstChar <='z')
	{
		FirstChar-=100;
	}
	
	LONG Offest=CIZUINFO.MapCZ[FirstChar].Offest;
	
	fseek(CIZUINFO.fp,Offest,SEEK_SET);
	
	BOOL bFalg=TRUE;
	
	static char *pComp;
	static char *pPinYin;
	
	while (!feof(CIZUINFO.fp) && bFalg)
	{
		pComp	=COMPLIST.Buffer;				//指向编码串
		pPinYin	=PinYin;						//指向文件中的拼音
		
		fgets(ReadLine,80,CIZUINFO.fp);		//读取一行
		
		int i=0;							//提出汉字
		while(ReadLine[i] != '\t')
		{
			hz[i]=ReadLine[i];
			i++;
		}
		hz[i]='\0';
		
		strcpy(PinYin,&ReadLine[i+1]);		//提出拼音部份
		
		if (COMPLIST.Buffer[0] != PinYin[0])
		{
			break;
		}

		BOOL bNext=TRUE;
		while (bNext)
		{
			if (*pPinYin == *pComp)
			{
				pPinYin++;
				pComp++;
				if (*pComp == '\0')							//找到符合条件的词组
				{
					int nLen=strlen(hz);
					for (i=0;i<nLen;i++)
					{
						if (nPos+1>nBuffLen)
						{
							bFalg=FALSE;
							bNext=FALSE;
							break;
						}
						CIZUINFO.strResult[nPos++]=hz[i];	//添加一个词组到词组信息库里面
					}
					CIZUINFO.strResult[nPos++]=' ';
					bRet=TRUE;								//找到相关词组
					break;
				}
			}
			else
			{
				while (*pPinYin++ !=' ' && *pPinYin !='\0');
				
				if (*pPinYin == '\0' || *pPinYin!=*pComp )
				{
					break;
				}
				
			}//else
		}//while
	}//while

	CIZUINFO.strResult[nPos]='\0';
	return bRet;
}

/************************************************************************/
/* 
/* 初始化英语信息文件。
/*                                                                      */
/************************************************************************/
BOOL InitEngInfo()
{
	ENGINFO.fp=fopen("DIC\\EngInfo.ini","r");	//打开信息文件
	if (!ENGINFO.fp)
	{
		AssPrint("打开英语信息出错!");
		return FALSE;
	}
	
	int i=0;
	char tmpFirstChar[2];						//记录第一个字母
	while (!feof(ENGINFO.fp))					//载入信息
	{
		fscanf(ENGINFO.fp,"%s %ld",tmpFirstChar,&ENGINFO.Offest[i]);	
		i++;
	}
	fclose(ENGINFO.fp);
	
	ENGINFO.fp=fopen("DIC\\EngDic.txt","r");		//找开英语字库
	if (!ENGINFO.fp)
	{
		AssPrint("打开英语字库出错!");
		return FALSE;
	}
	return TRUE;
}

/************************************************************************/
/* 
/*	检索英文，成功返回真，没有检索到返回假
/*                                                                     */
/************************************************************************/
BOOL SearchEng()
{
	ENGINFO.strRestult[0]='\0';					//清空原有数据
	char	FirstChar=COMPLIST.Buffer[0];		
	LONG	offest=ENGINFO.Offest[FirstChar-97];
	
	char	ReadLine[80];
	char	comp[20];

	fseek(ENGINFO.fp,offest,SEEK_SET);			//移动文件指针

	while (!feof(ENGINFO.fp))
	{
		fgets(ReadLine,80,ENGINFO.fp);
		if (ReadLine[0] != FirstChar)
		{
			break;
		}

		int i=0;
		while (ReadLine[i] != '\t')
		{
			comp[i]=ReadLine[i];
			i++;
		}
		comp[i]='\0';
		
		if (strcmp(comp,COMPLIST.Buffer) == 0 )
		{
			strcpy(ENGINFO.strRestult,&ReadLine[i+1]);
			return TRUE;
		}
	}

	return FALSE;
}

/************************************************************************/
/* 
/* 英语模式下：检索英语单词
/*                                                                      */
/************************************************************************/
void SearchEnglish()
{
	CHAR FirstChar	=COMPLIST.Buffer[0];			//获得输入的首字符
	LONG offest		=ENGINFO.Offest[FirstChar-97];	//获得偏移量
	CHAR Line[100];
	fseek(ENGINFO.fp,offest,SEEK_SET);				//移动文件指针

	CANDLIST.nLen=0;
	
	while (!feof(ENGINFO.fp))
	{
		fgets(Line,100,ENGINFO.fp);

		if (_mbsnbcmp((UCHAR*)COMPLIST.Buffer,(UCHAR*)Line,COMPLIST.nLen) == 0 )
		{
			//////////////////////////////////////////////////////////////////////////
			// 直接生成候选字

			int num=1;
			int pos=0;
			int i=0;

			ENGINFO.strList[0][0]=(char)(num+48);
			ENGINFO.strList[0][1]=' ';

			pos=2;
			while(Line[i]!='\t')
			{
				ENGINFO.strList[0][pos]=Line[i];
				pos++;
				i++;
			}

			ENGINFO.strList[0][pos++]=' ';
			ENGINFO.strList[0][pos++]=' ';
			ENGINFO.strList[0][pos]='\0';

			ENGINFO.nListLen=1;
			num++;


			//生成第二以后的候选字
			while (num <= CANDLIST.nPageSize && !feof(ENGINFO.fp) )
			{
				
				fgets(Line,100,ENGINFO.fp);
				
				ENGINFO.strList[num-1][0]=(char)(num+48);
				ENGINFO.strList[num-1][1]=' ';
				i=0;
				pos=2;
				while (Line[i] !='\t')
				{
					ENGINFO.strList[num-1][pos++]=Line[i++];
				}
				ENGINFO.strList[num-1][pos++]=' ';
				ENGINFO.strList[num-1][pos++]=' ';
				ENGINFO.strList[num-1][pos]='\0';
				
				num++;
			}

			ENGINFO.nListLen=num;
			
			return;
		}

	}
}
/************************************************************************/
/* 
/*  读入笔画字库，装入内存
/*	                                                                    */
/************************************************************************/
BOOL InitBHInfo()
{
	//初始化指针
	BHINFO.LPBHTABLE[1]=BHINFO.BHTABLE1;
	BHINFO.LPBHTABLE[2]=BHINFO.BHTABLE2;
	BHINFO.LPBHTABLE[3]=BHINFO.BHTABLE3;
	BHINFO.LPBHTABLE[4]=BHINFO.BHTABLE4;
	BHINFO.LPBHTABLE[5]=BHINFO.BHTABLE5;
	
	
	LPBHTABLESTRUCT lpBHTable;
	//读取文件信息
	int i;
	
	
	char FileNameBuff[20];			//文件名缓冲区
	for (i=1;i<6;i++)
	{
		sprintf(FileNameBuff,"DIC\\bhinfo%d.ini",i);
		FILE *fp=fopen(FileNameBuff,"r");
		if (!fp)
		{
			AssPrint("读取笔画信息文件%s 出错!",FileNameBuff);
			return FALSE;
		}
		lpBHTable=BHINFO.LPBHTABLE[i];
		while (!feof(fp))
		{
			fscanf(fp,"%d%ld",&lpBHTable->nLen,&lpBHTable->lOffest);
			lpBHTable++;
		}
		
		fclose(fp);
	}
	
	return MapBHFile();
}
/************************************************************************/
/* 
/* 读入壁画信息文件
/*                                                                     */
/************************************************************************/
BOOL MapBHFile()
{

		char DicNameBuff[20];
		
		for (int i=1;i<6;i++)
		{
			sprintf(DicNameBuff,"DIC\\bh%d.txt",i);
			BHINFO.fpBHDic[i]=fopen(DicNameBuff,"r");
			if (!BHINFO.fpBHDic[i])
			{
				return FALSE;
			}
		}
		return TRUE;
}

/************************************************************************/
/* 
/* 关闭已打开的笔画字库文件
/*                                                                      */
/************************************************************************/
void CloseBHFile()
{
	for (int i=1;i<6;i++)
	{
		fclose(BHINFO.fpBHDic[i]);
	}
}

/************************************************************************/
/* 
/*	查找笔画文件头，找到相应开头并且长度相等的偏移，
/*	fp: 对应的字库文件
/*                                                                      */
/************************************************************************/
LONG SearchBH(FILE **fp)
{
	char cFirstChar=COMPLIST.Buffer[0];		//第一个输入的数字
	int i;
	switch(cFirstChar)
	{
	case '1':
		{
			for (i=0;i<36;i++)
			{
				if (BHINFO.BHTABLE1[i].nLen == COMPLIST.nLen)
				{
					*fp=BHINFO.fpBHDic[1];
					return BHINFO.BHTABLE1[i].lOffest;
					
				}
			}
			return -1;
		}
		break;
		
	case '2':
		{
			for (i=0;i<36;i++)
			{
				if (BHINFO.BHTABLE1[i].nLen == COMPLIST.nLen)
				{
					*fp=BHINFO.fpBHDic[2];
					return BHINFO.BHTABLE2[i].lOffest;
				}
			}
			return -1;
		}
		break;
		
	case '3':
		{
			
			for (i=0;i<36;i++)
			{
				if (BHINFO.BHTABLE1[i].nLen == COMPLIST.nLen)
				{
					*fp=BHINFO.fpBHDic[3];
					return BHINFO.BHTABLE3[i].lOffest;
				}
			}
			return -1;
		}
		break;
		
	case '4':
		{
			for (i=0;i<36;i++)
			{
				if (BHINFO.BHTABLE1[i].nLen == COMPLIST.nLen)
				{
					*fp=BHINFO.fpBHDic[4];
					return BHINFO.BHTABLE4[i].lOffest;
				}
			}
			return -1;
			
		}
		break;
		
	case '5':
		{
			for (i=0;i<36;i++)
			{
				if (BHINFO.BHTABLE1[i].nLen == COMPLIST.nLen)
				{
					*fp=BHINFO.fpBHDic[5];
					return BHINFO.BHTABLE5[i].lOffest;
				}
			}
			return -1;
		}
		break;
		
	default:
		break;
	}
	return -1;						//找不到相同返回 -1 
}
