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


#include "extdll_menu.h"
#include "BaseMenu.h"
#include "Utils.h"
#include "MenuStrings.h"

#define EMPTY_STRINGS_1 ""
#define EMPTY_STRINGS_2 EMPTY_STRINGS_1, EMPTY_STRINGS_1
#define EMPTY_STRINGS_5 EMPTY_STRINGS_2, EMPTY_STRINGS_2, EMPTY_STRINGS_1
#define EMPTY_STRINGS_10 EMPTY_STRINGS_5, EMPTY_STRINGS_5
#define EMPTY_STRINGS_20 EMPTY_STRINGS_10, EMPTY_STRINGS_10
#define EMPTY_STRINGS_50 EMPTY_STRINGS_20, EMPTY_STRINGS_20, EMPTY_STRINGS_10
#define EMPTY_STRINGS_100 EMPTY_STRINGS_50, EMPTY_STRINGS_50

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

void UI_InitAliasStrings( void )
{
	char token[1024];

	// some strings needs to be initialized here
	sprintf( token, "Quit %s without\nsaving current game?", gMenu.m_gameinfo.title );
	MenuStrings[IDS_MAIN_QUITPROMPTINGAME] = StringCopy( token );

	sprintf( token, "Learn how to play %s", gMenu.m_gameinfo.title );
	MenuStrings[IDS_MAIN_TRAININGHELP] = StringCopy( token );

	sprintf( token, "Play %s on the 'easy' skill setting", gMenu.m_gameinfo.title );
	MenuStrings[IDS_NEWGAME_EASYHELP] = StringCopy( token );

	sprintf( token, "Play %s on the 'medium' skill setting", gMenu.m_gameinfo.title );
	MenuStrings[IDS_NEWGAME_MEDIUMHELP] = StringCopy( token );

	sprintf( token, "Play %s on the 'difficult' skill setting", gMenu.m_gameinfo.title );
	MenuStrings[IDS_NEWGAME_DIFFICULTHELP] = StringCopy( token );

	sprintf( token, "Quit playing %s", gMenu.m_gameinfo.title );
	MenuStrings[IDS_MAIN_QUITHELP] = StringCopy( token );

	sprintf( token, "Search for %s servers, configure character", gMenu.m_gameinfo.title );
	MenuStrings[IDS_MAIN_MULTIPLAYERHELP] = StringCopy( token );
}

void UI_LoadCustomStrings( void )
{
	char *afile = (char *)EngFuncs::COM_LoadFile( "gfx/shell/strings.lst", NULL );
	char *pfile = afile;
	char token[1024];
	int string_num;

	UI_InitAliasStrings ();

	if( !afile )
		return;

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
}
