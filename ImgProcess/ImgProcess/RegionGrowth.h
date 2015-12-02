/*
* This file is part of RegionGrowth program. This lib is V1.0.
*
* Copyright (C) 2012-0000 Mingran Tsagaan
* For informations write to mingran@live.com
* My blog is: http://blog.csdn.net/Sera_ph 
*             or http://www.mingran.co.cc
* 
* You can use my code and you are welcomed to discuss bugs as well
* as some new approaches with me.
* The following words are nonsense. I put them there in memorial of 
* someone.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*
*/

#include "stdafx.h"


#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

#include <vector>
#include <list>


#define INIT 0  
#define SEED -1  
#define CONT -2  
#define INVAL -3  


//定义数据类型
class g_point  
{  
public:  
	g_point();  
	g_point(int x_i, int y_i);  
	g_point(int x_i, int y_i, int lbl_i);  
public:  
	int x;  
	int y;  
	int lbl;      //标签  
};  

void regionGrowth(IplImage* img, int* p_mat, int x_g, int y_g, int threshold_g);
void display_mat(int * p_mat, int width, int height, int id);
bool render_mat(IplImage* matImg, int * p_mat, int width, int height, int id);
void regionLabling(int* p_mat, int width, int height, int x_g, int y_g, int id);
void regionGrowth_pro(IplImage* img, int * p_mat, int width, int height, int x, int y, int threshold, IplImage* matImg = NULL);
