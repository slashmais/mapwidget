#ifndef _mt_mindmap_h_
#define _mt_mindmap_h_

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#include "maptypes.h"
#include "toolbox.h"
#include <string>
#include <vector>
#include <map>


//==============================================================================================
struct TextBoxCtrl : public Label
{
	typedef TextBoxCtrl CLASSNAME;
	TextBoxCtrl();
	void TBCSetText(const std::string sText, int W=200);
};

//==============================================================================================
struct TopicWidget : public Ctrl
{
	typedef TopicWidget CLASSNAME;
	TextBoxCtrl tbc;
	std::string sText; //name
	int Disp; //expanded / .. (disposition)
	bool bShowPics;
	bool bShowBoxed;
	bool bSelected;
	TOPICTYPE TT;
	Image pic; //supplied by user-app
	Color DefaultBg, DefaultFg, FocusBg, FocusFg, PaintBg, PaintFg;
	Color DispLockfg, DispLockbg, DispExpfg, DispExpbg;
	bool bShowFocus, bMoving, bLButtonDown;
	Point MP; //mouse-point
	void DrawFocus(bool bShow=true);
	Point GetLinkPointFrom();
	Point GetLinkPointTo();
	void CalcRect();
	Event<TopicWidget*, Point, dword> WhenRightDown;
	Event<TopicWidget*, Point, dword> WhenRightUp;
	Event<TopicWidget*, Point, dword> WhenLeftDown;
	Event<TopicWidget*, Point, dword> WhenLeftUp;
	Event<TopicWidget*, Point, dword> WhenLeftDouble;
	Event<TopicWidget*, Point, dword> WhenMouseMove;
	Event<TopicWidget* > WhenGotFocus;
	Event<TopicWidget* > WhenLostFocus;
	Event<TopicWidget* > WhenMoving;
	TopicWidget();
	void Paint(Draw &w);
	void Select(bool b=true)	{ bSelected=b; Refresh(); }
	void SetShowFocus(bool b=true);
	void SetTypePic(Image img);
	void ResetTypePic();
	void SetText(const std::string sCap);
	void SetDisp(int disp);
	const std::string GetText();
	void SetToolTip(const std::string &sTip);
	virtual void MouseEnter(Point p, dword keyflags);
	virtual void MouseLeave();
	virtual void RightDown(Point p, dword keyflags);
	virtual void RightUp(Point p, dword keyflags);
	virtual void LeftDown(Point p, dword keyflags);
	virtual void LeftUp(Point p, dword keyflags);
	virtual void LeftDouble(Point p, dword keyflags);
	virtual void MouseMove(Point p, dword keyflags);
	virtual void MouseWheel(Point p, int z, dword d) { Ctrl::MouseWheel(p, z, d); }
	void SetTopicType(TOPICTYPE tt);
	TOPICTYPE GetTopicType();
};

struct Topics : public std::vector<TopicWidget*>
{
	//using list=std::vector<TopicWidget*>;
	void clean() { while (size()) { delete (*(begin())); erase(begin()); }}
	~Topics() { clean(); }
	Topics() { clean(); }
	bool has(TopicWidget *pt) { for (auto p:(*this)) { if (p==pt) return true; } return false; }
	void Add(TopicWidget *pt) { if (!has(pt)) push_back(pt); }
	void Delete(TopicWidget *pt)
	{
		auto it=begin();
		while (it!=end()) { if ((*it)==pt) { erase(it); break; } else it++; }
		delete pt;
	}
};

//==============================================================================================
struct LinkWidget //NB: fake widget!
{
	TopicWidget *pTF;
	TopicWidget *pTT;
	std::string name;
	Color linecolor;
	int linewidth;
	LINKTYPE LT;
	Image pic;
	Color selcolor;
	Color oldcolor;
	int selwidth;
	int oldwidth;
	LinkWidget();
	virtual ~LinkWidget();
	Event<LinkWidget*> WhenLinkDataChange;
	void DoCallback();
	bool PoLL(Point p); //point-on(near)-linkline
	void SetLineWidth(int n);
	int GetLineWidth();
	void SetLineColor(Color c=Black());
	Color GetLineColor();
	void SetLinkType(LINKTYPE lt);
	LINKTYPE GetLinkType();
	void SelectLine(bool b=true);
};

struct Links : public std::vector<LinkWidget*>
{
	//using list=std::vector<LinkWidget*>;
	void clear() { while (size()) { delete (*(begin())); erase(begin()); }}
	~Links() { clear(); }
	Links() { clear(); }
	bool has(LinkWidget *p) { for (auto l:(*this)) { if (l==p) return true; } return false; }
	LinkWidget* Get(TopicWidget *pf, TopicWidget *pt) { for (auto l:(*this)) { if ((l->pTF==pf)&&(l->pTT==pt)) return l; } return nullptr; }
	void Add(LinkWidget *p) { if (!has(p)) push_back(p); }
	void Delete(LinkWidget *p)
	{
		auto it=begin();
		while (it!=end()) { if ((*it)==p) { erase(it); break; } else it++; }
		delete p;
	}
	LinkWidget* frompoint(Point p) { for (auto l:(*this)) { if (l->PoLL(p)) return l; } return nullptr; }
};

//==============================================================================================
//-------------------------------------------------------------------------------------------------ShapePoint
struct ShapePoint : public Point
{
	Color fg;
	Color bg;
	int width;
	void clear();
	virtual ~ShapePoint();
	ShapePoint();
	ShapePoint(Point p, Color f, Color b, int w);
	ShapePoint(const std::string &spdata);
	ShapePoint(const ShapePoint &P);
	ShapePoint(const Point &p);
	ShapePoint& operator=(const ShapePoint &P);
	ShapePoint& operator-=(const ShapePoint &P);
	ShapePoint& operator+=(const ShapePoint &P);
	ShapePoint& operator=(const Point &p);
	ShapePoint& operator-=(const Point &p);
	ShapePoint& operator+=(const Point &p);
	std::string SPData();
	void SPData(const std::string &sd);
};

ShapePoint operator-(const ShapePoint &l, const ShapePoint &r);
ShapePoint operator+(const ShapePoint &l, const ShapePoint &r);
ShapePoint operator-(const ShapePoint &l, const Point &r);
ShapePoint operator+(const ShapePoint &l, const Point &r);

enum { FLIP_LEFT=1, FLIP_RIGHT, };
ShapePoint flip_point(ShapePoint P, int lr, int wh); //left, right
ShapePoint flate_point(ShapePoint P, int cw, int ch, int nw, int nh); //in-, de- resizing

//-------------------------------------------------------------------------------------------------ShapePoints
struct ShapePoints
{
private:
	std::vector<ShapePoint> points;
public:
	using iterator=std::vector<ShapePoint>::iterator;
	using const_iterator=std::vector<ShapePoint>::const_iterator;
	void clear() { points.clear(); }
	~ShapePoints() {}
	ShapePoints() { clear(); }
	ShapePoints(const ShapePoints &L) { *this=L; }
	ShapePoints& operator=(const ShapePoints &L) { for (auto p:L) points.push_back(p); return *this; }
	bool empty() { return points.empty(); }
	iterator begin() { return points.begin(); }
	iterator end() { return points.end(); }
	const_iterator begin() const { return points.begin(); }
	const_iterator end() const { return points.end(); }
	void Add(const ShapePoint &P) { points.push_back(P); }
	void Remove() { if (points.size()>0) points.pop_back(); }
	void set_first(const ShapePoint &P) { points.at(0)=P; }
};

//-------------------------------------------------------------------------------------------------Shape
enum { ISA_SHA=1, ISA_SKE, ISA_LIN, ISA_BOX, ISA_OVL, ISA_JOT, };
struct Shape
{
	ShapePoint f;
	ShapePoint t;
	bool bSelected;
	
	virtual ~Shape() {}
	virtual int isa() { return ISA_SHA; }
	virtual const std::string shapename() { return "shape"; }
	virtual void SelectShape(bool bSel=true) { bSelected=bSel; }
	virtual bool IsSelected() { return bSelected; }
	virtual void clear()=0;
	virtual void flip(int lr, int wh)=0;
	virtual void flate(int cw, int ch, int nw, int nh)=0;
	virtual void Render(Draw &drw)=0;
	virtual std::string ShapeData()=0;
	virtual void ShapeData(const std::string &sd)=0;
	virtual void process_MouseMove(ShapePoint sp, dword keyflags)=0;
	virtual void process_LeftDown(ShapePoint sp, dword keyflags)=0;
	virtual bool process_LeftUp(ShapePoint sp, dword keyflags)=0; //bool because may need edit & can cancel
};

//-------------------------------------------------------------------------------------------------Shapes
struct Shapes : public std::vector<Shape*>
{
	void clear() { while (size()) { delete (*(begin())); erase(begin()); }}
	void push(Shape *pS) { if (pS) push_back(pS); }
	Shape* pop();
	~Shapes() { clear(); }
	Shapes() { clear(); }
	bool has(Shape *pS) { for (auto p:(*this)) { if (p==pS) return true; } return false; }
	void Add(Shape *pS) { if (!has(pS)) push(pS); }
	Shape* add_restore(const std::string &shapedata); //uses any of sketch,line,box,oval,jot,.?.
	void Delete(Shape *pS)
	{
		auto it=begin();
		while (it!=end()) { if ((*it)==pS) { erase(it); break; } else it++; }
		delete pS;
	}
	Shape* frompoint(Point p);
};

typedef Shapes URShapes; //undo-redo

//-------------------------------------------------------------------------------------------------Sketch
struct Sketch : public Shape //free-hand-drawing
{
	ShapePoints points;
	virtual int isa() { return ISA_SKE; }
	virtual const std::string shapename() { return "sketch"; }
	virtual void clear();
	~Sketch();
	Sketch();
	Sketch(const Sketch &S);
	Sketch& operator=(const Sketch &S);
	void AddPoint(ShapePoint p);
	void DeletePoint();
	virtual void flip(int lr, int wh);
	virtual void flate(int cw, int ch, int nw, int nh);
	virtual void Render(Draw &drw);
	virtual std::string ShapeData();
	virtual void ShapeData(const std::string &sd);
	virtual void process_MouseMove(ShapePoint sp, dword keyflags);
	virtual void process_LeftDown(ShapePoint sp, dword keyflags);
	virtual bool process_LeftUp(ShapePoint sp, dword keyflags);
};

//-------------------------------------------------------------------------------------------------Line
struct Line : public Shape
{
	virtual int isa() { return ISA_LIN; }
	virtual const std::string shapename() { return "line"; }
	virtual void clear();
	virtual ~Line();
	Line();
	Line(const Line &L);
	Line& operator=(const Line &L);
	virtual void flip(int lr, int wh);
	virtual void flate(int cw, int ch, int nw, int nh);
	virtual void Render(Draw &drw);
	virtual std::string ShapeData();
	virtual void ShapeData(const std::string &sd);
	virtual void process_MouseMove(ShapePoint sp, dword keyflags);
	virtual void process_LeftDown(ShapePoint sp, dword keyflags);
	virtual bool process_LeftUp(ShapePoint sp, dword keyflags);
	bool PoLL(Point P);
};

//-------------------------------------------------------------------------------------------------Box
struct Box : public Shape
{
	virtual int isa() { return ISA_BOX; }
	virtual const std::string shapename() { return "box"; }
	virtual void clear();
	virtual ~Box();
	Box();
	Box(const Box &B);
	Box& operator=(const Box &B);
	virtual void flip(int lr, int wh);
	virtual void flate(int cw, int ch, int nw, int nh);
	virtual void Render(Draw &drw);
	virtual std::string ShapeData();
	virtual void ShapeData(const std::string &sd);
	virtual void process_MouseMove(ShapePoint sp, dword keyflags);
	virtual void process_LeftDown(ShapePoint sp, dword keyflags);
	virtual bool process_LeftUp(ShapePoint sp, dword keyflags);
};

//-------------------------------------------------------------------------------------------------Oval
struct Oval : public Shape
{
	virtual int isa() { return ISA_OVL; }
	virtual const std::string shapename() { return "oval"; }
	virtual void clear();
	virtual ~Oval();
	Oval();
	Oval(const Oval &O);
	Oval& operator=(const Oval &O);
	virtual void flip(int lr, int wh);
	virtual void flate(int cw, int ch, int nw, int nh);
	virtual void Render(Draw &drw);
	virtual std::string ShapeData();
	virtual void ShapeData(const std::string &sd);
	virtual void process_MouseMove(ShapePoint sp, dword keyflags);
	virtual void process_LeftDown(ShapePoint sp, dword keyflags);
	virtual bool process_LeftUp(ShapePoint sp, dword keyflags);
};

//-------------------------------------------------------------------------------------------------Jot
struct Jot : public Shape
{
	Font font;
	std::string text;
	int padding{0}; //between lines not used & not persistent(not saved)
	virtual int isa() { return ISA_JOT; }
	virtual const std::string shapename() { return "jot"; }
	virtual void clear();
	virtual ~Jot();
	Jot();
	Jot(const Jot &J);
	Jot& operator=(const Jot &J);
	virtual void flip(int lr, int wh);
	virtual void flate(int cw, int ch, int nw, int nh);
	virtual void Render(Draw &drw);
	virtual std::string ShapeData();
	virtual void ShapeData(const std::string &sd);
	virtual void process_MouseMove(ShapePoint sp, dword keyflags);
	virtual void process_LeftDown(ShapePoint sp, dword keyflags);
	virtual bool process_LeftUp(ShapePoint sp, dword keyflags);
	bool Edit();
};

//==============================================================================================
struct MapWidget : public Ctrl
{
	using CLASSNAME=MapWidget;

	int mdx, mdy, Lm, Rm, Tm, Bm;
	bool bLeftDown, bRightDown, bMoving, bLinking;
	bool bShowMargins;
	TopicWidget *pCurTopic;
	LinkWidget *pLDCur;
	struct TempFT { Point f; Point t; bool isvalid(){return !(((f.x==0)&&(f.y==0))||((t.x==0)&&(t.y==0)));} }; //not trusting Point::IsZero()!
	TempFT lldraw; //temporary for drawing the link
	TempFT selector; //multiple(-box-)selection
	bool bSelecting;

	//>>>drawer.cpp>>>
	bool bIsDrawing;
	Point SPTOPLEFT;
	typedef std::map<ClickBox*, int> ToolList;
	enum { TOOL_NONE=0, TOOL_SKETCH=1, TOOL_LINE, TOOL_BOX, TOOL_OVAL, TOOL_JOT, };
	bool bSketch;
	bool bLine;
	bool bBox;
	bool bOval;
	bool bJot;
	Shape *pCurShape;
	Color curColor; //fg
	Color curBColor; //bg
	int curWidth;
	ToolPanel toolpanel;
	ToolList undoredotools;
	ToolList shapetools;
	ToolList sizetools;
	ClickBox *pSelectTool;
	ClickBox *pResetTool;
	enum { LP_LTOP=1, LP_PTOL, };
	void flip_shapes(int lp, int wh);
	void flate_shapes(int cw, int ch, int nw, int nh);
	void init_shapes(); //for ctor-use only, in drawer.cpp
	bool IsDrawing();
	void ToolBoxPaint(Draw &drw); //called from Paint() in seeder
	void set_undoredotools();
	void set_shapetools();
	void set_sizetools();
	void do_selecttool();
	void do_shapetools(ClickBox *pcb);
	void do_sizetools(ClickBox *pcb);
	void clear_toolselections(ToolList &TL);
	void save_new_shape(Shape *p);
	void undoshape();
	void redoshape();
	void SetResetTool();
	void check_reset();
	void ResetToolpanel(bool b=true);
	void SetCurColor(ClickBox *pcb, bool bfg=true);
	void unset_tools();
	void SetTool(int T, ClickBox *pcb=nullptr);
	void ToolMouseMove(Point p, dword keyflags);
	void ToolLeftDown(Point p, dword keyflags);
	void ToolLeftUp(Point p, dword keyflags);
	virtual bool Key(dword key, int count); ///todo later - place in MapWidget proper
	//<<<drawer.cpp<<<

	Topics topics;
	Links links;
	Shapes shapes;
	Shape *pSCur;
	URShapes urshapes;
	Topics vmultisel;

	virtual ~MapWidget();
	MapWidget();

	virtual void Paint(Draw &w);
	void DrawMargins(Draw &drw, Size sz);
	void DrawLinkText(Draw &drw, LinkWidget *pL);
	void SetMargins(int L, int R, int T, int B);
	int GetLmargin();
	int GetRmargin();
	int GetTmargin();
	int GetBmargin();
	
	TopicWidget* MakeTopic(int x, int y, const std::string &sCap="new topic", int disp=0, TOPICTYPE tt=TT_GENERIC);
	void DeleteTopic(TopicWidget *pT);
	
	LinkWidget* MakeLink(TopicWidget *ptf, TopicWidget *ptt, const std::string &desc="-->", LINKTYPE lt=LT_ASSOC);
	void DeleteLink(LinkWidget *pL);
	
	Shape* MakeShape(const std::string &sdata);
	void DeleteShape(Shape *pS);

	Point GetTopicPos(TopicWidget *pT);
	
	bool SelectLink(LinkWidget *pl=nullptr, bool bSel=true);
	bool check_links(Point p);

	bool SelectShape(Shape *ps=nullptr, bool bSel=true);
	bool check_shapes(Point p);
	
	void check_links_and_shapes(Point p);

	bool has_multi_sel() { return (vmultisel.size()>0); }
	void clear_multi_sel() { for (auto& pt:vmultisel) pt->Select(false); vmultisel.clear(); } //NOT!!clean()!!

	virtual void MouseWheel(Point p, int z, dword d)	{ Ctrl::MouseWheel(p,z,d); } //explicitly passing event on (scrolling fails otherwise)

	Gate<TopicWidget*> WhenNewTopic; //return true if user-accepted
	Gate<LinkWidget*> WhenNewLink; //return true if user-accepted
	Gate<Shape*> WhenNewShape; //return true if db-saved-ok

	//map & link & shape handlers------------------------------------------------------
	virtual void RightDown(Point p, dword keyflags);
	virtual void RightUp(Point p, dword keyflags);
	virtual void LeftDown(Point p, dword keyflags);
	virtual void LeftUp(Point p, dword keyflags);
	virtual void LeftDouble(Point p, dword keyflags);
	virtual void MouseMove(Point p, dword keyflags);

	//topic handlers-----------------------------------------------------------
	
	void OnTopicRightDown(TopicWidget *pT, Point p, dword keyflags);
	void OnTopicRightUp(TopicWidget *pT, Point p, dword keyflags);
	void OnTopicLeftDown(TopicWidget *pT, Point p, dword keyflags);
	void OnTopicLeftUp(TopicWidget *pT, Point p, dword keyflags);
	void OnTopicLeftDouble(TopicWidget *pT, Point p, dword keyflags);
	void OnTopicMouseMove(TopicWidget *pT, Point p, dword keyflags);
	void OnTopicGotFocus(TopicWidget *pt);
	void OnTopicLostFocus(TopicWidget *pt);
	void OnTopicMoving(TopicWidget *pt);

	//linkline handlers--------------------------------------------------------
	void OnLinkDataChange(LinkWidget *pL); ///what's the purpose of this? todo?
	//void OnLinkRightDown(LinkWidget *pL, Point p, dword keyflags);
	//void OnLinkRightUp(LinkWidget *pL, Point p, dword keyflags);
	//void OnLinkLeftDown(LinkWidget *pL, Point p, dword keyflags);
	//void OnLinkLeftUp(LinkWidget *pL, Point p, dword keyflags);
	//void OnLinkLeftDouble(LinkWidget *pL, Point p, dword keyflags);

	//shape-events
	Event<bool> WhenIsDrawingChange; //switching between shape-drawing and topic/linking
	Event<Shape*> WhenShapeChanged; //flip/flate
	Event<Shape*> WhenShapeDeleted;
	Event<Shape*, Point, dword > WhenShapeRightClick;
	Event<Shape*, Point, dword > WhenShapeLeftDoubleClick;
	Event<Shape*, bool> WhenShapeHover; //bool=>mouse enter/leave
	
	//topic-events
	Event<TopicWidget*, Point, dword> WhenTopicRightClick;
	Event<TopicWidget*, Point, dword> WhenTopicLeftDoubleClick;
	Event<TopicWidget* > WhenTopicMoved;
	Event<TopicWidget*, bool> WhenTopicHover; //bool=>mouse enter/leave

	//link-events
	Event<LinkWidget*, Point, dword > WhenLinkRightClick;
	Event<LinkWidget*, Point, dword > WhenLinkLeftDoubleClick;
	Event<LinkWidget*, bool> WhenLinkHover; //bool=>mouse enter/leave

	//map-events
	Event<Point, dword> WhenMapRightClick;
	Event<Point, dword> WhenMapLeftDoubleClick;
	Event<TopicWidget*, TopicWidget*> WhenTopicLinkTopic;
	Event<TopicWidget*, Point> WhenTopicLinkNewTopic;
	Event<std::vector<TopicWidget*>> WhenMultiSelect;

};



#endif
