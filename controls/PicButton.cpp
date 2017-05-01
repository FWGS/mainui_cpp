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

#include "extdll.h"
#include "BaseMenu.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "Utils.h"

CMenuPicButton::CMenuPicButton() : CMenuBaseItem()
{
	bEnableTransitions = true;
	eFocusAnimation = QM_HIGHLIGHTIFFOCUS;
	iFlags = QMF_DROPSHADOW;

	hPic = 0;
	button_id = 0;
}

/*
=================
CMenuPicButton::Init
=================
*/
void CMenuPicButton::VidInit( void )
{
	if( size.w < 1 || size.h < 1 )
	{
		if( size.w < 1 )
			size.w = charSize.w * strlen( szName );

		if( size.h < 1 )
			size.h = charSize.h * 1.5;
	}

	m_scPos = pos.Scale();
	m_scSize = size.Scale();
	m_scChSize = charSize.Scale();
}

/*
=================
CMenuPicButton::Key
=================
*/
const char *CMenuPicButton::Key( int key, int down )
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
		if( iFlags & QMF_MOUSEONLY )
			break;
		sound = uiSoundLaunch;
	}
	if( sound && ( iFlags & QMF_SILENT ))
		sound = uiSoundNull;

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
				event = QM_ACTIVATED;

			TACheckMenuDepth();
			_Event( event );
			SetTitleAnim( AS_TO_TITLE );
		}
		else if( down )
		{
			_Event( QM_ACTIVATED );
		}
	}

	return sound;
}

/*
=================
CMenuPicButton::Draw
=================
*/
void CMenuPicButton::Draw( )
{
	int state = BUTTON_NOFOCUS;

	if( iFlags & (QMF_HASMOUSEFOCUS|QMF_HASKEYBOARDFOCUS))
	{
		state = BUTTON_FOCUS;
	}

	// make sure what cursor in rect
	if( m_bPressed && g_bCursorDown )
		state = BUTTON_PRESSED;
	else m_bPressed = false;

	if( szStatusText && iFlags & QMF_NOTIFY )
	{
		int	x;

		x = 290;
		UI_ScaleCoords( &x, NULL, NULL, NULL );
		x += m_scPos.x;

		int	r, g, b;

		UnpackRGB( r, g, b, uiColorHelp );
		EngFuncs::DrawSetTextColor( r, g, b );
		EngFuncs::DrawConsoleString( x, m_scPos.y, szStatusText );
	}

	if( hPic )
	{
		int r, g, b, a;

		UnpackRGB( r, g, b, iFlags & QMF_GRAYED ? uiColorDkGrey : uiColorWhite );

		wrect_t rects[]=
		{
		{ 0, uiStatic.buttons_width, 0, 26 },
		{ 0, uiStatic.buttons_width, 26, 52 },
		{ 0, uiStatic.buttons_width, 52, 78 }
		};

		EngFuncs::PIC_EnableScissor( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height - 2 );

		a = (512 - (uiStatic.realTime - m_iLastFocusTime)) >> 1;

		if( state == BUTTON_NOFOCUS && a > 0 )
		{
			EngFuncs::PIC_Set( hPic, r, g, b, a );
			EngFuncs::PIC_DrawAdditive( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height, &rects[BUTTON_FOCUS] );
		}

		EngFuncs::PIC_Set( hPic, r, g, b, 255 );

		// pulse code.
		if( ( state == BUTTON_NOFOCUS && bPulse ) ||
			( state == BUTTON_FOCUS   && eFocusAnimation == QM_PULSEIFFOCUS ) )
		{
			EngFuncs::PIC_Set( hPic, r, g, b, 255 *(0.5 + 0.5 * sin( (float)uiStatic.realTime / ( UI_PULSE_DIVISOR * 2 ))));
			EngFuncs::PIC_DrawAdditive( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height, &rects[BUTTON_FOCUS] );

			EngFuncs::PIC_DrawAdditive( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height, &rects[BUTTON_NOFOCUS] );
		}
		else
		{
			// special handling for focused
			if( state == BUTTON_FOCUS )
			{
				EngFuncs::PIC_DrawAdditive( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height, &rects[BUTTON_FOCUS] );

				EngFuncs::PIC_Set( hPic, r, g, b, 255 ); // set colors again
				EngFuncs::PIC_DrawAdditive( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height, &rects[BUTTON_NOFOCUS] );
			}
			else
			{
				// just draw
				EngFuncs::PIC_DrawAdditive( m_scPos.x, m_scPos.y, uiStatic.buttons_draw_width, uiStatic.buttons_draw_height, &rects[state] );
			}
		}

		EngFuncs::PIC_DisableScissor();
	}
	else
	{
		int	shadow;

		shadow = (iFlags & QMF_DROPSHADOW);

		if( iFlags & QMF_GRAYED )
		{
			UI_DrawString( m_scPos, m_scSize, szName, uiColorDkGrey, true, m_scChSize, eTextAlignment, shadow );
			return; // grayed
		}

		if(this != m_pParent->ItemAtCursor())
		{
			UI_DrawString( m_scPos, m_scSize, szName, iColor, false, m_scChSize, eTextAlignment, shadow );
			return; // no focus
		}

		if(!( iFlags & QMF_FOCUSBEHIND ))
			UI_DrawString( m_scPos, m_scSize, szName, iColor, false, m_scChSize, eTextAlignment, shadow );

		if( eFocusAnimation == QM_HIGHLIGHTIFFOCUS )
			UI_DrawString( m_scPos, m_scSize, szName, iFocusColor, false, m_scChSize, eTextAlignment, shadow );
		else if( eFocusAnimation == QM_PULSEIFFOCUS )
		{
			int	color;

			color = PackAlpha( iColor, 255 * (0.5 + 0.5 * sin( (float)uiStatic.realTime / UI_PULSE_DIVISOR )));

			UI_DrawString( m_scPos, m_scSize, szName, color, false, m_scChSize, eTextAlignment, shadow );
		}

		if( iFlags & QMF_FOCUSBEHIND )
			UI_DrawString( m_scPos, m_scSize, szName, iColor, false, m_scChSize, eTextAlignment, shadow );
	}
}

void CMenuPicButton::SetPicture( int ID )
{
	if( ID < 0 || ID > PC_BUTTONCOUNT )
		return; // bad id

#if 0	// too different results on various games. disabled
	width = PicButtonWidth( ID ) * UI_BUTTON_CHARWIDTH;
#else
	size.w = UI_BUTTONS_WIDTH;
#endif
	size.h = UI_BUTTONS_HEIGHT;

	hPic = uiStatic.buttonsPics[ID];
	button_id = ID;

	if( hPic ) // text buttons not use it
		iFlags |= QMF_ACT_ONRELEASE;
}

void CMenuPicButton::SetPicture(const char *filename)
{
	size.w = UI_BUTTONS_WIDTH;
	size.h = UI_BUTTONS_HEIGHT;

	hPic = EngFuncs::PIC_Load( filename );

	if( hPic ) // text buttons not use it
		iFlags |= QMF_ACT_ONRELEASE;

}

// ============================ Animations ===========================

// Title Transition Time period
#define TTT_PERIOD		200.0f


CMenuPicButton *ButtonStack[UI_MAX_MENUDEPTH];
int		ButtonStackDepth;

CMenuPicButton::Quad	CMenuPicButton::TitleLerpQuads[2];
int		CMenuPicButton::transition_state = CMenuPicButton::AS_TO_TITLE;
HIMAGE	CMenuPicButton::TransPic = 0;
wrect_t*CMenuPicButton::TransRect;
bool	CMenuPicButton::hold_button_stack = false;
int		CMenuPicButton::transition_initial_time;
int		CMenuPicButton::PreClickDepth;


void CMenuPicButton::TACheckMenuDepth( void )
{
	PreClickDepth = uiStatic.menuDepth;
}


void CMenuPicButton::PopPButtonStack()
{
	if( hold_button_stack ) return;

	if( ButtonStack[ButtonStackDepth] )
		ButtonStack[ButtonStackDepth]->SetTitleAnim( AS_TO_BUTTON );

	if( ButtonStackDepth )
		ButtonStackDepth--;
}

void CMenuPicButton::PushPButtonStack()
{
	if( ButtonStack[ButtonStackDepth] == this )
		return;

	ButtonStack[++ButtonStackDepth] = this;
}

float CMenuPicButton::GetTitleTransFraction( void )
{
	float fraction = (float)(uiStatic.realTime - transition_initial_time ) / TTT_PERIOD;

	if( fraction > 1.0f )
		fraction = 1.0f;

	return fraction;
}


CMenuPicButton::Quad CMenuPicButton::LerpQuad( Quad a, Quad b, float frac )
{
	Quad c;
	c.x = a.x + (b.x - a.x) * frac;
	c.y = a.y + (b.y - a.y) * frac;
	c.lx = a.lx + (b.lx - a.lx) * frac;
	c.ly = a.ly + (b.ly - a.ly) * frac;

	return c;
}

// TODO: Find CMenuBannerBitmap in next menu page and correct
void CMenuPicButton::SetupTitleQuad( int x, int y, int w, int h )
{
	TitleLerpQuads[1].x  = x * ScreenHeight / 768;
	TitleLerpQuads[1].y  = y * ScreenHeight / 768;
	TitleLerpQuads[1].lx = w * ScreenHeight / 768;
	TitleLerpQuads[1].ly = h * ScreenHeight / 768;
}

void CMenuPicButton::SetTransPic(HIMAGE pic, wrect_t *r )
{
	TransPic = pic;
	TransRect = r;
}

void CMenuPicButton::DrawTitleAnim()
{
	if( !TransPic ) return;

#if 1
	float frac = GetTitleTransFraction();
#else
	float frac = (sin(gpGlobals->time*4)+1)/2;
#endif

#ifdef TA_ALT_MODE
	if( frac == 1 && transition_state == AS_TO_BUTTON )
		return;
#else
	if( frac == 1 )
		return;
#endif

	Quad c;

	int f_idx = (transition_state == AS_TO_TITLE) ? 0 : 1;
	int s_idx = (transition_state == AS_TO_TITLE) ? 1 : 0;

	c = LerpQuad( TitleLerpQuads[f_idx], TitleLerpQuads[s_idx], frac );

	EngFuncs::PIC_Set( TransPic, 255, 255, 255 );
	EngFuncs::PIC_DrawAdditive( c.x, c.y, c.lx, c.ly, TransRect );
}

void CMenuPicButton::SetTitleAnim( int anim_state )
{
	static	wrect_t r = { 0, uiStatic.buttons_width, 26, 51 };
	CMenuPicButton *button = this;

	// check this before any button changes
	if( !bEnableTransitions )
		return;

	// skip buttons which don't call new menu
	if( uiStatic.menuDepth && !uiStatic.menuStack[uiStatic.menuDepth-1]->IsRoot() )
		return;

	if( PreClickDepth == uiStatic.menuDepth && anim_state == AS_TO_TITLE )
		return;

	// replace cancel\done button with button which called this menu
	if( PreClickDepth > uiStatic.menuDepth && anim_state == AS_TO_TITLE )
	{
		anim_state = AS_TO_BUTTON;

		// HACK HACK HACK
		if( ButtonStack[ButtonStackDepth + 1] )
			button = ButtonStack[ButtonStackDepth+1];
	}

	// don't reset anim if dialog buttons pressed
	//if( button->id == ID_YES || button->id == ID_NO )
	//	return;

	if( !button->bEnableTransitions )
		return;

	if( anim_state == AS_TO_TITLE )
		PushPButtonStack();

	transition_state = anim_state;

	TitleLerpQuads[0].x = button->m_scPos.x;
	TitleLerpQuads[0].y = button->m_scPos.y;
	TitleLerpQuads[0].lx = button->m_scSize.w;
	TitleLerpQuads[0].ly = button->m_scSize.h;

	transition_initial_time = uiStatic.realTime;
#ifdef TA_ALT_MODE2
	if( !TransPic )
#endif
	{
		button->SetTransPic( button->hPic, &r );
	}
}

void CMenuPicButton::InitTitleAnim()
{
	memset( TitleLerpQuads, 0, sizeof( CMenuPicButton::Quad ) * 2 );

	SetupTitleQuad( UI_BANNER_POSX, UI_BANNER_POSY, UI_BANNER_WIDTH, UI_BANNER_HEIGHT );

	ButtonStackDepth = 0;
	memset( ButtonStack, 0, sizeof( ButtonStack ));
}

void CMenuPicButton::ClearButtonStack( void )
{
	ButtonStackDepth = 0;
	memset( ButtonStack, 0, sizeof( ButtonStack ));
}
