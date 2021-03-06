#include "LP_Advanced.h"
#include <algorithm>
//#include <functional>
// 管理块的数据的类***********************************************************
LP_Block::LP_Block(void){
	name = "";
	basePoint.x = basePoint.y = basePoint.z = 0.0;
	type = LP_BLOCK;
}
LP_Block::LP_Block(LP_Point3 bpoint, string bname) {
	setBasePoint(bpoint);
	setName(bname);
	setFlags(-1);
	type = LP_BLOCK;
}
LP_Block::LP_Block(LP_Point3 bpoint, string bname, int bflags) {
	setBasePoint(bpoint);
	setName(bname);
	setFlags(bflags);
	type = LP_BLOCK;
}
void LP_Block::setBasePoint(const LP_Point3 bpoint){
	basePoint = bpoint;
}
void LP_Block::setName(const string bname) {
	name = bname;
}
void LP_Block::setFlags(const int bflags) {
	flags = bflags;
}
LP_Point3 LP_Block::getBasePoint() const {
	return basePoint;
}
string LP_Block::getName() const {
	return name;
}
const vector <LP_Lines>& LP_Block::getTemp() const
{
	return templines;
}
const vector<LP_Point3>& LP_Block::getHot() const
{
	return hotpoints;
}
int LP_Block::getFlags() const {
	return flags;
}
void LP_Block::addArc(LP_Ellipse arc){
	ellipses.push_back(arc);
}
void LP_Block::addEllipse(LP_Ellipse ellipse){
	ellipses.push_back(ellipse);
}
void LP_Block::addLine(LP_Line line){
	lines.push_back(line);
}
LP_Polyline& LP_Block::addPolyline(LP_Polyline polyline){
	polylines.push_back(polyline);
	return polylines.back();
}
LP_Spline& LP_Block::addSpline(LP_Spline spline){
	splines.push_back(spline);
	return splines.back();
}
void LP_Block::addPoint(LP_Point3 point){
	points.push_back(point);
}
void LP_Block::addInsert(LP_Insert ins){
	inserts.push_back(ins);
}
void LP_Block::addMText(LP_MText mt){
	mtexts.push_back(mt);
}
void LP_Block::setRect(){
	if(templines.empty() == false){
		rect = templines.front().rect;
		for(unsigned int i = 1; i < templines.size(); i ++){
        rect = rect + templines.at(i).rect;
	    }
	}
}
void LP_Block::Create(LP_Coordinate cod){  //该函数仅用于视图刷新用，若要全部重建图形请用void LP_Block::Create(const double smooth)
	if(templines.empty() == false){
		LP_Rect vrect = cod.getViewRect();
		unsigned int count = 0;
 		for(unsigned int k = 0; k < lines.size(); k++){
			count ++;
		//	templines.push_back(LP_Lines());
		//	templines.back().points.push_back(lines.at(k).point1);
		//	templines.back().points.push_back(lines.at(k).point2);
		//	templines.back().setAttributes(lines.at(k).getAttributes());
		}
		for(unsigned int k = 0; k < splines.size(); k++){
			if(splines.at(k).rect.isContactsRect(vrect)){
				splines.at(k).Create(cod.getSmooth());
				templines[count] = splines.at(k).templine;
			}
			count ++;
		}
		/*	for(unsigned int k = 0; k < blocks.size(); k++){
		blocks.at(k).Create(smooth);
		templines.insert(templines.end(),blocks.at(k).templines.begin(),blocks.at(k).templines.end());
		}*/
		for(unsigned int k = 0; k < inserts.size(); k++){
			if(inserts.at(k).rect.isContactsRect(vrect)){
				inserts.at(k).Create(cod);
				for(unsigned int m = 0; m < inserts.at(k).templines.size(); m ++){
					if(inserts.at(k).templines.at(m).rect.isContactsRect(vrect)){
						templines[count] = inserts.at(k).templines.at(m);
					}
					count ++;
				}
			}
		}
		for(unsigned int k = 0; k < ellipses.size(); k++){
			if(ellipses.at(k).rect.isContactsRect(vrect)){
			    ellipses.at(k).Create(cod.getSmooth());
			    templines[count] = ellipses.at(k).templine;
			}
			count ++;
		}
		for(unsigned int k = 0; k < polylines.size(); k++){
			if(ellipses.at(k).rect.isContactsRect(vrect)){
			    polylines.at(k).Create(cod.getSmooth());
			    templines[count] = polylines.at(k).templine;
			}
			count ++;
		}
		for(unsigned int i = 0; i < templines.size(); i ++){
			if(templines.at(i).rect.isContactsRect(vrect)){
				DL_Attributes temattrib = templines.at(i).getAttributes();
				if(temattrib.getColor() == 0){temattrib.setColor(Attributes.getColor());}
				if(temattrib.getLineType() == "BYBLOCK"){temattrib.setLineType(Attributes.getLineType());}
				if(temattrib.getWidth() == 0){temattrib.setWidth(Attributes.getWidth());}
				templines.at(i).setAttributes(temattrib);
			}
		}
	}
}
void LP_Block::Create(const double smooth){
	templines.clear();
	hotpoints.clear();
	unsigned int k = 0,gsize = 0;
	gsize = points.size();
	for(k = 0; k < gsize; k ++)         //将点压入缩冲区
	{
		templines.push_back(LP_Lines());
		templines.back().points.push_back(points.at(k));
	}
	gsize = lines.size();
	for(k = 0; k < gsize; k++){         //将线段压入缩冲区
		templines.push_back(LP_Lines());
		templines.back().points.push_back(lines.at(k).point1);
		templines.back().points.push_back(lines.at(k).point2);
		hotpoints.push_back(lines.at(k).point1);
		hotpoints.push_back(lines.at(k).point2);
		templines.back().setAttributes(lines.at(k).getAttributes());
	}
	gsize = splines.size();            //将样条曲线压入缓冲区
	for(k = 0; k < gsize; k++){
		splines.at(k).Create(smooth);
		templines.push_back(splines.at(k).templine);
		hotpoints.insert(hotpoints.end(),splines.at(k).hotpoints.begin(),splines.at(k).hotpoints.end());
	}
/*	for(unsigned int k = 0; k < blocks.size(); k++){
		blocks.at(k).Create(smooth);
		templines.insert(templines.end(),blocks.at(k).templines.begin(),blocks.at(k).templines.end());
	}*/
	gsize = inserts.size();
	for(k = 0; k < gsize; k++){        //将插入图形压入缓冲区
		inserts.at(k).Create(smooth);
		templines.insert(templines.end(),inserts.at(k).templines.begin(),inserts.at(k).templines.end());
		hotpoints.insert(hotpoints.end(),inserts.at(k).hotpoints.begin(),inserts.at(k).hotpoints.end());
	}
	gsize = ellipses.size();
	for(k = 0; k < gsize; k++){       //将椭圆（含圆及圆弧及椭圆弧）压入缓冲区
		ellipses.at(k).Create(smooth);
		templines.push_back(ellipses.at(k).templine);
		hotpoints.insert(hotpoints.end(),ellipses.at(k).hotpoints.begin(),ellipses.at(k).hotpoints.end());
	}
	gsize = polylines.size();         //将多段线压入缓冲区
	for(k = 0; k < gsize; k++){
		polylines.at(k).Create(smooth);
		templines.push_back(polylines.at(k).templine);
		hotpoints.insert(hotpoints.end(),polylines.at(k).hotpoints.begin(),polylines.at(k).hotpoints.end());
	}
	gsize = templines.size();         //设置缓冲区图形的属性
	LP_Point3 bp = -basePoint;
	for(k = 0; k < gsize; k ++){
		templines.at(k).move(bp);
		templines.at(k).setRect();
		DL_Attributes temattrib = templines.at(k).getAttributes();
		if(temattrib.getColor() == 0){temattrib.setColor(Attributes.getColor());}
		if(temattrib.getLineType() == "BYBLOCK"){temattrib.setLineType(Attributes.getLineType());}
		if(temattrib.getWidth() == 0){temattrib.setWidth(Attributes.getWidth());}
		templines.at(k).setAttributes(temattrib);
		templines.at(k).istemp = true;
	}
	gsize = hotpoints.size();
	for(k = 0; k < gsize; k ++)
	{
		hotpoints.at(k).move(bp);
	}
	sort(templines.begin(),templines.end());
	setRect();
}
void LP_Block::ReFresh(vector<LP_Block>& blks)
{
	for(unsigned int k = 0; k < inserts.size(); k ++)
	{
		if(inserts.at(k).name == name)  //如果当前块中包含的插入图形所引用的块是当前块，就会生成死循环，是不应该出现的，所以要删除它。
		{
			inserts.erase(inserts.begin() + k);
			k --;
		}
		else
		{
		    inserts.at(k).ReFresh(blks);
		}
	}
}

void LP_Block::Draw(wxDC * dc, LP_Coordinate &cod){
	cod.tempcolor = Attributes.getColor();
	if(templines.empty() == true){
		Create(cod.getSmooth());
	}
	wxPen oldpen = dc->GetPen();
	unsigned int m = 0;
	unsigned int i = 0;
	if(getPick())
	{
		dc->SetPen(cod.pickpen);
		dc->SetBrush(cod.nodebrush);
		if(cod.drawhot)
		{
			m = hotpoints.size();
			for(i = 0; i < m; i ++)
			{
				hotpoints[i].DrawPick(dc,cod);
			}
		}
	}

	m = templines.size();
    for(i = 0; i < m; i ++){
		templines.at(i).Draw(dc,cod);
	}
	dc->SetPen(oldpen);
}
void LP_Block::move(LP_Point3 &ref){
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k).move(ref);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).move(ref);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).move(ref);
	}
/*	for(unsigned int k = 0; k < blocks.size(); k++){
		blocks.at(k).move(ref);
	}*/
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).move(ref);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).move(ref);
	}
	rect.move(ref);
}
void LP_Block::rotate(double ang) {
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k).rotate(ang);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).rotate(ang);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).rotate(ang);
	}
/*	for(unsigned int k = 0; k < blocks.size(); k++){
		blocks.at(k).rotate(ang);
	}*/
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).rotate(ang);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).rotate(ang);
	}
	setRect();
}
//	* Rotates this vector around the given center by the given angle.
void LP_Block::rotate(LP_Point3 center, double ang) {
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k).rotate(center,ang);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).rotate(center,ang);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).rotate(center,ang);
	}
/*	for(unsigned int k = 0; k < blocks.size(); k++){
		blocks.at(k).rotate(center,ang);
	}*/
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).rotate(center,ang);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).rotate(center,ang);
	}
	setRect();
}
//	* Scales this vector by the given factors with 0/0 as center.
void LP_Block::scale(LP_Point3 factor) {
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k).scale(factor);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).scale(factor);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).scale(factor);
	}
/*	for(unsigned int k = 0; k < blocks.size(); k++){
		blocks.at(k).scale(factor);
	}*/
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).scale(factor);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).scale(factor);
	}
	setRect();
}
//	* Scales this vector by the given factors with the given center.
void LP_Block::scale(LP_Point3 center, LP_Point3 factor) {
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k).scale(center,factor);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).scale(center,factor);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).scale(center,factor);
	}
/*	for(unsigned int k = 0; k < blocks.size(); k++){
		blocks.at(k).scale(center,factor);
	}*/
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).scale(center,factor);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).scale(center,factor);
	}
	setRect();
}
//	* Mirrors this vector at the given axis.
void LP_Block::mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2) {
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k).mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).mirror(axisPoint1,axisPoint2);
	}
/*	for(unsigned int k = 0; k < blocks.size(); k++){
		blocks.at(k).mirror(axisPoint1,axisPoint2);
	}*/
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).mirror(axisPoint1,axisPoint2);
	}
	setRect();
}
void LP_Block::getTrack(LP_CAM_Group& buf)
{
	LP_CAM_Handle * temhdle;
	temhdle = new LP_CAM_Handle;
	if(temhdle == NULL)
	{
		string errormsg = "系统内存不足，不能分配足够内存.:void LP_Block::getTrack(LP_CAM_Group& buf)";
		CommandHandle(errormsg.c_str(),100);
		return;
	}
	for(unsigned int k = 0; k < templines.size(); k ++)
	{
		temhdle->push(&(templines.at(k)));
	}
	if(temhdle->empty() != true)
	{
		buf.push(temhdle);
	}
}
// 管理块的数据的类***********************************************************

LP_Insert::LP_Insert(const string& iName,
				   double iipx, double iipy, double iipz,
				   double isx, double isy, double isz,
				   double iAngle,
				   int iCols, int iRows,
				   double iColSp, double iRowSp) {
					   name = iName;
					   basePoint.x = iipx;
					   basePoint.y = iipy;
					   basePoint.z = iipz;
					   sx = isx;
					   sy = isy;
					   sz = isz;
					   angle = iAngle;
					   cols = iCols;
					   rows = iRows;
					   colSp = iColSp;
					   rowSp = iRowSp;
					   type = LP_INSERT;
}
LP_Insert::LP_Insert(const string& iName,
				   LP_Point3 point,
				   double isx, double isy, double isz,
				   double iAngle,
				   int iCols, int iRows,
				   double iColSp, double iRowSp) {
					   name = iName;
					   basePoint = point;
					   sx = isx;
					   sy = isy;
					   sz = isz;
					   angle = iAngle;
					   cols = iCols;
					   rows = iRows;
					   colSp = iColSp;
					   rowSp = iRowSp;
					   type = LP_INSERT;
}
void LP_Insert::setRect(){
	if(templines.empty() == false){
		rect = templines.front().rect;
		for(unsigned int i = 1; i < templines.size(); i ++){
            rect = rect + templines.at(i).rect;
	    }
	}
}
void LP_Insert::Create(LP_Coordinate cod){  //仅用于视图刷新时用，若不在显示矩形中则不重生成
	if(rect.isContactsRect(cod.getViewRect())){
		LP_Point3 tpoint = basePoint;
		iblock.Create(cod);
		templines.clear();
		hotpoints.clear();
		unsigned int k = iblock.getTemp().size();
		vector <LP_Lines> tempbln = iblock.getTemp();
		unsigned int p = 0;
		if(k > 0){
			for(unsigned int i = 0; i < k; i ++){
				tempbln.at(i).scale(sx,sy,sz);
				tempbln.at(i).rotate(angle*LP_PI/180.00);
			}
			for (int m = 1; m <= cols; m ++){//按行列生成块数组的数据
				tpoint.x = basePoint.x + colSp * m; //按当前列计算x的偏移值
				for(int n = 1; n <= rows; n++){
					tpoint.y = basePoint.y + rowSp * n;  //按当前行计算y的偏移值
					p = templines.size();
					templines.insert(templines.end(),tempbln.begin(),tempbln.end());//先向templines里加入一个iblock的副本
					for(unsigned int c = 0; c < k; c++){    //再将该副本偏移tpoint指定的位移
						templines.at(p+c).move(tpoint);
					}
				}
			}
		}
		setRect();
	}
}
void LP_Insert::Create(const double smooth){
	LP_Point3 tpoint = basePoint;
	iblock.Create(smooth);
	templines.clear();
	hotpoints.clear();
	unsigned int k = iblock.getTemp().size();
	vector <LP_Lines> tempbln = iblock.getTemp();
	unsigned int c = iblock.getHot().size();
	vector <LP_Point3> temphot = iblock.getHot();
	unsigned int p = 0;
	if(k > 0){
		for(unsigned int i = 0; i < k; i ++){
			tempbln[i].scale(sx,sy,sz);
			tempbln[i].rotate(angle*LP_PI/180.00);
		}
		for(unsigned int i = 0; i < c; i ++){
			temphot[i].scale(sx,sy,sz);
			temphot[i].rotate(angle*LP_PI/180.00);
		}
		for (int m = 1; m <= cols; m ++){//按行列生成块数组的数据
			tpoint.x = basePoint.x + colSp * m; //按当前列计算x的偏移值
			for(int n = 1; n <= rows; n++){
				tpoint.y = basePoint.y + rowSp * n;  //按当前行计算y的偏移
				p = templines.size();
				templines.insert(templines.end(),tempbln.begin(),tempbln.end());//先向templines里加入一个iblock的副本
				for(unsigned int ctl = 0; ctl < k; ctl++){    //再将该副本偏移tpoint指定的位移
					templines[p+ctl].move(tpoint);
				}
				p = hotpoints.size();
				hotpoints.insert(hotpoints.end(),temphot.begin(),temphot.end());  //向当前热点数组中插入一个引用块的热点数组副本
				for(unsigned int cth = 0; cth < c; cth ++)
				{
					hotpoints[p + cth].move(tpoint);
				}
			}
		}
	}
	setPick(getPick());  //刷新templines的选择状态
	setRect();
}
void LP_Insert::ReFresh(vector<LP_Block>& blks)
{
	for(unsigned int k = 0; k < blks.size(); k ++)
	{
		if(name == blks.at(k).getName())
		{
			if(blks.at(k).old == true)
			{
			    blks.at(k).ReFresh(blks);
			}
			iblock = blks.at(k);
		}
	}
}
void LP_Insert::setPick(bool flag){
	unsigned int ct = templines.size();
	for(unsigned int b = 0; b < ct; b ++)
	{
		templines[b].setPick(flag);
	}
	LP_Graph::setPick(flag);
}
bool LP_Insert::checkPick(LP_Point3 wp,LP_Coordinate &cod){
	unsigned int ct = templines.size();
	for(unsigned int c = 0; c < ct; c ++)
	{
		if(templines[c].checkPick(wp,cod))
		{
			return true;
		}
	}
	return false;
}
bool LP_Insert::rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod)
{
	unsigned int ct = templines.size();
	if(firstp.x >= secondp.x)
	{
		for(unsigned int c = 0; c < ct; c ++)
		{
			if(templines[c].rectPick(firstp,secondp,cod))
			{
				return true;
			}
		}
	}
	else
	{
		for(unsigned int c = 0; c < ct; c ++)
		{
			if(templines[c].rectPick(firstp,secondp,cod) == false)
			{
				return false;
			}
		}
		if(ct > 0)
		{
		    return true;
		}
	}
	return false;
}
void LP_Insert::Draw(wxDC * dc, LP_Coordinate &cod){
	cod.tempcolor = Attributes.getColor();
	if(iblock.getTemp().empty() == true){
		Create(cod.getSmooth());
	}
	wxPen oldpen = dc->GetPen();
	unsigned int m = 0;
	unsigned int i = 0;
	if(getPick())
	{
		dc->SetPen(cod.pickpen);
		dc->SetBrush(cod.nodebrush);
		if(cod.drawhot)
		{
			m = hotpoints.size();
			for(i = 0; i < m; i ++)
			{
				hotpoints[i].DrawPick(dc,cod);
			}
		}
	}

	m = templines.size();
    for(i = 0; i < m; i ++){
		templines.at(i).Draw(dc,cod);
	}
	dc->SetPen(oldpen);
}
void LP_Insert::move(LP_Point3 &ref){
	basePoint = basePoint + ref;
	unsigned int m = templines.size();
    for(unsigned int i = 0; i < m; i ++){
		templines.at(i).move(ref);
	}
}
void LP_Insert::rotate(double ang) {
    angle += ang;
}
//	* Rotates this vector around the given center by the given angle.
void LP_Insert::rotate(LP_Point3 center, double ang) {
	basePoint.rotate(center,ang);
	angle += ang;
}
//	* Scales this vector by the given factors with 0/0 as center.
void LP_Insert::scale(LP_Point3 factor) {
	sx *= factor.x;
	sy *= factor.y;
	sz *= factor.z;
}
//	* Scales this vector by the given factors with the given center.
void LP_Insert::scale(LP_Point3 center, LP_Point3 factor) {
    basePoint.scale(center, factor);
	scale(factor);
}
//	* Mirrors this vector at the given axis.
void LP_Insert::mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2) {       //@#$%@#$%^#&#%TRET%RWERTGDFYTWER%#$%@#$%@#$890$@#^%@^&^%*^&*(&&*^^&*%&%$^#%@#$@#$
    basePoint.mirror(axisPoint1,axisPoint2);
//	angle += 180;  *********************&&&&&&&&&&&&&&&&&&&&&&&&*&*&*&*&*&*&*&*&*&***&**#%#@#$@%$@%$#$%^$^$%^&$%^$^&$^%^&%镜像有问题。待解决
}
void LP_Insert::getTrack(LP_CAM_Group& buf)
{
	LP_CAM_Handle * temhdle;
	temhdle = new LP_CAM_Handle;
	if(temhdle == NULL)
	{
		string errormsg = "系统内存不足，不能分配足够内存.:void LP_Block::getTrack(LP_CAM_Group& buf)";
		CommandHandle(errormsg.c_str(),100);
		return;
	}
	for(unsigned int k = 0; k < templines.size(); k ++)
	{
		temhdle->push(&(templines.at(k)));
	}
	if(temhdle->empty() != true)
	{
		buf.push(temhdle);
	}
}
//图层的数据.*************************************所有图形对象都包含在图层里，每个图形对象的属性里也包含属于哪个图层的属性。当两个不一致时，以图形
//实际所属的图层为准；应将图形对象的图层属性更正为其所属图层。

LP_Layer::LP_Layer (){
	name = "";
	flags = 0;
	this->Attributes.setLayer("");
	this->Attributes.setColor(256);
	this->Attributes.setWidth(1);
	this->Attributes.setLineType("SOLID");
	this->setPick(false);
	isempty = true;
	type = LP_LAYER;
}
LP_Layer::LP_Layer(const string& lName,int lFlags) {
	LP_Layer();
	name = lName;
	flags = lFlags;
	type = LP_LAYER;
}
void LP_Layer::setRect(){
	bool flag = true;
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).setRect();
		if(flag && k == 0){
			flag = false;
			rect = lines.at(k).rect;
		}
		else{
			rect = rect + lines.at(k).rect;
		}
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).setRect();
		if(flag && k == 0){
			flag = false;
			rect = splines.at(k).rect;
		}
		else{
			rect = rect + splines.at(k).rect;
		}
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k).setRect();
		if(flag && k == 0){
			flag = false;
			rect = inserts.at(k).rect;
		}
		else{
			rect = rect + inserts.at(k).rect;
		}
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).setRect();
		if(flag && k == 0){
			flag = false;
			rect = ellipses.at(k).rect;
		}
		else{
			rect = rect + ellipses.at(k).rect;
		}
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).setRect();
		if(flag && k == 0){
			flag = false;
			rect = polylines.at(k).rect;
		}
		else{
			rect = rect + polylines.at(k).rect;
		}
	}
}
void LP_Layer::move(LP_Point3 &ref){
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k).move(ref);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).move(ref);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).move(ref);
	}
	/*for(unsigned int k = 0; k < blocks.size(); k++){
		blocks.at(k).move(ref);
	}*/
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k).move(ref);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).move(ref);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).move(ref);
	}
	rect.move(ref);
//	setRect();
}
void LP_Layer::Create(LP_Coordinate cod)  //将只刷新包围矩形与在cod中所定义的显示矩形有交叉的图形
{
	LP_Rect vrect = cod.getViewRect();
	DL_Attributes temattrib;
	for(unsigned int k = 0; k < splines.size(); k++){
		if(splines.at(k).rect.isContactsRect(vrect))
		{
			splines.at(k).Create(cod.getSmooth());
			temattrib = splines.at(k).templine.getAttributes();
			if(temattrib.getColor() == 256){
				temattrib.setColor(Attributes.getColor());
			}

			if(temattrib.getLineType() == "BYLAYER"){temattrib.setLineType(Attributes.getLineType());}
			if(temattrib.getWidth() == 0){temattrib.setWidth(Attributes.getWidth());}
			splines.at(k).templine.setAttributes(temattrib);
		}
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		if(inserts.at(k).rect.isContactsRect(vrect))
		{
			inserts.at(k).Create(cod.getSmooth());
			for(unsigned int i = 0; i < inserts.at(k).templines.size(); i ++){
				temattrib = inserts.at(k).templines.at(i).getAttributes();
				if(temattrib.getColor() == 256){
					temattrib.setColor(Attributes.getColor());
				}
				if(temattrib.getLineType() == "BYLAYER"){temattrib.setLineType(Attributes.getLineType());}
				if(temattrib.getWidth() == 0){temattrib.setWidth(Attributes.getWidth());}
				inserts.at(k).templines.at(i).setAttributes(temattrib);
			}
		}
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		if(ellipses.at(k).rect.isContactsRect(vrect))
		{
			ellipses.at(k).Create(cod.getSmooth());
			temattrib = ellipses.at(k).getAttributes();
			if(temattrib.getColor() == 256){temattrib.setColor(Attributes.getColor());}
			if(temattrib.getLineType() == "BYLAYER"){temattrib.setLineType(Attributes.getLineType());}
			if(temattrib.getWidth() == 0){temattrib.setWidth(Attributes.getWidth());}
			ellipses.at(k).templine.setAttributes(temattrib);
		}
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		if(polylines.at(k).rect.isContactsRect(vrect))
		{
			polylines.at(k).Create(cod.getSmooth());
			temattrib = polylines.at(k).getAttributes();
			if(temattrib.getColor() == 256){
				temattrib.setColor(Attributes.getColor());
			}
			if(temattrib.getLineType() == "BYLAYER"){temattrib.setLineType(Attributes.getLineType());}
			if(temattrib.getWidth() == 0){temattrib.setWidth(Attributes.getWidth());}
			polylines.at(k).templine.setAttributes(temattrib);
		}
	}
	setRect();
}
void LP_Layer::Create(const double smooth){
	DL_Attributes temattrib;
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).Create(smooth);
		temattrib = splines.at(k).templine.getAttributes();
		if(temattrib.getColor() == 256){
			temattrib.setColor(Attributes.getColor());
		}
		if(temattrib.getLineType() == "BYLAYER"){temattrib.setLineType(Attributes.getLineType());}
		if(temattrib.getWidth() == 0){temattrib.setWidth(Attributes.getWidth());}
		splines.at(k).templine.setAttributes(temattrib);
	}
/*	for(unsigned int k = 0; k < blocks.size(); k++){
		blocks.at(k).Create(smooth);
		for(unsigned int i = 0; i < blocks.at(k).templines.size(); i ++){
			temattrib = blocks.at(k).templines.at(i).getAttributes();
			if(temattrib.getColor() == 256){
				temattrib.setColor(Attributes.getColor());
			}
			if(temattrib.getLineType() == "BYLAYER"){temattrib.setLineType(Attributes.getLineType());}
			if(temattrib.getWidth() == 0){temattrib.setWidth(Attributes.getWidth());}
			blocks.at(k).templines.at(i).setAttributes(temattrib);
		}
	}*/
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k).Create(smooth);
		for(unsigned int i = 0; i < inserts.at(k).templines.size(); i ++){
			temattrib = inserts.at(k).templines.at(i).getAttributes();
			if(temattrib.getColor() == 256){
				temattrib.setColor(Attributes.getColor());
			}
			if(temattrib.getLineType() == "BYLAYER"){temattrib.setLineType(Attributes.getLineType());}
			if(temattrib.getWidth() == 0){temattrib.setWidth(Attributes.getWidth());}
			inserts.at(k).templines.at(i).setAttributes(temattrib);
		}
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).Create(smooth);
		temattrib = ellipses.at(k).getAttributes();
		if(temattrib.getColor() == 256){temattrib.setColor(Attributes.getColor());}
		if(temattrib.getLineType() == "BYLAYER"){temattrib.setLineType(Attributes.getLineType());}
		if(temattrib.getWidth() == 0){temattrib.setWidth(Attributes.getWidth());}
		ellipses.at(k).templine.setAttributes(temattrib);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).Create(smooth);
		temattrib = polylines.at(k).getAttributes();
		if(temattrib.getColor() == 256){
			temattrib.setColor(Attributes.getColor());
		}
		if(temattrib.getLineType() == "BYLAYER"){temattrib.setLineType(Attributes.getLineType());}
		if(temattrib.getWidth() == 0){temattrib.setWidth(Attributes.getWidth());}
		polylines.at(k).templine.setAttributes(temattrib);
	}
	setRect();
}
void LP_Layer::ReFresh(vector<LP_Block>& blks)
{
	for(unsigned int k = 0; k < inserts.size(); k ++)
	{
		if(inserts.at(k).name == name)  //如果当前块中包含的插入图形所引用的块是当前块，就会生成死循环，是不应该出现的，所以要删除它。
		{
			inserts.erase(inserts.begin() + k);
			k --;
		}
		else
		{
		    inserts.at(k).ReFresh(blks);
		}
	}
}
bool LP_Layer::rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod){
	bool rpflag = false;
	for(unsigned int k = 0; k < lines.size(); k++){
		if(lines.at(k).rectPick(firstp,secondp,cod)){
			if(lines.at(k).getPick() && cod.revsel == true)
			{
				lines.at(k).setPick(false);
			}
			else
			{
				lines.at(k).setPick(true);
				rpflag = true;
			}
		}
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		if(splines.at(k).rectPick(firstp,secondp,cod)){
			if(splines.at(k).getPick() && cod.revsel == true)
			{
				splines.at(k).setPick(false);
			}
			else
			{
				splines.at(k).setPick(true);
				rpflag = true;
			}
		}
	}
	for(unsigned int k = 0; k < mtexts.size(); k++){
		if(mtexts.at(k).rectPick(firstp,secondp,cod)){
			if(mtexts.at(k).getPick() && cod.revsel == true)
			{
				mtexts.at(k).setPick(false);
			}
			else
			{
				mtexts.at(k).setPick(true);
				rpflag = true;
			}
		}
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		if(inserts.at(k).rectPick(firstp,secondp,cod)){
			if(inserts.at(k).getPick() && cod.revsel == true)
			{
				inserts.at(k).setPick(false);
			}
			else
			{
				inserts.at(k).setPick(true);
				rpflag = true;
			}
		}
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		if(ellipses.at(k).rectPick(firstp,secondp,cod)){
			if(ellipses.at(k).getPick() && cod.revsel == true)
			{
				ellipses.at(k).setPick(false);
			}
			else
			{
				ellipses.at(k).setPick(true);
				rpflag = true;
			}
		}
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		if(polylines.at(k).rectPick(firstp,secondp,cod)){
			if(polylines.at(k).getPick() && cod.revsel == true)
			{
				polylines.at(k).setPick(false);
			}
			else
			{
				polylines.at(k).setPick(true);
				rpflag = true;
			}
		}
	}
	return rpflag;
}
bool LP_Layer::checkPick(LP_Point3 wp,LP_Coordinate &cod){
	for(unsigned int k = 0; k < lines.size(); k++){
		if(lines.at(k).checkPick(wp,cod)){
			if(lines.at(k).getPick() && cod.revsel == true)
			{
				lines.at(k).setPick(false);
			}
			else
			{
				lines.at(k).setPick(true);
			}
			return true;
		}
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		if(splines.at(k).checkPick(wp,cod)){
			if(splines.at(k).getPick() && cod.revsel == true)
			{
				splines.at(k).setPick(false);
			}
			else
			{
				splines.at(k).setPick(true);
			}
			return true;
		}
	}
/*	for(unsigned int k = 0; k < blocks.size(); k++){
		if(blocks.at(k).checkPick(wp,cod)){
			return true;
		}
	}*/
	for(unsigned int k = 0; k < mtexts.size(); k++){
		if(mtexts.at(k).checkPick(wp,cod)){
			if(mtexts.at(k).getPick() && cod.revsel == true)
			{
				mtexts.at(k).setPick(false);
			}
			else
			{
				mtexts.at(k).setPick(true);
			}
			return true;
		}
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		if(inserts.at(k).checkPick(wp,cod)){
			if(inserts.at(k).getPick() && cod.revsel == true)
			{
				inserts.at(k).setPick(false);
			}
			else
			{
				inserts.at(k).setPick(true);
			}
			return true;
		}
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		if(ellipses.at(k).checkPick(wp,cod)){
			if(ellipses.at(k).getPick() && cod.revsel == true)
			{
				ellipses.at(k).setPick(false);
			}
			else
			{
				ellipses.at(k).setPick(true);
			}
			return true;
		}
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		if(polylines.at(k).checkPick(wp,cod)){
			if(polylines.at(k).getPick() && cod.revsel == true)
			{
				polylines.at(k).setPick(false);
			}
			else
			{
				polylines.at(k).setPick(true);
			}
			return true;
		}
	}
	return false;
}
bool LP_Layer::getPick(LP_Selected &sel)
{
	bool pickflag = false;
	for(unsigned int k = 0; k < lines.size(); k++){
		if(lines.at(k).getPick())
		{
			sel.lines.push_back(&(lines[k]));
			pickflag = true;
		}
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		if(splines.at(k).getPick())
		{
			sel.splines.push_back(&(splines[k]));
			pickflag = true;
		}
	}
	for(unsigned int k = 0; k < mtexts.size(); k++){
		if(mtexts.at(k).getPick())
		{
			sel.mtexts.push_back(&(mtexts[k]));
			pickflag = true;
		}
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		if(inserts.at(k).getPick())
		{
			sel.inserts.push_back(&(inserts[k]));
			pickflag = true;
		}
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		if(ellipses.at(k).getPick())
		{
			sel.ellipses.push_back(&(ellipses[k]));
			pickflag = true;
		}
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		if(polylines.at(k).getPick())
		{
			sel.polylines.push_back(&(polylines[k]));
			pickflag = true;
		}
	}
	if(pickflag == true)
	{
		sel.layers.push_back(this);
	}
	return pickflag;
}
void LP_Layer::deSelectAll()
{
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).setPick(false);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).setPick(false);
	}
	for(unsigned int k = 0; k < mtexts.size(); k++){
		mtexts.at(k).setPick(false);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		if(inserts.at(k).getPick())
		{
			inserts.at(k).setPick(false);
		}
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).setPick(false);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).setPick(false);
	}
}
void LP_Layer::delSelected()
{
	for(unsigned int k = 0; k < lines.size(); k++){
		if(lines.at(k).getPick())
		{
			lines.erase(lines.begin()+k);
			k--;
		}
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		if(splines.at(k).getPick())
		{
			splines.erase(splines.begin()+k);
			k--;
		}
	}
	for(unsigned int k = 0; k < mtexts.size(); k++){
		if(mtexts.at(k).getPick())
		{
			mtexts.erase(mtexts.begin()+k);
			k--;
		}
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		if(inserts.at(k).getPick())
		{
			inserts.erase(inserts.begin()+k);
			k--;
		}
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		if(ellipses.at(k).getPick())
		{
			ellipses.erase(ellipses.begin()+k);
			k--;
		}
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		if(polylines.at(k).getPick())
		{
			polylines.erase(polylines.begin()+k);
			k--;
		}
	}
	for(unsigned int k = 0; k < points.size(); k++){
		if(points.at(k).getPick())
		{
			points.erase(points.begin()+k);
			k--;
		}
	}
}
bool LP_Layer::Empty()
{
	isempty = true;
	if(lines.empty() == false)
	{
		isempty = false;
		return isempty;
    }
	if(splines.empty() == false)
	{
		isempty = false;
		return isempty;
	}
/*    if(mtexts.empty() == false) //现不提供对文字的支持
	{
		isempty = false;
		return isempty;
	}*/
	if(inserts.empty() == false)
	{
		isempty = false;
		return isempty;
	}
	if(ellipses.empty() == false)
	{
		isempty = false;
		return isempty;
	}
	if(polylines.empty() == false)
	{
		isempty = false;
		return isempty;
	}
	if(points.empty() == false)
	{
		isempty = false;
		return isempty;
	}
	return isempty;
}
void LP_Layer::Draw(wxDC * dc, LP_Coordinate &cod){
	LP_Rect vrect = cod.getViewRect(); //屏幕显示区域的矩形
	int colorindex = Attributes.getColor();	wxPen pen(wxColor(dxfColors[colorindex][0] * 255,dxfColors[colorindex][1] * 255,dxfColors[colorindex][2] * 255));
	dc->SetPen(pen);
	for(unsigned int k = 0; k < points.size(); k++)
	{
		if(points.at(k).isInRect(vrect.LeftBottom,vrect.RightTop))
	    {
		    dc->DrawPoint(cod.toScreen(points.at(k)));
		}
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		if(lines.at(k).getPick() == false)
		    lines.at(k).Draw(dc, cod);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		if(splines.at(k).getPick() == false)
		    splines.at(k).Draw(dc, cod);
	}
	for(unsigned int k = 0; k < mtexts.size(); k++){
		if(mtexts.at(k).getPick() == false)
		    mtexts.at(k).Draw(dc,cod);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		if(inserts.at(k).getPick() == false)
		    inserts.at(k).Draw(dc, cod);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		if(ellipses.at(k).getPick() == false)
		    ellipses.at(k).Draw(dc, cod);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		if(polylines.at(k).getPick() == false)
		    polylines.at(k).Draw(dc, cod);
	}
	//*******************************************先画未被拾取的，再画被拾取的
	for(unsigned int k = 0; k < lines.size(); k++){
		if(lines.at(k).getPick())
		    lines.at(k).Draw(dc, cod);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		if(splines.at(k).getPick())
		    splines.at(k).Draw(dc, cod);
	}
	for(unsigned int k = 0; k < mtexts.size(); k++){
		if(mtexts.at(k).getPick())
		    mtexts.at(k).Draw(dc,cod);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		if(inserts.at(k).getPick())
		    inserts.at(k).Draw(dc, cod);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		if(ellipses.at(k).getPick())
		    ellipses.at(k).Draw(dc, cod);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		if(polylines.at(k).getPick())
		    polylines.at(k).Draw(dc, cod);
	}
}
void LP_Layer::rotate(double ang) {
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k).rotate(ang);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).rotate(ang);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).rotate(ang);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k).rotate(ang);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).rotate(ang);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).rotate(ang);
	}
	setRect();
}
//	* Rotates this vector around the given center by the given angle.
void LP_Layer::rotate(LP_Point3 center, double ang) {
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k).rotate(center,ang);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).rotate(center,ang);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).rotate(center,ang);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k).rotate(center,ang);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).rotate(center,ang);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).rotate(center,ang);
	}
	setRect();
}
//	* Scales this vector by the given factors with 0/0 as center.
void LP_Layer::scale(LP_Point3 factor) {
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k).scale(factor);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).scale(factor);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).scale(factor);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k).scale(factor);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).scale(factor);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).scale(factor);
	}
	setRect();
}
//	* Scales this vector by the given factors with the given center.
void LP_Layer::scale(LP_Point3 center, LP_Point3 factor) {
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k).scale(center,factor);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).scale(center,factor);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).scale(center,factor);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k).scale(center,factor);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).scale(center,factor);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).scale(center,factor);
	}
	setRect();
}
//	* Mirrors this vector at the given axis.
void LP_Layer::mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2) {
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k).mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k).mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k).mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k).mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k).mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k).mirror(axisPoint1,axisPoint2);
	}
	setRect();
}
void LP_Layer::getTrack(LP_CAM_Group& buf){
	LP_CAM_Handle * temhdle;
	temhdle = new LP_CAM_Handle;
	if(temhdle == NULL)
	{
		string errormsg = "系统内存不足，不能分配足够内存.:void LP_Block::getTrack(LP_CAM_Group& buf)";
		CommandHandle(errormsg.c_str(),100);
		return;
	}
	unsigned int k = 0,gsize = 0;
	gsize = lines.size();
	templines.clear();
	for(k = 0; k < gsize; k++){         //将线段压入缩冲区
		templines.push_back(LP_Lines());
		templines.back().points.push_back(lines.at(k).point1);
		templines.back().points.push_back(lines.at(k).point2);
		hotpoints.push_back(lines.at(k).point1);
		hotpoints.push_back(lines.at(k).point2);
		templines.back().setAttributes(lines.at(k).getAttributes());
		temhdle->push(&(templines.back()));
	}
	gsize = splines.size();            //将样条曲线压入缓冲区
	for(k = 0; k < gsize; k++){
		temhdle->push(&(splines.at(k).templine));
	}
//	for(unsigned int k = 0; k < blocks.size(); k++){
//		blocks.at(k).Create(smooth);
//		templines.insert(templines.end(),blocks.at(k).templines.begin(),blocks.at(k).templines.end());
//	}
	gsize = inserts.size();
	LP_CAM_Handle * instem;
	instem = new LP_CAM_Handle;
	if(instem == NULL)
	{
		string errormsg = "系统内存不足，不能分配足够内存.:void LP_Block::getTrack(LP_CAM_Group& buf)";
		CommandHandle(errormsg.c_str(),100);
		return;
	}
	for(k = 0; k < gsize; k++){        //将插入图形压入缓冲区
		inserts.at(k).getTrack(buf);
	}
	gsize = ellipses.size();
	for(k = 0; k < gsize; k++){       //将椭圆（含圆及圆弧及椭圆弧）压入缓冲区
		temhdle->push(&(ellipses.at(k).templine));
	}
	gsize = polylines.size();         //将多段线压入缓冲区
	for(k = 0; k < gsize; k++){
		temhdle->push(&(polylines.at(k).templine));
	}
	//sort(templines.begin(),templines.end());

	if(temhdle->empty() != true)
	{
		buf.push(temhdle);
	}
}
//********************************************************************************
LP_Selected::LP_Selected()
{
	isempty = false;
	type = LP_SELECTED;
}
bool LP_Selected::Empty()
{
	isempty = true;
	if(lines.empty() == false)
	{
		isempty = false;
		return isempty;
    }
	if(splines.empty() == false)
	{
		isempty = false;
		return isempty;
	}
/*    if(mtexts.empty() == false) //现不提供对文字的支持
	{
		isempty = false;
		return isempty;
	}*/
	if(inserts.empty() == false)
	{
		isempty = false;
		return isempty;
	}
	if(ellipses.empty() == false)
	{
		isempty = false;
		return isempty;
	}
	if(polylines.empty() == false)
	{
		isempty = false;
		return isempty;
	}
	return isempty;
}
void LP_Selected::clear()
{
	lines.clear();
	splines.clear();
	mtexts.clear();
	inserts.clear();
	ellipses.clear();
	polylines.clear();
	layers.clear();
	rect.RightTop = rect.LeftBottom;
}
bool LP_Selected::checkPick(LP_Point3 wp,LP_Coordinate &cod)
{
	for(unsigned int k = 0; k < lines.size(); k++){
		if(lines.at(k)->checkPick(wp,cod))
			return true;
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		if(splines.at(k)->checkPick(wp,cod))
			return true;
	}
	for(unsigned int k = 0; k < mtexts.size(); k++){
		if(mtexts.at(k)->checkPick(wp,cod))
			return true;
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		if(inserts.at(k)->checkPick(wp,cod))
			return true;
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		if(ellipses.at(k)->checkPick(wp,cod))
			return true;
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		if(polylines.at(k)->checkPick(wp,cod))
			return true;
	}
	return false;
}
void LP_Selected::Draw(wxDC * dc, LP_Coordinate &cod){
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k)->Draw(dc, cod);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k)->Draw(dc, cod);
	}
	for(unsigned int k = 0; k < mtexts.size(); k++){
		mtexts.at(k)->Draw(dc,cod);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k)->Draw(dc, cod);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k)->Draw(dc, cod);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k)->Draw(dc, cod);
	}
}
void LP_Selected::move(LP_Point3 &ref)  //以原点与ref为参考点移动所选图形
{
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k)->move(ref);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k)->move(ref);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k)->move(ref);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k)->move(ref);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k)->move(ref);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k)->move(ref);
	}
	rect.move(ref);
}
void LP_Selected::rotate(double ang)    //以0/0为中心旋转ang角度
{
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k)->rotate(ang);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k)->rotate(ang);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k)->rotate(ang);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k)->rotate(ang);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k)->rotate(ang);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k)->rotate(ang);
	}
	setRect();
}
void LP_Selected::rotate(LP_Point3 center, double ang) //以所给点为中心旋转ang角度
{
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k)->rotate(center,ang);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k)->rotate(center,ang);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k)->rotate(center,ang);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k)->rotate(center,ang);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k)->rotate(center,ang);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k)->rotate(center,ang);
	}
	setRect();
}
void LP_Selected::scale(LP_Point3 factor)  //以0/0为中心缩放factor指定的倍率
{
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k)->scale(factor);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k)->scale(factor);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k)->scale(factor);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k)->scale(factor);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k)->scale(factor);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k)->scale(factor);
	}
	rect.scale(factor);
}
void LP_Selected::scale(LP_Point3 center, LP_Point3 factor)  //以所给点为中心缩放factor指定的倍率
{
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k)->scale(center,factor);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k)->scale(center,factor);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k)->scale(center,factor);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k)->scale(center,factor);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k)->scale(center,factor);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k)->scale(center,factor);
	}
	rect.scale(center,factor);
}
void LP_Selected::mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2)  //以给定为轴镜像图形
{
	for(unsigned int k = 0; k < points.size(); k++){
		points.at(k)->mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k)->mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k)->mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k)->mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k)->mirror(axisPoint1,axisPoint2);
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k)->mirror(axisPoint1,axisPoint2);
	}
	setRect();
}
void LP_Selected::setRect()
{
	bool flag = true;
	for(unsigned int k = 0; k < lines.size(); k++){
		lines.at(k)->setRect();
		if(flag && k == 0){
			flag = false;
			rect = lines.at(k)->rect;
		}
		else{
			rect = rect + lines.at(k)->rect;
		}
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		splines.at(k)->setRect();
		if(flag && k == 0){
			flag = false;
			rect = splines.at(k)->rect;
		}
		else{
			rect = rect + splines.at(k)->rect;
		}
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		inserts.at(k)->setRect();
		if(flag && k == 0){
			flag = false;
			rect = inserts.at(k)->rect;
		}
		else{
			rect = rect + inserts.at(k)->rect;
		}
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		ellipses.at(k)->setRect();
		if(flag && k == 0){
			flag = false;
			rect = ellipses.at(k)->rect;
		}
		else{
			rect = rect + ellipses.at(k)->rect;
		}
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		polylines.at(k)->setRect();
		if(flag && k == 0){
			flag = false;
			rect = polylines.at(k)->rect;
		}
		else{
			rect = rect + polylines.at(k)->rect;
		}
	}
}
vector<int> & LP_Selected::getColors()
{
	vector<bool> cflag;
	vector <LP_Lines> * tline;
	for(int a = 0; a < 257; a ++)
	{
		cflag.push_back(false);
	}
	int tcolor = 0;
	for(unsigned int k = 0; k < lines.size(); k++){
		tcolor = lines.at(k)->getAttributes().getColor();
		cflag.at(tcolor) = true;
	}
	for(unsigned int k = 0; k < splines.size(); k++){
		tcolor = splines.at(k)->templine.getAttributes().getColor();
		cflag.at(tcolor) = true;
	}
	for(unsigned int k = 0; k < inserts.size(); k++){
		tline = &(inserts.at(k)->templines);
		unsigned int m = tline->size();
		for(unsigned int n = 0; n < m; n ++)
		{
			tcolor = tline->at(n).getAttributes().getColor();
			cflag.at(tcolor) = true;
		}
	}
	for(unsigned int k = 0; k < ellipses.size(); k++){
		tcolor = ellipses.at(k)->templine.getAttributes().getColor();
		cflag.at(tcolor) = true;
	}
	for(unsigned int k = 0; k < polylines.size(); k++){
		tcolor = polylines.at(k)->templine.getAttributes().getColor();
		cflag.at(tcolor) = true;
	}
	colors.clear();
	for(int a = 0; a < 257; a ++)
	{
        if(cflag.at(a) == true)
		{
			colors.push_back(a);
		}
	}
	return colors;
}
//****************************************************************************
vector <LP_Layer>& LP_Graphics_Container::getGraphics(){
	return layers;
}
LP_Rect LP_Graphics_Container::getRect(double smooth){   //如果smooth==0表示不重建图形
	bool fl = true;
	for(unsigned int i = 0; i<layers.size(); i++){
		if(layers.at(i).Empty() == false)
		{
			if(smooth == 0){
				layers.at(i).setRect();
			}
			else{
				layers.at(i).ReFresh(blocks);
				layers.at(i).Create(smooth);
				layers.at(i).setRect();
			}
			if(fl == true){                //fl表示第一个非空的层，当遇到第一个非空的层时，
				rect = layers.at(i).rect;  //将图形的包围框置为该层的包围框，以后遇到的非空层的的包围框将与图形的包围框相加。
			}
			else{
				rect = rect + layers.at(i).rect;
			}
			fl = false;
		}
	}
	return rect;
}
void LP_Graphics_Container::addLayer(LP_Layer &layer){
	layers.push_back(layer);
}
vector<LP_Layer>& LP_Graphics_Container::getLayers(){
    return layers;
}
void LP_Graphics_Container::Create(const double smooth){
	for(unsigned int k = 0; k < blocks.size(); k ++)
	{
		blocks.at(k).old = true;
	}
	for(unsigned int k = 0; k < layers.size(); k++){
		for(unsigned int m = 0; m < layers.at(k).inserts.size(); m++)
		{
			layers.at(k).inserts.at(m).ReFresh(blocks);
		}
		layers.at(k).Create(smooth);
	}
}
void LP_Graphics_Container::Create(LP_Coordinate cod){
	for(unsigned int k = 0; k<layers.size(); k++){
		layers.at(k).Create(cod);
	}
}
bool LP_Graphics_Container::rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod){
	bool pickflag = false;
    for(unsigned int k = 0; k < layers.size(); k ++){
		if(layers.at(k).rectPick(firstp,secondp,cod)){
			pickflag = true;
		}
	}
	return pickflag;
}
bool LP_Graphics_Container::checkPick(LP_Point3 wp,LP_Coordinate &cod){
	for(unsigned int k = 0; k < layers.size(); k ++){
		if(layers.at(k).checkPick(wp,cod)){
			return true;
		}
	}
	return false;
}
void LP_Graphics_Container::Draw(wxDC * dc, LP_Coordinate &cod){
	for(unsigned int k = 0; k<layers.size(); k++){
		layers.at(k).Draw(dc, cod);
	}
}
bool LP_Graphics_Container::setPick(bool sflag){
	bool oldselectstatus = IsPick;
	IsPick = sflag;
	return oldselectstatus;
}
void LP_Graphics_Container::deSelectAll()
{
	for(unsigned int k = 0; k < layers.size(); k ++)
	{
		layers.at(k).deSelectAll();
	}
}
void LP_Graphics_Container::delSelected()
{
	for(unsigned int k = 0; k < layers.size(); k ++)
	{
		layers.at(k).delSelected();
	}
}
void LP_Graphics_Container::getTrack(vector<LP_CAM_Group>& camgp)
{
	LP_CAM_Group temcg;
	for(unsigned int i = 0; i < layers.size(); i ++)
	{
		camgp.push_back(temcg);
		layers.at(i).getTrack(camgp.back());
	}
}
bool LP_Graphics_Container::getPick(LP_Selected &sel)
{
	bool ispickf = false;
	for(unsigned int k = 0; k<layers.size(); k++){
		if(layers.at(k).getPick(sel))
		{
			ispickf = true;
		}
	}
	if(ispickf == true)
	{
		sel.setRect();
	}
	return ispickf;
}