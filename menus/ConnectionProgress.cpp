#include "extdll.h"
#include "BaseMenu.h"
#include "ConnectionProgress.h"
#include "Utils.h"

#include "ItemsHolder.h"
#include "ProgressBar.h"
#include "PicButton.h"
#include "YesNoMessageBox.h"

enum EState
{
STATE_NONE,
STATE_MENU, // do not hide when disconnected or in game
STATE_DOWNLOAD, // enlarge your connectionprogress window
STATE_CONNECTING, // showing single progress
STATE_CONSOLE // do not show until state reset
};

enum ESource
{
SOURCE_CONSOLE,
SOURCE_MENU
};


class CMenuConnectionProgress : public CMenuItemsHolder
{
public:
	CMenuConnectionProgress();
	virtual void _Init();
	virtual void _VidInit();
	virtual void Draw();
	virtual const char *Key( int key, int down );
	static void DisconnectCb( CMenuBaseItem *pSelf , void *pExtra );
	void HandleDisconnect( void );
	void HandlePrecache( void )
	{
		SetCommonText( "Precaching resources" );
		commonProgress.LinkCvar( "scr_loading", 0, 100 );
		m_iState = STATE_CONNECTING;
	}
	void HandleStufftext( float flProgress, char *pszText )
	{
		commonProgress.SetValue( flProgress );
		SetCommonText( pszText );
		m_iState = STATE_CONNECTING;
	}
	void HandleDownload( const char *pszFileName, const char *pszServerName, int iCurrent, int iTotal, const char *comment )
	{
		snprintf( sDownloadString, sizeof( sDownloadString ) - 1, "Downloading %s \nfrom %s", pszFileName, pszServerName );
		snprintf( sCommonString, sizeof( sCommonString ) - 1, "%d of %d %s", iCurrent, iTotal, comment );
		m_iState = STATE_DOWNLOAD;
		commonProgress.SetValue( (float)iCurrent/iTotal );
	}
	void SetCommonText( const char *pszText )
	{
		snprintf( sCommonString, sizeof( sCommonString ) - 1, "%s", pszText );
	}
	enum EState m_iState;
	enum ESource m_iSource;
	void SetServer( const char *pszName )
	{
		snprintf( sTitleString, sizeof( sTitleString ) - 1, "Connecting to %s...", pszName );
	}
private:
	CMenuProgressBar commonProgress;
	CMenuProgressBar downloadProgress;
	CMenuPicButton consoleButton;
	CMenuPicButton disconnectButton;
	CMenuPicButton skipButton;
	CMenuYesNoMessageBox dialog;
	CMenuAction title;
	CMenuAction downloadText;
	CMenuAction commonText;
	char sTitleString[256];
	char sDownloadString[512];
	char sCommonString[512];
};

static CMenuConnectionProgress uiConnectionProgress;

CMenuConnectionProgress::CMenuConnectionProgress() : CMenuItemsHolder()
{

}

const char *CMenuConnectionProgress::Key( int key, int down )
{
	if( down )
	{
		switch( key )
		{
		case K_ESCAPE:
			dialog.Show();
			return uiSoundOut;
		case '~':
			consoleButton.Activate();
			return uiSoundLaunch;
		case 'A':
			HandleDisconnect();
		default:
			break;
		}
	}

	return CMenuItemsHolder::Key( key, down );
}

void CMenuConnectionProgress::HandleDisconnect( void )
{
	if( m_iState == STATE_NONE )
		return;

	if( m_iState == STATE_CONSOLE )
	{
		m_iState = STATE_NONE;
		return;
	}

	if( UI_IsVisible() && uiStatic.menuActive == this )
	{
		Hide();
		if( m_iSource == SOURCE_MENU && m_iState != STATE_MENU )
		{
			UI_CloseMenu();
			UI_SetActiveMenu( true );
			UI_Main_Menu();
			UI_MultiPlayer_Menu();
			UI_ServerBrowser_Menu();
			if( m_iState == STATE_DOWNLOAD )
			{
				Show();
				return;
			}
			m_iSource = SOURCE_CONSOLE;
		}
	}
	
	SetCommonText( "Disconnected." );

	m_iState = STATE_NONE;
	VidInit();
}

void CMenuConnectionProgress::DisconnectCb( CMenuBaseItem *pSelf , void *pExtra )
{
	CMenuConnectionProgress *parent = (CMenuConnectionProgress*)pSelf->Parent();

	if( parent->m_iState == STATE_DOWNLOAD )
	{
		EngFuncs::ClientCmd( true, "http_clear\n" );
		parent->m_iState = STATE_CONNECTING;
		parent->HandleDisconnect();
	}

	EngFuncs::ClientCmd( FALSE, "cmd disconnect;endgame disconnect\n");
}

void CMenuConnectionProgress::_Init( void )
{
	iFlags |= QMF_DIALOG;
	consoleButton.SetPicture( PC_CONSOLE );
	consoleButton.szName = "Console";

	SET_EVENT( consoleButton, onActivated )
	{
		CMenuConnectionProgress *parent = (CMenuConnectionProgress *)pSelf->Parent();
		EngFuncs::KEY_SetDest( KEY_CONSOLE );
		parent->m_iState = STATE_CONSOLE;
		parent->m_iSource = SOURCE_CONSOLE;
		UI_CloseMenu();
		UI_SetActiveMenu( false );
	}
	END_EVENT( consoleButton, onActivated );

	disconnectButton.SetPicture( PC_DISCONNECT );
	disconnectButton.szName = "Disconnect";

	disconnectButton.onActivated = DisconnectCb;
	dialog.SetMessage( "Really disconnect?" );
	dialog.onPositive = DisconnectCb;

	title.iFlags = QMF_INACTIVE|QMF_DROPSHADOW;
	title.eTextAlignment = QM_CENTER;
	title.szName = sTitleString;

	skipButton.szName = "Skip";
	skipButton.onActivated.SetCommand( TRUE, "http_skip\n" );
	
	downloadText.szName = sDownloadString;
	commonText.szName = sCommonString;

	downloadProgress.LinkCvar( "scr_download", 0.0f, 100.0f );

	AddItem( consoleButton );
	AddItem( disconnectButton );
	AddItem( downloadProgress );
	AddItem( commonProgress );
	AddItem( title );
	AddItem( skipButton );
	AddItem( downloadText );
	AddItem( commonText );
}

void CMenuConnectionProgress::_VidInit( void )
{
	int dlg_h = ( m_iState == STATE_DOWNLOAD )?256:192;
	int dlg_y = 768 / 2 - dlg_h / 2;
	SetRect( DLG_X + 192, dlg_y, 640, dlg_h );
	int cursor = dlg_y + dlg_h;

	title.SetCharSize( UI_MED_CHAR_WIDTH, UI_MED_CHAR_HEIGHT );
	title.SetRect( DLG_X + 192, dlg_y + 16, 640, 20 );

	cursor -= 44;
	consoleButton.SetRect( DLG_X + 380, cursor, UI_BUTTONS_WIDTH / 2, UI_BUTTONS_HEIGHT );
	disconnectButton.SetRect( DLG_X + 530, cursor, UI_BUTTONS_WIDTH / 2, UI_BUTTONS_HEIGHT );

	if( gpGlobals->developer < 2 )
		consoleButton.iFlags |= QMF_HIDDEN;

	cursor -= 30;
	commonProgress.SetRect( DLG_X + 212, cursor, 600, 20 );

	cursor -= 50;
	commonText.SetCharSize( UI_SMALL_CHAR_WIDTH, UI_SMALL_CHAR_HEIGHT );
	commonText.SetRect( DLG_X + 212, cursor, 500, 40 );

	if( m_iState == STATE_DOWNLOAD )
	{
		cursor -= 30;
		downloadProgress.iFlags &= ~QMF_HIDDEN;
		downloadProgress.SetRect( DLG_X + 212, cursor, 500, 20 );
		skipButton.SetRect( DLG_X + 732, cursor, UI_BUTTONS_WIDTH / 2, UI_BUTTONS_HEIGHT );
		skipButton.iFlags &= ~QMF_HIDDEN;

		cursor -= 50;
		downloadText.iFlags &= ~QMF_HIDDEN;
		downloadText.SetCharSize( UI_SMALL_CHAR_WIDTH, UI_SMALL_CHAR_HEIGHT );
		downloadText.SetRect( DLG_X + 212, cursor, 500, 40 );
	}
	else
	{
		downloadProgress.iFlags |= QMF_HIDDEN;
		skipButton.iFlags |= QMF_HIDDEN;
		downloadText.iFlags |= QMF_HIDDEN;
	}

	m_scPos = pos.Scale();
	m_scSize = size.Scale();
}

void CMenuConnectionProgress::Draw( void )
{
	if( m_iState != STATE_MENU && CL_IsActive() || ( m_iState == STATE_NONE && uiStatic.menuActive == this ) )
	{
		m_iState = STATE_NONE;
		Hide();
		return;
	}
	UI_FillRect( 0,0, gpGlobals->scrWidth, gpGlobals->scrHeight, m_iState == STATE_NONE ? 0XFF000000 : 0x40000000 );
	UI_FillRect( m_scPos, m_scSize, uiPromptBgColor );
	CMenuItemsHolder::Draw();
}


void UI_ConnectionProgress_f( void )
{
	if( !strcmp( EngFuncs::CmdArgv(1), "disconnect" ) )
	{
		uiConnectionProgress.HandleDisconnect();
		return;
	}

	if( uiConnectionProgress.m_iState == STATE_CONSOLE )
		return;

	else if( !strcmp( EngFuncs::CmdArgv(1), "dl" ) )
	{
		uiConnectionProgress.HandleDownload(  EngFuncs::CmdArgv( 2 ), EngFuncs::CmdArgv( 3 ), atoi(EngFuncs::CmdArgv( 4 ))+1, atoi(EngFuncs::CmdArgv( 5 )), EngFuncs::CmdArgv( 6 ) );
	}

	else if( !strcmp( EngFuncs::CmdArgv(1), "dlend" ) )
	{
		uiConnectionProgress.m_iState = STATE_CONNECTING;
		uiConnectionProgress.HandleDisconnect();
		return;
	}

	else if( !strcmp( EngFuncs::CmdArgv(1), "stufftext" ) )
	{
		uiConnectionProgress.HandleStufftext( atof( EngFuncs::CmdArgv( 2 ) ), EngFuncs::CmdArgv( 3 ) );
	}

	else if( !strcmp( EngFuncs::CmdArgv(1), "precache" ) )
	{
		uiConnectionProgress.HandlePrecache();
	}

	else if( !strcmp( EngFuncs::CmdArgv(1), "menu" ) )
	{
		uiConnectionProgress.m_iState = STATE_MENU;
		uiConnectionProgress.m_iSource = SOURCE_MENU;
		if( EngFuncs::CmdArgc() > 2 )
			uiConnectionProgress.SetServer( EngFuncs::CmdArgv(2) );
		uiConnectionProgress.SetCommonText( "Establishing network connection to server...");
		uiConnectionProgress.Show();
	}

	else if( !strcmp( EngFuncs::CmdArgv(1), "localserver" ) )
	{
		uiConnectionProgress.m_iState = STATE_MENU;
		uiConnectionProgress.m_iSource = SOURCE_MENU;
		if( EngFuncs::CmdArgc() > 2 )
			uiConnectionProgress.SetServer( EngFuncs::CmdArgv(2) );
		uiConnectionProgress.SetCommonText( "Starting local server...");
		uiConnectionProgress.Show();
	}

	else if( !strcmp( EngFuncs::CmdArgv(1), "serverinfo" ) )
	{
		if( EngFuncs::CmdArgc() > 2 )
			uiConnectionProgress.SetServer( EngFuncs::CmdArgv(2) );
		uiConnectionProgress.m_iState = STATE_CONNECTING;
		uiConnectionProgress.SetCommonText( "Parsing server info..." );
		uiConnectionProgress.Show();
	}

	uiConnectionProgress.VidInit();
}
 