#ifndef LP_DXFINTERFACE_H
#define LP_DXFINTERFACE_H

#include "..\dxflib\src\dl_creationadapter.h"
#include "..\dxflib\src\dl_dxf.h"
#include "LP_Advanced.h"
        //定义层的状态 flags
#define	FROZEN  1
#define FROZEN_BY_DEFAULT 2
#define LOCKED 4
        //结束层的状态定义
  //定义当前操作标志
#define ACTION_LAYER 1
#define	ACTION_BLOCK 2
#define ACTION_INSERT 3
#define ACTION_TRACE 4
#define ACTION_SOLID 5
#define ACTION_SPLINE 6
#define ACTION_LINE 7
#define ACTION_POLYLINE 8
#define ACTION_ATTRIBUTES 9
#define ACTION_LEADER 10
class LP_dxfInterface :
	public DL_CreationAdapter
{

public:
    LP_dxfInterface(LP_Graphics_Container& tgraph) {
		layers = &(tgraph.getGraphics());
		blocks = &(tgraph.blocks);
		currentspline = NULL;
		currentpolyline = NULL;
    }
     ~LP_dxfInterface() {
    }
public:
    /**
     * Called for every layer.
     */
	void addLayer(const DL_LayerData& data) {
		LP_Layer layer(data.name, data.flags);
		layer.setAttributes(this->attributes);
		layers->push_back(layer);
		actionstack.clear();
		actionstack.push_back(ACTION_LAYER);
	}
	void setLayer(string name){                    //在层列表中寻找指定名字的层，若找到就将其置为当前层。
		bool find = false;
		for(unsigned int i = 0; i < layers->size(); i++){
			if(layers->at(i).name.compare(name) == 0){
				currentlayer = &(layers->at(i));
				find = true;
				break;
			}
		}
		if(find == false){                        //找不到就添加以name指定的字符串为名字的层，并置为当前层。其属性为上一个实体的属性
			DL_LayerData data(name,0);
//			if(attributes.getColor() == 256){           //若颜色为随层，就更改为0
 //               attributes.setColor(0);  
//			}
			addLayer(data);
			currentlayer = &(layers->back());
		}
	}
	/**
	* Called for every block. Note: all entities added after this
	* command go into this block until endBlock() is called.
	*
	* @see endBlock()
	*/
	void addBlock(const DL_BlockData& data) {
		setLayer(this->attributes.getLayer());
		LP_Point3 dxpoint(data.bpx, data.bpy, data.bpz);
		LP_Block dxblock(dxpoint, data.name);
		dxblock.setAttributes(this->attributes);
		/*if(!currentblock.empty()){
			currentblock.back()->blocks.push_back(dxblock);
			currentblock.push_back(&(currentblock.back()->blocks.back()));
		}
		else{*/
			blocks->push_back(dxblock);
			currentblock.push_back(&(blocks->back()));
		//}
		actionstack.push_back(ACTION_BLOCK);
	}

	/** Called to end the current block */
	void endBlock() {
		if(currentblock.empty() == false){
			currentblock.pop_back();
		}
		if((!actionstack.empty()) && (actionstack.back() == ACTION_BLOCK)){
			actionstack.pop_back();
		}
	}

	/** Called for every point */
	void addPoint(const DL_PointData& data) {
		setLayer(this->attributes.getLayer());
		LP_Point3 dxpoint(data.x, data.y, data.z);
		if(actionstack.empty() == false){
			if(actionstack.back() == ACTION_BLOCK){
				if(currentblock.empty() == false)
					currentblock.back()->addPoint(dxpoint);
			}
			else{
				currentlayer->points.push_back(dxpoint);
			}
		}
		else{
			currentlayer->points.push_back(dxpoint);
		}
	}

	/** Called for every line */
	void addLine(const DL_LineData& data) {
		setLayer(this->attributes.getLayer());
		LP_Line dxline(data.x1, data.y1, data.z1, data.x2, data.y2, data.z2);
		dxline.setAttributes(currentlayer->getAttributes());
		if(actionstack.empty() == false){
			if(actionstack.back() == ACTION_BLOCK){
				if(currentblock.empty() == false){
					currentblock.back()->addLine(dxline);
				}///////////////////////////////////////////////////////////////
			}
			else if(actionstack.back() == ACTION_LAYER){
				currentlayer->lines.push_back(dxline);
			}
		}
		else{
			currentlayer->lines.push_back(dxline);
		}
	}

	/** Called for every arc */
	void addArc(const DL_ArcData& data) {
		setLayer(this->attributes.getLayer());
		LP_Point3 arccenter(data.cx,data.cy,data.cz);
		LP_Ellipse dxarc(arccenter, data.radius, data.angle1, data.angle2,false);
		dxarc.setAttributes(this->attributes);
		if(actionstack.empty() == false){
			if(actionstack.back() == ACTION_BLOCK){
				if(currentblock.empty() == false)
					currentblock.back()->addArc(dxarc);
			}
			else if(actionstack.back() == ACTION_LAYER){
				currentlayer->ellipses.push_back(dxarc);
			}
		}
		else{
			currentlayer->ellipses.push_back(dxarc);
		}
	}

	/** Called for every circle */
	void addCircle(const DL_CircleData& data) {
		setLayer(this->attributes.getLayer());
		LP_Point3 lp3(data.cx,data.cy,data.cz);
		LP_Ellipse dxcircle(lp3, data.radius);
		dxcircle.setAttributes(this->attributes);
		if(actionstack.empty() == false){
			if(actionstack.back() == ACTION_BLOCK){
				if(currentblock.empty() == false)
					currentblock.back()->addEllipse(dxcircle);
			}
			else if(actionstack.back() == ACTION_LAYER){
				currentlayer->ellipses.push_back(dxcircle);
			}
		}
		else{
			currentlayer->ellipses.push_back(dxcircle);
		}
	}

	/** Called for every ellipse */
	void addEllipse(const DL_EllipseData& data) {
		setLayer(this->attributes.getLayer());
		LP_Point3 dxcenter(data.cx, data.cy, data.cz);
		LP_Point3 dxmx(data.mx, data.my, data.mz);
		LP_Ellipse dxellipse(dxcenter, dxmx, data.ratio, data.angle1, data.angle2);
		dxellipse.setAttributes(this->attributes);
		if(actionstack.empty() == false){
			if(actionstack.back() == ACTION_BLOCK){
				if(currentblock.empty() == false)
					currentblock.back()->addEllipse(dxellipse);
			}
			else if(actionstack.back() == ACTION_LAYER){
				currentlayer->ellipses.push_back(dxellipse);
			}
		}
		else{
			currentlayer->ellipses.push_back(dxellipse);
		}
	}

	/** Called for every polyline start */
	void addPolyline(const DL_PolylineData& data) {
		setLayer(this->attributes.getLayer());          //调用setLayer后，若当前层为空，则图形有错
		LP_Polyline dxpolyline(data.number, data.m, data.n, data.flags);
		dxpolyline.setAttributes(this->attributes);
		if(actionstack.empty() == false){
			if(actionstack.back() == ACTION_BLOCK){
				if(currentblock.empty() == false){
					currentpolyline = &(currentblock.back()->addPolyline(dxpolyline));
				}
			}
			else if(currentlayer != NULL){
					currentlayer->polylines.push_back(dxpolyline);
					currentpolyline = &(currentlayer->polylines.back());
			}
		}
		else if(currentlayer != NULL){
			currentlayer->polylines.push_back(dxpolyline);
			currentpolyline = &(currentlayer->polylines.back());
		}
//		actionstack.push_back(ACTION_POLYLINE);
	}

	/** Called for every polyline vertex */
	void addVertex(const DL_VertexData& data) {
		LP_Vertex dxvertex(data.x, data.y, data.z, data.bulge);
		if(currentpolyline != NULL){
			currentpolyline->vertexs.push_back(dxvertex);
		}
	}

	/** Called for every spline */
	void addSpline(const DL_SplineData& data) {
		setLayer(this->attributes.getLayer());
		LP_Spline dxspline(data.degree, data.nKnots, data.nControl, data.flags);
		dxspline.setAttributes(this->attributes);
		if(actionstack.empty() == false){
			if(actionstack.back() == ACTION_BLOCK){
				if(currentblock.empty() == false){
					currentspline = &(currentblock.back()->addSpline(dxspline));
				}
			}
			else if(currentlayer != NULL){
				currentlayer->splines.push_back(dxspline);
				currentspline = &(currentlayer->splines.back());
			}
		}
		else if(currentlayer != NULL){
			currentlayer->splines.push_back(dxspline);
			currentspline = &(currentlayer->splines.back());
		}
	}

	/** Called for every spline control point */
	void addControlPoint(const DL_ControlPointData& data) {
		LP_Point3 dxcpoint(data.x, data.y, data.z);
		if(currentspline != NULL){
			currentspline->addControl(dxcpoint);
	//		currentspline->nControl ++;
		}
	}

	/** Called for every spline knot value */
	void addKnot(const DL_KnotData& data){ 
		if(currentspline != NULL){
			currentspline->addKonts(data.k);
	//		currentspline->nKnots ++;
		}
	}

	/** Called for every insert. */
	void addInsert(const DL_InsertData& data) {
		setLayer(this->attributes.getLayer());
		LP_Insert dxins(data.name, data.ipx, data.ipy, data.ipz, data.sx, data.sy, data.sz, data.angle,
			data.cols,data.rows,data.colSp, data.rowSp);
		dxins.setAttributes(this->attributes);
		for(unsigned int k = 0; k < blocks->size(); k ++){
			if(blocks->at(k).getName() == data.name){
					dxins.iblock = blocks->at(k);
					break;
				}
		}
		if(actionstack.empty() == false){
			if(actionstack.back() == ACTION_BLOCK){
				if(currentblock.empty() == false){
					currentblock.back()->addInsert(dxins);
				}
			}
			else if(currentlayer != NULL){
				currentlayer->inserts.push_back(dxins);
			}
		}
		else if(currentlayer != NULL){
			currentlayer->inserts.push_back(dxins);
		}
	}    
    /** Called for every trace start */
 //    void addTrace(const DL_TraceData& data) { }

    /** Called for every solid start */
 //    void addSolid(const DL_SolidData& data) { }


    /** Called for every Multi Text entity. */
    void addMText(const DL_MTextData& data) {
	setLayer(this->attributes.getLayer());
		LP_MText dxmtext(data.ipx, data.ipy, data.ipz,
                 data.height, data.width,
                 data.attachmentPoint,
                 data.drawingDirection,
                 data.lineSpacingStyle,
                 data.lineSpacingFactor,
                 data.text,
                 data.style,
                 data.angle);
		dxmtext.setAttributes(this->attributes);
		if(actionstack.empty() == false){
			if(actionstack.back() == ACTION_BLOCK){
				if(currentblock.empty() == false){
					currentblock.back()->addMText(dxmtext);
				}
			}
			else if(currentlayer != NULL){
				currentlayer->mtexts.push_back(dxmtext);
			}
		}
		else if(currentlayer != NULL){
			currentlayer->mtexts.push_back(dxmtext);
		}
	}

    /**
     * Called for additional text chunks for MTEXT entities.
     * The chunks come at 250 character in size each. Note that 
     * those chunks come <b>before</b> the actual MTEXT entity.
     */
 //    void addMTextChunk(const char* text) { }

    /** Called for every Text entity. */
//     void addText(const DL_TextData& data) { }

    /**
     * Called for every aligned dimension entity. 
     */
 //    void addDimAlign(const DL_DimensionData& data,
 //                            const DL_DimAlignedData& edata) { }
    /**
     * Called for every linear or rotated dimension entity. 
     */
 //    void addDimLinear(const DL_DimensionData& data,
 //                             const DL_DimLinearData& edata) { }

	/**
     * Called for every radial dimension entity. 
     */
//     void addDimRadial(const DL_DimensionData& data,
//                              const DL_DimRadialData& edata) { }

	/**
     * Called for every diametric dimension entity. 
     */
 //    void addDimDiametric(const DL_DimensionData& data,
//                              const DL_DimDiametricData& edata) { }

	/**
     * Called for every angular dimension (2 lines version) entity. 
     */
//     void addDimAngular(const DL_DimensionData& data,
//                              const DL_DimAngularData& edata) { }

	/**
     * Called for every angular dimension (3 points version) entity. 
     */
 //    void addDimAngular3P(const DL_DimensionData& data,
 //                             const DL_DimAngular3PData& edata) { }
    
    /** 
	 * Called for every leader start. 
	 */
//     void addLeader(const DL_LeaderData& data) { }
	
	/** 
	 * Called for every leader vertex 
	 */
//     void addLeaderVertex(const DL_LeaderVertexData& data) { }
	
	/** 
	 * Called for every hatch entity. 
	 */
//     void addHatch(const DL_HatchData& data) { }
	
	/** 
	 * Called for every image entity. 
	 */
//     void addImage(const DL_ImageData& data) { }

	/**
	 * Called for every image definition.
	 */
//	 void linkImage(const DL_ImageDefData& data) { }

	/** 
	 * Called for every hatch loop. 
	 */
//     void addHatchLoop(const DL_HatchLoopData& data) { }

	/** 
	 * Called for every hatch edge entity. 
	 */
 //    void addHatchEdge(const DL_HatchEdgeData& data) { }
	
	/** 
	 * Called after an entity has been completed.  
	 */
/*     void endEntity() {
		 if(actionstack.empty() == false){
			actionstack.pop_back();
			}
	 }
*/
    /**
     * Called for every vector variable in the DXF file (e.g. "$EXTMIN").
     */
//     void setVariableVector(const char* key, 
//	               double v1, double v2, double v3, int code) { }
	
    /**
     * Called for every string variable in the DXF file (e.g. "$ACADVER").
     */
//     void setVariableString(const char* key, const char* value, int code) { }
	
    /**
     * Called for every int variable in the DXF file (e.g. "$ACADMAINTVER").
     */
//     void setVariableInt(const char* key, int value, int code) { }
	
    /**
     * Called for every double variable in the DXF file (e.g. "$DIMEXO").
     */
//     void setVariableDouble(const char* key, double value, int code) { }
	
     /**
      * Called when a SEQEND occurs (when a POLYLINE or ATTRIB is done)
      */
      void endSequence() {
		  if(actionstack.back() == ACTION_POLYLINE || actionstack.back() == ACTION_ATTRIBUTES)
		  {
			  actionstack.pop_back();
		  }
	  }

    /** Sets the current attributes for entities. */
    void setAttributes(const DL_Attributes& attrib) {
        attributes = attrib;
    }

    /** @return the current attributes used for new entities. */
    DL_Attributes getAttributes() {
        return attributes;
    }

    /** Sets the current attributes for entities. */
    void setExtrusion(double dx, double dy, double dz, double elevation) {
        extrusion->setDirection(dx, dy, dz);
		extrusion->setElevation(elevation);
    }

    /** @return the current attributes used for new entities. */
    DL_Extrusion* getExtrusion() {
        return extrusion;
    }
public:
	vector<LP_Layer>* layers;
	vector<LP_Block>* blocks;
protected:
	LP_Layer* currentlayer;
	LP_Spline*  currentspline;
	LP_Polyline*  currentpolyline;
	vector<int> actionstack;
	vector<LP_Block*> currentblock;
};
#endif