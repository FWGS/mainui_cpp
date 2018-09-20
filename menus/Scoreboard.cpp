#include <string.h>
#include "BaseWindow.h"
#include "Action.h"
#include "Table.h"
#include "BaseModel.h"
#include "Scoreboard.h"
#include "com_model.h"


#define RGB_YELLOWISH 0x00FFA000 //255,160,0
#define RGB_REDISH 0x00FF1010 //255,16,16
#define RGB_GREENISH 0x0000A000 //0,160,0
#define RGB_WHITE 0x00FFFFFF

uint g_ColorBlue = PackRGB( 154, 204, 255 );
uint g_ColorRed	= PackRGB( 255, 64, 64 );

struct player_t
{
	int kills;
	const char *name;
	const char *attrib;
	byte thisplayer;
	char deaths[6];
	char kills_buf[6];
	char ping[7];
};

class CMenuScoreboardModel : public CMenuBaseModel
{
public:
	CMenuScoreboardModel() : CMenuBaseModel()
	{
		players.EnsureCapacity( 32 );
	}

	static int PlayerCompar( const void *a, const void *b )
	{
		const player_t *_a = (const player_t *)a;
		const player_t *_b = (const player_t *)b;

		if( _a->kills < _b->kills ) return 1;
		else if( _a->kills > _b->kills ) return -1;
		return 0;
	}

	void Update() override
	{
		qsort( players.Base(), players.Count(), sizeof( player_t ), PlayerCompar );
	}

	int GetColumns() const override
	{
		return 5;
	}

	int GetRows() const override
	{
		return players.Count();
	}

	const char *GetCellText( int line, int column ) override
	{
		switch( column )
		{
		case 0: return players[line].name;
		case 1: return players[line].attrib;
		case 2: return players[line].kills_buf;
		case 3: return players[line].deaths;
		case 4: return players[line].ping;
		}

		return NULL;
	}
	bool GetLineColor( int line, uint &color, bool &force ) const override
	{
		if( players[line].thisplayer )
		{
			color = PackRGBA( 255, 255, 255, 32 );
			force = true;
			return true;
		}

		return false;
	}

	bool GetCellColors( int line, int column, uint &color, bool &force ) const override
	{
		color = isTerrorist ? g_ColorRed : g_ColorBlue;
		if( column == 0 )
			force = EngFuncs::GetCvarFloat("hud_colored") != 0.0f ? false : true;
		else force = false;
		return true;
	}

	unsigned int GetAlignmentForColumn( int column ) const override
	{
		if( column == 0 )
			return QM_LEFT;
		return QM_RIGHT;
	}

	bool isTerrorist;
	CUtlVector<player_t> players;
};

class CMenuScoreboard : public CMenuBaseWindow
{
public:
	CMenuScoreboard( ) : CMenuBaseWindow( "Scoreboard" ){ }

	void _Init() override;
	void _VidInit() override;
	void Draw() override;

	void Clear();

	bool bDrawStroke;

private:
	CMenuScoreboardModel CTs_model;
	CMenuScoreboardModel Ts_model;
	CMenuScoreboardModel specs_model;

	CMenuAction CTs_text;
	CMenuAction CTs_score;
	CMenuAction Ts_text;
	CMenuAction Ts_score;
	CMenuAction specs_text;
	CMenuAction serverName;
	CMenuTable CTs;
	CMenuTable Ts;

	CUtlVector<player_t> spectators;

	char spectators_buf[1024];
	char serverName_buf[256];
	char Ts_score_buf[8];
	char CTs_score_buf[8];
};

static CMenuScoreboard scoreboard;

void CMenuScoreboard::Clear()
{
	Ts_score_buf[0] = CTs_score_buf[0] = '0';
	Ts_score_buf[1] = CTs_score_buf[1] = '\0';

	snprintf( serverName_buf, sizeof( serverName_buf ), "Server name: %s", g_pClient->GetServerHostName() );

	spectators.RemoveAll();

	CTs_model.players.RemoveAll();
	Ts_model.players.RemoveAll();
}

void CMenuScoreboard::Draw()
{
	Clear();

	for( int i = 0; i < MAX_CLIENTS; i++ )
	{
		if( i < 3 ) // MAX_TEAMS
		{
			team_info_t *team;
			if( g_pClient->GetTeamInfo( i, &team ) )
			{
				if( !strcmp( team->name, "TERRORIST" ) )
					snprintf( Ts_score_buf, sizeof( Ts_score_buf ), "%d", team->frags );
				else if ( !strcmp( team->name, "CT" ) )
					snprintf( CTs_score_buf, sizeof( CTs_score_buf ), "%d", team->frags );
			}
		}

		extra_player_info_t *extra;
		hud_player_info_t *pplayer;
		bool isBot;

		if( g_pClient->GetPlayerExtraInfo( i, &pplayer, &extra, &isBot ))
		{
			player_t player;

			player.kills = extra->frags;
			player.name = pplayer->name;
			if( isBot ) strcpy( player.ping, "BOT" );
			else snprintf( player.ping, sizeof( player.ping), "%d", pplayer->ping );
			snprintf( player.deaths, sizeof( player.deaths), "%d", extra->deaths );
			snprintf( player.kills_buf, sizeof( player.kills_buf), "%d", player.kills );
			player.thisplayer = pplayer->thisplayer;
			if( extra->dead ) player.attrib = "Dead";
			else if( extra->has_c4 ) player.attrib = "Bomb";
			else if( extra->vip ) player.attrib = "VIP";
			else player.attrib = NULL;

			switch( extra->teamnumber )
			{
			case TEAM_TERRORIST:
				Ts_model.players.AddToTail( player ); break;
			case TEAM_CT:
				CTs_model.players.AddToTail( player ); break;
			case TEAM_SPECTATOR:
			case TEAM_UNASSIGNED:
				spectators.AddToTail( player ); break;
			}
		}
	}
	snprintf( spectators_buf, sizeof( spectators_buf ), "%s: ", g_pClient->Localize( "Cstrike_TitlesTXT_Spectators" ) );
	if( spectators.Count() )
	{
		char temp[128];

		for( int i = 0; i < spectators.Count() - 1; i++ )
		{
			snprintf( temp, sizeof( temp ), "^7%s, ^7", spectators[i].name );

			strncat( spectators_buf, temp, sizeof( spectators_buf ));
		}
		strncat( spectators_buf, spectators[spectators.Count() - 1].name, sizeof( spectators_buf ) );
	}
	specs_text.size.h = g_FontMgr.GetTextHeightExt( specs_text.font, spectators_buf, specs_text.charSize, specs_text.size.w );
	specs_text.pos.y = -specs_text.size.h;
	specs_text.CalcSizes();
	specs_text.CalcPosition();
	specs_text.bIgnoreColorstring = EngFuncs::GetCvarFloat("hud_colored") == 0.0f;

	int newSize = size.h - CTs.pos.y - specs_text.size.h;
	if( newSize != CTs.size.h )
	{
		Ts.size.h = CTs.size.h = newSize;
		Ts.VidInit();
		CTs.VidInit();
	}

	CTs_model.Update();
	Ts_model.Update();

	background.bDrawStroke = bDrawStroke;

	CMenuBaseWindow::Draw();
}

void CMenuScoreboard::_Init()
{
	Clear();

	iFlags |= QMF_DISABLESCAILING;

	background.bForceColor = true;
	background.iStrokeColor = uiInputTextColor;

	serverName.iFlags |= QMF_DISABLESCAILING;
	serverName.SetCharSize( QM_SMALLFONT );
	serverName.eTextAlignment = QM_LEFT;
	serverName.szName = serverName_buf;

	specs_text.iFlags |= QMF_DISABLESCAILING;
	specs_text.SetCharSize( QM_SMALLFONT );
	specs_text.eTextAlignment = QM_LEFT|QM_TOP;
	specs_text.szName = spectators_buf;

	CTs_text.iFlags |= QMF_DISABLESCAILING;
	CTs_text.SetCharSize( QM_BIGFONT );
	CTs_text.SetBackground( PackRGBA( 0, 0, 255, 64 ) );
	CTs_text.szName = g_pClient->Localize( "Cstrike_ScoreBoard_CT" );

	Ts_text.iFlags |= QMF_DISABLESCAILING;
	Ts_text.SetCharSize( QM_BIGFONT );
	Ts_text.SetBackground( PackRGBA( 255, 0, 0, 64 ) );
	Ts_text.szName = g_pClient->Localize( "Cstrike_ScoreBoard_Ter" );

	CTs_score.iFlags |= QMF_DISABLESCAILING;
	CTs_score.SetCharSize( QM_BIGFONT );
	CTs_score.szName = CTs_score_buf;

	Ts_score.iFlags |= QMF_DISABLESCAILING;
	Ts_score.SetCharSize( QM_BIGFONT );
	Ts_score.szName = Ts_score_buf;

	CTs_text.eTextAlignment = Ts_score.eTextAlignment = QM_LEFT;
	Ts_text.eTextAlignment = CTs_score.eTextAlignment = QM_RIGHT;

	CTs_score.iColor = Ts_score.iColor = CTs_text.iColor = Ts_text.iColor =  uiColorWhite;

	CTs.iFlags |= QMF_DISABLESCAILING|QMF_INACTIVE;
	Ts.bShowScrollBar = CTs.bShowScrollBar = false;
	Ts.bDrawStroke = CTs.bDrawStroke = true;
	Ts.iOutlineWidth = CTs.iOutlineWidth = 1;
	Ts.iStrokeColor = CTs.iStrokeColor = uiInputTextColor;
	Ts.iBackgroundColor = CTs.iBackgroundColor = 0;
	CTs.iHeaderColor = g_ColorBlue;
	CTs.SetupColumn( 0, g_pClient->Localize( "Cstrike_TitlesTXT_PLAYERS" ), 0.4f );
	CTs.SetupColumn( 1, "", 0.15f );
	CTs.SetupColumn( 2, g_pClient->Localize( "Cstrike_TitlesTXT_SCORE" ), 0.15f );
	CTs.SetupColumn( 3, g_pClient->Localize( "Cstrike_TitlesTXT_DEATHS" ), 0.15f );
	CTs.SetupColumn( 4, g_pClient->Localize( "Cstrike_TitlesTXT_LATENCY" ), 0.15f );
	CTs.SetModel( &CTs_model );
	CTs.SetCharSize( QM_DEFAULTFONT );

	CTs_model.isTerrorist = false;
	Ts_model.isTerrorist = true;
	Ts.iFlags |= QMF_DISABLESCAILING|QMF_INACTIVE;
	Ts.iHeaderColor = g_ColorRed;
	Ts.SetupColumn( 0, g_pClient->Localize( "Cstrike_TitlesTXT_PLAYERS" ), 0.4f );
	Ts.SetupColumn( 1, "", 0.15f );
	Ts.SetupColumn( 2, g_pClient->Localize( "Cstrike_TitlesTXT_SCORE" ), 0.15f );
	Ts.SetupColumn( 3, g_pClient->Localize( "Cstrike_TitlesTXT_DEATHS" ), 0.15f );
	Ts.SetupColumn( 4, g_pClient->Localize( "Cstrike_TitlesTXT_LATENCY" ), 0.15f );
	Ts.SetModel( &Ts_model );
	Ts.SetCharSize( QM_DEFAULTFONT );

	AddItem( background );
	AddItem( serverName );
	AddItem( specs_text );
	AddItem( CTs_text );
	AddItem( CTs_score );
	AddItem( Ts_text );
	AddItem( Ts_score );
	AddItem( CTs );
	AddItem( Ts );
}

void CMenuScoreboard::_VidInit()
{
	serverName.pos = Point( 4, 4 );
	serverName.size = Size( size.w - 8, g_FontMgr.GetFontTall( serverName.font ) * 1.25f );

	specs_text.size = Size( size.w - 8, 1 );
	specs_text.pos = Point( 4, size.h );

	CTs_score.pos = CTs_text.pos = Point( 4, serverName.size.h + serverName.pos.y + 4 );
	Ts_score.pos = Ts_text.pos = Point( size.w / 2 + 4, serverName.size.h + serverName.pos.y + 4 );

	Ts_score.size = Ts_text.size = CTs_score.size = CTs_text.size =
		Size( size.w / 2 - 8, g_FontMgr.GetFontTall( CTs_text.font ) * 1.25f );

	Ts_score.pos.x += g_FontMgr.GetFontTall( Ts_text.font ) / 2;
	CTs_score.size.w -= g_FontMgr.GetFontTall( Ts_text.font ) / 2;

	CTs.pos = Point( CTs_text.pos.x, CTs_text.size.h + CTs_text.pos.y + 4 );
	 Ts.pos = Point(  Ts_text.pos.x,  Ts_text.size.h +  Ts_text.pos.y + 4 );
	Ts.size = CTs.size = Size( CTs_text.size.w, size.h - CTs.pos.y );
}

void UI_SetupScoreboard( int xstart, int xend, int ystart, int yend, unsigned int color, bool drawStroke )
{
	Point pt = scoreboard.GetRenderPosition();
	Size sz = scoreboard.GetRenderSize();
	bool vidinit = false;

	if( pt.x != xstart || pt.y != ystart )
	{
		scoreboard.SetCoord( xstart, ystart );
		vidinit = true;
	}

	if( sz.w != xend - xstart || sz.h != yend - ystart )
	{
		scoreboard.SetSize( xend - xstart, yend - ystart );
		vidinit = true;
	}

	if( vidinit )
	{
		UI_VidInitScoreboard();
	}

	scoreboard.bDrawStroke = drawStroke;
	scoreboard.background.iColor = color;
}

void UI_VidInitScoreboard()
{
	scoreboard.Init();
	scoreboard.VidInit();
}

void UI_DrawScoreboard()
{
	scoreboard.Draw();
}
