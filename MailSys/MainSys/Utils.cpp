#include "StdAfx.h"
#include "Utils.h"

CUtils::CUtils(void)
{
}

CUtils::~CUtils(void)
{
}

CString CUtils::GetExePath()
{
	CString  strPathName;
	GetModuleFileName(NULL,strPathName.GetBuffer(256),256);
	strPathName.ReleaseBuffer(256);
	int nPos = strPathName.ReverseFind('\\');
	strPathName = strPathName.Left(nPos + 1);
	return strPathName;
}

CString CUtils::GetIPAddress()   
{  
    char chTempIp[128]={0}; 
    char chIP[64]={0}; 
	CString strPath;
	char *pData = NULL;
    CString strTemp = _T("");
	strPath = GetExePath() + _T("netip.ini");   
	//URLDownloadToFile(0,_T("http://www.ip138.com/ip2city.asp"),strPath,0,NULL);
	if(URLDownloadToFile(0,_T("http://iframe.ip138.com/ic.asp"),strPath,0,NULL) != S_OK)
        return strTemp;

	/*FILE *fp=fopen( strPath, "r" );   
	if ( fp != NULL )    
	{   
		fseek(fp, 0, SEEK_SET);   
		fread(buf,1,256,fp);   
		fclose(fp);   
		char* iIndex = strstr(buf,"[");   
		if (iIndex)    
		{   
			sprintf( chTempIp,"%s",iIndex);   
			int nBuflen = strlen(chTempIp);   
			for(int i =0; i<nBuflen; i++)   
			{   
				chIP[i] = chTempIp[i+1];   

				if(chTempIp[i] == ']')    
				{   
					chIP[i-1] = '/0';   
					DeleteFile(strPath);   
					continue;   
				}   
			}   
		}   
	}*/
	CFile file;
	try
	{
		ULONGLONG size;
        if(file.Open(strPath,CFile::modeRead)){
		    file.SeekToBegin();
		    size = file.GetLength();
		    pData = (char*)malloc(size);
		    if(pData){
			    file.Read(pData, size);
			    char* iIndex = strstr(pData,"[");
			    if (iIndex)    
			    {
				    sprintf(chTempIp,"%s",iIndex);
				    int nBuflen = sizeof(chTempIp);
				    memcpy(chTempIp, iIndex, nBuflen);
				    for(int i =0; i<nBuflen; i++)   
				    {
					    chIP[i] = chTempIp[i+1];
					    if(chTempIp[i] == ']')    
					    {
						    chIP[i-1] = 0;
						    break;
					    }
				    }
			    }
			    free(pData);
		    }
		    file.Close();
        }
	}
	catch(CFileException *e)
	{
		CString str;
		str.Format(_T("写入失败的原因是:%d"),e->m_cause);
		file.Abort();
		e->Delete();
	}
	/*CFile fileTest;
	try
	{
		fileTest.Open(strPath,CFile::modeCreate|CFile::modeWrite);
		fileTest.SeekToBegin();
		//fileTest.Write((unsigned char *)(m_data.GetBuffer(0)),m_data.GetLength());//CString m_data
		fileTest.Flush();
		fileTest.Close();
	}
	catch(CFileException *e)
	{
		CString str;
		str.Format(_T("写入失败的原因是:%d"),e->m_cause);
		fileTest.Abort();
		e->Delete();
	}*/
	DeleteFile(strPath);
	
	if(chIP != "")
	{
		strTemp.Format(_T("%S"),chIP);
	}
	return strTemp;
}   

/************************************************************************/        
/* 函数说明：获取当前主机名                                           
/* 参    数：无                                       
/* 返 回 值：返回主机名、CString类型字符串        
/* By:Koma   2009.08.21 14:23                                    
/************************************************************************/      
CString CUtils::GetHostName()
{
	char    szHostName[256];
	int     nRetCode;
	CString str=_T("");

	WORD 		wVersionRequested;
	WSADATA 	wsaData;
	int 		err;
	wVersionRequested = MAKEWORD(2,0);   
	err = WSAStartup(wVersionRequested,&wsaData);
	if(err != 0)   
	{   
		return str;
	}
	if(LOBYTE(wsaData.wVersion != 2) || HIBYTE(wsaData.wVersion) != 0)
	{
		WSACleanup();
		//return (char)WSAVERNOTSUPPORTED;
		return str;
	}

	//调用API来获得主机名   
	nRetCode = gethostname(szHostName,sizeof(szHostName));   
	if(nRetCode != 0)   
	{   
		//return WSAGetLastError();
		return str;
	}
	if(szHostName != "") 
	{
		str.Format(_T("主机名:%s"),szHostName);
	}
	return str;
}