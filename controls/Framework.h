#pragma once
#ifndef FRAMEWORK_H
#define FRAMEWORK_H


class CMenuBaseItem;

/*class CMenuItemHolder
{
public:
protected:
	int cursor;
	int cursorPrev;

	CMenuBaseItem *items[UI_MAX_MENUITEMS];
	int numItems;
};*/

class CMenuFramework //: public CMenuItemHolder
{
public:
	CMenuFramework();

	void Init( void );
	void VidInit( void );

	virtual void Draw( void );
	virtual const char *Key( int key, int down );
	virtual void Activate( void ){}
	virtual void Char( int key );
	virtual void MouseMove( int x, int y );

	void CursorMoved( void );
	void SetCursor( int newCursor, bool notify = true );
	void SetCursorToItem( CMenuBaseItem *item, bool notify = true );
	void AdjustCursor( int dir );

	void AddItem( CMenuBaseItem &item );
	CMenuBaseItem *ItemAtCursor( void );

	void PushMenu( void );
	void PopMenu( void );
	void ToggleItemsInactive( void );
	void SetItemsInactive( bool inactive );

	inline int GetCursor() const { return cursor; }
	inline int GetCursorPrev() const { return cursorPrev; }
	inline int ItemCount() const { return numItems; }
	inline bool WasInit() const { return initialized; }
	inline void Open()
	{
		Init();
		VidInit();

		PushMenu();
	}

	// Events library
	static void ToggleItemsInactiveCb(CMenuBaseItem *pSelf, void *pExtra);
	static void PopMenuCb( CMenuBaseItem *pSelf, void *pExtra );
	static void SaveAndPopMenuCb( CMenuBaseItem *pSelf, void *pExtra );
	static void CloseMenuCb( CMenuBaseItem *pSelf, void *pExtra );
private:
	virtual void _Init() {}
	virtual void _VidInit() {}
	void VidInitItems();
	bool initialized;
	int cursor;
	int cursorPrev;

	CMenuBaseItem *items[UI_MAX_MENUITEMS];
	int numItems;

};

#endif // FRAMEWORK_H
