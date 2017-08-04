#include "extdll.h"
#include "BaseMenu.h"
#include "Utils.h"

/*
==================
CMenuBaseItem::CMenuBaseItem
==================
*/

CMenuBaseItem::CMenuBaseItem()
{
	SetNameAndStatus( "", NULL );
	SetCharSize( UI_MED_CHAR_WIDTH, UI_MED_CHAR_HEIGHT );
	SetCoord( 0, 0 );
	SetSize( 0, 0 );

	iFlags = 0;

	iColor = uiPromptTextColor;
	iFocusColor = uiPromptFocusColor;

	eTextAlignment = QM_LEFT;
	eFocusAnimation = QM_NOFOCUSANIMATION;
	eLetterCase = QM_NOLETTERCASE;

	m_iLastFocusTime = 0;
	m_bPressed = false;

	m_pParent = NULL;
}

CMenuBaseItem::~CMenuBaseItem()
{
	;
}

void CMenuBaseItem::Init()
{
	;
}

void CMenuBaseItem::VidInit()
{
	CalcPosition();
	CalcSizes();
}

void CMenuBaseItem::Draw()
{
	;
}

void CMenuBaseItem::Char(int key)
{
	;
}

const char *CMenuBaseItem::Key(int key, int down)
{
	return uiSoundNull;
}

void CMenuBaseItem::SetCharSize(EFontSizes fs)
{
	switch( fs )
	{
	case QM_DEFAULTFONT:
		SetCharSize( UI_MED_CHAR_WIDTH, UI_MED_CHAR_HEIGHT );
		break;
	case QM_SMALLFONT:
		SetCharSize( UI_SMALL_CHAR_WIDTH, UI_SMALL_CHAR_HEIGHT );
		break;
	case QM_BIGFONT:
		SetCharSize( UI_BIG_CHAR_WIDTH, UI_BIG_CHAR_HEIGHT );
		break;
	}
}

/*
=================
CMenuBaseItem::Activate
=================
*/
const char *CMenuBaseItem::Activate( )
{
	_Event( QM_ACTIVATED );

	if( !( iFlags & QMF_SILENT ))
		return uiSoundMove;
	return 0;
}


void CMenuBaseItem::_Event( int ev )
{
	CEventCallback callback;

	switch( ev )
	{
	case QM_CHANGED:   callback = onChanged; break;
	case QM_PRESSED:   callback = onPressed; break;
	case QM_GOTFOCUS:  callback = onGotFocus; break;
	case QM_LOSTFOCUS: callback = onLostFocus; break;
	case QM_ACTIVATED:
		if( (bool)onActivatedClActive && CL_IsActive( ))
			callback = onActivatedClActive;
		else callback = onActivated;
		break;
	}

	if( callback ) callback( this );
}

bool CMenuBaseItem::IsCurrentSelected()
{
	if( m_pParent )
		return this == m_pParent->ItemAtCursor();
	return false;
}

void CMenuBaseItem::CalcPosition()
{
	m_scPos = pos.Scale();

	if( !IsAbsolutePositioned() && m_pParent )
		m_scPos += m_pParent->m_scPos;
}

void CMenuBaseItem::CalcSizes()
{
	m_scSize = size.Scale();
	m_scChSize = charSize.Scale();
}
