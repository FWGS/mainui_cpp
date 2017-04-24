#include "extdll.h"
#include "BaseMenu.h"
#include "Utils.h"
#include "PicButton.h"

CMenuFramework::CMenuFramework() :
	cursor(-1), cursorPrev(-1), items(), numItems(), initialized(false)
{
	;
}

/*
=================
UI_AddItem
=================
*/
void CMenuFramework::AddItem( CMenuBaseItem &generic )
{
	if( numItems >= UI_MAX_MENUITEMS )
		Host_Error( "UI_AddItem: UI_MAX_MENUITEMS limit exceeded\n" );

	items[numItems] = &generic;
	generic.m_pParent = this;
	generic.iFlags &= ~QMF_HASMOUSEFOCUS;

	numItems++;

	generic.Init();
}

/*
=================
menuFrawework_s::VidInitItems
=================
*/
void CMenuFramework::VidInitItems( void )
{
	for( int i = 0; i < numItems; i++ )
	{
		items[i]->VidInit();
	}
}

/*
=================
UI_CursorMoved
=================
*/
void CMenuFramework::CursorMoved( void )
{
	CMenuBaseItem *curItem;

	if( cursor == cursorPrev )
		return;

	if( cursorPrev >= 0 && cursorPrev < numItems )
	{
		curItem = items[cursorPrev];

		curItem->_Event( QM_LOSTFOCUS );
	}

	if( cursor >= 0 && cursor < numItems )
	{
		curItem = items[cursor];

		curItem->_Event( QM_GOTFOCUS );
	}
}

/*
=================
UI_SetCursor
=================
*/
void CMenuFramework::SetCursor( int newCursor, bool notify )
{
	if( newCursor < 0 || newCursor > numItems )
		return;

	if(items[newCursor]->iFlags & (QMF_GRAYED|QMF_INACTIVE|QMF_HIDDEN))
		return;

	cursorPrev = cursor;
	cursor = newCursor;

	if( notify )
		CursorMoved();
}

/*
=================
UI_SetCursorToItem
=================
*/
void CMenuFramework::SetCursorToItem( CMenuBaseItem *item , bool notify )
{
	for( int i = 0; i < numItems; i++ )
	{
		if( items[i] == item )
		{
			SetCursor( i, notify );
			return;
		}
	}
}

/*
=================
UI_ItemAtCursor
=================
*/
CMenuBaseItem *CMenuFramework::ItemAtCursor( void )
{
	if( cursor < 0 || cursor >= numItems )
		return 0;

	// inactive items can't be has focus
	if( items[cursor]->iFlags & QMF_INACTIVE )
		return 0;

	return items[cursor];
}

/*
=================
UI_AdjustCursor

This functiont takes the given menu, the direction, and attempts to
adjust the menu's cursor so that it's at the next available slot
=================
*/
void CMenuFramework::AdjustCursor( int dir )
{
	CMenuBaseItem	*item;
	int		wrapped = false;
wrap:
	while( cursor >= 0 && cursor < numItems )
	{
		item = items[cursor];
		if( item->iFlags & (QMF_GRAYED|QMF_INACTIVE|QMF_HIDDEN|QMF_MOUSEONLY))
			cursor += dir;
		else break;
	}

	if( dir == 1 )
	{
		if( cursor >= numItems )
		{
			if( wrapped )
			{
				cursor = cursorPrev;
				return;
			}

			cursor = 0;
			wrapped = true;
			goto wrap;
		}
	}
	else if( dir == -1 )
	{
		if( cursor < 0 )
		{
			if( wrapped )
			{
				cursor = cursorPrev;
				return;
			}
			cursor = numItems - 1;
			wrapped = true;
			goto wrap;
		}
	}
}

/*
=================
UI_DrawMenu
=================
*/
void CMenuFramework::Draw()
{
	static int	statusFadeTime;
	static CMenuBaseItem	*lastItem;
	CMenuBaseItem	*item;
	int		i;
	const char *statusText;

	// draw contents
	for( i = 0; i < numItems; i++ )
	{
		item = items[i];

		if( item->iFlags & QMF_HIDDEN )
			continue;

		item->Draw();
	}

	// draw status bar
	item = ItemAtCursor();
	if( item != lastItem )
	{
		// flash on selected button (like in GoldSrc)
		if( item ) item->m_iLastFocusTime = uiStatic.realTime;
		statusFadeTime = uiStatic.realTime;

		lastItem = item;
	}

	if( item && item == lastItem && ( ( statusText = item->szStatusText ) != NULL ))
	{
		// fade it in, but wait a second
		float alpha = bound(0, ((( uiStatic.realTime - statusFadeTime ) - 100 ) * 0.01f ), 1);
		int r, g, b, x, len;

		EngFuncs::ConsoleStringLen( statusText, &len, NULL );

		UnpackRGB( r, g, b, uiColorHelp );
		EngFuncs::DrawSetTextColor( r, g, b, alpha * 255 );
		x = ( ScreenWidth - len ) * 0.5; // centering

		EngFuncs::DrawConsoleString( x, 720 * uiStatic.scaleY, statusText );
	}
	else statusFadeTime = uiStatic.realTime;
}

/*
=================
UI_DefaultKey
=================
*/
const char *CMenuFramework::Key( int key, int down )
{
	const char	*sound = NULL;
	CMenuBaseItem	*item;
	int		_cursorPrev;

	// menu system key
	if( down && key == K_ESCAPE )
	{
		PopMenu();
		return uiSoundOut;
	}

	if( !numItems )
		return 0;

	item = ItemAtCursor();

	if( item && !(item->iFlags & (QMF_GRAYED|QMF_INACTIVE|QMF_HIDDEN)))
	{
		sound = item->Key( key, down );

		if( sound ) return sound; // key was handled
	}

	// system keys are always wait for keys down and never keys up
	if( !down )
		return 0;

	// default handling
	switch( key )
	{
	case K_UPARROW:
	case K_KP_UPARROW:
	case K_LEFTARROW:
	case K_KP_LEFTARROW:
		_cursorPrev = cursor;
		cursorPrev = cursor;
		cursor--;

		AdjustCursor( -1 );
		if( _cursorPrev != cursor )
		{
			CursorMoved();
			if( !(items[cursor]->iFlags & QMF_SILENT ))
				sound = uiSoundMove;

			items[cursorPrev]->iFlags &= ~QMF_HASKEYBOARDFOCUS;
			items[cursor]->iFlags |= QMF_HASKEYBOARDFOCUS;
		}
		break;
	case K_DOWNARROW:
	case K_KP_DOWNARROW:
	case K_RIGHTARROW:
	case K_KP_RIGHTARROW:
	case K_TAB:
		_cursorPrev = cursor;
		cursorPrev = cursor;
		cursor++;

		AdjustCursor(1);
		if( cursorPrev != cursor )
		{
			CursorMoved();
			if( !(items[cursor]->iFlags & QMF_SILENT ))
				sound = uiSoundMove;

			items[cursorPrev]->iFlags &= ~QMF_HASKEYBOARDFOCUS;
			items[cursor]->iFlags |= QMF_HASKEYBOARDFOCUS;
		}
		break;
	case K_MOUSE1:
		if( item )
		{
			if((item->iFlags & QMF_HASMOUSEFOCUS) && !(item->iFlags & (QMF_GRAYED|QMF_INACTIVE|QMF_HIDDEN)))
				return item->Activate();
		}

		break;
	case K_ENTER:
	case K_KP_ENTER:
	case K_AUX1:
	case K_AUX13:
		if( item )
		{
			if( !(item->iFlags & (QMF_GRAYED|QMF_INACTIVE|QMF_HIDDEN|QMF_MOUSEONLY)))
				return item->Activate();
		}
		break;
	}
	return sound;
}

void CMenuFramework::Char( int key )
{
	CMenuBaseItem	*item;

	if( numItems )
		return;

	item = ItemAtCursor( );

	if( item && !(item->iFlags & (QMF_GRAYED|QMF_INACTIVE|QMF_HIDDEN)))
	{
		item->Char( key );
	}
}

void CMenuFramework::MouseMove( int x, int y )
{
	int i;
	CMenuBaseItem *item;

	// region test the active menu items
	for( i = 0; i < numItems; i++ )
	{
		item = items[i];

		if( item->iFlags & (QMF_GRAYED|QMF_INACTIVE|QMF_HIDDEN) )
		{
			if( item->iFlags & (QMF_HASMOUSEFOCUS) )
			{
				if( !UI_CursorInRect( item->m_scPos, item->m_scSize ))
				{
					item->iFlags &= ~QMF_HASMOUSEFOCUS;
				}
				else item->m_iLastFocusTime = uiStatic.realTime;
			}
			continue;
		}

		if( !UI_CursorInRect( item->m_scPos, item->m_scSize ))
		{
			item->m_bPressed = false;
			item->iFlags &= ~QMF_HASMOUSEFOCUS;
			continue;
		}

		// set focus to item at cursor
		if( cursor != i )
		{
			SetCursor( i );
			items[cursorPrev]->iFlags &= ~QMF_HASMOUSEFOCUS;
			// reset a keyboard focus also, because we are changed cursor
			items[cursorPrev]->iFlags &= ~QMF_HASKEYBOARDFOCUS;

			if (!(items[cursor]->iFlags & QMF_SILENT ))
				UI_StartSound( uiSoundMove );
		}

		items[cursor]->iFlags |= QMF_HASMOUSEFOCUS;
		items[cursor]->m_iLastFocusTime = uiStatic.realTime;
		return;
	}

	// out of any region
	if( numItems )
	{
		items[cursor]->iFlags &= ~QMF_HASMOUSEFOCUS;
		items[cursor]->m_bPressed = false;

		// a mouse only item restores focus to the previous item
		if(items[cursor]->iFlags & QMF_MOUSEONLY )
		{
			if( cursorPrev != -1 )
				cursor = cursorPrev;
		}
	}
}

void CMenuFramework::Init()
{
	if( !initialized )
	{
		_Init();
		initialized = true;
	}

	// m_pLayout->Init();
}

void CMenuFramework::VidInit()
{
	_VidInit();

	VidInitItems();

	// m_pLayout->VidInit();
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

	cursor = 0;
	cursorPrev = 0;

	// force first available item to have focus
	for( i = 0; i < numItems; i++ )
	{
		item = items[i];

		if( item->iFlags & (QMF_GRAYED|QMF_INACTIVE|QMF_HIDDEN|QMF_MOUSEONLY))
			continue;

		cursorPrev = -1;
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

void CMenuFramework::ToggleItemsInactive()
{
	for( int i = 0; i < numItems; i++ )
	{
		if( !(items[i]->iFlags & QMF_DIALOG) )
			items[i]->ToggleInactive();
	}
}

void CMenuFramework::SetItemsInactive(bool inactive)
{
	for( int i = 0; i < numItems; i++ )
	{
		if( !(items[i]->iFlags & QMF_DIALOG) )
			items[i]->SetInactive( inactive );
	}
}

void CMenuFramework::ToggleItemsInactiveCb( CMenuBaseItem *pSelf, void *pExtra )
{
	pSelf->m_pParent->ToggleItemsInactive();
}

void CMenuFramework::PopMenuCb( CMenuBaseItem *pSelf, void *pExtra )
{
	pSelf->m_pParent->PopMenu();
}

void CMenuFramework::SaveAndPopMenuCb( CMenuBaseItem *pSelf, void *pExtra )
{
	EngFuncs::ClientCmd( FALSE, "trysaveconfig\n" );
	pSelf->m_pParent->PopMenu();
}

void CMenuFramework::CloseMenuCb(CMenuBaseItem *pSelf, void *pExtra)
{
	UI_CloseMenu();
}
