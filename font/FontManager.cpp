#include "FontManager.h"
#include "Utils.h"

#if defined(MAINUI_USE_FREETYPE)
#include "FreeTypeFont.h"
#elif defined(MAINUI_USE_STB)
#include "StbFont.h"
#elif defined(_WIN32)
#include "WinAPIFont.h"
#else
#error "No font rendering backend found"
#endif


// Probably isn't a good idea until I don't have implemented SDF
#define SCALE_FONTS 0

CFontManager g_FontMgr;

CFontManager::CFontManager()
{
#ifdef MAINUI_USE_FREETYPE
	FT_Init_FreeType( &CFreeTypeFont::m_Library );
#endif
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
	static bool calledOnce = false;

	DeleteAllFonts();

	// Ordering is important!
	uiStatic.hDefaultFont = CreateFont( "Noto Sans CJK JP", 26 * uiStatic.scaleY, 100, 0, 0.0f, FONT_NONE );
	uiStatic.hSmallFont   = CreateFont( "Noto Sans CJK JP", 20 * uiStatic.scaleY, 100, 0, 0, FONT_NONE );
	uiStatic.hBigFont     = CreateFont( "Noto Sans CJK JP", 40 * uiStatic.scaleY, 100, 0, 0, FONT_NONE );

	int consoleFontHeight;

	if( ScreenHeight < 320 ) consoleFontHeight = 11;
	else if( ScreenHeight < 640 ) consoleFontHeight = 14;
	else consoleFontHeight = 24;

	uiStatic.hConsoleFont = CreateFont( "Arial", consoleFontHeight, 100, 0, 0.0f, FONT_NONE );

	UploadTextureForFont( uiStatic.hDefaultFont );
	UploadTextureForFont( uiStatic.hSmallFont );
	UploadTextureForFont( uiStatic.hBigFont );
	UploadTextureForFont( uiStatic.hConsoleFont );

	calledOnce = true;
}

void CFontManager::DeleteAllFonts()
{
	for( int i = 0; i < m_Fonts.Count(); i++ )
	{
		delete m_Fonts[i];
	}
	m_Fonts.RemoveAll();
}

HFont CFontManager::CreateFont(const char *name, int tall, int weight, int blur, float brighten, int flags)
{
	// check existing font at first
	for( int i = 0; i < m_Fonts.Count(); i++ )
	{
		IBaseFont *font = m_Fonts[i];

		if( font->IsEqualTo( name, tall, weight, blur, flags ) )
			return i;
	}

	#ifdef _WIN32
	CWinAPIFont *font = new CWinAPIFont();
	#else
	CFreeTypeFont *font = new CFreeTypeFont();
	#endif

	if( !font->Create( name, tall, weight, blur, brighten, flags ) )
	{
		delete font;
		return -1;
	}

	return m_Fonts.AddToTail(font);
}

void CFontManager::GetCharABCWide(HFont font, int ch, int &a, int &b, int &c)
{
	if( m_Fonts.IsValidIndex( font ) )
		m_Fonts[font]->GetCharABCWidths( ch, a, b, c );
}

int CFontManager::GetCharacterWidth(HFont font, int ch)
{
	int a = 0, b = 0, c = 0;
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
	if( m_Fonts.IsValidIndex( font ))
		return m_Fonts[font]->GetHeight();
	return 0;
}

int CFontManager::GetFontAscent(HFont font)
{
	if( m_Fonts.IsValidIndex( font ) )
		return m_Fonts[font]->GetAscent();
	return 0;
}

bool CFontManager::GetFontUnderlined(HFont font)
{
	if( m_Fonts.IsValidIndex( font ))
		return m_Fonts[font]->GetUnderlined();
	return false;
}

void CFontManager::GetTextSize(HFont fontHandle, const wchar_t *text, int *wide, int *tall)
{
	if( !m_Fonts.IsValidIndex( fontHandle ) || !text || !text[0] )
	{
		if( wide ) *wide = 0;
		if( tall ) *tall = 0;
		return;
	}

	IBaseFont *font = m_Fonts[fontHandle];
	int fontTall = font->GetHeight(), x = 0;
	int _wide, _tall;
	const wchar_t *ch = text;
	_tall = fontTall;

	while( *ch )
	{
		// Skip colorcodes
		if( IsColorString( ch ) )
		{
			ch += 2;
			continue;
		}

		if( *ch == '\n' )
		{
			_tall += fontTall;
			x = 0;
		}
		else
		{
			x += GetCharacterWidth( fontHandle, *ch );
			if( x > _wide )
				_wide = x;
		}

		ch++;
	}

	if( tall ) *tall = _tall;
	if( wide ) *wide = _wide;
}

void CFontManager::GetTextSize(HFont font, const char *text, int *wide, int *tall)
{
	wchar_t *wText = UI::String::ConvertToWChar( text );

	GetTextSize( font, wText, wide, tall );

	delete[] wText;
}

int CFontManager::GetTextWide(HFont font, const wchar_t *text)
{
	int wide;

	GetTextSize( font, text, &wide );

	return wide;
}

int CFontManager::GetTextWide(HFont font, const char *text)
{
	wchar_t *wText = UI::String::ConvertToWChar(text);
	int ret = GetTextWide( font, wText );

	delete[] wText;
	return ret;
}

int CFontManager::GetTextHeight(HFont fontHandle, const wchar_t *text )
{
	if( !m_Fonts.IsValidIndex( fontHandle ) || !text || !text[0] )
	{
		return 0;
	}

	IBaseFont *font = m_Fonts[fontHandle];
	int height = font->GetHeight();

	// lightweight variant only for getting text height
	while( *text )
	{
		if( *text == '\n' )
			height += height;

		text++;
	}
	return height;
}

int CFontManager::GetTextHeight(HFont font, const char *text)
{
	wchar_t *wText = UI::String::ConvertToWChar(text);
	int ret = GetTextHeight( font, wText );

	delete[] wText;
	return ret;
}

int CFontManager::GetTextWideScaled(HFont font, const wchar_t *text, const int height)
{
	if( !m_Fonts.IsValidIndex( font ) )
		return 0;

	return GetTextWide( font, text )
	#if SCALE_FONTS
		* ((float)height / m_Fonts[font]->GetHeight())
	#endif
		;
}

int CFontManager::GetTextWideScaled(HFont font, const char *text, const int height)
{
	if( !m_Fonts.IsValidIndex( font ) )
		return 0;

	return GetTextWide( font, text )
	#if SCALE_FONTS
		* ((float)height / m_Fonts[font]->GetHeight())
	#endif
		;
}

void CFontManager::UploadTextureForFont(HFont fontHandle)
{
	// upload only latin needed for english and cyrillic needed for russian
	// maybe it would be extended someday...
	if( !m_Fonts.IsValidIndex( fontHandle ))
		return;

	IBaseFont *font = m_Fonts[fontHandle];

	IBaseFont::charRange_t range[] =
	{
	{ 33, 126 },			// ascii printable range
	{ 0x0410, 0x044F },		// cyrillic range
	{ 0x3040, 0x309f } // hiragana, just for test
	};

	font->UploadGlyphsForRanges( range, sizeof( range ) / sizeof( IBaseFont::charRange_t ) );
}

int CFontManager::DrawCharacter(HFont fontHandle, wchar_t ch, Point pt, Size sz, const int color )
{
	if( !m_Fonts.IsValidIndex( fontHandle ))
		return 0;

	Size charSize;
	IBaseFont *font = m_Fonts[fontHandle];
	int a, b, c, width, height;

	font->GetCharABCWidths( ch, a, b, c );
	width = a + b + c; // TODO: Rework this to have a nice looking Italic fonts
	height = font->GetHeight();

	// skip whitespace
	if( ch == ' ' )
	{
#if SCALE_FONTS
		if( sz.h > 0 )
		{
			return width * ((float)sz.h / (float)height);
		}
		else
#endif
		{
			return width;
		}
	}

	IBaseFont::glyph_t find = { ch };
	int idx = font->m_glyphs.Find( find );

	if( font->m_glyphs.IsValidIndex( idx ) )
	{
		IBaseFont::glyph_t &glyph = font->m_glyphs[idx];

		int r, g, b, a;

		UnpackRGBA(r, g, b, a, color );

#if SCALE_FONTS	// Scale font
		if( sz.h > 0 )
		{
			charSize.w = width * ((float)sz.h / (float)height);
			charSize.h = sz.h;
		}
		else
#endif
		{
			charSize.w = width;
			charSize.h = font->GetHeight();
		}

		EngFuncs::PIC_Set( glyph.texture, r, g, b, a );
		EngFuncs::PIC_DrawTrans( pt, charSize, &glyph.rect );
	}

	return charSize.w;
}

void CFontManager::DebugDraw(HFont font)
{
	m_Fonts[font]->DebugDraw();
}

#ifdef MAINUI_USE_CUSTOM_FONT_RENDER

unsigned int color;

void EngFuncs::DrawSetTextColor(int r, int g, int b, int alpha)
{
	color = PackRGBA( r, g, b, alpha );
}

int EngFuncs::DrawConsoleString(int x, int y, const char *string)
{
	int len = strlen( string );
	wchar_t *atext = new wchar_t[len+1], *ptext;
	mbstowcs( atext, string, len );
	atext[len] = 0;

	ptext = atext;
	Point pt(x, y);

	for( ; *ptext; ptext++ )
	{
		if( *ptext == '\n' )
		{
			pt.x = x;
			pt.y += g_FontMgr.GetFontTall( uiStatic.hConsoleFont );
			continue;
		}

		pt.x += g_FontMgr.DrawCharacter( uiStatic.hConsoleFont, *ptext, pt, Size(), color );
	}

	delete[] atext;
}

void EngFuncs::ConsoleStringLen(const char *string, int *length, int *height)
{
	g_FontMgr.GetTextSize( uiStatic.hConsoleFont, string, length, height );
}

#endif
