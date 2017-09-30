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
#include "YesNoMessageBox.h"
#include "Table.h"
#include "keydefs.h"
#include "Switch.h"

#define ART_BANNER_INET		"gfx/shell/head_inetgames"
#define ART_BANNER_LAN		"gfx/shell/head_lan"

// Server browser
#define QMSB_GAME_LENGTH	25
#define QMSB_MAPNAME_LENGTH	20+QMSB_GAME_LENGTH
#define QMSB_MAXCL_LENGTH	10+QMSB_MAPNAME_LENGTH
#define QMSB_PING_LENGTH    10+QMSB_MAXCL_LENGTH

class CMenuGameListModel : public CMenuBaseModel
{
public:
	void Update();
	int GetColumns() const
	{
		return 4; // game, mapname, maxcl, ping
	}
	int GetRows() const
	{
		return m_iNumItems;
	}
	const char *GetCellText( int line, int column )
	{
		return m_szCells[line][column];
	}
	void OnActivateEntry(int line);

private:
	char	m_szCells[UI_MAX_SERVERS][4][64];
	int m_iNumItems;
};

class CMenuServerBrowser: public CMenuFramework
{
public:
	CMenuServerBrowser() : CMenuFramework( "CMenuServerBrowser" ) { }
	virtual void Draw();

	void SetLANOnly( bool lanOnly )
	{
		m_bLanOnly = lanOnly;
	}
	void GetGamesList( void );
	void ClearList( void );
	void RefreshList( void );

	DECLARE_EVENT_TO_MENU_METHOD( CMenuServerBrowser, RefreshList )

	static void JoinGame( CMenuBaseItem *pSelf, void *pExtra );
	static void Connect( netadr_t adr );

	CMenuPicButton joinGame;
	CMenuPicButton createGame;
	CMenuPicButton gameInfo;
	CMenuPicButton refresh;
	CMenuPicButton done;
	CMenuSwitch natOrDirect;

	CMenuYesNoMessageBox msgBox;
	CMenuTable	gameList;
	CMenuGameListModel gameListModel;

	int	  refreshTime;
	int   refreshTime2;

	bool m_bLanOnly;
private:
	virtual void _Init();
	virtual void _VidInit();
};

static CMenuServerBrowser	uiServerBrowser;

/*
=================
CMenuServerBrowser::GetGamesList
=================
*/
void CMenuGameListModel::Update( void )
{
	int		i;
	const char	*info;

	for( i = 0; i < uiStatic.numServers; i++ )
	{
		if( i >= UI_MAX_SERVERS )
			break;
		info = uiStatic.serverNames[i];

		Q_strncpy( m_szCells[i][0], Info_ValueForKey( info, "host" ), 64 );
		Q_strncpy( m_szCells[i][1], Info_ValueForKey( info, "map" ), 64 );
		snprintf( m_szCells[i][2], 64, "%s\\%s", Info_ValueForKey( info, "numcl" ), Info_ValueForKey( info, "maxcl" ) );
		snprintf( m_szCells[i][3], 64, "%.f ms", uiStatic.serverPings[i] * 1000 );
	}

	m_iNumItems = i;

	if( uiStatic.numServers )
		uiServerBrowser.joinGame.SetGrayed( false );
}

void CMenuGameListModel::OnActivateEntry( int line )
{
	CMenuServerBrowser::Connect( uiStatic.serverAddresses[line] );
}

void CMenuServerBrowser::Connect( netadr_t adr )
{
	// prevent refresh during connect
	uiServerBrowser.refreshTime = uiStatic.realTime + 999999;
	//BUGBUG: ClientJoin not guaranted to return, need use ClientCmd instead!!!
	//BUGBUG: But server addres is known only as netadr_t here!!!
	EngFuncs::ClientJoin( adr );
	EngFuncs::ClientCmd( false, "menu_connectionprogress menu server\n" );
}

/*
=================
CMenuServerBrowser::JoinGame
=================
*/
void CMenuServerBrowser::JoinGame( CMenuBaseItem *pSelf, void *pExtra )
{
	CMenuServerBrowser *parent = (CMenuServerBrowser *)pSelf->Parent();

	Connect( uiStatic.serverAddresses[parent->gameList.GetCurrentIndex()] );
}

void CMenuServerBrowser::ClearList()
{
	uiStatic.numServers = 0;
	gameListModel.Update();
}

void CMenuServerBrowser::RefreshList()
{
	if( m_bLanOnly )
	{
		UI_RefreshServerList();
	}
	else
	{
		if( uiStatic.realTime > refreshTime2 )
		{
			UI_RefreshInternetServerList();
			refreshTime2 = uiStatic.realTime + (EngFuncs::GetCvarFloat("cl_nat") ? 4000:1000);
			refresh.iFlags |= QMF_GRAYED;
			if( uiStatic.realTime + 20000 < refreshTime )
				refreshTime = uiStatic.realTime + 20000;
		}
	}
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
		RefreshList();
		refreshTime = uiStatic.realTime + 20000; // refresh every 10 secs
	}

	if( uiStatic.realTime > refreshTime2 )
	{
		refresh.iFlags &= ~QMF_GRAYED;
	}

	// serverinfo has been changed update display
	if( uiStatic.updateServers )
	{
		gameListModel.Update();
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
	joinGame.iFlags |= QMF_GRAYED;
	joinGame.SetNameAndStatus( "Join game", "Join to selected game" );
	joinGame.SetPicture( PC_JOIN_GAME );
	joinGame.onActivatedClActive = msgBox.MakeOpenEvent();
	joinGame.onActivated = JoinGame;

	createGame.SetNameAndStatus( "Create game", "Create new Internet game" );
	createGame.SetPicture( PC_CREATE_GAME );
	SET_EVENT( createGame, onActivated )
	{
		if( ((CMenuServerBrowser*)pSelf->Parent())->m_bLanOnly )
			EngFuncs::CvarSetValue( "public", 0.0f );
		else EngFuncs::CvarSetValue( "public", 1.0f );

		UI_CreateGame_Menu();
	}
	END_EVENT( createGame, onActivated )

	gameInfo.iFlags |= QMF_GRAYED;
	gameInfo.SetNameAndStatus( "View game info", "Get detail game info" );
	gameInfo.SetPicture( PC_VIEW_GAME_INFO );
	// TODO: implement!

	refresh.SetNameAndStatus( "Refresh", "Refresh servers list" );
	refresh.SetPicture( PC_REFRESH );
	refresh.onActivated = RefreshListCb;

	done.SetNameAndStatus( "Done", "Return to main menu" );
	done.onActivated = HideCb;
	done.SetPicture( PC_DONE );

	msgBox.SetMessage( "Join a network game will exit\nany current game, OK to exit?" );
	msgBox.SetPositiveButton( "Ok", PC_OK );
	msgBox.HighlightChoice( 1 );
	msgBox.onPositive = JoinGame;
	msgBox.Link( this );

	gameList.SetCharSize( QM_SMALLFONT );
	gameList.SetupColumn( 0, "Name", 0.50f );
	gameList.SetupColumn( 1, "Map", 0.28f );
	gameList.SetupColumn( 2, "Players", 0.12f );
	gameList.SetupColumn( 3, "Ping", 0.10f );
	gameList.SetModel( &gameListModel );
	gameList.bFramedHintText = true;

	natOrDirect.szLeftName = "Direct";
	natOrDirect.szRightName = "NAT";
	natOrDirect.eTextAlignment = QM_CENTER;
	natOrDirect.bMouseToggle = false;
	natOrDirect.LinkCvar( "cl_nat" );
	natOrDirect.iSelectColor = uiInputFgColor;
	// bit darker
	natOrDirect.iFgTextColor = uiInputFgColor - 0x00151515;
	SET_EVENT( natOrDirect, onChanged )
	{
		CMenuSwitch *self = (CMenuSwitch*)pSelf;
		CMenuServerBrowser *parent = (CMenuServerBrowser*)self->Parent();

		self->WriteCvar();
		parent->ClearList();
		parent->RefreshList();
	}
	END_EVENT( natOrDirect, onChanged )

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
	AddItem( natOrDirect );
}

/*
=================
CMenuServerBrowser::VidInit
=================
*/
void CMenuServerBrowser::_VidInit()
{
	if( m_bLanOnly )
	{
		banner.SetPicture( ART_BANNER_LAN );
		createGame.szStatusText = ( "Create new LAN game" );
		natOrDirect.Hide();
	}
	else
	{
		banner.SetPicture( ART_BANNER_INET );
		createGame.szStatusText = ( "Create new Internet game" );
		natOrDirect.Show();
	}

	joinGame.SetCoord( 72, 230 );
	createGame.SetCoord( 72, 280 );
	gameInfo.SetCoord( 72, 330 );
	refresh.SetCoord( 72, 380 );
	done.SetCoord( 72, 430 );

	gameList.SetRect( 340, 255, 660, 440 );
	natOrDirect.SetCoord( 780, 255 - gameList.charSize.h * 1.5 - UI_OUTLINE_WIDTH * 2 - natOrDirect.size.h );

	refreshTime = uiStatic.realTime + 500; // delay before update 0.5 sec
	refreshTime2 = uiStatic.realTime + 500;
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
	uiServerBrowser.Show();
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
