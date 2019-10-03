

#include <cmath>
#include "mapwidget.h"

LinkWidget::LinkWidget()
{
	linecolor = LtGray();
	linewidth = 1;
	pTF=pTT=nullptr;
	LT=LT_ASSOC;
	name="<new link>";
	selcolor=Color(0,150,200); //LtCyan();
	selwidth=3;
}

LinkWidget::~LinkWidget() {}

void LinkWidget::DoCallback() { if (WhenLinkDataChange) WhenLinkDataChange(this); }

void LinkWidget::SetLinkType(LINKTYPE lt)
{
	LT=lt;
	DoCallback();
}

LINKTYPE LinkWidget::GetLinkType()
{
	return LT;
}

void LinkWidget::SetLineWidth(int n)
{
	linewidth=((n<1)||(n>20))?1:n;
	DoCallback();
}

int LinkWidget::GetLineWidth()
{
	return linewidth;
}

void LinkWidget::SetLineColor(Color c)
{
	linecolor=c;
	DoCallback();
}

Color LinkWidget::GetLineColor()
{
	return linecolor;
}

bool LinkWidget::PoLL(Point P)
{
	bool b=false;
	auto dot=[](Pointf a, Pointf b)->double{ return ((a.x*b.x)+(a.y*b.y)); };
	auto len=[dot](Pointf a)->double{ return (sqrt(dot(a, a))); };
	auto unit=[len](Pointf a)->Pointf{ Pointf r=a; r/=len(a); return r; };
	
	if (pTF&&pTT)
	{
		Pointf p=P, f=pTF->GetLinkPointFrom(), t=pTT->GetLinkPointTo();
		p-=f; t-=f; //locate to f
		if (len(p)<len(t))
		{
			double lt=dot(p, unit(t));
			if (lt>0)
			{
				double d=sqrt(dot(p, p)-(lt*lt)); //length of perpendicular
				b=(d<=5.0); //within ~5 pixels?
			}
		}
	}
	return b;
}

void LinkWidget::SelectLine(bool b)
{
	if (b) { oldcolor=linecolor; oldwidth=linewidth; linecolor=selcolor; linewidth=selwidth; }
	else { linecolor=oldcolor; linewidth=oldwidth; }
	DoCallback();
}


