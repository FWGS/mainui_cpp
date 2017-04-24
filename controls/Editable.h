/*
Editable.h - generic item for editables
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

#pragma once
#ifndef MENU_EDITABLE_H
#define MENU_EDITABLE_H

#include <assert.h>
#include "cvardef.h"
#include "Utils.h"

class CMenuEditable : public CMenuBaseItem
{
public:
	CMenuEditable() : CMenuBaseItem(),
		m_szCvarName(), m_eType(), m_szString(), m_flValue()
	{
	}

	virtual void UpdateEditable() = 0;

	enum cvarType_e
	{
		CVAR_STRING,
		CVAR_VALUE
	};

	virtual void LinkCvar( const char *name )
	{
		assert(("Derivative class does not implement LinkCvar(const char*) method. You need to specify types."));
	}

	void LinkCvar( const char *name, cvarType_e type )
	{
		m_szCvarName = name;
		m_eType = type;

		switch( m_eType )
		{
		case CVAR_STRING:
			strncpy( m_szString, EngFuncs::GetCvarString( name ), CS_SIZE);
			strncpy( m_szOriginalString, m_szString, CS_SIZE);
			m_szString[CS_SIZE-1] = m_szOriginalString[CS_SIZE-1] = 0;
			break;
		case CVAR_VALUE:
			m_flValue = m_flOriginalValue = EngFuncs::GetCvarFloat( name );
			break;
		}

		UpdateEditable();

		if( onCvarChange ) onCvarChange( this );
	}

	void DiscardChanges()
	{
		if( onCvarWrite ) onCvarWrite( this );

		switch( m_eType )
		{
		case CVAR_STRING: EngFuncs::CvarSetString( m_szCvarName, m_szOriginalString ); break;
		case CVAR_VALUE: EngFuncs::CvarSetValue( m_szCvarName, m_flOriginalValue ); break;
		}
	}

	void WriteCvar()
	{
		if( onCvarWrite ) onCvarWrite( this );

		switch( m_eType )
		{
		case CVAR_STRING: EngFuncs::CvarSetString( m_szCvarName, m_szString ); break;
		case CVAR_VALUE: EngFuncs::CvarSetValue( m_szCvarName, m_flValue ); break;
		}
	}

	static void WriteCvarCb( CMenuBaseItem *pSelf, void *pExtra )
	{
		((CMenuEditable*)pSelf)->WriteCvar();
	}
	static void DiscardChangesCb( CMenuBaseItem *pSelf, void *pExtra )
	{
		((CMenuEditable*)pSelf)->DiscardChanges();
	}

	float CvarValue() { return m_flValue; }
	const char *CvarString() { return m_szString; }

	void SetCvarValue( float value )
	{
		m_flValue = value;

		if( onCvarChange ) onCvarChange( this );
	}
	void SetCvarString( const char *string )
	{
		strncpy( m_szString, string, CS_SIZE );
		m_szString[CS_SIZE-1] = 0;

		if( onCvarChange ) onCvarChange( this );
	}

	CEventCallback onCvarWrite;
	CEventCallback onCvarChange;
	CEventCallback onCvarGet;

protected:
	cvarType_e  m_eType;
	const char *m_szCvarName;

	char		m_szString[CS_SIZE], m_szOriginalString[CS_SIZE];
	float		m_flValue, m_flOriginalValue;
};

#endif // MENU_EDITABLE_H
