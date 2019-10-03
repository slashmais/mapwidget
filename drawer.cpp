
#include "mapwidget.h"
#include "mappics.h"
#include <utilfuncs/utilfuncs.h>

//#include "seeder.h" //for SPage


void MapWidget::flip_shapes(int lp, int wh)
{
	for (auto& ps:shapes)
	{
		if (lp==LP_LTOP) { ps->flip(FLIP_RIGHT, wh); if (WhenShapeChanged) WhenShapeChanged(ps); }
		else if (lp==LP_PTOL) { ps->flip(FLIP_LEFT, wh); if (WhenShapeChanged) WhenShapeChanged(ps); }
	}
}

void MapWidget::flate_shapes(int cw, int ch, int nw, int nh)
{
	for (auto& ps:shapes)
	{
		ps->flate(cw, ch, nw, nh);
		if (WhenShapeChanged) WhenShapeChanged(ps);
	}
}

void MapWidget::init_shapes()
{
	curColor=Black();
	curWidth=1;
	shapetools.clear();
	sizetools.clear();
	unset_tools();
	
	pSelectTool=toolpanel.AddTool(GetMapPic(PICSELECT), [&](ClickBox *pcb){ do_selecttool(); }, "group select");
	toolpanel.AddSpacer();
	pResetTool=toolpanel.AddTool(GetMapPic(PICARROW), [&](ClickBox *pcb){ check_reset();  pcb->Select(false); }, "toggle drawing");
	toolpanel.AddSpacer();
	set_undoredotools();
	toolpanel.AddSpacer();
	set_shapetools();
	toolpanel.AddSpacer();
	set_sizetools();
	toolpanel.AddSpacer();
	toolpanel.AddTool(GetMapPic(PICFG), [&](ClickBox *pcb){ SetCurColor(pcb); }, "select foreground color")->fgcolortool=true;
	toolpanel.AddTool(GetMapPic(PICBG), [&](ClickBox *pcb){ SetCurColor(pcb, false); }, "select background color")->bgcolortool=true;

//	toolpanel.AddSpacer();
//	toolpanel.AddTool(GetMapPic(PICCOLOR), [&](ClickBox *pcb){ SetCurColor(pcb); }, "select color")->bcolortool=true;

	//toolpanel must be added to parent/owner
	//Add( -no-dont- toolpanel.AddFrame(ThinInsetFrame()).HSizePosZ().TopPosZ(0,30)); //this is added to the mapper-window

	toolpanel.EnableTools(false);
	SetResetTool();
}

bool MapWidget::IsDrawing()		{ return bIsDrawing; }

void MapWidget::ToolBoxPaint(Draw &drw)
{
	for (auto ps:shapes) ps->Render(drw);
	if (pCurShape) pCurShape->Render(drw);
}

void MapWidget::set_undoredotools()
{
	toolpanel.AddTool(GetMapPic(PICUNDO), [&](ClickBox *pcb){ undoshape(); pcb->Select(false); }, "undoshape");
	toolpanel.AddTool(GetMapPic(PICREDO), [&](ClickBox *pcb){ redoshape(); pcb->Select(false); }, "redoshape");
}
void MapWidget::set_shapetools()
{
	//AddTool() can return nullptr - check/fix/todo? - will be memory-issue
	shapetools[toolpanel.AddTool(GetMapPic(PICPEN), THISFN(do_shapetools), "free sketching")]=TOOL_SKETCH;
	shapetools[toolpanel.AddTool(GetMapPic(PICLINE), THISFN(do_shapetools), "draw lines")]=TOOL_LINE;
	shapetools[toolpanel.AddTool(GetMapPic(PICRECT), THISFN(do_shapetools), "draw boxes")]=TOOL_BOX;
	shapetools[toolpanel.AddTool(GetMapPic(PICOVAL), THISFN(do_shapetools), "draw ovals")]=TOOL_OVAL;
	shapetools[toolpanel.AddTool(GetMapPic(PICJOT), THISFN(do_shapetools), "jot a note")]=TOOL_JOT;
	//toolpanel.AddTool(GetMapPic(PICPAINT), [](ClickBox *pcb){ PromptOK("to do"); }, "no draw");
}

void MapWidget::set_sizetools()
{
	sizetools[toolpanel.AddTool(GetMapPic(PICSIZE1), [&](ClickBox *pcb){ do_sizetools(pcb); }, "line width 1 pixel")]=1;
	sizetools[toolpanel.AddTool(GetMapPic(PICSIZE3), [&](ClickBox *pcb){ do_sizetools(pcb); }, "line width 3 pixels")]=3;
	sizetools[toolpanel.AddTool(GetMapPic(PICSIZE5), [&](ClickBox *pcb){ do_sizetools(pcb); }, "line width 5 pixels")]=5;
}

void MapWidget::do_selecttool()
{
	bSelecting=true;
	selector.f.Clear();
	selector.t.Clear();
	OverrideCursor(GetMapPic(PICSELECT));
}

void MapWidget::do_shapetools(ClickBox *pcb)
{
	SetTool(shapetools[pcb], pcb);
}

void MapWidget::do_sizetools(ClickBox *pcb)
{
	clear_toolselections(sizetools);
	pcb->Select();
	curWidth=sizetools[pcb];
}

void MapWidget::clear_toolselections(ToolList &TL)		{ for (auto& p:TL) p.first->Select(false); }

void MapWidget::save_new_shape(Shape *p)
{
	shapes.Add(p);
	if (WhenNewShape) { if (!WhenNewShape(p)) shapes.Delete(p); }
}

void MapWidget::undoshape()
{
	if (!shapes.empty())
	{
		Shape *pS=shapes.pop();
		urshapes.push(pS);
		if (WhenShapeDeleted) WhenShapeDeleted(pS);
		Refresh();
	}
}

void MapWidget::redoshape() { if (!urshapes.empty()) { save_new_shape(urshapes.pop()); Refresh(); }}

void MapWidget::SetResetTool()
{
	pSelectTool->Enable(); pSelectTool->Select(false);
	pResetTool->Enable();
	if (toolpanel.IsToolsEnabled()) { pResetTool->Picture(GetMapPic(PICARROW)); bIsDrawing=true; }
	else { pResetTool->Picture(GetMapPic(PICDRAWTOOL)); bIsDrawing=false; }
	WhenIsDrawingChange(bIsDrawing);
}

void MapWidget::check_reset()
{
	urshapes.clear();
	if (toolpanel.IsToolsEnabled()) { SetTool(TOOL_NONE); toolpanel.EnableTools(false); }
	else { toolpanel.EnableTools(); }
	SetResetTool();
	if (toolpanel.IsToolsEnabled()) toolpanel.SetFocus(); else SetFocus();
}

void MapWidget::ResetToolpanel(bool b)
{
	toolpanel.EnableTools(b);
	SetResetTool();
}

void MapWidget::SetCurColor(ClickBox *pcb, bool bfg)
{
	bool b=false;
	Color c=RunDlgSelectColor(((bfg)?curColor:curBColor), false, 0, &b);
	if (b)
	{
		if (bfg) { curColor=c; pcb->c_fg=c; }
		else { curBColor=c; pcb->c_bg=c; }
		pcb->Refresh();
	}
}

void MapWidget::unset_tools()
{
	bSketch=bLine=bBox=bOval=bJot=false;
	pCurShape=nullptr;
	OverrideCursor(Image::Arrow());
	clear_toolselections(shapetools);
	//retain size & color selections
}

void MapWidget::SetTool(int T, ClickBox *pcb)
{
	unset_tools();
	switch(T)
	{
		case TOOL_SKETCH:	bSketch=true; break;
		case TOOL_LINE:		bLine=true; break;
		case TOOL_BOX:		bBox=true; break;
		case TOOL_OVAL:		bOval=true; break;
		case TOOL_JOT:		bJot=true; break;
	}
	if (pcb) { OverrideCursor(pcb->pic); pcb->Select(); } else OverrideCursor(Image::Arrow());
	Refresh();
}

void MapWidget::ToolMouseMove(Point pt, dword keyflags)
{
	if (HasCapture())
	{
		if (!pCurShape) { ReleaseCapture(); return; }
		ShapePoint sp(pt, curColor, curBColor, curWidth);
		pCurShape->process_MouseMove(sp, keyflags);
		Refresh();
	}
}

void MapWidget::ToolLeftDown(Point pt, dword keyflags)
{
	ShapePoint sp(pt, curColor, curBColor, curWidth);

	pCurShape=nullptr;
	if (bSketch) pCurShape=new Sketch;
	else if (bLine) pCurShape=new Line;
	else if (bBox) pCurShape=new Box;
	else if (bOval) pCurShape=new Oval;
	else if (bJot) pCurShape=new Jot;
	
	if (pCurShape)
	{
		urshapes.clear();
		SetCapture();
		pCurShape->process_LeftDown(sp, keyflags);
	}
	Refresh();
}

void MapWidget::ToolLeftUp(Point pt, dword keyflags)
{
	if (!pCurShape) { ReleaseCapture(); return; }
	ShapePoint sp(pt, curColor, curBColor, curWidth);
	if (pCurShape->isa()==ISA_JOT)
	{
		//prepping for direct in-map typing
		if (pCurShape->process_LeftUp(sp, keyflags)) save_new_shape(pCurShape);
		
	}
	else if (pCurShape->process_LeftUp(sp, keyflags)) save_new_shape(pCurShape);
	else delete pCurShape;
	pCurShape=nullptr;
	Refresh();
}

bool MapWidget::Key(dword key, int count)
{
	dword k=key;
	bool shift=k&K_SHIFT;
	bool ctrl=k&K_CTRL;
	bool alt=k&K_ALT;
	
	if (shift) k&=~K_SHIFT;
	if (ctrl) k&=~K_CTRL;
	if (alt) k&=~K_ALT;

	switch(key)
	{
/*
		case K_INSERT:				{ boverwrite=!boverwrite; Refresh(); } return true;
		case K_CTRL_INSERT:			Copy(); return true;
		case K_CTRL_DELETE:			Cut(); return true;
		case K_SHIFT_INSERT:		Paste(); return true;
		
		case K_SHIFT_F3:			return true; ///todo:...backwards find
		case K_CTRL_F3:				do_replace(); return true;
		case K_SHIFT_CTRL_F3:		return true; ///todo:...backwards f/r

		case K_SHIFT_TAB:			if (has_sel()) { unindent(); return true; } else break;
		
		case K_CTRL_PAGEUP:			/ *..* /
		case K_SHIFT_CTRL_PAGEUP:	/ *..* /
		case K_CTRL_HOME:			/ *..* /
		case K_SHIFT_CTRL_HOME:		DoMoveHome(shift); DoMoveHome(shift); return true;

		case K_CTRL_PAGEDOWN:		/ *..* /
		case K_SHIFT_CTRL_PAGEDOWN:	/ *..* /
		case K_CTRL_END:			/ *..* /
		case K_SHIFT_CTRL_END:		DoMoveEnd(shift); DoMoveEnd(shift); return true;

		case K_CTRL_LEFT:			/ *..* /
		case K_SHIFT_CTRL_LEFT:		DoMovePrevWord(shift); return true;
		case K_CTRL_RIGHT:			/ *..* /
		case K_SHIFT_CTRL_RIGHT:	DoMoveNextWord(shift); return true;
*/


	
		case K_CTRL_Z:				if (bIsDrawing) { undoshape(); return true; } return false;
		case K_SHIFT_CTRL_Z:		if (bIsDrawing) { redoshape(); return true; } return false;


		case K_ESCAPE:		{ SetTool(TOOL_NONE); return true; }

	

/*
		case K_CTRL_UP:				ScrollDown(); return true;
		case K_CTRL_DOWN:			ScrollUp(); return true;
		
		case K_CTRL_ADD:			pEPText->do_zoom(pEPText->dZoom+0.1); return true;
		case K_CTRL_SUBTRACT:		pEPText->do_zoom(pEPText->dZoom-0.1); return true;
		
		//case K_SHIFT_CTRL_A:		SelectAll(); return true;
		//..
*/

	}
	
/*
	if ((k>=K_SPACE)&&(k<127)) { AddCharCurPos(char(k)); return true; }
	else switch(k)
	{
		case K_TAB:			{ if (has_sel()) indent(); else AddCharCurPos('\t'); return true; }
		case K_ENTER:		{ NewLineCurPos();			return true; }
		case K_LEFT:		{ DoMoveLeft(shift);		return true; }
		case K_RIGHT:		{ DoMoveRight(shift);		return true; }
		case K_HOME:		{ DoMoveHome(shift);		return true; }
		case K_END:			{ DoMoveEnd(shift);			return true; }
		case K_UP:			{ DoMoveUp(shift);			return true; }
		case K_DOWN:		{ DoMoveDown(shift);		return true; }
		case K_PAGEUP:		{ DoMovePageUp(shift);		return true; }
		case K_PAGEDOWN:	{ DoMovePageDown(shift);	return true; }
		case K_DELETE:		{ DeleteCurPos();			return true; }
		case K_BACKSPACE:	{ BackspaceCurPos();		return true; }
		case K_F3:			{ do_find();				return true; }
		//..
	}

	return false;
	dword k=key;
	bool shift=k&K_SHIFT;
	bool ctrl=k&K_CTRL;
	bool alt=k&K_ALT;
	
	if (shift) k&=~K_SHIFT;
	if (ctrl) k&=~K_CTRL;
	if (alt) k&=~K_ALT;

	switch(key)
	{
		case K_INSERT:				{ boverwrite=!boverwrite; Refresh(); } return true;
		case K_CTRL_INSERT:			Copy(); return true;
		case K_CTRL_DELETE:			Cut(); return true;
		case K_SHIFT_INSERT:		Paste(); return true;
		
		case K_SHIFT_F3:			return true; ///todo:...backwards find
		case K_CTRL_F3:				do_replace(); return true;
		case K_SHIFT_CTRL_F3:		return true; ///todo:...backwards f/r

		case K_SHIFT_TAB:			if (has_sel()) { unindent(); return true; } else break;
		
		case K_CTRL_PAGEUP:			/ *..* /
		case K_SHIFT_CTRL_PAGEUP:	/ *..* /
		case K_CTRL_HOME:			/ *..* /
		case K_SHIFT_CTRL_HOME:		DoMoveHome(shift); DoMoveHome(shift); return true;

		case K_CTRL_PAGEDOWN:		/ *..* /
		case K_SHIFT_CTRL_PAGEDOWN:	/ *..* /
		case K_CTRL_END:			/ *..* /
		case K_SHIFT_CTRL_END:		DoMoveEnd(shift); DoMoveEnd(shift); return true;

		case K_CTRL_LEFT:			/ *..* /
		case K_SHIFT_CTRL_LEFT:		DoMovePrevWord(shift); return true;
		case K_CTRL_RIGHT:			/ *..* /
		case K_SHIFT_CTRL_RIGHT:	DoMoveNextWord(shift); return true;

		case K_CTRL_Z:				Undo(); return true;
		case K_SHIFT_CTRL_Z:		Redo(); return true;

		case K_CTRL_UP:				ScrollDown(); return true;
		case K_CTRL_DOWN:			ScrollUp(); return true;
		
		case K_CTRL_ADD:			pEPText->do_zoom(pEPText->dZoom+0.1); return true;
		case K_CTRL_SUBTRACT:		pEPText->do_zoom(pEPText->dZoom-0.1); return true;
		
		//case K_SHIFT_CTRL_A:		SelectAll(); return true;
		//..
		
	}
	
	if ((k>=K_SPACE)&&(k<127)) { AddCharCurPos(char(k)); return true; }
	else switch(k)
	{
		case K_TAB:			{ if (has_sel()) indent(); else AddCharCurPos('\t'); return true; }
		case K_ENTER:		{ NewLineCurPos();			return true; }
		case K_LEFT:		{ DoMoveLeft(shift);		return true; }
		case K_RIGHT:		{ DoMoveRight(shift);		return true; }
		case K_HOME:		{ DoMoveHome(shift);		return true; }
		case K_END:			{ DoMoveEnd(shift);			return true; }
		case K_UP:			{ DoMoveUp(shift);			return true; }
		case K_DOWN:		{ DoMoveDown(shift);		return true; }
		case K_PAGEUP:		{ DoMovePageUp(shift);		return true; }
		case K_PAGEDOWN:	{ DoMovePageDown(shift);	return true; }
		case K_DELETE:		{ DeleteCurPos();			return true; }
		case K_BACKSPACE:	{ BackspaceCurPos();		return true; }
		case K_F3:			{ do_find();				return true; }
		//..
	}
*/
	return false;
}



