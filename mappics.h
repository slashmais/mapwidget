#ifndef _mappics_h_
#define _mappics_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

enum
{
	PICINFO,
	PICSELECT,
	PICTBCURSOR,
	PICARROW,
	PICDRAWTOOL,
	PICUNDO,
	PICREDO,
	PICPEN,
	PICLINE,
	PICRECT,
	PICOVAL,
	PICJOT,
	PICSIZE1,
	PICSIZE3,
	PICSIZE5,
	PICFG,
	PICBG,
};

Image GetMapPic(int p);




#endif
