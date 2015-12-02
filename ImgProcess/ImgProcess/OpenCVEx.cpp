#include "StdAfx.h"

#define CV_CDECL __cdecl

/*typedef void (CV_CDECL *CvMouseCallback )(int event, int x, int y, int flags, void* param);
struct CvWindowEx;

typedef struct CvTrackbarEx
{
    int signature;
    HWND hwnd;
    char* name;
    CvTrackbarEx* next;
    CvWindowEx* parent;
    HWND buddy;
    int* data;
    int pos;
    int maxval;
    void (*notify)(int);
    void (*notify2)(int, void*);
    void* userdata;
    int id;
}CvTrackbarEx;

typedef struct CvWindowEx
{
    int signature;
    HWND hwnd;
    char* name;
    CvWindowEx* prev;
    CvWindowEx* next;
    HWND frame;

    HDC dc;
    HGDIOBJ image;
    int last_key;
    int flags;
    int status;//0 normal, 1 fullscreen (YV)

    CvMouseCallback on_mouse;
    void* on_mouse_param;

    struct
    {
        HWND toolbar;
        int pos;
        int rows;
        WNDPROC toolBarProc;
        CvTrackbarEx* first;
    }
    toolbar;
}CvWindowEx;

static LRESULT CALLBACK HighGUIProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CvWindow* window = icvWindowByHWND(hwnd);
    if( !window )
        // This window is not mentioned in HighGUI storage
        // Actually, this should be error except for the case of calls to CreateWindow
        return DefWindowProc(hwnd, uMsg, wParam, lParam);

    // Process the message
    switch(uMsg)
    {
    case WM_WINDOWPOSCHANGING:
        {
            LPWINDOWPOS pos = (LPWINDOWPOS)lParam;
            RECT rect = icvCalcWindowRect(window);
            pos->x = rect.left;
            pos->y = rect.top;
            pos->cx = rect.right - rect.left + 1;
            pos->cy = rect.bottom - rect.top + 1;
        }
        break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE:
        if( window->on_mouse )
        {
            POINT pt;
            RECT rect;
            SIZE size = {0,0};

            int flags = (wParam & MK_LBUTTON ? CV_EVENT_FLAG_LBUTTON : 0)|
                (wParam & MK_RBUTTON ? CV_EVENT_FLAG_RBUTTON : 0)|
                (wParam & MK_MBUTTON ? CV_EVENT_FLAG_MBUTTON : 0)|
                (wParam & MK_CONTROL ? CV_EVENT_FLAG_CTRLKEY : 0)|
                (wParam & MK_SHIFT ? CV_EVENT_FLAG_SHIFTKEY : 0)|
                (GetKeyState(VK_MENU) < 0 ? CV_EVENT_FLAG_ALTKEY : 0);
            int event = uMsg == WM_LBUTTONDOWN ? CV_EVENT_LBUTTONDOWN :
                uMsg == WM_RBUTTONDOWN ? CV_EVENT_RBUTTONDOWN :
                uMsg == WM_MBUTTONDOWN ? CV_EVENT_MBUTTONDOWN :
                uMsg == WM_LBUTTONUP ? CV_EVENT_LBUTTONUP :
                uMsg == WM_RBUTTONUP ? CV_EVENT_RBUTTONUP :
                uMsg == WM_MBUTTONUP ? CV_EVENT_MBUTTONUP :
                uMsg == WM_LBUTTONDBLCLK ? CV_EVENT_LBUTTONDBLCLK :
                uMsg == WM_RBUTTONDBLCLK ? CV_EVENT_RBUTTONDBLCLK :
                uMsg == WM_MBUTTONDBLCLK ? CV_EVENT_MBUTTONDBLCLK :
                CV_EVENT_MOUSEMOVE;
            if( uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN )
                SetCapture( hwnd );
            if( uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONUP || uMsg == WM_MBUTTONUP )
                ReleaseCapture();

            pt.x = LOWORD( lParam );
            pt.y = HIWORD( lParam );

            GetClientRect( window->hwnd, &rect );
            icvGetBitmapData( window, &size, 0, 0 );

            window->on_mouse( event, pt.x*size.cx/MAX(rect.right - rect.left,1),
                pt.y*size.cy/MAX(rect.bottom - rect.top,1), flags,
                window->on_mouse_param );
        }
        break;

    case WM_PAINT:
        if(window->image != 0)
        {
            int nchannels = 3;
            SIZE size = {0,0};
            PAINTSTRUCT paint;
            HDC hdc;
            RGBQUAD table[256];

            // Determine the bitmap's dimensions
            icvGetBitmapData( window, &size, &nchannels, 0 );

            hdc = BeginPaint(hwnd, &paint);
            SetStretchBltMode(hdc, COLORONCOLOR);

            if( nchannels == 1 )
            {
                int i;
                for(i = 0; i < 256; i++)
                {
                    table[i].rgbBlue = (unsigned char)i;
                    table[i].rgbGreen = (unsigned char)i;
                    table[i].rgbRed = (unsigned char)i;
                }
                SetDIBColorTable(window->dc, 0, 255, table);
            }

            if(window->flags & CV_WINDOW_AUTOSIZE)
            {
                BitBlt( hdc, 0, 0, size.cx, size.cy, window->dc, 0, 0, SRCCOPY );
            }
            else
            {
                RECT rect;
                GetClientRect(window->hwnd, &rect);
                StretchBlt( hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
                    window->dc, 0, 0, size.cx, size.cy, SRCCOPY );
            }
            //DeleteDC(hdc);
            EndPaint(hwnd, &paint);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        return 0;

    case WM_ERASEBKGND:
        if(window->image)
            return 0;
        break;

    case WM_DESTROY:

        icvRemoveWindow(window);
        // Do nothing!!!
        //PostQuitMessage(0);
        break;

    case WM_SETCURSOR:
        SetCursor((HCURSOR)icvGetClassLongPtr(hwnd, CV_HCURSOR));
        return 0;

    case WM_KEYDOWN:
        window->last_key = (int)wParam;
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


static LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT ret;

    if( hg_on_preprocess )
    {
        int was_processed = 0;
        int ret = hg_on_preprocess(hwnd, uMsg, wParam, lParam, &was_processed);
        if( was_processed )
            return ret;
    }
    ret = HighGUIProc(hwnd, uMsg, wParam, lParam);

    if(hg_on_postprocess)
    {
        int was_processed = 0;
        int ret = hg_on_postprocess(hwnd, uMsg, wParam, lParam, &was_processed);
        if( was_processed )
            return ret;
    }

    return ret;
}

int cvInitSystemEx( int, char** )
{
    static int wasInitialized = 0;

    // check initialization status
    if( !wasInitialized )
    {
        // Initialize the stogare
        //hg_windows = 0;

        // Register the class
        WNDCLASS wndc;
        wndc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
        wndc.lpfnWndProc = WindowProc;
        wndc.cbClsExtra = 0;
        wndc.cbWndExtra = 0;
        wndc.hInstance = hg_hinstance;
        wndc.lpszClassName = highGUIclassName;
        wndc.lpszMenuName = highGUIclassName;
        wndc.hIcon = LoadIcon(0, IDI_APPLICATION);
        wndc.hCursor = (HCURSOR)LoadCursor(0, (LPSTR)(size_t)IDC_CROSS );
        wndc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);

        RegisterClass(&wndc);

        wndc.lpszClassName = mainHighGUIclassName;
        wndc.lpszMenuName = mainHighGUIclassName;
        wndc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
        wndc.lpfnWndProc = MainWindowProc;

        RegisterClass(&wndc);
        atexit( icvCleanupHighgui );

        wasInitialized = 1;
    }

    return 0;
}

 int cvNamedWindowEx( const char* name, int flags )
{
    int result = 0;
    //CV_FUNCNAME( "cvNamedWindow" );

    //__BEGIN__;

    HWND hWnd, mainhWnd;
    CvWindow* window;
    DWORD defStyle = WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
    int len;
    CvRect rect;

    cvInitSystem(0,0);

    if( !name )
        return 0;

    // Check the name in the storage
    if( icvFindWindowByName( name ) != 0 )
    {
        result = 1;
        EXIT;
    }

    if( !(flags & CV_WINDOW_AUTOSIZE))//YV add border in order to resize the window
        defStyle |= WS_SIZEBOX;

    icvLoadWindowPos( name, rect );

    mainhWnd = CreateWindow( "Main HighGUI class", name, defStyle | WS_OVERLAPPED,
        rect.x, rect.y, rect.width, rect.height, 0, 0, hg_hinstance, 0 );
    if( !mainhWnd )
        CV_ERROR( CV_StsError, "Frame window can not be created" );

    ShowWindow(mainhWnd, SW_SHOW);

    //YV- remove one border by changing the style
    hWnd = CreateWindow("HighGUI class", "", (defStyle & ~WS_SIZEBOX) | WS_CHILD, CW_USEDEFAULT, 0, rect.width, rect.height, mainhWnd, 0, hg_hinstance, 0);
    if( !hWnd )
        CV_ERROR( CV_StsError, "Frame window can not be created" );

    ShowWindow(hWnd, SW_SHOW);

    len = (int)strlen(name);
    CV_CALL( window = (CvWindow*)cvAlloc(sizeof(CvWindow) + len + 1));

    window->signature = CV_WINDOW_MAGIC_VAL;
    window->hwnd = hWnd;
    window->frame = mainhWnd;
    window->name = (char*)(window + 1);
    memcpy( window->name, name, len + 1 );
    window->flags = flags;
    window->image = 0;
    window->dc = CreateCompatibleDC(0);
    window->last_key = 0;
    window->status = CV_WINDOW_NORMAL;//YV

    window->on_mouse = 0;
    window->on_mouse_param = 0;

    memset( &window->toolbar, 0, sizeof(window->toolbar));

    window->next = hg_windows;
    window->prev = 0;
    if( hg_windows )
        hg_windows->prev = window;
    hg_windows = window;
    icvSetWindowLongPtr( hWnd, CV_USERDATA, window );
    icvSetWindowLongPtr( mainhWnd, CV_USERDATA, window );

    // Recalculate window position
    icvUpdateWindowPos( window );

    result = 1;
    //__END__;

    return result;
}
*/