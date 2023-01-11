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
#include "Field.h"
#include "utlvector.h"
#include "CheckBox.h"

#define ART_BANNER_INET		"gfx/shell/head_inetgames"
#define ART_BANNER_LAN		"gfx/shell/head_lan"
#define ART_BANNER_LOCK		"gfx/shell/lock"

class CMenuServerBrowser;

enum
{
	COLUMN_PASSWORD = 0,
	COLUMN_NAME,
	COLUMN_MAP,
	COLUMN_PLAYERS,
	COLUMN_PING,
	COLUMN_IP
};

struct server_t
{
	netadr_t adr;
	char  info[256];
	float ping;
	int  numcl;
	int  maxcl;
	char name[64];
	char mapname[64];
	char clientsstr[64];
	char pingstr[64];
	char ipstr[64];
	bool havePassword;
	bool isLegacy;

	server_t( netadr_t adr, const char *info );
	void UpdateData();
	void SetPing( float ping );

	int Rank( const server_t &other ) const
	{
		if( isLegacy > other.isLegacy ) return 100;
		else if( isLegacy < other.isLegacy ) return -100;
		return 0;
	}

	int NameCmp( const server_t &other ) const
	{
		return colorstricmp( name, other.name );
	}

	int AdrCmp( const server_t &other ) const
	{
		return EngFuncs::NET_CompareAdr( &adr, &other.adr );
	}

	int MapCmp( const server_t &other ) const
	{
		return stricmp( mapname, other.mapname );
	}

	int ClientCmp( const server_t &other ) const
	{
		if( numcl > other.numcl ) return 1;
		else if( numcl < other.numcl ) return -1;
		return 0;
	}

	int PingCmp( const server_t &other ) const
	{
		if( ping > other.ping ) return 1;
		else if( ping < other.ping ) return -1;
		return 0;
	}

	// make generic
	// always rank new servers higher, even when sorting in reverse order
#define GENERATE_COMPAR_FN( method ) \
	static int method ## Ascend( const void *a, const void *b ) \
	{\
		return (( const server_t *)a)->Rank( *(( const server_t *)b) ) + (( const server_t *)a)->method( *(( const server_t *)b) );\
	}\
	static int method ## Descend( const void *a, const void *b ) \
	{\
		return (( const server_t *)a)->Rank( *(( const server_t *)b) ) + (( const server_t *)b)->method( *(( const server_t *)a) );\
	}\

	GENERATE_COMPAR_FN( NameCmp )
	GENERATE_COMPAR_FN( AdrCmp )
	GENERATE_COMPAR_FN( MapCmp )
	GENERATE_COMPAR_FN( ClientCmp )
	GENERATE_COMPAR_FN( PingCmp )
#undef GENERATE_COMPAR_FN
};

class CMenuGameListModel : public CMenuBaseModel
{
public:
	CMenuGameListModel( CMenuServerBrowser *parent ) : CMenuBaseModel(), parent( parent ), m_iSortingColumn(-1) {}

	void Update() override;

	int GetColumns() const override
	{
		return 6; // havePassword, game, mapname, maxcl, ping, (hidden)ip
	}

	int GetRows() const override
	{
		return servers.Count();
	}

	ECellType GetCellType( int line, int column ) override
	{
		if( column == 0 )
			return CELL_IMAGE_ADDITIVE;
		return CELL_TEXT;
	}

	const char *GetCellText( int line, int column ) override
	{
		switch( column )
		{
		case COLUMN_PASSWORD: return servers[line].havePassword ? ART_BANNER_LOCK : NULL;
		case COLUMN_NAME: return servers[line].name;
		case COLUMN_MAP: return servers[line].mapname;
		case COLUMN_PLAYERS: return servers[line].clientsstr;
		case COLUMN_PING: return servers[line].pingstr;
		case COLUMN_IP: return servers[line].ipstr;
		default: return NULL;
		}
	}

	bool GetCellColors( int line, int column, unsigned int &textColor, bool &force) const override
	{
		if( servers[line].isLegacy )
		{
			CColor color = uiPromptTextColor;
			color.a = color.a * 0.5;
			textColor = color;

			// allow colorstrings only in server name
			force = column != COLUMN_NAME;

			return true;
		}
		return false;
	}

	void OnActivateEntry( int line ) override;

	void Flush()
	{
		servers.RemoveAll();
		serversRefreshTime = gpGlobals->time;
	}

	bool IsHavePassword( int line )
	{
		return servers[line].havePassword;
	}

	void AddServerToList( netadr_t adr, const char *info );

	bool Sort( int column, bool ascend ) override;

	float serversRefreshTime;
	CUtlVector<server_t> servers;
private:
	CMenuServerBrowser *parent;

	int m_iSortingColumn;
	bool m_bAscend;
};

class CMenuServerBrowser: public CMenuFramework
{
public:
	CMenuServerBrowser() : CMenuFramework( "CMenuServerBrowser" ), gameListModel( this ) { }
	void Draw() override;
	void Show() override;
	bool KeyUp( int key ) override;

	void SetLANOnly( bool lanOnly )
	{
		m_bLanOnly = lanOnly;
	}
	void GetGamesList( void );
	void ClearList( void );
	void RefreshList( void );
	void JoinGame( void );
	void ResetPing( void )
	{
		gameListModel.serversRefreshTime = EngFuncs::DoubleTime();
	}

	void AddServerToList( netadr_t adr, const char *info );

	static void Connect( server_t &server );

	CMenuPicButton *joinGame;
	CMenuPicButton *createGame;
	CMenuPicButton *refresh;
	CMenuSwitch natOrDirect;

	CMenuYesNoMessageBox msgBox;
	CMenuTable	gameList;
	CMenuGameListModel gameListModel;
	CMenuCheckBox showip;

	CMenuYesNoMessageBox askPassword;
	CMenuField password;

	int	  refreshTime;
	int   refreshTime2;

	bool m_bLanOnly;
private:
	void _Init() override;
	void _VidInit() override;
};

static server_t staticServerSelect( netadr_t(), "" );
static bool staticWaitingPassword = false;

ADD_MENU3( menu_internetgames, CMenuServerBrowser, UI_InternetGames_Menu );
ADD_MENU4( menu_langame, NULL, UI_LanGame_Menu, NULL );

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
		uiStatic.m_iOldMenuDepth = uiStatic.menu.Count();
		EngFuncs::ClientCmd( FALSE, "stop\n" );
		uiStatic.m_fDemosPlayed = true;
	}

	menu_internetgames->Show();
}

void UI_InternetGames_Menu( void )
{
	menu_internetgames->SetLANOnly( false );

	UI_ServerBrowser_Menu();
}

void UI_LanGame_Menu( void )
{
	menu_internetgames->SetLANOnly( true );

	UI_ServerBrowser_Menu();
}

server_t::server_t( netadr_t adr, const char *info ) :
	adr( adr )
{
	Q_strncpy( this->info, info, sizeof( this->info ));
}

void server_t::UpdateData( void )
{
	Q_strncpy( name, Info_ValueForKey( info, "host" ), sizeof( name ));
	Q_strncpy( mapname, Info_ValueForKey( info, "map" ), sizeof( mapname ));
	Q_strncpy( ipstr, EngFuncs::NET_AdrToString( adr ), sizeof( ipstr ));
	numcl = atoi( Info_ValueForKey( info, "numcl" ));
	maxcl = atoi( Info_ValueForKey( info, "maxcl" ));
	snprintf( clientsstr, sizeof( clientsstr ), "%d\\%d", numcl, maxcl );
	havePassword = !strcmp( Info_ValueForKey( info, "password" ), "1" );
	isLegacy = !strcmp( Info_ValueForKey( info, "legacy" ), "1" );
}

void server_t::SetPing( float ping )
{
	ping = bound( 0.0f, ping, 9.999f );

	if( isLegacy )
		ping /= 2;

	this->ping = ping;
	snprintf( pingstr, sizeof( pingstr ), "%.f ms", ping * 1000 );
}

bool CMenuGameListModel::Sort(int column, bool ascend)
{
	m_iSortingColumn = column;
	if( column == -1 )
		return false; // disabled

	m_bAscend = ascend;
	switch( column )
	{
	case COLUMN_PASSWORD:
		return false;
	case COLUMN_NAME:
		qsort( servers.Base(), servers.Count(), sizeof( server_t ),
			ascend ? server_t::NameCmpAscend : server_t::NameCmpDescend );
		return true;
	case COLUMN_MAP:
		qsort( servers.Base(), servers.Count(), sizeof( server_t ),
			ascend ? server_t::MapCmpAscend : server_t::MapCmpDescend );
		return true;
	case COLUMN_PLAYERS:
		qsort( servers.Base(), servers.Count(), sizeof( server_t ),
			ascend ? server_t::ClientCmpAscend : server_t::ClientCmpDescend );
		return true;
	case COLUMN_PING:
		qsort( servers.Base(), servers.Count(), sizeof( server_t ),
			ascend ? server_t::PingCmpAscend : server_t::PingCmpDescend );
		return true;
	case COLUMN_IP:
		qsort( servers.Base(), servers.Count(), sizeof( server_t ),
			ascend ? server_t::AdrCmpAscend : server_t::AdrCmpDescend );
		return true;
	}

	return false;
}

/*
=================
CMenuServerBrowser::GetGamesList
=================
*/
void CMenuGameListModel::Update( void )
{
	int		i;
	const char	*info;

	// regenerate table data
	for( i = 0; i < servers.Count(); i++ )
		servers[i].UpdateData();

	if( servers.Count() )
	{
		parent->joinGame->SetGrayed( false );
		if( m_iSortingColumn != -1 )
			Sort( m_iSortingColumn, m_bAscend );
	}
}

void CMenuGameListModel::OnActivateEntry( int line )
{
	CMenuServerBrowser::Connect( servers[line] );
}

void CMenuGameListModel::AddServerToList( netadr_t adr, const char *info )
{
	int i;

	// ignore if duplicated
	for( i = 0; i < servers.Count(); i++ )
	{
		if( !EngFuncs::NET_CompareAdr( &servers[i].adr, &adr ))
			return;

		if( !stricmp( servers[i].info, info ))
			return;
	}

	server_t server( adr, info );

	server.UpdateData();
	server.SetPing( EngFuncs::DoubleTime() - serversRefreshTime );

	servers.AddToTail( server );

	if( m_iSortingColumn != -1 )
		Sort( m_iSortingColumn, m_bAscend );
}

void CMenuServerBrowser::Connect( server_t &server )
{
	char buf[256];

	// prevent refresh during connect
	menu_internetgames->refreshTime = uiStatic.realTime + 999999;

	// ask user for password
	if( server.havePassword )
	{
		// if dialog window is still open, then user have entered the password
		if( !staticWaitingPassword )
		{
			// save current select
			staticServerSelect = server;
			staticWaitingPassword = true;

			// show password request window
			menu_internetgames->askPassword.Show();

			return;
		}
	}
	else
	{
		// remove password, as server don't require it
		EngFuncs::CvarSetString( "password", "" );
	}

	staticWaitingPassword = false;

	snprintf( buf, sizeof( buf ), "connect %s %s",
		EngFuncs::NET_AdrToString( server.adr ),
		server.isLegacy ? "legacy" : "" );
	buf[sizeof( buf ) - 1] = 0;

	EngFuncs::ClientCmd( FALSE, buf );
	UI_ConnectionProgress_Connect( "" );
}

/*
=================
CMenuServerBrowser::JoinGame
=================
*/
void CMenuServerBrowser::JoinGame()
{
	gameListModel.OnActivateEntry( gameList.GetCurrentIndex() );
}

void CMenuServerBrowser::ClearList()
{
	gameListModel.Flush();
	joinGame->SetGrayed( true );
}

void CMenuServerBrowser::RefreshList()
{
	ClearList();

	if( m_bLanOnly )
	{
		EngFuncs::ClientCmd( FALSE, "localservers\n" );
	}
	else
	{
		if( uiStatic.realTime > refreshTime2 )
		{
			EngFuncs::ClientCmd( FALSE, "internetservers\n" );
			refreshTime2 = uiStatic.realTime + (EngFuncs::GetCvarFloat("cl_nat") ? 4000:1000);
			refresh->SetGrayed( true );
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
		refresh->SetGrayed( false );
	}
}

bool CMenuServerBrowser::KeyUp( int key )
{
	if( key == 'i' )
	{
		gameList.SetColumnWidth( COLUMN_IP, 300, true );

		gameList.VidInit();
		gameListModel.Update();
	}

	return CMenuFramework::KeyUp( key );
}

/*
=================
CMenuServerBrowser::Init
=================
*/
void CMenuServerBrowser::_Init( void )
{
	AddItem( background );
	AddItem( banner );

	joinGame = AddButton( L( "Join game" ), L( "Join to selected game" ), PC_JOIN_GAME,
		VoidCb( &CMenuServerBrowser::JoinGame ), QMF_GRAYED );
	joinGame->onReleasedClActive = msgBox.MakeOpenEvent();

	createGame = AddButton( L( "GameUI_GameMenu_CreateServer" ), NULL, PC_CREATE_GAME );
	SET_EVENT_MULTI( createGame->onReleased,
	{
		if( ((CMenuServerBrowser*)pSelf->Parent())->m_bLanOnly )
			EngFuncs::CvarSetValue( "public", 0.0f );
		else EngFuncs::CvarSetValue( "public", 1.0f );

		UI_CreateGame_Menu();
	});

	// TODO: implement!
	AddButton( L( "View game info" ), L( "Get detail game info" ), PC_VIEW_GAME_INFO, CEventCallback::NoopCb, QMF_GRAYED );

	refresh = AddButton( L( "Refresh" ), L( "Refresh servers list" ), PC_REFRESH, VoidCb( &CMenuServerBrowser::RefreshList ) );

	AddButton( L( "Done" ), L( "Return to main menu" ), PC_DONE, VoidCb( &CMenuServerBrowser::Hide ) );

	msgBox.SetMessage( L( "Join a network game will exit any current game, OK to exit?" ) );
	msgBox.SetPositiveButton( L( "GameUI_OK" ), PC_OK );
	msgBox.HighlightChoice( CMenuYesNoMessageBox::HIGHLIGHT_YES );
	msgBox.onPositive = VoidCb( &CMenuServerBrowser::JoinGame );
	msgBox.Link( this );

	gameList.SetCharSize( QM_SMALLFONT );
	gameList.SetupColumn( COLUMN_PASSWORD, NULL, 32.0f, true );
	gameList.SetupColumn( COLUMN_NAME, L( "Name" ), 0.40f );
	gameList.SetupColumn( COLUMN_MAP, L( "GameUI_Map" ), 0.25f );
	gameList.SetupColumn( COLUMN_PLAYERS, L( "Players" ), 100.0f, true );
	gameList.SetupColumn( COLUMN_PING, L( "Ping" ), 120.0f, true );
	gameList.SetupColumn( COLUMN_IP, L( "IP" ), 0, true );
	gameList.SetModel( &gameListModel );
	gameList.bFramedHintText = true;
	gameList.bAllowSorting = true;

	natOrDirect.AddSwitch( L( "Direct" ) );
	natOrDirect.AddSwitch( "NAT" );
	natOrDirect.eTextAlignment = QM_CENTER;
	natOrDirect.bMouseToggle = false;
	natOrDirect.LinkCvar( "cl_nat" );
	natOrDirect.iSelectColor = uiInputFgColor;
	// bit darker
	natOrDirect.iFgTextColor = uiInputFgColor - 0x00151515;
	SET_EVENT_MULTI( natOrDirect.onChanged,
	{
		CMenuSwitch *self = (CMenuSwitch*)pSelf;
		CMenuServerBrowser *parent = (CMenuServerBrowser*)self->Parent();

		self->WriteCvar();
		parent->ClearList();
		parent->RefreshList();
	});

	// server.dll needs for reading savefiles or startup newgame
	if( !EngFuncs::CheckGameDll( ))
		createGame->SetGrayed( true );	// server.dll is missed - remote servers only

	password.bHideInput = true;
	password.bAllowColorstrings = false;
	password.bNumbersOnly = false;
	password.szName = L( "GameUI_Password" );
	password.iMaxLength = 16;
	password.SetRect( 188, 140, 270, 32 );

	SET_EVENT_MULTI( askPassword.onPositive,
	{
		CMenuServerBrowser *parent = (CMenuServerBrowser*)pSelf->Parent();

		EngFuncs::CvarSetString( "password", parent->password.GetBuffer() );
		parent->password.Clear(); // we don't need entered password anymore
		CMenuServerBrowser::Connect( staticServerSelect );
	});

	SET_EVENT_MULTI( askPassword.onNegative,
	{
		CMenuServerBrowser *parent = (CMenuServerBrowser*)pSelf->Parent();

		EngFuncs::CvarSetString( "password", "" );
		parent->password.Clear(); // we don't need entered password anymore
		staticWaitingPassword = false;
	});

	askPassword.SetMessage( L( "GameUI_PasswordPrompt" ) );
	askPassword.Link( this );
	askPassword.Init();
	askPassword.AddItem( password );

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
		createGame->szStatusText = ( L( "Create new LAN game" ) );
		natOrDirect.Hide();
	}
	else
	{
		banner.SetPicture( ART_BANNER_INET );
		createGame->szStatusText = ( L( "Create new Internet game" ) );
		natOrDirect.Show();
	}

	gameList.SetRect( 360, 230, -20, 465 );
	natOrDirect.SetCoord( -20 - natOrDirect.size.w, gameList.pos.y - UI_OUTLINE_WIDTH - natOrDirect.size.h );

	refreshTime = uiStatic.realTime + 500; // delay before update 0.5 sec
	refreshTime2 = uiStatic.realTime + 500;
}

void CMenuServerBrowser::Show()
{
	CMenuFramework::Show();

	// clear out server table
	staticWaitingPassword = false;
	gameListModel.Flush();
	gameList.SetSortingColumn( COLUMN_PING );
	joinGame->SetGrayed( true );
}

void CMenuServerBrowser::AddServerToList( netadr_t adr, const char *info )
{
	if( stricmp( gMenu.m_gameinfo.gamefolder, Info_ValueForKey( info, "gamedir" )) != 0 )
		return;

	if( !WasInit() )
		return;

	if( !IsVisible() )
		return;

	gameListModel.AddServerToList( adr, info );

	joinGame->SetGrayed( false );
}

/*
=================
UI_AddServerToList
=================
*/
void UI_AddServerToList( netadr_t adr, const char *info )
{
	if( !menu_internetgames )
		return;

	menu_internetgames->AddServerToList( adr, info );
}

/*
=================
UI_MenuResetPing_f
=================
*/
void UI_MenuResetPing_f( void )
{
	if( menu_internetgames )
		menu_internetgames->ResetPing();
}
ADD_COMMAND( menu_resetping, UI_MenuResetPing_f );
