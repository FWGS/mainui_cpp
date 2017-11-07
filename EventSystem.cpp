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

CEventCallback::CEventCallback() :
	pExtra(), callback(), szName( 0 )
{

}

CEventCallback::CEventCallback( EventCallback cb, void *ex ) :
	pExtra( ex ), callback( cb ), szName( 0 )
{

}

CEventCallback::CEventCallback( VoidCallback cb ) :
	pExtra( (void *)cb ), callback( VoidCallbackWrapperCb ), szName( 0 )
{

}

CEventCallback::CEventCallback(int execute_now, const char *sz)
{
	SetCommand( execute_now, sz );
}
