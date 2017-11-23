#include "BaseWindow.h"
#include "CheckBox.h"
#include "ConnectionWarning.h"
#include "PicButton.h"
#include "Action.h"

class CMenuConnectionWarning : public CMenuBaseWindow
{
public:
	CMenuConnectionWarning() : CMenuBaseWindow( "ConnectionWarning" )
	{

	}
	virtual void _Init();
	virtual void _VidInit();
	virtual const char *Key( int key, int down );
	void ClearCheckboxes();
	CMenuPicButton options, done;
private:
	CMenuCheckBox normal, dsl, slowest;
	CMenuAction title, message;
};

CMenuConnectionWarning uiConnectionWarning;

const char *CMenuConnectionWarning::Key( int key, int down )
{
	if( down && UI::Key::IsEscape( key ) )
	{
		return uiSoundNull; // handled
	}

	return CMenuBaseWindow::Key( key, down );
}

void CMenuConnectionWarning::_Init()
{
	iFlags |= QMF_DIALOG;

	background.bForceColor = true;
	background.iColor = uiPromptBgColor;

	normal.szName = "Normal internet connection";
	SET_EVENT( normal, onChanged )
	{
		uiConnectionWarning.ClearCheckboxes();
		EngFuncs::CvarSetValue("cl_maxpacket", 1400 );
		EngFuncs::CvarSetValue("cl_maxpayload", 0 );
		EngFuncs::CvarSetValue("cl_cmdrate", 30 );
		EngFuncs::CvarSetValue("cl_updaterate", 60 );
		EngFuncs::CvarSetValue("rate", 25000 );
		((CMenuCheckBox*)pSelf)->bChecked = true;
	}
	END_EVENT( normal, onChanged )
	normal.SetCoord( 20, 140 );

	dsl.szName = "DSL or PPTP with limited packet size";
	SET_EVENT( dsl, onChanged )
	{
		uiConnectionWarning.ClearCheckboxes();
		EngFuncs::CvarSetValue("cl_maxpacket", 1200 );
		EngFuncs::CvarSetValue("cl_maxpayload", 1000 );
		EngFuncs::CvarSetValue("cl_cmdrate", 30 );
		EngFuncs::CvarSetValue("cl_updaterate", 60 );
		EngFuncs::CvarSetValue("rate", 25000 );
		((CMenuCheckBox*)pSelf)->bChecked = true;
	}
	END_EVENT( dsl, onChanged )
	dsl.SetCoord( 20, 200 );

	slowest.szName = "Slow connection mode (64kbps)";
	SET_EVENT( slowest, onChanged )
	{
		uiConnectionWarning.ClearCheckboxes();
		EngFuncs::CvarSetValue("cl_maxpacket", 900 );
		EngFuncs::CvarSetValue("cl_maxpayload", 700 );
		EngFuncs::CvarSetValue("cl_cmdrate", 25 );
		EngFuncs::CvarSetValue("cl_updaterate", 30 );
		EngFuncs::CvarSetValue("rate", 7500 );
		((CMenuCheckBox*)pSelf)->bChecked = true;
	}
	END_EVENT( slowest, onChanged )
	slowest.SetCoord( 20, 260 );

	done.SetPicture( PC_DONE );
	done.szName = "Done";
	done.iFlags |= QMF_GRAYED;
	done.SetRect( 410, 320, UI_BUTTONS_WIDTH / 2, UI_BUTTONS_HEIGHT );
	done.onActivated = HideCb;

	options.SetPicture( PC_ADV_OPT );
	options.szName = "Adv Options";
	SET_EVENT( options, onActivated )
	{
		UI_GameOptions_Menu();
		uiConnectionWarning.done.iFlags &= ~QMF_GRAYED;
	}
	END_EVENT( options, onActivated )
	options.SetRect( 154, 320, UI_BUTTONS_WIDTH, UI_BUTTONS_HEIGHT );

	title.iFlags = QMF_INACTIVE|QMF_DROPSHADOW;
	title.eTextAlignment = QM_CENTER;
	title.szName = "Connection problem";
	title.SetRect( 0, 16, 640, 20 );

	message.iFlags = QMF_INACTIVE;
	message.szName = "Too many lost packets while connecting!\nPlease select network settings";
	message.SetRect( 20, 60, 600, 32 );

	AddItem( background );
	AddItem( done );
	AddItem( options );
	AddItem( normal );
	AddItem( dsl );
	AddItem( slowest );
	AddItem( title );
	AddItem( message );
}

void CMenuConnectionWarning::_VidInit()
{
	SetRect( DLG_X + 192, 192, 640, 384 );
}

void CMenuConnectionWarning::ClearCheckboxes()
{
	normal.bChecked = dsl.bChecked = slowest.bChecked = false;
	done.iFlags &= ~QMF_GRAYED;
}

void UI_ConnectionWarning_f()
{
	if( !UI_IsVisible() )
		UI_Main_Menu();
	uiConnectionWarning.Show();
}
