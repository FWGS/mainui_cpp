/*
FontManager.h - font manager
Copyright (C) 2017 a1batross

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#pragma once
#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "utl/utlvector.h"
#include "Primitive.h"
#include "FontRenderer.h"

class CBaseFont;

/*
 * Font manager is used for creating and operating with fonts
 **/
class CFontManager
{
public:
	CFontManager();
	~CFontManager();

	void VidInit();

	void DeleteAllFonts();
	void DeleteFont( HFont hFont );

	HFont GetFontByName( const char *name );
	void  GetCharABCWide( HFont font, int ch, int &a, int &b, int &c );
	int   GetFontTall( HFont font );
	int   GetFontAscent( HFont font );
	bool  GetFontUnderlined( HFont font );

	int   GetCharacterWidthScaled(HFont font, int ch, int charH );

	void  GetTextSize( HFont font, const char *text, int *wide, int *tall = NULL, int size = -1 );
	int   GetTextHeight( HFont font, const char *text, int size = -1 );

	int   CutText( HFont fontHandle, const char *text, int height, int visibleSize , int &width);

	int GetTextWideScaled( HFont font, const char *text, const int height, int size = -1 );

	int DrawCharacter( HFont font, int ch, Point pt, int charH, const unsigned int color, bool forceAdditive = false );

	void DebugDraw( HFont font );
	CBaseFont *GetIFontFromHandle( HFont font );

	int GetEllipsisWide( HFont font ); // cached wide of "..."
private:
	int  GetCharacterWidth( HFont font, int ch );
	int  GetTextWide( HFont font, const char *text, int size = -1 );

	void UploadTextureForFont(CBaseFont *font );

	CUtlVector<CBaseFont*> m_Fonts;

	friend class CFontBuilder;
};

// lazy to fix code everywhere
#ifndef CLIENT_DLL
extern CFontManager g_FontMgr;
#endif

#endif // FONTMANAGER_H
