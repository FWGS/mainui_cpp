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
#include "PicButton.h"
#include "YesNoMessageBox.h"
#include "keydefs.h"
#include "MenuStrings.h"

#define ART_BANNER		"gfx/shell/head_newgame"

class CMenuNewGame : public CMenuFramework
{
public:
	CMenuNewGame() : CMenuFramework( "CMenuNewGame" ) { }
private:
	virtual void _Init();

	static void StartGameCb( CMenuBaseItem *pSelf, void *pExtra );
	static void ShowDialogCb( CMenuBaseItem *pSelf, void *pExtra );

	CMenuYesNoMessageBox  msgBox;
};

static CMenuNewGame	uiNewGame;

/*
=================
CMenuNewGame::StartGame
=================
*/
void CMenuNewGame::StartGameCb( CMenuBaseItem *pSelf, void *pExtra )
{
	if( EngFuncs::GetCvarFloat( "host_serverstate" ) && EngFuncs::GetCvarFloat( "maxplayers" ) > 1 )
		EngFuncs::HostEndGame( "end of the game" );

	EngFuncs::CvarSetValue( "skill", int((size_t)pExtra) );
	EngFuncs::CvarSetValue( "deathmatch", 0.0f );
	EngFuncs::CvarSetValue( "teamplay", 0.0f );
	EngFuncs::CvarSetValue( "pausable", 1.0f ); // singleplayer is always allowing pause
	EngFuncs::CvarSetValue( "maxplayers", 1.0f );
	EngFuncs::CvarSetValue( "coop", 0.0f );

	EngFuncs::PlayBackgroundTrack( NULL, NULL );

	EngFuncs::ClientCmd( FALSE, "newgame\n" );
}

void CMenuNewGame::ShowDialogCb( CMenuBaseItem *pSelf, void *pExtra )
{
	CMenuNewGame *parent = (CMenuNewGame *)pSelf->Parent();

	parent->msgBox.onPositive.pExtra = pExtra;
	parent->msgBox.Show();
}

/*
=================
CMenuNewGame::Init
=================
*/
void CMenuNewGame::_Init( void )
{
	AddItem( background );
	AddItem( banner );

	banner.SetPicture( ART_BANNER );
	
	CMenuPicButton *easy = AddButton( "Easy", MenuStrings[IDS_NEWGAME_EASYHELP], PC_EASY, StartGameCb, QMF_NOTIFY );
	easy->onActivatedClActive.pExtra = easy->onActivated.pExtra = (void*)1;

	CMenuPicButton *medium = AddButton( "Medium", MenuStrings[IDS_NEWGAME_MEDIUMHELP], PC_MEDIUM, StartGameCb, QMF_NOTIFY );
	medium->onActivatedClActive.pExtra = medium->onActivated.pExtra = (void*)2;

	CMenuPicButton *hard = AddButton( "Difficult", MenuStrings[IDS_NEWGAME_DIFFICULTHELP], PC_DIFFICULT, StartGameCb, QMF_NOTIFY );
	hard->onActivatedClActive.pExtra = hard->onActivated.pExtra = (void*)3;

	easy->onActivatedClActive = medium->onActivatedClActive = hard->onActivatedClActive = ShowDialogCb;

	AddButton( "Cancel", "Go back to the main menu", PC_CANCEL, HideCb, QMF_NOTIFY );

	msgBox.SetMessage( MenuStrings[IDS_NEWGAME_NEWPROMPT] );
	msgBox.HighlightChoice( CMenuYesNoMessageBox::HIGHLIGHT_NO );
	msgBox.Link( this );

}

/*
=================
UI_NewGame_Precache
=================
*/
void UI_NewGame_Precache( void )
{
	EngFuncs::PIC_Load( ART_BANNER );
}

/*
=================
UI_NewGame_Menu
=================
*/
void UI_NewGame_Menu( void )
{
	// completely ignore save\load menus for multiplayer_only
	if( gMenu.m_gameinfo.gamemode == GAME_MULTIPLAYER_ONLY || !EngFuncs::CheckGameDll() )
		return;

	UI_NewGame_Precache();
	uiNewGame.Show();
}
