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
#include "Slider.h"
#include "PicButton.h"
#include "CheckBox.h"
#include "Table.h"
#include "SpinControl.h"
#include "Field.h"
#include "YesNoMessageBox.h"
#include "StringArrayModel.h"

#define ART_BANNER	  	"gfx/shell/head_touch_options"

static class CMenuTouchOptions : public CMenuFramework
{
private:
	void _Init();

public:
	CMenuTouchOptions() : CMenuFramework( "CMenuTouchOptions" ) { }

	void DeleteProfileCb( );
	void ResetButtonsCb();

	void SaveAndPopMenu();
	void GetConfig();

	void ResetMsgBox();
	void DeleteMsgBox();

	void Apply();
	void Save();

	class CProfiliesListModel : public CStringArrayModel
	{
	public:
		CProfiliesListModel() : CStringArrayModel( (const char*)profileDesc, 95, 0 ) {}
		void Update();
		const char *GetText( int line ) { return profileDesc[line]; }
		char profileDesc[UI_MAXGAMES][95];
		int	 iHighlight;
		int	 firstProfile;
	} model;

	CMenuPicButton	done;

	CMenuSlider	lookX;
	CMenuSlider	lookY;
	CMenuSlider	moveX;
	CMenuSlider	moveY;
	CMenuCheckBox	enable;
	CMenuCheckBox	grid;
	CMenuCheckBox	nomouse;
	CMenuPicButton	reset;
	CMenuPicButton	save;
	CMenuPicButton	remove;
	CMenuPicButton	apply;
	CMenuField	profilename;
	CMenuTable profiles;
	CMenuSpinControl gridsize;

	// prompt dialog
	CMenuYesNoMessageBox msgBox;
	void UpdateProfilies();
} uiTouchOptions;

void CMenuTouchOptions::CProfiliesListModel::Update( void )
{
	char	**filenames;
	int	i = 0, numFiles, j = 0;
	const char *curprofile;

	Q_strncpy( profileDesc[i], "Presets:", CS_SIZE );
	i++;

	filenames = EngFuncs::GetFilesList( "touch_presets/*.cfg", &numFiles, TRUE );
	for ( ; j < numFiles; i++, j++ )
	{
		if( i >= UI_MAXGAMES ) break;

		// strip path, leave only filename (empty slots doesn't have savename)
		COM_FileBase( filenames[j], profileDesc[i] );
	}

	// Overwrite "Presets:" line if there is no presets
	if( i == 1 )
		i = 0;

	filenames = EngFuncs::GetFilesList( "touch_profiles/*.cfg", &numFiles, TRUE );
	j = 0;
	curprofile = EngFuncs::GetCvarString("touch_config_file");

	Q_strncpy( profileDesc[i], "Profiles:", CS_SIZE );
	i++;

	Q_strncpy( profileDesc[i], "default", CS_SIZE );

	iHighlight = firstProfile = i;
	i++;

	for ( ; j < numFiles; i++, j++ )
	{
		if( i >= UI_MAXGAMES ) break;

		COM_FileBase( filenames[j], profileDesc[i] );
		if( !strcmp( filenames[j], curprofile ) )
			iHighlight = i;
	}

	m_iCount = i;
}

/*
=================
UI_TouchOptions_SetConfig
=================
*/
void CMenuTouchOptions::SaveAndPopMenu( void )
{
	grid.WriteCvar();
	gridsize.WriteCvar();
	lookX.WriteCvar();
	lookY.WriteCvar();
	moveX.WriteCvar();
	moveY.WriteCvar();
	enable.WriteCvar();
	nomouse.WriteCvar();

	CMenuFramework::SaveAndPopMenu();
}

void CMenuTouchOptions::GetConfig( void )
{
	grid.UpdateEditable();
	gridsize.UpdateEditable();
	lookX.UpdateEditable();
	lookY.UpdateEditable();
	moveX.UpdateEditable();
	moveY.UpdateEditable();
	enable.UpdateEditable();
	nomouse.UpdateEditable();
}

void CMenuTouchOptions::ResetMsgBox()
{
	msgBox.SetMessage( "Reset all buttons?");
	msgBox.onPositive = VoidCb( &CMenuTouchOptions::ResetButtonsCb );
	msgBox.Show();
}

void CMenuTouchOptions::DeleteMsgBox()
{
	msgBox.SetMessage( "Delete selected profile?");
	msgBox.onPositive = VoidCb( &CMenuTouchOptions::DeleteProfileCb );
	msgBox.Show();
}

void CMenuTouchOptions::Apply()
{
	int i = profiles.GetCurrentIndex();

	// preset selected
	if( i > 0 && i < model.firstProfile - 1 )
	{
		char command[256];
		const char *curconfig = EngFuncs::GetCvarString( "touch_config_file" );
		snprintf( command, 256, "exec \"touch_presets/%s\"\n", model.profileDesc[ i ] );
		EngFuncs::ClientCmd( 1,  command );

		while( EngFuncs::FileExists( curconfig, TRUE ) )
		{
			char copystring[256];
			char filebase[256];

			COM_FileBase( curconfig, filebase );

			if( snprintf( copystring, 256, "touch_profiles/%s (new).cfg", filebase ) > 255 )
				break;

			EngFuncs::CvarSetString( "touch_config_file", copystring );
			curconfig = EngFuncs::GetCvarString( "touch_config_file" );
		}
	}
	else if( i == model.firstProfile )
		EngFuncs::ClientCmd( 1, "exec touch.cfg\n" );
	else if( i > model.firstProfile )
	{
		char command[256];
		snprintf( command, 256, "exec \"touch_profiles/%s\"\n", model.profileDesc[ i ] );
		EngFuncs::ClientCmd( 1,  command );
	}

	// try save config
	EngFuncs::ClientCmd( 1,  "touch_writeconfig\n" );

	// check if it failed ant reset profile to default if it is
	if( !EngFuncs::FileExists( EngFuncs::GetCvarString( "touch_config_file" ), TRUE ) )
	{
		EngFuncs::CvarSetString( "touch_config_file", "touch.cfg" );
		profiles.SetCurrentIndex( model.firstProfile );
	}
	model.Update();
	GetConfig();
}

void CMenuTouchOptions::Save()
{
	char name[256];

	if( profilename.GetBuffer()[0] )
	{
		snprintf(name, 256, "touch_profiles/%s.cfg", profilename.GetBuffer() );
		EngFuncs::CvarSetString("touch_config_file", name );
	}
	EngFuncs::ClientCmd( 1, "touch_writeconfig\n" );
	model.Update();
	profilename.Clear();
}

void CMenuTouchOptions::UpdateProfilies()
{
	char curprofile[256];
	int isCurrent;
	int idx = profiles.GetCurrentIndex();

	COM_FileBase( EngFuncs::GetCvarString( "touch_config_file" ), curprofile );
	isCurrent = !strcmp( curprofile, model.profileDesc[ idx ]);

	// Scrolllist changed, update availiable options
	remove.SetGrayed( true );
	if( ( idx > model.firstProfile ) && !isCurrent )
		remove.SetGrayed( false );

	apply.SetGrayed( false );
	if( idx == 0 || idx == model.firstProfile - 1 )
		profiles.SetCurrentIndex( idx + 1 );
	if( isCurrent )
		apply.SetGrayed( true );
}

void CMenuTouchOptions::DeleteProfileCb()
{
	char command[256];

	if( profiles.GetCurrentIndex() <= model.firstProfile )
		return;

	snprintf(command, 256, "touch_deleteprofile \"%s\"\n", model.profileDesc[ profiles.GetCurrentIndex() ] );
	EngFuncs::ClientCmd( TRUE, command );

	model.Update();
}

void CMenuTouchOptions::ResetButtonsCb()
{
	EngFuncs::ClientCmd( FALSE, "touch_pitch 90\n" );
	EngFuncs::ClientCmd( FALSE, "touch_yaw 120\n" );
	EngFuncs::ClientCmd( FALSE, "touch_forwardzone 0.06\n" );
	EngFuncs::ClientCmd( FALSE, "touch_sidezone 0.06\n" );
	EngFuncs::ClientCmd( FALSE, "touch_grid 1\n" );
	EngFuncs::ClientCmd( TRUE,  "touch_grid_count 50\n" );

	GetConfig();
}

/*
=================
UI_TouchOptions_Init
=================
*/
void CMenuTouchOptions::_Init( void )
{
	banner.SetPicture(ART_BANNER);

	done.SetNameAndStatus( "Done", "Go back to the Touch Menu" );
	done.SetPicture( PC_DONE );
	done.onActivated = VoidCb( &CMenuTouchOptions::SaveAndPopMenu );
	done.SetCoord ( 72, 680 );

	lookX.SetNameAndStatus( "Look X", "Horizontal look sensitivity" );
	lookX.Setup( 50, 500, 5 );
	lookX.LinkCvar( "touch_yaw" );
	lookX.SetCoord( 72, 280 );

	lookY.SetCoord( 72, 340 );
	lookY.SetNameAndStatus( "Look Y", "Vertical look sensitivity" );
	lookY.Setup( 50, 500, 5 );
	lookY.LinkCvar( "touch_pitch" );

	moveX.SetNameAndStatus( "Side", "Side move sensitivity" );
	moveX.Setup( 0.02, 1.0, 0.05 );
	moveX.LinkCvar( "touch_sidezone" );
	moveX.SetCoord( 72, 400 );

	moveY.SetCoord( 72, 460 );
	moveY.SetNameAndStatus( "Forward", "Forward move sensitivity" );
	moveY.Setup( 0.02, 1.0, 0.05 );
	moveY.LinkCvar( "touch_forwardzone" );

	gridsize.szStatusText = "Set grid size";
	gridsize.Setup( 25, 100, 5 );
	gridsize.LinkCvar( "touch_grid_count", CMenuEditable::CVAR_VALUE );
	gridsize.SetRect( 72, 580, 210, 30 );

	grid.SetNameAndStatus( "Grid", "Enable/disable grid" );
	grid.LinkCvar( "touch_grid_enable" );
	grid.SetCoord( 72, 520 );

	enable.SetNameAndStatus( "Enable", "enable/disable touch controls" );
	enable.LinkCvar( "touch_enable" );
	enable.SetCoord( 680, 630 );

	nomouse.SetNameAndStatus( "Ignore Mouse", "Ignore mouse input" );
	nomouse.LinkCvar( "m_ignore" );
	nomouse.SetCoord( 680, 580 );

	profiles.SetRect( 360, 255, 300, 340 );
	profiles.SetModel( &model );
	UpdateProfilies();
	profiles.onChanged = VoidCb( &CMenuTouchOptions::UpdateProfilies );

	profilename.szName = "New Profile:";
	profilename.iMaxLength = 16;
	profilename.SetRect( 680, 260, 205, 32 );

	reset.SetNameAndStatus( "Reset", "Reset touch to default state" );
	reset.SetPicture("gfx/shell/btn_touch_reset");
	reset.SetCoord( 72, 630 );
	reset.onActivated = VoidCb( &CMenuTouchOptions::ResetMsgBox );

	remove.SetNameAndStatus( "Delete", "Delete saved game" );
	remove.SetPicture( PC_DELETE );
	remove.SetCoord( 560, 630 );
	remove.size.w = 100;
	remove.onActivated = VoidCb( &CMenuTouchOptions::DeleteMsgBox );

	apply.SetNameAndStatus( "Activate", "Apply selected profile" );
	apply.SetPicture( PC_ACTIVATE );
	apply.SetCoord( 360, 630 );
	apply.size.w = 120;
	apply.onActivated = VoidCb( &CMenuTouchOptions::Apply );

	save.SetNameAndStatus( "Save", "Save new profile" );
	save.SetPicture("gfx/shell/btn_touch_save");
	save.onActivated = VoidCb( &CMenuTouchOptions::Save );
	save.SetCoord( 680, 330 );

	msgBox.SetPositiveButton( "Ok", PC_OK );
	msgBox.Link( this );
	
	AddItem( background );
	AddItem( banner );
	AddItem( done );
	AddItem( lookX );
	AddItem( lookY );
	AddItem( moveX );
	AddItem( moveY );
	AddItem( enable );
	AddItem( nomouse );
	AddItem( reset );
	AddItem( profiles );
	AddItem( save );
	AddItem( profilename );
	AddItem( remove );
	AddItem( apply );
	AddItem( grid );
	AddItem( gridsize );
}

/*
=================
UI_TouchOptions_Precache
=================
*/
void UI_TouchOptions_Precache( void )
{
	EngFuncs::PIC_Load( ART_BANNER );
}

/*
=================
UI_TouchOptions_Menu
=================
*/
void UI_TouchOptions_Menu( void )
{
	UI_TouchOptions_Precache();
	uiTouchOptions.Show();
}
