/*
FontManager.cpp - font manager
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
#include <locale.h>
#include "FontManager.h"
#include "Utils.h"

#include "BaseFontBackend.h"

#  if defined(MAINUI_USE_FREETYPE)
#include "FreeTypeFont.h"
#elif defined(MAINUI_USE_STB)
#include "StbFont.h"
#elif defined(_WIN32)
#include "WinAPIFont.h"
#else
#error "No font rendering backend found"
#endif

#include "BitmapFont.h"

#ifdef __ANDROID__
#define DEFAULT_MENUFONT "RobotoCondensed"
#define DEFAULT_CONFONT  "DroidSans"
#define SCALE_FONTS // Probably isn't a good idea until I don't have implemented SDF
#define DEFAULT_WEIGHT 1000
#else
#define DEFAULT_MENUFONT "Trebuchet MS"
#define DEFAULT_CONFONT  "Tahoma"
#define DEFAULT_WEIGHT 500
#define MIMIC_MAKEFONT_SCALES
#endif

// #define SCALE_FONTS

CFontManager g_FontMgr;

CFontManager::CFontManager()
{
#ifdef MAINUI_USE_FREETYPE
	FT_Init_FreeType( &CFreeTypeFont::m_Library );
#endif
	m_Fonts.EnsureCapacity( 4 );
}

CFontManager::~CFontManager()
{
	DeleteAllFonts();
#ifdef MAINUI_USE_FREETYPE
	FT_Done_FreeType( CFreeTypeFont::m_Library );
	CFreeTypeFont::m_Library = NULL;
#endif
}

void CFontManager::VidInit( void )
{
	static float prevScale = 0;

	bool updateConsoleFont = false; // a hint for re-render console font, because it was removed

	float scale = uiStatic.scaleY;

	if( !prevScale
#ifndef SCALE_FONTS // complete disables font re-rendering
	|| abs( scale - prevScale ) > 0.1f
#endif
	)
	{
		DeleteAllFonts();
		updateConsoleFont = true;
		uiStatic.hDefaultFont = CFontBuilder( DEFAULT_MENUFONT, UI_MED_CHAR_HEIGHT * scale, DEFAULT_WEIGHT )
			.SetHandleNum( QM_DEFAULTFONT )
			.Create();
		uiStatic.hSmallFont   = CFontBuilder( DEFAULT_MENUFONT, UI_SMALL_CHAR_HEIGHT * scale, DEFAULT_WEIGHT )
			.SetHandleNum( QM_SMALLFONT )
			.Create();
		uiStatic.hBigFont     = CFontBuilder( DEFAULT_MENUFONT, UI_BIG_CHAR_HEIGHT * scale, DEFAULT_WEIGHT )
			.SetHandleNum( QM_BIGFONT )
			.Create();

		uiStatic.hBoldFont = CFontBuilder( DEFAULT_MENUFONT, UI_MED_CHAR_HEIGHT * scale, 1000 )
			.SetHandleNum( QM_BOLDFONT )
			.Create();

#ifdef MAINUI_RENDER_PICBUTTON_TEXT
		uiStatic.hLightBlur = CFontBuilder( DEFAULT_MENUFONT, UI_MED_CHAR_HEIGHT * scale, 1000 )
			.SetHandleNum( QM_LIGHTBLUR )
			.SetBlurParams( 2, 1.0f )
			.SetFlags( FONT_ADDITIVE )
			.Create();

		uiStatic.hHeavyBlur = CFontBuilder( DEFAULT_MENUFONT, UI_MED_CHAR_HEIGHT * scale, 1000 )
			.SetHandleNum( QM_HEAVYBLUR )
			.SetBlurParams( 8, 1.75f )
			.SetFlags( FONT_ADDITIVE )
			.Create();
#endif
#ifndef MIMIC_MAKEFONT_SCALES
		uiStatic.hConsoleFont = CFontBuilder( DEFAULT_CONFONT, UI_SMALL_CHAR_HEIGHT * scale, DEFAULT_WEIGHT )
			.SetOutlineSize()
			.Create();
#endif
		prevScale = scale;
	}

#ifdef MIMIC_MAKEFONT_SCALES
	static int prevConsoleFontHeight;
	int consoleFontHeight;

	// makefont rules
	if( ScreenHeight < 320 ) consoleFontHeight = 11;
	else if( ScreenHeight < 640 ) consoleFontHeight = 14;
	else consoleFontHeight = 18;

	if( consoleFontHeight != prevConsoleFontHeight || updateConsoleFont )
	{
		if( uiStatic.hConsoleFont && !updateConsoleFont )
		{
			DeleteFont( uiStatic.hConsoleFont );
			uiStatic.hConsoleFont = 0;
		}

		uiStatic.hConsoleFont = CFontBuilder( DEFAULT_CONFONT, consoleFontHeight, 500 )
			.SetOutlineSize()
			.Create();

		prevConsoleFontHeight = consoleFontHeight;
	}
#endif
}

void CFontManager::DeleteAllFonts()
{
	for( int i = 0; i < m_Fonts.Count(); i++ )
	{
		delete m_Fonts[i];
	}
	m_Fonts.RemoveAll();
}

void CFontManager::DeleteFont(HFont hFont)
{
	IBaseFont *font = GetIFontFromHandle(hFont);
	if( font )
	{
		delete font;

		m_Fonts.FastRemove( hFont - 1 );
	}
}

IBaseFont *CFontManager::GetIFontFromHandle(HFont font)
{
	if( m_Fonts.IsValidIndex( font - 1 ) )
		return m_Fonts[font-1];

	return NULL;
}

void CFontManager::GetCharABCWide(HFont font, int ch, int &a, int &b, int &c)
{
	IBaseFont *pFont = GetIFontFromHandle( font );
	if( pFont )
		pFont->GetCharABCWidths( ch, a, b, c );
	else
		a = b = c = 0;
}

int CFontManager::GetCharacterWidth(HFont font, int ch)
{
	int a, b, c;
	GetCharABCWide( font, ch, a, b, c );
	return a + b + c;
}

HFont CFontManager::GetFontByName(const char *name)
{
	for( int i = 0; i < m_Fonts.Count(); i++ )
	{
		if( !stricmp( name, m_Fonts[i]->GetName() ) )
			return i;
	}
	return -1;
}

int CFontManager::GetFontTall(HFont font)
{
	IBaseFont *pFont = GetIFontFromHandle( font );
	if( pFont )
		return pFont->GetTall();
	return 0;
}

int CFontManager::GetFontAscent(HFont font)
{
	IBaseFont *pFont = GetIFontFromHandle( font );
	if( pFont )
		return pFont->GetAscent();
	return 0;
}

bool CFontManager::GetFontUnderlined(HFont font)
{
	IBaseFont *pFont = GetIFontFromHandle( font );
	if( pFont )
		return pFont->GetFlags() & FONT_UNDERLINE;
	return false;
}

void CFontManager::GetTextSize(HFont fontHandle, const char *text, int *wide, int *tall, int size )
{
	IBaseFont *font = GetIFontFromHandle( fontHandle );

	if( !font || !text || !text[0] )
	{
		if( wide ) *wide = 0;
		if( tall ) *tall = 0;
		return;
	}

	int fontTall = font->GetHeight(), x = 0;
	int _wide = 0, _tall;
	const char *ch = text;
	_tall = fontTall;
	int i = 0;

	EngFuncs::UtfProcessChar( 0 );

	while( *ch && ( size < 0 || i < size ) )
	{
		// Skip colorcodes
		if( IsColorString( ch ) )
		{
			ch += 2;
			continue;
		}

		int uch;

		uch = EngFuncs::UtfProcessChar( (unsigned char)*ch );
		if( uch )
		{
			if( uch == '\n' )
			{
				_tall += fontTall;
				x = 0;
			}
			else
			{
				int a, b, c;
				font->GetCharABCWidths( uch, a, b, c );
				x += a + b + c;
				if( x > _wide )
					_wide = x;
			}
		}
		i++;
		ch++;
	}
	EngFuncs::UtfProcessChar( 0 );

	if( tall ) *tall = _tall;
	if( wide ) *wide = _wide;
}

int CFontManager::CutText(HFont fontHandle, const char *text, int height, int visibleSize )
{
	IBaseFont *font = GetIFontFromHandle( fontHandle );

	if( !font || !text || !text[0] )

		return 0;

	int _wide = 0;
	const char *ch = text;
	int i = 0, x = 0, len = 0, lastlen = 0;
	if( visibleSize <= 0 )
		return 0;
#ifdef SCALE_FONTS
			visibleSize  = (float)visibleSize / height * (float)font->GetTall();
#endif
	EngFuncs::UtfProcessChar( 0 );

	while( *ch && _wide < visibleSize )
	{
		// Skip colorcodes
		if( IsColorString( ch ) )
		{
			ch += 2;
			continue;
		}

		int uch;

		uch = EngFuncs::UtfProcessChar( (unsigned char)*ch );
		if( uch )
		{
			if( uch == '\n' )
			{
				x = 0;
			}
			else
			{
				int a, b, c;
				font->GetCharABCWidths( uch, a, b, c );
				x += a + b + c;
				if( x > _wide )
					_wide = x;
			}
			lastlen = len;
			len = i+1;
		}
		i++;
		ch++;
	}
	EngFuncs::UtfProcessChar( 0 );

	if( !*ch && _wide < visibleSize )
		return len;

	return lastlen;
}


int CFontManager::GetTextWide(HFont font, const char *text, int size)
{
	int wide;

	GetTextSize( font, text, &wide, NULL, size );

	return wide;
}

int CFontManager::GetTextHeight(HFont fontHandle, const char *text, int size )
{
	IBaseFont *font = GetIFontFromHandle( fontHandle );
	if( !font || !text || !text[0] )
	{
		return 0;
	}

	int height = font->GetHeight();

	// lightweight variant only for getting text height
	int i = 0;
	while( *text&&( size < 0 || i < size ) )
	{
		if( *text == '\n' )
			height += height;

		text++;
		i++;
	}
	return height;
}

int CFontManager::GetTextWideScaled(HFont font, const char *text, const int height, int size)
{
	IBaseFont *pFont = GetIFontFromHandle( font );
	if( pFont )
	{
		return GetTextWide( font, text, size )
#ifdef SCALE_FONTS
			* ((float)height / (float)pFont->GetTall())
#endif
		;
	}

	return 0;
}

void CFontManager::UploadTextureForFont(IBaseFont *font)
{
	// upload only latin needed for english and cyrillic needed for russian
	// maybe it would be extended someday...

	IBaseFont::charRange_t range[] =
	{
	{ 33, 126 },			// ascii printable range
	{ 0x0400, 0x045F },		// cyrillic range
	};

	font->UploadGlyphsForRanges( range, sizeof( range ) / sizeof( IBaseFont::charRange_t ) );
}

int CFontManager::DrawCharacter(HFont fontHandle, int ch, Point pt, Size sz, const int color )
{
	IBaseFont *font = GetIFontFromHandle( fontHandle );

	if( !font )
		return 0;

	Size charSize;
	int a, b, c, width;

#ifdef SCALE_FONTS
	float factor = (float)sz.h / (float)font->GetTall();
#endif

	font->GetCharABCWidths( ch, a, b, c );
	width = a + b + c;

	// skip whitespace
	if( ch == ' ' )
	{
#ifdef SCALE_FONTS
		if( sz.h > 0 )
		{
			return width * factor + 0.5f;
		}
		else
#endif
		{
			return width;
		}
	}

	IBaseFont::glyph_t find( ch );
	int idx = font->m_glyphs.Find( find );

	if( font->m_glyphs.IsValidIndex( idx ) )
	{
		IBaseFont::glyph_t &glyph = font->m_glyphs[idx];

		int r, g, b, alpha;

		UnpackRGBA(r, g, b, alpha, color );

#ifdef SCALE_FONTS	// Scale font
		if( sz.h > 0 )
		{
			charSize.w = (glyph.rect.right - glyph.rect.left) * factor + 0.5f;
			charSize.h = font->GetHeight() * factor + 0.5f;
		}
		else
#endif
		{
			charSize.w = glyph.rect.right - glyph.rect.left;
			charSize.h = font->GetHeight();
		}

		pt.x += a;

		EngFuncs::PIC_Set( glyph.texture, r, g, b, alpha );
		if( font->IsAdditive() )
			EngFuncs::PIC_DrawAdditive( pt, charSize, &glyph.rect );
		else
			EngFuncs::PIC_DrawTrans( pt, charSize, &glyph.rect );
	}

#ifdef SCALE_FONTS
	if( sz.h > 0 )
	{
		return width * factor + 0.5f;
	}
#endif
	return width;
}

void CFontManager::DebugDraw(HFont fontHandle)
{
	IBaseFont *font = GetIFontFromHandle(fontHandle);

	font->DebugDraw();
}


HFont CFontBuilder::Create()
{
	IBaseFont *font;

	// check existing font at first
	if( !m_hForceHandle )
	{
		for( int i = 0; i < g_FontMgr.m_Fonts.Count(); i++ )
		{
			font = g_FontMgr.m_Fonts[i];

			if( font->IsEqualTo( m_szName, m_iTall, m_iWeight, m_iBlur, m_iFlags ) )
				return i + 1;
		}
	}

#if defined(MAINUI_USE_FREETYPE)
		font = new CFreeTypeFont();
#elif defined(MAINUI_USE_STB)
		font = new CStbFont();
#else
		font = new CWinAPIFont();
#endif

	double starttime = Sys_DoubleTime();

	if( !font->Create( m_szName, m_iTall, m_iWeight, m_iBlur, m_fBrighten, m_iOutlineSize, m_iScanlineOffset, m_fScanlineScale, m_iFlags ) )
	{
		delete font;
		return -1;
	}

	g_FontMgr.UploadTextureForFont( font );

	double endtime = Sys_DoubleTime();

	Con_DPrintf( "Rendering %s(%i, %i) took %f seconds\n", m_szName, m_iTall, m_iWeight, endtime - starttime );

	if( m_hForceHandle != -1 && g_FontMgr.m_Fonts.Count() != m_hForceHandle )
	{
		if( g_FontMgr.m_Fonts.IsValidIndex( m_hForceHandle ) )
		{
			g_FontMgr.m_Fonts.FastRemove( m_hForceHandle );
			return g_FontMgr.m_Fonts.InsertBefore( m_hForceHandle, font );
		}
	}

	return g_FontMgr.m_Fonts.AddToTail(font) + 1;
}
