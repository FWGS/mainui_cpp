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
#pragma once
#ifndef EVENTSYSTEM_H
#define EVENTSYSTEM_H

#include "Utils.h"

// Use these macros to set EventCallback, if no function pointer is available
// SET_EVENT must be followed by event code and closed by END_EVENT
// SET_EVENT_THIS can be used for setting event member inside it's class
#if defined(MY_COMPILER_SUCKS)

// WARN: can't rely on "item" name, because it can be something like "ptr->member"
// So we use something more valid for struct name

#define PASTE(x,y) __##x##_##y
#define PASTE2(x,y) PASTE(x,y)
#define EVNAME(x) PASTE2(x, __LINE__)

#define SET_EVENT( item, callback ) typedef struct { static void callback( CMenuBaseItem *pSelf, void *pExtra )
#define END_EVENT( item, callback ) } EVNAME(callback); (item).callback = EVNAME(callback)::callback;

#define SET_EVENT_PTR( ptr, callback ) SET_EVENT( (*ptr), callback )
#define END_EVENT_PTR( ptr, callback ) END_EVENT( (*ptr), callback )

#define SET_EVENT_THIS( callback ) SET_EVENT( (*this), callback )
#define END_EVENT_THIS( callback ) END_EVENT( (*this), callback )

#else

#define SET_EVENT( item, callback ) (item).callback = [](CMenuBaseItem *pSelf, void *pExtra)
#define END_EVENT( item, callback ) ;

#define SET_EVENT_PTR( ptr, callback ) SET_EVENT( (*ptr), callback )
#define END_EVENT_PTR( ptr, callback ) END_EVENT( (*ptr), callback )

#define SET_EVENT_THIS( callback ) SET_EVENT( (*this), callback )
#define END_EVENT_THIS( callback ) END_EVENT( (*this), callback )

#endif

#define DECLARE_NAMED_EVENT_TO_ITEM_METHOD( className, method, eventName ) \
	static void eventName##Cb( CMenuBaseItem *pSelf, void * ) \
	{\
		((className *)pSelf)->method();\
	}
#define DECLARE_NAMED_EVENT_TO_MENU_METHOD( className, method, eventName ) \
	static void eventName##Cb( CMenuBaseItem *pSelf, void * ) \
	{\
		((className *)pSelf->Parent())->method();\
	}

#define DECLARE_EVENT_TO_MENU_METHOD( className, method ) \
	DECLARE_NAMED_EVENT_TO_MENU_METHOD( className, method, method )

#define DECLARE_EVENT_TO_ITEM_METHOD( className, method ) \
	DECLARE_NAMED_EVENT_TO_ITEM_METHOD( className, method, method )

class CMenuBaseItem;
class CMenuItemsHolder;

enum menuEvent_e
{
	QM_GOTFOCUS = 1,
	QM_LOSTFOCUS,
	QM_ACTIVATED,
	QM_CHANGED,
	QM_PRESSED,
	QM_IMRESIZED
};

typedef void (*EventCallback)(CMenuBaseItem *, void *);
typedef void (*VoidCallback)(void);
#define CmdCallback (VoidCallback)0x1

class CEventCallback
{
public:
	CEventCallback();
	CEventCallback( EventCallback cb, void *ex = 0 );
	CEventCallback( VoidCallback cb );
	CEventCallback( int execute_now, const char *sz );

	void *pExtra;

	// convert to boolean for easy check in conditionals
	operator bool()          { return callback != 0; }
	operator EventCallback() { return callback; }

	void operator() ( CMenuBaseItem *pSelf ) { callback( pSelf, pExtra ); }

	EventCallback operator =( EventCallback cb ) { return callback = cb; }
	size_t        operator =( size_t null )      { return (size_t)(callback = (EventCallback)null); }
	void*         operator =( void *null )       { return (void*)(callback = (EventCallback)null); }
	VoidCallback  operator =( VoidCallback cb )
	{
		callback = VoidCallbackWrapperCb;
		return (VoidCallback)(pExtra = (void*)cb); // extradata can't be used anyway;
	}

	void SetCommand( int execute_now, const char *sz )
	{
		callback = execute_now ? CmdExecuteNowCb : CmdExecuteNextFrameCb;
		pExtra = (void*)sz;
	}

	static void NoopCb( CMenuBaseItem *, void * ) {}

private:
	EventCallback callback;

	// to find event command by name(for items holder)
	const char *szName;

	static void VoidCallbackWrapperCb( CMenuBaseItem *pSelf, void *pExtra )
	{
		((VoidCallback)pExtra)();
	}

	static void CmdExecuteNextFrameCb( CMenuBaseItem *pSelf, void *pExtra )
	{
		EngFuncs::ClientCmd( FALSE, (char *)pExtra );
	}

	static void CmdExecuteNowCb( CMenuBaseItem *pSelf, void *pExtra )
	{
		EngFuncs::ClientCmd( TRUE, (char *)pExtra );
	}

	friend class CMenuItemsHolder;
};


#endif // EVENTSYSTEM_H
