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
#include "Slider.h"
#include "CheckBox.h"

#define ART_BANNER	  	"gfx/shell/head_vidoptions"
#define ART_GAMMA		"gfx/shell/gamma"

#define LEGACY_VIEWSIZE 0

class CMenuVidOptions : public CMenuFramework
{
private:
	void _Init() override;
	void _VidInit() override;
	void Reload() override;

public:
	CMenuVidOptions() : CMenuFramework( "CMenuVidOptions" ) { }
	void SaveAndPopMenu() override;
	void UpdateConfig();
	void GetConfig();

	int		outlineWidth;

	class CMenuVidPreview : public CMenuBitmap
	{
		void Draw() override;
	} testImage;

	CMenuPicButton	done;

#if LEGACY_VIEWSIZE
	CMenuSlider	screenSize;
#endif
	CMenuSlider	gammaIntensity;
	CMenuSlider	glareReduction;
	CMenuCheckBox   vbo;
	CMenuCheckBox	swwater;
	CMenuCheckBox	overbright;
	CMenuCheckBox	filtering;
	CMenuCheckBox	detailtex;

	HIMAGE		hTestImage;
};

/*
=================
CMenuVidOptions::UpdateConfig
=================
*/
void CMenuVidOptions::UpdateConfig( void )
{
	float val1 = RemapVal( gammaIntensity.GetCurrentValue(), 0.0, 1.0, 1.8, 3.0 );
	float val2 = RemapVal( glareReduction.GetCurrentValue(), 0.0, 1.0, 0.0, 3.0 );
	EngFuncs::CvarSetValue( "gamma", val1 );
	EngFuncs::CvarSetValue( "brightness", val2 );
	EngFuncs::ProcessImage( hTestImage, val1, val2 );
}

void CMenuVidOptions::GetConfig( void )
{
	float val1 = EngFuncs::GetCvarFloat( "gamma" );
	float val2 = EngFuncs::GetCvarFloat( "brightness" );

	gammaIntensity.SetCurrentValue( RemapVal( val1, 1.8f, 3.0f, 0.0f, 1.0f ) );
	glareReduction.SetCurrentValue( RemapVal( val2, 0.0f, 3.0f, 0.0f, 1.0f ) );
	EngFuncs::ProcessImage( hTestImage, val1, val2 );

	gammaIntensity.SetOriginalValue( val1 );
	glareReduction.SetOriginalValue( val2 );
}

void CMenuVidOptions::SaveAndPopMenu( void )
{
#if LEGACY_VIEWSIZE
	screenSize.WriteCvar();
#endif
	detailtex.WriteCvar();
	vbo.WriteCvar();
	swwater.WriteCvar();
	overbright.WriteCvar();
	filtering.WriteCvar();
	// gamma and brightness is already written

	CMenuFramework::SaveAndPopMenu();
}

/*
=================
CMenuVidOptions::Ownerdraw
=================
*/
void CMenuVidOptions::CMenuVidPreview::Draw( )
{
	int		color = 0xFFFF0000; // 255, 0, 0, 255
	int		viewport[4];
	int		viewsize, size, sb_lines;

#if LEGACY_VIEWSIZE
	viewsize = EngFuncs::GetCvarFloat( "viewsize" );
#else
	viewsize = 120;
#endif

	if( viewsize >= 120 )
		sb_lines = 0;	// no status bar at all
	else if( viewsize >= 110 )
		sb_lines = 24;	// no inventory
	else sb_lines = 48;

	size = Q_min( viewsize, 100 );

	viewport[2] = m_scSize.w * size / 100;
	viewport[3] = m_scSize.h * size / 100;

	if( viewport[3] > m_scSize.h - sb_lines )
		viewport[3] = m_scSize.h - sb_lines;
	if( viewport[3] > m_scSize.h )
		viewport[3] = m_scSize.h;

	viewport[2] &= ~7;
	viewport[3] &= ~1;

	viewport[0] = (m_scSize.w - viewport[2]) / 2;
	viewport[1] = (m_scSize.h - sb_lines - viewport[3]) / 2;

	UI_DrawPic( m_scPos.x + viewport[0], m_scPos.y + viewport[1], viewport[2], viewport[3], uiColorWhite, szPic );
	UI_DrawRectangleExt( m_scPos, m_scSize, color, ((CMenuVidOptions*)Parent())->outlineWidth );
}

/*
=================
CMenuVidOptions::Init
=================
*/
void CMenuVidOptions::_Init( void )
{
	hTestImage = EngFuncs::PIC_Load( ART_GAMMA, PIC_KEEP_SOURCE | PIC_EXPAND_SOURCE );

	banner.SetPicture( ART_BANNER );

	testImage.iFlags = QMF_INACTIVE;
	testImage.SetRect( 390, 225, 480, 450 );
	testImage.SetPicture( ART_GAMMA );

	int height = 280;

#if LEGACY_VIEWSIZE
	screenSize.SetNameAndStatus( L( "Screen size" ), L( "Set the screen size" ) );
	screenSize.SetCoord( 72, height );
	screenSize.Setup( 30, 120, 10 );
	screenSize.onChanged = CMenuEditable::WriteCvarCb;

	height += 60;
#endif

	gammaIntensity.SetNameAndStatus( L( "GameUI_Gamma" ), L( "Set gamma value" ) );
	gammaIntensity.SetCoord( 72, height );
	gammaIntensity.Setup( 0.0, 1.0, 0.025 );
	gammaIntensity.onChanged = VoidCb( &CMenuVidOptions::UpdateConfig );
	gammaIntensity.onCvarGet = VoidCb( &CMenuVidOptions::GetConfig );
	height += 60;

	glareReduction.SetCoord( 72, height );
	glareReduction.SetNameAndStatus( L( "GameUI_Brightness" ), L( "Set brightness level" ) );
	glareReduction.Setup( 0, 1.0, 0.025 );
	glareReduction.onChanged = VoidCb( &CMenuVidOptions::UpdateConfig );
	glareReduction.onCvarGet = VoidCb( &CMenuVidOptions::GetConfig );
	height += 60;

	done.SetNameAndStatus( L( "GameUI_OK" ), L( "Go back to the Video Menu" ) );
	done.SetCoord( 72, height );
	done.SetPicture( PC_DONE );
	done.onReleased = VoidCb( &CMenuVidOptions::SaveAndPopMenu );
	height += 60;

	detailtex.SetNameAndStatus( L( "Detail textures" ), L( "Use detail textures if possible" ));
	detailtex.SetCoord( 72, height );
	height += 50;

	vbo.SetNameAndStatus( L( "Use VBO" ), L( "Use new world renderer. Faster, but rarely glitchy" ) );
	vbo.SetCoord( 72, height );
	height += 50;

	swwater.SetNameAndStatus( L( "Water ripples"), L( "Enable water ripple effect, like in software-mode GoldSrc" ));
	swwater.SetCoord( 72, height );
	height += 50;

	overbright.SetNameAndStatus( L( "Overbrights" ), L( "Enable overbrights, makes lighting less flat in bright areas" ));
	overbright.SetCoord( 72, height );
	height += 50;

	filtering.SetNameAndStatus( L( "Texture filtering" ), L( "Enable texture filtering, makes textures smoother" ));
	filtering.SetCoord( 72, height );
	height += 50;

	AddItem( banner );
	AddItem( done );
#if LEGACY_VIEWSIZE
	AddItem( screenSize );
#endif
	AddItem( gammaIntensity );
	AddItem( glareReduction );
	AddItem( detailtex );
	AddItem( vbo );
	AddItem( swwater );
	AddItem( overbright );
	AddItem( filtering );
	AddItem( testImage );

#if LEGACY_VIEWSIZE
	screenSize.LinkCvar( "viewsize" );
#endif

	gammaIntensity.LinkCvar( "gamma" );
	glareReduction.LinkCvar( "brightness" );

	detailtex.LinkCvar( "r_detailtextures" );
	swwater.LinkCvar( "r_ripple" );
	vbo.LinkCvar( "gl_vbo" );
	overbright.LinkCvar( "gl_overbright" );
	// skip filtering.LinkCvar, different names in ref_gl and ref_soft
}

void CMenuVidOptions::_VidInit()
{
	outlineWidth = 2;
	UI_ScaleCoords( NULL, NULL, &outlineWidth, NULL );
}

void CMenuVidOptions::Reload()
{
	CMenuFramework::Reload();
	bool gl_active = !strnicmp( EngFuncs::GetCvarString( "r_refdll_loaded" ), "gl", 2 );
	bool soft_active = !stricmp( EngFuncs::GetCvarString( "r_refdll_loaded" ), "soft" );

	detailtex.SetGrayed( !gl_active );
	detailtex.SetInactive( !gl_active );

	vbo.SetGrayed( !gl_active );
	vbo.SetInactive( !gl_active );

	swwater.SetGrayed( !gl_active );
	swwater.SetInactive( !gl_active );

	overbright.SetGrayed( !gl_active );
	overbright.SetInactive( !gl_active );

	if( soft_active || gl_active )
	{
		filtering.SetGrayed( false );
		filtering.SetInactive( false );

		if( soft_active )
		{
			// no need to invert, 0 disables filtering
			filtering.LinkCvar( "sw_texfilt" );
		}
		else
		{
			SET_EVENT_MULTI( filtering.onCvarGet,
			{
				CMenuCheckBox *cb = (CMenuCheckBox *)pSelf;

				if( EngFuncs::GetCvarFloat( cb->CvarName( )))
					cb->bChecked = false;
				else cb->bChecked = true;
			});
			SET_EVENT_MULTI( filtering.onCvarWrite,
			{
				CMenuCheckBox *cb = (CMenuCheckBox *)pSelf;
				if( cb->bChecked )
					EngFuncs::CvarSetValue( cb->CvarName(), 0.0f );
				else EngFuncs::CvarSetValue( cb->CvarName(), 1.0f );
			});

			filtering.LinkCvar( "gl_texture_nearest" );
		}
	}
	else
	{
		filtering.SetGrayed( true );
		filtering.SetInactive( true );
	}
}

ADD_MENU( menu_vidoptions, CMenuVidOptions, UI_VidOptions_Menu );
