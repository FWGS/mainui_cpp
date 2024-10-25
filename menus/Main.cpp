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
#include "gameinfo.h"

#define ART_MINIMIZE_N	"gfx/shell/min_n"
#define ART_MINIMIZE_F	"gfx/shell/min_f"
#define ART_MINIMIZE_D	"gfx/shell/min_d"
#define ART_CLOSEBTN_N	"gfx/shell/cls_n"
#define ART_CLOSEBTN_F	"gfx/shell/cls_f"
#define ART_CLOSEBTN_D	"gfx/shell/cls_d"

#define ART_LOGO      "resource/logo.tga"

#define ART_BLIP1     "resource/logo_blip.tga"
#define ART_BLIP2     "resource/logo_blip2.tga"
#define ART_BLIP_NUM  2

#define ART_BLUR      "resource/logo_big_blurred_%i"
#define ART_BLUR_BLIP "resource/logo_big_blurred_blip_%i"
#define ART_BLUR_NUM  4

class CMenuMain: public CMenuFramework
{
public:
	CMenuMain() : CMenuFramework( "CMenuMain" ) { }

	bool KeyDown( int key ) override;

private:
	void _Init() override;
	void _VidInit( ) override;
	void Think() override;

	void VidInit(bool connected);

	void QuitDialog( void *pExtra = NULL );
	void DisconnectCb();
	void DisconnectDialogCb();
	void HazardCourseDialogCb();
	void HazardCourseCb();

	CMenuPicButton	console;
	class CMenuMainBanner : public CMenuBannerBitmap
	{
	public:
		virtual void Init() override;
		virtual void VidInit() override;
		virtual void Draw() override;
		virtual void Think() override;

		void AnimatedTitleDraw();

		void RandomizeGoalTime( float time );
		void RandomizeSpeed( float initial_speed );
		void RandomizeBlipTime( float time );
		void RandomizeBlip( void );
	private:
		bool useAnimatedTitle;
		float scale;

		CImage logo;
		CImage logoBlip[ART_BLIP_NUM];

		CImage logoBlur[ART_BLUR_NUM];
		CImage logoBlurBlip[ART_BLUR_NUM];

		int   m_nLogoImageXMin;
		int   m_nLogoImageXMax;
		int   m_nLogoImageXGoal;
		float m_flPrevFrameTime;
		float m_flTimeLogoNewGoal;
		float m_flTimeLogoNewGoalMin;
		float m_flTimeLogoNewGoalMax;
		float m_flTimeUntilLogoBlipMin;
		float m_flTimeUntilLogoBlipMax;
		float m_flTimeLogoBlip;
		int   m_nLogoBlipType;
		int   m_nLogoImageY;
		float m_fLogoImageX;
		float m_fLogoSpeedMin;
		float m_fLogoSpeedMax;
		float m_fLogoSpeed;
		int   m_nLogoBGOffsetX;
		int   m_nLogoBGOffsetY;

		enum LogoBlip_t
		{
			E_LOGO_BLIP_BOTH,
			E_LOGO_BLIP_JUST_LOGO,
			E_LOGO_BLIP_JUST_BG,
			E_LOGO_BLIP_STAGGER,
			E_LOGO_BLIP_BOTH_SHOW_BLIP_LOGO_ONLY
		} m_nNextLogoBlipType;

		// not referenced
		bool drawBlip[ART_BLIP_NUM];
		bool drawBgBlip;
	} banner;

	CMenuPicButton	resumeGame;
	CMenuPicButton	disconnect;
	CMenuPicButton	newGame;
	CMenuPicButton	hazardCourse;
	CMenuPicButton	configuration;
	CMenuPicButton	saveRestore;
	CMenuPicButton	multiPlayer;
	CMenuPicButton	customGame;
	CMenuPicButton	previews;
	CMenuPicButton	quit;

	// buttons on top right. Maybe should be drawn if fullscreen == 1?
	CMenuBitmap	minimizeBtn;
	CMenuBitmap	quitButton;

	// quit dialog
	CMenuYesNoMessageBox dialog;

	bool bTrainMap;
	bool bCustomGame;
};

void CMenuMain::CMenuMainBanner::Init()
{
	useAnimatedTitle = false;

	if( !FBitSet( gMenu.m_gameinfo.flags, GFL_ANIMATED_TITLE ))
		return;

	logo.Load( ART_LOGO );
	if( !logo.IsValid( ))
		return;

	logoBlip[0].Load( ART_BLIP1 );
	logoBlip[1].Load( ART_BLIP2 );
	for( int i = 0; i < ART_BLIP_NUM; i++ )
	{
		if( !logoBlip[i].IsValid( ))
			return;
	}

	for( int i = 0; i < ART_BLUR_NUM; i++ )
	{
		char name[256];
		snprintf( name, sizeof( name ), ART_BLUR, i );
		logoBlur[i].Load( name );
		if( !logoBlur[i].IsValid( ))
			return;

		snprintf( name, sizeof( name ), ART_BLUR_BLIP, i );
		logoBlurBlip[i].Load( name );
		if( !logoBlurBlip[i].IsValid( ))
			return;
	}

	useAnimatedTitle = true;
}

void CMenuMain::CMenuMainBanner::RandomizeGoalTime( float time )
{
	const float randval = rand() / (float)RAND_MAX;
	const float range = ( m_flTimeLogoNewGoalMax - m_flTimeLogoNewGoalMin );

	m_flTimeLogoNewGoal = time + randval * range;
}

void CMenuMain::CMenuMainBanner::RandomizeSpeed( float initial_speed )
{
	const float range = m_fLogoSpeedMax - m_fLogoSpeedMin;
	const float randval = rand() / (float)RAND_MAX;

	m_fLogoSpeed = initial_speed + randval * range;
}

void CMenuMain::CMenuMainBanner::RandomizeBlipTime( float time )
{
	const float range = m_flTimeUntilLogoBlipMax - m_flTimeUntilLogoBlipMin;
	const float randval = rand() / (float)RAND_MAX;

	m_flTimeLogoBlip = time + randval * range;
}

void CMenuMain::CMenuMainBanner::RandomizeBlip()
{
	// why tho
	const float randval = rand() / (float)RAND_MAX;

	if( randval >= 0.6f )
		m_nLogoBlipType = E_LOGO_BLIP_BOTH;
	else if( randval > 0.4f )
		m_nLogoBlipType = E_LOGO_BLIP_STAGGER;
	else if( randval >= 0.25f )
		m_nLogoBlipType = E_LOGO_BLIP_JUST_LOGO;
	else if( randval > 0.1f )
		m_nLogoBlipType = E_LOGO_BLIP_BOTH_SHOW_BLIP_LOGO_ONLY;
	else
		m_nLogoBlipType = E_LOGO_BLIP_JUST_BG;
}

void CMenuMain::CMenuMainBanner::VidInit()
{
	if( !useAnimatedTitle )
		return;

	// in hardware mode, it's scaled by screen width
	// in software mode, scale must be 1.0f
	scale = ScreenWidth / 1024.0f;

	Size logoSz = EngFuncs::PIC_Size(logo.Handle());

	logoSz = logoSz * scale;
	m_nLogoImageXMin = (ScreenWidth / 2.0f - logoSz.w / 2.0f) - scale * 88.0f;
	m_nLogoImageXMax = (ScreenWidth / 2.0f - logoSz.w / 2.0f) + scale * 32.0f;
	m_nLogoImageXGoal = m_nLogoImageXMax;
	m_nNextLogoBlipType = E_LOGO_BLIP_BOTH;
	m_flTimeLogoNewGoalMin = 0.4f;
	m_flTimeLogoNewGoalMax = 1.95f;
	m_flTimeUntilLogoBlipMin = 0.5f;
	m_flTimeUntilLogoBlipMax = 1.4f;

	m_fLogoImageX = m_nLogoImageXMin + ((m_nLogoImageXMax - m_nLogoImageXMin) / 2.0f);

#if 0 // original GameUI logic
	if( ScreenHeight > 600 )
		m_nLogoImageY = 60 * uiStatic.scaleY;
	else m_nLogoImageY = 20 * uiStatic.scaleY;
#else // similar to logo.avi position
	m_nLogoImageY = ( 70 / 480.0f ) * 768.0f * uiStatic.scaleY;
#endif

	m_nLogoBGOffsetX = scale * -320.0f;
	m_nLogoBGOffsetY = m_nLogoImageY - scale * 54.0f;
	m_fLogoSpeedMin = scale * 13.0f;
	m_fLogoSpeedMax = scale * 65.0f;
	m_flPrevFrameTime = gpGlobals->time;

	RandomizeGoalTime( gpGlobals->time + m_flTimeLogoNewGoalMin );
	RandomizeSpeed( m_fLogoSpeedMin );
	RandomizeBlipTime( gpGlobals->time + m_flTimeUntilLogoBlipMin );
}

void CMenuMain::CMenuMainBanner::Draw()
{
	if( EngFuncs::ClientInGame() && EngFuncs::GetCvarFloat( "ui_renderworld" ) != 0.0f )
		return;

	if( useAnimatedTitle )
		AnimatedTitleDraw();

	if( !CMenuBackgroundBitmap::ShouldDrawLogoMovie( ))
		return; // no logos for steam background

	if( EngFuncs::GetLogoLength() <= 0.05f || EngFuncs::GetLogoWidth() <= 32 )
		return;	// don't draw stub logo (GoldSrc rules)

	float	logoWidth, logoHeight, logoPosY;
	float	scaleX, scaleY;

	scaleX = ScreenWidth / 640.0f;
	scaleY = ScreenHeight / 480.0f;

	// a1ba: multiply by height scale to look better on widescreens
	logoWidth = EngFuncs::GetLogoWidth() * scaleX;
	logoHeight = EngFuncs::GetLogoHeight() * scaleY * uiStatic.scaleY;
	logoPosY = 70 * scaleY * uiStatic.scaleY;	// 70 it's empirically determined value (magic number)

	EngFuncs::DrawLogo( "logo.avi", 0, logoPosY, logoWidth, logoHeight );
}

void CMenuMain::CMenuMainBanner::Think()
{
	if( !useAnimatedTitle )
		return;

	// m_fLogoImageX = uiStatic.cursorX;
	// return;

	float deltatime = gpGlobals->time - m_flPrevFrameTime;
	deltatime = bound( 0.0001, deltatime, 0.3 );

	m_flPrevFrameTime = gpGlobals->time;

	float deltaX = deltatime * m_fLogoSpeed;

	if( m_fLogoImageX >= m_nLogoImageXGoal )
	{
		m_fLogoImageX -= deltaX;

		if( m_fLogoImageX <= m_nLogoImageXGoal || gpGlobals->time >= m_flTimeLogoNewGoal )
		{
			RandomizeGoalTime( gpGlobals->time + m_flTimeLogoNewGoalMin );
			RandomizeSpeed( m_fLogoSpeedMin );

			m_nLogoImageXGoal = m_nLogoImageXMax;
		}
	}
	else
	{
		m_fLogoImageX += deltaX;

		if( m_fLogoImageX >= m_nLogoImageXGoal || gpGlobals->time >= m_flTimeLogoNewGoal )
		{
			RandomizeGoalTime( gpGlobals->time + m_flTimeLogoNewGoalMin );
			RandomizeSpeed( m_fLogoSpeedMin );

			m_nLogoImageXGoal = m_nLogoImageXMin;
		}
	}

	drawBlip[0] = drawBlip[1] = false;
	drawBgBlip = false;

	if( gpGlobals->time > m_flTimeLogoBlip )
	{
		RandomizeBlipTime( gpGlobals->time + m_flTimeUntilLogoBlipMin );
		RandomizeBlip();
	}
	else
	{
		float timeToBlip;

		if( m_nLogoBlipType == E_LOGO_BLIP_STAGGER )
			timeToBlip = 0.09f; // TODO
		else if( m_nLogoBlipType == E_LOGO_BLIP_BOTH_SHOW_BLIP_LOGO_ONLY )
			timeToBlip = 0.07f;
		else
			timeToBlip = 0.06f;

		if( gpGlobals->time + timeToBlip > m_flTimeLogoBlip )
		{
			switch( m_nLogoBlipType )
			{
			case E_LOGO_BLIP_BOTH:
				drawBgBlip  = true;
				drawBlip[0] = true;
				break;
			case E_LOGO_BLIP_JUST_LOGO:
				drawBlip[0] = true;
				break;
			case E_LOGO_BLIP_JUST_BG:
				drawBgBlip  = true;
				break;
			case E_LOGO_BLIP_STAGGER:
				drawBgBlip  = true;
				drawBlip[0] = gpGlobals->time + ( timeToBlip / 2.0f ) <= m_flTimeLogoBlip;
				break;
			case E_LOGO_BLIP_BOTH_SHOW_BLIP_LOGO_ONLY:
				drawBgBlip  = true;
				drawBlip[0] = true;
				drawBlip[1] = true;
				break;
			}
		}
	}
}

void CMenuMain::CMenuMainBanner::AnimatedTitleDraw()
{
	Point logoPt( m_fLogoImageX, m_nLogoImageY );
	Size logoSz = EngFuncs::PIC_Size( logo.Handle( )) * scale;

	EngFuncs::PIC_Set( logo.Handle(), 255, 255, 255 );
	EngFuncs::PIC_DrawTrans( logoPt, logoSz );

	for( int i = 0; i < ART_BLIP_NUM; i++ )
	{
		if( drawBlip[i] )
		{
			EngFuncs::PIC_Set( logoBlip[i], 255, 255, 255 );
			EngFuncs::PIC_DrawTrans( logoPt, logoSz );
		}
	}

#if 0
	// makes big logo centered but in original it's a bit offset
	logoPt.x = m_fLogoImageX + m_nLogoBGOffsetX;
#else
	logoPt.x = 0;
#endif

	float t1 = m_nLogoImageXMax - m_nLogoImageXMin;
	float t2 = m_nLogoBGOffsetX - t1 * 1.85f;
	float t3 = t2 + ( t1 * 3.7f ) * (( m_fLogoImageX - m_nLogoImageXMin ) / t1 );

	logoPt.x += t3;
	logoPt.y = m_nLogoBGOffsetY;

	const CImage *images = drawBgBlip ? logoBlurBlip : logoBlur;
	for( int i = 0; i < ART_BLUR_NUM; i++ )
	{
		logoSz = EngFuncs::PIC_Size( logoBlur[i].Handle( )) * scale;

		EngFuncs::PIC_Set( images[i].Handle( ), 255, 255, 255 );
		EngFuncs::PIC_DrawTrans( logoPt, logoSz );

		logoPt.x += logoSz.w;
	}
}

void CMenuMain::QuitDialog(void *pExtra)
{
	if( CL_IsActive() && EngFuncs::GetCvarFloat( "host_serverstate" ) && EngFuncs::GetCvarFloat( "maxplayers" ) == 1.0f )
		dialog.SetMessage( L( "StringsList_235" ) );
	else
		dialog.SetMessage( L( "GameUI_QuitConfirmationText" ) );

	dialog.onPositive.SetCommand( FALSE, "quit\n" );
	dialog.Show();
}

void CMenuMain::DisconnectCb()
{
	EngFuncs::ClientCmd( FALSE, "disconnect\n" );
	VidInit( false );
}

void CMenuMain::DisconnectDialogCb()
{
	dialog.onPositive = VoidCb( &CMenuMain::DisconnectCb );
	dialog.SetMessage( L( "Really disconnect?" ) );
	dialog.Show();
}

void CMenuMain::HazardCourseDialogCb()
{
	dialog.onPositive = VoidCb( &CMenuMain::HazardCourseCb );;
	dialog.SetMessage( L( "StringsList_234" ) );
	dialog.Show();
}

/*
=================
CMenuMain::Key
=================
*/
bool CMenuMain::KeyDown( int key )
{
	if( UI::Key::IsEscape( key ) )
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
		return true;
	}
	return CMenuFramework::KeyDown( key );
}

/*
=================
UI_Main_HazardCourse
=================
*/
void CMenuMain::HazardCourseCb()
{
	if( EngFuncs::GetCvarFloat( "host_serverstate" ) && EngFuncs::GetCvarFloat( "maxplayers" ) > 1 )
		EngFuncs::HostEndGame( "end of the game" );

	EngFuncs::CvarSetValue( "skill", 1.0f );
	EngFuncs::CvarSetValue( "deathmatch", 0.0f );
	EngFuncs::CvarSetValue( "teamplay", 0.0f );
	EngFuncs::CvarSetValue( "pausable", 1.0f ); // singleplayer is always allowing pause
	EngFuncs::CvarSetValue( "coop", 0.0f );
	EngFuncs::CvarSetValue( "maxplayers", 1.0f ); // singleplayer

	EngFuncs::PlayBackgroundTrack( NULL, NULL );

	EngFuncs::ClientCmd( FALSE, "hazardcourse\n" );
}

void CMenuMain::_Init( void )
{
	if( gMenu.m_gameinfo.trainmap[0] && stricmp( gMenu.m_gameinfo.trainmap, gMenu.m_gameinfo.startmap ) != 0 )
		bTrainMap = true;
	else bTrainMap = false;

	if( EngFuncs::GetCvarFloat( "host_allow_changegame" ))
		bCustomGame = true;
	else bCustomGame = false;

	// console
	console.SetNameAndStatus( L( "GameUI_Console" ), L( "Show console" ) );
	console.iFlags |= QMF_NOTIFY;
	console.SetPicture( PC_CONSOLE );
	console.SetVisibility( gpGlobals->developer );
	SET_EVENT_MULTI( console.onReleased,
	{
		UI_SetActiveMenu( FALSE );
		EngFuncs::KEY_SetDest( KEY_CONSOLE );
	});

	resumeGame.SetNameAndStatus( L( "GameUI_GameMenu_ResumeGame" ), L( "StringsList_188" ) );
	resumeGame.SetPicture( PC_RESUME_GAME );
	resumeGame.iFlags |= QMF_NOTIFY;
	resumeGame.onReleased = UI_CloseMenu;

	disconnect.SetNameAndStatus( L( "GameUI_GameMenu_Disconnect" ), L( "Disconnect from server" ) );
	disconnect.SetPicture( PC_DISCONNECT );
	disconnect.iFlags |= QMF_NOTIFY;
	disconnect.onReleased = VoidCb( &CMenuMain::DisconnectDialogCb );

	newGame.SetNameAndStatus( L( "GameUI_NewGame" ), L( "StringsList_189" ) );
	newGame.SetPicture( PC_NEW_GAME );
	newGame.iFlags |= QMF_NOTIFY;
	newGame.onReleased = UI_NewGame_Menu;

	hazardCourse.SetNameAndStatus( L( "GameUI_TrainingRoom" ), L( "StringsList_190" ) );
	hazardCourse.SetPicture( PC_HAZARD_COURSE );
	hazardCourse.iFlags |= QMF_NOTIFY;
	hazardCourse.onReleasedClActive = VoidCb( &CMenuMain::HazardCourseDialogCb );
	hazardCourse.onReleased = VoidCb( &CMenuMain::HazardCourseCb );

	multiPlayer.SetNameAndStatus( L( "GameUI_Multiplayer" ), L( "StringsList_198" ) );
	multiPlayer.SetPicture( PC_MULTIPLAYER );
	multiPlayer.iFlags |= QMF_NOTIFY;
	multiPlayer.onReleased = UI_MultiPlayer_Menu;

	configuration.SetNameAndStatus( L( "GameUI_Options" ), L( "StringsList_193" ) );
	configuration.SetPicture( PC_CONFIG );
	configuration.iFlags |= QMF_NOTIFY;
	configuration.onReleased = UI_Options_Menu;

	saveRestore.iFlags |= QMF_NOTIFY;

	customGame.SetNameAndStatus( L( "GameUI_ChangeGame" ), L( "StringsList_530" ) );
	customGame.SetPicture( PC_CUSTOM_GAME );
	customGame.iFlags |= QMF_NOTIFY;
	customGame.onReleased = UI_CustomGame_Menu;

	previews.SetNameAndStatus( L( "Previews" ), L( "StringsList_400" ) );
	previews.SetPicture( PC_PREVIEWS );
	previews.iFlags |= QMF_NOTIFY;
	SET_EVENT( previews.onReleased, EngFuncs::ShellExecute( MenuStrings[ IDS_MEDIA_PREVIEWURL ], NULL, false ) );

	quit.SetNameAndStatus( L( "GameUI_GameMenu_Quit" ), L( "GameUI_QuitConfirmationText" ) );
	quit.SetPicture( PC_QUIT );
	quit.iFlags |= QMF_NOTIFY;
	quit.onReleased = MenuCb( &CMenuMain::QuitDialog );

	quitButton.SetPicture( ART_CLOSEBTN_N, ART_CLOSEBTN_F, ART_CLOSEBTN_D );
	quitButton.iFlags = QMF_MOUSEONLY;
	quitButton.eFocusAnimation = QM_HIGHLIGHTIFFOCUS;
	quitButton.onReleased = MenuCb( &CMenuMain::QuitDialog );

	minimizeBtn.SetPicture( ART_MINIMIZE_N, ART_MINIMIZE_F, ART_MINIMIZE_D );
	minimizeBtn.iFlags = QMF_MOUSEONLY;
	minimizeBtn.eFocusAnimation = QM_HIGHLIGHTIFFOCUS;
	minimizeBtn.onReleased.SetCommand( FALSE, "minimize\n" );

	if ( gMenu.m_gameinfo.gamemode == GAME_MULTIPLAYER_ONLY || gMenu.m_gameinfo.startmap[0] == 0 )
		newGame.SetGrayed( true );

	if ( gMenu.m_gameinfo.gamemode == GAME_SINGLEPLAYER_ONLY )
		multiPlayer.SetGrayed( true );

	if ( gMenu.m_gameinfo.gamemode == GAME_MULTIPLAYER_ONLY )
	{
		saveRestore.SetGrayed( true );
		hazardCourse.SetGrayed( true );
	}

	// too short execute string - not a real command
	if( strlen( MenuStrings[IDS_MEDIA_PREVIEWURL] ) <= 3 )
	{
		previews.SetGrayed( true );
	}

	// server.dll needs for reading savefiles or startup newgame
	if( !EngFuncs::CheckGameDll( ))
	{
		saveRestore.SetGrayed( true );
		hazardCourse.SetGrayed( true );
		newGame.SetGrayed( true );
	}

	dialog.Link( this );

	AddItem( banner );
	AddItem( console );
	AddItem( disconnect );
	AddItem( resumeGame );
	AddItem( newGame );

	if ( bTrainMap )
		AddItem( hazardCourse );

	AddItem( saveRestore );
	AddItem( configuration );
	AddItem( multiPlayer );

	if ( bCustomGame )
		AddItem( customGame );

	AddItem( previews );
	AddItem( quit );
	AddItem( minimizeBtn );
	AddItem( quitButton );
}

/*
=================
UI_Main_Init
=================
*/
void CMenuMain::VidInit( bool connected )
{
	int hoffset = ( 70 / 640.0 ) * 1024.0;

	// in original menu Previews is located at specific point
	int previews_voffset = ( 404 / 480.0 ) * 768.0;

	// no visible console button gap
	int ygap = (( 404 - 373 ) / 480.0 ) * 768.0;

	// statically positioned items
	minimizeBtn.SetRect( uiStatic.width - 72, 13, 32, 32 );
	quitButton.SetRect( uiStatic.width - 36, 13, 32, 32 );

	previews.SetCoord( hoffset, previews_voffset );
	quit.SetCoord( hoffset, previews_voffset + ygap );

	// let's start calculating positions
	int yoffset = previews_voffset - ygap;

	if( bCustomGame )
	{
		customGame.SetCoord( hoffset, yoffset );
		yoffset -= ygap;
	}

	multiPlayer.SetCoord( hoffset, yoffset );
	yoffset -= ygap;

	bool single = gpGlobals->maxClients < 2;

	saveRestore.SetCoord( hoffset, yoffset );
	yoffset -= ygap;

	configuration.SetCoord( hoffset, yoffset );
	yoffset -= ygap;

	if( bTrainMap )
	{
		hazardCourse.SetCoord( hoffset, yoffset );
		yoffset -= ygap;
	}

	newGame.SetCoord( hoffset, yoffset );
	yoffset -= ygap;

	if( connected )
	{
		resumeGame.SetCoord( hoffset, yoffset );
		yoffset -= ygap;

		if( !single )
		{
			disconnect.SetCoord( hoffset, yoffset );
			yoffset -= ygap;
		}
	}

	console.SetCoord( hoffset, yoffset );
	yoffset -= ygap;

	// now figure out what's visible
	resumeGame.SetVisibility( connected );
	disconnect.SetVisibility( connected && !single );

	if( connected && single )
	{
		saveRestore.SetNameAndStatus( L( "Save\\Load Game" ), L( "StringsList_192" ) );
		saveRestore.SetPicture( PC_SAVE_LOAD_GAME );
		saveRestore.onReleased = UI_SaveLoad_Menu;
	}
	else
	{
		saveRestore.SetNameAndStatus( L( "GameUI_LoadGame" ), L( "StringsList_191" ) );
		saveRestore.SetPicture( PC_LOAD_GAME );
		saveRestore.onReleased = UI_LoadGame_Menu;
	}
}

void CMenuMain::_VidInit()
{
	VidInit( CL_IsActive() );
}

void CMenuMain::Think()
{
	if( gpGlobals->developer )
	{
		if( !console.IsVisible( ))
			console.Show();
	}
	else
	{
		if( console.IsVisible( ))
			console.Hide();
	}

	CMenuFramework::Think();
}

ADD_MENU( menu_main, CMenuMain, UI_Main_Menu );
