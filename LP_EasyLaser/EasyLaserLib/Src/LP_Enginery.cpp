#include <math.h>
#include "LP_Enginery.h"
#include "LP_Graph.h"
//#include <wx/dc.h>        // comment by ZWW
#define	MINSTEPSPACE 1.0e-6           //在单轴方向上的最小单步进给距离
#define MAXSTEPSPACE 1.0              //在单轴方向上的最大单步进给距离


void LP_EngineryBuffer::push(NC_Data& ncdata)
{
	data.push_back(ncdata);
}
void LP_CAM_Handle::push(LP_Graph *graph)
{
	graphics.push_back(graph);
}
int LP_CAM_Handle::getIndex()
{
	return index;
}
bool LP_CAM_Handle::empty()
{
	return graphics.empty();
}
void LP_CAM_Group::push(LP_CAM_Handle *handle)
{
	handles.push_back(handle);
}
LP_CAM_Handle* LP_CAM_Group::find(int index)
{
	for(unsigned int k = 0; k < handles.size(); k ++)
	{
		if(handles[k] ->getIndex() == index)
		{
			return handles[k];
		}
	}
	return NULL;
}