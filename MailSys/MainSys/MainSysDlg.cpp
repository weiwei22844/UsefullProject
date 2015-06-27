
// MainSysDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MainSys.h"
#include "MainSysDlg.h"

#include "Mail/CMail.h"
#include "Utils.h"
#include "H264ToMP4Dlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ATOM nShowDlg;
ATOM nHideDlg;
ATOM nShowDlg2;

char g_dbgInfo[500];

void CapScreen(char filename[]);
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CMainSysDlg 对话框


CMainSysDlg::CMainSysDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMainSysDlg::IDD, pParent)
	, m_strMailServer(_T("pop.163.com"))
	, m_strUserid(_T("15210882403"))
	, m_strPassword(_T("yanda19841216"))
	, m_strBody(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_pDesktopSender = NULL;
    m_pDesktopRecorder = NULL;

    m_bListening = FALSE;
    m_sListen = INVALID_SOCKET;
    m_bProcessCur = FALSE;
}

void CMainSysDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MAILS, m_ctlMessagesList);
	DDX_Text(pDX, IDC_EDIT_POPSERV, m_strMailServer);
	DDX_Text(pDX, IDC_EDIT_ACCOUNT, m_strUserid);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strPassword);
	DDX_Text(pDX, IDC_EDIT_BODY, m_strBody);
}

BEGIN_MESSAGE_MAP(CMainSysDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BTN_SEND, &CMainSysDlg::OnBnClickedBtnSend)
    ON_BN_CLICKED(IDC_BTN_TEST, &CMainSysDlg::OnBnClickedBtnTest)
    ON_WM_NCPAINT()
	ON_BN_CLICKED(IDC_BTN_RECEIVE, &CMainSysDlg::OnBnClickedBtnReceive)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MAILS, &CMainSysDlg::OnLvnItemchangedListMails)
    ON_BN_CLICKED(IDC_BTN_TEST2, &CMainSysDlg::OnBnClickedBtnTest2)
    ON_BN_CLICKED(IDC_BTN_STARTCAPDESKTOP, &CMainSysDlg::OnBnClickedBtnStartcapdesktop)
    ON_BN_CLICKED(IDC_BTN_STOPCAPDESKTOP, &CMainSysDlg::OnBnClickedBtnStopcapdesktop)
    ON_BN_CLICKED(IDC_BTN_STARTRECDESKTOP, &CMainSysDlg::OnBnClickedBtnStartrecdesktop)
    ON_BN_CLICKED(IDC_BTN_STOPRECDESKTOP, &CMainSysDlg::OnBnClickedBtnStoprecdesktop)
    ON_BN_CLICKED(IDC_BTN_TRANSFORMH264, &CMainSysDlg::OnBnClickedBtnTransformh264)
END_MESSAGE_MAP()

DWORD WINAPI ServListenFunc(LPVOID pServParam)
{
    CMainSysDlg *pMainDlg = (CMainSysDlg*)pServParam;
    sockaddr_in remoteAddr = {0};
    int nAddrLen = sizeof(remoteAddr);
    char buf[1024] = {0};

    SOCKET sClient = INADDR_ANY;
    int ret;
    while (pMainDlg->m_bListening) {
        sClient = accept(pMainDlg->m_sListen, (SOCKADDR*)&remoteAddr, &nAddrLen);
        if (sClient == INVALID_SOCKET) {
            OutputDebugStringA("accept failed!");
            Sleep(100);
            continue;
        }
        OutputDebugStringA("新的上位机连接\r\n\r\n");
        pMainDlg->m_bProcessCur = TRUE;

        int nCapNum = 0;
        while(pMainDlg->m_bProcessCur){
            ret = recv(sClient, buf, sizeof(buf), 0);
            if(ret == 0 || ret == SOCKET_ERROR ){
                int nErr = WSAGetLastError();
                if(nErr == WSAECONNRESET || nErr == WSAENOTSOCK || nErr == WSAECONNABORTED || nErr == ERROR_SUCCESS) {
                    sprintf(g_dbgInfo, "上位机断开连接\r\n\r\n");
                    OutputDebugStringA(g_dbgInfo);
                    pMainDlg->m_bProcessCur = FALSE;
                    //pMainDlg->m_bWorkRun = FALSE;
                    break;
                }
                TRACE("Recv data error: %d\n", nErr);
                Sleep(10);
                continue;
            }

            if(strstr(buf, "CapScreen")){
                sprintf(g_dbgInfo, "netCommad%d.bmp", nCapNum++);
                CapScreen(g_dbgInfo);
            }

            OutputDebugStringA(buf);
        }
    }

    return 1;
}

int CMainSysDlg::startServer()
{
    sockaddr_in sin  = {0};
    sockaddr_in remoteAddr = {0};
    char szText[] = "TCP Server Demo";
    int nAddrLen = 0;
    int nRet;

    nAddrLen = sizeof(sockaddr_in);
    //fill sin
#if 0
    sin.sin_port = htons(65521);			// 公司
#else
	sin.sin_port = htons(555);			// 龙锦东四
#endif
    sin.sin_family = AF_INET;
    sin.sin_addr.S_un.S_addr = INADDR_ANY;
    //sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    m_sListen = socket(AF_INET, SOCK_STREAM, 0);
    if(m_sListen == INVALID_SOCKET){
        MessageBox(_T("创建socket失败"));
        return 0;
    }

    char chReuse = 1;
    nRet = setsockopt(m_sListen, SOL_SOCKET, SO_REUSEADDR, &chReuse, sizeof(chReuse));
    if(nRet == SOCKET_ERROR)
    {
        char temp[100];
        sprintf(temp, "setsockopt error %d", WSAGetLastError());
        OutputDebugStringA(temp);
    }

    int optval = 1;
    //setsockopt(sListen, SOL_SOCKET, SO_REUSEADDR, (char *)(&optval), sizeof(optval));
    if (bind(m_sListen, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
    {
        MessageBox(_T("绑定监听socket失败"));
        return 0;
    }

    if (listen(m_sListen, 5) == SOCKET_ERROR)
    {
        MessageBox(_T("listen failed!"));
        return 0;
    }
    m_bListening = TRUE;

    HANDLE handl = CreateThread(NULL, 0, ServListenFunc, this, 0, NULL);
    if(handl) {
        CloseHandle(handl);
        return 1;
    }else{
        m_bListening = FALSE;
        return 0;
    }
}

// CMainSysDlg 消息处理程序

BOOL CMainSysDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
    nShowDlg = GlobalAddAtom(_T("ShowDlg"));     
    nHideDlg = GlobalAddAtom(_T("HideDlg")); 
    BOOL nRes = RegisterHotKey(NULL,nShowDlg,MOD_CONTROL | MOD_ALT,VK_NUMPAD0);// ctrl+alt+0(小键盘的0)
    nRes = RegisterHotKey(NULL,nHideDlg,MOD_CONTROL | MOD_ALT,VK_NUMPAD1); //ctrl+alt+1(小键盘的1)
    nRes = RegisterHotKey(NULL,nShowDlg2,MOD_CONTROL | MOD_ALT, VK_F1);

	CUtils utils;
	CString title = utils.GetIPAddress();
	SetWindowText(_T("MailSys-"+title));

    m_ctlMessagesList.SetExtendedStyle(m_ctlMessagesList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_ctlMessagesList.InsertColumn(0, _T("发件人"), LVCFMT_LEFT, 120);
	m_ctlMessagesList.InsertColumn(1, _T("主题"), LVCFMT_LEFT, 350);
	m_ctlMessagesList.InsertColumn(2, _T("日期"), LVCFMT_LEFT, 100);
    m_nMailCount = 0;

    startServer();
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMainSysDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMainSysDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMainSysDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMainSysDlg::OnBnClickedBtnSend()
{
    // TODO: 在此添加控件通知处理程序代码
    string str = "this is from MainSys";
    CMail mail;
    mail.SendMail("smtp.163.com","15210882403@163.com","yanda19841216","284454806@qq.com","测试邮件", str);
}

BYTE *image_buffer; //指向位图buffer的全局指针，window下像素格式: BGRA(4个字节)

void CapScreen(char filename[])
{
    CDC *pDC;
    pDC = CDC::FromHandle(GetDC(GetDesktopWindow()));
    if(pDC == NULL) return;
    int BitPerPixel = pDC->GetDeviceCaps(BITSPIXEL);
    int Width = pDC->GetDeviceCaps(HORZRES);
    int Height = pDC->GetDeviceCaps(VERTRES);

    CDC memDC;
    if(memDC.CreateCompatibleDC(pDC) == 0) return;

    CBitmap memBitmap, *oldmemBitmap;
    if(memBitmap.CreateCompatibleBitmap(pDC, Width, Height) == NULL) return;

    oldmemBitmap = memDC.SelectObject(&memBitmap);
    if(oldmemBitmap == NULL) return;
    if(memDC.BitBlt(0, 0, Width, Height, pDC, 0, 0, SRCCOPY) == 0) return;

    BITMAP bmp;
    memBitmap.GetBitmap(&bmp);

    FILE *fp = fopen(filename, "w+b");

    BITMAPINFOHEADER bih = {0};
    bih.biBitCount = bmp.bmBitsPixel;
    bih.biCompression = BI_RGB;
    bih.biHeight = bmp.bmHeight;
    bih.biPlanes = 1;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;
    bih.biWidth = bmp.bmWidth;

    BITMAPFILEHEADER bfh = {0};
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = bfh.bfOffBits + bmp.bmWidthBytes * bmp.bmHeight;
    bfh.bfType = (WORD)0x4d42;

    fwrite(&bfh, 1, sizeof(BITMAPFILEHEADER), fp);
    fwrite(&bih, 1, sizeof(BITMAPINFOHEADER), fp);

    image_buffer = new BYTE[bmp.bmWidthBytes * bmp.bmHeight];

    GetDIBits(memDC.m_hDC,
        (HBITMAP) memBitmap.m_hObject,
        0,
        Height,
        image_buffer,
        (LPBITMAPINFO) &bih,
        DIB_RGB_COLORS);
    memDC.SelectObject(oldmemBitmap);
    fwrite(image_buffer, 1, bmp.bmHeight * bmp.bmWidth * 4, fp);
    fclose(fp);
}

void CMainSysDlg::OnNcPaint()
{
    // TODO: 在此处添加消息处理程序代码
    // 不为绘图消息调用 CDialog::OnNcPaint()
    static int i = 2;   
    if(i > 0)   
    {   
        i--;
        ShowWindow(SW_HIDE);
    }   
    else   
        CDialog::OnNcPaint();  
}

void CMainSysDlg::OnHotKey(MSG* pMsg)
{
	WORD keyCode = LOWORD(pMsg->lParam);
	WORD vk = HIWORD(pMsg->lParam);
	if (pMsg->wParam == nShowDlg || pMsg->wParam == nShowDlg2)
	{
		ShowWindow(SW_SHOW);
	}
	else if (pMsg->wParam == nHideDlg)
	{
		//PostQuitMessage(0);
		ShowWindow(SW_HIDE);
	}
}

BOOL CMainSysDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (WM_HOTKEY == pMsg->message)
	{
		OnHotKey(pMsg);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

DWORD WINAPI UpdateAllProc(LPVOID pParam)
{
    CMainSysDlg* pMainDlg = NULL;
    pMainDlg = (CMainSysDlg *)pParam;
    
    CoInitialize(NULL);
    try
    {
        jmail::IPOP3Ptr pPOP3("JMail.POP3");
        jmail::IMessagesPtr pMessages;

        // 超时 30S
        pPOP3->Timeout = 30;

        // 连接邮件服务器
        pPOP3->Connect(pMainDlg->m_strUserid.AllocSysString(), pMainDlg->m_strPassword.AllocSysString(), pMainDlg->m_strMailServer.AllocSysString(), 110);
        pMessages = pPOP3->Messages;

        // 已下载的邮件的实际个数(因为第0个ITEM是未用的，所以-1)
        long lCount = pMessages->Count - 1;
        if(lCount == 0)
            AfxMessageBox(_T("信箱为空"));
        else
        {
            jmail::IMessagePtr pMessage;

            // 遍历每封信
            for(long i = 1; i <= lCount; i++)
            {
                pMessage = pMessages->Item[i];

                pMessage->Charset = _T("utf-8");  //采用utf-8形式，可以兼容多語言問題。
                pMessage->Silent = true;
                //EnableCharsetTranslation屬性必須設置true，否則郵件內容、標題會亂碼 .
                pMessage->EnableCharsetTranslation = true;
                pMessage->ContentTransferEncoding = _T("base64");
                pMessage->Encoding = "base64";
                //必須設置ISOEncodeHeaders為True,否則標題會出現亂碼。(此处与网友提供的设置完全相反)
                pMessage->ISOEncodeHeaders = true;

                _bstr_t bstrFrom = pMessage->From;
                _bstr_t bstrSubject = pMessage->Subject;
                _bstr_t bstrBody = pMessage->Body;
                COleDateTime oleDate = pMessage->Date;

                int nListItem = pMainDlg->m_ctlMessagesList.InsertItem(i, bstrFrom);
                pMainDlg->m_ctlMessagesList.SetItem(nListItem, 1, LVIF_TEXT, bstrSubject, 0, 0, 0, NULL);
                pMainDlg->m_ctlMessagesList.SetItem(nListItem, 2, LVIF_TEXT, oleDate.Format(_T("%Y-%m-%d")), 0, 0, 0, NULL);

                pMainDlg->m_strBodyArray.Add(bstrBody);

                if(lCount > pMainDlg->m_nMailCount){
                    pMainDlg->ProcessMail(bstrBody);
                }

                pMessage.Release();
            }

            pMainDlg->m_nMailCount=lCount;
        }
        // 断开连接
        pPOP3->Disconnect();
    }
    catch(_com_error e)
    {
        CString strErr;
        strErr.Format(_T("错误信息：%s\r\n错误描述：%s"), (LPCTSTR)e.ErrorMessage(), (LPCTSTR)e.Description());
        AfxMessageBox(strErr);		
    }
    CoUninitialize();
    pMainDlg->GetDlgItem(IDC_BTN_RECEIVE)->EnableWindow(TRUE);

    return 0;
}

void CMainSysDlg::OnBnClickedBtnReceive()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();

	m_strMailServer.TrimLeft();
	m_strMailServer.TrimRight();
	m_strUserid.TrimLeft();
	m_strUserid.TrimRight();
	m_strPassword.TrimLeft();
	m_strPassword.TrimRight();

	if(m_strMailServer.GetLength() <= 0)
	{
		AfxMessageBox(_T("邮件服务器不能为空"));
		return;
	}
	if(m_strUserid.GetLength() <= 0)
	{
		AfxMessageBox(_T("帐号不能为空"));
		return;
	}

	m_ctlMessagesList.DeleteAllItems();	// 清空列表
	m_strBodyArray.RemoveAll();

    // 创建线程收取邮件
    HANDLE handle = CreateThread(NULL, 0, UpdateAllProc, this, 0, NULL);
    if(handle)
    {
        GetDlgItem(IDC_BTN_RECEIVE)->EnableWindow(FALSE);
        CloseHandle(handle);
    }
}

void CMainSysDlg::OnLvnItemchangedListMails(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    POSITION pos = m_ctlMessagesList.GetFirstSelectedItemPosition();
    if(pos)
    {
        int nItem = m_ctlMessagesList.GetNextSelectedItem(pos);
        m_strBody = m_strBodyArray.GetAt(nItem);
        UpdateData(FALSE);
    }
    *pResult = 0;
}

void CMainSysDlg::ProcessMail(_bstr_t mailBody)
{
    CString csBody = mailBody;
    csBody.MakeLower();
    if(csBody.Find(_T("start desktop capture"))){
        ;
    }else if(csBody.Find(_T("stop desktop capture"))){
        ;
    }
}

void CMainSysDlg::OnBnClickedBtnTest()
{
    // TODO: 在此添加控件通知处理程序代码
#if 0
    if(!m_pDesktopSender){
        m_pDesktopSender = new DesktopSender;
    }
    m_pDesktopSender->Start();
#else
    if(!m_pDesktopRecorder){
        m_pDesktopRecorder = new DesktopRecorder;
    }
    //m_pDesktopRecorder->init(8,16,1);
    m_pDesktopRecorder->init(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1);
    m_pDesktopRecorder->StartRec();
#endif

    //CapScreen("test.bmp");
} 

void CMainSysDlg::OnBnClickedBtnTest2()
{
    // TODO: 在此添加控件通知处理程序代码
#if 0
    if(m_pDesktopSender){
        m_pDesktopSender->Stop();
    }
#else
    if(m_pDesktopRecorder)
        m_pDesktopRecorder->StopRec();
#endif
}

void CMainSysDlg::OnBnClickedBtnStartcapdesktop()
{
    // TODO: 在此添加控件通知处理程序代码
    if(!m_pDesktopSender){
        m_pDesktopSender = new DesktopSender;
    }
    m_pDesktopSender->Start();
}

void CMainSysDlg::OnBnClickedBtnStopcapdesktop()
{
    // TODO: 在此添加控件通知处理程序代码
    if(m_pDesktopSender){
        m_pDesktopSender->Stop();
    }
}

void CMainSysDlg::OnBnClickedBtnStartrecdesktop()
{
    // TODO: 在此添加控件通知处理程序代码
    if(!m_pDesktopRecorder){
        m_pDesktopRecorder = new DesktopRecorder;
    }
    //m_pDesktopRecorder->init(8,16,1);
    m_pDesktopRecorder->init(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 1);
    m_pDesktopRecorder->StartRec();
}

void CMainSysDlg::OnBnClickedBtnStoprecdesktop()
{
    // TODO: 在此添加控件通知处理程序代码
    if(m_pDesktopRecorder)
        m_pDesktopRecorder->StopRec();
}

void CMainSysDlg::OnBnClickedBtnTransformh264()
{
    // TODO: 在此添加控件通知处理程序代码
    CH264ToMP4Dlg h264ToMp4Dlg;
    h264ToMp4Dlg.DoModal();
}
