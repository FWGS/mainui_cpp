/*
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

#include "extdll.h"
#include "BaseMenu.h"
#include "Utils.h"
#include "CFGScript.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "Editable.h"
#include "CheckBox.h"
#include "Slider.h"
#include "SpinControl.h"
#include "Field.h"
#include "ItemsHolder.h"
#include "dynamic/DynamicItemsHolder.h"

#define ART_BANNER_SERVER "gfx/shell/head_advoptions"
#define ART_BANNER_USER "gfx/shell/head_advoptions"

class CMenuScriptConfigPage : public CMenuItemsHolder
{
public:
	CMenuScriptConfigPage();
	~CMenuScriptConfigPage();

	bool IsItemFits( CMenuEditable &item );
	void PrepareItem( CMenuEditable &item );
	void Save();

private:
	int m_iCurrentHeight;
	int m_iPadding;
};

class CMenuScriptConfig : public CMenuFramework
{
public:
	CMenuScriptConfig();
	~CMenuScriptConfig();

	void SetScriptConfig( const char *path )
	{
		m_szConfig = path;
	}

	virtual void SaveAndPopMenu()
	{
		for( int i = m_iPagesIndex, j = 0; j < m_iPagesCount; i++, j++ )
			((CMenuScriptConfigPage*)m_pItems[i])->Save();

		CMenuFramework::SaveAndPopMenu();
	}

	void FlipMenu();

	DECLARE_EVENT_TO_MENU_METHOD( CMenuScriptConfig, FlipMenu )

	CMenuBackgroundBitmap background;
	CMenuBannerBitmap banner;
private:
	CMenuPicButton done;
	CMenuPicButton cancel;
	CMenuSpinControl pageSelector;

	void FreeItems( void );

	virtual void _Init();

	const char *m_szConfig;
	scrvardef_t *m_pVars;
	int m_iVarsCount;
	int m_iPagesIndex;
	int m_iPagesCount;
	int m_iCurrentPage;
};

CMenuScriptConfigPage::CMenuScriptConfigPage()
{
	m_iCurrentHeight = 0;
	m_iPadding = 32;
	SetRect( 360, 230, 660, 440 );
}

CMenuScriptConfigPage::~CMenuScriptConfigPage()
{
	for( int i = 0; i < m_numItems; i++ )
	{
		delete m_pItems[i];
	}
}

bool CMenuScriptConfigPage::IsItemFits(CMenuEditable &item)
{
	if( m_iCurrentHeight + item.size.h + m_iPadding >= size.h )
		return false;
	return true;
}

void CMenuScriptConfigPage::PrepareItem(CMenuEditable &item)
{
	item.SetCoord( pos.x, pos.y + m_iCurrentHeight );
	m_iCurrentHeight += item.size.h + m_iPadding;
}

void CMenuScriptConfigPage::Save()
{
	for( int i = 0; i < m_numItems; i++ )
	{
		((CMenuEditable*)m_pItems[i])->WriteCvar();
	}
}

CMenuScriptConfig::CMenuScriptConfig() :
	m_szConfig( NULL ), m_pVars( NULL ), m_iVarsCount( 0 )
{

}

CMenuScriptConfig::~CMenuScriptConfig()
{
	CSCR_FreeList( m_pVars );
	for( int i = m_iPagesIndex; i < m_iPagesIndex + m_iPagesCount; i++ )
	{
		delete m_pItems[i];
	}
}

void CMenuScriptConfig::_Init( void )
{
	done.SetNameAndStatus( "Done", "Save and Go back to previous menu" );
	done.SetPicture( PC_DONE );
	done.SetCoord( 72, 230 );
	done.onActivated = SaveAndPopMenuCb;

	cancel.SetPicture( PC_CANCEL );
	cancel.SetNameAndStatus( "Cancel", "Go back to previous menu" );
	cancel.SetCoord( 72, 280 );
	cancel.onActivated = HideCb;

	pageSelector.SetRect( 780, 180, 160, 32 );

	AddItem( background );
	AddItem( banner );
	AddItem( done );
	AddItem( cancel );
	AddItem( pageSelector );

	m_pVars = CSCR_LoadDefaultCVars( m_szConfig, &m_iVarsCount );

	if( !m_pVars )
		return; // Show "Unavailable" label?


	CMenuScriptConfigPage *page = new CMenuScriptConfigPage;
	page->SetRect( 340, 255, 660, 500 );
	page->Show();
	m_iCurrentPage = 0;
	m_iPagesCount = 1;
	m_iPagesIndex = m_numItems;
	AddItem( page );

	for( scrvardef_t *var = m_pVars; var; var = var->next )
	{
		CMenuEditable *editable;
		CMenuEditable::cvarType_e cvarType;

		// TODO: Maybe ignore here "hostname", "sv_password", "maxplayers" stuff?

		switch( var->type )
		{
		case T_BOOL:
		{
			CMenuCheckBox *checkbox = new CMenuCheckBox;

			editable = checkbox;
			cvarType = CMenuEditable::CVAR_VALUE;
			break;
		}
		case T_NUMBER:
		{
			CMenuSpinControl *spinControl = new CMenuSpinControl;
			float fMin, fMax;

			if( var->fMin == -1 ) fMin = -9999;
			else fMin = var->fMin;


			if( var->fMax == -1 ) fMax = 9999;
			else fMax = var->fMax;

			spinControl->Setup( fMin, fMax, 1 );
			spinControl->SetSize( 200, 32 );
			editable = spinControl;

			cvarType = CMenuEditable::CVAR_VALUE;
			break;
		}
		case T_STRING:
		{
			CMenuField *field = new CMenuField;
			field->iMaxLength = CS_SIZE;
			editable = field;
			cvarType = CMenuEditable::CVAR_STRING;
			break;
		}
#if 0 // Parser not done yet!
		case T_LIST:
		{
			CMenuSpinControl *spinControl = new CMenuSpinControl;

			spinControl->Setup( var->szStringValues, var->iStringValuesCount );
			editable = spinControl;
			cvarType = CMenuEditable::CVAR_STRING;
			break;
		}
#endif
		default: continue;
		}

		editable->iFlags |= QMF_NOTIFY;
		// editable->szName = var->name;
		editable->szStatusText = var->desc;
		editable->SetCharSize( QM_DEFAULTFONT );
		editable->LinkCvar( var->name, cvarType );
		editable->iFlags &= ~(QMF_GRAYED|QMF_INACTIVE);
		editable->Show();

		// create new page
		if( !page->IsItemFits( *editable ) )
		{
			page = new CMenuScriptConfigPage;
			page->Hide();
			page->SetRect( 340, 255, 660, 440 );

			AddItem( page );
			m_iPagesCount++;
		}

		page->PrepareItem( *editable );
		page->AddItem( editable );
	}


	pageSelector.SetInactive(false);
	pageSelector.Setup( 1, m_iPagesCount, 1 );
	pageSelector.SetCurrentValue( 1 );
	pageSelector.onChanged = FlipMenuCb;
}

void CMenuScriptConfig::FlipMenu()
{
	int newIndex = (int)pageSelector.GetCurrentValue() - 1;

	CMenuScriptConfigPage *oldPage = *((CMenuScriptConfigPage**)m_pItems + m_iPagesIndex + m_iCurrentPage);
	CMenuScriptConfigPage *newPage = *((CMenuScriptConfigPage**)m_pItems + m_iPagesIndex + newIndex);

	oldPage->Hide();
	newPage->Show();

	m_iCurrentPage = newIndex;
}


void UI_AdvServerOptions_Menu()
{
	static CMenuScriptConfig staticServerOptions;
	staticServerOptions.SetScriptConfig( "settings.scr" );
	staticServerOptions.banner.SetPicture( ART_BANNER_SERVER );
	staticServerOptions.Show();
}

void UI_AdvUserOptions_Menu()
{
	static CMenuScriptConfig staticUserOptions;
	staticUserOptions.SetScriptConfig( "user.scr" );
	staticUserOptions.banner.SetPicture( ART_BANNER_USER );
	staticUserOptions.Show();
}
