/*
FreeTypeFont.cpp -- freetype2 font renderer
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
#if !defined(_WIN32) && defined(MAINUI_USE_FREETYPE)
#include <stdarg.h>
#include <stdint.h>

#include "FontManager.h"
#include "FreeTypeFont.h"
#include "Utils.h"

FT_Library CFreeTypeFont::m_Library;

/* round a 26.6 pixel coordinate to the nearest larger integer */
#define PIXEL(x) ((((x)+63) & -64)>>6)


CFreeTypeFont::CFreeTypeFont() : CBaseFont(),
	face()
{

}

CFreeTypeFont::~CFreeTypeFont()
{
}

bool CFreeTypeFont::FindFontDataFile( const char *name, int tall, int weight, int flags, char *dataFile, int dataFileChars )
{
	if( !strcmp( name, "Trebuchet MS" ))
	{
		Q_strncpy( dataFile, "gfx/fonts/FiraSans-Regular.ttf", dataFileChars );
		return true;
	}
	else if( !strcmp( name, "Tahoma" ))
	{
		Q_strncpy( dataFile, "gfx/fonts/tahoma.ttf", dataFileChars );
		return true;
	}

	return false;
}

bool CFreeTypeFont::Create(const char *name, int tall, int weight, int blur, float brighten, int outlineSize, int scanlineOffset, float scanlineScale, int flags)
{
	char font_face_path[256];
	int font_face_length;

	Q_strncpy( m_szName, name, sizeof( m_szName ) );
	m_iTall = tall;
	m_iWeight = weight;
	m_iFlags = flags;

	m_iBlur = blur;
	m_fBrighten = brighten;

	m_iOutlineSize = outlineSize;

	m_iScanlineOffset = scanlineOffset;
	m_fScanlineScale = scanlineScale;

	if( !FindFontDataFile( name, tall, weight, flags, font_face_path, sizeof( font_face_path )))
	{
		Con_Printf( "Unable to find font named %s\n", name );
		m_szName[0] = 0;
		return false;
	}

	m_pFontData = g_FontMgr->LoadFontDataFile( font_face_path, &font_face_length );

	if( !m_pFontData )
	{
		Con_Printf( "Unable to read font file %s!\n", font_face_path );
		return false;
	}

	if( FT_New_Memory_Face( m_Library, m_pFontData, font_face_length, 0, &face ))
	{
		Con_Printf( "Unable to create font %s!\n", font_face_path );
		return false;
	}

	FT_Set_Pixel_Sizes( face, 0, tall );
	m_iAscent = PIXEL(face->size->metrics.ascender );
	m_iHeight = PIXEL( face->size->metrics.height );
	m_iMaxCharWidth = PIXEL(face->size->metrics.max_advance );

	return true;
}

void CFreeTypeFont::GetCharRGBA(int ch, Point pt, Size sz, unsigned char *rgba, Size &drawSize )
{
	FT_UInt idx = FT_Get_Char_Index( face, ch );
	FT_Error error;
	FT_GlyphSlot slot;
	byte *buf, *dst;
	int a, b, c;

	GetCharABCWidths( ch, a, b, c );

	if( ( error = FT_Load_Glyph( face, idx, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL ) ) )
	{
		Con_Printf( "Error in FT_Load_Glyph: %x\n", error );
		return;
	}

	slot = face->glyph;
	buf = slot->bitmap.buffer;
	dst = rgba;

	// see where we should start rendering
	const int pushDown = m_iAscent - slot->bitmap_top;
	const int pushLeft = slot->bitmap_left;

	// set where we start copying from
	int ystart = 0;
	if( pushDown < 0 )
		ystart = -pushDown;

	int xstart = 0;
	if( pushLeft < 0 )
		xstart = -pushLeft;

	int yend = slot->bitmap.rows;
	if( pushDown + yend > sz.h )
		yend += sz.h - ( pushDown + yend );

	int xend = slot->bitmap.width;
	if( pushLeft + xend > sz.w )
		xend += sz.w - ( pushLeft + xend );

	buf = &slot->bitmap.buffer[ ystart * slot->bitmap.width ];
	dst = rgba + 4 * sz.w * ( ystart + pushDown );

	// iterate through copying the generated dib into the texture
	for (int j = ystart; j < yend; j++, dst += 4 * sz.w, buf += slot->bitmap.width )
	{
		uint32_t *xdst = (uint32_t*)(dst + 4 * ( m_iBlur + m_iOutlineSize ));
		for (int i = xstart; i < xend; i++, xdst++)
		{
			if( buf[i] > 0 )
			{
				// paint white and alpha
				*xdst = PackRGBA( 0xFF, 0xFF, 0xFF, buf[i] );
			}
			else
			{
				// paint black and null alpha
				*xdst = 0;
			}
		}
	}

	drawSize.w = xend - xstart + m_iBlur * 2 + m_iOutlineSize * 2;
	drawSize.h = yend - ystart + m_iBlur * 2 + m_iOutlineSize * 2;

	ApplyBlur( sz, rgba );
	ApplyOutline( Point( xstart, ystart ), sz, rgba );
	ApplyScanline( sz, rgba );
	ApplyStrikeout( sz, rgba );
}

void CFreeTypeFont::GetCharABCWidthsNoCache(int ch, int &a, int &b, int &c)
{
	if( FT_Load_Char( face, ch, FT_LOAD_DEFAULT ) )
	{
		a = 0;
		b = PIXEL(face->bbox.xMax);
		c = 0;
	}
	else
	{
		a = PIXEL(face->glyph->metrics.horiBearingX);
		b = PIXEL(face->glyph->metrics.width);
		c = PIXEL(face->glyph->metrics.horiAdvance -
			 face->glyph->metrics.horiBearingX -
			 face->glyph->metrics.width);
	}
}

bool CFreeTypeFont::HasChar(int ch) const
{
	return FT_Get_Char_Index( face, ch ) != 0;
}

#endif // WIN32 && MAINUI_USE_FREETYPE
