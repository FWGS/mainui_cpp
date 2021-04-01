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
#include "keydefs.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "Table.h"
#include "Action.h"
#include "YesNoMessageBox.h"

#define ART_BANNER_LOAD "gfx/shell/head_load"
#define ART_BANNER_SAVE "gfx/shell/head_save"

#define LEVELSHOT_X		72
#define LEVELSHOT_Y		400
#define LEVELSHOT_W		192
#define LEVELSHOT_H		160

#define MAX_CELLSTRING CS_SIZE

class CMenuLoadGame;

class CMenuSavePreview : public CMenuBaseItem
{
public:
	CMenuSavePreview() : CMenuBaseItem(), fallback( "{GRAF001" )
	{
		iFlags = QMF_INACTIVE;
	}

	void Draw() override;

	CImage fallback;
};

class CMenuSavesListModel : public CMenuBaseModel
{
public:
	CMenuSavesListModel( CMenuLoadGame *parent ) : parent( parent ) { }

	void Update() override;
	int GetColumns() const override
	{
		// time, name, gametime
		return 3;
	}
	int GetRows() const override
	{
		return m_iNumItems;
	}
	const char *GetCellText( int line, int column ) override
	{
		return m_szCells[line][column];
	}
	unsigned int GetAlignmentForColumn(int column) const override
	{
		if( column == 2 )
			return QM_RIGHT;
		return QM_LEFT;
	}
	void OnDeleteEntry( int line ) override;

	char		saveName[UI_MAXGAMES][CS_SIZE];
	char		delName[UI_MAXGAMES][CS_SIZE];

private:
	CMenuLoadGame *parent;
	char		m_szCells[UI_MAXGAMES][3][MAX_CELLSTRING];
	int			m_iNumItems;
};

class CMenuLoadGame : public CMenuFramework
{
public:
	CMenuLoadGame() : CMenuFramework( "CMenuLoadGame" ), savesListModel( this ) { }

	// true to turn this menu into save mode, false to turn into load mode
	void SetSaveMode( bool saveMode );
	bool IsSaveMode() { return m_fSaveMode; }
	void UpdateList() { savesListModel.Update(); }

private:
	void _Init( void );

	void LoadGame();
	void SaveGame();
	void UpdateGame();
	void DeleteGame();

	CMenuPicButton	load;
	CMenuPicButton  save;
	CMenuPicButton	remove;
	CMenuPicButton	cancel;

	CMenuTable	savesList;

	CMenuSavePreview	levelShot;
	bool m_fSaveMode;
	char		hintText[MAX_HINT_TEXT];

	// prompt dialog
	CMenuYesNoMessageBox msgBox;
	CMenuSavesListModel savesListModel;

	friend class CMenuSavesListModel;
};

void CMenuSavePreview::Draw()
{
	if( szName && *szName )
	{
		char saveshot[128];

		snprintf( saveshot, sizeof( saveshot ),
				  "save/%s.bmp", szName );

		if( EngFuncs::FileExists( saveshot ))
			UI_DrawPic( m_scPos, m_scSize, uiColorWhite, saveshot );
		else
			UI_DrawPic( m_scPos, m_scSize, uiColorWhite, fallback, QM_DRAWADDITIVE );
	}
	else
		UI_DrawPic( m_scPos, m_scSize, uiColorWhite, fallback, QM_DRAWADDITIVE );

	// draw the rectangle
	UI_DrawRectangle( m_scPos, m_scSize, uiInputFgColor );
}

/*
=================
CMenuSavesListModel::Update
=================
*/
void CMenuSavesListModel::Update( void )
{
	char	comment[256];
	char	**filenames;
	int	i = 0, j, numFiles;

	filenames = EngFuncs::GetFilesList( "save/*.sav", &numFiles, TRUE );

	// sort the saves in reverse order (oldest past at the end)
	qsort( filenames, numFiles, sizeof( char* ), (cmpfunc)COM_CompareSaves );

	if ( parent->IsSaveMode() && CL_IsActive() )
	{
		// create new entry for current save game
		Q_strncpy( saveName[i], L( "GameUI_SaveGame_New" ), CS_SIZE );
		Q_strncpy( delName[i], "", CS_SIZE );
		strcpy( m_szCells[i][0], L( "GameUI_SaveGame_Current" ) );
		strcpy( m_szCells[i][1], L( "GameUI_SaveGame_NewSavedGame" ) );
		strcpy( m_szCells[i][2], L( "GameUI_SaveGame_New" ) );
		i++;
	}

	for ( j = 0; j < numFiles; i++, j++ )
	{
		if( i >= UI_MAXGAMES ) break;

		if( !EngFuncs::GetSaveComment( filenames[j], comment ))
		{
			if( comment[0] )
			{
				// get name string even if not found - SV_GetComment can be mark saves
				// as <CORRUPTED> <OLD VERSION> etc
				Q_strncpy( m_szCells[i][0], comment, MAX_CELLSTRING );
				m_szCells[i][1][0] = 0;
				m_szCells[i][2][0] = 0;
				COM_FileBase( filenames[j], saveName[i] );
				COM_FileBase( filenames[j], delName[i] );
			}
			continue;
		}

		// strip path, leave only filename (empty slots doesn't have savename)
		COM_FileBase( filenames[j], saveName[i] );
		COM_FileBase( filenames[j], delName[i] );

		// they are defined by comment string format
		const char *time = comment + CS_SIZE;
		const char *date = comment + CS_SIZE + CS_TIME;
		const char *elapsedTime = comment + CS_SIZE + CS_TIME * 2;

		char *title = comment;
		char type[CS_SIZE] = {}, *p = nullptr;
		const char *translated_title = nullptr;

		// if comments begin with [ and there is second ]
		if( comment[0] == '[' && ( p = strchr( comment, ']' )))
		{
			title = p + 1;
			Q_strncpy( type, comment, title - comment + 1 );
		}

		if( *title == '#' )
		{
			// strip everything after first space
			char s[CS_SIZE];

			p = strchr( title, ' ' );

			size_t len = p ? (p - title + 1) : ( CS_SIZE - ( title - comment ));

			Q_strncpy( s, title, len );
			translated_title = L( s );
		}
		else
		{
			translated_title = title;
		}

		// fill save desc
		snprintf( m_szCells[i][0], MAX_CELLSTRING, "%s %s", time, date );
		snprintf( m_szCells[i][1], MAX_CELLSTRING, "%s%s", type, translated_title );
		Q_strncpy( m_szCells[i][2], elapsedTime, MAX_CELLSTRING );

	}

	m_iNumItems = i;

	if ( saveName[0][0] == 0 )
	{
		parent->load.SetGrayed( true );
	}
	else
	{
		parent->levelShot.szName = saveName[0];
		parent->load.SetGrayed( false );
	}

	if ( saveName[0][0] == 0 || !CL_IsActive() )
		parent->save.SetGrayed( true );
	else parent->save.SetGrayed( false );

	if ( delName[0][0] == 0 )
		parent->remove.SetGrayed( true );
	else parent->remove.SetGrayed( false );
}

void CMenuSavesListModel::OnDeleteEntry(int line)
{
	parent->msgBox.Show();
}

/*
=================
UI_LoadGame_Init
=================
*/
void CMenuLoadGame::_Init( void )
{
	save.SetNameAndStatus( L( "GameUI_Save" ), L( "Save current game" ) );
	save.SetPicture( PC_SAVE_GAME );
	save.onReleased = VoidCb( &CMenuLoadGame::SaveGame );
	save.SetCoord( 72, 230 );

	load.SetNameAndStatus( L( "GameUI_Load" ), L( "Load saved game" ) );
	load.SetPicture( PC_LOAD_GAME );
	load.onReleased = VoidCb( &CMenuLoadGame::LoadGame );
	load.SetCoord( 72, 230 );

	remove.SetNameAndStatus( L( "Delete" ), L( "Delete saved game" ) );
	remove.SetPicture( PC_DELETE );
	remove.onReleased = msgBox.MakeOpenEvent();
	remove.SetCoord( 72, 280 );

	cancel.SetNameAndStatus( L( "GameUI_Cancel" ), L( "Return back to main menu" ) );
	cancel.SetPicture( PC_CANCEL );
	cancel.onReleased = VoidCb( &CMenuLoadGame::Hide );
	cancel.SetCoord( 72, 330 );

	savesList.szName = hintText;
	savesList.onChanged = VoidCb( &CMenuLoadGame::UpdateGame );
	// savesList.onDeleteEntry = msgBox.MakeOpenEvent();
	savesList.SetupColumn( 0, L( "GameUI_Time" ), 0.30f );
	savesList.SetupColumn( 1, L( "GameUI_Game" ), 0.55f );
	savesList.SetupColumn( 2, L( "GameUI_ElapsedTime" ), 0.15f );

	savesList.SetModel( &savesListModel );
	savesList.SetCharSize( QM_SMALLFONT );
	savesList.SetRect( 360, 230, -20, 465 );

	msgBox.SetMessage( L( "Delete this saved game?" ) );
	msgBox.onPositive = VoidCb( &CMenuLoadGame::DeleteGame );
	msgBox.Link( this );

	levelShot.SetRect( LEVELSHOT_X, LEVELSHOT_Y, LEVELSHOT_W, LEVELSHOT_H );

	AddItem( background );
	AddItem( banner );
	AddItem( load );
	AddItem( save );
	AddItem( remove );
	AddItem( cancel );
	AddItem( levelShot );
	AddItem( savesList );
}

void CMenuLoadGame::LoadGame()
{
	const char *saveName = savesListModel.saveName[savesList.GetCurrentIndex()];
	if( saveName[0] )
	{
		char	cmd[128];
		sprintf( cmd, "load \"%s\"\n", saveName );

		EngFuncs::StopBackgroundTrack( );

		EngFuncs::ClientCmd( FALSE, cmd );

		UI_CloseMenu();
	}
}

void CMenuLoadGame::SaveGame()
{
	const char *saveName = savesListModel.saveName[savesList.GetCurrentIndex()];
	if( saveName[0] )
	{
		char	cmd[128];

		sprintf( cmd, "save/%s.bmp", saveName );
		EngFuncs::PIC_Free( cmd );

		sprintf( cmd, "save \"%s\"\n", saveName );
		EngFuncs::ClientCmd( FALSE, cmd );

		UI_CloseMenu();
	}
}

void CMenuLoadGame::UpdateGame()
{
	// first item is for creating new saves
	if( IsSaveMode() && savesList.GetCurrentIndex() == 0 )
	{
		remove.SetGrayed( true );
		levelShot.szName = NULL;
	}
	else
	{
		remove.SetGrayed( false );
		levelShot.szName = savesListModel.saveName[savesList.GetCurrentIndex()];
	}
}

void CMenuLoadGame::DeleteGame()
{
	const char *delName = savesListModel.delName[savesList.GetCurrentIndex()];

	if( delName[0] )
	{
		char	cmd[128];
		sprintf( cmd, "killsave \"%s\"\n", delName );

		EngFuncs::ClientCmd( TRUE, cmd );

		sprintf( cmd, "save/%s.bmp", delName );
		EngFuncs::PIC_Free( cmd );

		savesListModel.Update();
	}
}

void CMenuLoadGame::SetSaveMode(bool saveMode)
{
	m_fSaveMode = saveMode;
	if( saveMode )
	{
		banner.SetPicture( ART_BANNER_SAVE );
		save.SetVisibility( true );
		load.SetVisibility( false );
		szName = "CMenuSaveGame";
	}
	else
	{
		banner.SetPicture( ART_BANNER_LOAD );
		save.SetVisibility( false );
		load.SetVisibility( true );
		szName = "CMenuLoadGame";
	}
}

static CMenuLoadGame *menu_loadgame = NULL;

/*
=================
UI_LoadGame_Precache
=================
*/
void UI_LoadSaveGame_Precache( void )
{
	menu_loadgame = new CMenuLoadGame();
	EngFuncs::PIC_Load( ART_BANNER_SAVE );
	EngFuncs::PIC_Load( ART_BANNER_LOAD );
}

void UI_LoadSaveGame_Menu( bool saveMode )
{
	if( gMenu.m_gameinfo.gamemode == GAME_MULTIPLAYER_ONLY )
	{
		// completely ignore save\load menus for multiplayer_only
		return;
	}

	if( !EngFuncs::CheckGameDll( )) return;

	menu_loadgame->Show();
	menu_loadgame->SetSaveMode( saveMode );
	menu_loadgame->UpdateList();
}

void UI_LoadSaveGame_Shutdown( void )
{
	delete menu_loadgame;
}

/*
=================
UI_LoadGame_Menu
=================
*/
void UI_LoadGame_Menu( void )
{
	UI_LoadSaveGame_Menu( false );
}

void UI_SaveGame_Menu( void )
{
	UI_LoadSaveGame_Menu( true );
}
ADD_MENU4( menu_loadgame, UI_LoadSaveGame_Precache, UI_LoadGame_Menu, UI_LoadSaveGame_Shutdown );
ADD_MENU4( menu_savegame, NULL, UI_SaveGame_Menu, NULL );
