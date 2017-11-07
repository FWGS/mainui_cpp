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
#include "YesNoMessageBox.h"

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
	void Draw(); // put test mode timer here
public:
	CMenuVidModes() : CMenuFramework( "CMenuVidModes" ) { testModeTimer = 0; }
	void SetConfig();
	void RevertChanges();
	void ApplyChanges();

	DECLARE_EVENT_TO_MENU_METHOD( CMenuVidModes, SetConfig );
	DECLARE_EVENT_TO_MENU_METHOD( CMenuVidModes, RevertChanges );
	DECLARE_EVENT_TO_MENU_METHOD( CMenuVidModes, ApplyChanges );

	CMenuCheckBox	windowed;
	CMenuCheckBox	vsync;

	CMenuTable	vidList;
	CMenuVidModesModel vidListModel;

	CMenuYesNoMessageBox testModeMsgBox;

	int prevMode;
	bool prevFullscreen;
	float testModeTimer;
	char testModeMsg[256];
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
	bool testMode = false;
	if( prevMode != vidList.GetCurrentIndex() )
	{
		EngFuncs::CvarSetValue( "vid_mode", vidList.GetCurrentIndex() );
		testMode = true;
	}

	if( prevFullscreen == windowed.bChecked )
	{
		EngFuncs::CvarSetValue( "fullscreen", !windowed.bChecked );
		testMode = true;
	}

	vsync.WriteCvar();

	if( testMode )
	{
		testModeMsgBox.Show();
		testModeTimer = gpGlobals->time + 10.0f; // ten seconds should be enough
	}
	else
	{
		// We're done now, just close
		Hide();
	}
}

void CMenuVidModes::ApplyChanges()
{
	prevMode = EngFuncs::GetCvarFloat( "vid_mode" );
	prevFullscreen = EngFuncs::GetCvarFloat( "fullscreen" );
}

void CMenuVidModes::RevertChanges()
{
	EngFuncs::CvarSetValue( "vid_mode", prevMode );
	EngFuncs::CvarSetValue( "fullscreen", prevFullscreen );
}

void CMenuVidModes::Draw()
{
	if( testModeMsgBox.IsVisible() )
	{
		if( testModeTimer - gpGlobals->time > 0 )
		{
			snprintf( testModeMsg, sizeof( testModeMsg ) - 1, "Keep this resolution? %i seconds remaining", (int)(testModeTimer - gpGlobals->time) );
			testModeMsg[sizeof(testModeMsg)-1] = 0;
		}
		else
		{
			RevertChanges();
			testModeMsgBox.Hide();
		}
	}
	CMenuFramework::Draw();
}

/*
=================
UI_VidModes_Init
=================
*/
void CMenuVidModes::_Init( void )
{
	banner.SetPicture(ART_BANNER);

	vidList.SetRect( 360, 255, -20, 340 );
	vidList.SetupColumn( 0, MenuStrings[IDS_VIDEO_MODECOL], 1.0f );
	vidList.SetModel( &vidListModel );

	windowed.SetNameAndStatus( "Run in a window", "Run game in window mode" );
	windowed.SetCoord( 360, 620 );

	vsync.SetNameAndStatus( "Vertical sync", "Enable vertical synchronization" );
	vsync.SetCoord( 360, 670 );
	vsync.LinkCvar( "gl_swapInterval" );

	testModeMsgBox.SetMessage( testModeMsg );
	testModeMsgBox.onPositive = ApplyChangesCb;
	testModeMsgBox.onNegative = RevertChangesCb;
	testModeMsgBox.Link( this );

	AddItem( background );
	AddItem( banner );
	AddButton( "Apply", "Apply changes", PC_OK, SetConfigCb );
	AddButton( "Cancel", "Return back to previous menu", PC_CANCEL, HideCb );
	AddItem( windowed );
	AddItem( vsync );
	AddItem( vidList );
}

void CMenuVidModes::_VidInit()
{
	// don't overwrite prev values
	if( !testModeMsgBox.IsVisible() )
	{
		prevMode = EngFuncs::GetCvarFloat( "vid_mode" );
		vidList.SetCurrentIndex( prevMode );

		prevFullscreen = EngFuncs::GetCvarFloat( "fullscreen" );
		windowed.bChecked = !prevFullscreen;
	}
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
