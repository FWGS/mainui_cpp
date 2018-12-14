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
#include "Action.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "YesNoMessageBox.h"
#include "keydefs.h"
#include "MenuStrings.h"
#include "PlayerIntroduceDialog.h"
#include "Switch.h"

#define ART_MINIMIZE_N	"gfx/shell/min_n"
#define ART_MINIMIZE_F	"gfx/shell/min_f"
#define ART_MINIMIZE_D	"gfx/shell/min_d"
#define ART_CLOSEBTN_N	"gfx/shell/cls_n"
#define ART_CLOSEBTN_F	"gfx/shell/cls_f"
#define ART_CLOSEBTN_D	"gfx/shell/cls_d"

class CMenuMain: public CMenuFramework
{
public:
	CMenuMain() : CMenuFramework( "CMenuMain" ) { }

	const char *Key( int key, int down ) override;
	const char *Activate( ) override;

private:
	void _Init() override;
	void _VidInit( ) override;

	void QuitDialog( void *pExtra = NULL );
	void DisconnectDialogCb();

	CMenuPicButton	console;
	CMenuPicButton	resumeGame;
	CMenuPicButton	disconnect;
	CMenuPicButton	createGame;
	CMenuPicButton	configuration;
	CMenuPicButton	multiPlayer;
	CMenuPicButton	previews;
	CMenuPicButton	quit;
	CMenuSwitch		renderworld;

	// quit dialog
	CMenuYesNoMessageBox dialog;
};

static CMenuMain uiMain;

void CMenuMain::QuitDialog(void *pExtra)
{
	if( CL_IsActive() && EngFuncs::GetCvarFloat( "host_serverstate" ) && EngFuncs::GetCvarFloat( "maxplayers" ) == 1.0f )
		dialog.SetMessage( L( "StringsList_235" ) );
	else
		dialog.SetMessage( L( "StringsList_236" ) );

	dialog.onPositive.SetCommand( FALSE, "quit\n" );
	dialog.Show();
}

void CMenuMain::DisconnectDialogCb()
{
	dialog.onPositive.SetCommand( FALSE, "cmd disconnect;endgame disconnect;wait;wait;wait;menu_options;menu_main\n" );
	dialog.SetMessage( L( "Really disconnect?" ) );
	dialog.Show();
}

/*
=================
CMenuMain::Key
=================
*/
const char *CMenuMain::Key( int key, int down )
{
	if( down && UI::Key::IsEscape( key ) )
	{
		if ( CL_IsActive( ))
		{
			if( !dialog.IsVisible() )
				UI_CloseMenu();
		}
		else
		{
			QuitDialog( );
		}
		return uiSoundNull;
	}
	return CMenuFramework::Key( key, down );
}

/*
=================
UI_Main_ActivateFunc
=================
*/
const char *CMenuMain::Activate( void )
{
	if ( CL_IsActive( ))
	{
		resumeGame.Show();
		disconnect.Show();
		renderworld.Show();
	}
	else
	{
		resumeGame.Hide();
		disconnect.Hide();
		renderworld.Hide();
	}

	CMenuPicButton::ClearButtonStack();

	return 0;
}

void CMenuMain::_Init( void )
{
	// console
	console.SetNameAndStatus( L( "GameUI_Console" ), L( "Show console" ) );
	console.iFlags |= QMF_NOTIFY;
	console.SetPicture( PC_CONSOLE );
	SET_EVENT_MULTI( console.onActivated,
	{
		UI_SetActiveMenu( FALSE );
		EngFuncs::KEY_SetDest( KEY_CONSOLE );
	});

	createGame.SetNameAndStatus( GameUI_GameMenu_CreateServer, "" );
	createGame.SetPicture( PC_CREATE_GAME );
	createGame.iFlags |= QMF_NOTIFY;
	createGame.onActivated = UI_CreateGame_Menu;

	resumeGame.SetNameAndStatus( L( "GameUI_GameMenu_ResumeGame" ), L( "StringsList_188" ) );
	resumeGame.SetPicture( PC_RESUME_GAME );
	resumeGame.iFlags |= QMF_NOTIFY;
	resumeGame.onActivated = UI_CloseMenu;

	disconnect.SetNameAndStatus( L( "GameUI_GameMenu_Disconnect" ), L( "Disconnect from server" ) );
	disconnect.SetPicture( PC_DISCONNECT );
	disconnect.iFlags |= QMF_NOTIFY;
	disconnect.onActivated = VoidCb( &CMenuMain::DisconnectDialogCb );

	multiPlayer.SetNameAndStatus( L( "GameUI_Multiplayer" ), L( "StringsList_198" ) );
	multiPlayer.SetPicture( PC_MULTIPLAYER );
	multiPlayer.iFlags |= QMF_NOTIFY;
	multiPlayer.onActivated = UI_ServerBrowser_Menu;

	configuration.SetNameAndStatus( L( "GameUI_Options" ), L( "StringsList_193" ) );
	configuration.SetPicture( PC_CONFIG );
	configuration.iFlags |= QMF_NOTIFY;
	configuration.onActivated = UI_Options_Menu;

	previews.SetNameAndStatus( L( "Previews" ), L( "StringsList_400" ) );
	previews.SetPicture( PC_PREVIEWS );
	previews.iFlags |= QMF_NOTIFY;
	SET_EVENT( previews.onActivated, EngFuncs::ShellExecute( MenuStrings[ IDS_MEDIA_PREVIEWURL ], NULL, false ) );

	quit.SetNameAndStatus( L( "GameUI_GameMenu_Quit" ), L( "StringsList_236" ) );
	quit.SetPicture( PC_QUIT );
	quit.iFlags |= QMF_NOTIFY;
	quit.onActivated = MenuCb( &CMenuMain::QuitDialog );

	renderworld.bKeepToggleWidth = true;
	renderworld.bUpdateImmediately = true;
	renderworld.szName = "";
	renderworld.AddSwitch( "O" );
	renderworld.AddSwitch( "I" );
	renderworld.LinkCvar( "ui_renderworld" );
	renderworld.SetSize( renderworld.size.w * 0.75f, renderworld.size.h );
	renderworld.SetCoord( -renderworld.size.w - renderworld.size.h, renderworld.size.h );

	dialog.Link( this );

	AddItem( background );
	if ( EngFuncs::GetCvarFloat( "developer" ))
		AddItem( console );
	AddItem( disconnect );
	AddItem( resumeGame );
	AddItem( createGame );
	AddItem( multiPlayer );
	AddItem( configuration );
	AddItem( previews );
	AddItem( quit );
	AddItem( renderworld );
}

/*
=================
UI_Main_Init
=================
*/
void CMenuMain::_VidInit( void )
{
	if( CL_IsActive() )
	{
		console.SetCoord( 72, 280 );
		resumeGame.Show();
		resumeGame.SetCoord( 72, 330 );

		disconnect.Show();
		disconnect.SetCoord( 72, 380 );
	}
	else
	{
		console.SetCoord( 72, 380 );
		resumeGame.Hide();
		disconnect.Hide();
	}
	createGame.SetCoord( 72, 430 );
	multiPlayer.SetCoord( 72, 480 );
	configuration.SetCoord( 72, 530 );
	previews.SetCoord( 72, 580 );
	quit.SetCoord( 72, 630 );
}

/*
=================
UI_Main_Precache
=================
*/
void UI_Main_Precache( void )
{
}

/*
=================
UI_Main_Menu
=================
*/
void UI_Main_Menu( void )
{
	uiMain.Show();
}
ADD_MENU( menu_main, UI_Main_Precache, UI_Main_Menu );
