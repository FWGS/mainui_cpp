/*
Copyright (C) 1997-2001 Id Software, Inc.

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
#include "basemenu.h"
#include "utils.h"
#include "keydefs.h"
#include "menu_btnsbmp_table.h"

#define ART_BANNER	     	"gfx/shell/head_save"

#define ID_BACKGROUND	0
#define ID_BANNER		1
#define ID_SAVE		2
#define ID_DELETE		3
#define ID_CANCEL		4
#define ID_SAVELIST		5
#define ID_TABLEHINT	6
#define ID_LEVELSHOT	7
#define ID_MSGBOX	 	8
#define ID_MSGTEXT	 	9
#define ID_YES	 	130
#define ID_NO	 	131

#define LEVELSHOT_X		72
#define LEVELSHOT_Y		400
#define LEVELSHOT_W		192
#define LEVELSHOT_H		160

#define TIME_LENGTH		20
#define NAME_LENGTH		32+TIME_LENGTH
#define GAMETIME_LENGTH	15+NAME_LENGTH

class
{
	char		saveName[UI_MAXGAMES][CS_SIZE];
	char		delName[UI_MAXGAMES][CS_SIZE];
	char		saveDescription[UI_MAXGAMES][256];
	char		*saveDescriptionPtr[UI_MAXGAMES];

	public CMenuFramework

	CMenuBackgroundBitmap background;
	CMenuBannerBitmap banner;
	CMenuPicButton	save;
	CMenuPicButton	remove;
	CMenuPicButton	cancel;

	CMenuScrollList	savesList;

	CMenuBitmap	levelShot;
	CMenuAction	hintMessage;
	char		hintText[MAX_HINT_TEXT];

	// prompt dialog
	CMenuAction	msgBox;
	CMenuAction	promptMessage;
	CMenuPicButton	yes;
	CMenuPicButton	no;
} uiSaveGame_t;

static uiSaveGame_t		uiSaveGame;

/*
=================
UI_MsgBox_Ownerdraw
=================
*/
static void UI_MsgBox_Ownerdraw( void *self )
{
	menuCommon_s	*item = (menuCommon_s *)self;

	UI_FillRect( item->x, item->y, item->width, item->height, uiPromptBgColor );
}

static void UI_DeleteDialog( void )
{
	// toggle main menu between active\inactive
	// show\hide remove dialog
	uiSaveGame.save.iFlags ^= QMF_INACTIVE; 
	uiSaveGame.remove.iFlags ^= QMF_INACTIVE;
	uiSaveGame.cancel.iFlags ^= QMF_INACTIVE;
	uiSaveGame.savesList.iFlags ^= QMF_INACTIVE;

	uiSaveGame.msgBox.iFlags ^= QMF_HIDDEN;
	uiSaveGame.promptMessage.iFlags ^= QMF_HIDDEN;
	uiSaveGame.no.iFlags ^= QMF_HIDDEN;
	uiSaveGame.yes.iFlags ^= QMF_HIDDEN;

}

/*
=================
UI_SaveGame_KeyFunc
=================
*/
static const char *UI_SaveGame_KeyFunc( int key, int down )
{
	if( down && key == K_ESCAPE && uiSaveGame.save.iFlags & QMF_INACTIVE )
	{
		UI_DeleteDialog();
		return uiSoundNull;
	}
	return UI_DefaultKey( &uiSaveGame.menu, key, down );
}

/*
=================
UI_SaveGame_GetGameList
=================
*/
static void UI_SaveGame_GetGameList( void )
{
	char	comment[256];
	char	**filenames;
	int	i = 0, j, numFiles;

	filenames = FS_SEARCH( "save/*.sav", &numFiles, TRUE );

	// sort the saves in reverse order (oldest past at the end)
	qsort( filenames, numFiles, sizeof( char* ), (cmpfunc)COM_CompareSaves );

	if ( CL_IsActive() && !gpGlobals->demoplayback )
	{
		// create new entry for current save game
		strncpy( uiSaveGame.saveName[i], "new", CS_SIZE );
		StringConcat( uiSaveGame.saveDescription[i], "Current", TIME_LENGTH );
		AddSpaces( uiSaveGame.saveDescription[i], TIME_LENGTH ); // fill remaining entries
		StringConcat( uiSaveGame.saveDescription[i], "New Saved Game", NAME_LENGTH );
		AddSpaces( uiSaveGame.saveDescription[i], NAME_LENGTH );
		StringConcat( uiSaveGame.saveDescription[i], "New", GAMETIME_LENGTH );
		AddSpaces( uiSaveGame.saveDescription[i], GAMETIME_LENGTH );
		uiSaveGame.saveDescriptionPtr[i] = uiSaveGame.saveDescription[i];
		i++;
	}

	for ( j = 0; j < numFiles; i++, j++ )
	{
		if ( i >= UI_MAXGAMES )
			break;
		
		if ( !GET_SAVE_COMMENT( filenames[j], comment ))
		{
			if ( comment[0] )
			{
				// get name string even if not found - SV_GetComment can be mark saves
				// as <CORRUPTED> <OLD VERSION> etc
				AddSpaces( uiSaveGame.saveDescription[i], TIME_LENGTH );
				StringConcat( uiSaveGame.saveDescription[i], comment, NAME_LENGTH );
				AddSpaces( uiSaveGame.saveDescription[i], NAME_LENGTH );
				uiSaveGame.saveDescriptionPtr[i] = uiSaveGame.saveDescription[i];
				COM_FileBase( filenames[j], uiSaveGame.saveName[i] );
				COM_FileBase( filenames[j], uiSaveGame.delName[i] );
			}
			else uiSaveGame.saveDescriptionPtr[i] = NULL;
			continue;
		}

		// strip path, leave only filename (empty slots doesn't have savename)
		COM_FileBase( filenames[j], uiSaveGame.saveName[i] );
		COM_FileBase( filenames[j], uiSaveGame.delName[i] );

		// fill save desc
		StringConcat( uiSaveGame.saveDescription[i], comment + CS_SIZE, TIME_LENGTH );
		StringConcat( uiSaveGame.saveDescription[i], " ", TIME_LENGTH );
		StringConcat( uiSaveGame.saveDescription[i], comment + CS_SIZE + CS_TIME, TIME_LENGTH );
		AddSpaces( uiSaveGame.saveDescription[i], TIME_LENGTH ); // fill remaining entries
		StringConcat( uiSaveGame.saveDescription[i], comment, NAME_LENGTH );
		AddSpaces( uiSaveGame.saveDescription[i], NAME_LENGTH );
		StringConcat( uiSaveGame.saveDescription[i], comment + CS_SIZE + (CS_TIME * 2), GAMETIME_LENGTH );
		AddSpaces( uiSaveGame.saveDescription[i], GAMETIME_LENGTH );
		uiSaveGame.saveDescriptionPtr[i] = uiSaveGame.saveDescription[i];
	}

	for ( ; i < UI_MAXGAMES; i++ )
		uiSaveGame.saveDescriptionPtr[i] = NULL;

	uiSaveGame.savesList.itemNames = (const char **)uiSaveGame.saveDescriptionPtr;

	if ( uiSaveGame.saveName[0][0] == 0 || CL_IsActive() == FALSE )
		uiSaveGame.save.iFlags |= QMF_GRAYED;
	else uiSaveGame.save.iFlags &= ~QMF_GRAYED;

	if ( uiSaveGame.delName[0][0] == 0 )
		uiSaveGame.remove.iFlags |= QMF_GRAYED;
	else uiSaveGame.remove.iFlags &= ~QMF_GRAYED;
}

/*
=================
UI_SaveGame_Callback
=================
*/
static void UI_SaveGame_Callback( void *self, int event )
{
	menuCommon_s	*item = (menuCommon_s *)self;

	if( event == QM_CHANGED )
	{
		// never overwrite existing saves, because their names was never get collision
		if ( uiSaveGame.saveName[uiSaveGame.savesList.curItem][0] == 0 || CL_IsActive() == FALSE )
			uiSaveGame.save.iFlags |= QMF_GRAYED;
		else uiSaveGame.save.iFlags &= ~QMF_GRAYED;

		if ( uiSaveGame.delName[uiSaveGame.savesList.curItem][0] == 0 )
			uiSaveGame.remove.iFlags |= QMF_GRAYED;
		else uiSaveGame.remove.iFlags &= ~QMF_GRAYED;
		return;
	}

	if( event != QM_ACTIVATED )
		return;
	
	switch( item->id )
	{
	case ID_CANCEL:
		UI_PopMenu();
		break;
	case ID_SAVE:
		if( uiSaveGame.saveName[uiSaveGame.savesList.curItem][0] )
		{
			char	cmd[128];

			sprintf( cmd, "save/%s.bmp", uiSaveGame.saveName[uiSaveGame.savesList.curItem] );
			PIC_Free( cmd );

			sprintf( cmd, "save \"%s\"\n", uiSaveGame.saveName[uiSaveGame.savesList.curItem] );
			EngFuncs::ClientCmd( FALSE, cmd );
			UI_CloseMenu();
		}
		break;
	case ID_NO:
	case ID_DELETE:
		UI_DeleteDialog();
		break;
	case ID_YES:
		if( uiSaveGame.delName[uiSaveGame.savesList.curItem][0] )
		{
			char	cmd[128];
			sprintf( cmd, "killsave \"%s\"\n", uiSaveGame.delName[uiSaveGame.savesList.curItem] );

			EngFuncs::ClientCmd( TRUE, cmd );

			sprintf( cmd, "save/%s.bmp", uiSaveGame.delName[uiSaveGame.savesList.curItem] );
			PIC_Free( cmd );

			// restarts the menu
			UI_PopMenu();
			UI_SaveGame_Menu();
			return;
		}
		UI_DeleteDialog();
		break;
	}
}

/*
=================
UI_SaveGame_Ownerdraw
=================
*/
static void UI_SaveGame_Ownerdraw( void *self )
{
	menuCommon_s	*item = (menuCommon_s *)self;

	if( item->type != QMTYPE_ACTION && item->id == ID_LEVELSHOT )
	{
		int	x, y, w, h;

		// draw the levelshot
		x = LEVELSHOT_X;
		y = LEVELSHOT_Y;
		w = LEVELSHOT_W;
		h = LEVELSHOT_H;
		
		UI_ScaleCoords( &x, &y, &w, &h );

		if( uiSaveGame.saveName[uiSaveGame.savesList.curItem][0] )
		{
			char	saveshot[128];

			sprintf( saveshot, "save/%s.bmp", uiSaveGame.saveName[uiSaveGame.savesList.curItem] );

			if( !FILE_EXISTS( saveshot, TRUE ))
				UI_DrawPicAdditive( x, y, w, h, uiColorWhite, "{GRAF001" );
			else UI_DrawPic( x, y, w, h, uiColorWhite, saveshot );
		}
		else UI_DrawPicAdditive( x, y, w, h, uiColorWhite, "{GRAF001" );

		// draw the rectangle
		UI_DrawRectangle( item->x, item->y, item->width, item->height, uiInputFgColor );
	}
}

/*
=================
UI_SaveGame_Init
=================
*/
static void UI_SaveGame_Init( void )
{
	// memset( &uiSaveGame, 0, sizeof( uiSaveGame_t ));

	uiSaveGame.menu.vidInitFunc = UI_SaveGame_Init;
	uiSaveGame.menu.keyFunc = UI_SaveGame_KeyFunc;

	StringConcat( uiSaveGame.hintText, "Time", TIME_LENGTH );
	AddSpaces( uiSaveGame.hintText, TIME_LENGTH );
	StringConcat( uiSaveGame.hintText, "Game", NAME_LENGTH );
	AddSpaces( uiSaveGame.hintText, NAME_LENGTH );
	StringConcat( uiSaveGame.hintText, "Elapsed time", GAMETIME_LENGTH );
	AddSpaces( uiSaveGame.hintText, GAMETIME_LENGTH );

	uiSaveGame.background.generic.id = ID_BACKGROUND;
	uiSaveGame.background.generic.type = QMTYPE_BITMAP;
	uiSaveGame.background.iFlags = QMF_INACTIVE;
	uiSaveGame.background.generic.x = 0;
	uiSaveGame.background.generic.y = 0;
	uiSaveGame.background.generic.width = uiStatic.width;
	uiSaveGame.background.generic.height = 768;
	uiSaveGame.background.pic = ART_BACKGROUND;

	uiSaveGame.banner.generic.id = ID_BANNER;
	uiSaveGame.banner.generic.type = QMTYPE_BITMAP;
	uiSaveGame.banner.iFlags = QMF_INACTIVE|QMF_DRAW_ADDITIVE;
	uiSaveGame.banner.generic.x = UI_BANNER_POSX;
	uiSaveGame.banner.generic.y = UI_BANNER_POSY;
	uiSaveGame.banner.generic.width = UI_BANNER_WIDTH;
	uiSaveGame.banner.generic.height = UI_BANNER_HEIGHT;
	uiSaveGame.banner.SetPicture(ART_BANNER;

	uiSaveGame.save.generic.id = ID_SAVE;
	uiSaveGame.save.generic.type = QMTYPE_BM_BUTTON;
	uiSaveGame.save.iFlags = QMF_HIGHLIGHTIFFOCUS|QMF_DROPSHADOW;
	uiSaveGame.save.generic.x = 72;
	uiSaveGame.save.generic.y = 230;
	uiSaveGame.saveSetNameAndStatus( "Save";
	uiSaveGame.save.generic.statusText = "Save current game";
	uiSaveGame.save.generic.callback = UI_SaveGame_Callback;

	uiSaveGame.save.SetPicture( PC_SAVE_GAME );

	uiSaveGame.remove.generic.id = ID_DELETE;
	uiSaveGame.remove.generic.type = QMTYPE_BM_BUTTON;
	uiSaveGame.remove.iFlags = QMF_HIGHLIGHTIFFOCUS|QMF_DROPSHADOW;
	uiSaveGame.remove.generic.x = 72;
	uiSaveGame.remove.generic.y = 280;
	uiSaveGame.removeSetNameAndStatus( "Delete";
	uiSaveGame.remove.generic.statusText = "Delete saved game";
	uiSaveGame.remove.generic.callback = UI_SaveGame_Callback;

	uiSaveGame.remove.SetPicture( PC_DELETE );

	uiSaveGame.cancel.generic.id = ID_CANCEL;
	uiSaveGame.cancel.generic.type = QMTYPE_BM_BUTTON;
	uiSaveGame.cancel.iFlags = QMF_HIGHLIGHTIFFOCUS|QMF_DROPSHADOW;
	uiSaveGame.cancel.generic.x = 72;
	uiSaveGame.cancel.generic.y = 330;
	uiSaveGame.cancelSetNameAndStatus( "Cancel";
	uiSaveGame.cancel.generic.statusText = "Return back to main menu";
	uiSaveGame.cancel.generic.callback = UI_SaveGame_Callback;

	uiSaveGame.cancel.SetPicture( PC_CANCEL );

	uiSaveGame.hintMessage.generic.id = ID_TABLEHINT;
	uiSaveGame.hintMessage.generic.type = QMTYPE_ACTION;
	uiSaveGame.hintMessage.iFlags = QMF_INACTIVE|QMF_SMALLFONT;
	uiSaveGame.hintMessage.iColor = uiColorHelp;
	uiSaveGame.hintMessageSetNameAndStatus( uiSaveGame.hintText;
	uiSaveGame.hintMessage.generic.x = 360;
	uiSaveGame.hintMessage.generic.y = 225;

	uiSaveGame.levelShot.generic.id = ID_LEVELSHOT;
	uiSaveGame.levelShot.generic.type = QMTYPE_BITMAP;
	uiSaveGame.levelShot.iFlags = QMF_INACTIVE;
	uiSaveGame.levelShot.generic.x = LEVELSHOT_X;
	uiSaveGame.levelShot.generic.y = LEVELSHOT_Y;
	uiSaveGame.levelShot.generic.width = LEVELSHOT_W;
	uiSaveGame.levelShot.generic.height = LEVELSHOT_H;
	uiSaveGame.levelShot.generic.ownerdraw = UI_SaveGame_Ownerdraw;

	uiSaveGame.savesList.generic.id = ID_SAVELIST;
	uiSaveGame.savesList.generic.type = QMTYPE_SCROLLLIST;
	uiSaveGame.savesList.iFlags = QMF_HIGHLIGHTIFFOCUS|QMF_DROPSHADOW|QMF_SMALLFONT;
	uiSaveGame.savesList.generic.x = 360;
	uiSaveGame.savesList.generic.y = 255;
	uiSaveGame.savesList.generic.width = 640;
	uiSaveGame.savesList.generic.height = 440;
	uiSaveGame.savesList.generic.callback = UI_SaveGame_Callback;

	uiSaveGame.msgBox.generic.id = ID_MSGBOX;
	uiSaveGame.msgBox.generic.type = QMTYPE_ACTION;
	uiSaveGame.msgBox.iFlags = QMF_INACTIVE|QMF_HIDDEN;
	uiSaveGame.msgBox.generic.ownerdraw = UI_MsgBox_Ownerdraw; // just a fill rectangle
	uiSaveGame.msgBox.generic.x = DLG_X + 192;
	uiSaveGame.msgBox.generic.y = 256;
	uiSaveGame.msgBox.generic.width = 640;
	uiSaveGame.msgBox.generic.height = 256;

	uiSaveGame.promptMessage.generic.id = ID_MSGBOX;
	uiSaveGame.promptMessage.generic.type = QMTYPE_ACTION;
	uiSaveGame.promptMessage.iFlags = QMF_INACTIVE|QMF_DROPSHADOW|QMF_HIDDEN;
	uiSaveGame.promptMessageSetNameAndStatus( "Delete selected game?";
	uiSaveGame.promptMessage.generic.x = DLG_X + 315;
	uiSaveGame.promptMessage.generic.y = 280;

	uiSaveGame.yes.generic.id = ID_YES;
	uiSaveGame.yes.generic.type = QMTYPE_BM_BUTTON;
	uiSaveGame.yes.iFlags = QMF_HIGHLIGHTIFFOCUS|QMF_DROPSHADOW|QMF_HIDDEN;
	uiSaveGame.yesSetNameAndStatus( "Ok";
	uiSaveGame.yes.generic.x = DLG_X + 380;
	uiSaveGame.yes.generic.y = 460;
	uiSaveGame.yes.generic.callback = UI_SaveGame_Callback;

	uiSaveGame.yes.SetPicture( PC_OK );

	uiSaveGame.no.generic.id = ID_NO;
	uiSaveGame.no.generic.type = QMTYPE_BM_BUTTON;
	uiSaveGame.no.iFlags = QMF_HIGHLIGHTIFFOCUS|QMF_DROPSHADOW|QMF_HIDDEN;
	uiSaveGame.noSetNameAndStatus( "Cancel";
	uiSaveGame.no.generic.x = DLG_X + 530;
	uiSaveGame.no.generic.y = 460;
	uiSaveGame.no.generic.callback = UI_SaveGame_Callback;

	uiSaveGame.no.SetPicture( PC_CANCEL );

	UI_SaveGame_GetGameList();

	AddItem( background );
	AddItem( banner );
	AddItem( save );
	AddItem( remove );
	AddItem( cancel );
	AddItem( hintMessage );
	AddItem( levelShot );
	AddItem( savesList );
	AddItem( msgBox );
	AddItem( promptMessage );
	AddItem( no );
	AddItem( yes );
}

/*
=================
UI_SaveGame_Precache
=================
*/
void UI_SaveGame_Precache( void )
{
	PIC_Load( ART_BACKGROUND );
	PIC_Load( ART_BANNER );
}

/*
=================
UI_SaveGame_Menu
=================
*/
void UI_SaveGame_Menu( void )
{
	if( gMenu.m_gameinfo.gamemode == GAME_MULTIPLAYER_ONLY )
	{
		// completely ignore save\load menus for multiplayer_only
		return;
	}

	if( !CheckGameDll( )) return;

	UI_SaveGame_Precache();
	UI_SaveGame_Init();

	UI_PushMenu( &uiSaveGame.menu );
}
