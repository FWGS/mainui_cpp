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

	CalcPosition();
	CalcSizes();
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
	if( !szPic )
	{
		UI_FillRect( m_scPos, m_scSize, iColor );
		return;
	}

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
}

void CMenuBackgroundBitmap::VidInit()
{
	if( m_pParent )
	{
		pos.x = pos.y = 0;
		size = m_pParent->size;
		CalcPosition();
		CalcSizes();
	}
}

/*
=================
CMenuBackgroundBitmap::Draw
=================
*/
void CMenuBackgroundBitmap::Draw()
{
	if( bFillColor )
	{
		UI_FillRect( m_scPos, m_scSize, iColor );
		return;
	}

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

	float xScale, yScale;
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
	xScale = ScreenWidth / uiStatic.m_SteamBackgroundSize.w * (1 + flParallaxScale);
	yScale = xScale;

	if (!uiStatic.m_iSteamBackgroundCount || bForceBackground)
	{
		UI_DrawPic( xoffset * xScale, yoffset * yScale,
			ScreenWidth * xScale, ScreenWidth * 3.0f / 4.0f * yScale, uiColorWhite, szPic );
		return;
	}


	// iterate and draw all the background pieces
	for (int i = 0; i < uiStatic.m_iSteamBackgroundCount; i++)
	{
		bimage_t &bimage = uiStatic.m_SteamBackground[i];
		int dx = (int)ceil(bimage.coord.x * xScale);
		int dy = (int)ceil(bimage.coord.y * yScale);
		int dw = (int)ceil(bimage.size.w * xScale);
		int dt = (int)ceil(bimage.size.h * yScale);

		EngFuncs::PIC_Set( bimage.hImage, 255, 255, 255, 255 );
		EngFuncs::PIC_Draw( xoffset + dx, yoffset + dy, dw, dt );
	}
}

void CMenuBannerBitmap::Draw()
{
	// don't draw banners until transition is done
#ifdef TA_ALT_MODE
	return;
#endif

	if( CMenuPicButton::GetTitleTransFraction() < 1.0f )
	{
		return;
	}

	CMenuBitmap::Draw();
}

void CMenuBannerBitmap::VidInit()
{
	CMenuBitmap::VidInit();
	if( !szPic )
		return;
	// CMenuPicButton::SetTitleAnim( CMenuPicButton::AS_TO_TITLE );
	CMenuPicButton::SetupTitleQuad( pos.x, pos.y, size.w, size.h );
#if defined(TA_ALT_MODE2) && !defined(TA_ALT_MODE)
	HIMAGE hPic = EngFuncs::PIC_Load( szPic );
	CMenuPicButton::SetTransPic( hPic );
#endif
}
