#include "StdAfx.h"
#include <mbstring.h>

//////////////////////////////////////////////////////////////////////////
// ƴ��
BOOL	InitPYInfo();
BOOL	SearchPY();
void	MakePYStr();

//////////////////////////////////////////////////////////////////////////
//�ʻ�
BOOL InitBHInfo();
LONG SearchBH(FILE **fp);
//////////////////////////////////////////////////////////////////////////
// ƴ���ʿ�
BOOL	InitCZInfo();
BOOL	SearchPYCZ();
//////////////////////////////////////////////////////////////////////////
// ���
BOOL InitWBInfo();
BOOL SearchWB();
void CloseWBFile();
BOOL SearchSingleWB();
//////////////////////////////////////////////////////////////////////////
// Ӣ��
BOOL InitEngInfo();
BOOL SearchEng();
void SearchEnglish();