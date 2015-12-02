#include "stdafx.h"
#include "RegionGrowth.h"

g_point::g_point(int x_i, int y_i)  
{  
	x = x_i;  
	y = y_i;  
	lbl = SEED;  
}  

g_point::g_point()  
{  
	x = 0;  
	y = 0;  
	lbl = INIT;  
}  

g_point::g_point(int x_i, int y_i, int lbl_i)  
{  
	x = x_i;  
	y = y_i;  
	lbl = lbl_i;  
}  

void regionGrowth(IplImage* img, int* p_mat, int x_g, int y_g, int threshold_g)  
{  
	//常用变量：  
	int x;  
	int y;  
	int gradient = 0;  


	//设置前次边缘点  
	std::list<class g_point> cont_pts_pre;  
	std::list<class g_point> * pcont_pts_pre = &cont_pts_pre;


	//设置边缘点  
	std::list<class g_point> cont_pts;  
	std::list<class g_point> * pcont_pts = &cont_pts;


	//种子点+边缘点 = 下次的种子点  
	//上次的边缘点 = 前次边缘点（用于下次减少点数)   
	cont_pts_pre.push_back(class g_point(x_g, y_g, CONT));  
	p_mat[y_g * img->width + x_g] = SEED;  

	std::list<class g_point>::iterator iter_cont;  
	std::list<class g_point>::iterator iter_prt;  
	std::list<class g_point>::iterator iter_swap;  

	while( !cont_pts_pre.empty() )  
	{
		//一轮生长  
		iter_cont = cont_pts_pre.begin();  
		while(iter_cont != cont_pts_pre.end())  
		{  
			x = (*iter_cont).x;  
			y = (*iter_cont).y;  
			//         if( !(x-1<0 || y-1<0) )                       //#1  
			//         {     
			//             if(p_mat[(y-1)*img->width + (x-1)] == INIT)  
			//             {  
			//                 gradient = ((char*)(img->imageData + y*img->widthStep))[x] -   
			//                     ((char*)(img->imageData + (y-1)*img->widthStep))[x-1];  
			//                 if(abs(gradient) < threshold_g)      //满足阈值要求  
			//                 {  
			//                     cont_pts.push_back(class g_point(x-1, y-1, CONT));  
			//                     p_mat[(y-1)*img->width + (x-1)] = SEED;  
			//                 }  
			//                 else                                //不满足阈值要求  
			//                 {  
			//                     p_mat[(y-1)*img->width + (x-1)] = INVAL;  
			//                 }  
			//             }  
			//         }  
			if( !(x-1<0) )                                   //#2  
			{  
				if(p_mat[(y)*img->width + (x-1)] == INIT)  
				{  
					gradient = ((char*)(img->imageData + y*img->widthStep))[x] -   
						((char*)(img->imageData + (y)*img->widthStep))[x-1];  
					if(abs(gradient) < threshold_g)      //满足阈值要求  
					{  
						cont_pts.push_back(class g_point(x-1, y, CONT));  
						p_mat[(y)*img->width + (x-1)] = SEED;  
					}  
					else                                //不满足阈值要求  
					{  
						p_mat[(y)*img->width + (x-1)] = INVAL;  
					}         

				}  
			}  
			//         if( !(x-1<0 || y+1 >= img->height) )           //#3  
			//         {  
			//             if(p_mat[(y+1)*img->width + (x-1)] == INIT)  
			//             {  
			//                 gradient = ((char*)(img->imageData + y*img->widthStep))[x] -   
			//                     ((char*)(img->imageData + (y+1)*img->widthStep))[x-1];  
			//                 if(abs(gradient) < threshold_g)      //满足阈值要求  
			//                 {  
			//                     cont_pts.push_back(class g_point(x-1, y+1, CONT));  
			//                     p_mat[(y+1)*img->width + (x-1)] = SEED;  
			//                 }  
			//                 else                                //不满足阈值要求  
			//                 {  
			//                     p_mat[(y+1)*img->width + (x-1)] = INVAL;  
			//                 }  
			//                   
			//             }  
			//         }  
			if( !(y+1 >= img->height) )                       //#4  
			{  
				if(p_mat[(y+1)*img->width + (x)] == INIT)  
				{  
					gradient = ((char*)(img->imageData + y*img->widthStep))[x] -   
						((char*)(img->imageData + (y+1)*img->widthStep))[x];  
					if(abs(gradient) < threshold_g)      //满足阈值要求  
					{  
						cont_pts.push_back(class g_point(x, y+1, CONT));  
						p_mat[(y+1)*img->width + (x)] = SEED;  
					}  
					else                                //不满足阈值要求  
					{  
						p_mat[(y+1)*img->width + (x)] = INVAL;  
					}  
				}  
			}  
			//         if( !(x+1>=img->width || y+1>=img->height) )    //#5  
			//         {  
			//             if(p_mat[(y+1)*img->width + (x+1)] == INIT)  
			//             {  
			//                 gradient = ((char*)(img->imageData + y*img->widthStep))[x] -   
			//                     ((char*)(img->imageData + (y+1)*img->widthStep))[x+1];  
			//                 if(abs(gradient) < threshold_g)      //满足阈值要求  
			//                 {  
			//                     cont_pts.push_back(class g_point(x+1, y+1, CONT));  
			//                     p_mat[(y+1)*img->width + (x+1)] = SEED;  
			//                 }  
			//                 else                                //不满足阈值要求  
			//                 {  
			//                     p_mat[(y+1)*img->width + (x+1)] = INVAL;  
			//                 }  
			//                   
			//             }  
			//         }  
			if( !(x+1>=img->width) )                      //#6  
			{  
				if(p_mat[(y)*img->width + (x+1)] == INIT)  
				{  
					gradient = ((char*)(img->imageData + y*img->widthStep))[x] -   
						((char*)(img->imageData + (y)*img->widthStep))[x+1];  
					if(abs(gradient) < threshold_g)      //满足阈值要求  
					{  
						cont_pts.push_back(class g_point(x+1, y, CONT));  
						p_mat[(y)*img->width + (x+1)] = SEED;  
					}  
					else                                //不满足阈值要求  
					{  
						p_mat[(y)*img->width + (x+1)] = INVAL;  
					}  

				}  
			}  
			//         if( !(x+1>=img->width || y-1<0) )              //#7  
			//         {  
			//             if(p_mat[(y-1)*img->width + (x+1)] == INIT)  
			//             {  
			//                 gradient = ((char*)(img->imageData + y*img->widthStep))[x] -   
			//                     ((char*)(img->imageData + (y-1)*img->widthStep))[x+1];  
			//                 if(abs(gradient) < threshold_g)      //满足阈值要求  
			//                 {  
			//                     cont_pts.push_back(class g_point(x+1, y-1, CONT));  
			//                     p_mat[(y-1)*img->width + (x+1)] = SEED;  
			//                 }  
			//                 else                                //不满足阈值要求  
			//                 {  
			//                     p_mat[(y-1)*img->width + (x+1)] = INVAL;  
			//                 }  
			//                   
			//             }  
			//         }  
			if( !(y-1<0) )                                   //#8  
			{  
				if(p_mat[(y-1)*img->width + (x)] == INIT)  
				{  
					gradient = ((char*)(img->imageData + y*img->widthStep))[x] -   
						((char*)(img->imageData + (y-1)*img->widthStep))[x];  
					if(abs(gradient) < threshold_g)      //满足阈值要求  
					{  
						cont_pts.push_back(class g_point(x, y-1, CONT));  
						p_mat[(y-1)*img->width + (x)] = SEED;  
					}  
					else                                //不满足阈值要求  
					{  
						p_mat[(y-1)*img->width + (x)] = INVAL;  
					}  
				}  
			}  

			iter_cont ++;  
		}  

		//将cont_pts中的点赋给cont_pts_pre  
		cont_pts_pre.clear();
		iter_swap = cont_pts.begin();  
		while(iter_swap != cont_pts.end() )  
		{  
			cont_pts_pre.push_back((*iter_swap));  
			iter_swap ++;  
		}
		cont_pts.clear();
	}  
	//结束while  

	cont_pts_pre.clear(); 
	cont_pts.clear();
	// 	delete pseed_pts;
	// 	delete pcont_pts_pre;
	// 	delete pcont_pts;
}  

void display_mat(int * p_mat, int width, int height, int id = 0)
{
	IplImage* disp;
	int color_templ[4*6*3]
	= {
		0,0,121,	93,0,158,	91,20,237,	255,61,255,	170,110,240,	200,161,245,
		76,0,13,	113,52,0,	127,91,0,	188,114,0,	203,140,68,		236,211,190,
		38,88,0,	77,118,46,	66,130,98,	112,182,46,	118,197,124,	174,221,178,
		19,57,96,	10,98,163,	0,160,171,	0,242,255,	59,245,255,		139,247,255
		};
	int gray_templ[1*6*3]
	= {
		0,0,0,		40,40,40,	80,80,80,	120,120,120,160,160,160,	200,200,200,
		};

	disp = cvCreateImage( cvSize(width, height), 8, 3 );
	cvZero(disp);

	int nid = 1;
	for(int h = 0; h < height; h++)
	{
		for(int w = 0; w < width; w++)
		{
			if(p_mat[h * width + w] == SEED)
			{
				((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 0] = gray_templ[0*18 + 0*3 + 0];
				((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 1] = gray_templ[0*18 + 0*3 + 1];
				((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 2] = gray_templ[0*18 + 0*3 + 2];
				continue;
			}
			else if(p_mat[h * width + w] == INIT)
			{
				((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 0] = gray_templ[0*18 + 2*3 + 0];
				((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 1] = gray_templ[0*18 + 2*3 + 1];
				((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 2] = gray_templ[0*18 + 2*3 + 2];
				continue;
			}
			else if(p_mat[h * width + w] == INVAL)
			{
				((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 0] = gray_templ[0*18 + 4*3 + 0];
				((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 1] = gray_templ[0*18 + 4*3 + 1];
				((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 2] = gray_templ[0*18 + 4*3 + 2];
				continue;
			}
			else 
			{
				for(nid = 1; nid <= id; nid += 2)
				{
					if( p_mat[h * width + w] == nid )
					{
						((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 0] = color_templ[(((nid+1)/2)%4)*18 + 0*3 + 0];
						((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 1] = color_templ[(((nid+1)/2)%4)*18 + 0*3 + 1];
						((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 2] = color_templ[(((nid+1)/2)%4)*18 + 0*3 + 2];
						break;
					}
					else if( p_mat[h * width + w] == nid + 1 )
					{
						((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 0] = color_templ[(((nid+1)/2)%4)*18 + 4*3 + 0];
						((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 1] = color_templ[(((nid+1)/2)%4)*18 + 4*3 + 1];
						((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 2] = color_templ[(((nid+1)/2)%4)*18 + 4*3 + 2];
						break;
					}
				}
			}
		}
	}
	cvNamedWindow("display1");
	cvShowImage("display1", disp);
	cvWaitKey(0);

	cvDestroyWindow("display1");
	cvReleaseImage(&disp);
}

bool render_mat(IplImage* matImg, int * p_mat, int width, int height, int id = 0)
{
    if(!matImg){
        return false;
    }
    if(matImg->width != width || matImg->height != height || matImg->depth != 8 || matImg->nChannels != 3)
        return false;

    int color_templ[4*6*3] = {
        0,0,121,	93,0,158,	91,20,237,	255,61,255,	170,110,240,	200,161,245,
        76,0,13,	113,52,0,	127,91,0,	188,114,0,	203,140,68,		236,211,190,
        38,88,0,	77,118,46,	66,130,98,	112,182,46,	118,197,124,	174,221,178,
        19,57,96,	10,98,163,	0,160,171,	0,242,255,	59,245,255,		139,247,255
    };
    /*int gray_templ[1*6*3] = {
        0,0,0, 40,40,40, 80,80,80, 120,120,120, 160,160,160, 200,200,200,
    };*/
    int gray_templ[1*6*3] = {
        255,255,255, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
    };

    // matImg = cvCreateImage( cvSize(width, height), 8, 3 );
    cvZero(matImg);

    int nid = 1;
    for(int h = 0; h < height; h++)
    {
        for(int w = 0; w < width; w++)
        {
            if(p_mat[h * width + w] == SEED)
            {
                ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 0] = gray_templ[0*18 + 0*3 + 0];
                ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 1] = gray_templ[0*18 + 0*3 + 1];
                ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 2] = gray_templ[0*18 + 0*3 + 2];
                continue;
            }
            else if(p_mat[h * width + w] == INIT)
            {
                ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 0] = gray_templ[0*18 + 2*3 + 0];
                ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 1] = gray_templ[0*18 + 2*3 + 1];
                ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 2] = gray_templ[0*18 + 2*3 + 2];
                continue;
            }
            else if(p_mat[h * width + w] == INVAL)
            {
                ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 0] = gray_templ[0*18 + 4*3 + 0];
                ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 1] = gray_templ[0*18 + 4*3 + 1];
                ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 2] = gray_templ[0*18 + 4*3 + 2];
                continue;
            }
            else 
            {
                for(nid = 1; nid <= id; nid += 2)
                {
                    if( p_mat[h * width + w] == nid )
                    {
                        ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 0] = color_templ[(((nid+1)/2)%4)*18 + 0*3 + 0];
                        ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 1] = color_templ[(((nid+1)/2)%4)*18 + 0*3 + 1];
                        ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 2] = color_templ[(((nid+1)/2)%4)*18 + 0*3 + 2];
                        break;
                    }
                    else if( p_mat[h * width + w] == nid + 1 )
                    {
                        ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 0] = color_templ[(((nid+1)/2)%4)*18 + 4*3 + 0];
                        ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 1] = color_templ[(((nid+1)/2)%4)*18 + 4*3 + 1];
                        ((UCHAR *)(matImg->imageData + h*matImg->widthStep))[w*3 + 2] = color_templ[(((nid+1)/2)%4)*18 + 4*3 + 2];
                        break;
                    }
                }
            }
        }
    }
    
    return true;
}

void regionLabling(int* p_mat, int width, int height, int x_g, int y_g, int id)  
{  
	//常用变量：  
	int x;  
	int y; 

	//设置前次边缘点  
	std::list<class g_point> cont_pts_pre;  
	std::list<class g_point> * pcont_pts_pre = &cont_pts_pre;


	//设置边缘点  
	std::list<class g_point> cont_pts;  
	std::list<class g_point> * pcont_pts = &cont_pts;


	//种子点+边缘点 = 下次的种子点  
	//上次的边缘点 = 前次边缘点（用于下次减少点数)   
	cont_pts_pre.push_back(class g_point(x_g, y_g, CONT));  

	p_mat[y_g * width + x_g] = id;

	std::list<class g_point>::iterator iter_cont;  
	std::list<class g_point>::iterator iter_prt;  
	std::list<class g_point>::iterator iter_swap;  

	// 	IplImage* disp;
	// 	disp = cvCreateImage(cvSize(width, height), 8, 3);
	// 	cvZero(disp);
	// 	cvNamedWindow("disp", -1);
	// 
	// 	for(int h = 0; h < height; h++)
	// 	{
	// 		for(int w = 0; w < width; w++)
	// 		{
	// 			switch(p_mat[h * width + w])
	// 			{
	// 			case INIT:
	// 				((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 0] = color_templ[0 * 18 + 0*3 + 0],
	// 					((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 1] = color_templ[0 * 18 + 0*3 + 1],
	// 					((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 2] = color_templ[0 * 18 + 0*3 + 2];
	// 				break;  
	// 			case SEED: ((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 0] = color_templ[0 * 18 + 2*3 + 0],
	// 						   ((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 1] = color_templ[0 * 18 + 2*3 + 1],
	// 						   ((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 2] = color_templ[0 * 18 + 2*3 + 2]; 
	// 				break; 
	// 			case INVAL: ((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 0] = color_templ[0 * 18 + 4*3 + 0],
	// 							((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 1] = color_templ[0 * 18 + 4*3 + 1],
	// 							((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 2] = color_templ[0 * 18 + 4*3 + 2]; 
	// 				break;
	// 			}
	// 		}
	// 	}
	// 	cvShowImage("disp", disp);
	// 	cvWaitKey(0);

	while( !cont_pts_pre.empty() )		//生长条件
	{
		//一轮生长 
		iter_cont = cont_pts_pre.begin();  
		while(iter_cont != cont_pts_pre.end())  
		{  
			x = (*iter_cont).x;  
			y = (*iter_cont).y;  
			//         if( !(x-1<0 || y-1<0) )                       //#1  
			//         {     
			//             if(p_mat[(y-1)*img->width + (x-1)] == INIT)  
			//             {  
			//                 gradient = ((char*)(img->imageData + y*img->widthStep))[x] -   
			//                     ((char*)(img->imageData + (y-1)*img->widthStep))[x-1];  
			//                 if(abs(gradient) < threshold_g)      //满足阈值要求  
			//                 {  
			//                     cont_pts.push_back(class g_point(x-1, y-1, CONT));  
			//                     p_mat[(y-1)*img->width + (x-1)] = SEED;  
			//                 }  
			//                 else                                //不满足阈值要求  
			//                 {  
			//                     p_mat[(y-1)*img->width + (x-1)] = INVAL;  
			//                 }  
			//             }  
			//         }  
			if( !(x-1<0) )                                   //#2  
			{  
				if(p_mat[(y)*width + (x-1)] == INIT )  
				{  

					cont_pts.push_back(class g_point(x-1, y, CONT));  
					p_mat[(y)*width + (x-1)] = id;   
				}  
				else if(p_mat[(y)*width + (x-1)] == INVAL)
				{  
					p_mat[(y)*width + (x-1)] = id+1;  
				} 
			}  
			//         if( !(x-1<0 || y+1 >= img->height) )           //#3  
			//         {  
			//             if(p_mat[(y+1)*img->width + (x-1)] == INIT)  
			//             {  
			//                 gradient = ((char*)(img->imageData + y*img->widthStep))[x] -   
			//                     ((char*)(img->imageData + (y+1)*img->widthStep))[x-1];  
			//                 if(abs(gradient) < threshold_g)      //满足阈值要求  
			//                 {  
			//                     cont_pts.push_back(class g_point(x-1, y+1, CONT));  
			//                     p_mat[(y+1)*img->width + (x-1)] = SEED;  
			//                 }  
			//                 else                                //不满足阈值要求  
			//                 {  
			//                     p_mat[(y+1)*img->width + (x-1)] = INVAL;  
			//                 }  
			//                   
			//             }  
			//         }  
			if( !(y+1 >= height) )                       //#4  
			{  
				if(p_mat[(y+1)*width + (x)] == INIT )  
				{  

					cont_pts.push_back(class g_point(x, y+1, CONT));  
					p_mat[(y+1)*width + (x)] = id;   
				}  
				else if( p_mat[(y+1)*width + (x)] == INVAL)
				{  
					p_mat[(y+1)*width + (x)] = id+1;  
				}  
			}  
			//         if( !(x+1>=img->width || y+1>=img->height) )    //#5  
			//         {  
			//             if(p_mat[(y+1)*img->width + (x+1)] == INIT)  
			//             {  
			//                 gradient = ((char*)(img->imageData + y*img->widthStep))[x] -   
			//                     ((char*)(img->imageData + (y+1)*img->widthStep))[x+1];  
			//                 if(abs(gradient) < threshold_g)      //满足阈值要求  
			//                 {  
			//                     cont_pts.push_back(class g_point(x+1, y+1, CONT));  
			//                     p_mat[(y+1)*img->width + (x+1)] = SEED;  
			//                 }  
			//                 else                                //不满足阈值要求  
			//                 {  
			//                     p_mat[(y+1)*img->width + (x+1)] = INVAL;  
			//                 }  
			//                   
			//             }  
			//         }  
			if( !(x+1>=width) )                      //#6  
			{  
				if(p_mat[(y)*width + (x+1)] == INIT )  
				{
					cont_pts.push_back(class g_point(x+1, y, CONT));  
					p_mat[(y)*width + (x+1)] = id;   
				}  
				else if( p_mat[(y)*width + (x+1)] == INVAL )
				{  
					p_mat[(y)*width + (x+1)] = id+1;  
				} 
			}  
			//         if( !(x+1>=img->width || y-1<0) )              //#7  
			//         {  
			//             if(p_mat[(y-1)*img->width + (x+1)] == INIT)  
			//             {  
			//                 gradient = ((char*)(img->imageData + y*img->widthStep))[x] -   
			//                     ((char*)(img->imageData + (y-1)*img->widthStep))[x+1];  
			//                 if(abs(gradient) < threshold_g)      //满足阈值要求  
			//                 {  
			//                     cont_pts.push_back(class g_point(x+1, y-1, CONT));  
			//                     p_mat[(y-1)*img->width + (x+1)] = SEED;  
			//                 }  
			//                 else                                //不满足阈值要求  
			//                 {  
			//                     p_mat[(y-1)*img->width + (x+1)] = INVAL;  
			//                 }  
			//                   
			//             }  
			//         }  
			if( !(y-1<0) )                                   //#8  
			{  
				if(p_mat[(y-1)*width + (x)] == INIT )  
				{ 
					cont_pts.push_back(class g_point(x, y-1, CONT));  
					p_mat[(y-1)*width + (x)] = id;   
				}  
				else if(p_mat[(y-1)*width + (x)] == INVAL)		//使用原有边缘
				{  
					p_mat[(y-1)*width + (x)] = id+1;  
				} 
			}  

			iter_cont ++;  
		}  

		//将cont_pts中的点赋给cont_pts_pre  
		cont_pts_pre.clear();
		iter_swap = cont_pts.begin();  
		while(iter_swap != cont_pts.end() )  
		{  
			cont_pts_pre.push_back((*iter_swap));  
			iter_swap ++;  
		}
		cont_pts.clear();

		// 		//显示环节
		// 		for(int h = 0; h < height; h++)
		// 		{
		// 			for(int w = 0; w < width; w++)
		// 			{
		// 				if(p_mat[h * width + w] == id)
		// 				{
		// 					((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 0] = color_templ[id * 18 + 0*3 + 0],
		// 						((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 1] = color_templ[id * 18 + 0*3 + 1],
		// 						((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 2] = color_templ[id * 18 + 0*3 + 2];
		// 				}
		// 				else if(p_mat[h * width + w] == id+1)
		// 				{
		// 					((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 0] = color_templ[id * 18 + 5*3 + 0],
		// 						((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 1] = color_templ[id * 18 + 5*3 + 1],
		// 						((UCHAR *)(disp->imageData + h*disp->widthStep))[w*3 + 2] = color_templ[id * 18 + 5*3 + 2];
		// 				}
		// 			}
		// 		}
		// 		cvShowImage("disp", disp);
		// 		cvWaitKey(0);
	}  
	//结束while  

	cont_pts_pre.clear(); 
	cont_pts.clear();
	// 	delete pseed_pts;
	// 	delete pcont_pts_pre;
	// 	delete pcont_pts;
} 

void regionGrowth_pro(IplImage* img, int * p_mat, int width, int height, int x, int y, int threshold, IplImage* matImg)
{
	int id = 1;
	//首先进行区域生长
	regionGrowth(img, p_mat, x, y, threshold);
    if(matImg){
        render_mat(matImg, p_mat, img->width, img->height);
    }else{
	    display_mat(p_mat, img->width, img->height);
    }

	/*for(int h = 0; h < height; h++)
	{
		for(int w = 0; w < width; w++)
		{
			if(p_mat[h * width + w] == INIT)
			{
				regionLabling(p_mat, width, height,w, h, id);
				id += 2;
			}
		}
	}

	//闭合区域数量
	id -= 2;		

	display_mat(p_mat, img->width, img->height, id);*/
}