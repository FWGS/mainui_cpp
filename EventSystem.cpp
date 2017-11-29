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
	pExtra(), type( OLD_STYLE_CALLBACK ),
	callback(), itemsHolderCallback(), szName( 0 )
{

}

CEventCallback::CEventCallback( EventCallback cb, void *ex ) :
	pExtra( ex ), type( OLD_STYLE_CALLBACK ),
	callback( cb ), itemsHolderCallback(), szName( 0 )
{

}

CEventCallback::CEventCallback(int execute_now, const char *sz)
{
	SetCommand( execute_now, sz );
}

CEventCallback::CEventCallback(VoidCallback cb) :
	pExtra( (void*)cb ), type( OLD_STYLE_CALLBACK ),
	callback( VoidCallbackWrapperCb ), itemsHolderCallback(), szName( 0 )
{
}

CEventCallback::CEventCallback(ItemsHolderCallback cb) :
	pExtra(), type( ITEMS_HOLDER_CALLBACK ),
	callback(), itemsHolderCallback( cb ),  szName( 0 )

{

}

void CEventCallback::operator()(CMenuBaseItem *pSelf)
{
	// if( pSelf->Parent() )
	switch( type )
	{
	case OLD_STYLE_CALLBACK:
		callback( pSelf, pExtra );
		return;
	case ITEMS_HOLDER_CALLBACK:
		((*pSelf->Parent()).*itemsHolderCallback)( pExtra );
		return;
	/*case ITEM_CALLBACK:
		((*pSelf->Parent()).*itemsHolderCallback)( pExtra );
		return;*/
	}
}

EventCallback CEventCallback::operator =( EventCallback cb )
{
	type = OLD_STYLE_CALLBACK;
	return callback = cb;
}
size_t        CEventCallback::operator =( size_t null )
{
	type = OLD_STYLE_CALLBACK;
	return (size_t)(callback = (EventCallback)null);
}
void*         CEventCallback::operator =( void *null )
{
	type = OLD_STYLE_CALLBACK;
	return (void*)(callback = (EventCallback)null);
}
VoidCallback  CEventCallback::operator =( VoidCallback cb )
{
	type = OLD_STYLE_CALLBACK;
	callback = VoidCallbackWrapperCb;
	return (VoidCallback)(pExtra = (void*)cb); // extradata can't be used anyway;
}
/*ItemCallback CEventCallback::operator =( ItemCallback cb )
{
	type = ITEM_CALLBACK;
	return itemCallback = cb;
}*/

ItemsHolderCallback CEventCallback::operator =( ItemsHolderCallback cb )
{
	type = ITEMS_HOLDER_CALLBACK;
	return itemsHolderCallback = cb;
}


void CEventCallback::SetCommand( int execute_now, const char *sz )
{
	execute_now ? operator =(CmdExecuteNowCb) : operator =(CmdExecuteNextFrameCb);
	pExtra = (void*)sz;
}

