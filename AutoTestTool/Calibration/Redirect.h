#ifndef REDIRECT_H_INCLUDED__
#define REDIRECT_H_INCLUDED__

class CRedirect
{
public:

	//--------------------------------------------------------------------------
	//	constructor
	//--------------------------------------------------------------------------
	CRedirect(LPCTSTR szCommand, CEdit *pEdit, LPCTSTR szCurrentDirectory = NULL);

	//--------------------------------------------------------------------------
	//	destructor
	//--------------------------------------------------------------------------
	virtual ~CRedirect();

	//--------------------------------------------------------------------------
	//	public member functions
	//--------------------------------------------------------------------------
	virtual void		Run();
	virtual	void		Stop();
	BOOL				IsStop();

	CHAR				*m_pStrBuff;
	int					m_nBuffLen;
	LPCTSTR				m_szCommand;

protected:

	//--------------------------------------------------------------------------
	//	member functions
	//--------------------------------------------------------------------------
	void				AppendText(LPCTSTR Text);
	void				PeekAndPump();
	void				SetSleepInterval(DWORD dwMilliseconds);
	void				ShowLastError(LPCTSTR szText);

	//--------------------------------------------------------------------------
	//	member data
	//--------------------------------------------------------------------------
	bool				m_bStopped;
	DWORD				m_dwSleepMilliseconds;
	CEdit				*m_pEdit;
	LPCTSTR				m_szCurrentDirectory;
};

#endif	// REDIRECT_H_INCLUDED__
