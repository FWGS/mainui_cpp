#ifndef _WIN32
#include <stdarg.h>
#include <stdint.h>

#include "FontManager.h"
#include "FreeTypeFont.h"

FT_Library CFreeTypeFont::m_Library;

/* round a 26.6 pixel coordinate to the nearest larger integer */
#define PIXEL(x) ((((x)+63) & -64)>>6)


bool ABCCacheLessFunc( const abc_t &a, const abc_t &b )
{
	return a.ch < b.ch;
}

CFreeTypeFont::CFreeTypeFont() : IBaseFont(),
	m_iOutlineSize(),
	m_bAntialiased(), m_bAdditive(),
	m_ABCCache(0, 0, ABCCacheLessFunc), face(), m_szRealFontFile()
{
}

CFreeTypeFont::~CFreeTypeFont()
{
}

/**
 * Find a matching font where @type (one of FC_*) is equal to @value. For a
 * list of types, see http://fontconfig.org/fontconfig-devel/x19.html#AEN27.
 * The variable arguments are a list of triples, just like the first three
 * arguments, and must be NULL terminated.
 *
 * For example,
 *   FontMatchString(FC_FILE, FcTypeString, "/usr/share/fonts/myfont.ttf", NULL);
 *
 * Ripped from skia source code
 */
FcPattern* FontMatch(const char* type, FcType vtype, const void* value, ...)
{
	va_list ap;
	va_start(ap, value);

	FcPattern* pattern = FcPatternCreate();

	for (;;) {
		FcValue fcvalue;
		fcvalue.type = vtype;
		switch (vtype) {
			case FcTypeString:
				fcvalue.u.s = (FcChar8*) value;
				break;
			case FcTypeInteger:
				fcvalue.u.i = (int)(intptr_t)value;
				break;
			default:
				assert(("FontMatch unhandled type"));
		}
		FcPatternAdd(pattern, type, fcvalue, FcFalse);

		type = va_arg(ap, const char *);
		if (!type)
			break;
		// FcType is promoted to int when passed through ...
		vtype = static_cast<FcType>(va_arg(ap, int));
		value = va_arg(ap, const void *);
	};
	va_end(ap);

	FcConfigSubstitute(NULL, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	FcResult result;
	FcPattern* match = FcFontMatch(NULL, pattern, &result);
	FcPatternDestroy(pattern);

	return match;
}

bool CFreeTypeFont::FindFontDataFile( const char *name, int tall, int weight, int flags, char *dataFile, int dataFileChars )
{
	int nFcWeight = weight / 5;
	FcPattern *pattern;
	FcChar8 *filename;

	bool bRet;

	if( !FcInit() )
		return false;

	int slant = FC_SLANT_ROMAN;
	if( flags & ( FONT_ITALIC ))
		slant = FC_SLANT_ITALIC;

	pattern = FontMatch(
		"family", FcTypeString, name,
		"weight", FcTypeInteger, nFcWeight,
		"slant",  FcTypeInteger, slant, NULL );

	if( !pattern )
		return false;

	if( !FcPatternGetString( pattern, "file", 0, &filename ) )
	{
		bRet = true;
		strncpy( dataFile, (char*)filename, dataFileChars );
		dataFile[dataFileChars-1] = 0;
	}
	else bRet = false;

	FcPatternDestroy( pattern );
	return bRet;
}

bool CFreeTypeFont::Create(const char *name, int tall, int weight, int blur, float brighten, int flags)
{
	strncpy( m_szName, name, sizeof( m_szName ) - 1 );
	m_szName[sizeof( m_szName ) - 1] = 0;
	m_iTall = tall;
	m_iWeight = weight;
	m_iFlags = flags;
	m_bAntialiased = flags & FONT_ANTIALIAS;
	m_bUnderlined = flags & FONT_UNDERLINE;
	if( flags & FONT_GAUSSBLUR )
	{
		m_iBlur = blur;
		m_fBrighten = brighten;
	}
	m_iOutlineSize = 0;


	if( !FindFontDataFile( name, tall, weight, flags, m_szRealFontFile, 4096 ) )
	{
		Con_DPrintf( "Unable to find font named %s\n", name );
		m_szName[0] = 0;
		return false;
	}

	if( FT_New_Face( m_Library, m_szRealFontFile, 0, &face ))
	{
		return false;
	}

	FT_Set_Pixel_Sizes( face, 0, tall - 1 );
	m_iAscent = PIXEL(face->size->metrics.ascender );
	m_iHeight = 2 * m_iOutlineSize + PIXEL( face->size->metrics.height );
	m_iMaxCharWidth = PIXEL(face->size->metrics.max_advance );

	CreateGaussianDistribution();

	return true;
}

void CFreeTypeFont::GetCharRGBA(int ch, Point pt, Size sz, unsigned char *rgba, Size &drawSize )
{
	FT_UInt idx = FT_Get_Char_Index( face, ch );
	FT_Error error;
	FT_GlyphSlot slot;
	byte *buf, *dst;

	if( ( error = FT_Load_Glyph( face, idx, FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT ) ) )
	{
		printf( "Error in FT_Load_Glyph: %x\n", error );
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
	if( slot->bitmap_left < 0 )
		xstart = -pushLeft;

	int yend = slot->bitmap.rows;
	if( pushDown + yend > sz.h )
		yend += sz.h - ( pushDown + yend );

	int xend = slot->bitmap.width + pushLeft;
	if( pushLeft + xend > sz.w )
		xend += sz.w - ( pushLeft + xend );

	buf = &slot->bitmap.buffer[ ystart * slot->bitmap.width ];
	dst = rgba + 4 * sz.w * ( ystart + pushDown );
	// iterate through copying the generated dib into the texture
	for (int j = ystart; j < yend; j++)
	{
		byte *xdst = dst + 4 * ( xstart + m_iBlur + pushLeft );
		for (int i = xstart; i < xend; i++)
		{
			xdst[0] = -1;
			xdst[1] = -1;
			xdst[2] = -1;
			xdst[3] = buf[i];

			xdst += 4;
		}


		buf += slot->bitmap.width;
		dst += 4 * sz.w;
	}

	drawSize.w = xend - xstart;
	drawSize.h = yend - ystart;

	ApplyBlurToTexture( pt, sz, rgba );
}

bool CFreeTypeFont::IsValid()  const
{
	return (bool)m_szName[0];
}

void CFreeTypeFont::GetCharABCWidths(int ch, int &a, int &b, int &c)
{
	assert( IsValid() );

	abc_t find;
	find.ch = ch;

	unsigned short i = m_ABCCache.Find( find );
	if( i != 65535 && m_ABCCache.IsValidIndex(i) )
	{
		a = m_ABCCache[i].a;
		b = m_ABCCache[i].b;
		c = m_ABCCache[i].c;
		return;
	}

	// not found in cache

	if( FT_Load_Char( face, ch, FT_LOAD_DEFAULT ) )
	{
		find.a = 0;
		find.b = PIXEL(face->bbox.xMax) - m_iBlur;
		find.c = 0;
	}
	else
	{
		find.a = PIXEL(face->glyph->metrics.horiBearingX) - m_iBlur;
		find.b = PIXEL(face->glyph->metrics.width) + m_iBlur * 2;
		find.c = PIXEL(face->glyph->metrics.horiAdvance -
			 face->glyph->metrics.horiBearingX -
			 face->glyph->metrics.width) - m_iBlur;
		/*find.a = find.c = 0;
		find.b = (PIXEL(face->glyph->metrics.horiAdvance - face->glyph->metrics.horiBearingX - face->glyph->metrics.width) +
			PIXEL(face->glyph->metrics.width) + (m_iBlur * 2));*/

		/*find.a = face->glyph->metrics.horiBearingX / 64.0;
		find.b = face->glyph->metrics.width / 64.0;
		find.c = (face->glyph->metrics.horiAdvance - face->glyph->metrics.horiBearingX - face->glyph->metrics.width) / 64;*/
	}

	a = find.a;
	b = find.b;
	c = find.c;

	m_ABCCache.Insert(find);
}

bool CFreeTypeFont::HasChar(int ch) const
{
	return FT_Get_Char_Index( face, ch ) != 0;
}



#endif // WIN32


