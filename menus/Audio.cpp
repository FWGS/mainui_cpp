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
#include "Slider.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "CheckBox.h"
#include "SpinControl.h"
#include "StringArrayModel.h"

#define ART_BANNER			"gfx/shell/head_audio"

class CMenuAudio : public CMenuFramework
{
public:
	typedef CMenuFramework BaseClass;

	CMenuAudio() : CMenuFramework("CMenuAudio") { }

private:
	void _Init() override;
	void _VidInit() override;
	void GetConfig();
	void VibrateChanged();
	void SaveAndPopMenu() override;

	void LerpingCvarWrite();

	CMenuSlider	soundVolume;
	CMenuSlider	musicVolume;
	CMenuSlider	suitVolume;
	CMenuSlider	vibration;
	CMenuSpinControl lerping;
	CMenuCheckBox noDSP;
	CMenuCheckBox muteFocusLost;
	CMenuCheckBox vibrationEnable;

	float oldVibrate;
};

/*
=================
CMenuAudio::GetConfig
=================
*/
void CMenuAudio::GetConfig( void )
{
	soundVolume.LinkCvar( "volume" );
	musicVolume.LinkCvar( "MP3Volume" );
	suitVolume.LinkCvar( "suitvolume" );
	vibration.LinkCvar( "vibration_length" );

	lerping.LinkCvar( "s_lerping", CMenuEditable::CVAR_VALUE );
	noDSP.LinkCvar( "dsp_off" );
	muteFocusLost.LinkCvar( "snd_mute_losefocus" );
	vibrationEnable.LinkCvar( "vibration_enable" );

	if( !vibrationEnable.bChecked )
		vibration.SetGrayed( true );
	oldVibrate = vibration.GetCurrentValue();
}

void CMenuAudio::VibrateChanged()
{
	float newVibrate = vibration.GetCurrentValue();
	if( oldVibrate != newVibrate )
	{
		char cmd[64];
		snprintf( cmd, 64, "vibrate %f", newVibrate );
		EngFuncs::ClientCmd( FALSE, cmd );
		vibration.WriteCvar();
		oldVibrate = newVibrate;
	}
}

/*
=================
CMenuAudio::SetConfig
=================
*/
void CMenuAudio::SaveAndPopMenu()
{
	soundVolume.WriteCvar();
	musicVolume.WriteCvar();
	suitVolume.WriteCvar();
	vibration.WriteCvar();
	lerping.WriteCvar();
	noDSP.WriteCvar();
	muteFocusLost.WriteCvar();
	vibrationEnable.WriteCvar();

	CMenuFramework::SaveAndPopMenu();
}

/*
=================
CMenuAudio::Init
=================
*/
void CMenuAudio::_Init( void )
{
	static const char *lerpingStr[] =
	{
		L( "GameUI_Disable" ), L( "Balance" ), L( "Quality" )
	};

	banner.SetPicture(ART_BANNER);

	soundVolume.SetNameAndStatus( L( "GameUI_SoundEffectVolume" ), L( "Set master volume level" ) );
	soundVolume.Setup( 0.0, 1.0, 0.05f );
	soundVolume.onChanged = CMenuEditable::WriteCvarCb;
	soundVolume.SetCoord( 320, 280 );

	musicVolume.SetNameAndStatus( L( "GameUI_MP3Volume" ), L( "Set background music volume level" ) );
	musicVolume.Setup( 0.0, 1.0, 0.05f );
	musicVolume.onChanged = CMenuEditable::WriteCvarCb;
	musicVolume.SetCoord( 320, 340 );

	suitVolume.SetNameAndStatus( L( "GameUI_HEVSuitVolume" ), L( "Set suit volume level" ) );
	suitVolume.Setup( 0.0, 1.0, 0.05f );
	suitVolume.onChanged = CMenuEditable::WriteCvarCb;
	suitVolume.SetCoord( 320, 400 );

	static CStringArrayModel model( lerpingStr, V_ARRAYSIZE( lerpingStr ));
	lerping.SetNameAndStatus( L( "Sound interpolation" ), L( "Enable/disable interpolation on sound output" ) );
	lerping.Setup( &model );
	lerping.onChanged = CMenuEditable::WriteCvarCb;
	lerping.font = QM_SMALLFONT;
	lerping.SetRect( 320, 470, 300, 32 );

	noDSP.SetNameAndStatus( L( "Disable DSP effects" ), L( "Disable sound processing (like echo, flanger, etc)" ) );
	noDSP.onChanged = CMenuEditable::WriteCvarCb;
	noDSP.SetCoord( 320, 520 );

	muteFocusLost.SetNameAndStatus( L( "Mute when inactive" ), L( "Disable sound when game goes into background" ) );
	muteFocusLost.onChanged = CMenuEditable::WriteCvarCb;
	muteFocusLost.SetCoord( 320, 570 );

	vibrationEnable.SetNameAndStatus( L( "Enable vibration" ), L( "In-game vibration(when player injured, etc)" ) );
	vibrationEnable.iMask = (QMF_GRAYED|QMF_INACTIVE);
	vibrationEnable.bInvertMask = true;
	vibrationEnable.onChanged = CMenuCheckBox::BitMaskCb;
	vibrationEnable.onChanged.pExtra = &vibration.iFlags;
	vibrationEnable.SetCoord( 700, 470 );

	vibration.SetNameAndStatus( L( "Vibration" ), L( "Default vibration length" ) );
	vibration.Setup( 0.0f, 5.0f, 0.05f );
	vibration.onChanged = VoidCb( &CMenuAudio::VibrateChanged );
	vibration.SetCoord( 700, 570 );

	AddItem( background );
	AddItem( banner );
	AddButton( L( "Done" ), L( "Go back to the Configuration Menu" ), PC_DONE,
		VoidCb( &CMenuAudio::SaveAndPopMenu ) );
	AddItem( soundVolume );
	AddItem( musicVolume );
	AddItem( suitVolume );
	AddItem( lerping );
	AddItem( noDSP );
	AddItem( muteFocusLost );
	AddItem( vibrationEnable );
	AddItem( vibration );
}

void CMenuAudio::_VidInit( )
{
	GetConfig();
}

ADD_MENU( menu_audio, CMenuAudio, UI_Audio_Menu );
