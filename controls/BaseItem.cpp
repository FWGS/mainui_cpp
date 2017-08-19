#include "BaseItem.h"

/*
==================
CMenuBaseItem::CMenuBaseItem
==================
*/

CMenuBaseItem::CMenuBaseItem()
{
	SetNameAndStatus( "", NULL );
	SetCharSize( QM_DEFAULTFONT );
	SetCoord( 0, 0 );
	SetSize( 0, 0 );

	iFlags = 0;

	iColor = uiPromptTextColor;
	iFocusColor = uiPromptFocusColor;

	eTextAlignment = QM_TOPLEFT;
	eFocusAnimation = QM_NOFOCUSANIMATION;
	eLetterCase = QM_NOLETTERCASE;

	m_iLastFocusTime = 0;
	m_bPressed = false;

	m_pParent = NULL;

	m_bAllocName = false;
}

CMenuBaseItem::~CMenuBaseItem()
{
	if( m_bAllocName )
	{
		delete[] szName;
	}
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
	return 0;
}

void CMenuBaseItem::SetCharSize(EFontSizes fs)
{
	font = fs + 1; // It's guaranteed that handles will match font sizes

	switch( fs )
	{
	case QM_DEFAULTFONT:
		charSize.w = UI_MED_CHAR_WIDTH;
		charSize.h = UI_MED_CHAR_HEIGHT;
		break;
	case QM_SMALLFONT:
		charSize.w = UI_SMALL_CHAR_WIDTH;
		charSize.h = UI_SMALL_CHAR_HEIGHT;
		break;
	case QM_BIGFONT:
		charSize.w = UI_BIG_CHAR_WIDTH;
		charSize.h = UI_BIG_CHAR_HEIGHT;
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

// we need to remap position, because resource files are keep screen at 640x480, but we in 1024x768
#define REMAP_RATIO ( 1.6f )

bool CMenuBaseItem::KeyValueData(const char *key, const char *data)
{
	if( !strcmp( key, "xpos" ))
	{
		int coord;
		if( data[0] == 'c' ) // center
		{
			data++;

			coord = 320 + atoi( data );
		}
		else
		{
			coord = atoi( data );
			if( coord  < 0 )
				coord += 640;
		}
		pos.x = coord * REMAP_RATIO;
	}
	else if( !strcmp( key, "ypos" ) )
	{
		int coord;
		if( data[0] == 'c' ) // center
		{
			data++;

			coord = 240 + atoi( data );
		}
		else
		{
			coord = atoi( data );
			if( coord  < 0 )
				coord += 480;
		}
		pos.y = coord * REMAP_RATIO;
	}
	else if( !strcmp( key, "wide" ) )
	{
		size.w = atoi( data ) * REMAP_RATIO;
	}
	else if( !strcmp( key, "tall" ) )
	{
		size.h = atoi( data ) * REMAP_RATIO;
	}
	else if( !strcmp( key, "visible" ) )
	{
		SetVisibility( (bool) atoi( data ) );
	}
	else if( !strcmp( key, "enabled" ) )
	{
		bool enabled = (bool) atoi( data );

		SetInactive( !enabled );
		SetGrayed( !enabled );
	}
	else if( !strcmp( key, "labelText" ) )
	{
		/*if( *data == '#')
		{
			szName = Localize( data + 1 );
			if( szName == data + 1 ) // not localized
			{
				m_bAllocName = true;
			}
		}
		else*/ m_bAllocName = true;

		if( m_bAllocName )
		{
			char *name = new char[strlen( data ) + 1];
			strcpy( name, data );

			szName = name;
		}
	}
	else if( !strcmp( key, "textAlignment" ) )
	{
		if( !strcmp( data, "west" ) )
		{
			eTextAlignment = QM_LEFT;
		}
		else if( !strcmp( data, "east" ) )
		{
			eTextAlignment = QM_RIGHT;
		}
		else
		{
			Con_DPrintf( "KeyValueData: unknown textAlignment %s\n", data );
		}
	}
	else if( !strcmp( key, "command" ) )
	{
		CEventCallback ev;

		if( m_pParent && ( ev = m_pParent->FindEventByName( data )) )
		{
			onActivated = ev;

		}
		else if( !strcmp( data, "engine " ) )
		{
			onActivated.SetCommand( FALSE, data + sizeof( "engine " ) );
		}
		else
		{
			// should not happen, as parent parses the resource file and sends KeyValueData to every item inside
			// if this happens, there is a bug
			Con_DPrintf( "KeyValueData: cannot set command '%s' on '%s'\n", data, szName );
		}
	}
	// TODO: nomulti, nosingle, nosteam

	return true;
}
