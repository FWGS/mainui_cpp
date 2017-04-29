#pragma once
#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "ItemsHolder.h"
class CMenuBaseItem;

class CMenuFramework : public CMenuItemsHolder
{
public:
	CMenuFramework();

	void Show();
	void Hide();
	bool IsVisible();
	bool IsRoot() { return true; }

	virtual void SaveAndPopMenu();

	// Events library
	DECLARE_EVENT_TO_MENU_METHOD( CMenuFramework, SaveAndPopMenu );
};

#endif // FRAMEWORK_H
