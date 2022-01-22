/*
PicButton.h - animated button with picture
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
#include "PicButton.h"
#include "Utils.h"
#include "Scissor.h"
#include "BtnsBMPTable.h"
#include <stdlib.h>

CMenuPicButton::CMenuPicButton() : BaseClass()
{
	bEnableTransitions = true;
	eFocusAnimation = QM_HIGHLIGHTIFFOCUS;
	iFlags = QMF_DROPSHADOW;

	iFocusStartTime = 0;

	eTextAlignment = QM_TOPLEFT;

	hPic = 0;
	button_id = 0;
	iOldState = BUTTON_NOFOCUS;
	m_iLastFocusTime = -512;
	bPulse = false;

	SetSize( UI_BUTTONS_WIDTH, UI_BUTTONS_HEIGHT );

	SetCharSize( QM_DEFAULTFONT );
}

/*
=================
CMenuPicButton::Key
=================
*/
bool CMenuPicButton::KeyUp( int key )
{
	const char *sound = 0;

	if( UI::Key::IsEnter( key ) && !(iFlags & QMF_MOUSEONLY) )
		sound = uiSoundLaunch;
	else if( UI::Key::IsLeftMouse( key ) && ( iFlags & QMF_HASMOUSEFOCUS ) )
		sound = uiSoundLaunch;

	if( sound )
	{
		_Event( QM_RELEASED );
		PlayLocalSound( sound );
	}

	return sound != NULL;
}

bool CMenuPicButton::KeyDown( int key )
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


// #define ALT_PICBUTTON_FOCUS_ANIM

/*
=================
CMenuPicButton::DrawButton
=================
*/
void CMenuPicButton::DrawButton(int r, int g, int b, int a, wrect_t *rects, int state)
{
	EngFuncs::PIC_Set( hPic, r, g, b, a );
#ifdef ALT_PICBUTTON_FOCUS_ANIM
	UI::PushScissor( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width * flFill, uiStatic.buttons_draw_height );
#endif
	EngFuncs::PIC_DrawAdditive( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height, &rects[state] );

#ifdef ALT_PICBUTTON_FOCUS_ANIM
	UI::PopScissor();
#endif
}

/*
=================
CMenuPicButton::Draw
=================
*/
void CMenuPicButton::Draw( )
{
	int state = BUTTON_NOFOCUS;

#ifdef CS16CLIENT
	if( UI_CursorInRect( m_scPos, m_scSize ) &&
		m_pParent && m_pParent->IsVisible() )
	{
		if( !bRollOver )
		{
			PlayLocalSound( uiSoundRollOver );
			bRollOver = true;
		}
	}
	else
	{
		if( bRollOver )
			bRollOver = false;
	}
#endif // CS16CLIENT

	if( iFlags & (QMF_HASMOUSEFOCUS|QMF_HASKEYBOARDFOCUS))
	{
		state = BUTTON_FOCUS;
	}

	// make sure what cursor in rect
	if( m_bPressed )
		state = BUTTON_PRESSED;

	if( iOldState == BUTTON_NOFOCUS && state != BUTTON_NOFOCUS )
		iFocusStartTime = uiStatic.realTime;

#ifndef CS16CLIENT
	if( szStatusText && iFlags & QMF_NOTIFY )
	{
		Point coord;

		coord.x = m_scPos.x + 290 * uiStatic.scaleX;
		coord.y = m_scPos.y + m_scSize.h / 2 - EngFuncs::ConsoleCharacterHeight() / 2;

		int	r, g, b;

		UnpackRGB( r, g, b, uiColorHelp );
		EngFuncs::DrawSetTextColor( r, g, b );
		EngFuncs::DrawConsoleString( coord, szStatusText );
	}
#endif

	int a = (512 - (uiStatic.realTime - m_iLastFocusTime)) >> 1;

	if( hPic )
	{
		int r, g, b;

		UnpackRGB( r, g, b, iFlags & QMF_GRAYED ? uiColorDkGrey : uiColorWhite );

		wrect_t rects[] =
		{
		{ 0, uiStatic.buttons_width, 0,  26 },
		{ 0, uiStatic.buttons_width, 26, 52 },
		{ 0, uiStatic.buttons_width, 52, 78 },
		};
		if( state == BUTTON_NOFOCUS && a > 0 )
		{
			DrawButton( r, g, b, a, rects, BUTTON_FOCUS );
		}

		// pulse code.
		if( ( state == BUTTON_NOFOCUS && bPulse ) ||
			( state == BUTTON_FOCUS   && eFocusAnimation == QM_PULSEIFFOCUS ) )
		{
			EngFuncs::PIC_Set( hPic, r, g, b, 255 *(0.5f + 0.5f * sin( (float)uiStatic.realTime / ( UI_PULSE_DIVISOR * 2 ))));
			EngFuncs::PIC_DrawAdditive( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height, &rects[BUTTON_FOCUS] );

			EngFuncs::PIC_DrawAdditive( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height, &rects[BUTTON_NOFOCUS] );
		}
		else
		{
			// special handling for focused
			if( state == BUTTON_FOCUS )
			{
				EngFuncs::PIC_Set( hPic, r, g, b, 255 );
				EngFuncs::PIC_DrawAdditive( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height, &rects[BUTTON_FOCUS] );

				EngFuncs::PIC_Set( hPic, r, g, b, 255 ); // set colors again
				EngFuncs::PIC_DrawAdditive( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height, &rects[BUTTON_NOFOCUS] );
			}
			else
			{
				// just draw
				EngFuncs::PIC_Set( hPic, r, g, b, 255 );
				EngFuncs::PIC_DrawAdditive( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height, &rects[state] );
			}
		}
	}
	else if( !uiStatic.lowmemory )
	{
		uint textflags = ETF_NOSIZELIMIT | ETF_FORCECOL;

		SetBits( textflags, ETF_ADDITIVE );

		if( iFlags & QMF_GRAYED )
		{
			if( a > 0 )
			{
				UI_DrawString( uiStatic.hHeavyBlur, m_scPos, m_scSize, szName,
					InterpColor( uiColorBlack, uiColorDkGrey, a / 255.0f ), m_scChSize, eTextAlignment, textflags );
			}
			UI_DrawString( uiStatic.hLightBlur, m_scPos, m_scSize, szName, uiColorDkGrey, m_scChSize, eTextAlignment, textflags );
		} else if( this != m_pParent->ItemAtCursor() )
		{
			if( a > 0 )
			{
				UI_DrawString( uiStatic.hHeavyBlur, m_scPos, m_scSize, szName,
					InterpColor( uiColorBlack, colorBase, a / 255.0f ), m_scChSize, eTextAlignment, textflags );
			}
			UI_DrawString( uiStatic.hLightBlur, m_scPos, m_scSize, szName, colorBase, m_scChSize, eTextAlignment, textflags );
		}
		else if( m_bPressed )
		{
			UI_DrawString( uiStatic.hHeavyBlur, m_scPos, m_scSize, szName, colorBase, m_scChSize, eTextAlignment, textflags );
			ClearBits( textflags, ETF_ADDITIVE );
			UI_DrawString( uiStatic.hLightBlur, m_scPos, m_scSize, szName, 0xFF000000, m_scChSize, eTextAlignment, textflags );
		}
		else if( eFocusAnimation == QM_HIGHLIGHTIFFOCUS )
		{
			UI_DrawString( uiStatic.hHeavyBlur, m_scPos, m_scSize, szName, colorBase, m_scChSize, eTextAlignment, textflags );
			UI_DrawString( uiStatic.hLightBlur, m_scPos, m_scSize, szName, colorBase, m_scChSize, eTextAlignment, textflags );
		}
		else if( eFocusAnimation == QM_PULSEIFFOCUS )
		{
			float pulsar = 0.5f + 0.5f * sin( (float)uiStatic.realTime / UI_PULSE_DIVISOR );

			UI_DrawString( uiStatic.hHeavyBlur, m_scPos, m_scSize, szName,
				InterpColor( uiColorBlack, colorBase, pulsar ), m_scChSize, eTextAlignment, textflags );
			UI_DrawString( uiStatic.hLightBlur, m_scPos, m_scSize, szName, colorBase, m_scChSize, eTextAlignment, textflags );
		}
	}
	else
	{
		uint textflags = ETF_NOSIZELIMIT | ETF_FORCECOL;

		SetBits( textflags, (iFlags & QMF_DROPSHADOW) ? ETF_SHADOW : 0 );

		if( iFlags & QMF_GRAYED )
		{
			UI_DrawString( font, m_scPos, m_scSize, szName, uiColorDkGrey, m_scChSize, eTextAlignment, textflags );
		}
		else if( this != m_pParent->ItemAtCursor() )
		{
			UI_DrawString( font, m_scPos, m_scSize, szName, colorBase, m_scChSize, eTextAlignment, textflags );
		}
		else if( eFocusAnimation == QM_HIGHLIGHTIFFOCUS )
		{
			UI_DrawString( font, m_scPos, m_scSize, szName, colorFocus, m_scChSize, eTextAlignment, textflags );
		}
		else if( eFocusAnimation == QM_PULSEIFFOCUS )
		{
			float pulsar = 0.5f + 0.5f * sin( (float)uiStatic.realTime / UI_PULSE_DIVISOR );

			UI_DrawString( font, m_scPos, m_scSize, szName,
				InterpColor( colorBase, colorFocus, pulsar ), m_scChSize, eTextAlignment, textflags );
		}
	}

	iOldState = state;
}

void CMenuPicButton::SetPicture( EDefaultBtns ID )
{
	if( ID < 0 || ID > PC_BUTTONCOUNT )
		return; // bad id

	hPic = uiStatic.buttonsPics[ID];

	button_id = ID;
}

void CMenuPicButton::SetPicture( const char *filename )
{
	hPic = EngFuncs::PIC_Load( filename );
}
