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
#include "CheckBox.h"
#include "Slider.h"
#include "Field.h"
#include "SpinControl.h"
#include "YesNoMessageBox.h"
#include "PlayerModelView.h"
#include "StringArrayModel.h"
#include "StringVectorModel.h"

#define ART_BANNER		"gfx/shell/head_customize"

#define MAX_PLAYERMODELS	100

static const byte Orange[] = { 255, 120,  24 };
static const byte Yellow[] = { 225, 180,  24 };
static const byte Blue[]   = {   0,  60, 255 };
static const byte Ltblue[] = {   0, 167, 255 };
static const byte Green[]  = {   0, 167,   0 };
static const byte Red[]    = { 255,  43,   0 };
static const byte Brown[]  = { 123,  73,   0 };
static const byte Ltgray[] = { 100, 100, 100 };
static const byte Dkgray[] = {  36,  36,  36 };
static const byte Rainbow[] = {
	0xE4, 0x03, 0x03,
	0xFF, 0x8C, 0x00,
	0xFF, 0xED, 0x00,
	0x00, 0x80, 0x26,
	0x24, 0x40, 0x8E,
	0x73, 0x29, 0x82,
};
static const byte Lesbian[] = {
	0xD5, 0x2D, 0x00,
	0xEF, 0x76, 0x27,
	0xFF, 0x9A, 0x56,
	0xFF, 0xFF, 0xFF,
	0xD1, 0x62, 0xA4,
	0xB5, 0x56, 0x90,
	0xA3, 0x02, 0x62,
};
static const byte Gay[] = {
	0x07, 0x8D, 0x70,
	0x26, 0xCE, 0xAA,
	0x98, 0xE8, 0xC1,
	0xFF, 0xFF, 0xFF,
	0x7B, 0xAD, 0xE2,
	0x50, 0x49, 0xCC,
	0x3D, 0x1A, 0x78,
};
static const byte Bi[] = {
	0xD6, 0x02, 0x70,
	0xD6, 0x02, 0x70,
	0x9B, 0x4F, 0x96,
	0x00, 0x38, 0xA8,
	0x00, 0x38, 0xA8,
};
static const byte Trans[] = {
	0x5B, 0xCE, 0xFA,
	0xF5, 0xA9, 0xB8,
	0xFF, 0xFF, 0xFF,
	0xF5, 0xA9, 0xB8,
	0x5B, 0xCE, 0xFA,
};
static const byte Pan[] = {
	0xFF, 0x21, 0x8C,
	0xFF, 0xD8, 0x00,
	0x21, 0xB1, 0xFF,
};
static const byte Enby[] = {
	0xFC, 0xF4, 0x34,
	0xFF, 0xFF, 0xFF,
	0x9C, 0x59, 0xD1,
	0x2C, 0x2C, 0x2C,
};

#define FLAG_L( str, x ) str, x, sizeof( x ) / 3
#define FLAG( x ) FLAG_L( #x, x )

// TODO: Get rid of this hardcoded mess
// allow user to set whatever they want
// through UI or some config lst file
static const struct logo_color_t
{
	const char *name;
	const byte *rgb;
	int stripes;
} g_LogoColors[] =
{
{ "FullColor", 0, 0 },
{ FLAG_L( "#Valve_Orange", Orange ) }, // L( "Valve_Orange" )
{ FLAG_L( "#Valve_Yellow", Yellow ) }, // L( "Valve_Yellow" )
{ FLAG_L( "#Valve_Blue",   Blue )   }, // L( "Valve_Blue" )
{ FLAG_L( "#Valve_Ltblue", Ltblue ) }, // L( "Valve_Ltblue" )
{ FLAG_L( "#Valve_Green",  Green )  }, // L( "Valve_Green" )
{ FLAG_L( "#Valve_Red",    Red )    }, // L( "Valve_Red" )
{ FLAG_L( "#Valve_Brown",  Brown )  }, // L( "Valve_Brown" )
{ FLAG_L( "#Valve_Ltgray", Ltgray ) }, // L( "Valve_Ltgray" )
{ FLAG_L( "#Valve_Dkgray", Dkgray ) }, // L( "Valve_Dkgray" )
{ FLAG( Rainbow ) },
{ FLAG( Lesbian ) },
{ FLAG( Gay )     },
{ FLAG( Bi )      },
{ FLAG( Trans )   },
{ FLAG( Pan )     },
{ FLAG( Enby )    },
};

class CMenuPlayerSetup : public CMenuFramework
{
private:
	void _Init() override;
	void Reload() override;
public:
	CMenuPlayerSetup() : CMenuFramework( "CMenuPlayerSetup" ), msgBox( true ) { }

	void SetConfig();
	void UpdateModel();
	void UpdateLogo();
	void ApplyColorToImagePreview();
	void ApplyColorToLogoPreview();
	void WriteNewLogo();
	void SaveAndPopMenu() override;

	class CModelListModel : public CStringArrayModel
	{
	public:
		CModelListModel() : CStringArrayModel( (const char *)models, CS_SIZE, 0 ) {}
		void Update();

	private:
		char models[MAX_PLAYERMODELS][CS_SIZE];
	} modelsModel;

	class CLogosListModel : public CStringVectorModel
	{
	public:
		void Update() override;

		int GetFullPath( char *buf, size_t size, int pos )
		{
			const char *file, *ext;

			file = Element( pos ).String();
			ext = IsPng( pos ) ? "png" : "bmp";

			return snprintf( buf, size, "logos/%s.%s", file, ext );
		}

		bool IsPng( int pos )
		{
			return m_isPngs[pos];
		}

	private:
		CUtlVector<bool> m_isPngs;
	} logosModel;

	CMenuPlayerModelView	view;

	CMenuCheckBox	showModels;
	CMenuCheckBox	hiModels;
	CMenuSlider	topColor;
	CMenuSlider	bottomColor;

	CMenuField	name;
	CMenuSpinControl	model;

	class CMenuLogoPreview : public CMenuBaseItem
	{
	public:
		virtual void Draw();
		const logo_color_t *color;
		HIMAGE hImage;
	} logoImage;

	CMenuSpinControl	logo;
	CMenuSpinControl	logoColor;

	CMenuYesNoMessageBox msgBox;

	bool hideModels, hideLogos;
};

void CMenuPlayerSetup::CMenuLogoPreview::Draw()
{
	if( !hImage )
	{
		// draw the background
		UI_FillRect( m_scPos, m_scSize, uiPromptBgColor );

		UI_DrawString( font, m_scPos, m_scSize, L( "No logo" ), colorBase, m_scChSize, QM_CENTER, ETF_SHADOW );
	}
	else if( color->stripes == 0 )
	{
		EngFuncs::PIC_Set( hImage, 255, 255, 255 );
		EngFuncs::PIC_DrawTrans( m_scPos, m_scSize );
	}
	else
	{
		const Size img_sz = EngFuncs::PIC_Size( hImage );
		Size  ui_sz = m_scSize;
		wrect_t rc = { 0 };

		rc.right = img_sz.w;
		rc.bottom = img_sz.h;

		double texture_pixels_per_stripe = img_sz.h / (double)color->stripes;
		double screen_pixels_per_stripe  = ui_sz.h  / (double)color->stripes;

		ui_sz.h = round( screen_pixels_per_stripe );

		for( int i = 0; i < color->stripes; i++ )
		{
			wrect_t rc2 = rc;
			Point ui_pt;

			rc2.top    = round( i * texture_pixels_per_stripe );
			rc2.bottom = round(( i + 1 ) * texture_pixels_per_stripe );

			ui_pt.x = m_scPos.x;
			ui_pt.y = m_scPos.y + round( i * screen_pixels_per_stripe );

			EngFuncs::PIC_Set( hImage, color->rgb[i * 3 + 0], color->rgb[i * 3 + 1], color->rgb[i * 3 + 2] );
			EngFuncs::PIC_DrawTrans( ui_pt, ui_sz, &rc2 );
		}
	}

	// draw the rectangle
	if( eFocusAnimation == QM_HIGHLIGHTIFFOCUS && IsCurrentSelected() )
		UI_DrawRectangle( m_scPos, m_scSize, uiInputTextColor );
	else
		UI_DrawRectangle( m_scPos, m_scSize, uiInputFgColor );

}

/*
=================
UI_PlayerSetup_FindModels
=================
*/
void CMenuPlayerSetup::CModelListModel::Update( void )
{
	char	name[256];
	char	**filenames;
	int numFiles, i;

	m_iCount = 0;

	// Get file list
	// search in basedir too, because that's how GoldSrc does this
	filenames = EngFuncs::GetFilesList(  "models/player/*", &numFiles, FALSE );

	// build the model list
	for( i = 0; i < numFiles; i++ )
	{
		COM_FileBase( filenames[i], name, sizeof( name ));
		Q_strncpy( models[m_iCount], name, sizeof( models[0] ) );

		// check if the path is a valid model
		snprintf( name, sizeof( name ), "models/player/%s/%s.mdl", models[m_iCount], models[m_iCount] );
		if( !EngFuncs::FileExists( name ) )
			continue;

		m_iCount++;
	}
}

/*
=================
CMenuPlayerSetup::FindLogos

=================
*/
void CMenuPlayerSetup::CLogosListModel::Update( )
{
	char	**filenames;
	int numFiles, i;

	// Get file list
	filenames = EngFuncs::GetFilesList( "logos/*.*", &numFiles, FALSE );

	if( !filenames || !numFiles )
	{
		m_isPngs.RemoveAll();
		RemoveAll();
		return;
	}

	// build the model list
	for( i = 0; i < numFiles; i++ )
	{
		CUtlString logoFileName = filenames[i];
		char temp[256];
		bool png = logoFileName.BEndsWithCaseless( ".png" );

		if( png || logoFileName.BEndsWithCaseless( ".bmp" ))
		{
			COM_FileBase( logoFileName.String(), temp, sizeof( temp ));

			if( !stricmp( temp, "remapped" ))
				continue;

			AddToTail( temp );
			m_isPngs.AddToTail( png );
		}
	}
}

/*
=================
UI_PlayerSetup_SetConfig
=================
*/
void CMenuPlayerSetup::SetConfig( void )
{
	name.WriteCvar();
	model.WriteCvar();
	topColor.WriteCvar();
	bottomColor.WriteCvar();
	hiModels.WriteCvar();
	showModels.WriteCvar();
	WriteNewLogo();
}

void CMenuPlayerSetup::SaveAndPopMenu()
{
	if( !UI::Names::CheckIsNameValid( name.GetBuffer() ) )
	{
		msgBox.Show();
		return;
	}

	SetConfig();
	CMenuFramework::SaveAndPopMenu();
}

void CMenuPlayerSetup::UpdateModel()
{
	char image[256];
	const char *mdl = model.GetCurrentString();

	// seems we DON'T have this model locally
	// just force display string and do nothing
	if( !mdl )
	{
		model.ForceDisplayString( EngFuncs::GetCvarString( "model" ) );
		return;
	}

	snprintf( image, sizeof( image ), "models/player/%s/%s.bmp", mdl, mdl );
	view.hPlayerImage = EngFuncs::PIC_Load( image, PIC_KEEP_SOURCE );

	ApplyColorToImagePreview();
	EngFuncs::CvarSetString( "model", mdl );
	if( !strcmp( mdl, "player" ) )
		strcpy( image, "models/player.mdl" );
	else
		snprintf( image, sizeof(image), "models/player/%s/%s.mdl", mdl, mdl );
	if( view.ent )
		EngFuncs::SetModel( view.ent, image );
}

void CMenuPlayerSetup::UpdateLogo()
{
	char filename[1024];
	int pos = logo.GetCurrentValue();

	if( pos < 0 )
		return;

	logosModel.GetFullPath( filename, sizeof( filename ), pos );
	logoImage.hImage = EngFuncs::PIC_Load( filename, 0 );
	if( logosModel.IsPng( pos ))
	{
		logoImage.color = &g_LogoColors[0];
		logoColor.SetGrayed( true );
	}
	else
	{
		CBMP *bmpFile = CBMP::LoadFile( filename );
		if( bmpFile->GetBitmapHdr()->bitsPerPixel == 8 )
		{
			ApplyColorToLogoPreview();
			logoColor.SetGrayed( false );
		}
		else
		{
			logoImage.color = &g_LogoColors[0];
			logoColor.SetGrayed( true );
		}
		delete bmpFile;
	}

	EngFuncs::CvarSetString( "cl_logofile", logo.GetCurrentString() );
	logoColor.WriteCvar();
}

void CMenuPlayerSetup::ApplyColorToImagePreview()
{
	EngFuncs::ProcessImage( view.hPlayerImage, -1,
		topColor.GetCurrentValue(), bottomColor.GetCurrentValue() );
}

void CMenuPlayerSetup::ApplyColorToLogoPreview()
{
	const char *logoColorStr = logoColor.GetCurrentString();

	for( size_t i = 0; i < V_ARRAYSIZE( g_LogoColors ) && logoColorStr; i++ )
	{
		if( !stricmp( logoColorStr, L( g_LogoColors[i].name )))
		{
			logoImage.color = &g_LogoColors[i];
			return;
		}
	}

	logoColor.SetCurrentValue( L( g_LogoColors[0].name ) );
	logoImage.color = &g_LogoColors[0];
}

void CMenuPlayerSetup::WriteNewLogo( void )
{
	char filename[1024];
	int pos = logo.GetCurrentValue();

	if( pos < 0 || hideLogos )
		return;

	EngFuncs::DeleteFile( "logos/remapped.png" );
	EngFuncs::DeleteFile( "logos/remapped.bmp" );

	logosModel.GetFullPath( filename, sizeof( filename ), pos );

	// TODO: check file size and throw a messagebox if it's too big?
	if( logosModel.IsPng( pos ))
	{
		int len;
		void *afile = EngFuncs::COM_LoadFile( filename, &len );

		// just copy file, nothing special
		EngFuncs::COM_SaveFile( "logos/remapped.png", afile, len );

		EngFuncs::COM_FreeFile( afile );

		EngFuncs::CvarSetString( "cl_logoext", "png" );
	}
	else
	{
		CBMP *bmpFile = CBMP::LoadFile( filename );

		// not valid logo BMP file
		if( !bmpFile )
			return;

		// remap logo if needed
		if( logoImage.color->stripes >= 1 )
			bmpFile->RemapLogo( logoImage.color->stripes, logoImage.color->rgb );

		EngFuncs::COM_SaveFile( "logos/remapped.bmp", bmpFile->GetBitmap(), bmpFile->GetBitmapHdr()->fileSize );
		EngFuncs::CvarSetString( "cl_logoext", "bmp" );

		delete bmpFile;
	}

	logo.WriteCvar();
	logoColor.WriteCvar();
}

/*
=================
UI_PlayerSetup_Init
=================
*/
void CMenuPlayerSetup::_Init( void )
{
	int addFlags = 0;

	hideModels = hideLogos = false;

	// disable playermodel preview for HLRally to prevent crash
	if( !stricmp( gMenu.m_gameinfo.gamefolder, "hlrally" ))
		hideModels = true;

	if( gMenu.m_gameinfo.flags & GFL_NOMODELS )
		addFlags |= QMF_INACTIVE;

	banner.SetPicture(ART_BANNER);

	name.szStatusText = L( "Enter your multiplayer display name" );
	name.iMaxLength = 32;
	name.LinkCvar( "name" );
	name.SetRect( 320, 260, 256, 36 );

	modelsModel.Update();
	if( !modelsModel.GetRows() )
	{
		model.SetVisibility( false );
		hideModels = true;
	}
	else
	{
		model.Setup( &modelsModel );
		model.LinkCvar( "model", CMenuEditable::CVAR_STRING );
		model.onChanged = VoidCb( &CMenuPlayerSetup::UpdateModel );
		model.SetRect( 660, 580 + UI_OUTLINE_WIDTH, 260, 32 );
	}

	topColor.iFlags |= addFlags;
	topColor.SetNameAndStatus( L( "GameUI_PrimaryColor" ), L( "Set a player model top color" ) );
	topColor.Setup( 0.0, 255, 1 );
	topColor.LinkCvar( "topcolor" );
	topColor.onCvarChange = CMenuEditable::WriteCvarCb;
	topColor.onChanged = VoidCb( &CMenuPlayerSetup::ApplyColorToImagePreview );
	topColor.SetCoord( 340, 520 );
	topColor.size.w = 300;

	bottomColor.iFlags |= addFlags;
	bottomColor.SetNameAndStatus( L( "GameUI_SecondaryColor" ), L( "Set a player model bottom color" ) );
	bottomColor.Setup( 0.0, 255.0, 1 );
	bottomColor.LinkCvar( "bottomcolor" );
	bottomColor.onCvarChange = CMenuEditable::WriteCvarCb;
	bottomColor.onChanged = VoidCb( &CMenuPlayerSetup::ApplyColorToImagePreview );;
	bottomColor.SetCoord( 340, 590 );
	bottomColor.size.w = 300;

	showModels.iFlags |= addFlags;
	showModels.SetNameAndStatus( L( "Show 3D preview" ), L( "Show 3D player models instead of preview thumbnails" ) );
	showModels.LinkCvar( "ui_showmodels" );
	showModels.onCvarChange = CMenuEditable::WriteCvarCb;
	showModels.SetCoord( 340, 380 );

	hiModels.iFlags |= addFlags;
	hiModels.SetNameAndStatus( L( "GameUI_HighModels" ), L( "Show HD models in multiplayer" ) );
	hiModels.LinkCvar( "cl_himodels" );
	hiModels.onCvarChange = CMenuEditable::WriteCvarCb;
	hiModels.SetCoord( 340, 430 );

	view.iFlags |= addFlags;
	view.SetRect( 660, 260, 260, 320 );

	msgBox.SetMessage( L( "Please, choose another player name" ) );
	msgBox.Link( this );

	AddItem( banner );

	AddButton( L( "Done" ), L( "Go back to the Multiplayer Menu" ), PC_DONE, VoidCb( &CMenuPlayerSetup::SaveAndPopMenu ) );
	CMenuPicButton *gameOpt = AddButton( L( "Game options" ), L( "Configure handness, fov and other advanced options" ), PC_GAME_OPTIONS );
	SET_EVENT_MULTI( gameOpt->onReleased,
	{
		((CMenuPlayerSetup*)pSelf->Parent())->SetConfig();
		UI_AdvUserOptions_Menu();
	});

	AddButton( L( "Adv. Options" ), "", PC_ADV_OPT, UI_GameOptions_Menu );
	gameOpt->SetGrayed( !UI_AdvUserOptions_IsAvailable() );


	if( !hideLogos )
	{
		logosModel.Update();
		if( !logosModel.GetRows() )
		{
			// don't add to framework
			hideLogos = true;
		}
		else
		{
			static const char *itemlist[V_ARRAYSIZE( g_LogoColors )];
			static CStringArrayModel colors( itemlist, V_ARRAYSIZE( g_LogoColors ) );
			for( size_t i = 0; i < V_ARRAYSIZE( g_LogoColors ); i++ )
				itemlist[i] = L( g_LogoColors[i].name );

			logoImage.SetRect( 72, 230 + m_iBtnsNum * 50 + 10, 200, 200 );

			logo.Setup( &logosModel );
			logo.LinkCvar( "cl_logofile", CMenuEditable::CVAR_STRING );
			logo.onChanged = VoidCb( &CMenuPlayerSetup::UpdateLogo );
			logo.SetRect( 72, logoImage.pos.y + logoImage.size.h + UI_OUTLINE_WIDTH, 200, 32 );

			logoColor.Setup( &colors );
			logoColor.LinkCvar( "cl_logocolor", CMenuEditable::CVAR_STRING );
			logoColor.onChanged = VoidCb( &CMenuPlayerSetup::ApplyColorToLogoPreview );;
			logoColor.SetRect( 72, logo.pos.y + logo.size.h + UI_OUTLINE_WIDTH, 200, 32 );
		}
	}

	AddItem( name );
	if( !hideLogos )
	{
		UpdateLogo();
		AddItem( logo );
		AddItem( logoColor );
		AddItem( logoImage );
	}

	if( !(gMenu.m_gameinfo.flags & GFL_NOMODELS) )
	{
		AddItem( topColor );
		AddItem( bottomColor );
		AddItem( showModels );
		AddItem( hiModels );
		AddItem( model );
		// disable playermodel preview for HLRally to prevent crash
		if( !hideModels )
		{
			AddItem( view );
		}
	}
}

void CMenuPlayerSetup::Reload()
{
	if( !hideLogos ) UpdateLogo();
	if( !hideModels ) UpdateModel();
}

ADD_MENU( menu_playersetup, CMenuPlayerSetup, UI_PlayerSetup_Menu );
