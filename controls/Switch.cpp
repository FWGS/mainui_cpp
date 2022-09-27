/*
Switch.cpp - simple switches, like Android 4.0+
Copyright (C) 2017 a1batross

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

CMenuSwitch::CMenuSwitch( ) : BaseClass( )
{
	bMouseToggle = true;
	bKeepToggleWidth = false;
	SetSize( 220, 35 );
	SetCharSize( QM_BOLDFONT );

	// text offsets are not needed anymore,
	// they are useless now
	fTextOffsetX = 0.0f;
	fTextOffsetY = 0.0f;

	eTextAlignment = QM_CENTER;
	iFlags |= QMF_DROPSHADOW;

	m_iState = 0;
	m_iSwitches = 0;
	memset( m_szNames, 0, sizeof( m_szNames ));
	memset( m_Sizes, 0, sizeof( m_Sizes ));
	memset( m_Points, 0, sizeof( m_Points ));

	bChangeOnPressed = false;
}

void CMenuSwitch::AddSwitch(const char *text)
{
	m_szNames[m_iSwitches++] = text;
}

int CMenuSwitch::IsNewStateByMouseClick()
{
	int state = m_iState;

	if( bMouseToggle )
	{
		state++;

		if( state >= m_iSwitches )
			state = 0;
	}
	else
	{
		for( int i = 0; i < m_iSwitches; i++ )
		{
			if( ( UI_CursorInRect( m_Points[i], m_Sizes[i] ) && m_iState != i))
			{
				state = i;
			}
		}
	}

	return state;
}

void CMenuSwitch::VidInit()
{
	iSelectColor.SetDefault( uiPromptTextColor );
	iBackgroundColor.SetDefault( uiColorBlack );
	iFgTextColor.SetDefault( uiInputFgColor );
	iBgTextColor.SetDefault( uiPromptTextColor );

	BaseClass::VidInit();

	int sizes[UI_MAX_MENUITEMS];
	int sum = 0;
	int i;

	for( i = 0; i < m_iSwitches; i++ )
	{
		if( m_szNames[i] != NULL && !bKeepToggleWidth )
			sizes[i] = g_FontMgr->GetTextWideScaled( font, m_szNames[i], m_scChSize );
		else sizes[i] = (float)m_scSize.w / (float)m_iSwitches;

		sum += sizes[i];
	}

	for( i = 0; i < m_iSwitches; i++ )
	{
		float frac = (float)sizes[i] / (float)sum;

		m_Sizes[i].w = m_scSize.w * frac;
		m_Sizes[i].h = m_scSize.h;

		m_Points[i] = m_scPos;

		if( i != 0 )
			m_Points[i].x = m_Points[i-1].x + m_Sizes[i-1].w;
	}

	m_scTextPos.x = m_scPos.x + (m_scSize.w * 1.5f );
	m_scTextPos.y = m_scPos.y;

	m_scTextSize.w = g_FontMgr->GetTextWideScaled( font, szName, m_scChSize );
	m_scTextSize.h = m_scChSize;
}

bool CMenuSwitch::KeyUp( int key )
{
	const char *sound = NULL;
	bool haveNewState = false;
	int state = 0;

	if( UI::Key::IsLeftMouse( key ) && FBitSet( iFlags, QMF_HASMOUSEFOCUS ))
	{
		state = IsNewStateByMouseClick();
		haveNewState = state != m_iState;
		if( haveNewState )
			sound = uiStatic.sounds[SND_GLOW];
	}
	else if( UI::Key::IsEnter( key ) && !FBitSet( iFlags, QMF_MOUSEONLY ))
		sound = uiStatic.sounds[SND_GLOW];

	if( sound )
	{
		_Event( QM_RELEASED );
		if( haveNewState && !bChangeOnPressed )
		{
			m_iState = state;
			SetCvarValue( m_iState );
			_Event( QM_CHANGED );
			PlayLocalSound( sound ); // emit sound only on changes
		}
	}

	return sound != NULL;
}

bool CMenuSwitch::KeyDown( int key )
{
	const char *sound = NULL;
	bool haveNewState = false;
	int state = 0;

	if( UI::Key::IsLeftMouse( key ) && FBitSet( iFlags, QMF_HASMOUSEFOCUS ))
	{
		state = IsNewStateByMouseClick();
		haveNewState = state != m_iState;
		if( haveNewState )
			sound = uiStatic.sounds[SND_GLOW];
	}
	else if( UI::Key::IsEnter( key ) && !FBitSet( iFlags, QMF_MOUSEONLY ))
		sound = uiStatic.sounds[SND_GLOW];

	if( sound )
	{
		_Event( QM_PRESSED );
		if( haveNewState && bChangeOnPressed )
		{
			m_iState = state;
			SetCvarValue( m_iState );
			_Event( QM_CHANGED );
			PlayLocalSound( sound ); // emit sound only on changes
		}
	}

	return sound != NULL;
}

void CMenuSwitch::Draw( void )
{
	uint textflags = (iFlags & QMF_DROPSHADOW) ? ETF_SHADOW : 0;

	uint selectColor = iSelectColor;
	UI_DrawString( font, m_scTextPos, m_scTextSize, szName, uiColorHelp, m_scChSize, eTextAlignment, textflags | ETF_FORCECOL );

	if( szStatusText && iFlags & QMF_NOTIFY )
	{
		Point coord;

		coord.x = m_scPos.x + 250 * uiStatic.scaleX;
		coord.y = m_scPos.y + m_scSize.h / 2 - EngFuncs::ConsoleCharacterHeight() / 2;

		int	r, g, b;

		UnpackRGB( r, g, b, uiColorHelp );
		EngFuncs::DrawSetTextColor( r, g, b );
		EngFuncs::DrawConsoleString( coord, szStatusText );
	}

	if( iFlags & QMF_GRAYED )
	{
		selectColor = uiColorDkGrey;
	}

	for( int i = 0; i < m_iSwitches; i++ )
	{
		Point pt = m_Points[i];

		pt.x += fTextOffsetX * uiStatic.scaleX;
		pt.y += fTextOffsetY * uiStatic.scaleY;

		// draw toggle rectangles
		if( m_iState == i )
		{
			uint tempflags = textflags;
			tempflags |= ETF_FORCECOL;

			UI_FillRect( m_Points[i], m_Sizes[i], selectColor );
			UI_DrawString( font, pt, m_Sizes[i], m_szNames[i], iFgTextColor, m_scChSize, eTextAlignment, tempflags);
		}
		else
		{
			uint bgColor = iBackgroundColor;
			uint textColor = iBgTextColor;
			uint tempflags = textflags;

			if( UI_CursorInRect( m_Points[i], m_Sizes[i] ) && !(iFlags & (QMF_GRAYED|QMF_INACTIVE)))
			{
				bgColor = colorFocus;
				tempflags |= ETF_FORCECOL;
			}

			UI_FillRect( m_Points[i], m_Sizes[i], bgColor );
			UI_DrawString( font, pt, m_Sizes[i], m_szNames[i],
				textColor, m_scChSize, eTextAlignment, tempflags );
		}
	}

	// draw rectangle
	UI_DrawRectangle( m_scPos, m_scSize, uiInputFgColor );
}

void CMenuSwitch::UpdateEditable()
{
	m_iState = EngFuncs::GetCvarFloat( m_szCvarName );
}
