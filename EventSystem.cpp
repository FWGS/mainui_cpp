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
	pExtra(), callback(), cmd(), szName()
{

}

CEventCallback::CEventCallback( EventCallback cb, void *ex ) :
	pExtra( ex ), callback( cb ), cmd(), szName()
{

}

CEventCallback::~CEventCallback()
{
	FreeCommand();
}

CEventCallback::CmdCallback::CmdCallback(int _execute_now, const char *sz)
{
	execute_now = _execute_now;
	strncpy( cmd, sz, sizeof( cmd ) - 1 );
	cmd[sizeof(cmd)-1] = 0;
}
