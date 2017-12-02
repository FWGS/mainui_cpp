/*
EventSystem.h -- event system implementation
Copyright(C) 2017 a1batross

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#include "BaseMenu.h"
#include "BaseItem.h"

CEventCallback::CEventCallback() :
	pExtra( 0 ), type( CB_OLD_EXTRA ), szName( 0 )
{
	u.cb = 0;
}

CEventCallback::CEventCallback( EventCallback cb, void *ex ) :
	pExtra( ex ), type( CB_OLD_EXTRA ), szName( 0 )
{
	u.cb = cb;
}

CEventCallback::CEventCallback(int execute_now, const char *sz)
{
	SetCommand( execute_now, sz );
}

CEventCallback::CEventCallback(VoidCallback cb) :
	pExtra( 0 ), type( CB_OLD_VOID ), szName( 0 )
{
	u.voidCb = cb;
}

CEventCallback::CEventCallback( IHCallback cb, void *ex ) :
	pExtra( ex ), type( CB_IH_EXTRA ), szName( 0 )
{
	u.itemsHolderCb = cb;
}

CEventCallback::CEventCallback( VoidIHCallback cb ) :
	pExtra( 0 ), type( CB_IH_EXTRA ), szName( 0 )
{
	u.voidItemsHolderCb = cb;
}


void CEventCallback::operator()(CMenuBaseItem *pSelf)
{
	switch( type )
	{
	case CB_OLD_EXTRA: u.cb( pSelf, pExtra ); break;
	case CB_OLD_VOID:  u.voidCb(); break;
	case CB_IH_EXTRA:  (pSelf->Parent()->*u.itemsHolderCb)( pExtra ); break;
	case CB_IH_VOID:   (pSelf->Parent()->*u.voidItemsHolderCb)(); break;
	}
}

EventCallback CEventCallback::operator =( EventCallback cb )
{
	type = CB_OLD_EXTRA;
	return u.cb = cb;
}

VoidCallback  CEventCallback::operator =( VoidCallback cb )
{
	type = CB_OLD_VOID;
	return u.voidCb = cb;
}

IHCallback CEventCallback::operator =( IHCallback cb )
{
	type = CB_IH_EXTRA;
	return u.itemsHolderCb = cb;
}

VoidIHCallback CEventCallback::operator =( VoidIHCallback cb )
{
	type = CB_IH_EXTRA;
	return u.voidItemsHolderCb = cb;
}

void CEventCallback::Reset()
{
	type = CB_OLD_EXTRA;
	u.cb = 0;
}

void CEventCallback::SetCommand( int execute_now, const char *sz )
{
	execute_now ? operator =(CmdExecuteNowCb) : operator =(CmdExecuteNextFrameCb);
	pExtra = (void*)sz;
}

size_t        CEventCallback::operator =( size_t null )
{
	Reset();
	return 0;
}
void*         CEventCallback::operator =( void *null )
{
	Reset();
	return NULL;
}

#ifndef MY_COMPILER_SUCKS
std::nullptr_t CEventCallback::operator =( std::nullptr_t null )
{
	Reset();
	return nullptr;
}
#endif
