/*
YesNoMessageBox.h - simple generic message box
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
#ifndef MENU_GENERICMSGBOX_H
#define MENU_GENERICMSGBOX_H

#include "ItemsHolder.h"
#include "PicButton.h"
#include "Action.h"

class CMenuYesNoMessageBox : public CMenuItemsHolder
{
public:
	CMenuYesNoMessageBox();

	virtual void _Init();
	virtual void _VidInit();
	virtual void Draw();
	virtual const char *Key( int key, int down );
	void SetMessage( const char *msg );
	void SetPositiveButton( const char *msg, int buttonPic, void *extra = NULL );
	void SetNegativeButton( const char *msg, int buttonPic, void *extra = NULL );
	void HighlightChoice( int ch ); // 0 - not hightlight, 1 - yes, 2 - no
	// void ToggleInactive();
	// void SetInactive( bool enable );

	// Pass pointer to messagebox to extra of calling object
	static void ToggleInactiveCb( CMenuBaseItem *, void *pExtra );

	CEventCallback onPositive;
	CEventCallback onNegative;
private:
	CMenuAction		dlgMessage1;
	CMenuPicButton	yes;
	CMenuPicButton	no;
	bool m_bSetYes, m_bSetNo;
};

#endif // MENU_GENERICMSGBOX_H
