#pragma once

class CUtils
{
public:
	CUtils(void);
	~CUtils(void);

	CString GetExePath();
	CString GetIPAddress();
	CString GetHostName();
};
