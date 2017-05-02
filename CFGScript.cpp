/*
cfgscript.c - "Valve script" parsing routines
Copyright (C) 2016 mittorn

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/


#include "extdll.h"
#include "BaseMenu.h"
#include "enginecallback.h"

#define MAX_STRING 256
#define MAX_SCR_VARS 128
#define CVAR_USERINFO BIT(1)

typedef enum
{
	T_NONE = 0,
	T_BOOL,
	T_NUMBER,
	T_LIST,
	T_STRING,
	T_COUNT
} cvartype_t;

const char *cvartypes[] = { NULL, "BOOL" , "NUMBER", "LIST", "STRING" };

typedef struct parserstate_s
{
	char *buf;
	char token[MAX_STRING];
	const char *filename;
} parserstate_t;

typedef struct scrvardef_s
{
	int flags;
	char name[MAX_STRING];
	char value[MAX_STRING];
	char desc[MAX_STRING];
	float fMin, fMax;
	cvartype_t type;
	bool fHandled;
	scrvardef_s *next;
} scrvardef_t;

/*
===================
CSCR_ExpectString

Return true if next token is pExpext and skip it
===================
*/
bool CSCR_ExpectString( parserstate_t *ps, const char *pExpect, bool skip, bool error )
{
	char *tmp = EngFuncs::COM_ParseFile( ps->buf, ps->token );

	if( !stricmp( ps->token, pExpect ) )
	{
		ps->buf = tmp;
		return true;
	}

	if( skip )
		ps->buf = tmp;

	if( error )
		Con_DPrintf( "Syntax error in %s: got \"%s\" instead of \"%s\"\n", ps->filename, ps->token, pExpect );

	return false;
}

/*
===================
CSCR_ParseType

Determine script variable type
===================
*/
cvartype_t CSCR_ParseType( parserstate_t *ps )
{
	int i;

	for ( i = 1; i < T_COUNT; ++i )
	{
		if( CSCR_ExpectString( ps, cvartypes[i], false, false ) )
			return (cvartype_t)i;
	}

	Con_DPrintf( "Cannot parse %s: Bad type %s\n", ps->filename, ps->token );
	return T_NONE;
}



/*
=========================
CSCR_ParseSingleCvar
=========================
*/
bool CSCR_ParseSingleCvar( parserstate_t *ps, scrvardef_t *result )
{
	// read the name
	ps->buf = EngFuncs::COM_ParseFile( ps->buf, result->name );

	if( !CSCR_ExpectString( ps, "{", false, true ) )
		return false;

	// read description
	ps->buf = EngFuncs::COM_ParseFile( ps->buf, result->desc );

	if( !CSCR_ExpectString( ps, "{", false, true ) )
		return false;

	result->type = CSCR_ParseType( ps );

	switch( result->type )
	{
	case T_BOOL:
		// bool only has description
		if( !CSCR_ExpectString( ps, "}", false, true ) )
			return false;
		break;
	case T_NUMBER:
		// min
		ps->buf = EngFuncs::COM_ParseFile( ps->buf, ps->token );
		result->fMin = atof( ps->token );

		// max
		ps->buf = EngFuncs::COM_ParseFile( ps->buf, ps->token );
		result->fMax = atof( ps->token );

		if( !CSCR_ExpectString( ps, "}", false, true ) )
			return false;
		break;
	case T_STRING:
		if( !CSCR_ExpectString( ps, "}", false, true ) )
			return false;
		break;
	case T_LIST:
		while( !CSCR_ExpectString( ps, "}", true, false ) )
		{
			// Read token for each item here
		}
		break;
	default:
		return false;
	}

	if( !CSCR_ExpectString( ps, "{", false, true ) )
		return false;

	// default value
	ps->buf = EngFuncs::COM_ParseFile( ps->buf, result->value );

	if( !CSCR_ExpectString( ps, "}", false, true ) )
		return false;

	if( CSCR_ExpectString( ps, "SetInfo", false, false ) )
		result->flags |= CVAR_USERINFO;

	if( !CSCR_ExpectString( ps, "}", false, true ) )
		return false;

	return true;
}

/*
======================
CSCR_ParseHeader

Check version and seek to first cvar name
======================
*/
bool CSCR_ParseHeader( parserstate_t *ps )
{
	if( !CSCR_ExpectString( ps, "VERSION", false, true ) )
		return false;

	// Parse in the version #
	// Get the first token.
	ps->buf = EngFuncs::COM_ParseFile( ps->buf, ps->token );

	if( atof( ps->token ) != 1 )
	{
		Con_DPrintf( "File %s has wrong version %s!\n", ps->filename, ps->token );
		return false;
	}

	if( !CSCR_ExpectString( ps, "DESCRIPTION", false, true ) )
		return false;

	ps->buf = EngFuncs::COM_ParseFile( ps->buf, ps->token );

	if( stricmp( ps->token, "INFO_OPTIONS") && stricmp( ps->token, "SERVER_OPTIONS" ) )
	{
		Con_DPrintf( "DESCRIPTION must be INFO_OPTIONS or SERVER_OPTIONS\n");
		return false;
	}

	if( !CSCR_ExpectString( ps, "{", false, true ) )
		return false;

	return true;
}

/*
======================
CSCR_LoadDefaultCVars

Register all cvars declared in config file and set default values
======================
*/
scrvardef_t *CSCR_LoadDefaultCVars( const char *scriptfilename, int *count )
{
	int length = 0;
	char *start;
	parserstate_t state = {0};
	bool success = false;
	scrvardef_t *list = 0, *last;

	*count = 0;

	state.filename = scriptfilename;

	state.buf = (char*)EngFuncs::COM_LoadFile( scriptfilename, &length );

	start = state.buf;

	if( state.buf == 0 || length == 0)
	{
		if( start )
			EngFuncs::COM_FreeFile( start );
		return 0;
	}

	Con_DPrintf( "Reading config script file %s\n", scriptfilename );

	if( !CSCR_ParseHeader( &state ) )
	{
		Con_DPrintf( "Failed to	parse header!\n" );
		goto finish;
	}

	while( !CSCR_ExpectString( &state, "}", false, false ) )
	{
		scrvardef_t var = { 0 };

		// Create a new object
		if( CSCR_ParseSingleCvar( &state, &var ) )
		{
			// Cvar_Get( var.name, var.value, var.flags, var.desc );
			scrvardef_t *entry = (scrvardef_t*)MALLOC( sizeof( scrvardef_t ) );
			*entry = var;

			if( !list )
			{
				list = last = entry;
			}
			else
			{
				last = last->next = entry;
			}
			(*count)++;
		}
		else
			break;

		if( *count > 1024 )
			break;
	}

	if( EngFuncs::COM_ParseFile( state.buf, state.token ) )
		Con_DPrintf( "Got extra tokens!\n" );
	else
		success = true;

finish:
	if( !success )
	{
		state.token[ sizeof( state.token ) - 1 ] = 0;
		if( start && state.buf )
			Con_DPrintf( "Parse error in %s, byte %d, token %s\n", scriptfilename, (int)( state.buf - start ), state.token );
		else
			Con_DPrintf( "Parse error in %s, token %s\n", scriptfilename, state.token );
	}
	if( start )
		EngFuncs::COM_FreeFile( start );

	return list;
}

void CSCR_FreeList( scrvardef_t *list )
{
	scrvardef_t *i;
	while( i )
	{
		scrvardef_t *next = i->next;
		FREE( i );
		i = next;
	}
}
