#ifndef _modeler_fontsel_h_
#define _modeler_fontsel_h_

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

struct FontSel : public TopWindow
{
	typedef FontSel CLASSNAME;

	DropList	dlFace, dlHeight;
	Option		optBold, optItalic, optUnderline, optStrikeThrough, optNAA;
	Button		btnOK, btnCancel;
	bool bOK;
	int MIN_SIZE, MAX_SIZE, DEF_SIZE;
	FontSel(int min=3, int max=72, int def=12);
	virtual ~FontSel();
	virtual bool Key(dword key, int count);
	void fill_faces();
	void Set(Font f);
	Font Get();
};

Font SelectFont(const Font &def=StdFont());


#endif
