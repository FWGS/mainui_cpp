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

#include "extdll.h"
#include "BaseMenu.h"
#include "Action.h"
#include "PicButton.h"
#include "YesNoMessageBox.h"
#include "Utils.h"

static void ToggleInactiveInternalCb( CMenuBaseItem *pSelf, void *pExtra );

CMenuYesNoMessageBox::CMenuYesNoMessageBox()
{
	iFlags = QMF_INACTIVE|QMF_HIDDEN|QMF_DIALOG;
	dlgMessage1.iFlags = QMF_INACTIVE|QMF_DROPSHADOW|QMF_HIDDEN|QMF_DIALOG;
	dlgMessage1.eTextAlignment = QM_CENTER;

	yes.iFlags = no.iFlags = QMF_DROPSHADOW|QMF_HIDDEN|QMF_DIALOG;
	yes.onActivated.pExtra = no.onActivated.pExtra = this;
	yes.bEnableTransitions = no.bEnableTransitions = false;

	SET_EVENT( yes, onActivated )
	{
		CMenuYesNoMessageBox *msgBox = (CMenuYesNoMessageBox*)pExtra;
		msgBox->onPositive( msgBox );
	}
	END_EVENT( yes, onActivated )

	SET_EVENT( no, onActivated )
	{
		CMenuYesNoMessageBox *msgBox = (CMenuYesNoMessageBox*)pExtra;
		msgBox->onNegative( msgBox );
	}
	END_EVENT( no, onActivated )

	m_bSetYes = m_bSetNo = false;
}

/*
==============
CMenuYesNoMessageBox::Init
==============
*/
void CMenuYesNoMessageBox::Init( void )
{
	if( !m_bSetYes )
		SetPositiveButton( "Ok", PC_OK );

	if( !m_bSetNo )
		SetNegativeButton( "Cancel", PC_CANCEL );

	if( !onNegative )
		onNegative = ToggleInactiveInternalCb;

	if( !onPositive )
		onPositive = ToggleInactiveInternalCb;

	m_pParent->AddItem( dlgMessage1 );
	m_pParent->AddItem( yes );
	m_pParent->AddItem( no );
}

/*
==============
CMenuYesNoMessageBox::VidInit
==============
*/
void CMenuYesNoMessageBox::VidInit( void )
{
	dlgMessage1.SetRect( DLG_X + 192, 280, 640, 256 );
	yes.SetRect( DLG_X + 380, 460, UI_BUTTONS_WIDTH / 2, UI_BUTTONS_HEIGHT );
	no.SetRect( DLG_X + 530, 460, UI_BUTTONS_WIDTH / 2, UI_BUTTONS_HEIGHT );
	SetRect( DLG_X + 192, 256, 640, 256 );

	dlgMessage1.SetCharSize( UI_MED_CHAR_WIDTH, UI_MED_CHAR_HEIGHT );

	CMenuAction::VidInit(); // setup rect properly
}

/*
==============
CMenuYesNoMessageBox::Draw
==============
*/
void CMenuYesNoMessageBox::Draw( void )
{
	UI_FillRect( m_scPos, m_scSize, uiPromptBgColor );
}

/*
==============
CMenuYesNoMessageBox::Key
==============
*/
const char *CMenuYesNoMessageBox::Key(int key, int down)
{
	if( key == K_ESCAPE && down )
	{
		onNegative( this );
		return uiSoundNull;
	}
	else return CMenuAction::Key( key, down );
}

/*
==============
CMenuYesNoMessageBox::SetMessage
==============
*/
void CMenuYesNoMessageBox::SetMessage( const char *msg )
{
	dlgMessage1.szName = ( msg );
}

/*
==============
CMenuYesNoMessageBox::SetPositiveButton
==============
*/
void CMenuYesNoMessageBox::SetPositiveButton( const char *msg, int buttonPic, void *extra )
{
	m_bSetYes = true;
	yes.szName = msg;
	yes.SetPicture( buttonPic );
	onPositive.pExtra = extra;
}

/*
==============
CMenuYesNoMessageBox::SetNegativeButton
==============
*/
void CMenuYesNoMessageBox::SetNegativeButton( const char *msg, int buttonPic, void *extra )
{
	m_bSetNo = true;
	no.szName = msg;
	no.SetPicture( buttonPic );
	onNegative.pExtra = extra;
}

/*
==============
CMenuYesNoMessageBox::HighlightChoice
==============
*/
void CMenuYesNoMessageBox::HighlightChoice( int yesno )
{
	if( yesno == 1 )
	{
		yes.bPulse = true;
		no.bPulse = false;
	}
	else if( yesno == 2 )
	{
		yes.bPulse = false;
		no.bPulse = true;
	}
	else
	{
		yes.bPulse = no.bPulse = 0;
	}
}

/*
==============
CMenuYesNoMessageBox::ToggleInactive
==============
*/
void CMenuYesNoMessageBox::ToggleInactive(void)
{
	yes.iFlags ^= QMF_HIDDEN;
	no.iFlags ^= QMF_HIDDEN;
	dlgMessage1.iFlags ^= QMF_HIDDEN;
	iFlags ^= QMF_HIDDEN;
}

void CMenuYesNoMessageBox::SetInactive(bool enable)
{
	if( enable )
	{
		yes.iFlags |= QMF_HIDDEN;
		no.iFlags |= QMF_HIDDEN;
		dlgMessage1.iFlags |= QMF_HIDDEN;
		iFlags |= QMF_HIDDEN;
	}
	else
	{
		yes.iFlags &= ~(QMF_HIDDEN);
		no.iFlags &= ~(QMF_HIDDEN);
		dlgMessage1.iFlags &= ~(QMF_HIDDEN);
		iFlags &= ~(QMF_HIDDEN);
	}
}

/*
==============
CMenuYesNoMessageBox::ToggleInactiveCb
==============
*/
void CMenuYesNoMessageBox::ToggleInactiveCb( CMenuBaseItem *, void *pExtra )
{
	ToggleInactiveInternalCb( (CMenuBaseItem*)pExtra, NULL );
}

/*
==============
CMenuYesNoMessageBox::ToggleInactiveCb
==============
*/
static void ToggleInactiveInternalCb( CMenuBaseItem *pSelf, void * )
{
	pSelf->Parent()->ToggleItemsInactive();
	pSelf->ToggleInactive();
}
