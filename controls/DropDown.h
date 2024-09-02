/*
DropDown.h - simple drop down menus
Copyright (C) 2023 numas13

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef DROP_DOWN_H
#define DROP_DOWN_H

#include "Editable.h"

class CMenuDropDown : public CMenuEditable
{
public:
	typedef CMenuEditable BaseClass;

	enum valType_e
	{
		VALUE_STRING = 0,
		VALUE_INT,
		VALUE_FLOAT,
	};

	bool KeyDown( int key ) override;
	bool KeyUp( int key ) override;
	void VidInit() override;
	void Draw() override;

	inline void Clear()
	{
		m_iState = 0;
		m_iCount = 0;
	}

	inline int GetCount()
	{
		return m_iCount;
	}

	inline void SelectItem( int i, bool event = true )
	{
		if( i != m_iState)
		{
			m_iState = i;
			if( event )
				_Event( QM_CHANGED );
		}
		else
			m_iState = i;
	}

	inline void SelectLast( bool event = true )
	{
		SelectItem( GetCount( ) - 1, event );
	}

	void SetMenuOpen( bool state );
	inline void MenuOpen() { SetMenuOpen( true ); }
	inline void MenuClose() { SetMenuOpen( false ); }
	inline void MenuToggle() { SetMenuOpen( !m_isOpen ); }

	// recalculate size
	inline void ForceRecalc()
	{
		MenuToggle( );
		MenuToggle( );
	}

	virtual void SetCvar() = 0;

	CColor iSelectColor;
	CColor iBackgroundColor;
	CColor iFgTextColor;
	CColor iBgTextColor;

	bool bDropUp;

protected:
	CMenuDropDown();

	inline void AddItemName ( const char *text )
	{
		m_szNames[m_iCount++] = text;
		ForceRecalc();
	}

	int m_iCount;
	int m_iState;
	bool m_isOpen;

private:
	int IsNewStateByMouseClick();

	const char *m_szNames[UI_MAX_MENUITEMS];
	Size m_scItemSize;

	CImage m_ArrowClosed;
	CImage m_ArrowOpened;
	Size m_ArrowSize;
};

class CMenuDropDownStr : public CMenuDropDown
{
public:
	CMenuDropDownStr() : CMenuDropDown() {}
	void UpdateEditable() override;

	inline void SetCvar() override
	{
		SetCvarString( m_Values[m_iState] );
	}

	inline void AddItem( const char *text, const char *value )
	{
		m_Values[m_iCount] = value;
		AddItemName( text );
	}

	inline const char *GetItem()
	{
		return GetItem( m_iState );
	}

	inline const char *GetItem( int i )
	{
		return m_Values[i];
	}

private:
	const char *m_Values[UI_MAX_MENUITEMS];
};

class CMenuDropDownInt : public CMenuDropDown
{
public:
	CMenuDropDownInt() : CMenuDropDown() {}
	void UpdateEditable() override;

	inline void SetCvar() override
	{
		SetCvarValue( m_Values[m_iState] );
	}

	inline void AddItem( const char *text, int value )
	{
		m_Values[m_iCount] = value;
		AddItemName( text );
	}

	inline int GetItem()
	{
		return GetItem( m_iState );
	}

	inline int GetItem( int i )
	{
		return m_Values[i];
	}

private:
	int m_Values[UI_MAX_MENUITEMS];
};

class CMenuDropDownFloat : public CMenuDropDown
{
public:
	CMenuDropDownFloat() : CMenuDropDown() {}
	void UpdateEditable() override;

	inline void SetCvar() override
	{
		SetCvarValue( m_Values[m_iState] );
	}

	inline void AddItem( const char *text, float value )
	{
		m_Values[m_iCount] = value;
		AddItemName( text );
	}

	inline float GetItem()
	{
		return GetItem( m_iState );
	}

	inline float GetItem( int i )
	{
		return m_Values[i];
	}

private:
	float m_Values[UI_MAX_MENUITEMS];
};

#endif // DROP_DOWN_H
