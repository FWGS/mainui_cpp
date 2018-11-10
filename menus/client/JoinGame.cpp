#include "BaseMenu.h"
#include "ClientWindow.h"
#include "ScrollView.h"

static class CClientJoinGame : public CClientWindow
{
public:
	typedef CClientWindow BaseClass;
	CClientJoinGame() : BaseClass( "CClientJoinGame" ) {}

	void _Init();
	void Reload();
	void Draw();

	bool hasSpectator;
	bool hasVIP;
	bool hasCancel;
	CMenuAction *spectate;
	CMenuAction *vipbutton;
	CMenuAction *confirm;
	CMenuAction *cancel;
	CMenuAction text;
	CMenuScrollView scroll;

	CEventCallback MakeCb( const char *cmd )
	{
		return CEventCallback( MenuCb( &CClientJoinGame::cb ), (void*)cmd );
	}

private:
	const char *command;
	char textbuffer[1024];

	void ConfirmSelection()
	{
		EngFuncs::ClientCmd( FALSE, command );
		Hide();
	}

	void cb( void *pExtra )
	{
		command = (const char*)pExtra;
		confirm->Show();
	}

} uiJoinGame;

void CClientJoinGame::_Init()
{
	AddButton( '1', L( "Cstrike_Terrorist_Forces" ),
		Point( 100, 180 ), MakeCb( "jointeam 1" ));
	AddButton( '2', L( "Cstrike_CT_Forces" ),
		Point( 100, 230 ), MakeCb( "jointeam 2" ));
	vipbutton = AddButton( '3', L( "Cstrike_VIP_Team" ),
		Point( 100, 280 ), MakeCb( "jointeam 3" ));

	AddButton( '5', L( "Cstrike_Team_AutoAssign" ),
		Point( 100, 380 ), MakeCb( "jointeam 5" ));
	spectate = AddButton( '6', L( "Cstrike_Menu_Spectate" ),
		Point( 100, 430 ), MakeCb( "jointeam 6" ));

	confirm = AddButton( 0, L( "Cstrike_Join_Team" ),
		Point( 100, 480 ), CEventCallback( VoidCb( &CClientJoinGame::ConfirmSelection )));

	cancel = AddButton( '0', L( "Cancel" ),
		Point( 100, 530 ), CEventCallback( VoidCb( &CMenuBaseWindow::Hide ) ) );

	scroll.SetRect( 400, 180, 400, 200 );
	scroll.bDrawStroke = true;
	scroll.colorStroke = uiInputTextColor;
	scroll.iStrokeWidth = 1;

	text.SetRect( 0, 0, scroll.size.w, 0);
	text.SetBackground( 0U );
	text.SetInactive( true );
	text.SetCharSize( QM_SMALLFONT );
	text.szName = textbuffer;
	text.m_bLimitBySize = false;

	scroll.AddItem( text );
	AddItem( scroll );

	szName = L("Cstrike_Join_Team");
}

void CClientJoinGame::Reload()
{
	if( hasSpectator )
	{
		spectate->Show();
		keys[6] = spectate->onActivated;
	}
	else
	{
		keys[6].Reset();
		spectate->Hide();
	}

	if( hasVIP )
	{
		keys[3] = vipbutton->onActivated;
		vipbutton->Show();
	}
	else
	{
		keys[3].Reset();
		vipbutton->Hide();
	}

	if( hasCancel )
	{
		keys[0] = cancel->onActivated;
		cancel->Show();
	}
	else
	{
		keys[0].Reset();
		cancel->Hide();
	}

	confirm->Hide();

	const char *mapname;

	if( (mapname = g_pClient->GetLevelName())[0] )
	{
		char buf[256];
		snprintf( buf, 256, "maps/%s.txt", mapname );

		char *txt = (char*)EngFuncs::COM_LoadFile( buf );

		if( txt )
		{
			Q_strncpy( textbuffer, txt, sizeof( textbuffer ));

			EngFuncs::COM_FreeFile( txt );
		}
		else
		{
			Q_strncpy( textbuffer, L( "Cstrike_TitlesTXT_Map_Description_not_available" ), sizeof( textbuffer ));
		}
	}
	else
	{
		Q_strncpy( textbuffer, L( "Cstrike_TitlesTXT_Map_Description_not_available" ), sizeof( textbuffer ));
	}

	text.size.h = 0; // recalc
	scroll.VidInit();
}

void CClientJoinGame::Draw()
{
	// HACK!
	BaseClass::Draw();
}

void UI_JoinGame_Show( int param1, int param2 )
{
	uiJoinGame.hasSpectator = param1 & 1;
	uiJoinGame.hasVIP		= param1 & 2;
	uiJoinGame.hasCancel    = param1 & 4;

	EngFuncs::KEY_SetDest( KEY_MENU );
	uiJoinGame.Show();
}
