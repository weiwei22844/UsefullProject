#pragma once

#include <WinSock2.h>
#include "ZBase64.h"
#pragma comment(lib,"ws2_32")

#include <string>
using namespace std;

class CMail
{
public:
	CMail() { sSock = INVALID_SOCKET;};

	~CMail()
	{
		FreeSocket();
	}
	
public:
	BOOL SendMail(string SMTP,string UserName,string PassWorld,string DestMailUserName,string Subjuect,string Data);
private:
	//因为命令基本上都是按顺序来，所以阻塞也不会影响效率
	BOOL ConnectSMTPServer(string SMTP);
	BOOL MailCmd(char* MailCmd,char* lparam,string &RetStr);
protected:

	//对套接字进行操作
	BOOL InitSocket();

	BOOL CreateSocket();

	BOOL Listen();

	BOOL Bind(int nPort);

	void FreeSocket();

	BOOL Connect(LPSTR IP,int nPort = 25);

	//TCP 阻塞套接字
	BOOL Send(const char *data,int nLen);

	BOOL Recv(string & buffer);
	
private:
	SOCKET sSock;

public:
	string LastError;
};