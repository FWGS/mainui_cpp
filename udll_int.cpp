/*
dll_int.cpp - dll entry point
Copyright (C) 2010 Uncle Mike

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
#include "cl_dll/IGameClientExports.h"

ui_enginefuncs_t EngFuncs::engfuncs;
#ifndef XASH_DISABLE_FWGS_EXTENSIONS
ui_textfuncs_t	EngFuncs::textfuncs;
#endif
ui_globalvars_t	*gpGlobals;
IGameClientExports *g_pClient;
CMenu gMenu;

static UI_FUNCTIONS gFunctionTable = 
{
	UI_VidInit,
	UI_Init,
	UI_Shutdown,
	UI_UpdateMenu,
	UI_KeyEvent,
	UI_MouseMove,
	UI_SetActiveMenu,
	UI_AddServerToList,
	UI_GetCursorPos,
	UI_SetCursorPos,
	UI_ShowCursor,
	UI_CharEvent,
	UI_MouseInRect,
	UI_IsVisible,
	UI_CreditsActive,
	UI_FinalCredits
};

//=======================================================================
//			GetApi
//=======================================================================
extern "C" EXPORT int GetMenuAPI(UI_FUNCTIONS *pFunctionTable, ui_enginefuncs_t* pEngfuncsFromEngine, ui_globalvars_t *pGlobals)
{
	if( !pFunctionTable || !pEngfuncsFromEngine )
	{
		return FALSE;
	}

	// copy HUD_FUNCTIONS table to engine, copy engfuncs table from engine
	memcpy( pFunctionTable, &gFunctionTable, sizeof( UI_FUNCTIONS ));
	memcpy( &EngFuncs::engfuncs, pEngfuncsFromEngine, sizeof( ui_enginefuncs_t ));
#ifndef XASH_DISABLE_FWGS_EXTENSIONS
	memset( &EngFuncs::textfuncs, 0, sizeof( ui_textfuncs_t ));
#endif
	gpGlobals = pGlobals;

	return TRUE;
}

#ifndef XASH_DISABLE_FWGS_EXTENSIONS
extern "C" EXPORT int GiveTextAPI( ui_textfuncs_t* pTextfuncsFromEngine )
{
	if( !pTextfuncsFromEngine )
	{
		return FALSE;
	}

	// copy HUD_FUNCTIONS table to engine, copy engfuncs table from engine
	memcpy( &EngFuncs::textfuncs, pTextfuncsFromEngine, sizeof( ui_textfuncs_t ));

	return TRUE;
}
#endif

static class CGameMenuExports : public IGameMenuExports
{
public:
	bool Initialize( CreateInterfaceFn factory )
	{
		g_pClient = (IGameClientExports*)factory( GAMECLIENTEXPORTS_INTERFACE_VERSION, NULL );

		return g_pClient ? true : false;
	}

	bool IsActive()
	{
		return UI_IsVisible();
	}

	void  Key( int key, int down )
	{
		UI_KeyEvent( key, down );
	}

	void  MouseMove( int x, int y )
	{
		UI_MouseMove( x, y );
	}

	HFont BuildFont( CFontBuilder &builder )
	{
		return builder.Create();
	}

	void  GetCharABCWide( HFont font, int ch, int &a, int &b, int &c )
	{
		g_FontMgr.GetCharABCWide( font, ch, a, b, c );
	}

	int   GetFontTall( HFont font )
	{
		return g_FontMgr.GetFontTall( font );
	}

	int   GetCharacterWidth(HFont font, int ch, int charH )
	{
		return g_FontMgr.GetCharacterWidthScaled( font, ch, charH );
	}

	void  GetTextSize( HFont font, const char *text, int *wide, int *height = 0, int size = -1 )
	{
		g_FontMgr.GetTextSize( font, text, wide, height, size );
	}

	int	  GetTextHeight( HFont font, const char *text, int size = -1 )
	{
		return g_FontMgr.GetTextHeight( font, text, size );
	}

	int   DrawCharacter( HFont font, int ch, int x, int y, int charH, const unsigned int color, bool forceAdditive = false )
	{
		return g_FontMgr.DrawCharacter( font, ch, Point( x, y ), charH, color, forceAdditive );
	}

	void  DrawScoreboard( void )
	{

	}

	void  DrawSpectatorMenu( void )
	{

	}

	void  ShowVGUIMenu( int )
	{

	}
} s_Menu;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameMenuExports, IGameMenuExports, GAMEMENUEXPORTS_INTERFACE_VERSION, s_Menu );
