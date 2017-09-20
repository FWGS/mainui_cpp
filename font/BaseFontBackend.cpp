#include "BaseFontBackend.h"
#include "FontManager.h"
#include <math.h>
#include "Utils.h"

#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif

#ifndef M_E
# define M_E		2.7182818284590452354	/* e */
#endif

bool IBaseFont::GlyphLessFunc( const glyph_t &a, const glyph_t &b )
{
	return a.ch < b.ch;
}

IBaseFont::IBaseFont()
	: m_szName(), m_iTall(), m_iWeight(), m_iFlags(),
	m_iHeight(), m_iMaxCharWidth(), m_iAscent(),
	m_iGLTexture(), m_iBlur(), m_pGaussianDistribution(), m_fBrighten(),
	m_glyphs(0, 0, GlyphLessFunc), m_iPages()
{
}


/*
=========================
IBaseFont::GetTextureName

Mangle texture name, so using same font names with different attributes will not confuse engine or font renderer
=========================
+*/
void IBaseFont::GetTextureName(char *dst, size_t len, int pageNum) const
{
	char attribs[256];
	int i = 0;
	if( GetFlags() & FONT_ITALIC ) attribs[i++] = 'i'; // 1 parameter
	if( GetFlags() & FONT_UNDERLINE ) attribs[i++] = 'u'; // 1 parameter
	if( IsAdditive() ) attribs[i++] = 'a'; // 1
	if( m_iBlur )
	{
		int chars = snprintf( attribs + i, sizeof( attribs ) - 1 - i, "g%i%.2f", m_iBlur, m_fBrighten );
		i += chars;
	}
	if( m_iOutlineSize )
	{
		int chars = snprintf( attribs + i, sizeof( attribs ) - 1 - i, "o%i", m_iOutlineSize );
		i += chars;
	}
	if( m_iScanlineOffset )
	{
		int chars = snprintf( attribs + i, sizeof( attribs ) - 1 - i, "s%i%.2f", m_iScanlineOffset, m_fScanlineScale );
		i += chars;
	}
	attribs[i] = 0;

	if( i == 0 )
	{
		snprintf( dst, len - 1, "%s%i_%i_%i_font.bmp", GetName(), pageNum, GetTall(), GetWeight() );
		dst[len - 1] = 0;
	}
	else
	{
		attribs[i] = 0;
		snprintf( dst, len - 1, "%s%i_%i_%i_%s_font.bmp", GetName(), pageNum, GetTall(), GetWeight(), attribs );
		dst[len - 1] = 0;
	}
}

#define MAX_PAGE_SIZE 256

void IBaseFont::UploadGlyphsForRanges(IBaseFont::charRange_t *range, int rangeSize)
{
	char name[256];
	const int maxWidth = GetMaxCharWidth();
	const int height = GetHeight();

	// allocate temporary buffer for max possible glyph size
	const int tempSize = maxWidth * height * 4;

	int bmpSize;
	byte *bmp, *temp = new byte[tempSize],
		 *rgbdata = UI::Graphics::MakeBMP( MAX_PAGE_SIZE, MAX_PAGE_SIZE, &bmp, &bmpSize, NULL );

	Size tempDrawSize( maxWidth, height );
	const Point nullPt( 0, 0 );

	// texture is reversed by Y coordinates
	int xstart = 0, ystart = MAX_PAGE_SIZE-1;
	for( int iRange = 0; iRange < rangeSize; iRange++ )
	{
		for( int ch = range[iRange].chMin; ch <= range[iRange].chMax; ch++ )
		{
			// clear temporary buffer
			memset( temp, 0, tempSize );

			// draw it to temp buffer
			Size drawSize;
			GetCharRGBA( ch, nullPt, tempDrawSize, temp, drawSize );

			// see if we need to go down or create a new page
			if( xstart + drawSize.w > MAX_PAGE_SIZE )
			{
				xstart = 0;
				ystart -= height + 1; // HACKHACK: Add more space between rows, this removes ugly 1 height pixel rubbish
				// No free space now
				if( ystart - height - 1 <= 0 )
				{
					GetTextureName( name, sizeof( name ), m_iPages );
					HIMAGE hImage = EngFuncs::PIC_Load( name, bmp, bmpSize, 0 );
					Con_DPrintf( "Uploaded %s to %i\n", name, hImage );

					for( int i = m_glyphs.FirstInorder(); ;
						i = m_glyphs.NextInorder( i ) )
					{
						glyph_t &glyph = m_glyphs[i];
						if( !glyph.texture )
							glyph.texture = hImage;
						if( i == m_glyphs.LastInorder() )
							break;
					}

					// m_Pages.AddToTail( hImage );
					m_iPages++;
					memset( rgbdata, 0, MAX_PAGE_SIZE * MAX_PAGE_SIZE * 4 );
					ystart = MAX_PAGE_SIZE-1;
				}
			}

			// set rgbdata rect
			wrect_t rect;
			rect.top    = MAX_PAGE_SIZE - ystart;
			rect.bottom = MAX_PAGE_SIZE - ystart + height;
			rect.left   = xstart;
			rect.right  = xstart + drawSize.w;

			// copy glyph to rgbdata

			for( int y = 0; y < height; y++ )
			{
				byte *dst = &rgbdata[(ystart - y) * MAX_PAGE_SIZE * 4];
				byte *src = &temp[y * maxWidth * 4];
				for( int x = 0; x < drawSize.w; x++ )
				{
					byte *xdst = &dst[ ( xstart + x ) * 4 ];
					byte *xsrc = &src[ x * 4 ];

					// copy 4 bytes: R, G, B and A
					memcpy( xdst, xsrc, 4 );
				}
			}

			// move xstart
			xstart += drawSize.w;

			glyph_t glyph;
			glyph.ch = ch;
			glyph.rect = rect;
			glyph.texture = 0; // will be acquired later

			m_glyphs.Insert( glyph );
		}
	}

	GetTextureName( name, sizeof( name ), m_iPages );
	HIMAGE hImage = EngFuncs::PIC_Load( name, bmp, bmpSize, 0 );
	Con_DPrintf( "Uploaded %s to %i\n", name, hImage );
	delete[] bmp;
	delete[] temp;

	for( int i = m_glyphs.FirstInorder();; i = m_glyphs.NextInorder( i ) )
	{
		if( !m_glyphs[i].texture )
			m_glyphs[i].texture = hImage;
		if( i == m_glyphs.LastInorder() )
			break;
	}
	// m_Pages.AddToTail(hImage);
	m_iPages++;
}


IBaseFont::~IBaseFont()
{
	char name[256];
	for( int i = 0; i < m_iPages; i++ )
	{
		GetTextureName( name, sizeof( name ), i );
		EngFuncs::PIC_Free( name );
	}
	m_iPages = 0;

	delete[] m_pGaussianDistribution;
}

bool IBaseFont::IsEqualTo(const char *name, int tall, int weight, int blur, int flags)  const
{
	if( stricmp( name, m_szName ))
		return false;

	if( m_iTall != tall )
		return false;

	if( m_iWeight != weight )
		return false;

	if( m_iBlur != blur )
		return false;

	if( m_iFlags != flags )
		return false;

	return true;
}

void IBaseFont::DebugDraw() const
{
	HIMAGE hImage;
	char name[256];

	for( int i = 0; i < m_iPages; i++ )
	{
		int x = i * MAX_PAGE_SIZE;
		GetTextureName( name, sizeof( name ), i );

		hImage = EngFuncs::PIC_Load( name );

		EngFuncs::PIC_Set( hImage, 255, 255, 255 );
		if( IsAdditive() )
			EngFuncs::PIC_DrawAdditive(  Point(x, 0), Size( MAX_PAGE_SIZE, MAX_PAGE_SIZE ) );
		else
			EngFuncs::PIC_DrawTrans( Point(x, 0), Size( MAX_PAGE_SIZE, MAX_PAGE_SIZE ) );
	}
}

void IBaseFont::ApplyBlur(Size rgbaSz, byte *rgba)
{
	if( !m_pGaussianDistribution )
		return;

	const int size = rgbaSz.w * rgbaSz.h * 4;
	byte *src = new byte[size];
	memcpy( src, rgba, size );

	for( int y = 0; y < rgbaSz.h; y++ )
	{
		for( int x = 0; x < rgbaSz.w; x++ )
		{
			GetBlurValueForPixel( src, Point(x, y), rgbaSz, rgba );

			rgba += 4;
		}
	}

	delete[] src;
}

void IBaseFont::GetBlurValueForPixel(byte *src, Point srcPt, Size srcSz, byte *dest)
{
	float accum = 0.0f;
	bool additive = IsAdditive();

	// scan the positive x direction
	int maxX = min( srcPt.x + m_iBlur, srcSz.w );
	int minX = max( srcPt.x - m_iBlur, 0 );
	for( int x = minX; x <= maxX; x++ )
	{
		int maxY = min( srcPt.y + m_iBlur, srcSz.h - 1);
		int minY = max( srcPt.y - m_iBlur, 0);
		for (int y = minY; y <= maxY; y++)
		{
			byte *srcPos = src + ((x + (y * srcSz.w)) * 4);

			// muliply by the value matrix
			float weight = m_pGaussianDistribution[x - srcPt.x + m_iBlur];
			float weight2 = m_pGaussianDistribution[y - srcPt.y + m_iBlur];
			accum += ( additive ? srcPos[0] : srcPos[3] ) * (weight * weight2);
		}
	}

	// all the values are the same for fonts, just use the calculated alpha
	if( !additive )
	{
		dest[0] = dest[1] = dest[2] = 255;
		dest[3] = min( (int)accum, 255);
	}
	else
	{
		dest[0] = dest[1] = dest[2] = min( (int)accum, 255 );
		dest[3] = 255;
	}
}

void IBaseFont::CreateGaussianDistribution()
{
	double sigma2;
	if( m_iBlur < 1 )
		return;

	sigma2 = 0.683 * m_iBlur;
	sigma2 *= sigma2;
	m_pGaussianDistribution = new float[m_iBlur * 2 + 1];
	for( int x = 0; x <= m_iBlur * 2; x++ )
	{
		int val = x - m_iBlur;
		m_pGaussianDistribution[x] = (float)(1.0f / sqrt(2 * M_PI * sigma2)) * pow(M_E, -1 * (val * val) / (2 * sigma2));

		// brightening factor
		m_pGaussianDistribution[x] *= m_fBrighten;
	}
}

void IBaseFont::ApplyOutline(Point pt, Size rgbaSz, byte *rgba)
{
	if( !m_iOutlineSize )
		return;

	int x, y;

	for( y = pt.x; y < rgbaSz.h; y++ )
	{
		for( x = pt.y; x < rgbaSz.w; x++ )
		{
			byte *src = &rgba[(x + (y * rgbaSz.w)) * 4];

			if( src[3] != 0 )
				continue;

			int shadowX, shadowY;

			for( shadowX = -m_iOutlineSize; shadowX <= m_iOutlineSize; shadowX++ )
			{
				for( shadowY = -m_iOutlineSize; shadowY <= m_iOutlineSize; shadowY++ )
				{
					if( !shadowX && !shadowY )
						continue;

					int testX = shadowX + x, testY = shadowY + y;
					if( testX < 0 || testX >= rgbaSz.w ||
						testY < 0 || testY >= rgbaSz.h )
						continue;

					byte *test = &rgba[(testX + (testY * rgbaSz.w)) * 4];
					if( test[0] == 0 || test[1] == 0 || test[3] == 0 )
						continue;

					src[0] = src[1] = src[2] = 0;
					src[3] = -1;
				}
			}
		}
	}
}

void IBaseFont::ApplyScanline(Size rgbaSz, byte *rgba)
{
	if( m_iScanlineOffset < 2 )
		return;

	for( int y = 0; y < rgbaSz.h; y++ )
	{
		if( y % m_iScanlineOffset == 0 )
			continue;

		byte *src = &rgba[(y * rgbaSz.w) * 4];
		for( int x = 0; x < rgbaSz.w; x++, src += 4 )
		{
			src[0] *= m_fScanlineScale;
			src[1] *= m_fScanlineScale;
			src[2] *= m_fScanlineScale;
		}
	}
}

void IBaseFont::ApplyStrikeout(Size rgbaSz, byte *rgba)
{
	if( !(m_iFlags & FONT_STRIKEOUT) )
		return;

	const int y = rgbaSz.h * 0.5f;

	byte *src = &rgba[(y*rgbaSz.w) * 4];

	for( int x = 0; x < rgbaSz.w; x++, src += 4 )
	{
		src[0] = src[1] = src[2] = 127;
		src[3] = 255;
	}
}
