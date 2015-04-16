#include "CMail.h"


#define ASSERT_ERROR(VA) {if(!VA) return FALSE;}

BOOL CMail::InitSocket()
{
	WSADATA wsa;
	return (!::WSAStartup(MAKEWORD(2,2),&wsa));
}

//0 :错误 1:正确 2: 已经创建一个合法的SOCKET
BOOL CMail::CreateSocket()
{
	if(sSock != INVALID_SOCKET)
	{
		return 2;
	}

	sSock = socket(AF_INET,SOCK_STREAM,0);

	if(sSock == INVALID_SOCKET)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL CMail::Bind(int nPort)
{
	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = INADDR_ANY;

	int bVal = bind(sSock,(sockaddr*)&addr,sizeof(addr));
	if(SOCKET_ERROR == bVal)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CMail::Listen()
{
	int bVal = listen(sSock,5);
	if(SOCKET_ERROR == bVal)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CMail::Connect(LPSTR IP,int nPort /* = 25 */)
{
	SOCKADDR_IN addr;
	memset(&addr,0,sizeof(addr));

	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);

	int bVal = connect(sSock,(sockaddr*)&addr,sizeof(addr));
	if(bVal == SOCKET_ERROR)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CMail::Send(const char *data,int nLen)
{
	int bLongData = send(sSock,data,nLen,0);

	if(bLongData == SOCKET_ERROR)
	{
		return FALSE;
	}

	return TRUE;

}

void CMail::FreeSocket()
{
	if(sSock != INVALID_SOCKET)
	{
		closesocket(sSock);
	}
}

BOOL CMail::MailCmd(char* MailCmd,char* lparam,string &RetStr)
{
	if(sSock == INVALID_SOCKET)
	{
		return FALSE;
	}

	if(NULL == MailCmd)
	{
		return FALSE;
	}

	string strCMD = MailCmd;

	if(lparam != NULL)
	{
		strCMD += " ";
		strCMD += lparam;
	}

	strCMD += "\r\n";

	int bVal =  Send(strCMD.c_str(),strCMD.length());
	if(bVal == FALSE)
	{
		LastError = "发送消息给服务器失败";
		return FALSE;
	}

	string Respond;
	bVal = Recv(Respond);
	if(bVal == FALSE)
	{
		LastError = "接收服务器应答失败";
		return FALSE;
	}
	
	if(Respond[0] != '2' && Respond[0] != '3')
	{
		LastError = Respond;
		return FALSE;
	}

	RetStr = Respond;

	return TRUE;
}

BOOL CMail::Recv(string & buffer)
{
	char buf[1024] = {0};			//因为应答码 不可能会很长，我这个编程都是阻塞 都是按顺序来

	if(INVALID_SOCKET == sSock)
	{
		return FALSE;
	}

	int bVal = recv(sSock,buf,1024,0);
	if(bVal == 0 || bVal == SOCKET_ERROR)
	{
		return FALSE;
	}

	buffer = buf;
	return TRUE;
}

//测试SendMail
BOOL CMail::SendMail(string SMTP,string UserName,string PassWorld,string DestMailUserName,string Subjuect,string Data)
{
	BOOL bVal = ConnectSMTPServer(SMTP);
	if(bVal == FALSE)
	{
		LastError = "链接STMP 邮箱服务器失败";
		return FALSE;
	}

	string Respond;
	bVal = MailCmd("HELO","xiaoyuer",Respond);
	ASSERT_ERROR(bVal);

	printf("HELO <-> %s\n",(char*)Respond.c_str());

	//进行登录
	bVal = MailCmd("AUTH LOGIN",NULL,Respond);
	ASSERT_ERROR(bVal);

	printf("AUTH LOGIN <-> %s\n",(char*)Respond.c_str());

	//进行密码发送加密 然后发送
	ZBase64 base64;
	string strUserName=base64.Encode((unsigned char *)UserName.c_str(),UserName.length());
	MailCmd((char*)strUserName.c_str(),NULL,Respond);
	ASSERT_ERROR(bVal);

	printf("USER<-> %s\n",(char*)Respond.c_str());

	string strPasswordsWorld=base64.Encode((unsigned char *)PassWorld.c_str(),PassWorld.length());
	MailCmd((char*)strPasswordsWorld.c_str(),NULL,Respond);

	ASSERT_ERROR(bVal);

	printf("PASSWORD<-> %s\n",(char*)Respond.c_str());

	//发送我的邮箱地址
	string cmd = "MAIL FROM:<";
	cmd += UserName;
	cmd += ">";
	MailCmd((char*)cmd.c_str(),NULL,Respond);
	ASSERT_ERROR(bVal);

	printf("MAIL FROM<-> %s\n",(char*)Respond.c_str());

	cmd = "RCPT TO:<";
	cmd += DestMailUserName;
	cmd += ">";
	
	//cmd = "RCPT TO:<879801208@qq.com>";
	MailCmd((char*)cmd.c_str(),NULL,Respond);

	ASSERT_ERROR(bVal);

	printf("RCPT TO<-> %s\n",(char*)Respond.c_str());

	//发送数据
	MailCmd("DATA",NULL,Respond);
	printf("DATA<-> %s\n",(char*)Respond.c_str());

	//From:<username>\r\n
	string Heaerd  =  "From:<";
	Heaerd += UserName;
	Heaerd += ">\r\n";

	//To:<DestUserName>\r\n
	Heaerd += "To:<";
	Heaerd +=DestMailUserName;
	Heaerd +=">\r\n";

	Heaerd += "Subject:";
	Heaerd += Subjuect;
	Heaerd += "\r\n\r\n";

	Heaerd += Data;
	Heaerd += "\r\n.\r\n";

	Send((char*)Heaerd.c_str(),Heaerd.length());

	Recv(Respond);
	printf("数据%s\n",(char*)Respond.c_str());

	bVal = MailCmd("QUIT",NULL,Respond);
	ASSERT_ERROR(bVal);
	printf("QUIT<-> %s\n",(char*)Respond.c_str());
	
	return TRUE;
}


BOOL CMail::ConnectSMTPServer(string SMTP)
{
	BOOL bVal = InitSocket();
	ASSERT_ERROR(bVal);

	bVal = CreateSocket();
	ASSERT_ERROR(bVal);


	HOSTENT* m_HOSTENT;
	if((m_HOSTENT=gethostbyname(SMTP.c_str()))==NULL)
	{
		FreeSocket();
		return FALSE;
	}

	if(m_HOSTENT->h_addr_list[0] == NULL)
	{
		FreeSocket();
		return false;
	}

	in_addr addr;
	memcpy(&addr,(ULONG *)m_HOSTENT->h_addr_list[0],sizeof(ULONG));
	char * ip =inet_ntoa(addr);
	bVal = Connect(ip);
	if(FALSE == bVal)
	{
		return FALSE;
	}
	
	return TRUE;
}