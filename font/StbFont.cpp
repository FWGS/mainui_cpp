#if !defined(_WIN32) && defined(MAINUI_USE_STB)
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>

#include "FontManager.h"
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "StbFont.h"
#include "Utils.h"

/* round a 26.6 pixel coordinate to the nearest larger integer */
#define PIXEL(x) ((((x)+63) & -64)>>6)


bool ABCCacheLessFunc( const abc_t &a, const abc_t &b )
{
	return a.ch < b.ch;
}

CStbFont::CStbFont() : IBaseFont(),
	m_ABCCache(0, 0, ABCCacheLessFunc), m_szRealFontFile(), m_pFontData( NULL )
{
}

CStbFont::~CStbFont()
{
	delete m_pFontData;
}

bool CStbFont::FindFontDataFile(const char *name, int tall, int weight, int flags, char *dataFile, int dataFileChars)
{
#if 0
	strncpy( dataFile, "/usr/share/fonts/truetype/msttcorefonts/Trebuchet_MS.ttf", dataFileChars );
	return true;
#else
	// Silly code to load fonts on Android.
	// TODO: Find a way to find fonts on Android
	snprintf( dataFile, dataFileChars, "/system/fonts/%s.ttf", name );

	return true;
#endif
}

bool CStbFont::Create(const char *name, int tall, int weight, int blur, float brighten, int outlineSize, int scanlineOffset, float scanlineScale, int flags)
{
	strncpy( m_szName, name, sizeof( m_szName ) - 1 );
	m_szName[sizeof( m_szName ) - 1] = 0;
	m_iTall = tall;
	m_iWeight = weight;
	m_iFlags = flags;

	m_iBlur = blur;
	m_fBrighten = brighten;

	m_iOutlineSize = outlineSize;

	m_iScanlineOffset = scanlineOffset;
	m_fScanlineScale = scanlineScale;


	if( !FindFontDataFile( name, tall, weight, flags, m_szRealFontFile, 4096 ) )
	{
		Con_DPrintf( "Unable to find font named %s\n", name );
		m_szName[0] = 0;
		return false;
	}


	// EngFuncs::COM_LoadFile does not allow open files from /
	FILE *fd = fopen( m_szRealFontFile, "r" );
	if( !fd )
	{
		Con_DPrintf( "Unable to open font %s!\n", m_szRealFontFile );
		return false;
	}

	fseek( fd, 0, SEEK_END );
	size_t len = ftell( fd );
	fseek( fd, 0, SEEK_SET );

	m_pFontData = new byte[len+1];
	size_t red = fread( m_pFontData, 1, len, fd );
	if( red != len )
	{
		Con_DPrintf( "Unable to read font file %s!\n", m_szRealFontFile );
		return false;
	}

	if( !stbtt_InitFont( &m_fontInfo, m_pFontData, 0 ) )
	{
		Con_DPrintf( "Unable to create font %s!\n", m_szRealFontFile );
		m_szName[0] = 0;
		return false;
	}
	scale = stbtt_ScaleForPixelHeight(&m_fontInfo, tall);
	int x0, y0, x1, y1;
	stbtt_GetFontBoundingBox( &m_fontInfo, &x0, &y0, &x1, &y1 );

	stbtt_GetFontVMetrics(&m_fontInfo, &m_iAscent, NULL, NULL );

	m_iAscent *= scale;


	m_iHeight = (( y1 - y0 ) * scale); // maybe wrong!
	m_iMaxCharWidth = (( x1 - x0 ) * scale); // maybe wrong!

	CreateGaussianDistribution();

	return true;
}

void CStbFont::GetCharRGBA(int ch, Point pt, Size sz, unsigned char *rgba, Size &drawSize )
{
	byte *buf, *dst;
	int a, b, c;

	GetCharABCWidths( ch, a, b, c ); // speed up cache

	int bm_top, bm_left, bm_rows, bm_width;

	buf = stbtt_GetCodepointBitmap( &m_fontInfo, scale, scale, ch, &bm_width, &bm_rows, &bm_left, &bm_top );

	// see where we should start rendering
	const int pushDown = m_iAscent + bm_top;
	const int pushLeft = bm_left;

	// set where we start copying from
	int ystart = 0;
	if( pushDown < 0 )
		ystart = -pushDown;

	int xstart = 0;
	if( pushLeft < 0 )
		xstart = -pushLeft;

	int yend = bm_rows;
	if( pushDown + yend > sz.h )
		yend += sz.h - ( pushDown + yend );

	int xend = bm_width;
	if( pushLeft + xend > sz.w )
		xend += sz.w - ( pushLeft + xend );

	buf = &buf[ ystart * bm_width ];
	dst = rgba + 4 * sz.w * ( ystart + pushDown );

	bool additive = IsAdditive();

	// iterate through copying the generated dib into the texture
	for (int j = ystart; j < yend; j++, dst += 4 * sz.w, buf += bm_width )
	{
		uint32_t *xdst = (uint32_t*)(dst + 4 * ( m_iBlur + m_iOutlineSize ));
		for (int i = xstart; i < xend; i++, xdst++)
		{
			if( buf[i] > 0 )
			{
				// paint white and alpha
				if( !additive )
				{
					// paint white and alpha
					*xdst = PackRGBA( 0xFF, 0xFF, 0xFF, buf[i] );
				}
				else
				{
					*xdst = PackRGBA( buf[i], buf[i], buf[i], 0xFF );
				}
			}
			else
			{
				// paint black and null alpha
				*xdst = PackRGBA( 0x00, 0x00, 0x00, additive ? 0xFF : 0x00 );
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

bool CStbFont::IsValid()  const
{
	return (bool)m_szName[0];
}

void CStbFont::GetCharABCWidths(int ch, int &a, int &b, int &c)
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

	stbtt_GetCodepointHMetrics( &m_fontInfo, ch, &find.b, &find.a );
	find.c = 0;
	find.a *= scale;
	find.b *= scale;

	a = find.a;
	b = find.b;
	c = find.c;

	m_ABCCache.Insert(find);
}

bool CStbFont::HasChar(int ch) const
{
	return true;
}

#endif // WIN32 && MAINUI_USE_FREETYPE
