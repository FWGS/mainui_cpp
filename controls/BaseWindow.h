#pragma once
#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include "BaseItem.h"
#include "ItemsHolder.h"

class CMenuBaseWindow : public CMenuItemsHolder
{
public:
	virtual void Hide();
	virtual void Show();
	virtual bool IsVisible();

	virtual bool IsRoot() { return false; }

	virtual void SaveAndPopMenu();

	// Events library
	DECLARE_EVENT_TO_MENU_METHOD( CMenuBaseWindow, SaveAndPopMenu );
private:
	friend void UI_DrawMouseCursor( void ); // HACKHACK: Cursor should be set by menu item

	void PushMenu();
	void PopMenu();
};

#endif // BASEWINDOW_H
