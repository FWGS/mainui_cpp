#include "BaseMenu.h"
#include "ClientWindow.h"

void CClientWindow::VidInit()
{
	size.w = 1024 - 100;
	size.h = 768 - 100;
	pos.x = (( ScreenWidth - uiStatic.scaleX * 1024 ) / 2) / uiStatic.scaleX + 50;
	pos.y = 50;

	BaseClass::VidInit();
	roundCornerSize = Size( 16, 16 ).Scale();

	iTitleHeight = 96 * uiStatic.scaleY;
	iGap = 2 * uiStatic.scaleY > 1 ? 4 * uiStatic.scaleY : 1;
}

void CClientWindow::Draw()
{
	UI_DrawPic( m_scPos, roundCornerSize, uiColorBlack, "gfx/vgui/round_corner_nw.tga", QM_DRAWTRANS );
	UI_DrawPic( m_scPos + Size( m_scSize.w - roundCornerSize.w, 0 ), roundCornerSize, uiColorBlack, "gfx/vgui/round_corner_ne.tga", QM_DRAWTRANS );
	UI_DrawPic( m_scPos + Size( 0, m_scSize.h - roundCornerSize.h ), roundCornerSize, uiColorBlack, "gfx/vgui/round_corner_sw.tga", QM_DRAWTRANS );
	UI_DrawPic( m_scPos + (m_scSize - roundCornerSize), roundCornerSize, uiColorBlack, "gfx/vgui/round_corner_se.tga", QM_DRAWTRANS );

	UI_DrawPic( m_scPos + roundCornerSize, roundCornerSize * 4, PackAlpha( uiPromptTextColor, 255 ), "gfx/vgui/CS_logo.tga", QM_DRAWTRANS );

	UI_DrawString( font, m_scPos + Size( roundCornerSize.w * 6, roundCornerSize.h ), Size( m_scSize.w - roundCornerSize.w * 6, roundCornerSize.h * 4 ),
		szName, PackAlpha( uiPromptTextColor, 255 ), m_scChSize, QM_LEFT, ETF_NOSIZELIMIT );

	UI_FillRect( m_scPos + Size( roundCornerSize.w, 0 ), Size( m_scSize.w - roundCornerSize.w * 2, roundCornerSize.h ), uiColorBlack );
	UI_FillRect( m_scPos + Size( 0, roundCornerSize.h ), Size( m_scSize.w, iTitleHeight - roundCornerSize.h ), uiColorBlack );

	UI_FillRect( m_scPos + Size( 0, iTitleHeight + iGap ), Size( m_scSize.w, m_scSize.h - roundCornerSize.h - iTitleHeight - iGap ), uiColorBlack );
	UI_FillRect( m_scPos + Size( roundCornerSize.w, m_scSize.h - roundCornerSize.h ), Size( m_scSize.w - roundCornerSize.w * 2, roundCornerSize.h ), uiColorBlack );

	BaseClass::Draw();
}

const char *CClientWindow::Key( int key, int down )
{
	if( down && key >= '0' && key <= '9' )
	{
		if( keys[key-'0'] )
		{
			(keys[key-'0'])( buttons[0] );
			return uiSoundNull;
		}
	}

	return BaseClass::Key( key, down );
}

CMenuAction *CClientWindow::AddButton( int key, const char *name, Point pos, CEventCallback callback )
{
	CMenuAction *act = new CMenuAction();

	act->pos = pos;
	act->onActivated = callback;
	act->SetBackground( 0U, PackRGBA( 255, 0, 0, 64 ) );
	if( *name == '&' ) // fast hack
		name++;
	act->szName = name;
	act->SetCharSize( QM_SMALLFONT );
	act->size = Size( 250, 32 );
	act->bDrawStroke = true;
	act->m_bLimitBySize = true;
	act->colorStroke = uiInputTextColor;
	act->iStrokeWidth = 1;

	if( key >= '0' && key <= '9' )
		keys[key - '0'] = callback;

	buttons[m_iNumBtns] = act;
	m_iNumBtns++;

	AddItem( act );

	return act;
}
