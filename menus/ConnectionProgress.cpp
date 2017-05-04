#include "extdll.h"
#include "BaseMenu.h"
#include "ConnectionProgress.h"
#include "Utils.h"

CMenuConnectionProgress::CMenuConnectionProgress() : CMenuItemsHolder()
{

}

const char *CMenuConnectionProgress::Key( int key, int down )
{
	return CMenuItemsHolder::Key( key, down );
}

void CMenuConnectionProgress::_Init( void )
{
	consoleButton.SetPicture( PC_CONSOLE );

	SET_EVENT( consoleButton, onActivated )
	{
		EngFuncs::KEY_SetDest( KEY_CONSOLE );
		UI_CloseMenu();
		UI_SetActiveMenu( false );
	}
	END_EVENT( consoleButton, onActivated );

	disconnectButton.SetPicture( PC_DISCONNECT );

	SET_EVENT( disconnectButton, onActivated )
	{
		pSelf->Parent()->Hide();
		UI_CloseMenu();
		UI_SetActiveMenu( true );
		EngFuncs::ClientCmd( TRUE, "cmd disconnect;endgame disconnect;wait;wait;wait;menu_options;menu_main\n");
	}
	END_EVENT( disconnectButton, onActivated );

	downloadProgress.LinkCvar( "scr_download", 0.0f, 100.0f );

	precacheProgress.LinkCvar( "scr_loading", 0.0f, 100.0f );

	AddItem( consoleButton );
	AddItem( disconnectButton );
	AddItem( downloadProgress );
	AddItem( precacheProgress );
}

void CMenuConnectionProgress::_VidInit( void )
{
	SetRect( DLG_X + 192, 256, 640, 256 );

	consoleButton.SetRect( DLG_X + 380, 460, UI_BUTTONS_WIDTH / 2, UI_BUTTONS_HEIGHT );
	disconnectButton.SetRect( DLG_X + 530, 460, UI_BUTTONS_WIDTH / 2, UI_BUTTONS_HEIGHT );
	downloadProgress.SetRect( DLG_X + 212, 276, 500, 20 );
	precacheProgress.SetRect( DLG_X + 212, 316, 500, 20 );

	m_scPos = pos.Scale();
	m_scSize = size.Scale();
}

void CMenuConnectionProgress::Draw( void )
{
	UI_FillRect( 0,0, gpGlobals->scrWidth, gpGlobals->scrHeight, 0x40000000 );
	UI_FillRect( m_scPos, m_scSize, uiPromptBgColor );
	CMenuItemsHolder::Draw();
}


void UI_ConnectionProgress_f()
{
	static CMenuConnectionProgress staticProgress;
	staticProgress.Show();
}
