#include "extdll.h"
#include "BaseMenu.h"
#include "ConnectionProgress.h"
#include "Utils.h"

#include "ItemsHolder.h"
#include "ProgressBar.h"
#include "PicButton.h"
#include "YesNoMessageBox.h"

enum EMenuConnectionProgressState
{
STATE_NONE,
STATE_MENU, // do not hide when disconnected or in game
STATE_DOWNLOAD, // enlarge your connectionprogress window
STATE_STUFFTEXT, // stufftext progress from engine
STATE_PRECACHE, // precache stufftext from engine
STATE_CONSOLE // do not show until state reset
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
private:
	CMenuProgressBar precacheProgress;
	CMenuProgressBar downloadProgress;
	CMenuPicButton consoleButton;
	CMenuPicButton disconnectButton;
	CMenuPicButton skipButton;
	CMenuYesNoMessageBox dialog;
	CMenuAction title;
	CMenuAction downloadText;
	CMenuAction commonText;
	enum EMenuConnectionProgressState m_iState;
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
		case '`':
			consoleButton.Activate();
			return uiSoundLaunch;
		default:
			break;
		}
	}

	return CMenuItemsHolder::Key( key, down );
}

void CMenuConnectionProgress::DisconnectCb( CMenuBaseItem *pSelf , void *pExtra )
{
	EngFuncs::ClientCmd( FALSE, "cmd disconnect;endgame disconnect;wait;wait;wait;menu_options;menu_main\n");
}

void CMenuConnectionProgress::_Init( void )
{
	iFlags |= QMF_DIALOG;
	consoleButton.SetPicture( PC_CONSOLE );
	consoleButton.szName = "Console";

	SET_EVENT( consoleButton, onActivated )
	{
		EngFuncs::KEY_SetDest( KEY_CONSOLE );
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
	title.szName = "Connecting...";

	skipButton.szName = "Skip";
	
	downloadText.szName = "DOWNLOAD TEXT\nDOWNLOAD TEXT";
	commonText.szName = "COMMON TEXT\nCOMMON_TEXT";

	downloadProgress.LinkCvar( "scr_download", 0.0f, 100.0f );

	precacheProgress.LinkCvar( "scr_loading", 0.0f, 100.0f );

	AddItem( consoleButton );
	AddItem( disconnectButton );
	AddItem( downloadProgress );
	AddItem( precacheProgress );
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

	cursor -= 30;
	precacheProgress.SetRect( DLG_X + 212, cursor, 600, 20 );

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
	if( m_iState != STATE_MENU && CL_IsActive() )
	{
		m_iState = STATE_NONE;
		Hide();
		return;
	}
	UI_FillRect( 0,0, gpGlobals->scrWidth, gpGlobals->scrHeight, 0x40000000 );
	UI_FillRect( m_scPos, m_scSize, uiPromptBgColor );
	CMenuItemsHolder::Draw();
}


void UI_ConnectionProgress_f( void )
{
	uiConnectionProgress.Show();
}
