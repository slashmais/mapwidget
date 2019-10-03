
#include "mapwidget.h"
#include <cmath>


MapWidget::~MapWidget()
{
	links.clear();
}

MapWidget::MapWidget()
{
	bLeftDown=bRightDown=bMoving=bLinking=false;
	bShowMargins=true;

	init_shapes();

	bIsDrawing=false;
	bSelecting=false;
	pCurTopic=nullptr;
	pLDCur=nullptr;
	pSCur=nullptr;
	links.clear();
	vmultisel.clear();
	
	SetMargins(1,1,1,1);
	SetWantFocus();
}

void MapWidget::Paint(Draw &drw)
{
	Size sz = GetSize();
	drw.DrawRect(0,0,sz.cx,sz.cy, White());
	
	ToolBoxPaint(drw); //in drawer.cpp
	
	for (auto& l:links)
	{
		drw.DrawLine(l->pTF->GetLinkPointFrom(), l->pTT->GetLinkPointTo(), l->GetLineWidth(), l->GetLineColor());
		DrawLinkText(drw, l);
	}
	if (bSelecting&&selector.isvalid()) //.f.IsZero())
	{
		Color Pink(255,10,140);
		drw.DrawLine(selector.f.x, selector.f.y, selector.f.x, selector.t.y, PEN_DASH, Pink);
		drw.DrawLine(selector.f.x, selector.f.y, selector.t.x, selector.f.y, PEN_DASH, Pink);
		drw.DrawLine(selector.f.x, selector.t.y, selector.t.x, selector.t.y, PEN_DASH, Pink);
		drw.DrawLine(selector.t.x, selector.f.y, selector.t.x, selector.t.y, PEN_DASH, Pink);
	}
	else if (bLinking&&!lldraw.f.IsZero()) drw.DrawLine(lldraw.f, lldraw.t, 1, LtRed());
	if (bShowMargins) DrawMargins(drw, sz);
}

void MapWidget::DrawMargins(Draw &drw, Size sz)
{
	Color colMargin=Color(0xff, 0xb4, 0xa0);
	int l=Lm, r=(sz.cx-Rm-1), t=Tm, b=(sz.cy-Bm-1);
	drw.DrawLine(l, t, r, t, 1, colMargin);
	drw.DrawLine(l, t, l, b, 1, colMargin);
	drw.DrawLine(r, t, r, b, 1, colMargin);
	drw.DrawLine(l, b, r, b, 1, colMargin);
}

void MapWidget::DrawLinkText(Draw &drw, LinkWidget *pL)
{
	auto darken=[](Color c)->Color{ Color r(c.GetR()/2, c.GetG()/2, c.GetB()/2); return r; };
	Pointf f=pL->pTF->GetLinkPointFrom();
	Pointf p=pL->pTT->GetLinkPointTo();
	p-=f; //relocate to origin
	int ang=(int)(-std::atan2(p.y, p.x)*572.957795); //((180.0/pi)*10.0); NB: negative angle!
	//Font font=Font(Font::FindFaceNameIndex("arial"), 10);
	Font font=Font(Font::FindFaceNameIndex("monospace"), 10);
	String S=pL->name.c_str();
	double len=sqrt(p.x*p.x + p.y*p.y);
	p/=len; //unit vector
	Size szS=GetTextSize(S, font);
	p*=((len-szS.cx)/3); //dist along line ~3rd (/2 for centering)
	p+=f; //relocate to f
	drw.DrawTextOp(int(p.x), int(p.y), ang, S.ToWString(), font, darken(pL->GetLineColor()), S.GetLength(), NULL);
}

void MapWidget::SetMargins(int L, int R, int T, int B) { Lm=L; Rm=R; Tm=T; Bm=B; }

int MapWidget::GetLmargin() { return Lm; }
int MapWidget::GetRmargin() { return Rm; }
int MapWidget::GetTmargin() { return Tm; }
int MapWidget::GetBmargin() { return Bm; }

TopicWidget* MapWidget::MakeTopic(int x, int y, const std::string &sCap, int disp, TOPICTYPE tt)
{
	TopicWidget *pt=new TopicWidget();
	if (pt)
	{
		pt->SetText(sCap.c_str());
		pt->SetDisp(disp);
		pt->SetTopicType(tt);
		pt->WhenRightDown	= THISFN(OnTopicRightDown);
		pt->WhenRightUp		= THISFN(OnTopicRightUp);
		pt->WhenLeftDown	= THISFN(OnTopicLeftDown);
		pt->WhenLeftUp		= THISFN(OnTopicLeftUp);
		pt->WhenLeftDouble	= THISFN(OnTopicLeftDouble);
		pt->WhenMouseMove	= THISFN(OnTopicMouseMove);
		pt->WhenGotFocus	= THISFN(OnTopicGotFocus);
		pt->WhenLostFocus	= THISFN(OnTopicLostFocus);
		pt->WhenMoving		= THISFN(OnTopicMoving);
		
		Rect r=pt->GetRect();
		pt->SetRect(x,y,r.right,r.bottom);
		topics.Add(pt);
		Add(*pt);
	}
	return pt;
}

void MapWidget::DeleteTopic(TopicWidget *pT)
{
	if (!pT) return;
	if (pCurTopic==pT) pCurTopic=nullptr;
	RemoveChild(pT);
	topics.Delete(pT);
}

LinkWidget* MapWidget::MakeLink(TopicWidget *ptf, TopicWidget *ptt, const std::string &desc, LINKTYPE lt)
{
	LinkWidget *pl=links.Get(ptf,ptt);
	if (!pl)
	{
		pl=new LinkWidget();
		if (pl)
		{
			pl->pTF = ptf;
			pl->pTT = ptt;
			pl->name=desc;
			pl->SetLinkType(lt);
			//Note: link-events are determined in mapwidget
			pl->WhenLinkDataChange << [&](LinkWidget *pL){ this->Refresh(); };
			if (pl) links.Add(pl);
		}
	}
	return pl;
}

void MapWidget::DeleteLink(LinkWidget *pL)
{
	if (pLDCur==pL) pLDCur=0;
	links.Delete(pL);
}

Shape* MapWidget::MakeShape(const std::string &sdata) { return shapes.add_restore(sdata); }

void MapWidget::DeleteShape(Shape *pS) {  if (pSCur==pS) pSCur=nullptr; shapes.Delete(pS); Refresh(); }

Point MapWidget::GetTopicPos(TopicWidget *pT)
{
	return pT->GetRect().TopLeft();
}

bool MapWidget::SelectLink(LinkWidget *pl, bool bSel)
{
	if (!pl) { if (pLDCur) pLDCur->SelectLine(false); pLDCur=nullptr; }
	else
	{
		if (pLDCur) pLDCur->SelectLine(false);
		pLDCur=pl;
		if (bSel) pLDCur->SelectLine();
	}
	return (pLDCur!=nullptr);
}

bool MapWidget::check_links(Point p)
{
	return SelectLink(links.frompoint(p));
}

bool MapWidget::SelectShape(Shape *ps, bool bSel)
{
	if (!ps) { if (pSCur) pSCur->SelectShape(false); pSCur=nullptr; }
	else
	{
		if (pSCur) pSCur->SelectShape(false);
		pSCur=ps;
		if (bSel) pSCur->SelectShape();
	}
	Refresh();
	return (pSCur!=nullptr);
}

bool MapWidget::check_shapes(Point p)
{
	return SelectShape(shapes.frompoint(p));
}

void MapWidget::check_links_and_shapes(Point p)
{
	if (check_links(p)) { SelectShape(); }
	else check_shapes(p);
}

//-------------------------------------------------------------------------------------------------map & link handlers
void MapWidget::RightDown(Point p, dword keyflags)
{
	//clear_multi_sel(); MENU!!!
	
	//SelectLink(links.frompoint(p));
	check_links_and_shapes(p);
	
//	if (pLDCur) WhenLinkRightDown(pLDCur, p, keyflags);

}

void MapWidget::RightUp(Point p, dword keyflags)
{
	if (bLinking)
	{
		TopicWidget *pT=pCurTopic;

		bRightDown=bLinking=false;
		lldraw.f.Clear();
		pCurTopic=nullptr;

		TopicWidget *pN=MakeTopic(p.x, p.y);
		if (WhenNewTopic) { if (!WhenNewTopic(pN)) { delete pN; pN=nullptr; }}
		if (pN)
		{
			LinkWidget *pL=MakeLink(pT, pN);
			if (WhenNewLink) { if (!WhenNewLink(pL)) { DeleteLink(pL); DeleteTopic(pN); }}
		}
		Refresh();
	}
	else
	{
		//SelectLink(links.frompoint(p));
		check_links_and_shapes(p);
		
		if (pLDCur) WhenLinkRightClick(pLDCur, p, keyflags);
		else if (pSCur) WhenShapeRightClick(pSCur, p, keyflags);
		else WhenMapRightClick(p, keyflags);
	}
}

void MapWidget::LeftDown(Point p, dword keyflags)
{
	if (bSelecting)
	{
		selector.f=selector.t=p;
		clear_multi_sel();
	}
	else
	{
		clear_multi_sel();
		if (bIsDrawing) ToolLeftDown(p, keyflags);
		else //SelectLink(links.frompoint(p));
			check_links_and_shapes(p);

	}
}

void MapWidget::LeftUp(Point p, dword keyflags)
{
	if (bSelecting)
	{
		bSelecting=false;
		selector.t=p;
		Rect R(selector.f, selector.t);
		clear_multi_sel();
		for (auto& pt:topics) { if (R.Contains(pt->GetRect())) vmultisel.push_back(pt); }
		if ((vmultisel.size())&&WhenMultiSelect) WhenMultiSelect(vmultisel);
		SetResetTool();
		OverrideCursor(Image::Arrow());
		Refresh();
	}
	else
	{
		if (bIsDrawing) ToolLeftUp(p, keyflags);
		else //SelectLink(links.frompoint(p));
			check_links_and_shapes(p);
		
	}
}

void MapWidget::LeftDouble(Point p, dword keyflags)
{
//	SelectLink(links.frompoint(p));
	check_links_and_shapes(p);
		
	if (pLDCur) WhenLinkLeftDoubleClick(pLDCur, p, keyflags);
	else WhenMapLeftDoubleClick(p, keyflags);
}

void MapWidget::MouseMove(Point p, dword keyflags)
{
	if (bSelecting)
	{
		selector.t=p; Refresh();
		if (selector.isvalid())
		{
			Rect R(selector.f, selector.t);
			for (auto& pt:topics) pt->Select(R.Contains(pt->GetRect()));
		}
	}
	else if (bIsDrawing) ToolMouseMove(p,keyflags);
	else if (bMoving)
	{
		Rect r=pCurTopic->GetRect();
		Point pp;
		pp.x = p.x - r.left;
		pp.y = p.y - r.top;
		pCurTopic->MouseMove(pp,keyflags);
		Refresh();
	}
	else if (bLinking) { lldraw.t=p; Refresh(); }
	else check_links_and_shapes(p);
}

//-------------------------------------------------------------------------------------------------topic handlers
void MapWidget::OnTopicRightDown(TopicWidget *pT, Point p, dword keyflags)
{
	if (!bRightDown && !bLeftDown && !bMoving && !bLinking)
	{
		bRightDown = true;
		pCurTopic=pT;
		lldraw.f=lldraw.t=pT->GetLinkPointFrom();
	}
}

void MapWidget::OnTopicRightUp(TopicWidget *pT, Point p, dword keyflags)
{
	if (!bLinking)
	{
		bRightDown = false;
		if (pT == pCurTopic) WhenTopicRightClick(pT, p, keyflags);
		else pCurTopic = nullptr;
	}
	else if (bLinking)
	{
		TopicWidget *pF=pCurTopic;
		bRightDown=bLinking=false;
		lldraw.f.Clear();
		pCurTopic=nullptr;
		//if (pF!=pT) WhenTopicLinkTopic(pF, pT);
		if (pF!=pT)
		{
			LinkWidget *pL=MakeLink(pF, pT);
			if (WhenNewLink) { if (!WhenNewLink(pL)) { DeleteLink(pL); }}
		}
		Refresh();
	}
}

void MapWidget::OnTopicLeftDown(TopicWidget *pT, Point p, dword keyflags)
{
	if (!bRightDown && !bLeftDown && !bMoving && !bLinking)
	{
		bLeftDown = true;
		pCurTopic = pT;
		mdx = p.x; mdy = p.y;
		Add(*pCurTopic); //top of zorder
	}
}

void MapWidget::OnTopicLeftUp(TopicWidget *pT, Point p, dword keyflags)
{
	bool b = bMoving;
	bLeftDown=bMoving=false;
	pCurTopic=nullptr;
	if (b)
	{
		if (has_multi_sel())
		 {
		  for (auto& pt:vmultisel)
		  {
		   WhenTopicMoved(pt);
		  }
		  }
		else WhenTopicMoved(pT);
	}
}

void MapWidget::OnTopicLeftDouble(TopicWidget *pT, Point p, dword keyflags)
{
	WhenTopicLeftDoubleClick(pT, p, keyflags);
}

void MapWidget::OnTopicMouseMove(TopicWidget *pT, Point p, dword keyflags)
{
	if ((bMoving=pT->bMoving))
	{
		int dx=(p.x-pT->MP.x), dy=(p.y-pT->MP.y);
		Rect r = pT->GetRect();
		r.left += dx;
		r.top += dy;
		r.right += dx;
		r.bottom += dy;
		pT->SetRect(r);
		
		if (has_multi_sel())
		{
			for (auto& pt:vmultisel)
			{
				if (pt!=pT)
				{
					Rect r=pt->GetRect();
					r.left += dx;
					r.top += dy;
					r.right += dx;
					r.bottom += dy;
					pt->SetRect(r);
				}
				//WhenTopicMoved(pt);
			}
		}
		//else OnTopicMoving(pT);
		Refresh();
	}
	else
	{
		if (bLinking) { Rect r=pT->GetRect(); Point pp=p; pp.x+=r.left; pp.y+=r.top; this->MouseMove(pp,keyflags); }
		if (bRightDown) { bLinking=true; }
	}
}

void MapWidget::OnTopicGotFocus(TopicWidget *pT)
{
	SelectLink();
	SelectShape();
	if (!(bMoving || bLinking)) pCurTopic = pT;
}

void MapWidget::OnTopicLostFocus(TopicWidget *pT)
{
	if (!(bMoving || bLinking)) pCurTopic=nullptr;
}

void MapWidget::OnTopicMoving(TopicWidget *pt)
{
	TopicWidget *ptmp = pCurTopic;
	bool b = (bMoving && (pCurTopic!=nullptr) && (pt==nullptr));
	pCurTopic = pt;
	bMoving = (pt != nullptr);
	Refresh();
	if (b) WhenTopicMoved(ptmp);
}

//-------------------------------------------------------------------------------------------------linkline handlers

void MapWidget::OnLinkDataChange(LinkWidget *pL) { Refresh(); }

//void MapWidget::OnLinkRightDown(LinkWidget *pL, Point p, dword keyflags)
//{
//	SelectLink(pL);
//}
//
//void MapWidget::OnLinkRightUp(LinkWidget *pL, Point p, dword keyflags)
//{
//	if (pLLCur != pL) SelectLink();
//	WhenLinkRightClick(pL,p,keyflags);
//}
//
//void MapWidget::OnLinkLeftDown(LinkWidget *pL, Point p, dword keyflags)
//{
//	SelectLink(pL);
//}
//
//void MapWidget::OnLinkLeftUp(LinkWidget *pL, Point p, dword keyflags)
//{
//	if (pLLCur != pL) SelectLink();
//}
//
//void MapWidget::OnLinkLeftDouble(LinkWidget *pL, Point p, dword keyflags)
//{
//	SelectLink(pL);
//	WhenLinkLeftDoubleClick(pL,p,keyflags);
//}


