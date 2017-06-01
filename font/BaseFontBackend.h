#pragma once
#ifndef BASEFONT_H
#define BASEFONT_H

#include "BaseMenu.h"
#include "utlrbtree.h"

class IBaseFont
{
public:
	IBaseFont();
	virtual ~IBaseFont( ) = 0;

	virtual bool Create(
		const char *name,
		int tall, int weight,
		int blur, float brighten,
		int flags ) = 0;
	virtual void GetCharRGBA( int ch, Point pt, Size sz, unsigned char *rgba, Size &drawSize ) = 0;
	virtual bool IsValid() const = 0;
	virtual void GetCharABCWidths( int ch, int &a, int &b, int &c ) = 0;
	virtual bool HasChar( int ch ) const = 0;

	int GetHeight() const;
	int GetTall() const;
	const char *GetName() const;
	int GetAscent() const;
	int GetMaxCharWidth() const;
	int GetFlags() const;
	int GetWeight() const;
	bool GetUnderlined() const;
	bool IsEqualTo( const char *name, int tall, int weight, int blur, int flags ) const;

	void DebugDraw() const;

	struct charRange_t
	{
		int chMin;
		int chMax;
	};

	void UploadGlyphsForRanges( charRange_t *range, int rangeSize );

protected:
	void ApplyBlurToTexture(Point rgbaPt, Size rgbaSz, unsigned char *rgba);
	void CreateGaussianDistribution();

	char m_szName[32];
	int	 m_iTall, m_iWeight, m_iFlags, m_iHeight, m_iMaxCharWidth;
	bool m_bUnderlined;
	int  m_iAscent;
	int  m_iGLTexture;
	int  m_iBlur;
	float *m_pGaussianDistribution;
	float m_fBrighten;

private:
	void GetBlurValueForPixel(unsigned char *src, Point srcPt, Size srcSz, unsigned char *dest);

	struct glyph_t
	{
		int ch;
		HIMAGE texture;
		wrect_t rect;
	};

	CUtlRBTree<glyph_t, int> m_glyphs;
	// CUtlVector<HIMAGE> m_Pages;
	int m_iPages;
	friend bool GlyphLessFunc( const glyph_t &a, const glyph_t &b );
	friend class CFontManager;
};


#endif // BASEFONT_H
