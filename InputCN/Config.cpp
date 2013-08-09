#include "StdAfx.h"
#include "Global.h"
#include "NewIme.h"
#include "config.h"
#include "resource.h"
#include "InputWnd.h"
#include "Assistance.h"

#include <vector>
#include<string>
//////////////////////////////////////////////////////////////////////////
// 变量声明
std::vector<std::string>SkinName;
std::vector<std::string>TmpSkin;

SETHOTKEYSTRUCT tmpSETKEY;
BOOL		bSkin=FALSE;

//////////////////////////////////////////////////////////////////////////
//局部函数声明
HFONT		CreateMyFont();
COLORREF	ChooseMyColor(HWND hWnd); 
void		SaveConfig();
BOOL		InitLoadPic();
/************************************************************************/
/* 
/*	SetUsualProc()	: 常规页消息处理过程
/*                                                                     */
/************************************************************************/
BOOL WINAPI SetUsualProc(HWND hWnd,UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	static HWND hCombList=NULL;
	switch(uMsg)
	{
	case WM_CLOSE:
		{
			EndDialog(hWnd,0);
		}
		break;
		
    case WM_INITDIALOG:
		{
			char chNum[5];
			hCombList=GetDlgItem(hWnd,IDC_CMB_PAGENUM);
			for (int i=2;i<9;i++)
			{
				sprintf(chNum,"%d",i);
				SendMessage(hCombList,CB_ADDSTRING,0,(WPARAM)chNum);
			}
			
		}
		break; 
	case WM_COMMAND:
		{
			switch(HIWORD(wParam))
			case CBN_SELENDOK:
			{
				int nSelect=SendMessage(hCombList,CB_GETCURSEL,0,0);
				if (nSelect != CB_ERR)
				{
					char SelectNum[5];
					SendMessage(hCombList,CB_GETLBTEXT,nSelect,(LPARAM)SelectNum);
					AssPrint(SelectNum);
					CANDLIST.nPageSize=atoi(SelectNum);
				}
			}
		}
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}


/************************************************************************/
/* 
/* SetKeyProc()	: 热键页消息处理过程
/*                                                                     */
/************************************************************************/
BOOL WINAPI SetKeyProc(HWND hWnd,UINT message, WPARAM wParam,LPARAM lParam)
{
	switch(message)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
        break;	
	case WM_INITDIALOG:
		{
			if (SETKEY.dwControl & LEFT_VALUE)
			{
				CheckDlgButton(hWnd,IDC_LCTRL,BST_CHECKED);
			}
			else if (SETKEY.dwControl & RIGHT_VALUE)
			{
				CheckDlgButton(hWnd,IDC_RCTRL,BST_CHECKED);
			}

			if (SETKEY.dwShift & LEFT_VALUE)
			{
				CheckDlgButton(hWnd,IDC_LSHIFT,BST_CHECKED);
			}
			else if (SETKEY.dwShift & RIGHT_VALUE)
			{
				CheckDlgButton(hWnd,IDC_RSHIFT,BST_CHECKED);
			}

			if (SETKEY.dwPage & PAGE1)
			{
				CheckDlgButton(hWnd,IDC_FANYE1,BST_CHECKED);
			}
			if (SETKEY.dwPage & PAGE2)
			{
				CheckDlgButton(hWnd,IDC_FANYE2,BST_CHECKED);
			}
			if (SETKEY.dwPage & PAGE3)
			{
				CheckDlgButton(hWnd,IDC_FANYE3,BST_CHECKED);
			}
			if (SETKEY.dwPage & PAGE4)
			{
				CheckDlgButton(hWnd,IDC_FANYE4,BST_CHECKED);
			}
		}
		break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_LSHIFT:
				{
					tmpSETKEY.dwShift=LEFT_VALUE;
				}
				break;
			case IDC_RSHIFT:
				{
					tmpSETKEY.dwShift=RIGHT_VALUE;
				}
				break;
			case IDC_LCTRL:
				{
					tmpSETKEY.dwControl=LEFT_VALUE;
				}
				break;
			case IDC_RCTRL:
				{
					tmpSETKEY.dwControl=RIGHT_VALUE;
				}
				break;

			case IDC_FANYE1:
				{
					tmpSETKEY.dwPage|=PAGE1;
				}
				break;
			case IDC_FANYE2:
				{
					tmpSETKEY.dwPage|=PAGE2;
				}
				break;
			case IDC_FANYE3:
				{
					tmpSETKEY.dwPage|=PAGE3;
				}
				break;
			case IDC_FANYE4:
				{
					tmpSETKEY.dwPage|=PAGE4;
				}
				break;

			default:
				break;
			}
		}
		break;
	case WM_NOTIFY:
		break;
		
	default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			
	}
	return 0;
	
}
//////////////////////////////////////////////////////////////////////////



/************************************************************************/
/*
/* CreateMyFont()	: 设置字体
/*                                                                      */
/************************************************************************/
HFONT CreateMyFont() 
{
    HFONT hfont; 
	
    cf.lStructSize	= sizeof(CHOOSEFONT); 
    cf.hwndOwner	= (HWND)NULL; 
    cf.hDC			= (HDC)NULL; 
    cf.lpLogFont	= &lf; 
    cf.iPointSize	= 0; 
    cf.Flags		= CF_SCREENFONTS; 
    cf.rgbColors	= RGB(0,0,0); 
    cf.lCustData	= 0L; 
    cf.lpfnHook		= (LPCFHOOKPROC)NULL; 
    cf.lpTemplateName = (LPSTR)NULL; 
    cf.hInstance	= (HINSTANCE) NULL; 
    cf.lpszStyle	= (LPSTR)NULL; 
    cf.nFontType	= SCREEN_FONTTYPE; 
    cf.nSizeMin		= 0; 
    cf.nSizeMax		= 0; 
	
    
	if (ChooseFont(&cf))
	{
		hfont = CreateFontIndirect(cf.lpLogFont); 
		return (hfont);
	}
	else
	{
		return (HFONT)NULL;
	}
}
 
/************************************************************************/
/* 
/*	ChooseMyColor()	: 选择字体
/*                                                                      */
/************************************************************************/
COLORREF ChooseMyColor(HWND hWnd)
{
	CHOOSECOLOR		cc;                 
	COLORREF		acrCustClr[16];  
	DWORD			rgbCurrent;
	
	rgbCurrent=RGB(255,255,255);

	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hWnd;
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult = rgbCurrent;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	
	if (ChooseColor(&cc)==TRUE)
	{
		rgbCurrent = cc.rgbResult; 
	}
	return rgbCurrent;
}

/************************************************************************/
/* 
/* InitSlider()		:初始化微调按钮
/*                                                                     */
/************************************************************************/
void InitSlider(HWND hWnd,UINT uValue)
{
	SendMessage(hWnd,TBM_SETRANGE,(WPARAM)TRUE,(LPARAM)MAKELONG(0,100));	//初始化范围
	SendMessage(hWnd,TBM_SETPAGESIZE,(WPARAM)TRUE,(LPARAM)5);
	SendMessage(hWnd,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)uValue);
}

/************************************************************************/
/* 
/* SetViewProc()	: 显示窗口消息处理过程
/*                                                                      */
/************************************************************************/
BOOL WINAPI SetViewProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{				
	HWND hCombList=GetDlgItem(hWnd,IDC_SKIN);

	switch(uMsg)
	{
		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;

		case WM_INITDIALOG:
			{
				//
				HWND hSlider=GetDlgItem(hWnd,IDC_SLIDER1);
				InitSlider(hSlider,SETVIEW.nTransparent);

				//
				char buff[20];
				sprintf(buff,"%d",SETVIEW.nTransparent);
				SetDlgItemText(hWnd,IDC_EDT_OPT,buff);

				//添加文件夹名到列表筐
				for (int i=0;i<SkinName.size();i++)
				{
					SendMessage(hCombList,CB_ADDSTRING,0,(WPARAM)SkinName[i].c_str());
				}

				//
				if (SETVIEW.bSkin)
				{
					CheckDlgButton(hWnd,IDC_CHK_SKIN,BST_CHECKED);
				}
			}
			break;
		///////////////////////////////
		case WM_COMMAND:
		{
			////////////////////////////////////////////////////////////////////
			switch(LOWORD(wParam))
			{
				case IDC_BTN_SETFONT:
				{
						SETVIEW.hFont=CreateMyFont();
						UpdateInputWnd();
				}
				break;

				case IDC_BTN_CHOSECOLOR:
				{
					SETVIEW.rgbFontColor=ChooseMyColor(hWnd);
					UpdateInputWnd();
				}
				break;


				case IDC_BTN_SETBGCOLOR:
				{
					SETVIEW.regBackColor=ChooseMyColor(hWnd);
					UpdateInputWnd();
				}
				break;

				case IDC_CHK_SKIN:
				if (IsDlgButtonChecked(hWnd,IDC_CHK_SKIN))
				{
					bSkin=TRUE;
					SETVIEW.bSkin=TRUE;
				}
				else
				{
					bSkin=FALSE;
					SETVIEW.bSkin=FALSE;
				}
				UpdateInputWnd();
				break;

			default:
				break;
			}//switch
			//////////////////////////////////////////////////////////////////////////
			switch(HIWORD(wParam))
			{
				case CBN_SELENDOK:
				{
					int nSelect=SendMessage(hCombList,CB_GETCURSEL,0,0);

					if (nSelect != CB_ERR)
					{
						//添加到TempSkin ,按确定后再换
						TmpSkin.clear();

						char buffer[80];
						char FloderName[80];

						SendMessage(hCombList,CB_GETLBTEXT,nSelect,(LPARAM)FloderName);
						
					
						sprintf(buffer,"pic\\%s\\hStatus.bmp",FloderName);
						TmpSkin.push_back(buffer);
						sprintf(buffer,"pic\\%s\\hChar.bmp",FloderName);
						TmpSkin.push_back(buffer);
						sprintf(buffer,"pic\\%s\\hInput.bmp",FloderName);
						TmpSkin.push_back(buffer);
						sprintf(buffer,"pic\\%s\\hEnglish.bmp",FloderName);
						TmpSkin.push_back(buffer);

						//在预览区看图.



						HWND hLook=GetDlgItem(hWnd,IDC_PREVIEW);
						HDC	 dc=GetDC(hLook);
						
						HDC	 mdc=CreateCompatibleDC(dc);
						HDC	 dcPic=CreateCompatibleDC(dc);
						
						strcpy(buffer,TmpSkin[0].c_str());
						HBITMAP btStatus=(HBITMAP) LoadImage(hInst,buffer,IMAGE_BITMAP,160,20,LR_LOADFROMFILE|LR_SHARED);
						SelectObject(dcPic,btStatus);
						
						BitBlt(mdc,0,0,160,20,dcPic,0,0,SRCCOPY);

						strcpy(buffer,TmpSkin[1].c_str());
						HBITMAP btChar=(HBITMAP) LoadImage(hInst,buffer,IMAGE_BITMAP,360,40,LR_LOADFROMFILE|LR_SHARED);
						SelectObject(dcPic,btChar);
						
						BitBlt(mdc,20,0,40,20,dcPic,100,0,SRCCOPY); //混合

						BitBlt(mdc,60,0,20,20,dcPic,220,0,SRCCOPY); //中
						
						BitBlt(mdc,80,0,20,20,dcPic,260,0,SRCCOPY); //半角

						BitBlt(mdc,100,0,20,20,dcPic,300,0,SRCCOPY); //标点

						BitBlt(mdc,120,0,20,20,dcPic,320,0,SRCCOPY); //软
				
						BitBlt(mdc,140,0,20,20,dcPic,340,0,SRCCOPY); //设
					
						BitBlt(dc,0,0,160,20,mdc,0,0,SRCCOPY);

						/*
						btStatus=LoadBitmap(hInst,(LPSTR)IDB_STATUS_BG);
						SelectObject(mdc,btStatus);
						BitBlt(dc,0,0,160,20,mdc,0,0,SRCCOPY);
						*/

						ReleaseDC(hLook,dcPic);
						ReleaseDC(hLook,mdc);
						ReleaseDC(hLook,dc);
					}//if



				}//case
				break;
				
				default:
					break;

			}//switch

		}//case command
		break;
	//控件的通知消息
	case WM_NOTIFY:
		{
			switch( ((LPNMHDR)lParam)->code )
			{
				case NM_RELEASEDCAPTURE:
					{
						static char chNumBuff[20];
						LONG lResult=SendMessage( ((LPNMHDR)lParam)->hwndFrom, TBM_GETPOS ,(WPARAM)0,(LPARAM)0 );
						sprintf(chNumBuff,"%ld",lResult);
						SetDlgItemText(hWnd,IDC_EDT_OPT,chNumBuff);
						ShowWindow(hInput,SW_SHOWNA);
						SETVIEW.nTransparent=(int)lResult;
						TranspanentWindow((int)lResult);
					}
					break;

				default:
					break;
			}
		}//case
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}//switch
	return 0;
}

/************************************************************************/
/* 
/* SetCiKuProc()	:设置词库消息处理过程
/*                                                                     */
/************************************************************************/
BOOL WINAPI SetCiKuProc(HWND hWnd,UINT message, WPARAM wParam,LPARAM lParam)
{
	switch(message)
	{
		
	case WM_CLOSE:
		DestroyWindow(hWnd);
        break;
 
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
	
}

/************************************************************************/
/* 
/* DoPropertySheet()	: 后台设置
/*                                                                     */
/************************************************************************/
int DoPropertySheet(HWND hWnd)
{
	
	PROPSHEETPAGE psp[4];			//设置四个属性页
    PROPSHEETHEADER psh;
	int nRect=-1;
   
	//////////////////////////////////////////////////////////////////////////
	// 第一页
	//////////////////////////////////////////////////////////////////////////
    psp[0].dwSize		= sizeof(PROPSHEETPAGE);
    psp[0].dwFlags		= PSP_USEICONID | PSP_USETITLE;
    psp[0].hInstance	= hInst;
    psp[0].pszTemplate	= MAKEINTRESOURCE(IDD_SETUSUAL);
    psp[0].pszIcon		= NULL;
    psp[0].pfnDlgProc	= (DLGPROC)SetUsualProc;
    psp[0].pszTitle		= "常规";
    psp[0].lParam		= 0;
    psp[0].pfnCallback	= NULL;
	//////////////////////////////////////////////////////////////////////////
	// 第二页
	//////////////////////////////////////////////////////////////////////////
	psp[1].dwSize		= sizeof(PROPSHEETPAGE);
    psp[1].dwFlags		= PSP_USEICONID | PSP_USETITLE;
    psp[1].hInstance	= hInst;
    psp[1].pszTemplate	= MAKEINTRESOURCE(IDD_SETKEY);	//设置热键这里设置键盘的键
    psp[1].pszIcon		= NULL;
    psp[1].pfnDlgProc	= (DLGPROC)SetKeyProc;
    psp[1].pszTitle		= "热键";
    psp[1].lParam		= 0;
    psp[1].pfnCallback	= NULL;
	//////////////////////////////////////////////////////////////////////////
	// 第三页
	//////////////////////////////////////////////////////////////////////////
	psp[2].dwSize		= sizeof(PROPSHEETPAGE);
    psp[2].dwFlags		= PSP_USEICONID | PSP_USETITLE;
    psp[2].hInstance	= hInst;
    psp[2].pszTemplate	= MAKEINTRESOURCE(IDD_SETVIEW);//设置外观,
    psp[2].pszIcon		= NULL;
    psp[2].pfnDlgProc	= (DLGPROC)SetViewProc;
    psp[2].pszTitle		= "外观";
    psp[2].lParam		= 0;
    psp[2].pfnCallback	= NULL;
	
    //////////////////////////////////////////////////////////////////////////
	// 第四页
	//////////////////////////////////////////////////////////////////////////
	psp[3].dwSize		= sizeof(PROPSHEETPAGE);
    psp[3].dwFlags		= PSP_USEICONID | PSP_USETITLE;
    psp[3].hInstance	= hInst;
    psp[3].pszTemplate	= MAKEINTRESOURCE(IDD_SETCIKU);// 设置词库
    psp[3].pszIcon		= NULL;
    psp[3].pfnDlgProc	= (DLGPROC)SetCiKuProc;
    psp[3].pszTitle		= "词库设置";
    psp[3].lParam		= 0;
    psp[3].pfnCallback	= NULL;
	//////////////////////////////////////////////////////////////////////////
 
    psh.dwSize		= sizeof(PROPSHEETHEADER);
    psh.dwFlags		= PSH_USEICONID | PSH_PROPSHEETPAGE|PSH_USEPAGELANG;
    psh.hwndParent	= hWnd;
    psh.hInstance	= hInst;
    psh.pszIcon		= NULL;
    psh.pszCaption	= (LPSTR)"输入法后台设置";
    psh.nPages		= sizeof(psp) /sizeof(PROPSHEETPAGE);
    psh.nStartPage	= 0;
    psh.ppsp		= (LPCPROPSHEETPAGE) &psp;
    psh.pfnCallback = NULL;

    nRect=PropertySheet(&psh);

	AssPrint("nRect:%d",nRect);
	if (nRect == 1)
	{
		
		if (bSkin)
		{
			strcpy(SETVIEW.bmpStatus,TmpSkin[0].c_str());
			strcpy(SETVIEW.bmpChar,TmpSkin[1].c_str());
			strcpy(SETVIEW.bmpInput,TmpSkin[2].c_str());
			strcpy(SETVIEW.bmpEng,TmpSkin[3].c_str());
		}

		SETKEY=tmpSETKEY;

		SaveConfig();

		//更新显示
		RECT ClientRect;
		GetClientRect(hStatus,&ClientRect);
		InvalidateRect(hStatus,&ClientRect,FALSE);
		Input_OnCreate(hInput);
		GetClientRect(hInput,&ClientRect);
		InvalidateRect(hInput,&ClientRect,FALSE);



	}
    return nRect;
}


/************************************************************************/
/*
/* 进行对话框选项进行保存设置
/*                                                                      */
/************************************************************************/
void SaveConfig()
{
	
	CHAR Buffer[50];
	int		nRet;
	//////////////////////////////////////////////////////////////////////////
	//保存外观设置
	sprintf(Buffer,"%ld",SETVIEW.regBackColor);
	nRet=WritePrivateProfileString("VIEW","BackColor",Buffer,".\\Config.ini");
	if (!nRet)
	{
		AssPrint("写入文件失败!");
		return;
	}
	sprintf(Buffer,"%ld",SETVIEW.rgbFontColor);
	nRet=WritePrivateProfileString("VIEW","FontColor",Buffer,".\\Config.ini");
	if (!nRet)
	{
		AssPrint("写入文件失败!");
		return;
	}

	sprintf(Buffer,"%d",SETVIEW.bSkin);
	nRet=WritePrivateProfileString("VIEW","bSkin",Buffer,".\\Config.ini");
	if (!nRet)
	{
		AssPrint("写入文件失败!");
		return;
	}
	
	sprintf(Buffer,"%ld",SETVIEW.hFont);
	nRet=WritePrivateProfileString("VIEW","Font",Buffer,".\\Config.ini");
	if (!nRet)
	{
		AssPrint("写入文件失败!");
		return;
	}
	sprintf(Buffer,"%ld",SETVIEW.nTransparent);
	nRet=WritePrivateProfileString("VIEW","nTransparent",Buffer,".\\Config.ini");
	if (!nRet)
	{
		AssPrint("写入文件失败!");
		return;
	}

	WritePrivateProfileString("SKIN","bmpStatus",SETVIEW.bmpStatus,".\\Config.ini");
	WritePrivateProfileString("SKIN","bmpChar",SETVIEW.bmpChar,".\\Config.ini");
	WritePrivateProfileString("SKIN","bmpInput",SETVIEW.bmpInput,".\\Config.ini");
	WritePrivateProfileString("SKIN","bmpEng",SETVIEW.bmpEng,".\\Config.ini");

	//////////////////////////////////////////////////////////////////////////
	//保存字体

	sprintf(Buffer,"%ld",lf.lfHeight);
	WritePrivateProfileString("LOGFONT","lfHeight",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",lf.lfWidth);
	WritePrivateProfileString("LOGFONT","lfWidth",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",lf.lfEscapement);
	WritePrivateProfileString("LOGFONT","lfEscapement",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",lf.lfOrientation);
	WritePrivateProfileString("LOGFONT","lfOrientation",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",lf.lfWeight);
	WritePrivateProfileString("LOGFONT","lfWeight",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",lf.lfItalic);
	WritePrivateProfileString("LOGFONT","lfItalic",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",lf.lfUnderline);
	WritePrivateProfileString("LOGFONT","lfUnderline",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",lf.lfStrikeOut);
	WritePrivateProfileString("LOGFONT","lfStrikeOut",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",lf.lfCharSet);
	WritePrivateProfileString("LOGFONT","lfCharSet",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",lf.lfOutPrecision);
	WritePrivateProfileString("LOGFONT","lfOutPrecision",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",lf.lfQuality);
	WritePrivateProfileString("LOGFONT","lfQuality",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",lf.lfPitchAndFamily);
	WritePrivateProfileString("LOGFONT","lfPitchAndFamily",Buffer,".\\Config.ini");

	WritePrivateProfileString("LOGFONT","lfFaceName",lf.lfFaceName,".\\Config.ini");


	//////////////////////////////////////////////////////////////////////////
	sprintf(Buffer,"%ld",cf.lStructSize);
	WritePrivateProfileString("FONT","lStructSize",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.hwndOwner);
	WritePrivateProfileString("FONT","hwndOwner",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.hDC);
	WritePrivateProfileString("FONT","hDC",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.lpLogFont);
	WritePrivateProfileString("FONT","lpLogFont",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.iPointSize);
	WritePrivateProfileString("FONT","iPointSize",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.Flags);
	WritePrivateProfileString("FONT","Flags",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.rgbColors);
	WritePrivateProfileString("FONT","rgbColors",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.lCustData);
	WritePrivateProfileString("FONT","lCustData",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.lpfnHook);
	WritePrivateProfileString("FONT","lpfnHook",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.lpTemplateName);
	WritePrivateProfileString("FONT","lpTemplateName",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.hInstance);
	WritePrivateProfileString("FONT","hInstance",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.lpszStyle);
	WritePrivateProfileString("FONT","lpszStyle",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.nFontType);
	WritePrivateProfileString("FONT","nFontType",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.nSizeMin);
	WritePrivateProfileString("FONT","nSizeMin",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",cf.nSizeMax);
	WritePrivateProfileString("FONT","nSizeMax",Buffer,".\\Config.ini");

	//////////////////////////////////////////////////////////////////////////
	// 热键设置页
	sprintf(Buffer,"%ld",SETKEY.dwControl);
	WritePrivateProfileString("HOTKEY","Ctrl",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",SETKEY.dwShift);
	WritePrivateProfileString("HOTKEY","Shift",Buffer,".\\Config.ini");

	sprintf(Buffer,"%ld",SETKEY.dwPage);
	WritePrivateProfileString("HOTKEY","Page",Buffer,".\\Config.ini");

}
/************************************************************************/
/* 
/* 读取配置文件
/*                                                                      */
/************************************************************************/
void LoadConfig()
{
	SETVIEW.regBackColor=GetPrivateProfileInt("VIEW","BackColor",16776960,".\\Config.ini");
	SETVIEW.rgbFontColor=GetPrivateProfileInt("VIEW","FontColor",8404992,".\\Config.ini");
	SETVIEW.nTransparent=GetPrivateProfileInt("VIEW","nTransparent",85,".\\Config.ini");
	SETVIEW.bSkin=GetPrivateProfileInt("VIEW","bSkin",1,".\\Config.ini");
	
	lf.lfCharSet		= GetPrivateProfileInt("LOGFONT","lfCharSet",1,".\\Config.ini");
	lf.lfClipPrecision	= GetPrivateProfileInt("LOGFONT","lfClipPrecision",1,".\\Config.ini");
	lf.lfEscapement		= GetPrivateProfileInt("LOGFONT","lfEscapement",1,".\\Config.ini");

	GetPrivateProfileString("LOGFONT","lfFaceName","宋体",lf.lfFaceName,32,".\\Config.ini");

	lf.lfHeight			= GetPrivateProfileInt("LOGFONT","lfHeight",1,".\\Config.ini");
	lf.lfItalic			= GetPrivateProfileInt("LOGFONT","lfItalic",1,".\\Config.ini");
	lf.lfOrientation	= GetPrivateProfileInt("LOGFONT","lfOrientation",1,".\\Config.ini");
	lf.lfOutPrecision	= GetPrivateProfileInt("LOGFONT","lfOutPrecision",1,".\\Config.ini");
	lf.lfPitchAndFamily	= GetPrivateProfileInt("LOGFONT","lfPitchAndFamily",1,".\\Config.ini");
	lf.lfQuality		= GetPrivateProfileInt("LOGFONT","lfQuality",1,".\\Config.ini");
	lf.lfStrikeOut		= GetPrivateProfileInt("LOGFONT","lfStrikeOut",1,".\\Config.ini");
	lf.lfUnderline		= GetPrivateProfileInt("LOGFONT","lfUnderline",1,".\\Config.ini");
	lf.lfWeight			= GetPrivateProfileInt("LOGFONT","lfWeight",1,".\\Config.ini");
	lf.lfWidth			= GetPrivateProfileInt("LOGFONT","lfWidth",1,".\\Config.ini");

	SETVIEW.hFont=CreateFontIndirect(&lf);


	//皮肤
	GetPrivateProfileString("SKIN","bmpStatus",NULL,SETVIEW.bmpStatus,80,".\\Config.ini");
	GetPrivateProfileString("SKIN","bmpChar",NULL,SETVIEW.bmpChar,80,".\\Config.ini");
	GetPrivateProfileString("SKIN","bmpInput",NULL,SETVIEW.bmpInput,80,".\\Config.ini");
	GetPrivateProfileString("SKIN","bmpEng",NULL,SETVIEW.bmpEng,80,".\\Config.ini");

	InitLoadPic();

	//热键设置
	SETKEY.dwControl	= GetPrivateProfileInt("HOTKEY","Ctrl",1,".\\Config.ini");
	SETKEY.dwShift	= GetPrivateProfileInt("HOTKEY","Shift",1,".\\Config.ini");
	SETKEY.dwPage	= GetPrivateProfileInt("HOTKEY","Page",1,".\\Config.ini");

}

/************************************************************************/
/* 
/* InitConfig()		:根据读入的信息初始化
/*                                                                     */
/************************************************************************/
void InitConfig()
{
	//////////////////////////////////////////////
	//VIEW

	TranspanentWindow(SETVIEW.nTransparent);
	
}

/************************************************************************/
/* 
/*
/*                                                                     */
/************************************************************************/
BOOL InitLoadPic()
{

	WIN32_FIND_DATA FindFileData; 
		
	HANDLE hFind;
		
	hFind = FindFirstFile("PIC\\*", &FindFileData);
		
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	else
	{
		do 
		{
			if(FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY &&
				FindFileData.cFileName[0]!='.')
			{
				SkinName.push_back(std::string(FindFileData.cFileName));
			}
				
		}while(FindNextFile(hFind, &FindFileData));
			
		FindClose(hFind);
	}
	return TRUE;
}