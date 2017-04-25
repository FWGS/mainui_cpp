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
#include "Bitmap.h"
#include "YesNoMessageBox.h"
#include "ScrollList.h"
#include "Utils.h"
#include "keydefs.h"
#include "BtnsBMPTable.h"

#define ART_BANNER_INET		"gfx/shell/head_inetgames"
#define ART_BANNER_LAN		"gfx/shell/head_lan"

// Server browser
#define QMSB_GAME_LENGTH	25
#define QMSB_MAPNAME_LENGTH	20+QMSB_GAME_LENGTH
#define QMSB_MAXCL_LENGTH	10+QMSB_MAPNAME_LENGTH
#define QMSB_PING_LENGTH    10+QMSB_MAXCL_LENGTH

class CMenuServerBrowser: public CMenuFramework
{
public:
	virtual const char *Key( int key, int down );
	virtual void Draw();

	void SetLANOnly( bool lanOnly ) { m_bLanOnly = lanOnly; }

private:
	virtual void _Init();
	virtual void _VidInit();

	static void PromptDialog( CMenuBaseItem *pSelf, void *pExtra );
	void GetGamesList( void );
	static void JoinGame( CMenuBaseItem *pSelf, void *pExtra );

	char		gameDescription[UI_MAX_SERVERS][256];
	char		*gameDescriptionPtr[UI_MAX_SERVERS];

	CMenuBackgroundBitmap background;
	CMenuBannerBitmap banner;

	CMenuPicButton joinGame;
	CMenuPicButton createGame;
	CMenuPicButton gameInfo;
	CMenuPicButton refresh;
	CMenuPicButton done;

	CMenuYesNoMessageBox msgBox;
	CMenuScrollList	gameList;

	char		hintText[MAX_HINT_TEXT];
	int		refreshTime;

	bool m_bLanOnly;
};

static CMenuServerBrowser	uiServerBrowser;

void CMenuServerBrowser::PromptDialog( CMenuBaseItem *pSelf, void *pExtra )
{
	CMenuServerBrowser *parent = (CMenuServerBrowser *)pSelf->Parent();

	// toggle main menu between active\inactive
	// show\hide quit dialog
	parent->ToggleInactive();

	parent->msgBox.ToggleVisibility();
}

/*
=================
CMenuServerBrowser::KeyFunc
=================
*/
const char *CMenuServerBrowser::Key( int key, int down )
{
	if( down && key == K_ESCAPE && !( msgBox.iFlags & QMF_HIDDEN ))
	{
		PromptDialog( &msgBox, NULL );
		return uiSoundNull;
	}
	return CMenuFramework::Key( key, down );
}

/*
=================
CMenuServerBrowser::GetGamesList
=================
*/
void CMenuServerBrowser::GetGamesList( void )
{
	int		i;
	const char	*info;

	for( i = 0; i < uiStatic.numServers; i++ )
	{
		if( i >= UI_MAX_SERVERS )
			break;
		info = uiStatic.serverNames[i];

		gameDescription[i][0] = 0; // mark this string as empty

		StringConcat( gameDescription[i], Info_ValueForKey( info, "host" ), QMSB_GAME_LENGTH );
		AddSpaces( gameDescription[i], QMSB_GAME_LENGTH );

		StringConcat( gameDescription[i], Info_ValueForKey( info, "map" ), QMSB_MAPNAME_LENGTH );
		AddSpaces( gameDescription[i], QMSB_MAPNAME_LENGTH );

		StringConcat( gameDescription[i], Info_ValueForKey( info, "numcl" ), QMSB_MAXCL_LENGTH );
		StringConcat( gameDescription[i], "\\", QMSB_MAXCL_LENGTH );
		StringConcat( gameDescription[i], Info_ValueForKey( info, "maxcl" ), QMSB_MAXCL_LENGTH );
		AddSpaces( gameDescription[i], QMSB_MAXCL_LENGTH );

		char ping[10];
		snprintf( ping, 10, "%.f ms", uiStatic.serverPings[i] * 1000 );
		StringConcat( gameDescription[i], ping, QMSB_PING_LENGTH );
		AddSpaces( gameDescription[i], QMSB_PING_LENGTH );

		gameDescriptionPtr[i] = gameDescription[i];
	}

	for( ; i < UI_MAX_SERVERS; i++ )
		gameDescriptionPtr[i] = NULL;

	gameList.pszItemNames = (const char **)gameDescriptionPtr;
	gameList.iNumItems = 0; // reset it
	gameList.iCurItem = 0; // reset it

	if( !gameList.charSize.h )
		return; // to avoid divide integer by zero

	// count number of items
	while( gameList.pszItemNames[gameList.iNumItems] )
		gameList.iNumItems++;

	// calculate number of visible rows
	int h = gameList.size.h;
	UI_ScaleCoords( NULL, NULL, NULL, &h );

	gameList.iNumRows = (h / gameList.charSize.h) - 2;
	if( gameList.iNumRows > gameList.iNumItems )
		gameList.iNumRows = gameList.iNumItems;

	if( uiStatic.numServers )
		joinGame.iFlags &= ~QMF_GRAYED;
}

/*
=================
CMenuServerBrowser::JoinGame
=================
*/
void CMenuServerBrowser::JoinGame( CMenuBaseItem *pSelf, void *pExtra )
{
	CMenuServerBrowser *parent = (CMenuServerBrowser *)pSelf->Parent();

	// close dialog
	if( !(parent->msgBox.iFlags & QMF_HIDDEN ) )
		PromptDialog( pSelf, pExtra );

	if( parent->gameDescription[parent->gameList.iCurItem][0] == 0 )
		return;

	EngFuncs::ClientJoin( uiStatic.serverAddresses[parent->gameList.iCurItem] );
	// prevent refresh durning connect
	parent->refreshTime = uiStatic.realTime + 999999999;
}

/*
=================
UI_Background_Ownerdraw
=================
*/
void CMenuServerBrowser::Draw( void )
{
	CMenuFramework::Draw();

	if( uiStatic.realTime > refreshTime )
	{
		refreshTime = uiStatic.realTime + 20000; // refresh every 10 secs
		if( m_bLanOnly ) UI_RefreshServerList();
		else UI_RefreshInternetServerList();
	}

	// serverinfo has been changed update display
	if( uiStatic.updateServers )
	{
		GetGamesList ();
		uiStatic.updateServers = false;
	}
}

/*
=================
CMenuServerBrowser::Init
=================
*/
void CMenuServerBrowser::_Init( void )
{
	// memset( &uiServerBrowser, 0, sizeof( CMenuServerBrowser ));

	StringConcat( hintText, "Name", QMSB_GAME_LENGTH );
	AddSpaces( hintText, QMSB_GAME_LENGTH );
	StringConcat( hintText, "Map", QMSB_MAPNAME_LENGTH );
	AddSpaces( hintText, QMSB_MAPNAME_LENGTH );
	StringConcat( hintText, "Players", QMSB_MAXCL_LENGTH );
	AddSpaces( hintText, QMSB_MAXCL_LENGTH );
	StringConcat( hintText, "Ping", QMSB_PING_LENGTH );
	AddSpaces( hintText, QMSB_PING_LENGTH );


	joinGame.iFlags |= QMF_GRAYED;
	joinGame.SetNameAndStatus( "Join game", "Join to selected game" );
	joinGame.SetPicture( PC_JOIN_GAME );
	SET_EVENT( joinGame, onActivated )
	{
		if( CL_IsActive() ) PromptDialog( pSelf, pExtra );
		else JoinGame( pSelf, pExtra );
	}
	END_EVENT( joinGame, onActivated )

	createGame.SetNameAndStatus( "Create game", "Create new Internet game" );
	createGame.SetPicture( PC_CREATE_GAME );
	SET_EVENT( createGame, onActivated )
	{
		if( ((CMenuServerBrowser*)pSelf->Parent())->m_bLanOnly )
			EngFuncs::CvarSetValue( "public", 0.0f );
		else EngFuncs::CvarSetValue( "public", 1.0f );

		// UI_CreateGame_Menu();
	}
	END_EVENT( createGame, onActivated )

	gameInfo.iFlags |= QMF_GRAYED;
	gameInfo.SetNameAndStatus( "View game info", "Get detail game info" );
	gameInfo.SetPicture( PC_VIEW_GAME_INFO );
	// TODO: implement!

	refresh.SetNameAndStatus( "Refresh", "Refresh servers list" );
	refresh.SetPicture( PC_REFRESH );

	done.SetNameAndStatus( "Done", "Return to main menu" );
	done.onActivated = PopMenuCb;
	done.SetPicture( PC_DONE );

	msgBox.SetMessage( "Join a network game will exit\nany current game, OK to exit?" );
	msgBox.SetPositiveButton( "Ok", PC_OK );
	msgBox.SetNegativeButton( "Cancel", PC_CANCEL );
	msgBox.HighlightChoice( 1 );
	msgBox.onPositive = JoinGame;
	msgBox.onNegative = PromptDialog;

	gameList.SetCharSize( QM_SMALLFONT );
	gameList.eFocusAnimation = QM_HIGHLIGHTIFFOCUS;
	gameList.pszItemNames = (const char **)gameDescriptionPtr;
	gameList.szName = hintText;

	// server.dll needs for reading savefiles or startup newgame
	if( !EngFuncs::CheckGameDll( ))
		createGame.iFlags |= QMF_GRAYED;	// server.dll is missed - remote servers only


	AddItem( background );
	AddItem( banner );
	AddItem( joinGame );
	AddItem( createGame );
	AddItem( gameInfo );
	AddItem( refresh );
	AddItem( done );
	AddItem( gameList );
	AddItem( msgBox );

}

/*
=================
CMenuServerBrowser::VidInit
=================
*/
void CMenuServerBrowser::_VidInit()
{
	memset( gameDescription, 0, sizeof( gameDescription ));
	memset( gameDescriptionPtr, 0, sizeof( gameDescriptionPtr ));

	if( m_bLanOnly )
	{
		banner.SetPicture( ART_BANNER_LAN );
		createGame.szStatusText = ( "Create new LAN game" );
		refresh.onActivated = UI_RefreshServerList;
	}
	else
	{
		banner.SetPicture( ART_BANNER_INET );
		createGame.szStatusText = ( "Create new Internet game" );
		createGame.onActivated = UI_RefreshInternetServerList;
	}

	joinGame.SetCoord( 72, 230 );
	createGame.SetCoord( 72, 280 );
	gameInfo.SetCoord( 72, 330 );
	refresh.SetCoord( 72, 380 );
	done.SetCoord( 72, 430 );

	gameList.SetRect( 340, 255, 660, 240 );

	refreshTime = uiStatic.realTime + 500; // delay before update 0.5 sec
}

/*
=================
CMenuServerBrowser::Precache
=================
*/
void UI_InternetGames_Precache( void )
{
	EngFuncs::PIC_Load( ART_BANNER_INET );
	EngFuncs::PIC_Load( ART_BANNER_LAN );
}

/*
=================
CMenuServerBrowser::Menu
=================
*/
void UI_ServerBrowser_Menu( void )
{
	if ( gMenu.m_gameinfo.gamemode == GAME_SINGLEPLAYER_ONLY )
		return;

	// stop demos to allow network sockets to open
	if ( gpGlobals->demoplayback && EngFuncs::GetCvarFloat( "cl_background" ))
	{
		uiStatic.m_iOldMenuDepth = uiStatic.menuDepth;
		EngFuncs::ClientCmd( FALSE, "stop\n" );
		uiStatic.m_fDemosPlayed = true;
	}

	UI_InternetGames_Precache();
	uiServerBrowser.Open();
}

void UI_InternetGames_Menu( void )
{
	uiServerBrowser.SetLANOnly( false );

	UI_ServerBrowser_Menu();
}

void UI_LanGame_Menu( void )
{
	uiServerBrowser.SetLANOnly( true );

	UI_ServerBrowser_Menu();
}
