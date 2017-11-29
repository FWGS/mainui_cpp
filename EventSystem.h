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

// SET_EVENT_MULTI( event, callback )
// event -- CEventCallback object
// callback -- callback code block. Must contain a braces

// SET_EVENT( event, callback )
// same as MULTI, but does not require code block. Inteded for one-line events

#if defined(MY_COMPILER_SUCKS)
// WARN: can't rely on "item" name, because it can be something like "ptr->member"
// So we use something more valid for struct name
#define PASTE(x,y) __##x##_##y
#define PASTE2(x,y) PASTE(x,y)
#define EVNAME(x) PASTE2(x, __LINE__)

#define SET_EVENT_MULTI( event, callback ) \
	typedef struct                                                   \
	{                                                                \
		static void __callback( CMenuBaseItem *pSelf, void *pExtra )  \
		callback                                                     \
	} EVNAME( _event ); (event) = EVNAME( _event )::__callback

#else

#define SET_EVENT_MULTI( event, callback ) \
	(event) = [](CMenuBaseItem *pSelf, void *pExtra) callback
#endif

#define SET_EVENT( event, callback ) SET_EVENT_MULTI( event, { callback; } )

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
typedef void (CMenuItemsHolder::*ItemsHolderCallback)( void * );
typedef void (CMenuBaseItem::*ItemCallback)( void * );

#define MenuCb( a ) static_cast<ItemsHolderCallback>((a))

class CEventCallback
{
public:
	CEventCallback();
	CEventCallback( EventCallback cb, void *ex = 0 );
	CEventCallback( int execute_now, const char *sz );
	CEventCallback( VoidCallback cb );
	CEventCallback( ItemsHolderCallback cb );

	void *pExtra;

	// convert to boolean for easy check in conditionals
	operator bool()
	{
		switch(	type )
		{
		case OLD_STYLE_CALLBACK: return callback != 0;
		case ITEMS_HOLDER_CALLBACK: return itemsHolderCallback != 0;
		// case ITEM_CALLBACK: return itemCallback != 0;
		}
		return false;
	}

	void operator() ( CMenuBaseItem *pSelf );

	// ItemCallback operator =( ItemCallback cb );
	ItemsHolderCallback operator =( ItemsHolderCallback cb );
	EventCallback operator =( EventCallback cb );
	size_t        operator =( size_t null );
	void*         operator =( void *null );
	VoidCallback  operator =( VoidCallback cb );
	void SetCommand( int execute_now, const char *sz );

	static void NoopCb( CMenuBaseItem *, void * ) {}
private:
	enum
	{
		OLD_STYLE_CALLBACK = 0,
		ITEMS_HOLDER_CALLBACK
		// ITEM_CALLBACK
	} type;

	EventCallback callback;
	ItemsHolderCallback itemsHolderCallback;
	// ItemCallback itemCallback;

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
