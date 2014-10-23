#ifndef LP_ENGINERY_H
#define LP_ENGINERY_H
#include "LP_Advanced.h"

class LP_Reference
{
public:
	int maxV;     //最大速度
	int maxA;  //最大加速度
	int jumpV;  //起跳速度
	double cornerDistance;  //拐角距离,值越小拐角越尖锐，值越大，拐角越圆滑
};
class LP_PointSpeed
{
public:
	double px;
	double py;
	double pz;
	double vx;
	double vy;
	double vz;
};
struct NC_Data  
{
	long	N;					          //代码段号
	unsigned char    Code;				        //曲线类型
	unsigned char    PlaneGroup;			    //圆弧平面
	double 	x,y,z,R,cx,cy;	    //X，Y，Z坐标，圆心，半径
	double  F;                  //用户目标速度
	double  v;					//速度预处理后得到的终点速度
};
class LP_EngineryBuffer         //保存加工数据的缓存类
{
public:
	void push(NC_Data& ncdata);
private:
	vector<NC_Data> data;
};
class LP_CAM_Handle           //一个Handle为同一颜色的一组图形
{
public:
	void push(LP_Graph * graph);
    inline	int getIndex();
	bool empty();
private:
	int index;               //加工顺序的索引。对应于颜色的索引
	LP_PointSpeed startP;             //起点的位置和所允许的最大速度。
	LP_PointSpeed endP;            //终点的位置和所允许的最大速度。
	vector<LP_Graph *> graphics;
};
class LP_CAM_Group        //一个Group为一个加工块，块中包含不同的颜色的图形，块作为一个加工单元，按其中图形的颜色顺次加工完毕后再进入下一个加工块。
{
public:
	void push(LP_CAM_Handle *handle);
	LP_CAM_Handle* find(int index);
private:
	vector<LP_CAM_Handle *> handles;
};
class LP_Enginery
{
	LP_Enginery(); 
};
#endif