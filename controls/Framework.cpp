#include "extdll.h"
#include "BaseMenu.h"
#include "Utils.h"
#include "PicButton.h"

CMenuFramework::CMenuFramework() : CMenuItemsHolder()
{
	SetCoord( 0, 0 );
}

void CMenuFramework::SaveAndPopMenu( )
{
	EngFuncs::ClientCmd( FALSE, "trysaveconfig\n" );
	Hide();
}

void CMenuFramework::Show()
{
	CMenuItemsHolder::Show();

	uiStatic.rootActive = this;
}

void CMenuFramework::Hide()
{
	int i;
	CMenuItemsHolder::Hide();

	for( i = uiStatic.menuDepth-1; i >= 0; i-- )
	{
		if( uiStatic.menuStack[i]->IsRoot() )
		{
			uiStatic.rootActive = uiStatic.menuStack[i];
			return;
		}
	}

	// looks like we are have a modal or some window over game
	uiStatic.rootActive = NULL;
}

bool CMenuFramework::IsVisible()
{
	return this == uiStatic.rootActive;
}

