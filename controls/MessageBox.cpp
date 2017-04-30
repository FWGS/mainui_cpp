#include "extdll.h"
#include "BaseMenu.h"
#include "Utils.h"
#include "Action.h"
#include "ItemsHolder.h"
#include "MessageBox.h"

CMenuMessageBox::CMenuMessageBox() : CMenuItemsHolder()
{
	iFlags |= QMF_INACTIVE;
}

void CMenuMessageBox::_Init()
{
	background.SetBackground( uiPromptBgColor );
	background.SetRect( DLG_X + 192, 256, 640, 128 );
	background.iFlags = QMF_INACTIVE;

	dlgMessage.iFlags = QMF_INACTIVE|QMF_DROPSHADOW;
	dlgMessage.SetCoord( DLG_X + 320, 300 );

	AddItem( background );
	AddItem( dlgMessage );
}

void CMenuMessageBox::SetMessage( const char *sz )
{
	dlgMessage.szName = sz;
}
