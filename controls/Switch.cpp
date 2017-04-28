/*
Copyright (C) 2017 a1batross
Switch.cpp - simple switches, like Android 4.0+

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "BaseMenu.h"
#include "Switch.h"

CMenuSwitch::CMenuSwitch( ) : CMenuEditable( )
{
	szLeftName = szRightName = NULL;
	bMouseToggle = true;
	bState = false;
	SetSize( 220, 35 );
	SetCharSize( QM_DEFAULTFONT );

	iSelectColor = uiPromptTextColor;
	iBackgroundColor = uiColorBlack;

	eTextAlignment = QM_CENTER;
	iFlags |= QMF_DROPSHADOW;
}

void CMenuSwitch::VidInit()
{
	m_scPos = pos.Scale();
	m_scSize = size.Scale();
	m_scChSize = charSize.Scale();

	int leftSize, rightSize;

	if( szLeftName )
		leftSize = strlen( szLeftName ) * m_scChSize.w;
	else
		leftSize = m_scSize.w / 2;

	if( szRightName )
		rightSize = strlen( szRightName ) * m_scChSize.w;
	else
		rightSize = m_scSize.w / 2;

	// calculate fraction from two string sizes
	float frac = (float)leftSize / (float)( leftSize + rightSize );

	// then adjust widths
	m_leftSize.w = m_scSize.w * frac;
	m_rightSize.w = m_scSize.w - m_leftSize.w;
	m_rightSize.h = m_leftSize.h = m_scSize.h; // height never changes

	m_rightPoint = m_leftPoint = m_scPos; // correct positions based on width sizes
	m_rightPoint.x += m_leftSize.w;

	m_scTextPos.x = m_scPos.x + (m_scSize.w * 1.7f );
	m_scTextPos.y = m_scPos.y + (m_scSize.h >> 2);

	m_scTextSize.w = strlen( szName ) * m_scChSize.w;
	m_scTextSize.h = m_scChSize.h;
}

const char * CMenuSwitch::Key(int key, int down)
{
	const char *sound = NULL;

	switch( key )
	{
	case K_MOUSE1:
		if(!( iFlags & QMF_HASMOUSEFOCUS ))
			break;
		if( bMouseToggle )
		{
			sound = uiSoundGlow;
		}
		else
		{
			if( UI_CursorInRect( m_leftPoint, m_leftSize ) && bState ||
				UI_CursorInRect( m_rightPoint, m_rightSize ) && !bState )
			{
				sound = uiSoundGlow;
			}
		}
		break;
	case K_ENTER:
	case K_KP_ENTER:
	case K_SPACE:
	case K_AUX1:
		if( iFlags & QMF_MOUSEONLY )
			break;
		sound = uiSoundGlow;
		break;
	}

	if( sound )
	{
		if( iFlags & QMF_ACT_ONRELEASE )
		{
			int event;

			if( down )
			{
				event = QM_PRESSED;
				m_bPressed = true;
			}
			else
			{
				event = QM_CHANGED;
				bState = !bState;
				SetCvarValue( bState );
			}
			_Event( event );
		}
		else if( down )
		{
			bState = !bState;
			SetCvarValue( bState );
			_Event( QM_CHANGED );
		}
	}

	if( iFlags & QMF_SILENT )
		return 0;

	return sound;
}

void CMenuSwitch::Draw( void )
{
	bool shadow = (iFlags & QMF_DROPSHADOW);

	int leftSelect = iSelectColor, rightSelect = iSelectColor;

	UI_DrawString( m_scTextPos, m_scTextSize, szName, uiColorHelp, true, m_scChSize, eTextAlignment, shadow );

	if( szStatusText && iFlags & QMF_NOTIFY )
	{
		int	x;

		x = 250;
		UI_ScaleCoords( &x, NULL, NULL, NULL );
		x += m_scPos.x;

		int	r, g, b;

		UnpackRGB( r, g, b, uiColorHelp );
		EngFuncs::DrawSetTextColor( r, g, b );
		EngFuncs::DrawConsoleString( x, m_scPos.y, szStatusText );
	}

	if( iFlags & QMF_GRAYED )
	{
		leftSelect = rightSelect = uiColorDkGrey;
	}
	else if( bMouseToggle )
	{
		if( UI_CursorInRect( m_scPos, m_scSize ) )
		{
			leftSelect = rightSelect = iFocusColor;
		}
	}
	else
	{
		if( UI_CursorInRect( m_leftPoint, m_leftSize ) )
		{
			leftSelect = iFocusColor;
		}
		else if( UI_CursorInRect( m_rightPoint, m_rightSize ))
		{
			rightSelect = iFocusColor;
		}
	}

	// draw toggle rectangles
	UI_FillRect( m_leftPoint, m_leftSize, bState? iBackgroundColor: leftSelect );
	UI_FillRect( m_rightPoint, m_rightSize, bState? rightSelect: iBackgroundColor );

	UI_DrawString( m_leftPoint, m_leftSize, szLeftName, bState?leftSelect: uiColorHelp, iColor, m_scChSize, eTextAlignment, shadow, QM_VCENTER );
	UI_DrawString( m_rightPoint, m_rightSize, szRightName, bState?uiColorHelp:rightSelect, iColor, m_scChSize, eTextAlignment, shadow, QM_VCENTER );

	// draw rectangle
	UI_DrawRectangle( m_scPos, m_scSize, uiInputFgColor );
}

void CMenuSwitch::UpdateEditable()
{
	bState = !!EngFuncs::GetCvarFloat( m_szCvarName );
}
