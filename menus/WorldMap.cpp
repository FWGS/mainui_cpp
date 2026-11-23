/*
WorldMap.cpp - Condition Zero: Deleted Scenes world map and mission select
Copyright (C) 2026 SergioPoverony

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll_menu.h"
#include "BaseMenu.h"
#include "BaseWindow.h"
#include "Action.h"
#include "Bitmap.h"
#include "BackgroundBitmap.h"
#include "Utils.h"

// the map picture is cut into gfx/vgui/worldmap_<x>_<y>.tga tiles, placed by their sizes
static constexpr int TILES_X = 3;
static constexpr int TILES_Y = 2;

// the layout follows resource/UI/WorldMap.res and WorldMapMissionSelect.res of the game, scaled up from 640x480
static constexpr int MISSION_HEIGHT = 24;
static constexpr int MISSION_MARKER_SIZE = 10;
static constexpr int MISSION_TEXT_OFFSET = 16;

static constexpr int SKILL_HEIGHT = 48;

// localization keys of the missions, like SECRETWAR_ORGANIZATION
static constexpr int KEY_SIZE = 32;

static constexpr int CHAR_SMALL  = 14;
static constexpr int CHAR_MEDIUM = 16;
static constexpr int CHAR_LARGE  = 20;

static constexpr unsigned int s_ColorHighlight = PackRGBA( 255, 255, 0, 255 );
static constexpr unsigned int s_ColorLabel = PackRGBA( 215, 215, 215, 255 );
static constexpr unsigned int s_ColorTitle = PackRGBA( 40, 210, 60, 255 );
static constexpr unsigned int s_ColorText = PackRGBA( 190, 190, 190, 255 );
static constexpr unsigned int s_ColorSkill = PackRGBA( 170, 170, 170, 255 );

enum
{
	MARKER_CURRENT,
	MARKER_UNLOCKED,
	MARKER_LOCKED,
	MARKER_COUNT
};

// the order of the mission bits in cl_levellocks, starting from bit 1, see LEVEL_* in the czeror game library
static constexpr struct WorldMapMission_t
{
	const char *title; // #<title>_TITLE
	const char *info;  // #<info>_BRIEF, _TIME, ..., differs from title only for Silo
	const char *image; // gfx/vgui/<image>_large.tga
	const char *map;
	int x, y, w; // label on the map
} s_Missions[] =
{
	{ "TRAINING",  "TRAINING",  "cz_train",     "cz_train01",      144, 184, 288 },
	{ "RECOIL",    "RECOIL",    "cz_recoil",    "cz_recoil-intro", 531, 331, 240 },
	{ "LOSTCAUSE", "LOSTCAUSE", "cz_lostcause", "cz_lostcause",    766, 315, 248 },
	{ "SILO",      "SECRETWAR", "cz_silo",      "cz_silo01",       724, 118, 288 },
	{ "BRECON",    "BRECON",    "cz_brecon",    "cz_brecon01",     528, 144, 320 },
	{ "DRUGLAB",   "DRUGLAB",   "cz_druglab",   "cz_druglab1",     222, 337, 272 },
	{ "MOTORCADE", "MOTORCADE", "cz_motor",     "cz_motor1",       459, 206, 320 },
	{ "THINICE",   "THINICE",   "cz_thinice",   "cz_thinice01",    380, 142, 144 },
	{ "DOWNED",    "DOWNED",    "cz_downed",    "cz_downed1",      238, 411, 320 },
	{ "HANKAGAI",  "HANKAGAI",  "cz_hankagai",  "cz_hankagai1",    864, 184, 152 },
	{ "TURNCRANK", "TURNCRANK", "cz_turncrank", "cz_turncrank",    118, 211, 304 },
	{ "ALAMO",     "ALAMO",     "cz_alamo",     "cz_alamo",        507, 230, 88  },
	{ "HIGHRISE",  "HIGHRISE",  "cz_highrise",  "cz_hr01",         438, 177, 320 },
	// bonus missions
	{ "TRUTH",     "TRUTH",     "cz_truth",     "cz_truth1",       856, 208, 160 },
	{ "RUN",       "RUN",       "cz_run",       "cz_run1",         856, 158, 144 },
	{ "MIAMI",     "MIAMI",     "cz_miami",     "cz_miami1",       222, 246, 272 },
	{ "FASTLINE",  "FASTLINE",  "cz_fastline",  "cz_fastline1",    859, 236, 152 },
	{ "PIPEDREAM", "PIPEDREAM", "cz_pipedream", "cz_pipedream1",   595, 225, 144 },
	{ "SANDSTORM", "SANDSTORM", "cz_sandstorm", "cz_sandstorm",    592, 252, 256 },
};

static constexpr int MISSION_COUNT = V_ARRAYSIZE( s_Missions );
static constexpr int CAMPAIGN_MISSION_COUNT = MISSION_COUNT - 6; // 6 bonus missions

static int MissionBit( int mission )
{
	return 1 << ( mission + 1 );
}

// the rows go one under another
static constexpr struct WorldMapInfoRow_t
{
	const char *label;
	const char *suffix;
	int h;
} s_InfoRows[] =
{
	{ "CZERO_Time",         "_TIME",         32  },
	{ "CZERO_Locale",       "_LOCATION",     64  },
	{ "CZERO_Organization", "_ORGANIZATION", 32  },
	{ "CZERO_Overview",     "_OVERVIEW",     240 },
	{ "CZERO_Objective",    "_OBJECTIVE",    160 },
};

static constexpr int INFO_ROW_COUNT = V_ARRAYSIZE( s_InfoRows );

static const char *s_SkillKeys[] = { "CZERO_EasySkill", "CZERO_MediumSkill", "CZERO_HardSkill" };

static constexpr int SKILL_COUNT = V_ARRAYSIZE( s_SkillKeys );

static void InitText( CMenuBaseItem &item, int charSize, unsigned int color, unsigned int align )
{
	// fonts are scaled to the character height when drawn
	item.SetCharSize( QM_SMALLFONT );
	item.charSize = charSize;
	item.colorBase = color;
	item.eTextAlignment = align;
}

static void InitLabel( CMenuAction &label, int x, int y, int w, int h, int charSize, unsigned int color, unsigned int align )
{
	label.iFlags |= QMF_INACTIVE;
	label.SetRect( x, y, w, h );
	InitText( label, charSize, color, align );
}

static void InitButton( CMenuAction &button, const char *text, int x, int y, int w, int h )
{
	button.szName = text;
	button.SetRect( x, y, w, h );
	InitText( button, CHAR_MEDIUM, uiColorWhite, QM_CENTER );
	button.colorFocus = s_ColorHighlight;
	button.eFocusAnimation = QM_HIGHLIGHTIFFOCUS;
	button.bDrawStroke = true;
	button.colorStroke = uiColorWhite;
}

static void SetLocalized( CMenuBaseItem &item, char *key, size_t size, const char *prefix, const char *suffix )
{
	snprintf( key, size, "%s%s", prefix, suffix );
	item.szName = L_GAME( key );
}

static void SetMissionPicture( CMenuBitmap &bitmap, const WorldMapMission_t &mission )
{
	char path[64];

	snprintf( path, sizeof( path ), "gfx/vgui/%s_large.tga", mission.image );
	bitmap.SetPicture( path );
}

/*
=================
CMenuWorldMapPage

Full screen, with the layout of the game in the middle
=================
*/
class CMenuWorldMapPage : public CMenuBaseWindow
{
public:
	typedef CMenuBaseWindow BaseClass;

	CMenuWorldMapPage( const char *name ) : BaseClass( name ) { }

	void Draw() override;

protected:
	void _VidInit() override;
	void LoadTiles( const char *name );

private:
	CMenuBackgroundBitmap::bimage_t m_Tiles[TILES_X][TILES_Y];
	Size m_MapSize;
};

void CMenuWorldMapPage::_VidInit()
{
	SetRect( uiStatic.xOffset + ( uiStatic.width - 1024 ) / 2, uiStatic.yOffset, 1024, 768 );
}

void CMenuWorldMapPage::LoadTiles( const char *name )
{
	int x = 0, y = 0;

	for( int i = 0; i < TILES_X; i++ )
	{
		y = 0;

		for( int j = 0; j < TILES_Y; j++ )
		{
			CMenuBackgroundBitmap::bimage_t &tile = m_Tiles[i][j];
			char path[64];

			snprintf( path, sizeof( path ), "gfx/vgui/%s_%d_%d.tga", name, i, j );
			tile.hImage = EngFuncs::PIC_Load( path );
			tile.coord = Point( x, y );
			tile.size = Size( EngFuncs::PIC_Width( tile.hImage ), EngFuncs::PIC_Height( tile.hImage ));

			y += tile.size.h;
		}

		x += m_Tiles[i][0].size.w;
	}

	m_MapSize = Size( x, y );
}

void CMenuWorldMapPage::Draw()
{
	const int right = m_scPos.x + m_scSize.w;
	const int bottom = m_scPos.y + m_scSize.h;

	// black only around the map, every pixel drawn twice by a fading window gets darker
	UI_FillRect( 0, 0, ScreenWidth, m_scPos.y, uiColorBlack );
	UI_FillRect( 0, bottom, ScreenWidth, ScreenHeight - bottom, uiColorBlack );
	UI_FillRect( 0, m_scPos.y, m_scPos.x, m_scSize.h, uiColorBlack );
	UI_FillRect( right, m_scPos.y, ScreenWidth - right, m_scSize.h, uiColorBlack );

	if( m_MapSize.w > 0 && m_MapSize.h > 0 )
	{
		const float xScale = (float)m_scSize.w / m_MapSize.w;
		const float yScale = (float)m_scSize.h / m_MapSize.h;

		for( int i = 0; i < TILES_X; i++ )
		{
			for( int j = 0; j < TILES_Y; j++ )
				CMenuBackgroundBitmap::DrawBackgroundPiece( m_Tiles[i][j], m_scPos, 0, 0, xScale, yScale );
		}
	}
	else
	{
		UI_FillRect( m_scPos, m_scSize, uiColorBlack );
	}

	BaseClass::Draw();
}

/*
=================
CMenuWorldMapSelect
=================
*/
class CMenuWorldMapSkill : public CMenuAction
{
public:
	void Draw() override;

	int skill;
};

class CMenuWorldMapSelect : public CMenuWorldMapPage
{
public:
	typedef CMenuWorldMapPage BaseClass;

	CMenuWorldMapSelect() : BaseClass( "CMenuWorldMapSelect" ), m_iSkill( 1 ), m_iMission( 0 ) { }

	void Open( int mission );

	int m_iSkill;

protected:
	void _Init() override;

private:
	void StartMission();

	int m_iMission;

	CMenuAction m_MissionLabel;
	CMenuAction m_MissionTitle;
	CMenuAction m_InfoLabels[INFO_ROW_COUNT];
	CMenuAction m_InfoValues[INFO_ROW_COUNT];
	char m_szTitleKey[KEY_SIZE];
	char m_szInfoKeys[INFO_ROW_COUNT][KEY_SIZE];
	CMenuBitmap m_Picture;
	CMenuWorldMapSkill m_Skills[SKILL_COUNT];
	CMenuAction m_Cancel;
	CMenuAction m_Ok;
};

void CMenuWorldMapSkill::Draw()
{
	// the box is centered in a square on the left, text goes after it
	const int h = m_scSize.h;
	const int box = h / 3;
	const int boxX = m_scPos.x + ( h - box ) / 2;
	const int boxY = m_scPos.y + ( h - box ) / 2;
	const bool selected = GetParent( CMenuWorldMapSelect )->m_iSkill == skill;
	unsigned int color = colorBase;

	if( selected )
		color = uiColorWhite;
	else if( this == m_pParent->ItemAtCursor( ))
		color = colorFocus;

	UI_DrawRectangle( boxX, boxY, box, box, uiColorWhite );

	if( selected )
		UI_FillRect( boxX + box / 4, boxY + box / 4, box / 2, box / 2, uiColorWhite );

	UI_DrawString( font, m_scPos.x + h, m_scPos.y, m_scSize.w - h, h, szName, color, m_scChSize, QM_LEFT, ETF_FORCECOL );
}

void CMenuWorldMapSelect::_Init()
{
	LoadTiles( "worldmap2" );

	m_MissionLabel.szName = L_GAME( "CZERO_Mission" );
	InitLabel( m_MissionLabel, 32, 96, 144, 32, CHAR_LARGE, s_ColorLabel, QM_LEFT );
	AddItem( m_MissionLabel );

	InitLabel( m_MissionTitle, 176, 96, 400, 32, CHAR_LARGE, s_ColorTitle, QM_LEFT );
	AddItem( m_MissionTitle );

	int y = 144;

	for( int i = 0; i < INFO_ROW_COUNT; i++ )
	{
		const WorldMapInfoRow_t &row = s_InfoRows[i];

		m_InfoLabels[i].szName = L_GAME( row.label );
		InitLabel( m_InfoLabels[i], 16, y, 176, row.h, CHAR_MEDIUM, s_ColorLabel, QM_TOPRIGHT );
		AddItem( m_InfoLabels[i] );

		InitLabel( m_InfoValues[i], 208, y, 368, row.h, CHAR_MEDIUM, s_ColorText, QM_TOPLEFT );
		AddItem( m_InfoValues[i] );

		y += row.h;
	}

	m_Picture.iFlags |= QMF_INACTIVE;
	m_Picture.SetRenderMode( QM_DRAWTRANS );
	m_Picture.SetRect( 609, 96, 388, 388 );
	AddItem( m_Picture );

	for( int i = 0; i < SKILL_COUNT; i++ )
	{
		CMenuWorldMapSkill &item = m_Skills[i];

		item.skill = i + 1;
		item.szName = L_GAME( s_SkillKeys[i] );
		item.SetRect( 688, 502 + i * 45, 320, SKILL_HEIGHT );
		InitText( item, CHAR_MEDIUM, s_ColorSkill, QM_LEFT );
		item.colorFocus = s_ColorHighlight;
		SET_EVENT_MULTI( item.onReleased,
		{
			CMenuWorldMapSkill *self = (CMenuWorldMapSkill *)pSelf;
			self->GetParent( CMenuWorldMapSelect )->m_iSkill = self->skill;
		});
		AddItem( item );
	}

	InitButton( m_Cancel, L_GAME( "CZERO_Cancel" ), 614, 656, 176, 35 );
	m_Cancel.onReleased = VoidCb( &CMenuWorldMapSelect::Hide );
	AddItem( m_Cancel );

	InitButton( m_Ok, L_GAME( "CZERO_Okay" ), 816, 656, 160, 35 );
	m_Ok.onReleased = VoidCb( &CMenuWorldMapSelect::StartMission );
	AddItem( m_Ok );
}

void CMenuWorldMapSelect::Open( int mission )
{
	const WorldMapMission_t &m = s_Missions[mission];
	const int skill = EngFuncs::GetCvarFloat( "skill" );

	m_iMission = mission;
	m_iSkill = bound( 1, skill, SKILL_COUNT );

	SetLocalized( m_MissionTitle, m_szTitleKey, sizeof( m_szTitleKey ), m.title, "_TITLE" );

	for( int i = 0; i < INFO_ROW_COUNT; i++ )
		SetLocalized( m_InfoValues[i], m_szInfoKeys[i], sizeof( m_szInfoKeys[i] ), m.info, s_InfoRows[i].suffix );

	SetMissionPicture( m_Picture, m );

	Show();
	SetCursorToItem( m_Ok );
}

void CMenuWorldMapSelect::StartMission()
{
	const char *map = s_Missions[m_iMission].map;

	// a running game goes on with the picked mission, as in the original client
	if( CL_IsActive() && EngFuncs::GetCvarFloat( "host_serverstate" ) && EngFuncs::GetCvarFloat( "maxplayers" ) <= 1 )
	{
		EngFuncs::CvarSetValue( "skill", m_iSkill );
		EngFuncs::ClientCmdF( false, "changelevel %s\n", map );
		return;
	}

	UI_PrepareSingleplayerGame( m_iSkill );
	EngFuncs::ClientCmdF( false, "map %s\n", map );
}

/*
=================
CMenuWorldMap
=================
*/
class CMenuWorldMapMission : public CMenuAction
{
public:
	typedef CMenuAction BaseClass;

	bool KeyUp( int key ) override;
	void Draw() override;

	int mission;
	char titleKey[KEY_SIZE];
};

class CMenuWorldMap : public CMenuWorldMapPage
{
public:
	typedef CMenuWorldMapPage BaseClass;

	CMenuWorldMap() : BaseClass( "CMenuWorldMap" ), m_iLocks( 0 ), m_iCurrent( -1 ), m_iInfoMission( -1 ) { }

	void Show() override;
	void Think() override;

	bool IsUnlocked( int mission ) const;
	CImage &Marker( int mission );
	void OpenMission( int mission );

protected:
	void _Init() override;

private:
	void UpdateLocks();
	void ShowMissionInfo( int mission );

	int m_iLocks;
	int m_iCurrent;
	int m_iInfoMission;

	CImage m_Markers[MARKER_COUNT];
	CMenuWorldMapMission m_Missions[MISSION_COUNT];
	CMenuBitmap m_InfoPicture;
	CMenuAction m_InfoLabel;
	CMenuAction m_InfoTitle;
	CMenuAction m_InfoBrief;
	char m_szInfoTitleKey[KEY_SIZE];
	char m_szInfoBriefKey[KEY_SIZE];
	CMenuAction m_Resume;

	CMenuWorldMapSelect m_Select;
};

bool CMenuWorldMapMission::KeyUp( int key )
{
	if( GetParent( CMenuWorldMap )->IsUnlocked( mission ))
		return BaseClass::KeyUp( key );

	if( UI::Key::IsEnter( key ) || ( UI::Key::IsLeftMouse( key ) && FBitSet( iFlags, QMF_HASMOUSEFOCUS )))
	{
		PlayLocalSound( uiStatic.sounds[SND_BUZZ] );
		return true;
	}

	return false;
}

void CMenuWorldMapMission::Draw()
{
	const int h = m_scSize.h;
	const int marker = h * MISSION_MARKER_SIZE / MISSION_HEIGHT;
	const int text = h * MISSION_TEXT_OFFSET / MISSION_HEIGHT;
	const unsigned int color = this == m_pParent->ItemAtCursor() ? colorFocus : colorBase;

	UI_DrawPic( m_scPos.x, m_scPos.y + ( h - marker ) / 2, marker, marker, uiColorWhite, GetParent( CMenuWorldMap )->Marker( mission ));
	UI_DrawString( font, m_scPos.x + text, m_scPos.y, m_scSize.w - text, h, szName, color, m_scChSize, QM_LEFT, ETF_FORCECOL );
}

bool CMenuWorldMap::IsUnlocked( int mission ) const
{
	return ( m_iLocks & MissionBit( mission )) != 0;
}

CImage &CMenuWorldMap::Marker( int mission )
{
	if( !IsUnlocked( mission ))
		return m_Markers[MARKER_LOCKED];

	return m_Markers[mission == m_iCurrent ? MARKER_CURRENT : MARKER_UNLOCKED];
}

void CMenuWorldMap::OpenMission( int mission )
{
	m_Select.Open( mission );
}

void CMenuWorldMap::UpdateLocks()
{
	m_iLocks = EngFuncs::GetCvarFloat( "cl_levellocks" );
	m_iCurrent = -1;

	// the czeror game library sets the bit after the last mission when the ending is reached
	if( m_iLocks & MissionBit( MISSION_COUNT ))
		return;

	// the newest unlocked campaign mission is the current one, as in the original client
	for( int i = CAMPAIGN_MISSION_COUNT - 1; i >= 0; i-- )
	{
		if( IsUnlocked( i ))
		{
			m_iCurrent = i;
			break;
		}
	}
}

void CMenuWorldMap::ShowMissionInfo( int mission )
{
	if( m_iInfoMission == mission )
		return;

	const WorldMapMission_t &m = s_Missions[mission];

	m_iInfoMission = mission;
	SetMissionPicture( m_InfoPicture, m );
	SetLocalized( m_InfoTitle, m_szInfoTitleKey, sizeof( m_szInfoTitleKey ), m.title, "_TITLE" );
	SetLocalized( m_InfoBrief, m_szInfoBriefKey, sizeof( m_szInfoBriefKey ), m.info, "_BRIEF" );
}

void CMenuWorldMap::_Init()
{
	LoadTiles( "worldmap" );

	m_Markers[MARKER_CURRENT].Load( "gfx/vgui/missioncurrent.tga" );
	m_Markers[MARKER_UNLOCKED].Load( "gfx/vgui/missionunlocked.tga" );
	m_Markers[MARKER_LOCKED].Load( "gfx/vgui/missionlocked.tga" );

	for( int i = 0; i < MISSION_COUNT; i++ )
	{
		const WorldMapMission_t &m = s_Missions[i];
		CMenuWorldMapMission &item = m_Missions[i];

		item.mission = i;
		SetLocalized( item, item.titleKey, sizeof( item.titleKey ), m.title, "_TITLE" );
		item.SetRect( m.x, m.y, MISSION_TEXT_OFFSET + m.w, MISSION_HEIGHT );
		InitText( item, CHAR_SMALL, uiColorWhite, QM_LEFT );
		item.colorFocus = s_ColorHighlight;
		SET_EVENT_MULTI( item.onReleased,
		{
			CMenuWorldMapMission *self = (CMenuWorldMapMission *)pSelf;
			self->GetParent( CMenuWorldMap )->OpenMission( self->mission );
		});
		AddItem( item );
	}

	m_InfoPicture.iFlags |= QMF_INACTIVE;
	m_InfoPicture.SetRenderMode( QM_DRAWTRANS );
	m_InfoPicture.SetRect( 32, 598, 139, 139 );
	AddItem( m_InfoPicture );

	m_InfoLabel.szName = L_GAME( "CZERO_Mission" );
	InitLabel( m_InfoLabel, 222, 577, 144, 62, CHAR_LARGE, s_ColorLabel, QM_LEFT );
	AddItem( m_InfoLabel );

	InitLabel( m_InfoTitle, 360, 577, 640, 62, CHAR_LARGE, s_ColorTitle, QM_LEFT );
	AddItem( m_InfoTitle );

	InitLabel( m_InfoBrief, 224, 627, 480, 112, CHAR_MEDIUM, s_ColorText, QM_TOPLEFT );
	AddItem( m_InfoBrief );

	InitButton( m_Resume, L_GAME( "CZERO_Resume" ), 806, 699, 128, 41 );
	m_Resume.onReleased = UI_CloseMenu;
	AddItem( m_Resume );
}

void CMenuWorldMap::Show()
{
	// menu_worldmap command is registered for every game
	if( !UI_IsCZDS( ))
		return;

	// the czeror client opens the map from the game, with the worldmap command and the MapInfo message
	if( !uiStatic.menu.IsActive( ))
		UI_SetActiveMenu( true );

	const bool inGame = CL_IsActive();

	m_Resume.SetGrayed( !inGame );
	m_Resume.colorStroke = inGame ? uiColorWhite : uiColorDkGrey;

	BaseClass::Show();

	UpdateLocks();

	const int mission = m_iCurrent >= 0 ? m_iCurrent : 0;

	SetCursorToItem( m_Missions[mission] );
	ShowMissionInfo( mission );
}

void CMenuWorldMap::Think()
{
	CMenuBaseItem *focused = ItemAtCursor();

	UpdateLocks();

	for( int i = 0; i < MISSION_COUNT; i++ )
	{
		if( focused == &m_Missions[i] )
		{
			ShowMissionInfo( i );
			break;
		}
	}

	BaseClass::Think();
}

ADD_MENU( menu_worldmap, CMenuWorldMap, UI_WorldMap_Menu );
