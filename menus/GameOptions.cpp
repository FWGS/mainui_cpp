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
#include "keydefs.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "SpinControl.h"
#include "Action.h"
#include "CheckBox.h"

#define ART_BANNER			"gfx/shell/head_advoptions"

class CMenuGameOptions : public CMenuFramework
{
public:
	CMenuGameOptions() : CMenuFramework("CMenuGameOptions") { }

	virtual const char *Key(int key, int down);
private:
	virtual void _Init();
	virtual void _VidInit();
	static void SaveCb( CMenuBaseItem *pSelf, void *pExtra );
	static void RestoreCb( CMenuBaseItem *pSelf, void *pExtra );
	void Restore();
	void GetConfig();

	CMenuSpinControl	maxFPS;
	CMenuAction	maxFPSmessage;
	CMenuCheckBox	hand;
	CMenuCheckBox	allowDownload;
	CMenuCheckBox	alwaysRun;

	CMenuSpinControl	maxPacket;
	CMenuAction	maxPacketmessage1;
};

static CMenuGameOptions	uiGameOptions;

/*
=================
UI_GameOptions_KeyFunc
=================
*/
const char *CMenuGameOptions::Key( int key, int down )
{
	if( down && UI::Key::IsEscape( key ) )
		Restore();
	return CMenuFramework::Key( key, down );
}

void CMenuGameOptions::SaveCb(CMenuBaseItem *pSelf, void *pExtra)
{
	CMenuGameOptions *parent = (CMenuGameOptions*)pSelf->Parent();

	parent->maxFPS.WriteCvar();
	parent->hand.WriteCvar();
	parent->allowDownload.WriteCvar();
	parent->alwaysRun.WriteCvar();
	parent->maxPacket.WriteCvar();

	EngFuncs::ClientCmd( FALSE, "trysaveconfig\n" );
	parent->Hide();
}

void CMenuGameOptions::Restore()
{
	maxFPS.DiscardChanges();
	hand.DiscardChanges();
	allowDownload.DiscardChanges();
	alwaysRun.DiscardChanges();
	maxPacket.DiscardChanges();
}

void CMenuGameOptions::RestoreCb(CMenuBaseItem *pSelf, void *pExtra)
{
	CMenuGameOptions *parent = (CMenuGameOptions*)pSelf->Parent();

	parent->Restore();
	parent->Hide();
}

/*
=================
UI_GameOptions_Init
=================
*/
void CMenuGameOptions::_Init( void )
{
	banner.SetPicture( ART_BANNER );

	maxFPS.szStatusText = "Cap your game frame rate";
	maxFPS.Setup( 20, 500, 20 );
	maxFPS.LinkCvar( "fps_max", CMenuEditable::CVAR_VALUE );

	maxFPSmessage.szName = "Limit game FPS";
	maxFPSmessage.iFlags = QMF_INACTIVE|QMF_DROPSHADOW;
	maxFPSmessage.iColor = uiColorHelp;
	maxFPSmessage.SetCharSize( QM_SMALLFONT );

	hand.SetNameAndStatus( "Use left hand", "Draw gun at left side" );
	hand.LinkCvar( "hand" );

	allowDownload.SetNameAndStatus( "Allow download", "Allow download of files from servers" );
	allowDownload.LinkCvar( "sv_allow_download" );

	alwaysRun.SetNameAndStatus( "Always run", "Switch between run/step models when pressed 'run' button" );
	alwaysRun.LinkCvar( "cl_run" );

	maxPacket.szStatusText = "Limit packet size";
	maxPacket.Setup( 200, 1500, 50 );
	maxPacket.LinkCvar( "cl_maxpacket", CMenuEditable::CVAR_VALUE );
	SET_EVENT( maxPacket, onChanged )
	{
		CMenuSpinControl *self = (CMenuSpinControl *)pSelf;
		if( self->GetCurrentValue() >= 1500 )
		{
			self->ForceDisplayString( "default" );
		}
	}
	END_EVENT( maxPacket, onChanged )


	maxPacketmessage1.iFlags = QMF_INACTIVE|QMF_DROPSHADOW;
	maxPacketmessage1.szName = "Limit network packet size\n";
	maxPacketmessage1.iColor = uiColorHelp;
	maxPacketmessage1.SetCharSize( QM_SMALLFONT );

	AddItem( background );
	AddItem( banner );
	AddButton( "Done", "Save changes and go back to the Customize Menu", PC_DONE, SaveCb );
	AddButton( "Cancel", "Go back to the Customize Menu", PC_CANCEL, RestoreCb );

	AddItem( maxFPS );
	AddItem( maxFPSmessage );
	AddItem( hand );
	AddItem( allowDownload );
	AddItem( alwaysRun );
	AddItem( maxPacket );
	AddItem( maxPacketmessage1 );

}

void CMenuGameOptions::_VidInit()
{
	maxFPS.SetRect( 240, 270, 220, 32 );
	maxFPSmessage.SetCoord( 240, 230 );
	hand.SetCoord( 240, 330 );
	allowDownload.SetCoord( 240, 390 );
	alwaysRun.SetCoord( 240, 450 );
	maxPacket.SetRect( 240, 560, 200, 32 );
	maxPacketmessage1.SetCoord( 240, 520 );
}

/*
=================
UI_GameOptions_Precache
=================
*/
void UI_GameOptions_Precache( void )
{
	EngFuncs::PIC_Load( ART_BANNER );
}

/*
=================
UI_GameOptions_Menu
=================
*/
void UI_GameOptions_Menu( void )
{
	UI_GameOptions_Precache();

	uiGameOptions.Show();
}
