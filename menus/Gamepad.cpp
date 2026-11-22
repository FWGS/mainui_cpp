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

#include "build.h"
#include "Framework.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "Slider.h"
#include "CheckBox.h"
#include "SpinControl.h"
#include "StringArrayModel.h"
#include "BaseModel.h"
#include "utlvector.h"
#include "utlstring.h"
#include "Table.h"
#include "YesNoMessageBox.h"
#include "Action.h"

#define ART_BANNER			"gfx/shell/head_gamepad"
#define ART_BANNER_FAVORITE "gfx/shell/favorite"

enum engineAxis_t
{
	JOY_AXIS_SIDE = 0,
	JOY_AXIS_FWD,
	JOY_AXIS_PITCH,
	JOY_AXIS_YAW,
	JOY_AXIS_RT,
	JOY_AXIS_LT,
	JOY_AXIS_NULL
};

static const char *axisNames[7] =
{
	"Side",
	"Forward",
	"Yaw",
	"Pitch",
	"Right Trigger",
	"Left Trigger",
	"NOT BOUND"
};

class CMenuGamePad : public CMenuFramework
{
public:
	CMenuGamePad() : CMenuFramework("CMenuGamePad") { }

private:
	void _Init() override;
	void _VidInit() override;
	void GetAxisBinding();
	void GetConfig();
	void SaveAndPopMenu() override;
	void UpdatePresets();
	void ApplyPreset();

	CMenuSlider side, forward, pitch, yaw;
	CMenuCheckBox invSide, invFwd, invPitch, invYaw;

	CMenuSpinControl axisBind[6];

	CMenuAction axisBind_label;

	CMenuCheckBox enableOsk;

	class CPresetsListModel : public CMenuBaseModel, public CUtlVector<CUtlString>
	{
	public:
		void Update() override {}
		int GetColumns() const override { return 2; }
		int GetRows() const override { return Count(); }

		const char *GetCellText( int line, int column ) override
		{
			if( column == 0 )
				return Element( line ).String();

			return line == iActive ? ART_BANNER_FAVORITE : NULL;
		}

		ECellType GetCellType( int line, int column ) override
		{
			return column == 0 ? CELL_TEXT : CELL_IMAGE_ADDITIVE;
		}

		unsigned int GetAlignmentForColumn( int column ) const override
		{
			return column == 0 ? QM_LEFT : QM_CENTER;
		}

		int iActive = -1;
	} presetsModel;
	CMenuTable presets;
	CMenuPicButton applyPreset;
	CMenuYesNoMessageBox msgBox;
};

/*
=================
CMenuGamePad::GetAxisBinding

=================
*/
void CMenuGamePad::GetAxisBinding( void )
{
	char binding[7] = { 0 };

	Q_strncpy( binding, EngFuncs::GetCvarString( "joy_axis_binding"), sizeof( binding ));

	// I made a monster...
	for( unsigned int i = 0; i < sizeof( binding ) - 1; i++ )
	{
		switch( binding[i] )
		{
		case 's':
			axisBind[i].ForceDisplayString( L( axisNames[JOY_AXIS_SIDE] ) );
			axisBind[i].SetCurrentValue( JOY_AXIS_SIDE );
			break;
		case 'f':
			axisBind[i].ForceDisplayString( L( axisNames[JOY_AXIS_FWD] ) );
			axisBind[i].SetCurrentValue( JOY_AXIS_FWD );
			break;
		case 'p':
			axisBind[i].ForceDisplayString( L( axisNames[JOY_AXIS_PITCH] ) );
			axisBind[i].SetCurrentValue( JOY_AXIS_PITCH );
			break;
		case 'y':
			axisBind[i].ForceDisplayString( L( axisNames[JOY_AXIS_YAW] ) );
			axisBind[i].SetCurrentValue( JOY_AXIS_YAW );
			break;
		case 'r':
			axisBind[i].ForceDisplayString( L( axisNames[JOY_AXIS_RT] ) );
			axisBind[i].SetCurrentValue( JOY_AXIS_RT );
			break;
		case 'l':
			axisBind[i].ForceDisplayString( L( axisNames[JOY_AXIS_LT] ) );
			axisBind[i].SetCurrentValue( JOY_AXIS_LT );
			break;
		default:
			axisBind[i].ForceDisplayString( L( axisNames[JOY_AXIS_NULL] ) );
			axisBind[i].SetCurrentValue( JOY_AXIS_NULL );
		}
	}
}

/*
=================
CMenuGamePad::GetConfig

=================
*/
void CMenuGamePad::GetConfig( void )
{
	float _side, _forward, _pitch, _yaw;

	enableOsk.LinkCvar( "osk_enable" );

	_side = EngFuncs::GetCvarFloat( "joy_side" );
	_forward = EngFuncs::GetCvarFloat( "joy_forward" );
	_pitch = EngFuncs::GetCvarFloat( "joy_pitch" );
	_yaw = EngFuncs::GetCvarFloat( "joy_yaw" );

	side.SetCurrentValue( fabs( _side ) );
	forward.SetCurrentValue( fabs( _forward ));
	pitch.SetCurrentValue( fabs( _pitch ));
	yaw.SetCurrentValue( fabs( _yaw ));

	invSide.bChecked = _side < 0.0f;
	invFwd.bChecked = _forward < 0.0f;
	invPitch.bChecked = _pitch < 0.0f;
	invYaw.bChecked = _yaw < 0.0f;

	GetAxisBinding();
	UpdatePresets();
}

/*
=================
CMenuGamePad::SetConfig
=================
*/
void CMenuGamePad::SaveAndPopMenu()
{
	float _side, _forward, _pitch, _yaw;
	char binding[7] = { 0 };

	_side = side.GetCurrentValue();
	if( invSide.bChecked )
		_side *= -1;

	_forward = forward.GetCurrentValue();
	if( invFwd.bChecked )
		_forward *= -1;

	_pitch = pitch.GetCurrentValue();
	if( invPitch.bChecked )
		_pitch *= -1;

	_yaw = yaw.GetCurrentValue();
	if( invYaw.bChecked )
		_yaw *= -1;

	for( int i = 0; i < 6; i++ )
	{
		switch( (int)axisBind[i].GetCurrentValue() )
		{
		case JOY_AXIS_SIDE: binding[i]  = 's'; break;
		case JOY_AXIS_FWD: binding[i]   = 'f'; break;
		case JOY_AXIS_PITCH: binding[i] = 'p'; break;
		case JOY_AXIS_YAW: binding[i]   = 'y'; break;
		case JOY_AXIS_RT: binding[i]    = 'r'; break;
		case JOY_AXIS_LT: binding[i]    = 'l'; break;
		default: binding[i] = '0'; break;
		}
	}

	EngFuncs::CvarSetValue( "joy_side", _side );
	EngFuncs::CvarSetValue( "joy_forward", _forward );
	EngFuncs::CvarSetValue( "joy_pitch", _pitch );
	EngFuncs::CvarSetValue( "joy_yaw", _yaw );
	EngFuncs::CvarSetString( "joy_axis_binding", binding );

	enableOsk.WriteCvar();

	CMenuFramework::SaveAndPopMenu();
}

/*
=================
CMenuGamePad::UpdatePresets

=================
*/
void CMenuGamePad::UpdatePresets()
{
	const char *preset = EngFuncs::GetCvarString( "joy_preset" );
	char **filenames;
	int numFiles;

	presetsModel.RemoveAll();
	presetsModel.iActive = -1;

	// presets are shipped in engine's extras.pk3, so don't limit search to game directory
	filenames = EngFuncs::GetFilesList( "gamepad_presets/*.cfg", &numFiles, false );

	for( int i = 0; filenames && i < numFiles; i++ )
	{
		char name[128];

		COM_FileBase( filenames[i], name, sizeof( name ));
		presetsModel.AddToTail( name );

		if( !strcmp( name, preset ))
			presetsModel.iActive = presetsModel.Count() - 1;
	}

	presets.SetCurrentIndex( presetsModel.iActive >= 0 ? presetsModel.iActive : 0 );
	applyPreset.SetGrayed( presetsModel.Count() == 0 );
}

/*
=================
CMenuGamePad::ApplyPreset

=================
*/
void CMenuGamePad::ApplyPreset()
{
	int i = presets.GetCurrentIndex();
	char name[256];

	if( !presetsModel.IsValidIndex( i ))
		return;

	Com_EscapeCommand( name, presetsModel[i].Get(), sizeof( name ));
	EngFuncs::ClientCmdF( true, "joy_loadpreset \"%s\"\n", name );
	EngFuncs::ClientCmd( false, "host_writeconfig\n" );

	// preset changes axis binding, reload it or Done writes old one back
	GetAxisBinding();
	UpdatePresets();
}

/*
=================
CMenuGamePad::Init
=================
*/
void CMenuGamePad::_Init( void )
{
	int i, y;

	static CStringArrayModel model( axisNames, V_ARRAYSIZE( axisNames ) );

	banner.SetPicture( ART_BANNER );

	enableOsk.SetNameAndStatus( L( "Builtin on-screen keyboard" ), L( "Enable builtin on-screen keyboard in case your platform doesn't have any" ));

	axisBind_label.eTextAlignment = QM_CENTER;
	axisBind_label.iFlags = QMF_INACTIVE|QMF_DROPSHADOW;
	axisBind_label.colorBase = uiColorHelp;
	axisBind_label.szName = L( "Axis binding map" );

	for( i = 0, y = 230; i < 6; i++, y += 50 )
	{
		axisBind[i].szStatusText = L( "Set axis binding" );
		axisBind[i].Setup( &model );
	}

	side.Setup( 0.0f, 1.0f, 0.1f );
	side.SetNameAndStatus( L( "Side" ), L( "Side movement sensitivity" ) );
	invSide.SetNameAndStatus( L( "Invert" ), L( "Invert side movement axis" ) );

	forward.Setup( 0.0f, 1.0f, 0.1f );
	forward.SetNameAndStatus( L( "Forward" ), L( "Forward movement sensitivity" ) );
	invFwd.SetNameAndStatus( L( "Invert" ), L( "Invert forward movement axis" ) );

	pitch.Setup( 0.0f, 200.0f, 0.1f );
	pitch.SetNameAndStatus( L( "Look X" ), L( "Horizontal look sensitivity" ) );
	invPitch.SetNameAndStatus( L( "Invert" ), L( "Invert pitch axis" ) );

	yaw.Setup( 0.0f, 200.0f, 0.1f );
	yaw.SetNameAndStatus( L( "Look Y" ), L( "Vertical look sensitivity" ) );
	invYaw.SetNameAndStatus( L( "Invert" ), L( "Invert yaw axis" ) );

	presets.SetModel( &presetsModel );
	presets.SetupColumn( 0, L( "Presets" ), 1.0f );
	presets.SetupColumn( 1, NULL, 32.0f, true );

	applyPreset.SetNameAndStatus( L( "Activate" ), L( "Apply selected preset" ) );
	applyPreset.SetPicture( PC_ACTIVATE );
	applyPreset.onReleased = msgBox.MakeOpenEvent();

	msgBox.SetMessage( L( "Replace gamepad bindings with the selected preset?" ) );
	msgBox.onPositive = VoidCb( &CMenuGamePad::ApplyPreset );
	msgBox.Link( this );

	AddItem( banner );
	AddButton( L( "Controls" ), nullptr, PC_CONTROLS, UI_Controls_Menu );
	AddButton( L( "Gyroscope" ), nullptr, PC_GYRO, UI_GamePadGyro_Menu, QMF_NOTIFY, 'g' );
	AddButton( L( "Done" ), nullptr, PC_DONE, VoidCb( &CMenuGamePad::SaveAndPopMenu ) );	// Обе строки уже встречались ранее !!
	for( i = 0; i < 6; i++ )
		AddItem( axisBind[i] );
	AddItem( enableOsk );
	AddItem( side );
	AddItem( invSide );
	AddItem( forward );
	AddItem( invFwd );
	AddItem( pitch );
	AddItem( invPitch );
	AddItem( yaw );
	AddItem( invYaw );
	AddItem( axisBind_label );
	AddItem( presets );
	AddItem( applyPreset );
}

void CMenuGamePad::_VidInit()
{
	axisBind_label.SetCoord( 360, 230 );
	axisBind_label.SetCharSize( QM_SMALLFONT );

	int y = 280;
	for( int i = 0; i < 6; i++, y += 50 )
	{
		axisBind[i].SetRect( 360, y, 256, invSide.size.h );
		axisBind[i].SetCharSize( QM_SMALLFONT );
	}

	int sliderAlign = invSide.size.h - side.size.h;

	side.SetCoord( 630, 280 + sliderAlign );
	side.SetCharSize( QM_SMALLFONT );
	invSide.SetCoord( 850, 280 );

	forward.SetCoord( 630, 330 + sliderAlign );
	forward.SetCharSize( QM_SMALLFONT );
	invFwd.SetCoord( 850, 330 );

	pitch.SetCoord( 630, 380 + sliderAlign );
	pitch.SetCharSize( QM_SMALLFONT );
	invPitch.SetCoord( 850, 380 );

	yaw.SetCoord( 630, 430 + sliderAlign );
	yaw.SetCharSize( QM_SMALLFONT );
	invYaw.SetCoord( 850, 430 );

	enableOsk.SetCoord( 630, 480 );

	presets.SetRect( 630, 530, 330, 140 );

	applyPreset.SetCoord( 630, 680 );
	applyPreset.size.w = 120;

	GetConfig();
}

ADD_MENU( menu_gamepad, CMenuGamePad, UI_GamePad_Menu );
