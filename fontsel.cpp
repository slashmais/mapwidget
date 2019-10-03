
#include "fontsel.h"
#include <map>

//---------------------------------------------------------------------------------------------------
struct ffDisplay : public Display
{
	void Paint(Draw &drw, const Rect &r, const Value &val, Color fg, Color bg, dword dw) const
	{
		int h=12;
		String f=val.ToString();
		String e="eg: Ll, Ii, 1.23; \"Oo'0'\"";
		Font fe=Font(Font::FindFaceNameIndex(f), h);
		Font ff=StdFont(h);
		int w=(r.Width()-GetTextSize(e, fe).cx-5); //150;
		drw.DrawRect(r, bg);
		drw.DrawText(r.left, r.top+(r.Height()-ff.Info().GetHeight())/2, f, ff, fg); //font-name
		drw.DrawText(r.left+w, r.top+(r.Height()-fe.Info().GetHeight())/2, e, fe, fg); //sample
	}
};

FontSel::FontSel(int min, int max, int def)
{
	Title("Select Font");
	SetRect(0,0,360,110);
	CenterOwner();
	MIN_SIZE=min;
	MAX_SIZE=max;
	DEF_SIZE=def;
	Add(dlFace.HSizePosZ(4,4).TopPosZ(4, 19));
	Add(dlHeight.LeftPosZ(4, 64).TopPosZ(28, 19));
	Add(optBold.SetLabel(t_("B")).SetFont(StdFont(11).Bold()).LeftPosZ(80, 30).TopPosZ(28, 19).Tip("Bold"));
	Add(optItalic.SetLabel(t_("I")).SetFont(Monospace(11).Italic()).LeftPosZ(115, 30).TopPosZ(28, 19).Tip("Italic"));
	Add(optUnderline.SetLabel(t_("U")).SetFont(StdFont(11).Underline()).LeftPosZ(150, 30).TopPosZ(28, 19).Tip("Underline"));
	Add(optStrikeThrough.SetLabel(t_("S")).SetFont(StdFont(11).Strikeout()).LeftPosZ(185, 30).TopPosZ(28, 19).Tip("Strike-through"));
	Add(optNAA.SetLabel(t_("NA-A")).SetFont(StdFont().NonAntiAliased()).LeftPosZ(220, 40).TopPosZ(28, 19).Tip("Non-Anti-Aliased"));
	Add(btnOK.SetLabel(t_("OK")).RightPosZ(76, 60).BottomPosZ(12, 20));
	Add(btnCancel.SetLabel(t_("Cancel")).RightPosZ(4, 60).BottomPosZ(12, 20));
	bOK=false;
	dlFace.SetDisplay(Single<ffDisplay>());
	btnOK.WhenPush << [&]{ bOK=true; Close(); };
	btnCancel.WhenPush << [&]{ bOK=false; Close(); };
	fill_faces();
}

FontSel::~FontSel() {}

bool FontSel::Key(dword key, int count)
{
	switch (key)
	{
		case K_ENTER: { bOK=true; Close(); return true; }
		case K_ESCAPE: { bOK=false; Close(); return true; }
	}
	return TopWindow::Key(key, count);
}

void FontSel::fill_faces()
{
	int i;
	auto fillh=[&](int f, int t){ for (i=f; i<=t; i++) dlHeight.Add(i); };
	dlFace.Clear();
	std::map<String, std::pair<int, String> >mf;
	String S;
	for (i=0; i<Font::GetFaceCount(); i++) { S=Font::GetFaceName(i); mf[ToLower(S)]=std::make_pair(i, S); }
	for (auto p:mf) dlFace.Add(p.second.first, p.second.second);
	dlFace.SetIndex(0);
	dlHeight.ClearList();
	fillh(MIN_SIZE, MAX_SIZE);
	dlHeight.SetData(DEF_SIZE);
}

void FontSel::Set(Font f)
{
	int fi = f.GetFace();
	if (!dlFace.HasKey(fi))
	{
		fi=dlFace.FindValue(f.GetFaceName());
		if (fi<0) fi=Font::STDFONT; else fi=dlFace.GetKey(fi);
	}
	dlFace.SetData(fi);
	dlHeight.SetData(f.GetHeight());
	for (int i=0; i<dlHeight.GetCount(); i++)
	{
		int q=dlHeight.GetKey(i);
		if (f.GetHeight()<=q) { dlHeight.SetData(q); break; }
	}
	optBold=f.IsBold();
	optItalic=f.IsItalic();
	optUnderline=f.IsUnderline();
	optStrikeThrough=f.IsStrikeout();
	optNAA = f.IsNonAntiAliased();
}

Font FontSel::Get()
{
	Font f(dlFace.GetData(), dlHeight.GetData());
	if (optBold) f.Bold();
	if (optItalic) f.Italic();
	if (optUnderline) f.Underline();
	if (optStrikeThrough) f.Strikeout();
	if (optNAA) f.NonAntiAliased();
	return f;
}

//------------------------------------------------
Font SelectFont(const Font &deffnt)
{
	Font f=deffnt;
	FontSel dlgFS;
	dlgFS.Set(deffnt);
	dlgFS.Execute();
	if (dlgFS.bOK) f=dlgFS.Get();
	return f;
}


