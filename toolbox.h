#ifndef _modeler_toolbox_h_
#define _modeler_toolbox_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#include <string>
#include <vector>


//struct Panel : public Ctrl
//{
//	using CLASSNAME=Panel;
//	Panel() {} // SetFrame(ThinOutsetFrame()); } //?
//	virtual ~Panel(){}
//	virtual void Paint(Draw &drw) { drw.DrawRect(GetSize(), SColorFace()); } //if needed...
//
//};

struct ToolPanelItem : public Ctrl //because: separator!
{
	using CLASSNAME=ToolPanelItem;
	bool bselected{false};
	virtual ~ToolPanelItem() {}
};

struct ToolPanel;

struct ClickBox : public ToolPanelItem
{
	using CLASSNAME=ClickBox;

	Image pic;
	Image offpic;
	bool bldn;
	bool boxme;
	String tiptext;
	bool fgcolortool;
	bool bgcolortool;
	Color c_fg; //pfillc;
	Color c_bg; //pfillc;
	//Image prevcursor;
	
	Event<ClickBox*> WhenClick;
	
	virtual~ClickBox(){ }
	ClickBox()
	{
		fgcolortool=false;
		bgcolortool=false;
		c_fg=Black();
		c_bg=White();
		
		bselected=boxme=bldn=false;
		pic=Image::Wait();
		tiptext="<not initialized>";
		Tip(tiptext);
		NoWantFocus();
	}

	virtual void Paint(Draw &drw)
	{
		Size sz=GetSize();
		
		if (!IsEnabled()) { drw.DrawImage(sz, offpic); return; }
		
		if (fgcolortool)
		{
			for (int j=0; j<20; j+=5)
			{
				for (int i=0; i<20; i+=5)
				{
					if (((i+j)%2)==1) drw.DrawRect(i, j, 5, 5, LtGray());
					else drw.DrawRect(i, j, 5, 5, Gray());
				}
			}
			drw.DrawRect(4, 4, 12, 12, c_fg);
			return;
		}
		if (bgcolortool)
		{
			drw.DrawRect(sz, c_bg);
			drw.DrawRect(6, 6, 4, 4, LtGray());
			drw.DrawRect(10, 6, 4, 4, Gray());
			drw.DrawRect(6, 10, 4, 4, Gray());
			drw.DrawRect(10, 10, 4, 4, LtGray());
			return;
		}
		
		if (bselected) drw.DrawRect(sz, Color(176,176,176)); //Color(0,150,200));
		else drw.DrawRect(sz, SColorFace());
		
		drw.DrawImage(sz, pic);
		
		Color ctl;
		Color cbr;
		int w=1;
		
		if (boxme) { w=2; ctl=cbr=Color(0,150,200); }
		else if (bselected) { ctl=Black(); cbr=White(); }
		else { ctl=White(); cbr=Black(); }
		drw.DrawLine(0, 0, sz.cx-2, 0, w, ctl);
		drw.DrawLine(0, 0, 0, sz.cy-2, w, ctl);
		drw.DrawLine(sz.cx-1, 1, sz.cx-1, sz.cy-1, w, cbr);
		drw.DrawLine(1, sz.cy-1, sz.cx-1, sz.cy-1, w, cbr);
		
	}
	
	void set_off_pic()
	{
		offpic=pic;
		ImageBuffer ib(offpic);
		int H=pic.GetHeight();
		int W=pic.GetWidth();
		for (int y=0; y<H; y++)
		{
			RGBA *p=ib[y];
			for (int x=0; x<W; x++)
			{
				if (!y||!x||(y==(H-1))||(x==(W-1))) p[x]=Gray();
				else if (((x+y)%2)==0) p[x]=LtGray();
			}
		}
		offpic=ib;
	}
	
	ClickBox& Picture(Image img)						{ pic=img; set_off_pic(); return *this; }
	ClickBox& TipText(const String &S)					{ tiptext=S; Tip(tiptext); return *this; }
	ClickBox& ClickCallback(Event<ClickBox*> cb)		{ WhenClick=cb; return *this; }
	
	virtual void LeftDown(Point p, dword kf)			{ bldn=true; }
	virtual void LostFocus()							{ bldn=false; }
	virtual void LeftUp(Point p, dword kf) { if (bldn)	{ bselected=true; Refresh(); if (WhenClick) WhenClick(this); }}
	virtual void MouseEnter(Point p, dword keyflags)	{ boxme=true; Refresh(); }
	virtual void MouseLeave()							{ boxme=false; Refresh(); }
	
//	virtual void MouseEnter(Point p, dword keyflags) { prevcursor=OverrideCursor(GetMapPic(PICTBCURSOR)); }
//	virtual void MouseLeave() { OverrideCursor(prevcursor); } --- will have to jump through a few hoops to sort this shit out!
	

	
	void Select(bool b=true)							{ bselected=b; Refresh(); }

};

struct ToolPanel : public Ctrl //Panel
{
	using CLASSNAME=ToolPanel;
	
	bool bHorz; //true=>horz else vert
	int side; //tool-item (square)side size
	Point pos; //cumulative position for next tool-item
	//Image prevcursor;
	bool bToolsEnabled;

	struct TPSpacer : public ToolPanelItem { using CLASSNAME=TPSpacer; virtual ~TPSpacer(){} };

	std::vector<ToolPanelItem*> tools;
	
	virtual ~ToolPanel() { for (auto p:tools) { RemoveChild(p); delete p; }}
	
	ToolPanel(bool horz=true)
	{
		bHorz=horz;
		side=20; //square-pic-size
		pos.Clear();
		bToolsEnabled=true;
	}

//	virtual void MouseEnter(Point p, dword keyflags) { prevcursor=OverrideCursor(Image::Arrow()); } //GetMapPic(PICTBCURSOR)); }
//	virtual void MouseLeave() { OverrideCursor(prevcursor); } --- will have to jump through a few hoops to sort this shit out!
	
	void inc_pos() { if (bHorz) pos.x+=side; else pos.y+=side; }
	
	virtual void Layout()
	{
		if (bHorz) { pos.x=3; pos.y=((GetSize().cy-side)/2); } else { pos.x=((GetSize().cx-side)/2); pos.y=3; }
		for (auto& p:tools)
		{
			p->SetRect(pos.x, pos.y, p->GetSize().cx, p->GetSize().cy);
			p->Refresh();
			if (bHorz) pos.x+=p->GetSize().cx; else pos.y+=p->GetSize().cy;
		}
	}

	virtual void Paint(Draw &drw) { drw.DrawRect(GetSize(), SColorFace()); }

	void AddSpacer()
	{
		int w=(side/2);
		TPSpacer *ptps=new TPSpacer;
		if (ptps) tools.push_back(ptps); else return;
		Add((*ptps).LeftPos(pos.x, w).TopPos(pos.y, w));
		if (bHorz) pos.x+=w; else pos.y+=w;
	}
	
	ClickBox* AddTool(Image img, Event<ClickBox*> cb, const std::string &desc)
	{
		ClickBox *pc=new ClickBox;
		if (pc) tools.push_back(pc); else return nullptr;
		pc->Picture(img).TipText(desc).ClickCallback(cb);
		SetRect(0, 0, pos.x, pos.y);
		Add((*pc).LeftPos(pos.x, side).TopPos(pos.y, side));
		inc_pos();
		return pc;
	}

	void EnableTools(bool b=true)
	{
		for (auto& p:tools)
		{
			p->Enable(b);
			p->Refresh();
		}
		bToolsEnabled=b;
	}
	
	bool IsToolsEnabled() { return bToolsEnabled; }
};


#endif
