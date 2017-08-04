#pragma once
#ifndef EMBEDITEM_H
#define EMBEDITEM_H

#include "BaseItem.h"

class CMenuItemsHolder : public CMenuBaseItem
{
public:
	CMenuItemsHolder();

	void Init();
	void VidInit();
	virtual const char *Key( int key, int down );
	virtual void Char( int key );
	virtual const char *Activate( void );
	virtual void ToggleInactive( void );
	virtual void SetInactive( bool visible );
	virtual void Draw( void );

	virtual bool MouseMove( int x, int y );

	void CursorMoved( void );
	void SetCursor( int newCursor, bool notify = true );
	void SetCursorToItem( CMenuBaseItem &item, bool notify = true );
	void AdjustCursor( int dir );

	void AddItem( CMenuBaseItem &item );
	void RemoveItem( CMenuBaseItem &item );
	CMenuBaseItem *ItemAtCursor( void );
	CMenuBaseItem *ItemAtCursorPrev( void );

	void CalcItemsPositions();
	void CalcItemsSizes();

	inline void AddItem(CMenuBaseItem *item ) { return AddItem( *item ); }
	inline int GetCursor() const { return m_iCursor; }
	inline int GetCursorPrev() const { return m_iCursorPrev; }
	inline int ItemCount() const { return m_numItems; }
	inline bool WasInit() const { return m_bInit; }

	DECLARE_EVENT_TO_MENU_METHOD( CMenuItemsHolder, Show );
	DECLARE_EVENT_TO_MENU_METHOD( CMenuItemsHolder, Hide );

protected:
	virtual void _Init() {}
	virtual void _VidInit() {}

	int m_iCursor;
	int m_iCursorPrev;

	CMenuBaseItem *m_pItems[UI_MAX_MENUITEMS];
	int m_numItems;
	bool m_bInit;
	bool m_bAllowEnterActivate;
};

#endif // EMBEDITEM_H
