#pragma once
#if !defined( _WIN32 ) && !defined( FREETYPEFONT_H )
#define FREETYPEFONT_H

#include "BaseFontBackend.h"

extern "C"
{
    #include <fontconfig/fontconfig.h>
    #include <ft2build.h>
    #include FT_FREETYPE_H
}

#include "utl/utlmemory.h"
#include "utl/utlrbtree.h"

struct abc_t
{
	int ch;
	int a, b, c;
};

class CFreeTypeFont : public IBaseFont
{
public:
	CFreeTypeFont();
	~CFreeTypeFont();

	bool Create(const char *name, int tall, int weight, int blur, float brighten, int flags);
	void GetCharRGBA(int ch, Point pt, Size sz, unsigned char *rgba, Size &drawSize);
	bool IsValid() const;
	void GetCharABCWidths( int ch, int &a, int &b, int &c );
	bool HasChar( int ch ) const;
private:
	int  m_iOutlineSize;

	bool m_bAntialiased, m_bAdditive;

	CUtlRBTree<abc_t, int> m_ABCCache;

	FT_Face face;
	static FT_Library m_Library;
	char m_szRealFontFile[4096];
	bool FindFontDataFile(const char *name, int tall, int weight, int flags, char *dataFile, int dataFileChars);

	friend class CFontManager;
};


#endif // FREETYPEFONT_H
