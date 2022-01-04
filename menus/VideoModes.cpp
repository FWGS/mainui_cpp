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
#include "SpinControl.h"
#include "utlvector.h"

#define ART_BANNER		"gfx/shell/head_vidmodes"

enum
{
	VID_NOMODE = -2, // engine values
	VID_AUTOMODE = -1,
	VID_NOMODE_POS = 0, // values in array
	VID_AUTOMODE_POS = 1,
	VID_MODES_POS = 2, // there starts engine modes
};

class CMenuVidModesModel : public CMenuBaseModel
{
public:
	void Update();
	int GetColumns() const { return 1; }
	int GetRows() const { return m_iNumModes; }
	const char *GetCellText(int line, int column) { return m_szModes[line]; }
private:
	int m_iNumModes;
	const char *m_szModes[64];
};

class CMenuRenderersModel : public CMenuBaseArrayModel
{
public:
	void Update() override
	{
		refdll temp;

		for( unsigned int i = 0;
			EngFuncs::GetRenderers( i, temp.shortName, sizeof( temp.shortName ), temp.readable, sizeof( temp.readable ));
			i++ )
		{
			m_refs.AddToTail( temp );
		}
	}
	int GetRows() const override { return m_refs.Count(); }
	const char *GetText( int i ) override { return m_refs[i].readable; }

	const char *GetShortName( int i ) { return m_refs[i].shortName; }
	void Add( const char *str )
	{
		refdll temp;

		Q_strncpy( temp.shortName, str, sizeof( temp.shortName ));
		Q_strncpy( temp.readable, str, sizeof( temp.readable ));

		m_refs.AddToTail( temp );
	}
private:
	struct refdll
	{
		char shortName[64];
		char readable[64];
	};

	CUtlVector<refdll> m_refs;
};

class CMenuVidModes : public CMenuFramework
{
private:
	void _Init();
	void _VidInit();
	void Draw(); // put test mode timer here
public:
	CMenuVidModes() : CMenuFramework( "CMenuVidModes" ) { testModeTimer = 0; }

	void SetMode( int mode );
	void SetMode( int w, int h );
	void SetConfig( );
	void RevertChanges();
	void ApplyChanges();
	void GetRendererConfig()
	{
		const char *refdll = EngFuncs::GetCvarString( "r_refdll" );

		if( !refdll[0] )
		{
			renderers.SetCurrentValue( 0.0f );
		}

		int i;
		for( i = 0; i < renderersModel.GetRows(); i++ )
		{
			if( !stricmp( renderersModel.GetShortName( i ), refdll ) )
			{
				renderers.SetCurrentValue( i );
				break;
			}
		}

		if( i == renderersModel.GetRows() )
		{
			renderersModel.Add( refdll );
			renderers.SetCurrentValue( i );
		}
	}

	void WriteRendererConfig()
	{
		int i = renderers.GetCurrentValue();
		EngFuncs::CvarSetString( "r_refdll", renderersModel.GetShortName( i ));
	}

	CMenuCheckBox	windowed;
	CMenuCheckBox	vsync;

	CMenuTable	vidList;
	CMenuVidModesModel vidListModel;

	CMenuYesNoMessageBox testModeMsgBox;

	CMenuRenderersModel renderersModel;
	CMenuSpinControl renderers;

	int prevMode;
	int prevModeX;
	int prevModeY;
	bool prevFullscreen;
	float testModeTimer;
	char testModeMsg[256];
};


/*
=================
UI_VidModes_GetModesList
=================
*/
void CMenuVidModesModel::Update( void )
{
	unsigned int i;

	m_szModes[0] = L( "<Current window size>" );
	m_szModes[1] = L( "<Desktop size>" );

	for( i = VID_MODES_POS; i < 64 - VID_MODES_POS; i++ )
	{
		const char *mode = EngFuncs::GetModeString( i - VID_MODES_POS );
		if( !mode ) break;
		m_szModes[i] = mode;
	}
	m_iNumModes = i;
}

void CMenuVidModes::SetMode( int w, int h )
{
	// only possible on Xash3D FWGS!
	char cmd[64];
	snprintf( cmd, sizeof( cmd ), "vid_setmode %i %i\n", w, h );
	EngFuncs::ClientCmd( TRUE, cmd );
}

void CMenuVidModes::SetMode( int mode )
{
	char cmd[64];

	// vid_setmode is a new command, which does not depends on 
	// static resolution list but instead uses dynamic resolution
	// list provided by video backend
	snprintf( cmd, sizeof( cmd ), "vid_setmode %i\n", mode );

	EngFuncs::ClientCmd( TRUE, cmd );
}

/*
=================
UI_VidModes_SetConfig
=================
*/
void CMenuVidModes::SetConfig( )
{
	bool testMode = false;
	int  currentModeIndex = vidList.GetCurrentIndex() - VID_MODES_POS;
	bool isVidModeChanged = prevMode != currentModeIndex;
	bool isWindowedModeChanged = prevFullscreen != !windowed.bChecked;

	/*
	checking windowed mode first because it'll be checked next in
	screen resolution changing code, otherwise when user try to
	change screen resolution and windowed flag at same time,
	only resolution will be changed.
	*/
	if( isWindowedModeChanged )
	{
		EngFuncs::CvarSetValue( "fullscreen", !windowed.bChecked );
		// moved to fullscreen, enable test mode
		testMode |= !windowed.bChecked;
	}

	if( isVidModeChanged )
	{
		SetMode( currentModeIndex );
		EngFuncs::CvarSetValue( "vid_mode", currentModeIndex );
		vidList.SetCurrentIndex( currentModeIndex + VID_MODES_POS );
		// have changed resolution, but enable test mode only in fullscreen
		testMode |= !windowed.bChecked;
	}

	vsync.WriteCvar();

	if( testMode )
	{
		testModeMsgBox.Show();
		testModeTimer = gpGlobals->time + 10.0f; // ten seconds should be enough
	}
	else
	{
		Hide();
	}
}

void CMenuVidModes::ApplyChanges()
{
	prevMode = EngFuncs::GetCvarFloat( "vid_mode" );
	prevFullscreen = EngFuncs::GetCvarFloat( "fullscreen" );
	prevModeX = EngFuncs::GetCvarFloat( "width" );
	prevModeY = EngFuncs::GetCvarFloat( "height" );
}

void CMenuVidModes::RevertChanges()
{
	bool fullscreenChanged = false;

	// if we switched FROM fullscreen TO windowed, then we must get rid of fullscreen mode
	// so we can easily change the window size, without jerking display mode
	if( prevFullscreen == false && EngFuncs::GetCvarFloat( "fullscreen" ) != 0.0f )
	{
		EngFuncs::CvarSetValue( "fullscreen", prevFullscreen );
		fullscreenChanged = true;
	}

	SetMode( prevModeX, prevModeY );

	// otherwise, we better to set window size at first, then switch TO fullscreen
	if( !fullscreenChanged )
	{
		EngFuncs::CvarSetValue( "fullscreen", prevFullscreen );
	}
}

void CMenuVidModes::Draw()
{
	if( testModeMsgBox.IsVisible() )
	{
		if( testModeTimer - gpGlobals->time > 0 )
		{
			snprintf( testModeMsg, sizeof( testModeMsg ) - 1, L( "Keep this resolution? %i seconds remaining" ), (int)(testModeTimer - gpGlobals->time) );
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

	vidList.SetRect( 360, 230, -20, 365 );
	vidList.SetupColumn( 0, L( "GameUI_Resolution" ), 1.0f );
	vidList.SetModel( &vidListModel );

	windowed.SetNameAndStatus( L( "GameUI_Windowed" ), L( "GameUI_Windowed" ) );
	windowed.SetCoord( 360, 620 );
	SET_EVENT_MULTI( windowed.onChanged,
	{
		CMenuVidModes *parent = pSelf->GetParent(CMenuVidModes);
		if( !parent->windowed.bChecked && parent->vidList.GetCurrentIndex() < VID_AUTOMODE_POS )
			parent->vidList.SetCurrentIndex( VID_AUTOMODE_POS );
	});

	SET_EVENT_MULTI( vidList.onChanged,
	{
		CMenuVidModes *parent = pSelf->GetParent(CMenuVidModes);
		if( !parent->windowed.bChecked && parent->vidList.GetCurrentIndex() < VID_AUTOMODE_POS )
			parent->vidList.SetCurrentIndex( VID_AUTOMODE_POS );
	});

	vsync.SetNameAndStatus( L( "GameUI_VSync" ), L( "GameUI_VSync" ) );
	vsync.SetCoord( 360, 670 );
	vsync.LinkCvar( "gl_vsync" );

	testModeMsgBox.SetMessage( testModeMsg );
	testModeMsgBox.onPositive = VoidCb( &CMenuVidModes::ApplyChanges );
	testModeMsgBox.onNegative = VoidCb( &CMenuVidModes::RevertChanges );
	testModeMsgBox.Link( this );

	renderersModel.Update();
	renderers.szName = L( "GameUI_Renderer" );
	renderers.Setup( &renderersModel );
	renderers.SetRect( 80, 480, 250, 32 );
	renderers.SetCharSize( QM_SMALLFONT );
	renderers.onCvarGet = VoidCb( &CMenuVidModes::GetRendererConfig );
	renderers.onCvarWrite = VoidCb( &CMenuVidModes::WriteRendererConfig );
	renderers.bUpdateImmediately = true;

	AddItem( background );
	AddItem( banner );
	AddButton( L( "GameUI_Apply" ), L( "Apply changes" ), PC_OK, VoidCb( &CMenuVidModes::SetConfig ) );
	AddButton( L( "GameUI_Cancel" ), L( "Return back to previous menu" ), PC_CANCEL, VoidCb( &CMenuVidModes::Hide ) );
	AddItem( renderers );
	AddItem( windowed );
	AddItem( vsync );
	AddItem( vidList );

	renderers.LinkCvar( "r_refdll", CMenuEditable::CVAR_STRING );
}

void CMenuVidModes::_VidInit()
{
	// don't overwrite prev values
	if( !testModeMsgBox.IsVisible() )
	{
		ApplyChanges( );
		vidList.SetCurrentIndex( prevMode + VID_MODES_POS );
		windowed.bChecked = !prevFullscreen;
	}
}

ADD_MENU( menu_vidmodes, CMenuVidModes, UI_VidModes_Menu );
