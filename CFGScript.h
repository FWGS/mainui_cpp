#pragma once
#ifndef CFGSCRIPT_H
#define CFGSCRIPT_H

#define MAX_STRING 256

typedef enum
{
	T_NONE = 0,
	T_BOOL,
	T_NUMBER,
	T_LIST,
	T_STRING,
	T_COUNT
} cvartype_t;

typedef struct scrvarlistentry_s
{
	char *szName;
	float flValue;
	struct scrvarlistentry_s *next;
} scrvarlistentry_t;

typedef struct scrvarlist_s
{
	int iCount;
	scrvarlistentry_t *pEntries;
	scrvarlistentry_t *pLast;
	const char **pArray;
} scrvarlist_t;

typedef struct
{
	float fMin;
	float fMax;
} scrvarnumber_t;

typedef struct scrvardef_s
{
	int flags;
	char name[MAX_STRING];
	char value[MAX_STRING];
	char desc[MAX_STRING];
	union
	{
		scrvarnumber_t number;
		scrvarlist_t list;
	};
	cvartype_t type;
	struct scrvardef_s *next;
} scrvardef_t;

scrvardef_t *CSCR_LoadDefaultCVars( const char *scriptfilename, int *count );
void CSCR_FreeList( scrvardef_t *list );

#endif // CFGSCRIPT_H
