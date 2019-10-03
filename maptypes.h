#ifndef _maptypes_h_
#define _maptypes_h_


enum TOPICTYPE
{
	TT_GENERIC=0,
	TT_MAIN=1,
	TT_ISSUE, //"the scream" supposed to be
	TT_IDEA,
	TT_TASK,
	TT_DATA,
	TT_EVALUATE,
	TT_NOTE,
	TT_THING,
	TT_USERINTERFACE,
	TT_OBJECTIVE,
	TT_PROJECT,
	TT_PROCESS,
	TT_ROLE,
	TT_RULE,
	TT_WARN,

};

enum LINKTYPE
{
	LT_ASSOC=0, //generic
	LT_FLOW,
	LT_DATAIN,
	LT_DATAOUT,
	LT_CONTENT,
	LT_ONETOMANY,
};


#endif
