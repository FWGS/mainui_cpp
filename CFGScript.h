#pragma once
#ifndef CFGSCRIPT_H
#define CFGSCRIPT_H

#define MAX_STRING 256

enum cvartype_t
{
	T_NONE = 0,
	T_BOOL,
	T_NUMBER,
	T_LIST,
	T_STRING,
	T_COUNT
};

struct scrvardef_t
{
	int flags;
	char name[MAX_STRING];
	char value[MAX_STRING];
	char desc[MAX_STRING];
	float fMin, fMax;
	cvartype_t type;
	bool fHandled;
	scrvardef_t *next;
};

scrvardef_t *CSCR_LoadDefaultCVars( const char *scriptfilename, int *count );
void CSCR_FreeList( scrvardef_t *list );

#endif // CFGSCRIPT_H
