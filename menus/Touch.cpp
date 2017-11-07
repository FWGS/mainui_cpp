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

#include "Framework.h"
#include "Bitmap.h"
#include "PicButton.h"

#define ART_BANNER		"gfx/shell/head_touch"

class CMenuTouch : public CMenuFramework
{
public:
	CMenuTouch() : CMenuFramework( "CMenuTouch" ) { }

private:
	void _Init();
};

static CMenuTouch	uiTouch;

/*
=================
UI_Touch_Init
=================
*/
void CMenuTouch::_Init( void )
{
	banner.SetPicture(ART_BANNER );
	AddItem( background );
	AddItem( banner );

	AddButton( "Touch options", "Touch sensitivity and profile options", "gfx/shell/btn_touch_options",
		UI_TouchOptions_Menu, QMF_NOTIFY );

	AddButton( "Touch buttons", "Add, remove, edit touch buttons", "gfx/shell/btn_touch_buttons",
		UI_TouchButtons_Menu, QMF_NOTIFY );

	AddButton( "Done",  "Go back to the previous menu", PC_DONE, HideCb, QMF_NOTIFY );

}


/*
=================
UI_Touch_Precache
=================
*/
void UI_Touch_Precache( void )
{
	EngFuncs::PIC_Load( ART_BANNER );
}

/*
=================
UI_Touch_Menu
=================
*/
void UI_Touch_Menu( void )
{
	UI_Touch_Precache();
	uiTouch.Show();
}

