
#include "mapwidget.h"
#include <utilfuncs/utilfuncs.h>
#include "fontsel.h"
#include <limits.h>


//-------------------------------------------------------------------------------------------------
const std::string SDELIM{";"}; //data-delimiter - see .ShapeData(..)


//-------------------------------------------------------------------------------------------------ShapePoint
void ShapePoint::clear()	{ Point::Clear(); fg=Black(); bg=White(); width=1; }
ShapePoint::~ShapePoint()		{}
ShapePoint::ShapePoint()		{ clear(); }
ShapePoint::ShapePoint(Point p, Color f, Color b, int w)	{ clear(); x=p.x; y=p.y; fg=f; bg=b; width=w; }
ShapePoint::ShapePoint(const std::string &spdata)			{ clear(); SPData(spdata); }
ShapePoint::ShapePoint(const ShapePoint &P)						{ *this=P; }
ShapePoint::ShapePoint(const Point &p)						{ clear(); x=p.x; y=p.y; }
ShapePoint& ShapePoint::operator=(const ShapePoint &P)	{ x=P.x; y=P.y; fg=P.fg, bg=P.bg; width=P.width; return *this; }
ShapePoint& ShapePoint::operator-=(const ShapePoint &P)	{ x-=P.x; y-=P.y; return *this; }
ShapePoint& ShapePoint::operator+=(const ShapePoint &P)	{ x+=P.x; y+=P.y; return *this; }
ShapePoint& ShapePoint::operator=(const Point &p)	{ x=p.x; y=p.y; return *this; }
ShapePoint& ShapePoint::operator-=(const Point &p)	{ x-=p.x; y-=p.y; return *this; }
ShapePoint& ShapePoint::operator+=(const Point &p)	{ x+=p.x; y+=p.y; return *this; }
std::string ShapePoint::SPData()	{ return spf("spt ", fg.GetRaw(), " ", bg.GetRaw(), " ", width, " ", x, " ", y); }

void ShapePoint::SPData(const std::string &sd)
{
	if (!sieqs("spt", sd.substr(0,3))) return;
	std::vector<std::string> v{};
	splitslist(sd, ' ', v, false);
	if (v.size()!=6) return;
	//dword c;
	fg=Color::FromRaw(stot<dword>(v[1]));
	bg=Color::FromRaw(stot<dword>(v[2]));
	width=stot<int>(v[3]);
	x=stot<int>(v[4]);
	y=stot<int>(v[5]);
}

//-----------------------------------------------
ShapePoint operator-(const ShapePoint &l, const ShapePoint &r) { ShapePoint t(l); t.x-=r.x; t.y-=r.y; return t; }
ShapePoint operator+(const ShapePoint &l, const ShapePoint &r) { ShapePoint t(l); t.x+=r.x; t.y+=r.y; return t; }
ShapePoint operator-(const ShapePoint &l, const Point &r) { ShapePoint t(l); t.x-=r.x; t.y-=r.y; return t; }
ShapePoint operator+(const ShapePoint &l, const Point &r) { ShapePoint t(l); t.x+=r.x; t.y+=r.y; return t; }

ShapePoint flip_point(ShapePoint P, int lr, int wh)
{
	ShapePoint R(P);
	if (lr==FLIP_LEFT) { R.x=P.y; R.y=(wh-P.x); } //landscape to portrait
	else if (lr==FLIP_RIGHT) { R.x=(wh-P.y); R.y=P.x; }
	return R;
}

ShapePoint flate_point(ShapePoint P, int cw, int ch, int nw, int nh)
{
	ShapePoint R(P);
	R.x=((P.x*nw)/cw);
	R.y=((P.y*nh)/ch);
	return R;
}

//-------------------------------------------------------------------------------------------------Shapes
Shape* Shapes::pop() { Shape *p=nullptr; if (!empty()) { p=back(); pop_back(); } return p; }

Shape* Shapes::add_restore(const std::string &shapedata)
{
	Shape *p=nullptr;
	std::string st=lcase(shapedata.substr(0,3));
	if (seqs(st, "ske")) p=new Sketch;
	else if (seqs(st, "lin")) p=new Line;
	else if (seqs(st, "box")) p=new Box;
	else if (seqs(st, "ovl")) p=new Oval;
	else if (seqs(st, "jot")) p=new Jot;
	if (p) { p->ShapeData(shapedata); push(p); } //else ignore? make a note somewhere?
	return p;
}

Shape* Shapes::frompoint(Point p)
{
	for (auto ps:(*this))
	{
		if (ps->isa()==ISA_LIN) { if (((Line*)ps)->PoLL(p)) return ps; }
		else
		{
			int x1,x2,y1,y2;
			if (ps->f.x<ps->t.x) { x1=ps->f.x; x2=ps->t.x; } else { x1=ps->t.x; x2=ps->f.x; }
			if (ps->f.y<ps->t.y) { y1=ps->f.y; y2=ps->t.y; } else { y1=ps->t.y; y2=ps->f.y; }
			if ((p.x>=x1)&&(p.x<=x2)&&(p.y>=y1)&&(p.y<=y2)) return ps;
		}
	}
	return nullptr;
}

//-------------------------------------------------------------------------------------------------Sketch
void Sketch::clear() { f.x=f.y=INT_MAX; t.x=t.y=INT_MIN; points.clear(); bSelected=false; }
Sketch::~Sketch() { clear(); }
Sketch::Sketch() { clear(); }
Sketch::Sketch(const Sketch &S)	{ *this=S; }
Sketch& Sketch::operator=(const Sketch &S)	{ points=S.points; f=S.f; t=S.t; return *this; }
void Sketch::AddPoint(ShapePoint p) { points.Add(p); }
void Sketch::DeletePoint() { points.Remove(); }
void Sketch::flip(int lr, int wh) { for (auto& p:points) { p=flip_point(p, lr, wh); }}
void Sketch::flate(int cw, int ch, int nw, int nh) { for (auto& p:points) { p=flate_point(p, cw, ch, nw, nh); }}

void Sketch::Render(Draw &drw)
{
	if (points.empty()) return;
	auto it=points.begin();
	ShapePoint p1=(*it);
	f=t=p1;
	it++;
	while (it!=points.end())
	{
		ShapePoint p2=(*it);
		if (p2.x<f.x) { f.x=p2.x; } if (p2.y<f.y) { f.y=p2.y; }
		if (p2.x>t.x) { t.x=p2.x; } if (p2.y>t.y) { t.y=p2.y; }
		drw.DrawLine((Point)p1, (Point)p2, p1.width, p1.fg);
		p1=p2;
		it++;
	}
	if (bSelected)
	{
		Color selcolor=Color(0,150,200); //LtCyan();
		int x1,x2,y1,y2;
		if (f.x<t.x) { x1=f.x; x2=t.x; } else { x1=t.x; x2=f.x; }
		if (f.y<t.y) { y1=f.y; y2=t.y; } else { y1=t.y; y2=f.y; }
		x1-=3; y1-=3; x2+=3; y2+=3;
		drw.DrawLine(x1, y1, x2, y1, 3, selcolor);
		drw.DrawLine(x1, y1, x1, y2, 3, selcolor);
		drw.DrawLine(x1, y2, x2, y2, 3, selcolor);
		drw.DrawLine(x2, y1, x2, y2, 3, selcolor);
	}
}
	
std::string Sketch::ShapeData()
{
	std::string s{};
	if (!points.empty())
	{
		s=spf("ske", SDELIM);
		for (auto p:points) { s+=p.SPData(); s+=SDELIM; }
		TRIM(s, SDELIM.c_str());
	}
	return s;
}
	
void Sketch::ShapeData(const std::string &sd)
{
	std::vector<std::string> v{};
	splitsslist(sd, SDELIM, v, false);
	if (v.size()<2) return;
	if (!sieqs("ske", v[0])) return;
	v.erase(v.begin());
	for (auto s:v) { points.Add(ShapePoint(s)); }
}

void Sketch::process_MouseMove(ShapePoint sp, dword keyflags)	{ AddPoint(sp); }
void Sketch::process_LeftDown(ShapePoint sp, dword keyflags)	{ AddPoint(sp); }
bool Sketch::process_LeftUp(ShapePoint sp, dword keyflags)		{ AddPoint(sp); return true; }

//-------------------------------------------------------------------------------------------------Line
void Line::clear() { f.clear(); t.clear(); bSelected=false; }
Line::~Line() {}
Line::Line() { clear(); }
Line::Line(const Line &L) { *this=L; }
Line& Line::operator=(const Line &L) { f=L.f; t=L.t; return *this; }
void Line::flip(int lr, int wh) { f=flip_point(f, lr, wh); t=flip_point(t, lr, wh); }
void Line::flate(int cw, int ch, int nw, int nh) { f=flate_point(f, cw, ch, nw, nh); t=flate_point(t, cw, ch, nw, nh); }

void Line::Render(Draw &drw)
{
	if (bSelected)
	{
		Color selcolor=Color(0,150,200);
		drw.DrawLine((Point)f, (Point)t, 3, selcolor);
		drw.DrawLine((Point)f, (Point)t, 1, Black());
	}
	else drw.DrawLine((Point)f, (Point)t, f.width, f.fg);
}
std::string Line::ShapeData() { return spf("lin", SDELIM, f.SPData(), SDELIM, t.SPData()); }
	
void Line::ShapeData(const std::string &sd)
{
	std::vector<std::string> v{};
	splitsslist(sd, SDELIM, v, false);
	if (v.size()<3) return;
	if (!sieqs("lin", v[0])) return;
	f=ShapePoint(v[1]);
	t=ShapePoint(v[2]);
}

void Line::process_MouseMove(ShapePoint sp, dword keyflags)	{ t=sp;}//(sp-f); }
void Line::process_LeftDown(ShapePoint sp, dword keyflags)	{ f=sp; t=sp; }//(sp-f); }
bool Line::process_LeftUp(ShapePoint sp, dword keyflags)	{ t=sp; /*(sp-f);*/ return true; }

bool Line::PoLL(Point p)
{
	bool b=false;
	auto dot=[](Pointf a, Pointf b)->double{ return ((a.x*b.x)+(a.y*b.y)); };
	auto len=[dot](Pointf a)->double{ return (sqrt(dot(a, a))); };
	auto unit=[len](Pointf a)->Pointf{ Pointf r=a; r/=len(a); return r; };
	Pointf P, F, T;
	P=p; F=f; T=t;
	P-=F; T-=F; //locate to F
	if (len(P)<len(T))
	{
		double lt=dot(P, unit(T));
		if (lt>0)
		{
			double d=sqrt(dot(P, P)-(lt*lt)); //length of perpendicular
			b=(d<=5.0); //within ~5 pixels?
		}
	}
	return b;
}

//-------------------------------------------------------------------------------------------------Box
void Box::clear() { f.clear(); t.clear(); bSelected=false; }
Box::~Box() {}
Box::Box() { clear(); }
Box::Box(const Box &B) { *this=B; }
Box& Box::operator=(const Box &B) { f=B.f; t=B.t; return *this; }
void Box::flip(int lr, int wh) { f=flip_point(f, lr, wh); t=flip_point(t, lr, wh); }
void Box::flate(int cw, int ch, int nw, int nh) { f=flate_point(f, cw, ch, nw, nh); t=flate_point(t, cw, ch, nw, nh); }

void Box::Render(Draw &drw)
{
	ShapePoint T=t;//(t+f);
	int x1,x2,y1,y2;
	if (f.x<T.x) { x1=f.x; x2=T.x; } else { x1=T.x; x2=f.x; }
	if (f.y<T.y) { y1=f.y; y2=T.y; } else { y1=T.y; y2=f.y; }
	drw.DrawRect(x1, y1, x2-x1+1, y2-y1+1, f.bg);
	drw.DrawLine(x1, y1, x2, y1, f.width, f.fg);
	drw.DrawLine(x1, y1, x1, y2, f.width, f.fg);
	drw.DrawLine(x1, y2, x2, y2, f.width, f.fg);
	drw.DrawLine(x2, y1, x2, y2, f.width, f.fg);
	if (bSelected)
	{
		Color selcolor=Color(0,150,200); //LtCyan();
		int x1,x2,y1,y2;
		if (f.x<t.x) { x1=f.x; x2=t.x; } else { x1=t.x; x2=f.x; }
		if (f.y<t.y) { y1=f.y; y2=t.y; } else { y1=t.y; y2=f.y; }
		x1-=3; y1-=3; x2+=3; y2+=3;
		drw.DrawLine(x1, y1, x2, y1, 3, selcolor);
		drw.DrawLine(x1, y1, x1, y2, 3, selcolor);
		drw.DrawLine(x1, y2, x2, y2, 3, selcolor);
		drw.DrawLine(x2, y1, x2, y2, 3, selcolor);
	}
}

std::string Box::ShapeData() { return spf("box", SDELIM, f.SPData(), SDELIM, t.SPData()); }
	
void Box::ShapeData(const std::string &sd)
{
	std::vector<std::string> v{};
	splitsslist(sd, SDELIM, v, false);
	if (v.size()<3) return;
	if (!sieqs("box", v[0])) return;
	f=ShapePoint(v[1]);
	t=ShapePoint(v[2]);
}

void Box::process_MouseMove(ShapePoint sp, dword keyflags)	{ t=sp; }//(sp-f); }
void Box::process_LeftDown(ShapePoint sp, dword keyflags)	{ f=sp; t=sp; }//(sp-f); }
bool Box::process_LeftUp(ShapePoint sp, dword keyflags)		{ t=sp; /*(sp-f);*/ return true; }

//-------------------------------------------------------------------------------------------------Oval
void Oval::clear() { f.clear(); t.clear(); bSelected=false; }
Oval::~Oval() {}
Oval::Oval() { clear(); }
Oval::Oval(const Oval &O) { *this=O; }
Oval& Oval::operator=(const Oval &O) { f=O.f; t=O.t; return *this; }
void Oval::flip(int lr, int wh) { f=flip_point(f, lr, wh); t=flip_point(t, lr, wh); }
void Oval::flate(int cw, int ch, int nw, int nh) { f=flate_point(f, cw, ch, nw, nh); t=flate_point(t, cw, ch, nw, nh); }

void Oval::Render(Draw &drw)
{
	ShapePoint T=t;//(t+f);
	int x1,x2,y1,y2;
	if (f.x<T.x) { x1=f.x; x2=T.x; } else { x1=T.x; x2=f.x; }
	if (f.y<T.y) { y1=f.y; y2=T.y; } else { y1=T.y; y2=f.y; }
	drw.DrawEllipse(x1, y1, x2-x1+1, y2-y1+1, f.bg, f.width, f.fg);
	if (bSelected)
	{
		Color selcolor=Color(0,150,200); //LtCyan();
		int x1,x2,y1,y2;
		if (f.x<t.x) { x1=f.x; x2=t.x; } else { x1=t.x; x2=f.x; }
		if (f.y<t.y) { y1=f.y; y2=t.y; } else { y1=t.y; y2=f.y; }
		x1-=3; y1-=3; x2+=3; y2+=3;
		drw.DrawLine(x1, y1, x2, y1, 3, selcolor);
		drw.DrawLine(x1, y1, x1, y2, 3, selcolor);
		drw.DrawLine(x1, y2, x2, y2, 3, selcolor);
		drw.DrawLine(x2, y1, x2, y2, 3, selcolor);
	}
}

std::string Oval::ShapeData() { return spf("ovl", SDELIM, f.SPData(), SDELIM, t.SPData()); }
	
void Oval::ShapeData(const std::string &sd)
{
	std::vector<std::string> v{};
	splitsslist(sd, SDELIM, v, false);
	if (v.size()<3) return;
	if (!sieqs("ovl", v[0])) return;
	f=ShapePoint(v[1]);
	t=ShapePoint(v[2]);
}

void Oval::process_MouseMove(ShapePoint sp, dword keyflags)	{ t=sp; }//(sp-f); }
void Oval::process_LeftDown(ShapePoint sp, dword keyflags)	{ f=sp; t=sp; }//(sp-f); }
bool Oval::process_LeftUp(ShapePoint sp, dword keyflags)	{ t=sp; /*(sp-f);*/ return true; }

//-------------------------------------------------------------------------------------------------JotDlg
struct JotDlg : public TopWindow
{
	using CLASSNAME=JotDlg;
	bool bOK{false};
	DocEdit eb;
	Button btnFont;
	Button btnOK;
	Button btnCancel;
	Font fnt;
	void SetFont(Font f) { fnt=f; eb.SetFont(fnt); }
	virtual ~JotDlg() {}
	JotDlg()
	{
		Title("Jot ..");
		SetRect(0,0,400,300);
		Sizeable();
		Add(eb.HSizePosZ().VSizePosZ(0,44));
		Add(btnFont.SetLabel(t_("Font..")).LeftPosZ(12, 60).BottomPosZ(12, 20));
		Add(btnOK.SetLabel(t_("OK")).RightPosZ(76, 60).BottomPosZ(12, 20));
		Add(btnCancel.SetLabel(t_("Cancel")).RightPosZ(8, 60).BottomPosZ(12, 20));
		bOK=false;
		btnFont.WhenAction << [&]{ fnt=SelectFont(fnt); eb.SetFont(fnt); };
		btnOK.WhenAction << [&]{ bOK=true; Close(); };
		btnCancel.WhenAction << [&]{ bOK=false; Close(); };
	}
};

//-------------------------------------------------------------------------------------------------Jot
void Jot::clear()	{ f.x=f.y=INT_MAX; t.x=t.y=INT_MIN; font=StdFont(); text.clear(); bSelected=false; }
Jot::~Jot() { clear(); }
Jot::Jot() { clear(); }
Jot::Jot(const Jot &J)	{ *this=J; }
Jot& Jot::operator=(const Jot &J)	{ f=J.f; t=J.t; font=J.font; text=J.text; return *this; }
void Jot::flip(int lr, int wh) { f=flip_point(f, lr, wh); } ///todo t as well...
void Jot::flate(int cw, int ch, int nw, int nh)	{ f=flate_point(f, cw, ch, nw, nh); }

void Jot::Render(Draw &drw)
{
	std::vector<std::string> v{};
	t=f;
	splitslist(text, '\n', v);
	for (size_t i=0; i<v.size(); i++)
	{
		Size wh=GetTextSize(v[i].c_str(), font);
		if (t.x<(f.x+wh.cx)) t.x=(f.x+wh.cx);
		drw.DrawRect(f.x, t.y, wh.cx, wh.cy, f.bg);
		drw.DrawText(f.x, t.y, v[i].c_str(), font, f.fg);
		t.y+=wh.cy; //+padding);
	}
	if (bSelected)
	{
		Color selcolor=Color(0,150,200); //LtCyan();
		int x1,x2,y1,y2;
		if (f.x<t.x) { x1=f.x; x2=t.x; } else { x1=t.x; x2=f.x; }
		if (f.y<t.y) { y1=f.y; y2=t.y; } else { y1=t.y; y2=f.y; }
		x1-=3; y1-=3; x2+=3; y2+=3;
		drw.DrawLine(x1, y1, x2, y1, 3, selcolor);
		drw.DrawLine(x1, y1, x1, y2, 3, selcolor);
		drw.DrawLine(x1, y2, x2, y2, 3, selcolor);
		drw.DrawLine(x2, y1, x2, y2, 3, selcolor);
	}
}

std::string Jot::ShapeData()
{
	return spf("jot", SDELIM, f.SPData(), SDELIM, font.GetFaceName().ToStd(), SDELIM, font.GetHeight(), SDELIM, text);
}

void Jot::ShapeData(const std::string &sd)
{
	std::vector<std::string> v{};
	splitsslist(sd, SDELIM, v, false);
	if (v.size()<5) return;
	if (!sieqs("jot", v[0])) return;
	f=ShapePoint(v[1]);
	font=Font(Font::FindFaceNameIndex(v[2].c_str()), stot<int>(v[3]));
	size_t i=4;
	text=v[i]; i++; while (i<v.size()) { text+=";"; text+=v[i]; i++; }
}

void Jot::process_MouseMove(ShapePoint sp, dword keyflags)	{}
void Jot::process_LeftDown(ShapePoint sp, dword keyflags)	{ f=sp; }

bool Jot::process_LeftUp(ShapePoint sp, dword keyflags)
{
	f=sp;
	JotDlg dlg;
	dlg.SetFont(font);
	dlg.Execute();
	if (dlg.bOK) { font=dlg.fnt; text=dlg.eb.GetData().ToString().ToStd(); return true; }
	return false;
}

bool Jot::Edit()
{
	JotDlg dlg;
	dlg.SetFont(font);
	dlg.eb.SetData(text.c_str());
	dlg.Execute();
	if (dlg.bOK)
	{
		text=dlg.eb.GetData().ToString().ToStd();
		font=dlg.fnt;
		return true;
	}
	return false;
}


