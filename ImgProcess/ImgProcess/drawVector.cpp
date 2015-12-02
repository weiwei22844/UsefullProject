#include "stdafx.h"
#include <windows.h>
#include "resource.h"
#include <math.h>
#include <vector>
#include "json/json.h"

using namespace std;

#define PI 3.1415926

HINSTANCE hInstance = NULL;
HWND g_hDrawVectWnd = NULL;
BOOL g_bRegisterClass = FALSE;
char g_fileName[MAX_PATH] = {0};
BOOL g_firstDraw = TRUE;
BOOL g_readDxf = FALSE;
BOOL g_readJson = FALSE;
int g_nShiftX = 0;
int g_nShiftY = 0;
POINT downPoint;
HCURSOR g_hOldCursor = NULL;
char g_dbgStr[200];
float g_fZoomSize = 1;

typedef struct _drawingNode
{  
    BOOL bMove;
    int x;
    int y;
}drawingNode;
// not read path from file every time
std::vector <drawingNode> drawingPath;

int WaitForExit( int delay )
{
    int time0 = GetTickCount();

    for(;;)
    {
        MSG message;
        int is_processed = 0;

        if( (delay > 0 && abs((int)(GetTickCount() - time0)) >= delay) || g_hDrawVectWnd == NULL )
            return -1;

        if( delay <= 0 )
            GetMessage(&message, 0, 0, 0);
        else if( PeekMessage(&message, 0, 0, 0, PM_REMOVE) == FALSE )
        {
            Sleep(1);
            continue;
        }

        if( g_hDrawVectWnd == message.hwnd )
        {
            is_processed = 1;
            switch(message.message)
            {
            case WM_DESTROY:
            case WM_CHAR:
                DispatchMessage(&message);
                return (int)message.wParam;

            case WM_SYSKEYDOWN:
                if( message.wParam == VK_F10 )
                {
                    is_processed = 1;
                    return (int)(message.wParam << 16);
                }
                break;

            case WM_KEYDOWN:
                TranslateMessage(&message);
                if( (message.wParam >= VK_F1 && message.wParam <= VK_F24) ||
                    message.wParam == VK_HOME || message.wParam == VK_END ||
                    message.wParam == VK_UP || message.wParam == VK_DOWN ||
                    message.wParam == VK_LEFT || message.wParam == VK_RIGHT ||
                    message.wParam == VK_INSERT || message.wParam == VK_DELETE ||
                    message.wParam == VK_PRIOR || message.wParam == VK_NEXT )
                {
                    DispatchMessage(&message);
                    is_processed = 1;
                    return (int)(message.wParam << 16);
                }
            default:
                DispatchMessage(&message);
                is_processed = 1;
                break;
            }
        }

        if( !is_processed )
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
}

LRESULT CALLBACK mainProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc,hcmdc;
    PAINTSTRUCT ps;
    RECT rect;
    HBITMAP hBmp;
    SIZE   size;
    BITMAP bmp;
    HPEN pen;
    char temp[100];

    switch(uMsg)
    {
    case WM_CREATE:
        drawingPath.clear();
        downPoint.x = downPoint.y = 0;
        g_nShiftX = 0;
        g_nShiftY = 0;
        g_readDxf = FALSE;
        g_readJson = FALSE;
        break;
    case WM_PAINT:
        {
            HBRUSH hbrush = CreateSolidBrush(RGB(255,0,0));
            HBRUSH hbrold;
            hdc = BeginPaint(hwnd, &ps);
            //pen = CreatePen(PS_DOT,1,RGB(0,0,0));
            pen = CreatePen(PS_SOLID,1,RGB(0,0,0));
            SelectObject(hdc,pen);
            if(g_firstDraw){
                FILE *pFile;
                char str[50];
                BOOL bMoveTo;
                int x, y;
                int num;
                pFile=fopen(g_fileName,"r+");
                if(pFile){
                    if(strstr(g_fileName, ".ply")){
                        fseek(pFile,0,SEEK_SET);
                        num = 0;
                        bMoveTo = FALSE;
                        while(!feof(pFile)){
                            char *p;
                            drawingNode tmpNode;
                            fgets(str,sizeof(str),pFile);
                            OutputDebugString(str);
                            if(strstr(str, "POLYLINE")){
                                bMoveTo = TRUE;
                                continue;
                            }else if(strstr(str,"END")){
                                num++;
                                continue;
                            }
                            p = strtok(str, "     ");
                            //x = atoi(p) - 2000;
                            x = atoi(p);
                            p = strtok(NULL, "     ");
                            y = atoi(p);
                            if(bMoveTo){
                                tmpNode.bMove = TRUE;
                                sprintf(temp, "moveto %d %d\n", x, y);
                                MoveToEx(hdc,x,y,NULL);
                                bMoveTo = FALSE;
                            }else{
                                tmpNode.bMove = FALSE;
                                sprintf(temp, "lineto %d %d\n", x, y);
                                LineTo(hdc,x,y);
                            }
                            tmpNode.x = x;
                            tmpNode.y = y;
                            drawingPath.push_back(tmpNode);
                            OutputDebugString(temp);
                        }
                    }else if(strstr(g_fileName, ".json")){
                        Json::Features features;
                        Json::Reader reader( features );
                        Json::Value root;
                        char *pData = NULL;
                        char str[50];
                        BOOL bMoveTo;
                        int x, y;
                        int xFirst, yFirst;
                        int xLast, yLast;
                        int xLastMDis, yLastMDis;
                        int num = 0;
                        if(!g_readJson){
                            int nLen;
                            fseek(pFile,0,SEEK_END);
                            nLen=ftell(pFile);
                            fseek(pFile,0,SEEK_SET);
                            pData = (char*)malloc(nLen+1);
                            if(pData){
                                hdc = GetDC(hwnd);
                                HBRUSH hbrush = CreateSolidBrush(RGB(0,0,255));
                                HBRUSH hbrold;//= (HBRUSH)SelectObject(hdc, hbrush);
                                pen = CreatePen(PS_SOLID,1,RGB(255,0,0));
                                SelectObject(hdc,pen);
                                memset(pData, 0, nLen+1);
                                fread(pData,nLen,1,pFile);
                                std::string text(pData);
                                if (reader.parse(text, root)){
                                    if(root["features"].isArray()){
                                        Json::Value featureNodes = root["features"];
                                        int size = featureNodes.size();
                                        drawingNode tmpNode;
                                        for(int i = 0; i < size; i++){
                                            Json::Value featNode = featureNodes[i];
                                            Json::Value geoNode = featNode["geometry"];
                                            OutputDebugString(geoNode["type"].asCString());
                                            if(strcmp(geoNode["type"].asCString(), "Polygon") == 0){
                                                Json::Value coordinates = geoNode["coordinates"];
                                                int coordinatesSize = coordinates.size();
                                                for(int j = 0; j < coordinatesSize; j++){
                                                    Json::Value coordinatesNode = coordinates[j];
                                                    int coordinatesNodeSize = coordinatesNode.size();
                                                    for(int k = 0; k < coordinatesNodeSize; k++){
                                                        Json::Value xy = coordinatesNode[k];
                                                        int l = 0;
                                                        x = xy[l++].asInt();
                                                        //y = 550 - xy[l].asInt();
                                                        y = xy[l].asInt();
                                                        if(k == 0){
                                                            MoveToEx(hdc,x,y,NULL);
                                                            TRACE("MoveToEx %d %d\n", x, y);
                                                            xLast = x;
                                                            yLast = y;
                                                            xLastMDis = yLastMDis = 0;
                                                            tmpNode.x = x;
                                                            tmpNode.y = y;
                                                            tmpNode.bMove = TRUE;
                                                            if(drawingPath.size()>0){
                                                                vector <drawingNode>::iterator iter = drawingPath.end();
                                                                iter--;
                                                                if(iter->bMove)
                                                                    drawingPath.pop_back();
                                                            }
                                                            drawingPath.push_back(tmpNode);
                                                        }else{
                                                            if(x == xLast && y == yLast)
                                                                continue;
                                                            vector <drawingNode>::iterator iter = drawingPath.end();
                                                            iter--;
                                                            LineTo(hdc, x, y);
                                                            TRACE("lineto %d %d\n", x, y);
                                                            xLast = x;
                                                            yLast = y;
#if 0
                                                            tmpNode.x = x;
                                                            tmpNode.y = y;
                                                            tmpNode.bMove = FALSE;
                                                            drawingPath.push_back(tmpNode);
#else
                                                            int xMDis = x - iter->x;
                                                            int yMDis = y - iter->y;
                                                            if(iter->bMove){       // 上次动作是移动
                                                                xLastMDis = xMDis;
                                                                yLastMDis = yMDis;
                                                                tmpNode.x = x;
                                                                tmpNode.y = y;
                                                                tmpNode.bMove = FALSE;
                                                                drawingPath.push_back(tmpNode);
                                                            }else{                  // 上次动作是画线
                                                                if(xLastMDis == 0 && xMDis == 0){               // 沿y轴方向移动
                                                                    if(yLastMDis*yMDis > 0){
                                                                        iter->y += yMDis;
                                                                    }else{
                                                                        sprintf(g_dbgStr, "!!!!!!!!!!yLastMDis: %d yMDis: %d", yLastMDis, yMDis);
                                                                        OutputDebugString(g_dbgStr);
                                                                        //tmpNode.x = x;
                                                                        //tmpNode.y = y;
                                                                        //tmpNode.bMove = FALSE;
                                                                        //drawingPath.push_back(tmpNode);
                                                                        // fix this type
                                                                        iter--;
                                                                        tmpNode.x = x;
                                                                        drawingPath.pop_back();
                                                                        if(yLastMDis+yMDis){
                                                                            tmpNode.y = iter->y+yLastMDis+yMDis;
                                                                            drawingPath.push_back(tmpNode);
                                                                        }
                                                                    }
                                                                    yLastMDis += yMDis;
                                                                }else if(xLastMDis != 0 && xMDis != 0 && yLastMDis*xMDis == xLastMDis*yMDis){     // 斜率相同
                                                                    if(xLastMDis*xMDis > 0){            // 方向相同
                                                                        iter->x += xMDis;
                                                                        iter->y += yMDis;
                                                                    }else{
                                                                        sprintf(g_dbgStr, "!!!!!!!!!!xLastMDis: %d xMDis: %d yLastMDis: %d yMDis: %d", xLastMDis, xMDis, yLastMDis, yMDis);
                                                                        OutputDebugString(g_dbgStr);
                                                                        //tmpNode.x = x;
                                                                        //tmpNode.y = y;
                                                                        //tmpNode.bMove = FALSE;
                                                                        //drawingPath.push_back(tmpNode);
                                                                        // fix this type
                                                                        iter--;
                                                                        drawingPath.pop_back();
                                                                        if(xLastMDis+xMDis || yLastMDis+yMDis){
                                                                            tmpNode.x = iter->x+xLastMDis+xMDis;
                                                                            tmpNode.y = iter->y+yLastMDis+yMDis;
                                                                            drawingPath.push_back(tmpNode);
                                                                        }
                                                                    }
                                                                    xLastMDis += xMDis;
                                                                    yLastMDis += yMDis;
                                                                }else{
                                                                    tmpNode.x = x;
                                                                    tmpNode.y = y;
                                                                    tmpNode.bMove = FALSE;
                                                                    drawingPath.push_back(tmpNode);
                                                                    xLastMDis = xMDis;
                                                                    yLastMDis = yMDis;
                                                                }
                                                            }
#endif
                                                            num++;
                                                            //if(i > 0)
                                                            {
                                                                hbrold = (HBRUSH)SelectObject(hdc, hbrush);
                                                                Ellipse(hdc,x-2,y-2,x+2,y+2);
                                                                SelectObject(hdc, hbrold);
                                                            }
                                                        }
                                                    }
                                                    TRACE("coordinates node %d line %d ok", j, num);
                                                }
                                                TRACE("feature node %d line %d ok", i, num);
                                            }else{
                                                MessageBox(NULL, "please check", "exception", MB_OK);
                                            }
                                        }
                                        if(drawingPath.size()>0){
                                            vector <drawingNode>::iterator iter = drawingPath.end();
                                            iter--;
                                            if(iter->bMove)
                                                drawingPath.pop_back();
                                        }
                                    }
                                }

                                DeleteObject(hbrush);
                                ReleaseDC(hwnd,hdc);
                                free(pData);
                            }
                            fclose(pFile);
                            g_readJson = TRUE;

                            // change the path to G code
                            /*int num = 0;
                            FILE *pGFile = fopen("out.nc", "wb");
                            FILE *pGExFile = fopen("outDragen.nc", "wb");
                            if(pGFile && pGExFile){
                                char str[100];
                                double dAngle, dAngleLast;
                                dAngleLast = PI;
                                fwrite("SP1\n", strlen("SP1\n"), 1, pGExFile);
                                fwrite("MSP1=1\n", strlen("MSP1=1\n"), 1, pGExFile);
                                for(vector <drawingNode>::iterator it = drawingPath.begin(); it != drawingPath.end(); it++)
                                {
                                    int x = it->x;
                                    int y = it->y;
                                    if(it->bMove){
                                        sprintf(str, "G0 X%d Y%d F3000\n", x, y);
                                        fwrite(str, strlen(str), 1, pGFile);
                                        fwrite(str, strlen(str), 1, pGExFile);
                                        xLast = x;
                                        yLast = y;
                                    }else{
                                        sprintf(str, "G1 X%d Y%d F2000\n", x, y);
                                        fwrite(str, strlen(str), 1, pGFile);
                                        fwrite(str, strlen(str), 1, pGExFile);
                                        if(x-xLast == 0){
                                            dAngle = PI/2;
                                        }else{
                                            dAngle = atan((double)(y-yLast)/(x-xLast));
                                        }
                                        if(dAngleLast < PI){
                                            if(abs(dAngle-dAngleLast)>PI/4)
                                                fwrite("G91 Z10\n", strlen("G91 Z10\n"), 1, pGExFile);
                                        }
                                        dAngleLast = dAngle;
                                    }
                                }
                                fclose(pGFile);
                                fclose(pGExFile);
                            }*/
                        }
                    }
                    // change the path to G code
                    int xLast, yLast;
                    int num = 0;
                    FILE *pGFile = fopen("out.nc", "wb");
                    FILE *pGExFile = fopen("outDragen.nc", "wb");
                    if(pGFile && pGExFile){
                        char str[100];
                        double dAngle, dAngleLast;
                        dAngleLast = PI;
                        fwrite("SP1\n", strlen("SP1\n"), 1, pGExFile);
                        fwrite("MSP1=1\n", strlen("MSP1=1\n"), 1, pGExFile);
                        for(vector <drawingNode>::iterator it = drawingPath.begin(); it != drawingPath.end(); it++)
                        {
                            int x = it->x;
                            int y = it->y;
                            if(it->bMove){
                                sprintf(str, "G0 X%d Y%d F3000\n", x, y);
                                fwrite(str, strlen(str), 1, pGFile);
                                fwrite(str, strlen(str), 1, pGExFile);
                                xLast = x;
                                yLast = y;
                            }else{
                                sprintf(str, "G1 X%d Y%d F2000\n", x, y);
                                fwrite(str, strlen(str), 1, pGFile);
                                fwrite(str, strlen(str), 1, pGExFile);
                                if(x-xLast == 0){
                                    dAngle = PI/2;
                                }else{
                                    dAngle = atan((double)(y-yLast)/(x-xLast));
                                }
                                if(dAngleLast < PI){
                                    if(abs(dAngle-dAngleLast)>PI/4)
                                        fwrite("G91 Z10\n", strlen("G91 Z10\n"), 1, pGExFile);
                                }
                                dAngleLast = dAngle;
                            }
                        }
                        fclose(pGFile);
                        fclose(pGExFile);
                    }
                    
                    fclose(pFile);
                }
                //Arc(hdc,0,0,100,100,50,0,0,50);
                g_firstDraw = FALSE;
            }else{
                sprintf(g_dbgStr, "will redraw g_nShiftX: %d g_nShiftY: %d", g_nShiftX, g_nShiftY);
                OutputDebugString(g_dbgStr);
                int num = 0;
                for(vector <drawingNode>::iterator it = drawingPath.begin(); it != drawingPath.end(); it++)
                {
                    //int x = (it->x + g_nShiftX)*g_fZoomSize;
                    //int y = (it->y + g_nShiftY)*g_fZoomSize;
                    int x = it->x*g_fZoomSize+g_nShiftX;
                    int y = it->y*g_fZoomSize+g_nShiftY;
                    if(it->bMove){
                        sprintf(temp, "moveto %d %d\n", x, y);
                        MoveToEx(hdc, x, y, NULL);
                    }else{
                        sprintf(temp, "lineto %d %d\n", x, y);
                        LineTo(hdc, x, y);
                        hbrold = (HBRUSH)SelectObject(hdc, hbrush);
                        Ellipse(hdc,x-2,y-2,x+2,y+2);
                        SelectObject(hdc, hbrold);
                        num++;
                    }
                    OutputDebugString(temp);
                }
                sprintf(temp, "total %d lines", num);
                OutputDebugString(temp);
                //Arc(hdc,0,0,100,100,50,0,0,50);
                //Arc(hdc,0,0,100,100,0,50,50,0);
            }
            EndPaint(hwnd,&ps);
        }
        break;

    case WM_LBUTTONDOWN:
        downPoint.x = LOWORD( lParam );
        downPoint.y = HIWORD( lParam );
        SetCapture(g_hDrawVectWnd);
        g_hOldCursor = SetCursor(LoadCursor(NULL, IDC_HAND));
        break;

    case WM_LBUTTONUP:
        if(g_hOldCursor){
            ReleaseCapture();
            SetCursor(g_hOldCursor);
            g_hOldCursor = NULL;
            if(LOWORD( lParam )!=downPoint.x || HIWORD( lParam ) != downPoint.y){
                g_nShiftX += (LOWORD( lParam ) - downPoint.x);
                g_nShiftY += (HIWORD( lParam ) - downPoint.y);
                InvalidateRect(g_hDrawVectWnd, NULL, TRUE);
            }
        }
        break;

    case WM_KEYDOWN:
        switch(wParam)  
        {   
        case VK_UP:
            g_fZoomSize*=2;
            InvalidateRect(g_hDrawVectWnd, NULL, TRUE);
            break;
        case VK_DOWN:
            g_fZoomSize/=2;
            InvalidateRect(g_hDrawVectWnd, NULL, TRUE);
            break;
        case VK_RIGHT:
            {
                FILE *pFile;
                char str[50];
                BOOL bMoveTo;
                int x, y;
                int xFirst, yFirst;
                int num;
                if(!g_readDxf){
                    //pFile=fopen("RectCirR12Demo.dxf","r+");
                    pFile=fopen("dragonLR12Demo.dxf", "r+");
                    if(pFile){
                        hdc = GetDC(hwnd);
                        HBRUSH hbrush = CreateSolidBrush(RGB(0,0,255));
                        HBRUSH hbrold;//= (HBRUSH)SelectObject(hdc, hbrush);
                        pen = CreatePen(PS_SOLID,1,RGB(255,0,0));
                        SelectObject(hdc,pen);
                        fseek(pFile,0,SEEK_SET);
                        num = 0;   
                        bMoveTo = FALSE;
                        while(!feof(pFile)){
                            char *p;
                            drawingNode tmpNode;
                            fgets(str,sizeof(str),pFile);
                            OutputDebugString(str);
                            if(strstr(str, "VERTEX")){
                                if(!bMoveTo){
                                    bMoveTo = TRUE;
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    x = atoi(str);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    y = atoi(str);
                                    MoveToEx(hdc,x,y,NULL);
                                    hbrold = (HBRUSH)SelectObject(hdc, hbrush);
                                    Ellipse(hdc,x-2,y-2,x+2,y+2);
                                    SelectObject(hdc, hbrold);
                                    xFirst = x;
                                    yFirst = y;
                                    tmpNode.x = x;
                                    tmpNode.y = y;
                                    tmpNode.bMove = TRUE;
                                    drawingPath.push_back(tmpNode);
                                }else{
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    x = atoi(str);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    y = atoi(str);
                                    LineTo(hdc,x,y);
                                    hbrold = (HBRUSH)SelectObject(hdc, hbrush);
                                    Ellipse(hdc,x-2,y-2,x+2,y+2);
                                    SelectObject(hdc, hbrold);
                                    tmpNode.x = x;
                                    tmpNode.y = y;
                                    tmpNode.bMove = FALSE;
                                    drawingPath.push_back(tmpNode);
                                }
                                continue;
                            }else if(strstr(str, "POLYLINE")){
                                if(bMoveTo){
                                    tmpNode.x = xFirst;
                                    tmpNode.y = yFirst;
                                    tmpNode.bMove = FALSE;
                                    drawingPath.push_back(tmpNode);
                                    LineTo(hdc,xFirst,yFirst);
                                }
                                bMoveTo = FALSE;
                            }else if(strstr(str,"EOF")){
                                tmpNode.x = xFirst;
                                tmpNode.y = yFirst;
                                tmpNode.bMove = FALSE;
                                drawingPath.push_back(tmpNode);
                                LineTo(hdc,xFirst,yFirst);
                                break;
                            }
                        }
                        
                        DeleteObject(hbrush);
                        fclose(pFile);
                        ReleaseDC(hwnd,hdc);
                    }
                    g_readDxf = TRUE;
                }
            }
            break;
        case VK_LEFT:
            {
                FILE *pFile;
                char str[50];
                BOOL bMoveTo;
                double x, y;
                double xFirst, yFirst;
                double xLast, yLast;
                int num;
                float fBulge = 0;
                if(!g_readDxf){
                    pFile=fopen("dragonL.dxf", "r+");
                    //pFile=fopen("RectCir.dxf", "r+");
                    //pFile=fopen("niu.dxf", "r+");
                    if(pFile){
                        hdc = GetDC(hwnd);
                        HBRUSH hbrush = CreateSolidBrush(RGB(0,0,255));
                        HBRUSH hbrold;//= (HBRUSH)SelectObject(hdc, hbrush);
                        pen = CreatePen(PS_SOLID,1,RGB(255,0,0));
                        SelectObject(hdc,pen);
                        fseek(pFile,0,SEEK_SET);
                        num = 0;   
                        bMoveTo = FALSE;
                        while(!feof(pFile)){
                            char *p;
                            drawingNode tmpNode;
                            fgets(str,sizeof(str),pFile);
                            OutputDebugString(str);
                            if(strstr(str, "VERTEX")){
                                if(!bMoveTo){
                                    bMoveTo = TRUE;
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    x = atof(str);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    y = atof(str);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    fBulge = atof(str);
                                    MoveToEx(hdc,x,y,NULL);
                                    //hbrold = (HBRUSH)SelectObject(hdc, hbrush);
                                    //Ellipse(hdc,x-2,y-2,x+2,y+2);
                                    //SelectObject(hdc, hbrold);
                                    xFirst = xLast = x;
                                    yFirst = yLast = y;
                                }else{
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    x = atof(str);
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    y = atof(str);
                                    if(fBulge == 0)
                                        LineTo(hdc,x,y);
                                    else{
                                        float fBulgeAbs = fabs(fBulge);
                                        double theta_arc = atan(fBulgeAbs) * 4;
                                        double theta_degree = theta_arc*180/PI;
                                        int xCen, yCen;
                                        double dfR = 0;     // 半径
                                        // 计算圆心及半径
                                        if( fabs(180 - fabs(theta_degree)) < 0.1 )   //半圆，直接计算中心
                                        {
                                            xCen = (x + xLast)/2;
                                            yCen = (y + yLast)/2;
                                            dfR = sqrt(pow(double(x - xLast),2) + pow(double(y - yLast),2))/2;
                                        }
                                        else
                                        {
                                            double len = sqrt(pow(double(x - xLast),2) + pow(double(y - yLast),2)); //弦长
                                            double sinValue = sin(fabs(theta_arc)/2.0);
                                            dfR = len/(2.0*sinValue);
                                            POINT cen1,cen2;   //暂存两个圆心
                                            if (y == yLast)    //中垂线平行与y轴
                                            {
                                                //中垂线为 x = （x1 + x2)/2
                                                //以x1,y1为圆心，dfR为半径作圆，求中垂线与圆交点
                                                //(x-x1)*(x-x1) + (y-y1)*(y-y1) = dfR*dfR
                                                double dfDiff = pow(dfR,2) - pow(double((x-xLast)/2),2);
                                                if (dfDiff < 0){
                                                    break;
                                                }
                                                cen1.x = cen2.x = (x+xLast)/2;
                                                cen1.y = yLast + sqrt(dfDiff);
                                                cen2.y = yLast - sqrt(dfDiff);
                                            }
                                            else
                                            {
                                                //中垂线 y = kx + bk
                                                double k = -(x-xLast)/(y-yLast); //中垂线斜率
                                                double bk = 0.5*(pow(double(y),2) - pow(double(yLast),2) + pow(double(x),2) - pow(double(xLast),2))/(y-yLast);

                                                //以x1,y1为圆心，dfR为半径作圆，求中垂线与圆交点
                                                //(x-x1)*(x-x1) + (y-y1)*(y-y1) = dfR*dfR
                                                double a = pow(k,2) + 1;
                                                double b = 2 *(k*(bk-yLast)-xLast);
                                                double c = pow(double(xLast),2) + pow((bk-yLast),2) - pow(dfR,2);
                                                double delta = pow(b,2) - 4*a*c;
                                                if (delta < 0)
                                                    break;

                                                cen1.x = (-b + sqrt(delta))/(2*a);
                                                cen1.y = k*cen1.x + bk;
                                                cen2.x = (-b - sqrt(delta))/(2*a);
                                                cen2.y = k*cen2.x + bk;
                                            }
                                            //根据矢量判断 P0为起点，P2为终点，P1为圆心（即判断P1P0 差乘 P1P2）
                                            //P1P0 X P1P2  =  lP1P0l*lP1P2l*Sin(theta) =  (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
                                            //那么该值应该与弧度方向一致，在弧度大于PI（3.14159265）的时候为另外一种情况
                                            POINT p0 = cen1;
                                            POINT p2 = {x, y};
                                            POINT p1 = {xLast, yLast};
                                            double dfDiff = (p1.x - p0.x)*(p2.y - p0.y) - (p2.x - p0.x)*(p1.y - p0.y);
                                            if (fabs(theta_arc) < PI)
                                            {
                                                if (dfDiff * theta_arc > 0)
                                                {
                                                    xCen = cen1.x;
                                                    yCen = cen1.y;
                                                }
                                                else
                                                {
                                                    xCen = cen2.x;
                                                    yCen = cen2.y;
                                                }
                                            }
                                            else
                                            {
                                                if (dfDiff * theta_arc < 0)
                                                {
                                                    xCen = cen1.x;
                                                    yCen = cen1.y;
                                                }
                                                else
                                                {
                                                    xCen = cen2.x;
                                                    yCen = cen2.y;
                                                }
                                            }
                                        }
                                        //Arc(hdc,xCen-dfR,yCen-dfR,xCen+dfR,yCen+dfR,xLast,yLast,x,y);
                                        Arc(hdc,xCen-dfR,yCen-dfR,xCen+dfR,yCen+dfR,x,y,xLast,yLast);
                                        MoveToEx(hdc,x,y,NULL);
                                    }
                                    fgets(str,sizeof(str),pFile);
                                    fgets(str,sizeof(str),pFile);
                                    fBulge = atof(str);
                                    xLast = x;
                                    yLast = y;
                                    hbrold = (HBRUSH)SelectObject(hdc, hbrush);
                                    Ellipse(hdc,x-2,y-2,x+2,y+2);
                                    SelectObject(hdc, hbrold);
                                }
                                continue;
                            }else if(strstr(str, "POLYLINE")){
                                if(bMoveTo){
                                    tmpNode.x = xFirst;
                                    tmpNode.y = yFirst;
                                    tmpNode.bMove = FALSE;
                                    drawingPath.push_back(tmpNode);
                                    LineTo(hdc,xFirst,yFirst);
                                }
                                bMoveTo = FALSE;
                            }else if(strstr(str,"EOF")){
                                tmpNode.x = xFirst;
                                tmpNode.y = yFirst;
                                tmpNode.bMove = FALSE;
                                drawingPath.push_back(tmpNode);
                                LineTo(hdc,xFirst,yFirst);
                                break;
                            }
                        }

                        DeleteObject(hbrush);
                        fclose(pFile);
                        ReleaseDC(hwnd,hdc);
                    }
                    g_readDxf = TRUE;
                }
            }
            break;
        case VK_SPACE:
            {
                Json::Features features;
                Json::Reader reader( features );
                Json::Value root;
                char *pData = NULL;
                FILE *pFile;
                char str[50];
                BOOL bMoveTo;
                int x, y;
                int xFirst, yFirst;
                int xLast, yLast;
                int xLastMDis, yLastMDis;
                int num = 0;
                if(!g_readJson){
                    pFile=fopen("dragonLSmall4.json", "r+");
                    if(pFile)
                    {
                        int nLen;
                        fseek(pFile,0,SEEK_END);
                        nLen=ftell(pFile);
                        fseek(pFile,0,SEEK_SET);
                        pData = (char*)malloc(nLen+1);
                        if(pData){
                            hdc = GetDC(hwnd);
                            HBRUSH hbrush = CreateSolidBrush(RGB(0,0,255));
                            HBRUSH hbrold;//= (HBRUSH)SelectObject(hdc, hbrush);
                            pen = CreatePen(PS_SOLID,1,RGB(255,0,0));
                            SelectObject(hdc,pen);
                            memset(pData, 0, nLen+1);
                            fread(pData,nLen,1,pFile);
                            std::string text(pData);
                            if (reader.parse(text, root)){
                                if(root["features"].isArray()){
                                    Json::Value featureNodes = root["features"];
                                    int size = featureNodes.size();
                                    drawingNode tmpNode;
                                    for(int i = 0; i < size; i++){
                                        Json::Value featNode = featureNodes[i];
                                        Json::Value geoNode = featNode["geometry"];
                                        OutputDebugString(geoNode["type"].asCString());
                                        if(strcmp(geoNode["type"].asCString(), "Polygon") == 0){
                                            Json::Value coordinates = geoNode["coordinates"];
                                            int coordinatesSize = coordinates.size();
                                            for(int j = 0; j < coordinatesSize; j++){
                                                Json::Value coordinatesNode = coordinates[j];
                                                int coordinatesNodeSize = coordinatesNode.size();
                                                for(int k = 0; k < coordinatesNodeSize; k++){
                                                    Json::Value xy = coordinatesNode[k];
                                                    int l = 0;
                                                    x = xy[l++].asInt();
                                                    //y = 550 - xy[l].asInt();
                                                    y = xy[l].asInt();
                                                    if(k == 0){
                                                        MoveToEx(hdc,x,y,NULL);
                                                        TRACE("MoveToEx %d %d\n", x, y);
                                                        xLast = x;
                                                        yLast = y;
                                                        xLastMDis = yLastMDis = 0;
                                                        tmpNode.x = x;
                                                        tmpNode.y = y;
                                                        tmpNode.bMove = TRUE;
                                                        if(drawingPath.size()>0){
                                                            vector <drawingNode>::iterator iter = drawingPath.end();
                                                            iter--;
                                                            if(iter->bMove)
                                                                drawingPath.pop_back();
                                                        }
                                                        drawingPath.push_back(tmpNode);
                                                    }else{
                                                        if(x == xLast && y == yLast)
                                                            continue;
                                                        vector <drawingNode>::iterator iter = drawingPath.end();
                                                        iter--;
                                                        LineTo(hdc, x, y);
                                                        TRACE("lineto %d %d\n", x, y);
                                                        xLast = x;
                                                        yLast = y;
#if 0
                                                        tmpNode.x = x;
                                                        tmpNode.y = y;
                                                        tmpNode.bMove = FALSE;
                                                        drawingPath.push_back(tmpNode);
#else
                                                        int xMDis = x - iter->x;
                                                        int yMDis = y - iter->y;
                                                        if(iter->bMove){       // 上次动作是移动
                                                            xLastMDis = xMDis;
                                                            yLastMDis = yMDis;
                                                            tmpNode.x = x;
                                                            tmpNode.y = y;
                                                            tmpNode.bMove = FALSE;
                                                            drawingPath.push_back(tmpNode);
                                                        }else{                  // 上次动作是画线
                                                            if(xLastMDis == 0 && xMDis == 0){               // 沿y轴方向移动
                                                                if(yLastMDis*yMDis > 0){
                                                                    iter->y += yMDis;
                                                                }else{
                                                                    sprintf(g_dbgStr, "!!!!!!!!!!yLastMDis: %d yMDis: %d", yLastMDis, yMDis);
                                                                    OutputDebugString(g_dbgStr);
                                                                    //tmpNode.x = x;
                                                                    //tmpNode.y = y;
                                                                    //tmpNode.bMove = FALSE;
                                                                    //drawingPath.push_back(tmpNode);
                                                                    // fix this type
                                                                    iter--;
                                                                    tmpNode.x = x;
                                                                    drawingPath.pop_back();
                                                                    if(yLastMDis+yMDis){
                                                                        tmpNode.y = iter->y+yLastMDis+yMDis;
                                                                        drawingPath.push_back(tmpNode);
                                                                    }
                                                                }
                                                                yLastMDis += yMDis;
                                                            }else if(xLastMDis != 0 && xMDis != 0 && yLastMDis*xMDis == xLastMDis*yMDis){     // 斜率相同
                                                                if(xLastMDis*xMDis > 0){            // 方向相同
                                                                    iter->x += xMDis;
                                                                    iter->y += yMDis;
                                                                }else{
                                                                    sprintf(g_dbgStr, "!!!!!!!!!!xLastMDis: %d xMDis: %d yLastMDis: %d yMDis: %d", xLastMDis, xMDis, yLastMDis, yMDis);
                                                                    OutputDebugString(g_dbgStr);
                                                                    //tmpNode.x = x;
                                                                    //tmpNode.y = y;
                                                                    //tmpNode.bMove = FALSE;
                                                                    //drawingPath.push_back(tmpNode);
                                                                    // fix this type
                                                                    iter--;
                                                                    drawingPath.pop_back();
                                                                    if(xLastMDis+xMDis || yLastMDis+yMDis){
                                                                        tmpNode.x = iter->x+xLastMDis+xMDis;
                                                                        tmpNode.y = iter->y+yLastMDis+yMDis;
                                                                        drawingPath.push_back(tmpNode);
                                                                    }
                                                                }
                                                                xLastMDis += xMDis;
                                                                yLastMDis += yMDis;
                                                            }else{
                                                                tmpNode.x = x;
                                                                tmpNode.y = y;
                                                                tmpNode.bMove = FALSE;
                                                                drawingPath.push_back(tmpNode);
                                                                xLastMDis = xMDis;
                                                                yLastMDis = yMDis;
                                                            }
                                                        }
#endif
                                                        num++;
                                                        //if(i > 0)
                                                        {
                                                            hbrold = (HBRUSH)SelectObject(hdc, hbrush);
                                                            Ellipse(hdc,x-2,y-2,x+2,y+2);
                                                            SelectObject(hdc, hbrold);
                                                        }
                                                    }
                                                }
                                                TRACE("coordinates node %d line %d ok", j, num);
                                            }
                                            TRACE("feature node %d line %d ok", i, num);
                                        }else{
                                            MessageBox(NULL, "please check", "exception", MB_OK);
                                        }
                                    }
                                    if(drawingPath.size()>0){
                                        vector <drawingNode>::iterator iter = drawingPath.end();
                                        iter--;
                                        if(iter->bMove)
                                            drawingPath.pop_back();
                                    }
                                }
                            }

                            DeleteObject(hbrush);
                            ReleaseDC(hwnd,hdc);
                            free(pData);
                        }
                        fclose(pFile);
                        g_readJson = TRUE;

                        // change the path to G code
                        int num = 0;
                        FILE *pGFile = fopen("outDragen.nc", "wb");
                        if(pGFile){
                            char str[100];
                            double dAngle, dAngleLast;
                            dAngleLast = PI;
                            fwrite("SP1\n", strlen("SP1\n"), 1, pGFile);
                            fwrite("MSP1=1\n", strlen("MSP1=1\n"), 1, pGFile);
                            for(vector <drawingNode>::iterator it = drawingPath.begin(); it != drawingPath.end(); it++)
                            {
                                int x = it->x;
                                int y = it->y;
                                if(it->bMove){
                                    sprintf(str, "G0 X%d Y%d F3000\n", x, y);
                                    fwrite(str, strlen(str), 1, pGFile);
                                    xLast = x;
                                    yLast = y;
                                }else{
                                    sprintf(str, "G1 X%d Y%d F2000\n", x, y);
                                    fwrite(str, strlen(str), 1, pGFile);
                                    if(x-xLast == 0){
                                        dAngle = PI/2;
                                    }else{
                                        dAngle = atan((double)(y-yLast)/(x-xLast));
                                    }
                                    if(dAngleLast < PI){
                                        if(abs(dAngle-dAngleLast)>PI/4)
                                            fwrite("G91 Z10\n", strlen("G91 Z10\n"), 1, pGFile);
                                    }
                                    dAngleLast = dAngle;
                                }
                            }
                            fclose(pGFile);
                        }
                    }
                }
            }
            break;
        case VK_SHIFT:
            drawingPath.clear();
            downPoint.x = downPoint.y = 0;
            g_nShiftX = 0;
            g_nShiftY = 0;
            g_readDxf = FALSE;
            g_readJson = FALSE;
            InvalidateRect(g_hDrawVectWnd, NULL, TRUE);
            break;
        case 'S':
            {
                int iWidth = 1000*g_fZoomSize;
                int iHeight = 550*g_fZoomSize;
                int iPixel  = 16;
                //图形格式参数
                LPBITMAPINFO lpbmih = new BITMAPINFO;
                lpbmih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                lpbmih->bmiHeader.biWidth = iWidth;
                lpbmih->bmiHeader.biHeight = iHeight;
                lpbmih->bmiHeader.biPlanes = 1;
                lpbmih->bmiHeader.biBitCount = iPixel;
                lpbmih->bmiHeader.biCompression = BI_RGB;
                lpbmih->bmiHeader.biSizeImage = 0;
                lpbmih->bmiHeader.biXPelsPerMeter = 0;
                lpbmih->bmiHeader.biYPelsPerMeter = 0;
                lpbmih->bmiHeader.biClrUsed = 0;
                lpbmih->bmiHeader.biClrImportant = 0;

                //创建位图数据
                HDC hdc,hdcMem;
                HBITMAP hBitMap = NULL;
                CBitmap *pBitMap = NULL;
                CDC *pMemDC = NULL;
                BYTE *pBits;

                hdc = CreateIC(TEXT("DISPLAY"),NULL,NULL,NULL);
                hdcMem = CreateCompatibleDC(hdc);
                hBitMap = CreateDIBSection(hdcMem,lpbmih,DIB_PAL_COLORS,(void **)&pBits,NULL,0);
                int e = GetLastError();
                pBitMap = new CBitmap;
                pBitMap->Attach(hBitMap);
                pMemDC = new CDC;
                pMemDC->Attach(hdcMem);
                pMemDC->SelectObject(pBitMap);
                pMemDC->SetBkMode(TRANSPARENT);
                //pMemDC->SetBkColor(RGB(255,255,255));
                //添加自绘图形 
                //pMemDC->Ellipse(0,0,100,100);
                pMemDC->SelectObject(CPen(PS_SOLID,0,RGB(255,255,255)));
                pMemDC->Rectangle(0,0,iWidth,iHeight);
                CPen pen;
                pen.CreatePen(PS_SOLID,1,RGB(255,0,0));
                pMemDC->SelectObject(pen);
                pMemDC->SelectStockObject(NULL_BRUSH);
                for(vector <drawingNode>::iterator it = drawingPath.begin(); it != drawingPath.end(); it++)
                {
                    int x = it->x*g_fZoomSize+g_nShiftX;
                    int y = it->y*g_fZoomSize+g_nShiftY;
                    if(it->bMove){
                        pMemDC->MoveTo(x, y);
                    }else{
                        pMemDC->LineTo(x, y);
                    }
                    HGDIOBJ hOldBrush = pMemDC->SelectObject(CreateSolidBrush(RGB(0,0,255)));
                    pMemDC->Ellipse(x-2,y-2,x+2,y+2);
                    pMemDC->SelectObject(hOldBrush);
                }
                //保存到文件并创建位图结构
                BITMAPFILEHEADER bmfh;
                ZeroMemory(&bmfh,sizeof(BITMAPFILEHEADER));
                *((char *)&bmfh.bfType) = 'B';
                *(((char *)&bmfh.bfType) + 1) = 'M';
                bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
                bmfh.bfSize = bmfh.bfOffBits + (iWidth * iHeight) * iPixel / 8;

                TCHAR szBMPFileName[128];
                UINT uBMPBytes = iWidth * iHeight * iPixel / 8;
                strcpy(szBMPFileName,"vector.bmp");
                
                /*CFile file;
                if(file.Open(szBMPFileName,CFile::modeWrite | CFile::modeCreate))
                {
                    file.Write(&bmfh,sizeof(BITMAPFILEHEADER));
                    file.Write(&(lpbmih->bmiHeader),sizeof(BITMAPINFOHEADER));
                    file.Write(pBits,uBMPBytes);
                    file.Close();
                }*/
                FILE *pFile = NULL;
                pFile = fopen(szBMPFileName, "wb");
                if(pFile){
                    fwrite(&bmfh,sizeof(BITMAPFILEHEADER), 1, pFile);
                    fwrite(&(lpbmih->bmiHeader),sizeof(BITMAPINFOHEADER), 1, pFile);
                    if(fwrite(pBits, uBMPBytes, 1, pFile) != 1){
                        MessageBox(hwnd, "保存路径bmp文件失败", "error", MB_OK);
                    }
                    fclose(pFile);
                }

                pMemDC->DeleteDC();
                delete pMemDC;    pMemDC  = NULL;
                delete pBitMap; pBitMap = NULL;
                delete lpbmih;  lpbmih  = NULL;
            }
            break;
        default:
            sprintf(g_dbgStr, "%d", wParam);
            OutputDebugString(g_dbgStr);
            break;
        }
        break;

    case WM_DESTROY:
        //PostQuitMessage(0);
        g_hDrawVectWnd = NULL;
        break;
    }

    return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

int showVectorWnd(char* pFileName)
{
    WNDCLASS wndclass;
    MSG msg;
    
    if(!pFileName)
        return 0;
    if(g_hDrawVectWnd){
        ShowWindow(g_hDrawVectWnd, SW_SHOW);
        return 1;
    }
    g_firstDraw = TRUE;
    strcpy(g_fileName, pFileName);

    //设计窗口类
    wndclass.cbClsExtra=0;
    wndclass.cbWndExtra=0;
    wndclass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
    wndclass.hIcon=LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDR_MAINFRAME));
    wndclass.hInstance=hInstance;
    wndclass.lpfnWndProc=mainProc;
    wndclass.lpszClassName="drawVector";
    wndclass.lpszMenuName=NULL;
    wndclass.style=CS_HREDRAW | CS_VREDRAW;
    
    //注册窗口类
    if(!g_bRegisterClass){
        if(!RegisterClass(&wndclass))
        {
            MessageBox(NULL, "create windows error!", "error", MB_OK | MB_ICONSTOP);
        }else{
            g_bRegisterClass = TRUE;
        }
    }

    //创建无菜单资源的窗口窗口
    g_hDrawVectWnd=CreateWindow("drawVector","vector file drawing", WS_MAXIMIZE | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME, 0, 0, 1000, 600, NULL, NULL, hInstance, NULL);
    if(g_hDrawVectWnd){
        //显示更新窗口
        ShowWindow(g_hDrawVectWnd, SW_SHOW);
        UpdateWindow(g_hDrawVectWnd);

        WaitForExit(0);
    }else{
        ;
    }

    return 0;
}

void destroyVectorWnd()
{
    if(g_hDrawVectWnd){
        SendMessage(g_hDrawVectWnd, WM_CLOSE, 0, 0);
    }
}