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
#include "MenuStrings.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "Table.h"
#include "CheckBox.h"
#include "Action.h"

#define ART_BANNER		"gfx/shell/head_vidmodes"

class CMenuVidModesModel : public CMenuBaseModel
{
public:
	void Update();
	int GetColumns() const { return 1; }
	int GetRows() const { return m_iNumModes; }
	const char *GetCellText(int line, int column) { return m_szModes[line]; }
private:
	int m_iNumModes;
	const char *m_szModes[32];
};

class CMenuVidModes : public CMenuFramework
{
private:
	void _Init();
	void _VidInit();
public:
	CMenuVidModes() : CMenuFramework( "CMenuVidModes" ) { }
	void SetConfig();


	CMenuPicButton	ok;
	CMenuPicButton	cancel;
	CMenuCheckBox	windowed;
	CMenuCheckBox	vsync;

	CMenuTable	vidList;
	CMenuVidModesModel vidListModel;
} uiVidModes;


/*
=================
UI_VidModes_GetModesList
=================
*/
void CMenuVidModesModel::Update( void )
{
	unsigned int i;

	for( i = 0; i < 64; i++ )
	{
		const char *mode = EngFuncs::GetModeString( i );
		if( !mode ) break;
		m_szModes[i] = mode;
	}
	m_iNumModes = i;
}

/*
=================
UI_VidModes_SetConfig
=================
*/
void CMenuVidModes::SetConfig( void )
{
	EngFuncs::CvarSetValue( "vid_mode", vidList.iCurItem );
	EngFuncs::CvarSetValue( "fullscreen", !windowed.bChecked );
	vsync.WriteCvar();
}


/*
=================
UI_VidModes_Init
=================
*/
void CMenuVidModes::_Init( void )
{
	banner.SetPicture(ART_BANNER);

	ok.SetCoord( 72, 230 );
	ok.SetNameAndStatus( "Apply", "Apply changes" );
	ok.SetPicture( PC_OK );
	SET_EVENT( ok, onActivated )
	{
		((CMenuVidModes*)pSelf->Parent())->SetConfig();
		pSelf->Parent()->Hide();
	}
	END_EVENT( ok, onActivated )

	cancel.SetCoord( 72, 280 );
	cancel.SetNameAndStatus( "Cancel", "Return back to previous menu" );
	cancel.SetPicture( PC_CANCEL );
	cancel.onActivated = HideCb;

	vidList.SetRect( 400, 300, 560, 300 );
	vidList.SetupColumn( 0, MenuStrings[IDS_VIDEO_MODECOL], 1.0f );
	vidList.SetModel( &vidListModel );

	windowed.SetNameAndStatus( "Run in a window", "Run game in window mode" );
	windowed.SetCoord( 400, 620 );

	vsync.SetNameAndStatus( "Vertical sync", "Enable vertical synchronization" );
	vsync.SetCoord( 400, 670 );
	vsync.LinkCvar( "gl_swapInterval" );

	AddItem( background );
	AddItem( banner );
	AddItem( ok );
	AddItem( cancel );
	AddItem( windowed );
	AddItem( vsync );
	AddItem( vidList );
}

void CMenuVidModes::_VidInit()
{
	vidList.iCurItem = EngFuncs::GetCvarFloat( "vid_mode" );

	windowed.bChecked = !EngFuncs::GetCvarFloat( "fullscreen" );
}

/*
=================
UI_VidModes_Precache
=================
*/
void UI_VidModes_Precache( void )
{
	EngFuncs::PIC_Load( ART_BANNER );
}

/*
=================
UI_VidModes_Menu
=================
*/
void UI_VidModes_Menu( void )
{
	UI_VidModes_Precache();
	uiVidModes.Show();
}
