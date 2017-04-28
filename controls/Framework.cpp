#include "extdll.h"
#include "BaseMenu.h"
#include "Utils.h"
#include "PicButton.h"

CMenuFramework::CMenuFramework() : CMenuItemsHolder()
{
	SetCoord( 0, 0 );
}

/*
=================
UI_PushMenu
=================
*/
void CMenuFramework::PushMenu( void )
{
	int		i;
	CMenuBaseItem	*item;

	// if this menu is already present, drop back to that level to avoid stacking menus by hotkeys
	for( i = 0; i < uiStatic.menuDepth; i++ )
	{
		if( uiStatic.menuStack[i] == this )
		{
			uiStatic.menuDepth = i;
			break;
		}
	}

	if( i == uiStatic.menuDepth )
	{
		if( uiStatic.menuDepth >= UI_MAX_MENUDEPTH )
			Host_Error( "UI_PushMenu: menu stack overflow\n" );
		uiStatic.menuStack[uiStatic.menuDepth++] = this;
	}

	uiStatic.menuActive = this;
	uiStatic.firstDraw = true;
	uiStatic.enterSound = gpGlobals->time + 0.15;	// make some delay
	uiStatic.visible = true;

	EngFuncs::KEY_SetDest ( KEY_MENU );

	m_iCursor = 0;
	m_iCursorPrev = 0;

	// force first available item to have focus
	for( i = 0; i < m_numItems; i++ )
	{
		item = m_pItems[i];

		if( item->IsVisible() || item->iFlags & (QMF_GRAYED|QMF_INACTIVE|QMF_MOUSEONLY))
			continue;

		m_iCursorPrev = -1;
		SetCursor( i );
		break;
	}
}

/*
=================
UI_PopMenu
=================
*/
void CMenuFramework::PopMenu( void )
{
	UI_StartSound( uiSoundOut );

	uiStatic.menuDepth--;

	if( uiStatic.menuDepth < 0 )
		Host_Error( "UI_PopMenu: menu stack underflow\n" );

	CMenuPicButton::PopPButtonStack();

	if( uiStatic.menuDepth )
	{
		uiStatic.menuActive = uiStatic.menuStack[uiStatic.menuDepth-1];
		uiStatic.firstDraw = true;
	}
	else if ( CL_IsActive( ))
	{
		UI_CloseMenu();
	}
	else
	{
		// never trying the close menu when client isn't connected
		EngFuncs::KEY_SetDest( KEY_MENU );
		UI_Main_Menu();
	}

	if( uiStatic.m_fDemosPlayed && uiStatic.m_iOldMenuDepth == uiStatic.menuDepth )
	{
		EngFuncs::ClientCmd( FALSE, "demos\n" );
		uiStatic.m_fDemosPlayed = false;
		uiStatic.m_iOldMenuDepth = 0;
	}
}


void CMenuFramework::SaveAndPopMenu( )
{
	EngFuncs::ClientCmd( FALSE, "trysaveconfig\n" );
	PopMenu();
}

void CMenuFramework::Show()
{
	PushMenu();
}

void CMenuFramework::Hide()
{
	PopMenu();
}

bool CMenuFramework::IsVisible()
{
	return this == uiStatic.menuStack[uiStatic.menuDepth-1];
}

