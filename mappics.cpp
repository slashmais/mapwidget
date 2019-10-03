
#include "mappics.h"

#define  IMAGEFILE <mapwidget/mappics.iml>
#define  IMAGECLASS MAPPICS
#include <Draw/iml.h>


Image GetMapPic(int p)
{
	switch(p)
	{
		case PICINFO:			return MAPPICS::picInfo();
		case PICSELECT:			return MAPPICS::picSelect();
		case PICTBCURSOR:		return MAPPICS::picTBCursor();
		case PICARROW:			return MAPPICS::picArrow();
		case PICDRAWTOOL:		return MAPPICS::picDrawTool();
		case PICUNDO:			return MAPPICS::picUndo();
		case PICREDO:			return MAPPICS::picRedo();
		case PICPEN:			return MAPPICS::picPen();
		case PICLINE:			return MAPPICS::picLine();
		case PICRECT:			return MAPPICS::picRect();
		case PICOVAL:			return MAPPICS::picOval();
		case PICJOT:			return MAPPICS::picJot();
		case PICSIZE1:			return MAPPICS::picSize1();
		case PICSIZE3:			return MAPPICS::picSize3();
		case PICSIZE5:			return MAPPICS::picSize5();
		case PICFG:				return MAPPICS::picFG();
		case PICBG:				return MAPPICS::picBG();
		
	}
	return MAPPICS::picDefault();
}





