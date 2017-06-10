#include "BaseFontBackend.h"
#include <cmath>
#include "Utils.h"

bool GlyphLessFunc( const IBaseFont::glyph_t &a, const IBaseFont::glyph_t &b )
{
	return a.ch < b.ch;
}

IBaseFont::IBaseFont()
	: m_szName(), m_iTall(), m_iWeight(), m_iFlags(),
	m_iHeight(), m_iMaxCharWidth(), m_bUnderlined(), m_iAscent(),
	m_iGLTexture(), m_iBlur(), m_pGaussianDistribution(), m_fBrighten(),
	m_glyphs(0, 0, GlyphLessFunc), m_iPages()
{
}

void IBaseFont::ApplyBlurToTexture(Point rgbaPt, Size rgbaSz, unsigned char *rgba)
{
	if( !m_pGaussianDistribution )
		return;

	const int size = rgbaSz.w * rgbaSz.h * 4;
	unsigned char *src = new unsigned char[size];
	memcpy( src, rgba, size );
	unsigned char *dest = rgba;

	for( int y = 0; y < rgbaSz.h; y++ )
	{
		for( int x = 0; x < rgbaSz.w; x++, dest += 4 )
		{
			GetBlurValueForPixel( rgba, Point(x, y), rgbaSz, dest );
		}
	}

	delete[] src;
}

void IBaseFont::GetBlurValueForPixel(unsigned char *src, Point srcPt, Size srcSz, unsigned char *dest)
{
	float accum;

	accum = 0.0f;

	// scan the positive x direction
	int maxX = min( srcPt.x + m_iBlur, srcSz.w );
	int minX = max( srcPt.x - m_iBlur, 0 );
	for( int x = minX; x <= maxX; x++ )
	{
		int maxY = min( srcPt.y + m_iBlur, srcSz.h - 1);
		int minY = max( srcPt.y - m_iBlur, 0);
		for (int y = minY; y <= maxY; y++)
		{
			unsigned char *srcPos = src + ((x + (y * srcSz.w)) * 4);

			// muliply by the value matrix
			float weight = m_pGaussianDistribution[x - srcPt.x + m_iBlur];
			float weight2 = m_pGaussianDistribution[y - srcPt.y + m_iBlur];
			accum += (srcPos[0] * (weight * weight2));
		}
	}

	// all the values are the same for fonts, just use the calculated alpha
	dest[0] = dest[1] = dest[2] = 255;
	dest[3] = min( (int)accum, 255);
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

void IBaseFont::GetTextureName(char *dst, size_t len, int pageNum) const
{
	char attribs[4];
	int i = 0;
	if( GetFlags() & FONT_ITALIC ) attribs[i++] = 'i';
	if( GetFlags() & FONT_UNDERLINE ) attribs[i++] = 'u';
	if( GetFlags() & FONT_GAUSSBLUR ) attribs[i++] = 'g';

	if( i == 0 )
	{
		snprintf( dst, len, "%s%i_%i_%i_font.bmp", GetName(), pageNum, GetTall(), GetWeight() );
	}
	else
	{
		attribs[i] = 0;
		snprintf( dst, len, "%s%i_%i_%i_%s_font.bmp", GetName(), pageNum, GetTall(), GetWeight(), attribs );
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
	int lastCharPageChanged = range[0].chMin;
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
				ystart -= height;
				// No free space now
				if( ystart - height <= 0 )
				{
					GetTextureName( name, sizeof( name ), m_iPages );
					HIMAGE hImage = EngFuncs::PIC_Load( name, bmp, bmpSize, PIC_KEEP_RGBDATA );
					Con_DPrintf( "Uploaded %s to %i\n", name, hImage );

					for( int i = lastCharPageChanged; i < ch; i++ )
					{
						glyph_t find = { i };
						int idx = m_glyphs.Find( find );
						m_glyphs[idx].texture = hImage;
					}

					lastCharPageChanged = ch;

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
	HIMAGE hImage = EngFuncs::PIC_Load( name, bmp, bmpSize, PIC_KEEP_RGBDATA );
	Con_DPrintf( "Uploaded %s to %i\n", name, hImage );
	delete[] bmp;
	delete[] temp;

	for( int i = lastCharPageChanged; i < range[rangeSize-1].chMin; i++ )
	{
		glyph_t find = { i };
		int idx = m_glyphs.Find( find );
		m_glyphs[idx].texture = hImage;
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

bool IBaseFont::GetUnderlined() const
{
	return m_bUnderlined;
}

const char *IBaseFont::GetName() const
{
	return m_szName;
}

int IBaseFont::GetHeight() const
{
	return m_iHeight;
}

int IBaseFont::GetAscent() const
{
	return m_iAscent;
}

int IBaseFont::GetMaxCharWidth() const
{
	return m_iMaxCharWidth;
}

int IBaseFont::GetFlags() const
{
	return m_iFlags;
}

int IBaseFont::GetWeight() const
{
	return m_iWeight;
}

int IBaseFont::GetTall() const
{
	return m_iTall;
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
		EngFuncs::PIC_DrawTrans( Point(x, 0), Size( MAX_PAGE_SIZE, MAX_PAGE_SIZE ) );
	}
}
