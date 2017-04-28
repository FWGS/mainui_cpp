/*
Bitmap.cpp - bitmap menu item
Copyright (C) 2010 Uncle Mike
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

#include "extdll.h"
#include "BaseMenu.h"
#include "Bitmap.h"
#include "PicButton.h" // GetTitleTransFraction
#include "Utils.h"


CMenuBitmap::CMenuBitmap() : CMenuBaseItem()
{
	SetPicture( NULL, NULL, NULL );
	bDrawAdditive = false;
	iColor = uiColorWhite;
	iFocusColor = uiColorWhite;
}

/*
=================
CMenuBitmap::Init
=================
*/
void CMenuBitmap::VidInit( )
{
	if( !szFocusPic )
		szFocusPic = szPic;

	m_scPos = pos.Scale();
	m_scSize = size.Scale();
}

/*
=================
CMenuBitmap::Key
=================
*/
const char *CMenuBitmap::Key( int key, int down )
{
	const char	*sound = 0;

	switch( key )
	{
	case K_MOUSE1:
		if(!( iFlags & QMF_HASMOUSEFOCUS ))
			break;
		sound = uiSoundLaunch;
		break;
	case K_ENTER:
	case K_KP_ENTER:
	case K_AUX1:
		//if( !down ) return sound;
		if( iFlags & QMF_MOUSEONLY )
			break;
		sound = uiSoundLaunch;
		break;
	}
	if( sound && ( iFlags & QMF_SILENT ))
		sound = uiSoundNull;

	if( iFlags & QMF_ACT_ONRELEASE )
	{
		if( sound )
		{
			int	event;

			if( down )
			{
				event = QM_PRESSED;
				m_bPressed = true;
			}
			else event = QM_ACTIVATED;
			_Event( event );
		}
	}
	else if( down )
	{
		if( sound )
			_Event( QM_ACTIVATED );
	}

	return sound;
}

/*
=================
CMenuBitmap::Draw
=================
*/
void CMenuBitmap::Draw( void )
{
	if( iFlags & QMF_GRAYED )
	{
		UI_DrawPic( m_scPos, m_scSize, uiColorDkGrey, szPic );
		return; // grayed
	}

	if(( iFlags & QMF_MOUSEONLY ) && !( iFlags & QMF_HASMOUSEFOCUS ))
	{
		UI_DrawPic( m_scPos, m_scSize, iColor, szPic );
		return; // no focus
	}

	if( this != m_pParent->ItemAtCursor() )
	{
		// UNDONE: only inactive bitmaps supported
		if( bDrawAdditive )
			UI_DrawPicAdditive( m_scPos, m_scSize, iColor, szPic );
		else UI_DrawPic( m_scPos, m_scSize, iColor, szPic );
		return; // no focus
	}

	if( this->m_bPressed )
	{
		UI_DrawPic( m_scPos, m_scSize, iColor, szPressPic );
	}
	else if(!( iFlags & QMF_FOCUSBEHIND ))
	{
		UI_DrawPic( m_scPos, m_scSize, iColor, szPic );
	}

	if( eFocusAnimation == QM_HIGHLIGHTIFFOCUS )
	{
		UI_DrawPic( m_scPos, m_scSize, iColor, szFocusPic );
	}
	else if( eFocusAnimation == QM_PULSEIFFOCUS )
	{
		int	color;

		color = PackAlpha( iColor, 255 * (0.5 + 0.5 * sin( (float)uiStatic.realTime / UI_PULSE_DIVISOR )));
		UI_DrawPic( m_scPos, m_scSize, color, szFocusPic );
	}

	if( iFlags & QMF_FOCUSBEHIND )
	{
		UI_DrawPic( m_scPos, m_scSize, iColor, szPic );
	}
}

/*
=================
CMenuBackgroundBitmap::Draw
=================
*/
void CMenuBackgroundBitmap::Draw()
{
	if( EngFuncs::ClientInGame() )
	{
		if( EngFuncs::GetCvarFloat( "cl_background" ) )
			return;

		if( EngFuncs::GetCvarFloat( "ui_renderworld" ) )
		{
			UI_FillRect( 0, 0, ScreenWidth, ScreenHeight, uiColorBlack );
			return;
		}
	}

	if (!uiStatic.m_fHaveSteamBackground)
	{
		UI_DrawPic( 0, 0, ScreenWidth, ScreenHeight, uiColorWhite, szPic );
		return;
	}

	float xScale, yScale;
	int xpos, ypos;
	int xoffset, yoffset;
	float flParallaxScale;

#if 0
	flParallaxScale = 0.02;
#else
	// Disable parallax effect. It's just funny, but not really needed
	flParallaxScale = 0.0f;
#endif

	xoffset = (uiStatic.cursorX - ScreenWidth) * flParallaxScale;
	yoffset = (uiStatic.cursorY - ScreenHeight) * flParallaxScale;

	// work out scaling factors
	xScale = ScreenWidth / uiStatic.m_flTotalWidth * (1 + flParallaxScale);
	yScale = xScale;

	// iterate and draw all the background pieces
	ypos = 0;
	for (int y = 0; y < BACKGROUND_ROWS; y++)
	{
		xpos = 0;
		for (int x = 0; x < BACKGROUND_COLUMNS; x++)
		{
			bimage_t &bimage = uiStatic.m_SteamBackground[y][x];

			int dx = (int)ceil(xpos * xScale);
			int dy = (int)ceil(ypos * yScale);
			int dw = (int)ceil(bimage.width * xScale);
			int dt = (int)ceil(bimage.height * yScale);

			if (x == 0) dx = 0;
			if (y == 0) dy = 0;

			EngFuncs::PIC_Set( bimage.hImage, 255, 255, 255, 255 );
			EngFuncs::PIC_Draw( xoffset + dx, yoffset + dy, dw, dt );
			xpos += bimage.width;
		}
		ypos += uiStatic.m_SteamBackground[y][0].height;
	}
}

void CMenuBannerBitmap::Draw()
{
	// don't draw banners until transition is done
#ifdef TA_ALT_MODE
	if( CMenuPicButton::GetTitleTransFraction() != 10 )
#else
	if( CMenuPicButton::GetTitleTransFraction() < 1.0f )
#endif
	{
		return;
	}

	CMenuBitmap::Draw();
}

void CMenuBannerBitmap::VidInit()
{
	CMenuBitmap::VidInit();
	CMenuPicButton::SetupTitleQuad( pos.x, pos.y, size.w, size.h );
#ifdef TA_ALT_MODE2
	HIMAGE hPic = EngFuncs::PIC_Load( szPic );
	CMenuPicButton::SetTransPic( hPic );
#endif
}
