/*
MenuStrings.cpp - custom menu strings
Copyright (C) 2011 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include <ctype.h>
#include "extdll_menu.h"
#include "BaseMenu.h"
#include "Utils.h"
#include "MenuStrings.h"
#include "unicode_strtools.h"

#define EMPTY_STRINGS_1 ""
#define EMPTY_STRINGS_2 EMPTY_STRINGS_1, EMPTY_STRINGS_1
#define EMPTY_STRINGS_5 EMPTY_STRINGS_2, EMPTY_STRINGS_2, EMPTY_STRINGS_1
#define EMPTY_STRINGS_10 EMPTY_STRINGS_5, EMPTY_STRINGS_5
#define EMPTY_STRINGS_20 EMPTY_STRINGS_10, EMPTY_STRINGS_10
#define EMPTY_STRINGS_50 EMPTY_STRINGS_20, EMPTY_STRINGS_20, EMPTY_STRINGS_10
#define EMPTY_STRINGS_100 EMPTY_STRINGS_50, EMPTY_STRINGS_50

#define HASH_SIZE 256 // 256 * 4 * 4 == 4096 bytes
static struct dictionary_t
{
	const char *name;
	const char *value;
	dictionary_t *next;
} *hashed_cmds[HASH_SIZE];

const char *MenuStrings[IDS_LAST] =
{
EMPTY_STRINGS_100, // 0..9
EMPTY_STRINGS_20, // 100..119
EMPTY_STRINGS_10, // 120..129
EMPTY_STRINGS_2, // 130..131
"Display mode", // 132
EMPTY_STRINGS_5, // 133..137
EMPTY_STRINGS_2, // 138..139
EMPTY_STRINGS_20, // 140..159
EMPTY_STRINGS_10, // 160..169
EMPTY_STRINGS_1, // 170
"Reverse mouse", // 171
EMPTY_STRINGS_10, // 172..181
EMPTY_STRINGS_2, // 182..183
"Mouse sensitivity", // 184
EMPTY_STRINGS_1, // 185
EMPTY_STRINGS_2, // 186..187
"Return to game.", // 188
"Start a new game.", // 189
EMPTY_STRINGS_1,	// 190
"Load a previously saved game.", // 191
"Load a saved game, save the current game.", // 192
"Change game settings, configure controls", // 193
EMPTY_STRINGS_20, // 194..213
EMPTY_STRINGS_20, // 214..233
"Starting a Hazard Course will exit\nany current game, OK to exit?", // 234
EMPTY_STRINGS_1, // 235
"Are you sure you want to quit?", // 236
EMPTY_STRINGS_2, // 237..238
EMPTY_STRINGS_1, // 239
"Starting a new game will exit\nany current game, OK to exit?",	// 240
EMPTY_STRINGS_5, // 241..245
EMPTY_STRINGS_2, // 246..247
EMPTY_STRINGS_2, // 248..249
EMPTY_STRINGS_100, // 250..349
EMPTY_STRINGS_50, // 350..399
"Find more about Valve's product lineup",	// 400
EMPTY_STRINGS_1, // 401
"http://store.steampowered.com/app/70/", // 402
EMPTY_STRINGS_5, // 403..407
EMPTY_STRINGS_2, // 408..409
EMPTY_STRINGS_100, // 410..509
EMPTY_STRINGS_20, // 510..529
"Select a custom game",	// 530
EMPTY_STRINGS_5, // 531..535
EMPTY_STRINGS_2, // 536..537
EMPTY_STRINGS_2, // 538..539
EMPTY_STRINGS_50, // 540..589
EMPTY_STRINGS_10, // 590..599
};

/*
=================
Com_HashKey

returns hash key for string
=================
*/
static uint Com_HashKey( const char *string, uint hashSize )
{
	uint	i, hashKey = 0;

	for( i = 0; string[i]; i++ )
	{
		hashKey = (hashKey + i) * 37 + tolower( string[i] );
	}

	return (hashKey % hashSize);
}

static inline dictionary_t *Dictionary_FindInBucket( dictionary_t *bucket, const char *name )
{
	dictionary_t *i = bucket;
	for( ; i && stricmp( name, i->name ); // filter out
		 i = i->next );

	return i;
}

static void Dictionary_Insert( const char *name, const char *second )
{
	uint hash = Com_HashKey( name, HASH_SIZE );
	dictionary_t *elem;

	elem = Dictionary_FindInBucket( hashed_cmds[hash], name );
	if( elem )
	{
		delete [] (char*) elem->value;
		elem->value = StringCopy( second );
	}
	else
	{
		elem = new dictionary_t;
		elem->name = StringCopy(name);
		elem->value = StringCopy(second);
		elem->next   = hashed_cmds[hash];
		hashed_cmds[hash] = elem;
	}
}

static inline dictionary_t *Dictionary_GetBucket( const char *name )
{
	return hashed_cmds[ Com_HashKey( name, HASH_SIZE ) ];
}

static void UI_InitAliasStrings( void )
{
	const struct
	{
		const char *defAliasString;
		int idx;
	} aliasStrings[] =
	{
	{ "Quit %s without\nsaving current game?", IDS_MAIN_QUITPROMPTINGAME },
	{ "Learn how to play %s", IDS_MAIN_TRAININGHELP },
	{ "Play %s on the 'easy' skill setting", IDS_NEWGAME_EASYHELP },
	{ "Play %s on the 'medium' skill setting", IDS_NEWGAME_MEDIUMHELP },
	{ "Play %s on the 'difficult' skill setting", IDS_NEWGAME_DIFFICULTHELP },
	{ "Quit playing %s", IDS_MAIN_QUITHELP },
	{ "Search for %s servers, configure character", IDS_MAIN_MULTIPLAYERHELP },
	};

	for( int i = 0; i < ARRAYSIZE( aliasStrings ); i++ )
	{
		if( MenuStrings[aliasStrings[i].idx][0]) // check if not initialized by strings.lst
			continue;

		char token[1024];
		char token2[1024];
		sprintf( token, "StringsList_%d", aliasStrings[i].idx );

		const char *fmt = L( token );
		if( fmt == token )
		{
			fmt = aliasStrings[i].defAliasString;
		}

		sprintf( token2, fmt, gMenu.m_gameinfo.title );
		MenuStrings[aliasStrings[i].idx] = StringCopy( token2 );

		Dictionary_Insert( token, MenuStrings[aliasStrings[i].idx] );
	}
}

int UTFToCP1251( char *out, const char *instr, int len, int maxoutlen )
{
	int m = -1, k = 0, uc = 0;
	const char *inbegin = instr;
	char *outbegin = out;

	while( *instr && (out - outbegin) < maxoutlen )
	{
		int in = *instr & 255;
		if( instr - inbegin >= len )
		{
			*out = 0;
			return out - outbegin;
		}

		// Get character length
		if(m == -1)
		{
			uc = 0;
			if( in >= 0xF8 )
			{
				instr++;
				continue;
			}
			else if( in >= 0xF0 )
				uc = in & 0x07, m = 3;
			else if( in >= 0xE0 )
				uc = in & 0x0F, m = 2;
			else if( in >= 0xC0 )
				uc = in & 0x1F, m = 1;
			else if( in <= 0x7F)
			{
				if( in == '\\' && *(instr + 1) != '\\'  )
				{
					instr++;
					continue;
				}
				*out++ = *instr++;
				continue;
			}
			// we need more chars to decode one
			k=0;
			instr++;
			continue;
		}
		// get more chars
		else if( k <= m )
		{
			uc <<= 6;
			uc += in & 0x3F;
			k++;
		}
		if( in > 0xBF || m < 0 )
		{
			m = -1;
			instr++;
			continue;
		}


		if( k == m )
		{
			k = m = -1;
			// cp1252 hackish translate
			if( uc > 0x7F && uc <= 0xFF )
			{
				*out++ = uc;
				instr++;
				continue;
			}

			// cp1251
			if( uc >= 0x0410 && uc <= 0x042F ) // Cyrillic Capital Letters
			{
				*out++ = uc - 0x410 + 0xC0;
				instr++;
				continue;
			}
			else if( uc >= 0x0430 && uc <= 0x044F ) // Cyrillic Small Letters
			{
				*out++ = uc - 0x430 + 0xE0;
				instr++;
				continue;
			}
			else
			{
				int i;
				for( i = 0; i < 64; i++ )
				{
					if( table_cp1251[i] == uc )
					{
						*out++ = i + 0x80;
						instr++;
						break;
					}
				}
				continue;
			}

			*out++ = *instr++;
			continue;
		}
		instr++;
	}
	*out = 0;
	return out - outbegin;
}

static void Localize_AddToDictionary( const char *name, const char *lang )
{
	char filename[64];
	snprintf( filename, sizeof( filename ), "resource/%s_%s.txt", name, lang );

	int unicodeLength;
	char *pFileBuf = (char*)EngFuncs::COM_LoadFile( filename, &unicodeLength );

	if( pFileBuf ) // no problem, so read it.
	{
		int ansiLength = unicodeLength + 1;
		char *afile = new char[ansiLength]; // save original pointer, so we can free it later
		uchar16 *autf16 = new uchar16[unicodeLength/2 + 1];
		char *pfile = afile;
		char token[4096];
		int i = 0;

		memcpy( autf16, pFileBuf + 2, unicodeLength - 1 );
		autf16[unicodeLength/2-1] = 0; //null terminator

		Q_UTF16ToUTF8( autf16, afile, ansiLength, STRINGCONVERT_ASSERT_REPLACE );

#ifdef XASH_DISABLE_FWGS_EXTENSIONS
		UTFToCP1251( afile, afile, ansiLength, ansiLength );
#endif // XASH_DISABLE_FWGS_EXTENSIONS

		pfile = EngFuncs::COM_ParseFile( pfile, token );

		if( stricmp( token, "lang" ))
		{
			Con_Printf( "Localize_AddToDict( %s, %s ): invalid header, got %s", name, lang, token );
			goto error;
		}

		pfile = EngFuncs::COM_ParseFile( pfile, token );

		if( strcmp( token, "{" ))
		{
			Con_Printf( "Localize_AddToDict( %s, %s ): want {, got %s", name, lang, token );
			goto error;
		}

		pfile = EngFuncs::COM_ParseFile( pfile, token );

		if( stricmp( token, "Language" ))
		{
			Con_Printf( "Localize_AddToDict( %s, %s ): want Language, got %s", name, lang, token );
			goto error;
		}

		// skip language actual name
		pfile = EngFuncs::COM_ParseFile( pfile, token );

		pfile = EngFuncs::COM_ParseFile( pfile, token );

		if( stricmp( token, "Tokens" ))
		{
			Con_Printf( "Localize_AddToDict( %s, %s ): want Tokens, got %s", name, lang, token );
			goto error;
		}

		pfile = EngFuncs::COM_ParseFile( pfile, token );

		if( strcmp( token, "{" ))
		{
			Con_Printf( "Localize_AddToDict( %s, %s ): want { after Tokens, got %s", name, lang, token );
			goto error;
		}

		while( (pfile = EngFuncs::COM_ParseFile( pfile, token )))
		{
			if( !strcmp( token, "}" ))
				break;

			char szLocString[4096];
			pfile = EngFuncs::COM_ParseFile( pfile, szLocString );

			if( !strcmp( szLocString, "}" ))
				break;


			if( pfile )
			{
				// Con_DPrintf("New token: %s %s\n", token, szLocString );
				Dictionary_Insert( token, szLocString );
				i++;
			}
		}

		Con_Printf( "Localize_AddToDict: loaded %i words from %s\n", i, filename );

error:
		delete[] afile;
		delete[] autf16;

		EngFuncs::COM_FreeFile( pFileBuf );
	}
	else
	{
		Con_Printf( "Couldn't open file %s. Some strings will not be localized!.\n", filename );
	}
}

static void Localize_Init( void )
{
	EngFuncs::ClientCmd( TRUE, "exec mainui.cfg\n" );

	memset( hashed_cmds, 0, sizeof( hashed_cmds ) );

	// strings.lst first
	for( int i = 0; i < IDS_LAST; i++ )
	{
		if( !MenuStrings[i][0] )
			continue;

		char buf[256];

		snprintf( buf, sizeof( buf ), "StringsList_%i", i );

		Dictionary_Insert( buf, MenuStrings[i] );
	}

	const char *language = EngFuncs::GetCvarString( "ui_language" );
	const char *gamedir = gMenu.m_gameinfo.gamefolder;

	if( !language[0] )
		language = "english"; // fallback to just english

	if( strcmp( gamedir, "mainui" ))
		Localize_AddToDictionary( "mainui", language );

	if( strcmp( gamedir, "gameui" ))
		Localize_AddToDictionary( "gameui", language );

	if( strcmp( gamedir, "valve" ))
		Localize_AddToDictionary( "valve",  language );

	Localize_AddToDictionary( gamedir,  language );
}

static void Localize_Free( void )
{
	for( int i = 0; i < HASH_SIZE; i++ )
	{
		dictionary_t *base = hashed_cmds[i];
		while( base )
		{
			dictionary_t *next = base->next;

			delete [] (char*)base->value;
			delete [] (char*)base->name;

			delete base;

			base = next;
		}
	}

	return;
}

void UI_LoadCustomStrings( void )
{
	char *afile = (char *)EngFuncs::COM_LoadFile( "gfx/shell/strings.lst", NULL );
	char *pfile = afile;
	char token[1024];
	int string_num;

	if( !afile )
		goto localize_init;

	while(( pfile = EngFuncs::COM_ParseFile( pfile, token )) != NULL )
	{
		if( isdigit( token[0] ))
		{
			string_num = atoi( token );

			// check for bad stiringnum
			if( string_num < 0 ) continue;
			if( string_num > ( IDS_LAST - 1 ))
				continue;
		}
		else continue; // invalid declaration ?

		// parse new string
		pfile = EngFuncs::COM_ParseFile( pfile, token );
		MenuStrings[string_num] = StringCopy( token ); // replace default string with custom
	}

	EngFuncs::COM_FreeFile( afile );

localize_init:
	Localize_Init();
	UI_InitAliasStrings ();
}

const char *L( const char *szStr ) // L means Localize!
{
	if( szStr )
	{
		if( *szStr == '#' )
			szStr++;

		dictionary_t *base = Dictionary_GetBucket( szStr );
		dictionary_t *found = Dictionary_FindInBucket( base, szStr );

		if( found )
			return found->value;
	}

	return szStr;
}

void UI_FreeCustomStrings( void )
{
	Localize_Free();
}
