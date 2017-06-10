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
#define SCALE_FONTS 1

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
	static bool calledOnce = false, forceUpdateConsoleFont = false;
	static int prevConsoleFontHeight;

	// Ordering is important!
#ifdef SCALE_FONTS
	if( !calledOnce )
#endif
	{

#ifdef SCALE_FONTS
		float scale = 1;
#else
		float scale = uiStatic.scaleY;
#endif

		// TODO: Remove this. We need to place basic fonts into appropriate handles
		DeleteAllFonts();
		forceUpdateConsoleFont = true;

		// ordering is important!
		// See CBaseMenuItem::SetCharSize()
		uiStatic.hDefaultFont = CreateFont( "Noto Sans", 26 * scale, 100, 0, 0, FONT_NONE );
		uiStatic.hSmallFont   = CreateFont( "Noto Sans", 20 * scale, 100, 0, 0, FONT_NONE );
		uiStatic.hBigFont     = CreateFont( "Noto Sans", 40 * scale, 100, 0, 0, FONT_NONE );
	}

	int consoleFontHeight;

	if( ScreenHeight < 320 ) consoleFontHeight = 11;
	else if( ScreenHeight < 640 ) consoleFontHeight = 14;
	else consoleFontHeight = 18;

	if( consoleFontHeight != prevConsoleFontHeight || forceUpdateConsoleFont )
	{
		DeleteFont( uiStatic.hConsoleFont );
		uiStatic.hConsoleFont = CreateFont( "Arial", consoleFontHeight, 100, 0, 0, FONT_NONE );
		prevConsoleFontHeight = consoleFontHeight;
	}

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

void CFontManager::DeleteFont(HFont hFont)
{
	IBaseFont *font = GetIFontFromHandle(hFont);
	if( font )
	{
		delete font;

		m_Fonts.FastRemove( hFont - 1 );
	}
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

	UploadTextureForFont( font );

	return m_Fonts.AddToTail(font) + 1;
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
		return pFont->GetUnderlined();
	return false;
}

void CFontManager::GetTextSize(HFont fontHandle, const wchar_t *text, int *wide, int *tall)
{
	IBaseFont *font = GetIFontFromHandle( fontHandle );

	if( !font || !text || !text[0] )
	{
		if( wide ) *wide = 0;
		if( tall ) *tall = 0;
		return;
	}

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
	IBaseFont *font = GetIFontFromHandle( fontHandle );
	if( !font || !text || !text[0] )
	{
		return 0;
	}

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
	IBaseFont *pFont = GetIFontFromHandle( font );
	if( pFont )
	{
		return GetTextWide( font, text )
#if SCALE_FONTS
			* ((float)height / (float)pFont->GetHeight())
#endif
		;
	}

	return 0;
}

int CFontManager::GetTextWideScaled(HFont font, const char *text, const int height)
{
	IBaseFont *pFont = GetIFontFromHandle( font );
	if( pFont )
	{
		return GetTextWide( font, text )
#if SCALE_FONTS
			* ((float)height / (float)pFont->GetHeight())
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
	{ 0x0410, 0x044F },		// cyrillic range
	// { 0x3040, 0x309f } // hiragana, just for test
	};

	font->UploadGlyphsForRanges( range, sizeof( range ) / sizeof( IBaseFont::charRange_t ) );
}

int CFontManager::DrawCharacter(HFont fontHandle, wchar_t ch, Point pt, Size sz, const int color )
{
	IBaseFont *font = GetIFontFromHandle( fontHandle );

	if( !font )
		return 0;

	Size charSize;
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

		int r, g, b, alpha;

		UnpackRGBA(r, g, b, alpha, color );

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

		EngFuncs::PIC_Set( glyph.texture, r, g, b, alpha );
		EngFuncs::PIC_DrawTrans( pt, charSize, &glyph.rect );
	}

	return charSize.w;
}

void CFontManager::DebugDraw(HFont font)
{
	GetIFontFromHandle(font)->DebugDraw();
}
