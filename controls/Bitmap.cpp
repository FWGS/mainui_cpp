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

#include "extdll_menu.h"
#include "BaseMenu.h"
#include "Bitmap.h"
#include "PicButton.h" // GetTitleTransFraction
#include "Utils.h"
#include "BaseWindow.h"

CMenuBitmap::CMenuBitmap() : BaseClass()
{
	SetPicture( NULL );
	SetRenderMode( QM_DRAWNORMAL );
}

/*
=================
CMenuBitmap::Init
=================
*/
void CMenuBitmap::VidInit( )
{
	colorBase.SetDefault( uiColorWhite );
	colorFocus.SetDefault( uiColorWhite );

	BaseClass::VidInit();
	if( !szFocusPic )
		szFocusPic = szPic;
}

bool CMenuBitmap::KeyUp( int key )
{
	const char *sound = 0;

	if( UI::Key::IsEnter( key ) && !(iFlags & QMF_MOUSEONLY) )
		sound = uiSoundLaunch;
	else if( UI::Key::IsLeftMouse( key ) && ( iFlags & QMF_HASMOUSEFOCUS ) )
		sound = uiSoundLaunch;

	if( sound )
	{
		_Event( QM_PRESSED );
		PlayLocalSound( sound );
	}

	return sound != NULL;
}

bool CMenuBitmap::KeyDown( int key )
{
	bool handled = false;

	if( UI::Key::IsEnter( key ) && !(iFlags & QMF_MOUSEONLY) )
		handled = true;
	else if( UI::Key::IsLeftMouse( key ) && ( iFlags & QMF_HASMOUSEFOCUS ) )
		handled = true;

	if( handled )
		_Event( QM_PRESSED );

	return handled;
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
		UI_FillRect( m_scPos, m_scSize, colorBase );
		return;
	}

	if( iFlags & QMF_GRAYED )
	{
		UI_DrawPic( m_scPos, m_scSize, uiColorDkGrey, szPic, eRenderMode );
		return; // grayed
	}

	if(( iFlags & QMF_MOUSEONLY ) && !( iFlags & QMF_HASMOUSEFOCUS ))
	{
		UI_DrawPic( m_scPos, m_scSize, colorBase, szPic, eRenderMode );
		return; // no focus
	}

	if( this != m_pParent->ItemAtCursor() )
	{
		UI_DrawPic( m_scPos, m_scSize, colorBase, szPic, eRenderMode );
		return; // no focus
	}

	if( this->m_bPressed )
	{
		UI_DrawPic( m_scPos, m_scSize, colorBase, szPressPic, ePressRenderMode );
	}

	switch( eFocusAnimation )
	{
	case QM_HIGHLIGHTIFFOCUS:
		UI_DrawPic( m_scPos, m_scSize, colorBase, szFocusPic, eFocusRenderMode );
		break;
	case QM_PULSEIFFOCUS:
	{
		int	color;

		color = PackAlpha( colorBase, 255 * (0.5f + 0.5f * sin( (float)uiStatic.realTime / UI_PULSE_DIVISOR )));
		UI_DrawPic( m_scPos, m_scSize, color, szFocusPic, eFocusRenderMode );
		break;
	}
	default:
		UI_DrawPic( m_scPos, m_scSize, colorBase, szPic, eRenderMode ); // ignore focus
		break;
	}
}

void CMenuBannerBitmap::Draw()
{
#ifdef CS16CLIENT
	const char *text = CMenuPicButton::GetLastButtonText();

	if( !text )
		return;

	UI_DrawString( uiStatic.hBigFont, m_scPos, m_scSize, text, uiPromptTextColor, m_scChSize, QM_LEFT, ETF_SHADOW | ETF_NOSIZELIMIT );
#else
	// don't draw banners until transition is done
#ifdef TA_ALT_MODE
	return;
#endif
	CMenuBaseWindow *window = NULL;

	if( m_pParent->IsWindow() )
		window = (CMenuBaseWindow*) m_pParent;

	if( CMenuPicButton::GetTitleTransFraction() < 1.0f )
		return;

	if( window && window->IsRoot() && window->eTransitionType == CMenuBaseWindow::ANIM_OUT )
		return;

	BaseClass::Draw();
#endif
}

void CMenuBannerBitmap::VidInit()
{
	BaseClass::VidInit();
#ifndef CS16CLIENT
	if( !szPic )
		return;

	HIMAGE hPic = EngFuncs::PIC_Load( szPic );

	if( !hPic )
		return;

	Size sz = EngFuncs::PIC_Size( hPic );
	float factor = (float)m_scSize.h / (float)sz.h;
	m_scSize.w = sz.w * factor;

	// CMenuPicButton::SetTitleAnim( CMenuPicButton::AS_TO_TITLE );
	CMenuPicButton::SetupTitleQuadForLast( uiStatic.xOffset + pos.x, uiStatic.yOffset + pos.y, m_scSize.w, m_scSize.h );
#if defined(TA_ALT_MODE2) && !defined(TA_ALT_MODE)
	CMenuPicButton::SetTransPicForLast( EngFuncs::PIC_Load( szPic ) );
#endif
#endif // CS16CLIENT
}
