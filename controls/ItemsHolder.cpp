/*
ItemsHolder.cpp -- an item that can contain and operate with other items
Copyright (C) 2017 a1batross

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#include "extdll_menu.h"
#include "BaseMenu.h"
#include "Utils.h"
#include "PicButton.h"
#include "ItemsHolder.h"
#include "Scissor.h"
#include <string.h>

CMenuItemsHolder::CMenuItemsHolder() :
	BaseClass(), m_iCursor( 0 ), m_iCursorPrev( 0 ), m_pItems( ),
	m_events(), m_bInit( false ),
	m_bWrapCursor( true ), m_szResFile( 0 ), m_pItemAtCursorOnDown( 0 )
{
	;
}

bool CMenuItemsHolder::Key( const int key, const bool down )
{
	bool handled = false;

	if( m_pItems.Count() )
	{
		CMenuBaseItem *item = NULL;
		int cursorPrev;

		if( down )
		{
			item = ItemAtCursor();
			m_pItemAtCursorOnDown = item;
		}
		else
		{
			// don't send released event to item, which don't got pressed before!
			if( m_pItemAtCursorOnDown && m_pItemAtCursorOnDown->m_pParent == this )
				item = m_pItemAtCursorOnDown;
			m_pItemAtCursorOnDown = NULL;
		}

		if( item && item->IsVisible() && !(item->iFlags & (QMF_GRAYED|QMF_INACTIVE) ) )
		{
			// mouse keys must be checked for item bounds

			if( UI::Key::IsMouse( key ))
			{
				if( UI_CursorInRect( item->m_scPos, item->m_scSize ))
				{
					handled = down ? item->KeyDown( key ) : item->KeyUp( key );
					if( handled )
						return handled;
				}
			}
			else
			{
				handled = down ? item->KeyDown( key ) : item->KeyUp( key );
				if( handled )
					return handled;
			}
		}

		FOR_EACH_VEC( m_pItems, i )
		{
			item = m_pItems[i];

			if( !item )
				continue;

			if( !(item->iFlags & QMF_EVENTSIGNOREFOCUS) )
				continue; // predict state: such items are rare

			if( item == ItemAtCursor() )
				continue;

			if( item->iFlags & (QMF_GRAYED|QMF_INACTIVE) )
				continue;

			if( !item->IsVisible() )
				continue;

			down ? item->KeyDown( key ) : item->KeyUp( key );
		}

		// system keys are always wait for keys down and never keys up
		if( !down )
			return false;

		// default handling -- items navigation
		switch( key )
		{
		case K_UPARROW:
		case K_KP_UPARROW:
		case K_DPAD_UP:
		case K_LEFTARROW:
		case K_KP_LEFTARROW:
		case K_DPAD_LEFT:
			cursorPrev = m_iCursorPrev = m_iCursor;

			m_iCursor--;
			if( AdjustCursor( -1 ) )
			{
				if( cursorPrev != m_iCursor )
				{
					CursorMoved();
					m_pItems[m_iCursor]->PlayLocalSound( uiSoundMove );
					handled = true;

					m_pItems[m_iCursorPrev]->iFlags &= ~QMF_HASKEYBOARDFOCUS;
					m_pItems[m_iCursor]->iFlags |= QMF_HASKEYBOARDFOCUS;
				}
			}
			else
			{
				handled = false;
			}
			break;
		case K_DOWNARROW:
		case K_KP_DOWNARROW:
		case K_DPAD_DOWN:
		case K_RIGHTARROW:
		case K_KP_RIGHTARROW:
		case K_DPAD_RIGHT:
		case K_TAB:
			cursorPrev = m_iCursorPrev = m_iCursor;
			m_iCursor++;
			if( AdjustCursor( 1 ) )
			{
				if( cursorPrev != m_iCursor )
				{
					CursorMoved();
					m_pItems[m_iCursor]->PlayLocalSound( uiSoundMove );
					handled = true;

					m_pItems[m_iCursorPrev]->iFlags &= ~QMF_HASKEYBOARDFOCUS;
					m_pItems[m_iCursor]->iFlags |= QMF_HASKEYBOARDFOCUS;
				}
			}
			else
			{
				handled = false;
			}
			break;
		}
	}

	return handled;
}

bool CMenuItemsHolder::KeyDown( int key )
{
	return CMenuItemsHolder::Key( key, true );
}

bool CMenuItemsHolder::KeyUp( int key )
{
	return CMenuItemsHolder::Key( key, false );
}

void CMenuItemsHolder::Char( int ch )
{
	if( m_pItems.IsEmpty() )
		return;

	CMenuBaseItem *item = ItemAtCursor();

	if( item && item->IsVisible() && !(item->iFlags & (QMF_GRAYED|QMF_INACTIVE)) )
		item->Char( ch );

	FOR_EACH_VEC( m_pItems, i )
	{
		item = m_pItems[i];

		if( !item )
			continue;

		if( !(item->iFlags & QMF_EVENTSIGNOREFOCUS) )
			continue; // predict state: such items are rare

		if( item == ItemAtCursor() )
			continue;

		if( item->iFlags & (QMF_GRAYED|QMF_INACTIVE) )
			continue;

		if( !item->IsVisible() )
			continue;

		item->Char( ch );
	}
}

bool CMenuItemsHolder::MouseMove( int x, int y )
{
	int i;
	// region test the active menu items
	// go in reverse direction, so last items will be first
	for ( i = m_pItems.Count()-1; i >= 0; i-- )
	{
		CMenuBaseItem *item = m_pItems[i];

		// just in case
		if( !item )
			continue;

		// Invisible or inactive items will be skipped
		if( !item->IsVisible() || item->iFlags & (QMF_INACTIVE) )
		{
			if( item->iFlags & QMF_HASMOUSEFOCUS )
			{
				if( !UI_CursorInRect( item->m_scPos, item->m_scSize ) )
					item->iFlags &= ~QMF_HASMOUSEFOCUS;
				else item->m_iLastFocusTime = uiStatic.realTime;
			}
			continue;
		}

		// simple region test
		if( !UI_CursorInRect( item->m_scPos, item->m_scSize ) || !item->MouseMove( x, y ) )
		{
			item->iFlags &= ~QMF_HASMOUSEFOCUS;
			continue;
		}

		if( m_iCursor != i )
		{
			SetCursor( i );
			// reset two focus states, because we are changed cursor
			if( m_iCursorPrev != -1 )
				m_pItems[m_iCursorPrev]->iFlags &= ~(QMF_HASMOUSEFOCUS|QMF_HASKEYBOARDFOCUS);

			m_pItems[m_iCursor]->PlayLocalSound( uiSoundMove );
		}

		m_pItems[m_iCursor]->iFlags |= QMF_HASMOUSEFOCUS;
		m_pItems[m_iCursor]->m_iLastFocusTime = uiStatic.realTime;
		// Should we stop at first matched item?
		return true;
	}

	// out of any region
	if( !i )
	{
		m_pItems[m_iCursor]->iFlags &= ~QMF_HASMOUSEFOCUS;

		// a mouse only item restores focus to the previous item
		if( m_pItems[m_iCursor]->iFlags & QMF_MOUSEONLY )
			if( m_iCursorPrev != -1 )
				m_iCursor = m_iCursorPrev;
	}

	return false;
}

void CMenuItemsHolder::Init()
{
	if( !WasInit() )
	{
		m_bInit = true;

		_Init();
		// m_pLayout->Init();

		if( m_szResFile )
			LoadRES( m_szResFile );
	}
}

void CMenuItemsHolder::VidInit()
{
	_VidInit();
	CalcPosition();
	CalcSizes();
	VidInitItems();
	// m_pLayout->VidInit();
}

void CMenuItemsHolder::Reload()
{
	FOR_EACH_VEC( m_pItems, i )
		m_pItems[i]->Reload();
}

void CMenuItemsHolder::VidInitItems()
{
	FOR_EACH_VEC( m_pItems, i )
		m_pItems[i]->VidInit();
}

void CMenuItemsHolder::ToggleInactive()
{
	FOR_EACH_VEC( m_pItems, i )
		m_pItems[i]->ToggleInactive();
}

void CMenuItemsHolder::SetInactive( bool inactive )
{
	FOR_EACH_VEC( m_pItems, i )
		m_pItems[i]->SetInactive( inactive );
}

void CMenuItemsHolder::Draw( )
{
	CMenuBaseItem *item;

	// draw contents
	FOR_EACH_VEC( m_pItems, i )
	{
		item = m_pItems[i];

		if( item->iFlags & QMF_HIDDENBYPARENT )
			continue;

		if( !item->IsVisible() )
			continue;

		item->Draw();

		if( ui_borderclip->value )
			UI_DrawRectangle( item->m_scPos, item->m_scSize, PackRGBA( 255, 0, 0, 255 ) );
	}
}

void CMenuItemsHolder::Think( void )
{
	FOR_EACH_VEC( m_pItems, i )
	{
		m_pItems[i]->Think();
	}
}

/*
=================
UI_AdjustCursor

This functiont takes the given menu, the direction, and attempts to
adjust the menu's cursor so that it's at the next available slot
=================
*/
bool CMenuItemsHolder::AdjustCursor( int dir )
{
	CMenuBaseItem *item;
	bool wrapped = false;

wrap:
	while( m_iCursor >= 0 && m_iCursor < m_pItems.Count() )
	{
		item = m_pItems[m_iCursor];
		if( !item->IsVisible() || item->iFlags & (QMF_INACTIVE|QMF_MOUSEONLY) )
		{
			m_iCursor += dir;
		}
		else break;
	}

	if( dir > 0 )
	{
		if( m_iCursor >= m_pItems.Count() )
		{
			if( wrapped )
			{
				m_iCursor = m_iCursorPrev;
				return false;
			}
			m_iCursor = 0;
			wrapped = true;
			goto wrap;
		}
	}
	else if( dir < 0 )
	{
		if( m_iCursor < 0 )
		{
			if( wrapped )
			{
				m_iCursor = m_iCursorPrev;
				return false;
			}
			m_iCursor = m_pItems.Count() - 1;
			wrapped = true;
			goto wrap;
		}
	}

	if( wrapped )
		return m_bWrapCursor;
	return true;
}

CMenuBaseItem *CMenuItemsHolder::ItemAtCursor()
{
	if( m_iCursor < 0 || m_iCursor >= m_pItems.Count() )
		return 0;

	// inactive items can't be has focus
	if( m_pItems[m_iCursor]->iFlags & QMF_INACTIVE )
		return 0;

	return m_pItems[m_iCursor];
}

CMenuBaseItem *CMenuItemsHolder::ItemAtCursorPrev()
{
	if( m_iCursorPrev < 0 || m_iCursorPrev >= m_pItems.Count() )
		return 0;

	// inactive items can't be has focus
	if( m_pItems[m_iCursorPrev]->iFlags & QMF_INACTIVE )
		return 0;

	return m_pItems[m_iCursorPrev];
}

CMenuBaseItem *CMenuItemsHolder::FindItemByTag(const char *tag)
{
	if( !tag )
		return NULL;

	if( this->szTag && !strcmp( this->szTag, tag ) )
		return this;

	FOR_EACH_VEC( m_pItems, i )
	{
		if( !m_pItems[i]->szTag )
			continue;

		if( !strcmp( m_pItems[i]->szTag, tag ) )
			return m_pItems[i];
	}

	return NULL;
}

void CMenuItemsHolder::SetCursorToItem(CMenuBaseItem &item, bool notify )
{
	FOR_EACH_VEC( m_pItems, i )
	{
		if( m_pItems[i] == &item )
		{
			SetCursor( i, notify );
			return;
		}
	}
}

void CMenuItemsHolder::CalcItemsPositions()
{
	FOR_EACH_VEC( m_pItems, i )
		m_pItems[i]->CalcPosition();
}

void CMenuItemsHolder::CalcItemsSizes()
{
	FOR_EACH_VEC( m_pItems, i )
		m_pItems[i]->CalcSizes();
}

void CMenuItemsHolder::SetCursor( int newCursor, bool notify )
{
	if( newCursor < 0 || newCursor >= m_pItems.Count() )
		return;

	if( !m_pItems[newCursor]->IsVisible() || (m_pItems[newCursor]->iFlags & (QMF_INACTIVE)) )
		return;

	m_iCursorPrev = m_iCursor;
	m_iCursor = newCursor;

	if( notify )
		CursorMoved();
}

void CMenuItemsHolder::CursorMoved()
{
	CMenuBaseItem *item;

	if( m_iCursor == m_iCursorPrev )
		return;

	if( m_iCursorPrev >= 0 && m_iCursorPrev < m_pItems.Count() )
	{
		item = m_pItems[m_iCursorPrev];

		item->_Event( QM_LOSTFOCUS );

		// unconditionally remove pressed state, because item without focus can't be pressed anymore
		item->m_bPressed = false;
	}

	if( m_iCursor >= 0 && m_iCursor < m_pItems.Count() )
	{
		item = m_pItems[m_iCursor];

		item->_Event( QM_GOTFOCUS );

		if( item == m_pItemAtCursorOnDown )
			item->m_bPressed = true; // restore pressed state
	}
}

void CMenuItemsHolder::AddItem(CMenuBaseItem &item)
{
	m_pItems.AddToTail( &item );
	item.m_pParent = this; // U OWNED
	item.iFlags &= ~(QMF_HASMOUSEFOCUS|QMF_HIDDENBYPARENT);

	item.Init();
}

void CMenuItemsHolder::RemoveItem(CMenuBaseItem &item)
{
	if( m_pItems.FindAndRemove( &item ) )
	{
		item.m_pParent = NULL;
	}
}

bool RES_ExpectString( char **data, const char *expect, bool skip = true )
{
	char token[1024];
	char *tmp;

	if( !data || !*data )
		return true;

	tmp = EngFuncs::COM_ParseFile( *data, token );

	if( skip )
		*data = tmp;

	if( !*data )
		return true;

	if( !strncmp( token, expect, 1024 ) )
	{
		*data = tmp; // skip anyway, if expected string was here
		return true;
	}

	return false;
}

inline bool FreeFile( byte *pfile, const bool retval )
{
	EngFuncs::COM_FreeFile( pfile );
	return retval;
}

bool CMenuItemsHolder::LoadRES(const char *filename)
{
	byte *pfile = EngFuncs::COM_LoadFile( filename );
	char *afile = (char*)pfile;
	char token[1024];

	if( !pfile )
		return false;

	afile = EngFuncs::COM_ParseFile( afile, token );

	Con_DPrintf( "Loading res file from %s, name %s\n", filename, token );

	if( !afile )
		return FreeFile( pfile, false );

	if( !RES_ExpectString( &afile, "{" ) )
	{
		Con_DPrintf( "LoadRES: failed to parse, want '{'\n" );
		return FreeFile( pfile, false );
	}

	do
	{
		CMenuBaseItem *item;

		afile = EngFuncs::COM_ParseFile( afile, token );

		if( !afile )
			return FreeFile( pfile, false );

		item = FindItemByTag( token );

		if( !RES_ExpectString( &afile, "{" ))
		{
			Con_DPrintf( "LoadRES: failed to parse, want '{', near %s\n", token );
			return FreeFile( pfile, false );
		}

		if( item )
		{
			do
			{
				char key[1024];
				char value[1024];

				afile = EngFuncs::COM_ParseFile( afile, key );
				if( !afile )
					return FreeFile( pfile, false );

				afile = EngFuncs::COM_ParseFile( afile, value );
				if( !afile )
					return FreeFile( pfile, false );

				item->KeyValueData( key, value );
			}
			while( !RES_ExpectString( &afile, "}", false ) );
		}
		else
		{
			Con_DPrintf( "LoadRES: cannot find item %s, skipping!\n", token );

			while( !RES_ExpectString( &afile, "}" ) );
		}
	}
	while( !RES_ExpectString( &afile, "}", false ) );

	if( !RES_ExpectString( &afile, "}" ))
	{
		Con_DPrintf( "LoadRES: failed to parse, want '{'\n" );
		return FreeFile( pfile, false );
	}

	return FreeFile( pfile, true );
}

void CMenuItemsHolder::RegisterNamedEvent(CEventCallback ev, const char *name)
{
	ASSERT( name );
	ASSERT( ev );

	ev.szName = name;
	m_events.AddToTail( ev );
}

CEventCallback CMenuItemsHolder::FindEventByName(const char *name)
{
	for( int i = 0; i < m_events.Count(); i++ )
	{
		if( !strcmp( m_events[i].szName, name ))
			return m_events[i];
	}

	return CEventCallback();
}

bool CMenuItemsHolder::KeyValueData(const char *key, const char *data)
{
	return BaseClass::KeyValueData( key, data );
}

Point CMenuItemsHolder::GetPositionOffset() const
{
	return m_scPos;
}
