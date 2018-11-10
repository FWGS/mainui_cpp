#include "BaseMenu.h"
#include "ClientWindow.h"
#include "PlayerModelView.h"

class CClientJoinClass : public CClientWindow
{
public:
	typedef CClientWindow BaseClass;
	CClientJoinClass() : BaseClass( "CClientJoinClass" ) {}

	CEventCallback MakeCb( const char *cmd )
	{
		return CEventCallback( MenuCb( &CClientJoinClass::cb ), (void*)cmd );
	}

	void _Init();
	void Reload();

	CMenuAction *AddButton( int key, const char *name, const char *modelname, Point pos );

	bool hasCzero;
	bool hasCancel;
	CMenuAction *confirm;
	CMenuAction *cancel;
	CMenuPlayerModelView player;
	CMenuAction text;

	void ConfirmSelection()
	{
		EngFuncs::ClientCmd( FALSE, command );
		Hide();
	}

private:
	const char *command;
	char textbuffer[1024];

	void cb( void *pExtra )
	{
		const char *sz = (const char *)pExtra;
		const char *loctext;
		char model[256];
		static const char *table[5 * 6] =
		{
			"terror",   "urban",    "joinclass 1", "Cstrike_Terror_Label", "Cstrike_Urban_Label",
			"leet",     "gsg9",     "joinclass 2", "Cstrike_Leet_Label", "Cstrike_GSG9_Label",
			"arctic",   "sas",      "joinclass 3", "Cstrike_Arctic_Label", "Cstrike_SAS_Label",
			"guerilla", "gign",     "joinclass 4", "Cstrike_Guerilla_Label", "Cstrike_GIGN_Label",
			"militia",  "spetsnaz", "joinclass 5", "Cstrike_Militia_Label", "Cstrike_Spetsnaz_Label",
			"ct_random", "t_random", "joinclass 6", "Cstrike_AutoSelect_Label", "Cstrike_AutoSelect_Label",
		};

		confirm->Show();

		int i;
		for( i = 0; i <= 5 * 6; i += 5 )
		{
			if( !strcmp( sz, table[i+0]) )
			{
				command = table[i+2];
				loctext = L( table[i+3] );
				break;
			}

			if( !strcmp( sz, table[i+1]) )
			{
				command = table[i+2];
				loctext = L( table[i+4] );
				break;
			}
		}

		bool showModel = true;

		if( i >= 5 * 5 )
			showModel = false;

		i = 0;
		const char *t = loctext;
		while( t && *t && i < sizeof( textbuffer ) - 1 )
		{
			if( *t == '\\' && *(t+1) == 'n' )
			{
				t += 2;
			}
			else
			{
				textbuffer[i] = *t;
				i++;
				t++;
			}
		}
		textbuffer[i] = '\0';

		if( showModel ) // try to load model
		{
			snprintf( model, sizeof( model ), "models/player/%s/%s.mdl", sz, sz );

			player.eOverrideMode = CMenuPlayerModelView::PMV_SHOWMODEL;
            player.ent->model = g_pClient->LoadModel( model, &player.ent->curstate.modelindex );
			if( !player.ent->model )
				showModel = false;
		}

		if( !showModel ) // failed, load picture
		{
			snprintf( model, sizeof( model ), "gfx/vgui/%s.tga", sz );

			player.hPlayerImage = EngFuncs::PIC_Load( model );
			player.eOverrideMode = CMenuPlayerModelView::PMV_SHOWIMAGE;
		}
	}
};

static class CClientJoinClassT: public CClientJoinClass
{
	typedef CClientJoinClass BaseClass;
public:
	void _Init();
} uiJoinClassT;

static class CClientJoinClassCT: public CClientJoinClass
{
	typedef CClientJoinClass BaseClass;
public:
	void _Init();
} uiJoinClassCT;

void CClientJoinClass::_Init()
{
	confirm = CClientWindow::AddButton( 0, L( "CONFIRM CLASS" ),
		Point( 100, 480 ), CEventCallback( VoidCb( &CClientJoinClass::ConfirmSelection )));

	cancel =  CClientWindow::AddButton( '0', L( "Cancel" ),
		Point( 100, 530 ), CEventCallback( VoidCb( &CMenuBaseWindow::Hide ) ) );

	player.SetRect( 400, 180, 284, 284 );
	player.backgroundColor = uiColorBlack;
	player.colorStroke = uiPromptTextColor;
	player.iStrokeWidth = 1;
	player.eFocusAnimation = QM_NOFOCUSANIMATION;
	player.iFlags |= QMF_INACTIVE;
	player.bDrawAsPlayer = false;

	text.SetRect( 400, 500, 400, 200 );
	text.SetBackground( 0U );
	text.iFlags |= QMF_INACTIVE;
	text.SetCharSize( QM_SMALLFONT );
	text.szName = textbuffer;

	szName = L( "Cstrike_Join_Class" );
	AddItem( player );
	AddItem( text );
}

void CClientJoinClass::Reload()
{
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
	textbuffer[0] = 0;
    //if( player.ent )
    //    memset( player.ent, 0, sizeof( player.ent ));
    player.ent->angles[1] += 15;
}


CMenuAction *CClientJoinClass::AddButton( int key, const char *name, const char *modelname, Point pos )
{
	CMenuAction *act = CClientWindow::AddButton( key, name, pos, MakeCb( modelname ) );

	act->szStatusText = modelname;

	return act;
}

void CClientJoinClassT::_Init()
{
	AddButton( '1', L( "Cstrike_Terror" ), "terror",
		Point( 100, 180 ));
	AddButton( '2', L( "Cstrike_L337_Krew" ), "leet",
		Point( 100, 230 ));
	AddButton( '3', L( "Cstrike_Arctic" ), "arctic",
		Point( 100, 280 ));
	AddButton( '4', L( "Cstrike_Guerilla" ), "guerilla",
		Point( 100, 330 ));
	if( hasCzero )
		AddButton( '5', L( "Cstrike_Militia" ), "militia",
			Point( 100, 380 ));
	AddButton( '6', L( "Cstrike_Auto_Select" ), "t_random",
		Point( 100, 430 ));

	BaseClass::_Init();
}

void CClientJoinClassCT::_Init()
{
	AddButton( '1', L( "Cstrike_Urban" ), "urban",
		Point( 100, 180 ));
	AddButton( '2', L( "Cstrike_GSG9" ), "gsg9",
		Point( 100, 230 ));
	AddButton( '3', L( "Cstrike_SAS" ), "sas",
		Point( 100, 280 ));
	AddButton( '4', L( "Cstrike_GIGN" ), "gign",
		Point( 100, 330 ));
	if( hasCzero )
		AddButton( '5', L( "Cstrike_Spetsnaz" ), "spetsnaz",
			Point( 100, 380 ));
	AddButton( '6', L( "Cstrike_Auto_Select" ), "ct_random",
		Point( 100, 430 ));

	BaseClass::_Init();
}

void UI_JoinClassT_Show( int param1, int param2 )
{
	uiJoinClassT.hasCzero = param1;
	uiJoinClassT.hasCancel = param2;
	EngFuncs::KEY_SetDest( KEY_MENU );
	uiJoinClassT.Show();
}

void UI_JoinClassCT_Show( int param1, int param2 )
{
	uiJoinClassCT.hasCzero = param1;
	uiJoinClassCT.hasCancel = param2;
	EngFuncs::KEY_SetDest( KEY_MENU );
	uiJoinClassCT.Show();
}
