/*
Copyright (C) 1997-2001 Id Software, Inc.

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

#include "extdll.h"
#include "BaseMenu.h"
#include "Utils.h"
#include "kbutton.h"
#include "BtnsBMPTable.h"
#include "MenuStrings.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "CheckBox.h"
#include "Slider.h"

#define ART_BANNER			"gfx/shell/head_advanced"

class CAdvancedControls : public CMenuFramework
{
public:

private:
	virtual void _Init( void );
	virtual void _VidInit( void );

	void GetConfig( void );

	CMenuBackgroundBitmap background;
	CMenuBannerBitmap banner;

	CMenuPicButton done;

	CMenuCheckBox	crosshair;
	CMenuCheckBox	invertMouse;
	CMenuCheckBox	mouseLook;
	CMenuCheckBox	lookSpring;
	CMenuCheckBox	lookStrafe;
	CMenuCheckBox	mouseFilter;
	CMenuCheckBox	autoaim;
	CMenuSlider	sensitivity;

};

CAdvancedControls	uiAdvControls;

/*
=================
UI_AdvControls_GetConfig
=================
*/
void CAdvancedControls::GetConfig( void )
{
	kbutton_t	*mlook;

	if( EngFuncs::GetCvarFloat( "m_pitch" ) < 0 )
		invertMouse.bChecked = true;

	mlook = (kbutton_s *)EngFuncs::KEY_GetState( "in_mlook" );
	if( mlook && mlook->state & 1 )
		mouseLook.bChecked = true;

	crosshair.LinkCvar( "crosshair" );
	lookSpring.LinkCvar( "lookspring" );
	lookStrafe.LinkCvar( "lookstrafe" );
	mouseFilter.LinkCvar( "m_filter" );
	autoaim.LinkCvar( "sv_aim" );
	sensitivity.LinkCvar( "sensitivity" );

	if( mouseLook.bChecked )
	{
		lookSpring.iFlags |= QMF_GRAYED;
		lookStrafe.iFlags |= QMF_GRAYED;
	}
	else
	{
		lookSpring.iFlags &= ~QMF_GRAYED;
		lookStrafe.iFlags &= ~QMF_GRAYED;
	}
}

/*
=================
UI_AdvControls_Init
=================
*/
void CAdvancedControls::_Init( void )
{
	// memset( &uiAdvControls, 0, sizeof( uiAdvControls_t ));

	banner.SetPicture( ART_BANNER );

	done.SetNameAndStatus( "Done", "save changed and go back to the Customize Menu" );
	done.onActivated = PopMenuCb;

	crosshair.SetNameAndStatus( "Crosshair", "Enable the weapon aiming crosshair" );

	invertMouse.SetNameAndStatus( "Invert mouse", "Reverse mouse up/down axis" );
	SET_EVENT( invertMouse, onChanged )
	{
		CMenuCheckBox *self = (CMenuCheckBox*)pSelf;
		float m_pitch = EngFuncs::GetCvarFloat( "m_pitch" );
		if( self->bChecked  && m_pitch > 0 ||
			!self->bChecked && m_pitch < 0 )
			EngFuncs::CvarSetValue( "m_pitch", -m_pitch );
	}
	END_EVENT( invertMouse, onChanged )

	mouseLook.SetNameAndStatus( "Mouse look", "Use the mouse to look around instead of using the mouse to move" );
	SET_EVENT( mouseLook, onChanged )
	{
		CMenuCheckBox *self = (CMenuCheckBox*)pSelf;
		CAdvancedControls *parent = (CAdvancedControls*)self->Parent();
		if( self->bChecked )
		{
			parent->lookSpring.iFlags |= QMF_GRAYED;
			parent->lookStrafe.iFlags |= QMF_GRAYED;
			EngFuncs::ClientCmd( TRUE, "+mlook" );
		}
		else
		{
			parent->lookSpring.iFlags &= ~QMF_GRAYED;
			parent->lookSpring.iFlags &= ~QMF_GRAYED;
			EngFuncs::ClientCmd( TRUE, "-mlook" );
		}
	}
	END_EVENT( mouseLook, onChanged )

	lookSpring.SetNameAndStatus("Look spring", "Causes the screen to 'spring' back to looking straight ahead when you\nmove forward" );

	lookStrafe.SetNameAndStatus( "Look strafe", "In combination with your mouse look modifier, causes left-right movements\nto strafe instead of turn");

	mouseFilter.SetNameAndStatus( "Mouse filter", "Average mouse inputs over the last two frames to smooth out movements" );

	autoaim.SetNameAndStatus( "Autoaim", "Let game to help you aim at enemies" );

	sensitivity.szStatusText = "Set in-game mouse sensitivity";
	sensitivity.Setup( 0.0, 20.0f, 0.1 );

	AddItem( background );
	AddItem( banner );
	AddItem( done );
	AddItem( crosshair );
	AddItem( invertMouse );
	AddItem( mouseLook );
	AddItem( lookSpring );
	AddItem( lookStrafe );
	AddItem( mouseFilter );
	AddItem( autoaim );
	AddItem( sensitivity );
}


void CAdvancedControls::_VidInit()
{
	done.SetCoord( 72, 680 );
	crosshair.SetCoord( 72, 230 );
	invertMouse.SetCoord( 72, 280 );
	mouseLook.SetCoord( 72, 330 );
	lookSpring.SetCoord( 72, 380 );
	lookStrafe.SetCoord( 72, 430 );
	mouseFilter.SetCoord( 72, 480 );
	autoaim.SetCoord( 72, 530 );
	sensitivity.SetCoord( 72, 625 );

	GetConfig();
}

/*
=================
UI_AdvControls_Precache
=================
*/
void UI_AdvControls_Precache( void )
{
	EngFuncs::PIC_Load( ART_BANNER );
}

/*
=================
UI_AdvControls_Menu
=================
*/
void UI_AdvControls_Menu( void )
{
	UI_AdvControls_Precache();
	uiAdvControls.Open();
}
