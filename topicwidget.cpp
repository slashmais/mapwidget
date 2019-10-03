
#include "mapwidget.h"
#include "mappics.h"

//=============================================================================
TextBoxCtrl::TextBoxCtrl() {}

void TextBoxCtrl::TBCSetText(const std::string sText, int W)
{
	int n,h,w=0,mC=sText.length();//,p=0,wc,nS
	if (mC<=0) return;
	Size tsz = GetTextSize(sText.c_str(), Draw::GetStdFont());
	std::string st, s="";
	
	h = tsz.cy;
	if (tsz.cx <= W) { w = tsz.cx; s = sText; }
	else
	{
		n=mC-1;
		s = sText.substr(0,n); s+="..";
		w = GetTextSize(s.c_str(), Draw::GetStdFont()).cx;
		while (w >= W)
		{
			n--;
			s = sText.substr(0,n); s+="..";
			w = GetTextSize(s.c_str(), Draw::GetStdFont()).cx;
		}
		Tip(sText.c_str());
	}
	SetRect(0,0,w,h);//+2);
	SetText(s.c_str());
}

//void TextBoxCtrl::
//TBCSetFieldText(StackList<std::string> &sl, int W)
//{
//	int n=sl.count(),h;
//	if (n==0) return;
//	std::string s=*(sl.at(0));
//	h = GetTextSize("Ay", Draw::GetStdFont()).cy * n;
//	for (UINT i=1;i<n;i++) { s+="\n"; s+=*(sl.at(i)); }
//	SetRect(0,0,W,h+2);
//	SetText(s.c_str());
//}



//=============================================================================
TopicWidget::TopicWidget()
{
	tbc.SetAlign(ALIGN_CENTER);
	Add(tbc);

	//fbox.SetFrame(BlackFrame());
	//fbox.SetAlign(ALIGN_LEFT);
	//Add(fbox);

	SetText("<unknown>");
	Disp=0;
	
	//slFields.clear();
	
	DefaultBg = White; ///TODO: methods to customize default & focus colors
	DefaultFg = Black;
	FocusBg = Color(0,150,200); //63,87,125);
	FocusFg = White;
	DispLockfg=Color(150, 50, 50); //reddish
	DispLockbg=Color(150, 250, 250);
	DispExpfg=Color(50,50,150); //blueish
	DispExpbg=Color(250,250,150);

	PaintFg=DefaultFg;
	PaintBg=DefaultBg;
	
	bShowFocus=true;

	bLButtonDown=false;
	bMoving = false;

	bShowPics=true;
	pic.Clear();
	
	bShowBoxed=true; //false;
	bSelected=false;
	
}

void TopicWidget::SetShowFocus(bool b)
{
	bShowFocus = b;
}

void TopicWidget::Paint(Draw &drw)
{
	Size sz = GetSize();
	int x=0, w=sz.cx-1,h=sz.cy-1;
	Color fg=PaintFg;
	Color bg=PaintBg;
	if (bSelected) { fg=FocusFg; bg=FocusBg; }
	else
	{ //disp==0(normal)/(-1)(local-lock)/(>0)(expanded)
		if (Disp<0) { fg=DispLockfg; bg=DispLockbg; }
		else if (Disp>0) { fg=DispExpfg; bg=DispExpbg; }
	}
	
	drw.DrawRect(0, 0, sz.cx, sz.cy, bg);

	if (!pic.IsEmpty()) { drw.DrawImage(0, 0, pic); x+=pic.GetWidth(); }

	Color boxcol=Color(198,198,198);
	if (bShowBoxed)
	{
		drw.DrawLine(x+2,1,x+3,1,0,boxcol);
		drw.DrawLine(x+4,0,w-4,0,0,boxcol);
		drw.DrawLine(w-3,1,w-2,1,0,boxcol);
		
		drw.DrawLine(x+2,h-1,x+3,h-1,0,boxcol);
		drw.DrawLine(x+4,h,w-4,h,0,boxcol);
		drw.DrawLine(w-3,h-1,w-2,h-1,0,boxcol);
	
		drw.DrawLine(x+1,2,x+1,3,0,boxcol);
		drw.DrawLine(x+0,4,x+0,h-4,0,boxcol);
		drw.DrawLine(x+1,h-2,x+1,h-3,0,boxcol);
	
		drw.DrawLine(w-1,2,w-1,3,0,boxcol);
		drw.DrawLine(w,4,w,h-4,0,boxcol);
		drw.DrawLine(w-1,h-3,w-1,h-2,0,boxcol);
	}

	tbc.SetInk(fg);

}

void TopicWidget::SetTypePic(Image img)
{
//	typepic = img;
//	CalcRect();
//	Refresh();
}
void TopicWidget::ResetTypePic()
{
//	typepic.Clear();
//	CalcRect();
//	Refresh();
}

Point TopicWidget::GetLinkPointTo()
{
	Point P;
	Rect r = GetRect();

	//incoming l inks to left of topic
//	P=r.CenterLeft(); //((szText.cy/2) + r.top);
//	P.x--;
//	return P;

	//incoming l inks to center of topic
	P=r.CenterPoint();
	return P;

}

Point TopicWidget::GetLinkPointFrom()
{ //outgoing l inks from right of topic
	Point P;
	Rect r = GetRect();
	//Size szText = tbc.GetSize();
	//int w=0;

///PIC MUST BE SUPPLIED BY USER-APP...
//	pic.Clear();
//	if (bShowPics)
//	{
//		switch (TT)
//		{
//			case TT_GENERIC: { pic = TopicPics::genericpic(); } break;
//			case TT_MAIN: { pic = TopicPics::mainpic(); } break;
//			case TT_IDEA: { pic = TopicPics::ideapic(); } break;
//			case TT_TASK: { pic = TopicPics::taskpic(); } break;
//			case TT_DATA: { pic = TopicPics::datapic(); } break;
//			case TT_NOTE: { pic = TopicPics::infonotepic(); } break;
//			case TT_THING: { pic = TopicPics::thingpic(); } break;
//			case TT_USERINTERFACE: { pic = TopicPics::userinterfacepic(); } break;
//			case TT_OBJECTIVE: { pic = TopicPics::objectivepic(); } break;
//			case TT_PROJECT: { pic = TopicPics::projectpic(); } break;
//			//case TT_EVALUATE: { pic = TopicPics::evaluatepic(); } break;
//			case TT_PROCESS: { pic = TopicPics::processpic(); } break;
//			case TT_ROLE: { pic = TopicPics::rolepic(); } break;
//			case TT_RULE: { pic = TopicPics::rulepic(); } break;
//			case TT_WARN: { pic = TopicPics::warnpic(); } break;
//			case TT_ISSUE:
//			default: { pic = TopicPics::genericpic(); } break;
//		}
//	}

	//if (!pic.IsEmpty()) { w=pic.GetWidth(); } //ph=pic.GetHeight(); }
//	w+=(szText.cx + 1);
//	P.x=(r.left+w);
	//P.y=((szText.cy/2) + r.top);
	P=r.CenterRight();
	P.x++;
	return P;
}

void TopicWidget::CalcRect()
{
	Rect r = GetRect();
	Size szt = tbc.GetSize();
	int w, h, ph=1, x=1;
	
///PIC MUST BE SUPPLIED BY USER-APP...2
//	pic.Clear();
//	if (bShowPics)
//	{
//		switch (TT)
//		{
//			case TT_GENERIC: { pic = TopicPics::genericpic(); } break;
//			case TT_MAIN: { pic = TopicPics::mainpic(); } break;
//			case TT_IDEA: { pic = TopicPics::ideapic(); } break;
//			case TT_TASK: { pic = TopicPics::taskpic(); } break;
//			case TT_DATA: { pic = TopicPics::datapic(); } break;
//			case TT_NOTE: { pic = TopicPics::infonotepic(); } break;
//			case TT_THING: { pic = TopicPics::thingpic(); } break;
//			case TT_USERINTERFACE: { pic = TopicPics::userinterfacepic(); } break;
//			case TT_OBJECTIVE: { pic = TopicPics::objectivepic(); } break;
//			case TT_PROJECT: { pic = TopicPics::projectpic(); } break;
//			//case TT_EVALUATE: { pic = TopicPics::evaluatepic(); } break;
//			case TT_PROCESS: { pic = TopicPics::processpic(); } break;
//			case TT_ROLE: { pic = TopicPics::rolepic(); } break;
//			case TT_RULE: { pic = TopicPics::rulepic(); } break;
//			case TT_WARN: { pic = TopicPics::warnpic(); } break;
//			case TT_ISSUE:
//			default: { pic = TopicPics::genericpic(); } break;
//		}
//	}
	
	if (!pic.IsEmpty()) { x = pic.GetWidth(); ph=pic.GetHeight(); }
	
	w = szt.cx + x + 2;
	h = (ph>szt.cy)?ph:szt.cy+2;
	SetRect(r.left, r.top, w, (h+2));
	tbc.SetRect(x+1,3, szt.cx, szt.cy);
}

void TopicWidget::SetText(const std::string sCap)
{
	sText = sCap;
	tbc.TBCSetText(sCap);
	CalcRect();
}

void TopicWidget::SetDisp(int disp)
{
	Disp=disp;
	if (Disp<0) pic=GetMapPic(PICINFO);
	Refresh();
}

//void TopicWidget::
//AddField(const std::string sF)
//{
//	slFields.put(ne w std::string(sF));
//	fbox.TBCSetFieldText(slFields, tbc.GetSize().cy);
//	SetText(sText);
//}

//void TopicWidget::
//RemoveField(const std::string sF)
//{
//	bool b=false;
//	UINT i=0;
//	while (!b && (i<slFields.count())) if (slFields.at(i)->compare(sF)==0) b=true; else i++;		
//	if (b)
//	{
//		slFields.destroy(i);
//		fbox.TBCSetFieldText(slFields, tbc.GetSize().cy);
//		SetText(sText);
//	}
//}

const std::string TopicWidget::
GetText()
{
	return sText;
}

void TopicWidget::SetToolTip(const std::string &sTip)
{
	tbc.Tip(sTip.c_str());
}

void TopicWidget::DrawFocus(bool bShow)
{
	if (bShowFocus)
	{
		if (bShow) { PaintFg=FocusFg; PaintBg=FocusBg; }
		else { PaintFg=DefaultFg; PaintBg=DefaultBg; }
		Refresh();
	}
}

void TopicWidget::MouseEnter(Point p, dword keyflags)
{
	tbc.MouseEnter(p, keyflags);
	DrawFocus();
	WhenGotFocus(this);
}

void TopicWidget::MouseLeave()
{
	tbc.MouseLeave();
	DrawFocus(false);
	WhenLostFocus(this);
}

void TopicWidget::RightDown(Point p, dword keyflags)
{
	tbc.RightDown(p, keyflags);
	WhenRightDown(this, p, keyflags);
}

void TopicWidget::RightUp(Point p, dword keyflags)
{
	tbc.RightUp(p, keyflags);
	WhenRightUp(this, p, keyflags);
}

void TopicWidget::LeftDown(Point p, dword keyflags)
{
	tbc.LeftDown(p, keyflags);
	bLButtonDown=true;
	MP=p;
	GetParent()->Add(*this); //top of zorder
}

void TopicWidget::LeftUp(Point p, dword keyflags)
{
	tbc.LeftUp(p, keyflags);
	bLButtonDown=false;
	bMoving=false;
	WhenLeftUp(this, p, keyflags);
}

void TopicWidget::LeftDouble(Point p, dword keyflags)
{
	tbc.LeftDouble(p, keyflags);
	WhenLeftDouble(this, p, keyflags);
}

void TopicWidget::MouseMove(Point p, dword keyflags)
{
	tbc.MouseMove(p, keyflags);
	bMoving=bLButtonDown;
	WhenMouseMove(this, p, keyflags);
}

void TopicWidget::SetTopicType(TOPICTYPE tt)
{
	TT=tt;
	CalcRect();
	Refresh();
}

TOPICTYPE TopicWidget::GetTopicType()
{
	return TT;
}



