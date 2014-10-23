#ifndef LP_Graph_H
#define LP_Graph_H
#include "LP_Base.h"
class LP_MText : public LP_Graph{     //存储并管理文本的类
    /**
     * Constructor.
     * Parameters: see member variables.
     */
public:
    LP_MText(double tipx = 0, double tipy = 0, double tipz = 0,
                 double tHeight = 12, double tWidth = 10,
                 int tAttachmentPoint = 0,
                 int tDrawingDirection = 0,
                 int tLineSpacingStyle = 0,
                 double tLineSpacingFactor = 0,
                 const string& tText = "",
                 const string& tStyle = "",
                 double tAngle = 0) {
        inspoint.x = tipx;
        inspoint.y = tipy;
        inspoint.z = tipz;

        height = tHeight;
        width = tWidth;
        attachmentPoint = tAttachmentPoint;
        drawingDirection = tDrawingDirection;
        lineSpacingStyle = tLineSpacingStyle;
        lineSpacingFactor = tLineSpacingFactor;
        text = tText;
        style = tStyle;
        angle = tAngle;
		type = LP_MTEXT; //定义数据类型
    }
	bool checkPick(LP_Point3 wp,LP_Coordinate &cod);
	bool rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod);
	void Draw(wxDC * dc, LP_Coordinate &cod);
    LP_Point3 inspoint;   //**Coordinate of insertion point.
    double height;/*! Text height */
    double width; /*! Width of the text box. */
    
    int attachmentPoint;/** Attachment point.* 1 = Top left, 2 = Top center, 3 = Top right,
                                             * 4 = Middle left, 5 = Middle center, 6 = Middle right,
                                             * 7 = Bottom left, 8 = Bottom center, 9 = Bottom right*/

    int drawingDirection;/* Drawing direction.** 1 = left to right, 3 = top to bottom, 5 = by style*/

    int lineSpacingStyle;/* Line spacing style.* 1 = at least, 2 = exact*/

    double lineSpacingFactor;/*Line spacing factor. 0.25 .. 4.0  */

    string text;/*! Text string. */
    string style;/*! Style string. */
    double angle;/*! Rotation angle. */
};
//直线数据****************************************************************************
class LP_Line : public LP_Graph {
public:
	LP_Line();
    LP_Line(LP_Point3 p1, LP_Point3 p2);
    LP_Line(double p1x, double p1y, double p1z, double p2x, double p2y, double p2z);
	void setRect();
    void Draw(wxDC * dc, LP_Coordinate &cod);
	bool checkPick(LP_Point3 wp,LP_Coordinate &cod);
	bool rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod);
	void move(LP_Point3 &ref);
	//	* Rotates this vector around 0/0 by the given angle.
    void rotate(double ang);
//	* Rotates this vector around the given center by the given angle.
    void rotate(LP_Point3 center, double ang);
//	* Scales this vector by the given factors with 0/0 as center.
    void scale(LP_Point3 factor);
//	* Scales this vector by the given factors with the given center.
    void scale(LP_Point3 center, LP_Point3 factor);
//	* Mirrors this vector at the given axis.
    void mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2);
public:
    LP_Point3 point1;
    LP_Point3 point2;
	LP_Rect rect;    //图元的包围矩形
};
class LP_Lines : public LP_Graph {
public:
	LP_Lines();
    LP_Lines(LP_Point3 p1, LP_Point3 p2);
	LP_Rect& setRect();
	bool checkPick(LP_Point3 wp,LP_Coordinate &cod);
	bool rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod);
    void Draw(wxDC * dc, LP_Coordinate &cod);
	void move(LP_Point3 &ref);
	//	* Rotates this vector around 0/0 by the given angle.
    void rotate(double ang);
//	* Rotates this vector around the given center by the given angle.
    void rotate(LP_Point3 center, double ang);
//	* Scales this vector by the given factors with 0/0 as center.
	void scale(double xscale,double yscale,double zscale);
	void scale(LP_Point3 center,double xscale,double yscale,double zscale);
    void scale(LP_Point3 factor);
//	* Scales this vector by the given factors with the given center.
    void scale(LP_Point3 center, LP_Point3 factor);
//	* Mirrors this vector at the given axis.
    void mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2);
	bool operator< (const LP_Lines& ls) const;
public:
    vector <LP_Point3> points;
	vector <LP_Point3> hotpoints;
	LP_Rect rect;    //图元的包围矩形
	bool istemp;
	//LP_Graph *master;  //指向生成此lines的图元的基类型指针，可在使用时向下类型转换成相应类型的指针
};

//管理椭圆的类***********************************************************
class LP_Ellipse : public LP_Graph {
public:
	LP_Ellipse();
    LP_Ellipse(LP_Point3 centerp, double radiuscircle);  //此方法构建圆
    LP_Ellipse(LP_Point3 centerp, double radiuscircle,double a1,double a2,bool reversedarc/*圆弧的正反向*/); //此方法构建圆弧
    LP_Ellipse(LP_Point3 centerp, LP_Point3 majorp, double ratiox, double a1, double a2);  //此方法构建椭圆弧
    LP_Ellipse(LP_Point3 centerp, LP_Point3 majorp, double ratiox);   //此方法构建椭圆
    LP_Ellipse& operator= (const LP_Ellipse& ellipse);
    void Create(double smooth);
	void setRect();
	void setPick(bool flag);
	bool checkPick(LP_Point3 wp,LP_Coordinate &cod);
	bool rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod);
    void Draw(wxDC * dc, LP_Coordinate &cod);
	void move(LP_Point3 &ref);
	//	* Rotates this vector around 0/0 by the given angle.
    void rotate(double ang);
//	* Rotates this vector around the given center by the given angle.
    void rotate(LP_Point3 cent, double ang);
//	* Scales this vector by the given factors with 0/0 as center.
    void scale(LP_Point3 factor);
//	* Scales this vector by the given factors with the given center.
    void scale(LP_Point3 cent, LP_Point3 factor);
//	* Mirrors this vector at the given axis.
    void mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2);
public:
    LP_Lines templine; //存放绘图用的模拟椭圆的直线。
	vector <LP_Point3> hotpoints;
    LP_Point3 center;     //椭圆的中心
    LP_Point3 majorAxis;  //椭圆长轴相对椭圆中心的端点
    double ratio;         //椭圆的短轴对长轴的比例
    double angle1;        //椭圆弧相对半长轴的起点弧度值
    double angle2;        //椭圆弧的相对半长轴终点弧度值
	LP_Rect rect;
	bool reversed;
};
//管理圆的类******************************************************************
/*class LP_Circle : public LP_Ellipse{
public:
    LP_Circle();
    LP_Circle(double acx, double acy, double acz,double aRadius);
    LP_Circle(LP_Point3 apoint,double aRadius);
};*/

// 线型数据****************************************************************************
class LP_LineType {
public:
    LP_LineType(const string& lName, int lFlags);
public:
    /** Line type name. */
    string name;
    /** Line type flags. */
    int flags;
};
// 多段线数据*************************************************************************
class LP_Vertex{
public:
        /**
     * Constructor.
     * Parameters: see member variables.
     */
    LP_Vertex(double px=0.0, double py=0.0, double pz=0.0,double pBulge=0.0);
    LP_Vertex& operator = (const LP_Vertex& vertex);
	void move(LP_Point3 &ref);
	//	* Rotates this vector around 0/0 by the given angle.
    void rotate(double ang);
//	* Rotates this vector around the given center by the given angle.
    void rotate(LP_Point3 center, double ang);
//	* Scales this vector by the given factors with 0/0 as center.
    void scale(LP_Point3 factor);
//	* Scales this vector by the given factors with the given center.
    void scale(LP_Point3 center, LP_Point3 factor);
//	* Mirrors this vector at the given axis.
    void mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2);
public:
    /*! X Coordinate of the vertex. */
    /*! Y Coordinate of the vertex. */
    /*! Z Coordinate of the vertex. */	
    LP_Point3 point;
    /*! Bulge of vertex.
     * (The tangent of 1/4 of the arc angle or 0 for lines) */

    double bulge;
};
class LP_Polyline : public LP_Graph {
public:
	LP_Polyline();
    LP_Polyline(int pNumber, int pMVerteces, int pNVerteces, int pFlags);
    void Create(double smooth);
	void setRect();
	void setPick(bool flag);
	bool checkPick(LP_Point3 wp,LP_Coordinate &cod);
	bool rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod);
    void Draw(wxDC * dc, LP_Coordinate &cod);
	void move(LP_Point3 &ref);
	//	* Rotates this vector around 0/0 by the given angle.
    void rotate(double ang);
//	* Rotates this vector around the given center by the given angle.
    void rotate(LP_Point3 center, double ang);
//	* Scales this vector by the given factors with 0/0 as center.
    void scale(LP_Point3 factor);
//	* Scales this vector by the given factors with the given center.
    void scale(LP_Point3 center, LP_Point3 factor);
//	* Mirrors this vector at the given axis.
    void mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2);
private:
     LP_Ellipse createArc(LP_Vertex &startp, LP_Vertex &endp);
public:
    LP_Lines templine;
    vector <LP_Vertex> vertexs;
	vector <LP_Point3> hotpoints;
    /*! Number of vertices in this polyline. */
    unsigned int number;
    /*! Number of vertices in m direction if polyline is a polygon mesh. */
    unsigned int m;
    /*! Number of vertices in n direction if polyline is a polygon mesh. */
    unsigned int n;
    /*! Flags */
    int flags;
	LP_Rect rect;
};
// 样条曲线数据**********************************************************************
class LP_Spline : public LP_Graph {
public:
	LP_Spline();
    LP_Spline(int pDegree, int pNKnots, int pNControl, int pFlags);
    void addKonts(const double kont);
    void addControl(const LP_Point3 point);
    void addKontsAt(int index, double knot);
    void addControlAt(int index, LP_Point3 point);
    void removeKontsAt(int index);
    void removeControlAt(int index);
    void Create(const double smooth);        //生新生成绘制样条曲线的型值点坐标
	void update(double smooth);
	static void rbasis(int c, double t, int npts, int x[], double h[], double r[]);
	
	static void knot(int num, int order, int knotVector[]);
	static void rbspline(int npts, int k, int p1,
		double b[], double h[], double p[]);
		
	static void knotu(int num, int order, int knotVector[]);
	static void rbsplinu(int npts, int k, int p1,
		double b[], double h[], double p[]);
	void setRect();
	void setPick(bool flag);
	bool checkPick(LP_Point3 wp,LP_Coordinate &cod);
	bool rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod);
    void Draw(wxDC * dc, LP_Coordinate &cod);
	void move(LP_Point3 &ref);
	//	* Rotates this vector around 0/0 by the given angle.
    void rotate(double ang);
//	* Rotates this vector around the given center by the given angle.
    void rotate(LP_Point3 center, double ang);
//	* Scales this vector by the given factors with 0/0 as center.
    void scale(LP_Point3 factor);
//	* Scales this vector by the given factors with the given center.
    void scale(LP_Point3 center, LP_Point3 factor);
//	* Mirrors this vector at the given axis.
    LP_Spline& mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2);
public:
    /*! Degree of the spline curve. */
    unsigned int degree;
    /*! Number of knots. */
    unsigned int nKnots;
    /*! Number of control points. */
    unsigned int nControl;
    /*! Flags */
    int flags;
    bool closed;
	LP_Rect rect;
    LP_Lines templine;
    vector <double>  konts;
    vector < LP_Point3>  controlPoints;
	vector <LP_Point3> hotpoints;
};
#endif