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
	static void StartGameCb( float skill );
	void Show() override
	{
		if( gMenu.m_gameinfo.flags & GFL_NOSKILLS )
		{
			StartGameCb( 1.0f );
			return;
		}

		CMenuFramework::Show();
	}
private:
	void _Init() override;

	static void ShowDialogCb( CMenuBaseItem *pSelf, void *pExtra  );

	CMenuYesNoMessageBox  msgBox;

	CEventCallback easyCallback;
	CEventCallback normCallback;
	CEventCallback hardCallback;
};

/*
=================
CMenuNewGame::StartGame
=================
*/
void CMenuNewGame::StartGameCb( float skill )
{
	if( EngFuncs::GetCvarFloat( "host_serverstate" ) && EngFuncs::GetCvarFloat( "maxplayers" ) > 1 )
		EngFuncs::HostEndGame( "end of the game" );

	EngFuncs::CvarSetValue( "skill", skill );
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
	CMenuNewGame *ui = (CMenuNewGame*)pSelf->Parent();

	ui->msgBox.onPositive = *(CEventCallback*)pExtra;
	ui->msgBox.Show();
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

	SET_EVENT( easyCallback, CMenuNewGame::StartGameCb( 1.0f ) );
	SET_EVENT( normCallback, CMenuNewGame::StartGameCb( 2.0f ) );
	SET_EVENT( hardCallback, CMenuNewGame::StartGameCb( 3.0f ) );
	
	CMenuPicButton *easy = AddButton( L( "GameUI_Easy" ), L( "StringsList_200" ), PC_EASY, easyCallback, QMF_NOTIFY );
	CMenuPicButton *norm = AddButton( L( "GameUI_Medium" ), L( "StringsList_201" ), PC_MEDIUM, normCallback, QMF_NOTIFY );
	CMenuPicButton *hard = AddButton( L( "GameUI_Hard" ), L( "StringsList_202" ), PC_DIFFICULT, hardCallback, QMF_NOTIFY );

	easy->onReleasedClActive =
		norm->onReleasedClActive =
		hard->onReleasedClActive = ShowDialogCb;
	easy->onReleasedClActive.pExtra = &easyCallback;
	norm->onReleasedClActive.pExtra = &normCallback;
	hard->onReleasedClActive.pExtra = &hardCallback;

	AddButton( L( "GameUI_Cancel" ), L( "Go back to the Main menu" ), PC_CANCEL, VoidCb( &CMenuNewGame::Hide ), QMF_NOTIFY );

	msgBox.SetMessage( L( "StringsList_240" ) );
	msgBox.HighlightChoice( CMenuYesNoMessageBox::HIGHLIGHT_NO );
	msgBox.Link( this );

}

ADD_MENU( menu_newgame, CMenuNewGame, UI_NewGame_Menu );
