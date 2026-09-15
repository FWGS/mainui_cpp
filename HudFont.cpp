/*
HudFont.cpp - engine callbacks for drawing HUD text
Uses mainui's font rendering stack instead of the engine bitmap font
*/
#include "extdll_menu.h"
#include "BaseMenu.h"
#include "font/FontManager.h"
#include "font/BaseFontBackend.h"

// all coordinates and return values are in real screen pixels

int UI_DrawHudCharacter( int x, int y, int ch, int r, int g, int b )
{
	if( !g_FontMgr || !uiStatic.hHudFont )
		return 0;

	CBaseFont *font = g_FontMgr->GetIFontFromHandle( uiStatic.hHudFont );
	if( !font )
		return 0;

	unsigned int color = PackRGBA( r, g, b, 255 );
	return font->DrawCharacter( ch, Point( x, y ), font->GetTall( ), color, true );
}

int UI_GetHudFontHeight( void )
{
	if( !g_FontMgr || !uiStatic.hHudFont )
		return 0;

	CBaseFont *font = g_FontMgr->GetIFontFromHandle( uiStatic.hHudFont );
	if( !font )
		return 0;

	return font->GetHeight( );
}

int UI_GetHudCharWidth( int ch )
{
	if( !g_FontMgr || !uiStatic.hHudFont )
		return 0;

	CBaseFont *font = g_FontMgr->GetIFontFromHandle( uiStatic.hHudFont );
	if( !font )
		return 0;

	int a, b, c;
	font->GetCharABCWidths( ch, a, b, c );
	return a + b + c;
}
