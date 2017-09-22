/*
Zoo.cpp -- examples
Copyright (C) 2017 a1batross

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
#include "BaseWindow.h"
#include "Action.h"
#include "Field.h"

class CMenuZoo : public CMenuBaseWindow
{
public:
	virtual void _Init();
	virtual void _VidInit();

	static void OKButtonCommand( CMenuBaseItem *pSelf, void *pExtra )
	{
		pSelf->Parent()->Hide();
	}

	static void CancelButtonCommand( CMenuBaseItem *pSelf, void *pExtra )
	{
		pSelf->Parent()->Hide();
	}

private:
	CMenuAction OKButton;
	CMenuAction CancelButton;
	CMenuAction Label1;
	CMenuAction Label2;
	CMenuAction Label3;
	CMenuAction Label4;
	CMenuField Entry1;
	CMenuField Entry2;
	CMenuField Entry3;
	CMenuField Entry4;
	CMenuField Entry5;
} g_Zoo;

#define SET_TAG_BY_MEMBER_NAME( member ) \
	member.szTag = STR( member ); \
	AddItem( member );

void CMenuZoo::_Init()
{
	szTag = "CDKeyEntryDialog";
	iFlags = QMF_DIALOG;

	RegisterNamedEvent( OKButtonCommand, "Ok" );
	RegisterNamedEvent( CancelButtonCommand, "Cancel" );

	background.bForceColor = true;

	AddItem( background );
	SET_TAG_BY_MEMBER_NAME( OKButton );
	SET_TAG_BY_MEMBER_NAME( CancelButton );
	SET_TAG_BY_MEMBER_NAME( Label1 );
	SET_TAG_BY_MEMBER_NAME( Label2 );
	SET_TAG_BY_MEMBER_NAME( Label3 );
	SET_TAG_BY_MEMBER_NAME( Label4 );
	SET_TAG_BY_MEMBER_NAME( Entry1 );
	SET_TAG_BY_MEMBER_NAME( Entry2 );
	SET_TAG_BY_MEMBER_NAME( Entry3 );
	//SET_TAG_BY_MEMBER_NAME( Entry4 );
	//SET_TAG_BY_MEMBER_NAME( Entry5 );
}

void CMenuZoo::_VidInit()
{

}

void UI_Zoo_Precache()
{
	g_Zoo.SetResourceFilename("resource/CDKeyEntryDialog.res");
}

void UI_Zoo_Menu()
{
	UI_Zoo_Precache();
	g_Zoo.Show();
}
