#include "FontManager.h"
#include "Utils.h"

#include "BaseFontBackend.h"

#if defined(MAINUI_USE_CUSTOM_FONT_RENDERER)
#if defined(MAINUI_USE_FREETYPE)
#include "FreeTypeFont.h"
#elif defined(MAINUI_USE_STB)
#include "StbFont.h"
#elif defined(_WIN32)
#include "WinAPIFont.h"
#else
#error "No font rendering backend found"
#endif
#endif

#include "BitmapFont.h"

#define DEFAULT_MENUFONT "Impact"
#define DEFAULT_CONFONT  "Arial"

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
		uiStatic.hDefaultFont = CFontBuilder( DEFAULT_MENUFONT, UI_MED_CHAR_HEIGHT * scale, 1000 )
			.Create();
		uiStatic.hSmallFont   = CFontBuilder( DEFAULT_MENUFONT, UI_SMALL_CHAR_HEIGHT * scale, 100 )
			.Create();
		uiStatic.hBigFont     = CFontBuilder( DEFAULT_MENUFONT, UI_BIG_CHAR_HEIGHT * scale, 1000 )
			.SetBlurParams( 5, 1.5f )
			.Create();
	}

	int consoleFontHeight;

	if( ScreenHeight < 320 ) consoleFontHeight = 11;
	else if( ScreenHeight < 640 ) consoleFontHeight = 14;
	else consoleFontHeight = 18;

	if( consoleFontHeight != prevConsoleFontHeight || forceUpdateConsoleFont )
	{
		DeleteFont( uiStatic.hConsoleFont );
		uiStatic.hConsoleFont = CFontBuilder( DEFAULT_CONFONT, consoleFontHeight, 100 )
			.SetOutlineSize()
			.Create();
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
			* ((float)height / (float)pFont->GetTall())
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
	int a, b, c, width;

	font->GetCharABCWidths( ch, a, b, c );
	width = a + b + c;

	// skip whitespace
	if( ch == ' ' )
	{
#if SCALE_FONTS
		if( sz.h > 0 )
		{
			return width * ((float)sz.h / (float)font->GetTall())  - 0.5f;
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
			charSize.w = (glyph.rect.right - glyph.rect.left) * ((float)sz.h / (float)font->GetTall()) - 0.5f;
			charSize.h = sz.h;
		}
		else
#endif
		{
			charSize.w = glyph.rect.right - glyph.rect.left;
			charSize.h = font->GetHeight();
		}

		pt.x += a;

		EngFuncs::PIC_Set( glyph.texture, r, g, b, alpha );
		EngFuncs::PIC_DrawTrans( pt, charSize, &glyph.rect );
	}

#if SCALE_FONTS
	if( sz.h > 0 )
	{
		return width * ((float)sz.h / (float)font->GetTall())  - 0.5f;
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
	for( int i = 0; i < g_FontMgr.m_Fonts.Count(); i++ )
	{
		font = g_FontMgr.m_Fonts[i];

#if defined(MAINUI_USE_CUSTOM_FONT_RENDER)
		// skip legacy
		if( m_bPreferRender && font->IsLegacyFont() )
			continue;
#endif

		// skip legacy
		if( m_bPreferVarWidth && font->IsLegacyFont() )
			continue;

		if( font->IsEqualTo( m_szName, m_iTall, m_iWeight, m_iBlur, m_iFlags ) )
			return i;
	}

#if defined(MAINUI_USE_CUSTOM_FONT_RENDER)
	if( m_bPreferRender )
	{
	#if defined(MAINUI_USE_FREETYPE)
		font = new CFreeTypeFont();
	#elseif defined(MAINUI_USE_STB)
		font = new CStbFont();
	#else
		font = new CWinAPIFont();
	#endif
	}
	else
#endif
	{
		// we added var width, because we can use VGUI1 bitmap fonts, that are variable width
		//if( m_bPreferVarWidth )
		//	font = new CBitmapFont_VGUI1();
		//else
			font = new CBitmapFont_XASH(); // or just fallback to XASH_SYSTEMFONT
	}

	if( !font->Create( m_szName, m_iTall, m_iWeight, m_iBlur, m_fBrighten, m_iOutlineSize, m_iScanlineOffset, m_fScanlineScale, m_iFlags ) )
	{
		delete font;
		return -1;
	}

	g_FontMgr.UploadTextureForFont( font );

	return g_FontMgr.m_Fonts.AddToTail(font) + 1;

}
