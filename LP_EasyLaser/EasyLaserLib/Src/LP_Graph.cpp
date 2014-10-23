#include "LP_Graph.h"
#include <math.h>
//****************************************************************************************
bool LP_MText::checkPick(LP_Point3 wp,LP_Coordinate &cod){
	return false;
}
bool LP_MText::rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod)
{
	return false;
}
void LP_MText::Draw(wxDC * dc, LP_Coordinate &cod){   //绘制文本的函数
//	int colorindex = Attributes.getColor();
//	wxPen pen(wxColor((unsigned char)dxfColors[colorindex][0] * 255,(unsigned char)dxfColors[colorindex][1] * 255,(unsigned char)dxfColors[colorindex][2] * 255));
//	wxBrush textBrush((wxColor((unsigned char)dxfColors[colorindex][0] * 255,(unsigned char)dxfColors[colorindex][1] * 255,(unsigned char)dxfColors[colorindex][2] * 255)),wxSOLID);
/*	StringFormat stringformat;
	stringformat.SetAlignment(StringAlignmentCenter); // StringAlignment
	CString fon(style.c_str());
	Font font(fon, (int)(height*cod.getScale()));
	SizeF size((float)(width*cod.getScale()),(float)(height*cod.getScale()));
	RectF rect1(cod.toScreen(inspoint), size);
	dc->DrawRectangle(&pen, rect1); //
    CString tex(text.c_str());
	bool turnmean = false;
	int c = tex.GetLength();
	for(int i = 0; i < c; i ++){
		if(tex[i] == 0x5C){   // 0x5C为"\",在CAD里为转义字符表示插入格式命令,其后跟用于设置文字格式的命令字符，若要输出"\"需在"\"后再跟"\"。即两个"\\"才能输出一个"\"
			if(turnmean == false){
			    tex.SetAt(i,0x0A);    // 0x0D为回车符号。0x0A为换行符号
				turnmean = true;
			}
			else{
				tex.Delete(i-1);
				c--;      //删除一个字符以后，字符总数要减1,计数值也要减1才能从上一字符处继续处理。
				i--;
				turnmean = false;
			}
		}
		else{
			turnmean = false;
		}
	}
    dc->DrawString(tex, tex.GetLength(), &font, cod.toScreen(inspoint), &textBrush);
	//dc->DrawString(tex, tex.GetLength(), &font, rect1, &stringformat, &textBrush);*/
}
//直线数据****************************************************************************
LP_Line::LP_Line()
{
	LP_Line(0,0,0,0,0,0);
}
LP_Line::LP_Line(LP_Point3 p1, LP_Point3 p2) {
	point1 = p1;
	point2 = p2;
	type = LP_LINE;
}
LP_Line::LP_Line(double p1x, double p1y, double p1z, double p2x, double p2y, double p2z) {
	point1.x = p1x;
	point1.y = p1y;
	point1.z = p1z;
	point1.valid = true;
	point2.x = p2x;
	point2.y = p2y;
	point2.z = p2z;
	point2.valid = true;
	type = LP_LINE;
}
void LP_Line::setRect(){
	rect.LeftBottom = point1;
	rect.RightTop = point1;
	rect.LeftBottom.minP(point2);
	rect.RightTop.maxP(point2);
	rect.LeftBottom.valid = true;
	rect.RightTop.valid = true;
}
bool LP_Line::checkPick(LP_Point3 wp,LP_Coordinate &cod){
	double x1 = point1.x;
	double y1 = point1.y;
	double x2 = point2.x;
	double y2 = point2.y;
	if(cod.getPickRect(wp).LineClip(x1,y1,x2,y2))
	{
		return true;
	}
	return false;
}
bool LP_Line::rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod)
{
	LP_Rect selrect;
	LP_Point3 fp = cod.toWorld(firstp.x,firstp.y);
	LP_Point3 sp = cod.toWorld(secondp.x,secondp.y);
	if(fp.x <= sp.x)
	{
		selrect.LeftBottom.x = fp.x;
		selrect.RightTop.x = sp.x;
	}
	else
	{
		selrect.LeftBottom.x = sp.x;
		selrect.RightTop.x = fp.x;
	}
	if(fp.y <= sp.y)
	{
		selrect.LeftBottom.y = fp.y;
		selrect.RightTop.y = sp.y;
	}
	else
	{
		selrect.LeftBottom.y = sp.y;
		selrect.RightTop.y = fp.y;
	}
	double x1,y1,x2,y2;
	x1 = point1.x;
	y1 = point1.y;
	x2 = point2.x;
	y2 = point2.y;
	if(fp.x <= sp.x)
	{
		if(point1.isInRect(selrect.LeftBottom,selrect.RightTop) && point2.isInRect(selrect.LeftBottom,selrect.RightTop))
		{
			return true;
		}
	}
	else
	{
		if(	selrect.LineClip(x1,y1,x2,y2))
		{
			return true;
		}
	}
	return false;
}
void LP_Line::Draw(wxDC * dc, LP_Coordinate &cod){
	LP_Rect vrect = cod.getViewRect(); //屏幕显示区域的矩形
	double x1,y1,x2,y2;
	x1 = point1.x;
	y1 = point1.y;
	x2 = point2.x;
	y2 = point2.y;
	bool tabf = vrect.LineClip(x1,y1,x2,y2);
	if(tabf){
		int colorindex = Attributes.getColor();
		//	if(colorindex == 7){colorindex = 0;}
		wxPen pen(wxColor(dxfColors[colorindex][0] * 255,dxfColors[colorindex][1] * 255,dxfColors[colorindex][2] * 255));
		dc->SetPen(pen);
		if(getPick())
		{
			dc->SetPen(cod.pickpen);
			dc->SetBrush(cod.nodebrush);
			if(cod.drawhot)
			{
			    point1.DrawPick(dc,cod);
			    point2.DrawPick(dc,cod);
			}
		}
        dc->DrawLine(cod.toScreen(x1,y1,0.0),cod.toScreen(x2,y2,0.0));
		//wxPoint ps = cod.toScreen(point1);
		//wxPoint pe = cod.toScreen(point2);
		//dc->DrawLine(ps, pe);
	}
}
void LP_Line::move(LP_Point3 &ref){
	point1.move(ref);
    point2.move(ref);
	setRect();
}
void LP_Line::rotate(double ang) {
	point1.rotate(ang);
	point2.rotate(ang);	
	setRect();
}
//	* Rotates this vector around the given center by the given angle.
void LP_Line::rotate(LP_Point3 center, double ang) {
	point1.rotate(center,ang);
	point2.rotate(center,ang);	
	setRect();
}
//	* Scales this vector by the given factors with 0/0 as center.
void LP_Line::scale(LP_Point3 factor) {
	point1.scale(factor);
	point2.scale(factor);
	setRect();
}
//	* Scales this vector by the given factors with the given center.
void LP_Line::scale(LP_Point3 center, LP_Point3 factor) {
	point1.scale(center,factor);
	point2.scale(center,factor);
	setRect();
}
//	* Mirrors this vector at the given axis.
void LP_Line::mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2) {
	point1.mirror(axisPoint1,axisPoint2);
	point2.mirror(axisPoint1,axisPoint2);
	setRect();
}
//多点直线数据****************************************************************************
LP_Lines::LP_Lines(){
	if(points.empty() == false){
		points.clear();
		istemp = false;
		type = LP_LINES;
	}
}
LP_Lines::LP_Lines(LP_Point3 p1, LP_Point3 p2) {
	points.clear();
	points.push_back(p1);
	points.push_back(p2);
	istemp = false;
	type = LP_LINES;
}
LP_Rect& LP_Lines::setRect(){
	if(points.empty() == false){
		rect.LeftBottom = points.front();
	    rect.RightTop = points.front();
	}
	for(unsigned int i = 0; i < points.size(); i++){
		rect.LeftBottom.minP(points.at(i));
	    rect.RightTop.maxP(points.at(i));
	}
	rect.LeftBottom.valid = true;
	rect.RightTop.valid = true;
	return this->rect;
}
bool LP_Lines::checkPick(LP_Point3 wp,LP_Coordinate &cod){
	LP_Rect prct = cod.getPickRect(wp);
	return rectPick(cod.toScreen(prct.RightTop),cod.toScreen(prct.LeftBottom),cod);
/*	unsigned int lsize = points.size();
	double x1,y1,x2,y2;
	for(unsigned int k = 1; k < lsize; k ++)
	{
		x1 = points[k-1].x;
		y1 = points[k-1].y;
		x2 = points[k].x;
		y2 = points[k].y;
		if(cod.getPickRect(wp).LineClip(x1,y1,x2,y2))
		{
			return true;
		}
	}
	return false;     */
}
bool LP_Lines::rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod)
{
	LP_Rect selrect;
	LP_Point3 fp = cod.toWorld(firstp.x,firstp.y);
	LP_Point3 sp = cod.toWorld(secondp.x,secondp.y);
	if(fp.x <= sp.x)
	{
		selrect.LeftBottom.x = fp.x;
		selrect.RightTop.x = sp.x;
	}
	else
	{
		selrect.LeftBottom.x = sp.x;
		selrect.RightTop.x = fp.x;
	}
	if(fp.y <= sp.y)
	{
		selrect.LeftBottom.y = fp.y;
		selrect.RightTop.y = sp.y;
	}
	else
	{
		selrect.LeftBottom.y = sp.y;
		selrect.RightTop.y = fp.y;
	}
	if(fp.x <= sp.x)
	{
		if(rect.isInRect(selrect))
		{
			return true;
		}
	}
	else
	{
		if(rect.isContactsRect(selrect))
		{
			unsigned int lsize = points.size();
			double x1,y1,x2,y2,prex,prey;
		    x1 = y1 = x2 = y2 = prex = prey = 0.0;
		    for(unsigned int i = 0; i < lsize; i++)
			{
			    if(i>=1)
			    {
				    x1 = prex;
				    y1 = prey;
				    x2 = points[i].x;
				    y2 = points[i].y;
				    if(selrect.LineClip(x1,y1,x2,y2))
				    {
					    return true;
				    }
				}
				prex = points[i].x;
			    prey = points[i].y;
			}
		}
	}
	return false;
}
void LP_Lines::Draw(wxDC * dc, LP_Coordinate &cod){
	LP_Rect vrect = cod.getViewRect(); //屏幕显示区域的矩形
	if(rect.isContactsRect(vrect)){
		int colorindex = Attributes.getColor();
		//	if(colorindex == 7){colorindex = 0;}
//		if(colorindex == 0 || colorindex == 256)
//		{
//			colorindex = cod.tempcolor;
//		}
		wxPen lpen(wxColour(dxfColors[colorindex][0] * 255,dxfColors[colorindex][1] * 255,dxfColors[colorindex][2] * 255),1,linetype);
		dc->SetPen(lpen);
		unsigned int lsize = points.size();
		if(getPick())                  //判断图形是否被拾取。若被拾取则绘制拾取点。并将画笔设置为拾取类型的画笔
		{
			dc->SetPen(cod.pickpen);
			dc->SetBrush(cod.nodebrush);
			if(cod.drawhot)
			{
				if(istemp == false){
					for(unsigned int c = 0; c < lsize; c++){
						points[c].DrawPick(dc,cod);
					}
				}
			}
		}

		wxPoint* pf = new wxPoint[lsize];
		unsigned int lcount = 0;
		double x1,y1,x2,y2,prex,prey;
		x1 = y1 = x2 = y2 = prex = prey = 0.0;
		for(unsigned int i = 0; i < lsize; i++){
			if(i>=1){
				x1 = prex;
				y1 = prey;
				x2 = points[i].x;
				y2 = points[i].y;
				if(vrect.LineClip(x1,y1,x2,y2)){
					if((prex == x1)&&(prey == y1)){
						if(i == 1){
							pf[lcount] = cod.toScreen(x1,y1,0.0);
							lcount ++;
						}
						pf[lcount] = cod.toScreen(x2,y2,0.0);
						if(abs(pf[lcount].x-pf[lcount-1].x)>=1 || abs(pf[lcount].y-pf[lcount-1].y)>=1){
							lcount ++;
						}
					}
					else{
						dc->DrawLines(lcount,pf);
						lcount = 0;
						pf[lcount] = cod.toScreen(x1,y1,0.0);
						lcount ++;
						pf[lcount] = cod.toScreen(x2,y2,0.0);
						if(abs(pf[lcount].x-pf[lcount-1].x)>=1 || abs(pf[lcount].y-pf[lcount-1].y)>=1){
							lcount ++;
						}
					}
				}
			}
			prex = points[i].x;
			prey = points[i].y;
		}		
		if(lsize == 1){
			dc->DrawPoint(cod.toScreen(points.at(0)));
		}
		else if(lsize >= 2){
			dc->DrawLines(lcount,pf);
		}
		delete[] pf;
		pf = NULL;
	}
}
void LP_Lines::move(LP_Point3 &ref){
	for(unsigned int i = 0; i < points.size(); i++){
		points.at(i).move(ref);
	}
	setRect();
}
void LP_Lines::rotate(double ang) {
	for(unsigned int i = 0; i < points.size(); i++){
		points.at(i).rotate(ang);
	}
	setRect();
}
//	* Rotates this vector around the given center by the given angle.
void LP_Lines::rotate(LP_Point3 center, double ang) {
	for(unsigned int i = 0; i < points.size(); i++){
		points.at(i).rotate(center,ang);
	}
	setRect();
}
//	* Scales this vector by the given factors with 0/0 as center.
void LP_Lines::scale(double xscale,double yscale,double zscale) {
	for(unsigned int i = 0; i < points.size(); i++){
		points.at(i).scale(xscale,yscale,zscale);
	}
	setRect();
}
void LP_Lines::scale(LP_Point3 center,double xscale,double yscale,double zscale) {
	for(unsigned int i = 0; i < points.size(); i++){
		points.at(i).scale(center,xscale,yscale,zscale);
	}
	setRect();
}
void LP_Lines::scale(LP_Point3 factor) {
	scale(factor.x,factor.y,factor.z);
}
//	* Scales this vector by the given factors with the given center.
void LP_Lines::scale(LP_Point3 center, LP_Point3 factor) {
	for(unsigned int i = 0; i < points.size(); i++){
		points.at(i).scale(center,factor);
	}
	setRect();
}
//	* Mirrors this vector at the given axis.
void LP_Lines::mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2) {
	for(unsigned int i = 0; i < points.size(); i++){
		points.at(i).mirror(axisPoint1,axisPoint2);
	}
	setRect();
}
bool LP_Lines::operator< (const LP_Lines& ls) const
{
	return Attributes.getColor() < ls.getAttributes().getColor();

}
//管理椭圆的类***********************************************************
LP_Ellipse::LP_Ellipse(){   //默认构造一个在原点，半径为0的圆
	center.x = center.y = center.z = 0.0;
	majorAxis = center;
	ratio = 1.0;
	angle1 = 0.0;
	angle2 = 2*LP_PI;
	reversed = false;
	type = LP_ELLIPSE;
}
LP_Ellipse::LP_Ellipse(LP_Point3 centerp, double radiuscircle) {  //此方法构建圆
	center = centerp;
	majorAxis.x = radiuscircle;
	majorAxis.y = majorAxis.z = 0.0;
	majorAxis.valid = true;
	ratio = 1.0;
	angle1 = 0.0;
	angle2 = 2*LP_PI;
	reversed = false;
	type = LP_ELLIPSE;
}
LP_Ellipse::LP_Ellipse(LP_Point3 centerp, double radiuscircle,double a1,double a2,bool reversedarc/*圆弧的正反向*/) {//此方法构建圆弧
	center = centerp;
	majorAxis.x = radiuscircle;
	majorAxis.y = 0;
	majorAxis.z = 0;
	majorAxis.valid = true;
	ratio = 1.0;
    angle1 = a1*LP_PI/180.00;
	angle2 = a2*LP_PI/180.00;
	reversed = reversedarc;
	type = LP_ELLIPSE;
}
LP_Ellipse::LP_Ellipse(LP_Point3 centerp, LP_Point3 majorp, double ratiox, double a1, double a2) {  //此方法构建椭圆弧
	center = centerp;
	majorAxis = majorp;
	majorAxis.valid = true;
	ratio = ratiox;
	angle1 = a1;
	angle2 = a2;
	reversed = false;
	type = LP_ELLIPSE;
}
LP_Ellipse::LP_Ellipse(LP_Point3 centerp, LP_Point3 majorp, double ratiox) {  //此方法构建椭圆
	center = centerp;
	majorAxis = majorp;
	ratio = ratiox;
	angle1 = 0.0;
	angle2 = 2*LP_PI;
	reversed = false;
	type = LP_ELLIPSE;
}
LP_Ellipse& LP_Ellipse::operator= (const LP_Ellipse& ellipse) {
	center = ellipse.center;
	majorAxis = ellipse.majorAxis;
	ratio = ellipse.ratio;
	angle1 = ellipse.angle1;
	angle2 = ellipse.angle2;
	setAttributes(ellipse.Attributes);
	reversed = ellipse.reversed;
	return *this;
}
void LP_Ellipse::Create(double smooth){
	templine.points.clear();       //把弧长切分成由smooth指定的份数。然后计算出每份的端点坐标，存入tempoint数组里
	hotpoints.clear();         //清空热点数组里的数据
//	hotpoints.push_back(center);
	if(smooth < 0.01){smooth = 0.01;}
	double angle_s = angle1;  //起始角度值
	double angle_e = angle2;  //终止角度值
	if(angle_s > 0.0){                                             //将两弧度转换成0 －－ 2PI之间的弧度值
		for(angle_s;  angle_s > 2*LP_PI; angle_s -=2*LP_PI){}
	}
	if(angle_s < 0.0){                                             //
		for(angle_s;  angle_s < 0; angle_s +=2*LP_PI){}
	}
	if(angle_e > 0.0){                                             //
		for(angle_e;  angle_e > 2*LP_PI; angle_e -=2*LP_PI){}
	}
	if(angle_e < 0.0){                                             //将两弧度转换成0 －－ 2PI之间的弧度值
		for(angle_e;  angle_e < 0; angle_e +=2*LP_PI){}
	}
	if(majorAxis.magnitude() != 0){
		if(smooth <= 0){smooth = 1;}
		double rototeAngle = majorAxis.angle();
		double angledec;
		double a = majorAxis.magnitude();
		double b = a * ratio;
		double L = 2*LP_PI*b+4*(a-b);      //椭圆的周长L等于2πb+4(a-b) 其中a为椭圆的长半轴长，b为椭圆的短半轴长
		angle_e = (angle_e > angle_s)? (angle_e):(angle_e + 2*LP_PI); //将角度转化成angle_e > angle_s的形式。      //**
		double angle = (reversed==true)? (2*LP_PI-(angle_e-angle_s)):(angle_e-angle_s);  //计算椭圆弧包围的弧度值  //**
		angledec = 2*LP_PI*smooth/L;  //计算弧度的步长                                                             //**
		if(angle/angledec > MAX_NODE){angledec = angle/MAX_NODE;}                                                  //**计算椭圆的切分数
		if(angledec > angle){angledec = angle / 2;}                                                                //**
		if(reversed == true){
			angle_s = (angle_s > angle_e)? (angle_s):(angle_s + 2*LP_PI);  //将角度转化成angle_s > angle_e的形式。
			if(2*LP_PI - angle_s + angle_e <= RS_TOLERANCE_ANGLE)
			{
				if(angle_s-2*LP_PI<=RS_TOLERANCE_ANGLE){///////////////////////////////////////////////////////////////////////////////////
					hotpoints.push_back(majorAxis+center);//判断该逆椭圆是否过四个象限点。将过的象限点加入热点数组
				}
				if(angle_s>=LP_PI/2 && angle_e<= LP_PI/2){
					hotpoints.push_back((majorAxis*ratio).rotate(LP_PI/2)+center);
				}
				if(angle_s>=LP_PI && angle_e<=LP_PI){
					hotpoints.push_back(-majorAxis+center);
				}
				if(angle_s>=LP_PI*3/2 && angle_e<=LP_PI*3/2){
					hotpoints.push_back((majorAxis*ratio).rotate(-LP_PI/2)+center);
				}///////////////////////////////////////////////////////////////////////////////////////////////////////
			}
			else
			{
				LP_Point3 peP(a * cos(angle_e),b * sin(angle_e),center.z);
			    hotpoints.push_back(peP.rotate(rototeAngle) + center);
			    peP.setD(a*cos(angle_s),b*sin(angle_s),center.z);
			    hotpoints.push_back(peP.rotate(rototeAngle)+center);
				double midang = angle_e+(angle_s - angle_e)/2;
				peP.setD(a*cos(midang),b*sin(midang),center.z);
				hotpoints.push_back(peP.rotate(rototeAngle)+center);
			}
			for(double i = angle_s; i>angle_e; i-=angledec){
				LP_Point3 tmP(a * cos(i),b * sin(i),center.z);
				templine.points.push_back(tmP.rotate(rototeAngle) + center);
			}
			LP_Point3 tmeP(a * cos(angle_e),b * sin(angle_e) ,center.z);
			templine.points.push_back(tmeP.rotate(rototeAngle) + center);
		}
		else{
			angle_e = (angle_e > angle_s)? (angle_e):(angle_e + 2*LP_PI); //将角度转化成angle_e > angle_s的形式。
			if(2*LP_PI - angle_e + angle_s <= RS_TOLERANCE_ANGLE)
			{
				if(angle_e-2*LP_PI<=RS_TOLERANCE_ANGLE){/////////////////////////////////////////////////////////////////////////////////
					hotpoints.push_back(majorAxis+center);//判断该正椭圆是否过四个象限点。将过的象限点加入热点数组
				}
				if(angle_s<=LP_PI/2 && angle_e>=LP_PI/2){
					hotpoints.push_back((majorAxis*ratio).rotate(LP_PI/2)+center);
				}
				if(angle_s<=LP_PI && angle_e>=LP_PI){
					hotpoints.push_back(-majorAxis+center);
				}
				if(angle_s<=LP_PI*3/2 && angle_e>=LP_PI*3/2){
					hotpoints.push_back((majorAxis*ratio).rotate(-LP_PI/2)+center);
				}///////////////////////////////////////////////////////////////////////////////////////////////////////
			}
			else
			{
				LP_Point3 peP(a * cos(angle_e),b * sin(angle_e),center.z);
				hotpoints.push_back(peP.rotate(rototeAngle) + center);
				peP.setD(a*cos(angle_s),b*sin(angle_s),center.z);
				hotpoints.push_back(peP.rotate(rototeAngle)+center);
				double midang = angle_s+(angle_e - angle_s)/2;
				peP.setD(a*cos(midang),b*sin(midang),center.z);
				hotpoints.push_back(peP.rotate(rototeAngle)+center);
			}
			for(double i = angle_s; i<angle_e; i+=angledec){
				LP_Point3 tmP(a * cos(i),b * sin(i),center.z);
				templine.points.push_back(tmP.rotate(rototeAngle) + center);
			}
			LP_Point3 tmeP(a * cos(angle_e),b * sin(angle_e),center.z);
			templine.points.push_back(tmeP.rotate(rototeAngle) + center);
		}
	}   //每个椭圆弧至少由起点至终点的一条直线组成
	else{
		templine.points.push_back(center);
	}
	templine.setAttributes(Attributes);
	templine.istemp = true;
}
void LP_Ellipse::setRect(){
	rect = templine.setRect();
}
void LP_Ellipse::setPick(bool flag){
	templine.setPick(flag);
	LP_Graph::setPick(flag);
}
bool LP_Ellipse::checkPick(LP_Point3 wp,LP_Coordinate &cod){
	if(templine.checkPick(wp,cod)){
		return true;
	}
	return false;
}
bool LP_Ellipse::rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod)
{
	if(templine.rectPick(firstp,secondp,cod))
	{
		return true;
	}
	return false;
}
void LP_Ellipse::Draw(wxDC * dc, LP_Coordinate &cod){
	if(templine.points.empty() == true){
		Create(cod.getSmooth());
	}
	if(getPick()){
		dc->SetPen(cod.pickpen);
		dc->SetBrush(cod.nodebrush);
		if(cod.drawhot)
		{
			unsigned int ch = hotpoints.size();
			for(unsigned int mh = 0; mh < ch; mh ++){
				hotpoints[mh].DrawPick(dc,cod);
			}
			if(ratio == 1)
			{
				wxPoint cp = cod.toScreen(center);
				dc->DrawCircle(cp.x,cp.y,cod.nodesize/2+1);
			}
		}
	}
	templine.Draw(dc,cod);
}
void LP_Ellipse::move(LP_Point3 &ref){
	center.move(ref);
	templine.move(ref);
	unsigned int ct = hotpoints.size();
	for(unsigned int k = 0; k < ct; k ++)
	{
		hotpoints[k].move(ref);
	}
	rect.move(ref);
}
void LP_Ellipse::rotate(double ang) {
	angle1 += ang;
	angle2 += ang;
	for(angle1; angle1 > 2*LP_PI; angle1 -= 2*LP_PI){}   //将angle1,angle2转换成0到360之间的弧度
	for(angle1; angle2 < 0.0; angle1 += 2*LP_PI){}   //
	for(angle2; angle2 > 2*LP_PI; angle2 -= 2*LP_PI){}  //
	for(angle2; angle2 < 0.0; angle2 += 2*LP_PI){}     //
	center.rotate(ang);
	majorAxis.rotate(ang);
	templine.rotate(ang);
	rect=templine.setRect();
}
//	* Rotates this vector around the given center by the given angle.
void LP_Ellipse::rotate(LP_Point3 cent, double ang) {
	angle1 += ang;
	angle2 += ang;
	for(angle1; angle1 > 2*LP_PI; angle1 -= 2*LP_PI){}   //将angle1,angle2转换成0到2*PI之间的弧度
	for(angle1; angle2 < 0.0; angle1 += 2*LP_PI){}   //
	for(angle2; angle2 > 2*LP_PI; angle2 -= 2*LP_PI){}  //
	for(angle2; angle2 < 0.0; angle2 += 2*LP_PI){}     //
	center.rotate(cent,ang);
	majorAxis.rotate(cent,ang);
	templine.rotate(cent,ang);
	rect=templine.setRect();
}
//	* Scales this vector by the given factors with 0/0 as center.
void LP_Ellipse::scale(LP_Point3 factor) {
	center.scale(factor);
	majorAxis.scale(factor);
	templine.scale(factor);
	rect.scale(factor);
}
//	* Scales this vector by the given factors with the given center.
void LP_Ellipse::scale(LP_Point3 cent, LP_Point3 factor) {
	center.scale(cent,factor);
	majorAxis.scale(cent,factor);
	templine.scale(cent,factor);
	rect.scale(cent,factor);
}
//	* Mirrors this vector at the given axis.
void LP_Ellipse::mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2) {
	center.mirror(axisPoint1,axisPoint2);
	majorAxis.mirror(axisPoint1,axisPoint2);
	angle1 = 2.0*(axisPoint1.angleTo(axisPoint2)) - angle1;
	angle2 = 2.0*(axisPoint1.angleTo(axisPoint2)) - angle2;
	for(angle1; angle1 > 2*LP_PI; angle1 -= 2*LP_PI){}   //将angle1,angle2转换成0到2*PI之间的弧度
	for(angle1; angle2 < 0.0; angle1 += 2*LP_PI){}   //
	for(angle2; angle2 > 2*LP_PI; angle2 -= 2*LP_PI){}  //
	for(angle2; angle2 < 0.0; angle2 += 2*LP_PI){}     //
	if(reversed == true){
		reversed = false;
	}else {
		reversed = true;
	}
	templine.mirror(axisPoint1,axisPoint2);
	rect=templine.setRect();
}
// 线型数据****************************************************************************

LP_LineType::LP_LineType(const string& lName,
						 int lFlags) {
							 name = lName;
							 flags = lFlags;
}

// 多段线数据*************************************************************************
/**
* Constructor.
* Parameters: see member variables.
*/
LP_Vertex::LP_Vertex(double px, double py, double pz, double pBulge) {
						 point.x = px;
						 point.y = py;
						 point.z = pz;
						 point.valid = true;
						 bulge = pBulge;
}
LP_Vertex& LP_Vertex::operator = (const LP_Vertex& vertex) {
	point = vertex.point;
	bulge = vertex.bulge;
	return *this;
}
void LP_Vertex::move(LP_Point3 &ref){
	point.move(ref);
}
void LP_Vertex::rotate(double ang) {
	point.rotate(ang);
}
//	* Rotates this vector around the given center by the given angle.
void LP_Vertex::rotate(LP_Point3 center, double ang) {
	point.rotate(center,ang);	
}
//	* Scales this vector by the given factors with 0/0 as center.
void LP_Vertex::scale(LP_Point3 factor) {
	point.scale(factor);	
	bulge *= factor.x;
}
//	* Scales this vector by the given factors with the given center.
void LP_Vertex::scale(LP_Point3 center, LP_Point3 factor) {
	point.scale(center,factor);	
	bulge *= factor.x;
}
//	* Mirrors this vector at the given axis.
void LP_Vertex::mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2) {
	point.mirror(axisPoint1,axisPoint2);
	bulge = -bulge;
}
//多段线成员函数************************************************************************
LP_Polyline::LP_Polyline(){
	LP_Polyline(0,0,0,0);
}
LP_Polyline::LP_Polyline(int pNumber, int pMVerteces, int pNVerteces, int pFlags) {
	number = pNumber;
	m = pMVerteces;
	n = pNVerteces;
	flags = pFlags;
	type = LP_POLYLINE;
}
void LP_Polyline::Create(double smooth){
	templine.points.clear();
	hotpoints.clear();
	templine.points.push_back(vertexs.front().point);
	hotpoints.push_back(vertexs.front().point);
	for(unsigned int i = 1; i < vertexs.size(); i++){
		if(fabs(vertexs.at(i-1).bulge) < RS_TOLERANCE){
			templine.points.push_back(vertexs.at(i).point);
			hotpoints.push_back(vertexs.at(i).point);
		}
		else
		{
			LP_Ellipse tarc = createArc(vertexs.at(i-1),vertexs.at(i));
			tarc.Create(smooth);
			templine.points.insert(templine.points.end(),tarc.templine.points.begin(),tarc.templine.points.end());
			hotpoints.insert(hotpoints.end(),tarc.hotpoints.begin(),tarc.hotpoints.end());
		}
	}
	if(flags % 2 == 1){
		templine.points.push_back(vertexs.front().point);
	}
	setRect();
	templine.setAttributes(Attributes);
	templine.istemp = true;
}
void LP_Polyline::setRect(){
	if(templine.points.empty() == true){Create(1);}
	rect = templine.setRect();
}
void LP_Polyline::setPick(bool flag){
	templine.setPick(flag);
	LP_Graph::setPick(flag);
}
bool LP_Polyline::checkPick(LP_Point3 wp,LP_Coordinate &cod){
	if(templine.checkPick(wp,cod))
	{
		return true;
	}
	return false;
}
bool LP_Polyline::rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod)
{
	if(templine.rectPick(firstp,secondp,cod))
	{
		return true;
	}
	return false;
}
void LP_Polyline::Draw(wxDC * dc, LP_Coordinate &cod){
	if(templine.points.empty() == true){
		Create(cod.getSmooth());
	}	
	if(getPick())
	{
		dc->SetPen(cod.pickpen);
		dc->SetBrush(cod.nodebrush);
		if(cod.drawhot)
		{
			unsigned int ct = hotpoints.size();
			for(unsigned int k = 0; k < ct; k ++)
			{
				hotpoints[k].DrawPick(dc,cod);
			}
		}
	}
	templine.Draw(dc,cod);
}
void LP_Polyline::move(LP_Point3 &ref){
	for(unsigned int i = 0; i < vertexs.size(); i++){
		vertexs.at(i).move(ref);
	}
	templine.move(ref);
	unsigned int ct = hotpoints.size();
	for(unsigned int k = 0; k < ct; k ++)
	{
		hotpoints[k].move(ref);
	}
	rect.move(ref);
}
void LP_Polyline::rotate(double ang) {
	for(unsigned int i = 0; i < vertexs.size(); i ++){
		vertexs.at(i).rotate(ang);
	}	
	templine.rotate(ang);
	rect = templine.setRect();
}
//	* Rotates this vector around the given center by the given angle.
void LP_Polyline::rotate(LP_Point3 center, double ang) {
	for(unsigned int i = 0; i < vertexs.size(); i ++){
		vertexs.at(i).rotate(center,ang);
	}	
	templine.rotate(center,ang);
	rect = templine.setRect();
}
//	* Scales this vector by the given factors with 0/0 as center.
void LP_Polyline::scale(LP_Point3 factor) {
    for(unsigned int i = 0; i < vertexs.size(); i ++){
		vertexs.at(i).scale(factor);
	}
	templine.scale(factor);
	rect.scale(factor);
}
//	* Scales this vector by the given factors with the given center.
void LP_Polyline::scale(LP_Point3 center, LP_Point3 factor) {
	for(unsigned int i = 0; i < vertexs.size(); i ++){
		vertexs.at(i).scale(center,factor);
	}		
	templine.scale(center,factor);
	rect.scale(center,factor);
}
//	* Mirrors this vector at the given axis.
void LP_Polyline::mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2) {
	for(unsigned int i = 0; i < vertexs.size(); i ++){
		vertexs.at(i).mirror(axisPoint1,axisPoint2);
	}
	templine.mirror(axisPoint1,axisPoint2);
	rect = templine.setRect();
}
LP_Ellipse LP_Polyline::createArc(LP_Vertex &startp, LP_Vertex &endp){
	// create lines for the polyline:
	bool reversed = (startp.bulge<0.0);
	double alpha = atan(startp.bulge)*4.0;

	double radius;
	LP_Point3 center;
	LP_Point3 middle;
	double dist;
	double angle;

	//		if (prepend==false) {
	middle = (startp.point +endp.point)/2.0;      //弦的中点
	dist = startp.point.distanceTo(endp.point)/2.0;     //半弦长
	angle = startp.point.angleTo(endp.point);           //终点相对起点的角度的弦度值
	//}
	// alpha can't be 0.0 at this point
	radius = fabs(dist / sin(alpha/2.0));              //半径

	double wu = fabs(radius * radius - dist * dist);   
	double h = sqrt(wu);                               //垂径的长度

	if (startp.bulge>0.0) {
		angle+=M_PI/2.0;
	} else {
		angle-=M_PI/2.0;
	}
	if (fabs(alpha)>M_PI) {
		h*=-1.0;
	}
	center.setPolar(h, angle);
	center+=middle;

	double a1;
	double a2;
	//		if (prepend==false) {
	a1 = center.angleTo(startp.point) * 180.0 / LP_PI;
	a2 = center.angleTo(endp.point) * 180.0 / LP_PI;
	//		}
	LP_Ellipse d(center, radius,a1, a2,reversed);
	return  d;
}

// 样条曲线数据**********************************************************************
LP_Spline::LP_Spline(){
	degree = 3;
	nKnots = 0;
	nControl = 0;
	flags = 0;
	closed = false;
	type = LP_SPLINE;
}
LP_Spline::LP_Spline(int pDegree, int pNKnots, int pNControl, int pFlags) {
	degree = pDegree;
	nKnots = pNKnots;
	nControl = pNControl;
	flags = pFlags;
	type = LP_SPLINE;
	if(flags == 11){closed = true;}else {closed = false;}
}
void LP_Spline::addKonts(const double kont) {
	konts.push_back(kont);
}
void LP_Spline::addControl(const LP_Point3 point) {
	controlPoints.push_back(point);
}
void LP_Spline::addKontsAt(int index, double knot) {
	konts.insert(konts.begin() + index, knot);
}
void LP_Spline::addControlAt(int index, LP_Point3 point) {
	controlPoints.insert(controlPoints.begin() + index, point);
}
void LP_Spline::removeKontsAt(int index) {
	konts.erase(konts.begin() + index);
}
void LP_Spline::removeControlAt(int index) {
	controlPoints.erase(controlPoints.begin() + index);
}
void LP_Spline::Create(const double smooth){        //生新生成绘制样条曲线的型值点坐标
	update(smooth);
	setRect();
}
/**
 * Updates the internal polygon of this spline. Called when the
 * spline or it's data, position, .. changes.
 */
void LP_Spline::update(double smooth) {
    if(smooth < 0.01){smooth = 0.01;}
    if (degree<1 || degree>3) {
		//Warning 样条次数超出范围，无法计算
        return;
    }
    if (controlPoints.size() < degree+1) {
        //Warning 没有足够的点来构建样条,控制点数到少为样条曲线的次数+1
        return;
    }
    vector <LP_Point3> tControlPoints = controlPoints;
    templine.points.clear();
    if (closed) {
        for (unsigned int i=0; i<degree; ++i) {
            tControlPoints.push_back(controlPoints[i]);
        }
    }
	double L = 0;
	for(unsigned int c = 1; c < tControlPoints.size(); c ++){
		L = L + tControlPoints.at(c).distanceTo(tControlPoints.at(c - 1));   //用控制多边形的边长和近似计算样条的长度值，该值必大于实际样条的长度值
	}
    unsigned int npts = tControlPoints.size();
    // order:
    int k = degree+1;
    // resolution:
	unsigned int p1 = /*getGraphicVariableInt("$SPLINESEGS", 8)8 * npts*/ (unsigned int)((L/smooth>MAX_INT)? MAX_INT:(L/smooth));
	if(p1 > MAX_NODE){p1 = MAX_NODE;}
	if(p1 < npts){p1 = npts;}
    double* b = new double[npts*3+1];
    double* h = new double[npts+1];
    double* p = new double[p1*3+1];
	vector <LP_Point3>::iterator it;
    unsigned int i = 1;
    for (it = tControlPoints.begin(); it!=tControlPoints.end(); ++it) {
        b[i] = (*it).x;
        b[i+1] = (*it).y;
        b[i+2] = 0.0;
        i+=3;
    }
    // set all homogeneous weighting factors to 1.0
    for (i=1; i <= npts; i++) {
        h[i] = 1.0;
    }
    for (i = 1; i <= 3*p1; i++) {
        p[i] = 0.0;
    }
    if (closed) {
        rbsplinu(npts,k,p1,b,h,p);
    } else {
        rbspline(npts,k,p1,b,h,p);
    }
    for (i = 1; i <= 3*p1; i=i+3) {
        templine.points.push_back(LP_Point3(p[i], p[i+1]));
  //    minV = RS_Vector::minimum(prev, minV);
  //    maxV = RS_Vector::maximum(prev, maxV);
    }
    delete[] b;
    delete[] h;
    delete[] p;
	templine.setAttributes(Attributes);
	templine.istemp = true;
	hotpoints.assign(controlPoints.begin(),controlPoints.end());
}
/**
 * Generates B-Spline open knot vector with multiplicity
 * equal to the order at the ends.
 */
void LP_Spline::knot(int num, int order, int knotVector[]) {
    knotVector[1] = 0;
    for (int i = 2; i <= num + order; i++) {
        if ( (i > order) && (i < num + 2) ) {
            knotVector[i] = knotVector[i-1] + 1;
        } else {
            knotVector[i] = knotVector[i-1];
        }
    }
}
/**
 * Generates rational B-spline basis functions for an open knot vector.
 */
void LP_Spline::rbasis(int c, double t, int npts,
                       int x[], double h[], double r[]) {
    int nplusc;
    int i,k;
    double d,e;
    double sum;
    //double temp[36];
    nplusc = npts + c;
    double* temp = new double[nplusc+1];
    // calculate the first order nonrational basis functions n[i]
    for (i = 1; i<= nplusc-1; i++) {
        if (( t >= x[i]) && (t < x[i+1]))
            temp[i] = 1;
        else
            temp[i] = 0;
    }
    /* calculate the higher order nonrational basis functions */
    for (k = 2; k <= c; k++) {
        for (i = 1; i <= nplusc-k; i++) {
            // if the lower order basis function is zero skip the calculation
            if (temp[i] != 0)
                d = ((t-x[i])*temp[i])/(x[i+k-1]-x[i]);
            else
                d = 0;
            // if the lower order basis function is zero skip the calculation
            if (temp[i+1] != 0)
                e = ((x[i+k]-t)*temp[i+1])/(x[i+k]-x[i+1]);
            else
                e = 0;
            temp[i] = d + e;
        }
    }
    // pick up last point
    if (t == (double)x[nplusc]) {
        temp[npts] = 1;
    }
    // calculate sum for denominator of rational basis functions
    sum = 0.;
    for (i = 1; i <= npts; i++) {
        sum = sum + temp[i]*h[i];
    }
    // form rational basis functions and put in r vector
    for (i = 1; i <= npts; i++) {
        if (sum != 0) {
            r[i] = (temp[i]*h[i])/(sum);
        } else
            r[i] = 0;
    }
    delete[] temp;
}
/**
 * Generates a rational B-spline curve using a uniform open knot vector.
 */
void LP_Spline::rbspline(int npts, int k, int p1,
                         double b[], double h[], double p[]) {
    int i,j,icount,jcount;
    int i1;
    //int x[30]; /* allows for 20 data points with basis function of order 5 */
    int nplusc;
    double step;
    double t;
    //double nbasis[20];
    double temp;
    nplusc = npts + k;
    int* x = new int[nplusc+1];
    double* nbasis = new double[npts+1];
    // zero and redimension the knot vector and the basis array
    for(i = 0; i <= npts; i++) {
        nbasis[i] = 0.0;
    }
    for(i = 0; i <= nplusc; i++) {
        x[i] = 0;
    }
    // generate the uniform open knot vector
    knot(npts,k,x);
    icount = 0;
    // calculate the points on the rational B-spline curve
    t = 0;
    step = ((double)x[nplusc])/((double)(p1-1));
    for (i1 = 1; i1<= p1; i1++) {
        if ((double)x[nplusc] - t < 5e-6) {
            t = (double)x[nplusc];
        }
        // generate the basis function for this value of t
        rbasis(k,t,npts,x,h,nbasis);
        // generate a point on the curve
        for (j = 1; j <= 3; j++) {
            jcount = j;
            p[icount+j] = 0.;
            // Do local matrix multiplication
            for (i = 1; i <= npts; i++) {
                temp = nbasis[i]*b[jcount];
                p[icount + j] = p[icount + j] + temp;
                jcount = jcount + 3;
            }
        }
        icount = icount + 3;
        t = t + step;
    }
    delete[] x;
    delete[] nbasis;
}


void LP_Spline::knotu(int num, int order, int knotVector[]) {
    int nplusc,nplus2,i;
    nplusc = num + order;
    nplus2 = num + 2;
    knotVector[1] = 0;
    for (i = 2; i <= nplusc; i++) {
        knotVector[i] = i-1;
    }
}
void LP_Spline::rbsplinu(int npts, int k, int p1,
                         double b[], double h[], double p[]) {
    int i,j,icount,jcount;
    int i1;
    //int x[30];		/* allows for 20 data points with basis function of order 5 */
    int nplusc;
    double step;
    double t;
    //double nbasis[20];
    double temp;
    nplusc = npts + k;
    int* x = new int[nplusc+1];
    double* nbasis = new double[npts+1];
    /*  zero and redimension the knot vector and the basis array */
    for(i = 0; i <= npts; i++) {
        nbasis[i] = 0.0;
    }
    for(i = 0; i <= nplusc; i++) {
        x[i] = 0;
    }
    /* generate the uniform periodic knot vector */
    knotu(npts,k,x);
    icount = 0;
    /*    calculate the points on the rational B-spline curve */
    t = k-1;
    step = ((double)((npts)-(k-1)))/((double)(p1-1));
    for (i1 = 1; i1<= p1; i1++) {
        if ((double)x[nplusc] - t < 5e-6) {
            t = (double)x[nplusc];
        }
        rbasis(k,t,npts,x,h,nbasis);      /* generate the basis function for this value of t */
        for (j = 1; j <= 3; j++) {      /* generate a point on the curve */
            jcount = j;
            p[icount+j] = 0.;
            for (i = 1; i <= npts; i++) { /* Do local matrix multiplication */
                temp = nbasis[i]*b[jcount];
                p[icount + j] = p[icount + j] + temp;
                jcount = jcount + 3;
            }
        }
        icount = icount + 3;
        t = t + step;
    }
    delete[] x;
    delete[] nbasis;
}
void LP_Spline::setRect(){
	rect = templine.setRect();
}
void LP_Spline::setPick(bool flag){
	templine.setPick(flag);
	LP_Graph::setPick(flag);
}
bool LP_Spline::checkPick(LP_Point3 wp,LP_Coordinate &cod){
	if(templine.checkPick(wp,cod))
	{
		return true;
	}
	return false;
}
bool LP_Spline::rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod)
{
	if(templine.rectPick(firstp,secondp,cod))
	{
		return true;
	}
	return false;
}
void LP_Spline::Draw(wxDC * dc, LP_Coordinate &cod){
	if(templine.points.empty() == true){
		Create(cod.getSmooth());
	}
	if(getPick())
	{
		dc->SetPen(cod.pickpen);
		dc->SetBrush(cod.nodebrush);
		if(cod.drawhot)
		{
			unsigned int ct = hotpoints.size();
			for(unsigned int k = 0; k < ct; k ++)
			{
				hotpoints[k].DrawPick(dc,cod);
			}
		}
	}
	templine.Draw(dc,cod);
}
void LP_Spline::move(LP_Point3 &ref){
	for(unsigned int i = 0; i < controlPoints.size(); i ++){
		controlPoints.at(i).move(ref);
	}
		templine.move(ref);
		rect.move(ref);
}
void LP_Spline::rotate(double ang) {
	for(unsigned int i = 0; i < controlPoints.size(); i ++){
		controlPoints.at(i).rotate(ang);
	}
		templine.rotate(ang);
		rect = templine.setRect();
}
//	* Rotates this vector around the given center by the given angle.
void LP_Spline::rotate(LP_Point3 center, double ang) {
	for(unsigned int i = 0; i < controlPoints.size(); i ++){
		controlPoints.at(i).rotate(center, ang);
	}
		templine.rotate(center,ang);
		rect = templine.setRect();	
}
//	* Scales this vector by the given factors with 0/0 as center.
void LP_Spline::scale(LP_Point3 factor) {
	for(unsigned int i = 0; i < controlPoints.size(); i ++){
		controlPoints.at(i).scale(factor);
	}
		templine.scale(factor);
		rect = templine.setRect();
}
//	* Scales this vector by the given factors with the given center.
void LP_Spline::scale(LP_Point3 center, LP_Point3 factor) {
	for(unsigned int i = 0; i < controlPoints.size(); i ++){
		controlPoints.at(i).scale(center, factor);
	}
		templine.scale(center,factor);
		rect = templine.setRect();	
}
//	* Mirrors this vector at the given axis.
LP_Spline& LP_Spline::mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2) {
	for(unsigned int i = 0; i < controlPoints.size(); i ++){
		controlPoints.at(i).mirror(axisPoint1,axisPoint2);
	}
	templine.mirror(axisPoint1,axisPoint2);
	rect = templine.setRect();
	return *this;
}