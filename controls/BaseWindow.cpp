#include "extdll_menu.h"
#include "BaseMenu.h"
#include "Utils.h"
#include "PicButton.h"
#include "ItemsHolder.h"
#include "BaseWindow.h"

CMenuBaseWindow::CMenuBaseWindow(const char *name) : CMenuItemsHolder()
{
	bAllowDrag = false; // UNDONE
	m_bHolding = false;
	bInTransition = false;
	szName = name;
}

void CMenuBaseWindow::Show()
{
	Init();
	VidInit();
	PushMenu();
	EnableTransition();
	m_bAllowEnterActivate = false;
}

void CMenuBaseWindow::Hide()
{
	PopMenu();
	EnableTransition();
}

bool CMenuBaseWindow::IsVisible()
{
	// slow!
	for( int i = uiStatic.rootPosition; i < uiStatic.menuDepth; i++  )
	{
		if( uiStatic.menuStack[i] == this )
			return true;
	}
	return false;
}

void CMenuBaseWindow::PushMenu()
{
	int		i;
	CMenuBaseItem	*item;

	// if this menu is already present, drop back to that level to avoid stacking menus by hotkeys
	for( i = 0; i < uiStatic.menuDepth; i++ )
	{
		if( uiStatic.menuStack[i] == this )
		{
			if( IsRoot() )
				uiStatic.menuDepth = i;
			else
			{
				if( i != uiStatic.menuDepth - 1 )
				{
					// swap windows
					uiStatic.menuStack[i] = uiStatic.menuActive;
					uiStatic.menuStack[uiStatic.menuDepth] = this;
				}
			}
			break;
		}
	}

	if( i == uiStatic.menuDepth )
	{
		if( uiStatic.menuDepth >= UI_MAX_MENUDEPTH )
			Host_Error( "UI_PushMenu: menu stack overflow\n" );
		uiStatic.menuStack[uiStatic.menuDepth++] = this;
	}

	uiStatic.prevMenu = uiStatic.menuActive;
	if( this->IsRoot() && uiStatic.prevMenu && uiStatic.prevMenu->IsRoot() )
		uiStatic.prevMenu->EnableTransition();
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

		if( !item->IsVisible() || item->iFlags & (QMF_GRAYED|QMF_INACTIVE|QMF_MOUSEONLY))
			continue;

		m_iCursorPrev = -1;
		SetCursor( i );
		break;
	}
}

void CMenuBaseWindow::PopMenu()
{
	EngFuncs::PlayLocalSound( uiSoundOut );

	uiStatic.menuDepth--;

	if( uiStatic.menuDepth < 0 )
		Host_Error( "UI_PopMenu: menu stack underflow\n" );

	if( uiStatic.menuDepth )
	{
		uiStatic.prevMenu = this;
		uiStatic.menuActive = uiStatic.menuStack[uiStatic.menuDepth-1];
		if( this->IsRoot() && uiStatic.menuActive->IsRoot() )
			uiStatic.menuActive->EnableTransition();

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

void CMenuBaseWindow::SaveAndPopMenu( )
{
	EngFuncs::ClientCmd( FALSE, "trysaveconfig\n" );
	Hide();
}

const char *CMenuBaseWindow::Key(int key, int down)
{
	if( key == K_MOUSE1 && bAllowDrag )
	{
		m_bHolding = down;
		m_bHoldOffset.x = uiStatic.cursorX / uiStatic.scaleX;
		m_bHoldOffset.y = uiStatic.cursorY / uiStatic.scaleX;
	}

	if( down && key == K_ESCAPE )
	{
		Hide( );
		return uiSoundOut;
	}

	return CMenuItemsHolder::Key( key, down );
}

void CMenuBaseWindow::Draw()
{
	if( m_bHolding && bAllowDrag )
	{
		pos.x += uiStatic.cursorX / uiStatic.scaleX - m_bHoldOffset.x;
		pos.y += uiStatic.cursorY / uiStatic.scaleX- m_bHoldOffset.y;

		m_bHoldOffset.x = uiStatic.cursorX / uiStatic.scaleX;
		m_bHoldOffset.y = uiStatic.cursorY / uiStatic.scaleX;
		CalcPosition();
		CalcItemsPositions();
	}
	CMenuItemsHolder::Draw();
}


bool CMenuBaseWindow::DrawAnimation(EAnimation anim)
{
	float alpha;

	if( anim == ANIM_IN )
	{
		alpha = ( uiStatic.realTime - m_iTransitionStartTime ) / 200.0f;
	}
	else if( anim == ANIM_OUT )
	{
		alpha = 1.0f - ( uiStatic.realTime - m_iTransitionStartTime ) / 200.0f;
	}

	if(	( anim == ANIM_IN  && alpha < 1.0f )
		|| ( anim == ANIM_OUT && alpha > 0.0f ) )
	{
		UI_EnableAlphaFactor( alpha );

		Draw();

		UI_DisableAlphaFactor();

		if( IsRoot() )
			return CMenuPicButton::DrawTitleAnim( anim );
		return false;
	}

	return true;
}

bool CMenuBaseWindow::KeyValueData(const char *key, const char *data)
{
	if( !strcmp( key, "enabled" ) || !strcmp( key, "visible" ) )
	{

	}
	else
	{
		if( !strcmp( key, "xpos" ) ||
		!strcmp( key, "ypos" ) ||
		!strcmp( key, "wide" ) ||
		!strcmp( key, "tall" ) )
		{
			background.KeyValueData( key, data );
		}

		return CMenuBaseItem::KeyValueData(key, data);
	}

	return true;
}

void CMenuBaseWindow::EnableTransition()
{
	bInTransition = true;
	m_iTransitionStartTime = uiStatic.realTime;
}
