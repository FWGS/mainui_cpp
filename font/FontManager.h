#pragma once
#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "BaseMenu.h"
#include "utl/utlvector.h"

class IBaseFont;

enum EFontFlags
{
	FONT_NONE      = 0,
	FONT_ITALIC    = BIT( 0 ),
	FONT_UNDERLINE = BIT( 1 ),
	FONT_STRIKEOUT = BIT( 2 ),
	FONT_SYMBOL    = BIT( 3 ),
	FONT_ANTIALIAS = BIT( 4 ),
	FONT_GAUSSBLUR = BIT( 5 ),
	// FONT_ROTARY    = BIT( 7 ),
	// FONT_DROPSHADOW = BIT( 8 )
};

/*
 * Font manager is used for creating and operating with fonts
 */
class CFontManager
{
public:
	CFontManager();
	~CFontManager();

	void VidInit();

	void DeleteAllFonts();

	HFont CreateFont(const char *name, int tall, int weight, int blur, float brighten, int flags);
	void DeleteFont( HFont hFont );

	HFont GetFontByName( const char *name );
	void  GetCharABCWide( HFont font, int ch, int &a, int &b, int &c );
	int   GetFontTall( HFont font );
	int   GetFontAscent( HFont font );
	int   GetCharacterWidth( HFont font, int ch );
	bool  GetFontUnderlined( HFont font );

	void  GetTextSize( HFont font, const wchar_t *text, int *wide, int *tall = NULL );
	void  GetTextSize( HFont font, const char *text, int *wide, int *tall = NULL );
	int   GetTextHeight( HFont font, const wchar_t *text );
	int   GetTextHeight( HFont font, const char *text );

	int   GetTextWide( HFont font, const char *text );
	int	  GetTextWide( HFont font, const wchar_t *text );

	int GetTextWideScaled( HFont font, const char *text, const int height );
	int GetTextWideScaled( HFont font, const wchar_t *text, const int height );

	int DrawCharacter(HFont font, wchar_t ch, Point pt, Size sz, const int color );

	void DebugDraw( HFont font );

private:
	IBaseFont *GetIFontFromHandle( HFont font );
	void UploadTextureForFont(IBaseFont *font );

	CUtlVector<IBaseFont*> m_Fonts;
};

extern CFontManager g_FontMgr;

#endif // FONTMANAGER_H
