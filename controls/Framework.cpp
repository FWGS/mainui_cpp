/*
Framework.cpp -- base menu fullscreen root window
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
	m_scPos.x = m_scPos.y = pos.x = pos.y = 0;
	m_scSize.w = size.w = ScreenWidth;
	m_scSize.h = size.h = ScreenHeight;
}

void CMenuFramework::VidInit()
{
	m_scPos.x = m_scPos.y = pos.x = pos.y = 0;
	m_scSize.w = size.w = ScreenWidth;
	m_scSize.h = size.h = ScreenHeight;
	CMenuBaseWindow::VidInit();
}

bool CMenuFramework::DrawAnimation(EAnimation anim)
{
	return CMenuBaseWindow::DrawAnimation( anim );
}

