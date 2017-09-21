#include "Framework.h"
#include "PicButton.h"

CMenuFramework::CMenuFramework( const char *name ) : CMenuBaseWindow( name )
{
	iFlags = QMF_DISABLESCAILING;
}

void CMenuFramework::Show()
{
	CMenuPicButton::RootChanged( true );
	CMenuBaseWindow::Show();

	uiStatic.rootActive = this;
	uiStatic.rootPosition = uiStatic.menuDepth-1;
}

void CMenuFramework::Hide()
{
	int i;
	CMenuBaseWindow::Hide();

	for( i = uiStatic.menuDepth-1; i >= 0; i-- )
	{
		if( uiStatic.menuStack[i]->IsRoot() )
		{
			uiStatic.rootActive = uiStatic.menuStack[i];
			uiStatic.rootPosition = i;
			CMenuPicButton::RootChanged( false );
			return;
		}
	}


	// looks like we are have a modal or some window over game
	uiStatic.rootActive = NULL;
	uiStatic.rootPosition = 0;
}

void CMenuFramework::Init()
{
	CMenuBaseWindow::Init();
	pos.x = pos.y = 0;
	size.w = ScreenWidth;
	size.h = ScreenHeight;
}

bool CMenuFramework::DrawAnimation(EAnimation anim)
{
	return CMenuBaseWindow::DrawAnimation( anim );
}

