#ifndef LP_ADVANCED_H
#define LP_ADVANCED_H
#include "LP_Graph.h"
#include "LP_Enginery.h"
// 管理块的数据的类***********************************************************
class LP_Insert;
//class LP_EngineryBuffer;
class LP_CAM_Group;
class LP_Block : public LP_Graph{
public:
	LP_Block(void);
    LP_Block(LP_Point3 bpoint, string bname);
    LP_Block(LP_Point3 bpoint, string bname, int bflags);
    void setBasePoint(const LP_Point3 bpoint);
    void setName(const string bname);
    void setFlags(const int bflags);
    LP_Point3 getBasePoint() const ;
    string getName() const ;
	const vector <LP_Lines>& getTemp() const;
	const vector<LP_Point3>& getHot() const;
    int getFlags() const ;
    void addArc(LP_Ellipse arc);
    void addEllipse(LP_Ellipse ellipse);
    void addLine(LP_Line line);
    LP_Polyline& addPolyline(LP_Polyline polyline);
    LP_Spline& addSpline(LP_Spline spline);
    void addPoint(LP_Point3 point);
	void addInsert(LP_Insert ins);
	void addMText(LP_MText mt);
	void setRect();
	void Create(LP_Coordinate cod);
	void Create(const double smooth);
	void ReFresh(vector<LP_Block>& blks);
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
	void getTrack(LP_CAM_Group &buf);
public:
	bool old;     //刷新块时所使用的表示块未刷新的标志，若已刷新则不再刷新
private:
        /*! Name of the referred block. */
    string name;
    /*! x Coordinate of insertion point. */
    LP_Point3 basePoint;
	LP_Rect rect;  //包围矩形
    int flags;    //块的状态标志
    vector <int> sequencetype;
//    vector <LP_Arc> arcs;

	vector <LP_Lines> templines;
	vector <LP_Point3> hotpoints;
    vector <LP_Point3> points;

//    vector <LP_Block> blocks;
	vector <LP_Insert> inserts;
    vector <LP_Ellipse> ellipses;
    vector <LP_Line> lines;
    vector <LP_Polyline> polylines;
    vector <LP_Spline> splines;
	vector <LP_MText> mtexts;
};
// 管理插入块的数据的类***********************************************************
class LP_Insert : public LP_Graph{
public:
    LP_Insert(const string& iName = "",
                  double iipx = 0, double iipy = 0, double iipz = 0,
                  double isx = 1, double isy = 1, double isz = 1,
                  double iAngle = 0,
                  int iCols = 1, int iRows = 1,
                  double iColSp = 0, double iRowSp = 0);
    LP_Insert(const string& iName,
                  LP_Point3 point,
                  double isx, double isy, double isz,
                  double iAngle,
                  int iCols, int iRows,
                  double iColSp, double iRowSp);
	void setRect();
	void Create(LP_Coordinate cod);
	void Create(const double smooth);
	void ReFresh(vector<LP_Block>& blks);
	bool checkPick(LP_Point3 wp,LP_Coordinate &cod);
	bool rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod);
	void setPick(bool flag);
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
	void getTrack(LP_CAM_Group &buf);
public:
        /*! Name of the referred block. */
    string name;
    /*插入图形的基点 */
    LP_Point3 basePoint;
	LP_Rect rect;  //包围矩形
    /*! x Scale factor. */
    double sx;
    /*! y Scale factor. */
    double sy;
    /*! z Scale factor. */
    double sz;
    /*! Rotation angle in rad. */
    double angle;
    /*! Number of columns if we insert an array of the block or 1. */
    int cols;
    /*! Number of rows if we insert an array of the block or 1. */
    int rows;
    /*! Values for the spacing between cols. */
    double colSp;
    /*! Values for the spacing between rows. */
    double rowSp;
	vector <LP_Lines> templines;
	vector <LP_Point3> hotpoints;
    LP_Block iblock;
};
class LP_Layer;
class LP_Selected : public LP_Graph {
public:
	LP_Selected();
	bool Empty();
	void clear();
	bool checkPick(LP_Point3 wp,LP_Coordinate &cod);
	void move(LP_Point3 &ref);  //以原点与ref为参考点移动所选图形
    void rotate(double ang);    //以0/0为中心旋转ang角度
    void rotate(LP_Point3 center, double ang); //以所给点为中心旋转ang角度
    void scale(LP_Point3 factor);  //以0/0为中心缩放factor指定的倍率
    void scale(LP_Point3 center, LP_Point3 factor);  //以所给点为中心缩放factor指定的倍率
    void mirror(LP_Point3 axisPoint1, LP_Point3 axisPoint2);  //以给定为轴镜像图形
	void setRect();
	vector <int>& getColors();
	void Draw(wxDC * dc, LP_Coordinate &cod);
public:
    vector <LP_Point3 *> points;
    vector <LP_Block *> blocks;
	vector <LP_Insert *> inserts;
    vector <LP_Ellipse *> ellipses;
    vector <LP_Line *> lines;
    vector <LP_Polyline *> polylines;
    vector <LP_Spline *> splines;
	vector <LP_MText *> mtexts;
	vector <LP_Layer *> layers;
	LP_Rect rect;
private:
	bool isempty;
	vector <int> colors;
};

//图层的数据.*************************************所有图形对象都包含在图层里，每个图形对象的属性里也包含属于哪个图层的属性。当两个不一致时，以图形
class LP_Layer : public LP_Graph {              //实际所属的图层为准；应将图形对象的图层属性更正为其所属图层。
public:
	LP_Layer ();
	LP_Layer(const string& lName,int lFlags);
	void setRect();
	void Create(const double smooth);
	void move(LP_Point3 &ref);
	void Create(LP_Coordinate cod);
	void ReFresh(vector<LP_Block>& blks);
	bool checkPick(LP_Point3 wp,LP_Coordinate &cod);
	bool rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod);
	bool getPick(LP_Selected &sel);
	void deSelectAll();
	void delSelected();
	bool Empty();
	void Draw(wxDC * dc, LP_Coordinate &cod);
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
	void getTrack(LP_CAM_Group& buf);
public:
    /** Layer name. */
    string name;
    /** Layer flags. (1 = frozen, 2 = frozen by default, 4 = locked) */
    int flags;
	LP_Rect rect;
public:
	vector <LP_Lines> templines;
	vector <LP_Point3> hotpoints;
    vector <LP_Point3> points;
//    vector <LP_Arc> arcs;
	vector <LP_Insert> inserts;
    vector <LP_Ellipse> ellipses;
    vector <LP_Line> lines;
    vector <LP_Polyline> polylines;
    vector <LP_Spline> splines;
	vector <LP_MText> mtexts;
private:
	bool isempty;
};
// added by ZWW
#define		MAX_DIMSTYLES	64
#define		MAX_LAYERS		128
#define		MAX_LTYPES		128
#define		MAX_STYLES		128

typedef unsigned int UINT;

// Tables Structures *******************************************
/*typedef struct tag_LTYPE {
    OBJHANDLE Objhandle;			// Handle
    string	name;		            // Line Type Name
    char	StandardFlags;			// Standard flags
    char	DescriptiveText[512];	// Descriptive Text
    short	ElementsNumber;			// Line Elements Number
    double	Elements[30];			// Line Elements (Max=30)
    double	PatternLength;          // Length of linetype pattern
} LTYPE, *PLTYPE;*/
typedef struct tag_LTYPE {
    string name;
    int flags;
}LTYPE, *PLTYPE;

typedef struct tag_LAYER {
    //OBJHANDLE Objhandle;			// Handle
    string	name;		            // Layer Name
    char	StandardFlags;			// Standard flags
    short	Color; 					// Layer Color (if negative, layer is Off)
    //OBJHANDLE LineTypeObjhandle;	// Handle of linetype for this layer
    short	LineWeight;				// Layer's Line Weight                      (*2000*)
    bool	PlotFlag;				// TRUE=Plot this layer                     (*2000*)
    //OBJHANDLE PlotStyleObjhandle;	// handle of PlotStyleName object           (*2000*)
} LAYER, *PLAYER;

typedef struct tag_STYLE {
    string	name;		            // Style Name
    int	    flags;			        // Standard flag values -> 4=Vertical text
    double fixedTextHeight;
    double widthFactor;
    double obliqueAngle;
    int textGenerationFlags;
    double lastHeightUsed;
    string primaryFontFile;
    string bigFontFile;
} STYLE, *PSTYLE;

typedef struct tag_DIMSTYLE {
    //OBJHANDLE Objhandle;			// Handle
    string	name;		            // name of dimension style
    char	StandardFlags;			// Standard flag values
    double	dimasz;					// Arrow size
    char	dimblk1[16];			// 1st Arrow head
    char	dimblk2[16];			// 2nd Arrow head
    short	dimclrd;				// Dimension line & Arrow heads color
    short	dimclre;				// Extension line color
    short	dimclrt;				// Text color
    double	dimdle;					// Dimension line size after Extensionline
    double	dimexe;					// Extension line size after Dimline
    double	dimexo;					// Offset from origin
    double	dimgap;					// Offset from dimension line
    double	dimtxt;					// Text height
    char	dimtad;					// Vertical Text Placement
    //OBJHANDLE dimtxstyObjhandle;	// Text style handle
} DIMSTYLE, *PDIMSTYLE;

typedef struct tag_TABLES
{
    /*LTYPE		LType[MAX_LTYPES];
    LAYER		Layer[MAX_LAYERS];
    STYLE		Style[MAX_STYLES];
    DIMSTYLE	DimStyle[MAX_DIMSTYLES];*/
    vector <DL_LineTypeData> LTypes;
    vector <LAYER> Layers;
    vector <DL_StyleData> Styles;
    vector <DIMSTYLE> DimStyles;

    UINT		NumLayers;
    UINT		NumLTypes;
    UINT		NumStyles;
    UINT		NumDimStyles;

    UINT		CurrentLayer;
    UINT		CurrentLType;
    UINT		CurrentStyle;
    UINT		CurrentDimStyle;
} TABLES, *PTABLES;
// end added by ZWW

class LP_Graphics_Container {
public:
	vector <LP_Layer>& getGraphics();
	LP_Rect getRect(double smooth);
	bool setPick(bool sflag);
	bool getPick(LP_Selected &sel);
	void addLayer(LP_Layer &layer);
	vector<LP_Layer>& getLayers();
	void Create(const double smooth);
	void Create(LP_Coordinate cod);
	bool rectPick(wxPoint firstp,wxPoint secondp,LP_Coordinate &cod);
	bool checkPick(LP_Point3 wp,LP_Coordinate &cod);
	void deSelectAll();
	void delSelected();
	void getTrack(vector<LP_CAM_Group>& camgp);
	void Draw(wxDC * dc, LP_Coordinate &cod);
public:
    // added by ZWW
    TABLES		Tables;
    // end added
	vector <LP_Layer> layers;
	vector <LP_Block> blocks;
	LP_Rect rect;
	bool IsPick;
};
#endif