/*
Table.cpp - table
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
#include <stdarg.h>
#include "extdll_menu.h"
#include "BaseMenu.h"
#include "Table.h"
#include "Utils.h"
#include "Scissor.h"

CMenuTable::CMenuTable() : CMenuBaseItem()
{
	// iCurItem = 0;
	iHighlight = -1;

	SetUpArrowPicture( UI_UPARROW, UI_UPARROWFOCUS, UI_UPARROWPRESSED );
	SetDownArrowPicture( UI_DOWNARROW, UI_DOWNARROWFOCUS, UI_DOWNARROWPRESSED );

	iNumColumns = 0;
	pColumns = NULL;

	flScroll = 0.0;

	eFocusAnimation = QM_HIGHLIGHTIFFOCUS;
	SetCharSize( QM_SMALLFONT );

	bFramedHintText = true;
}

CMenuTable::~CMenuTable()
{
	Reset();
}

void CMenuTable::VidInit()
{
	CalcPosition();
	CalcSizes();

	if( m_scChSize.h )
	{
		iNumRows = ( m_scSize.h / m_scChSize.h ) - 2;
		if( iNumRows > vRows.Count() )
			iNumRows = vRows.Count();
	}

	Point p = pos;
	Size sz;

	for( int i = 0; i < vRows.Count(); i++ )
	{

		for( int j = 0; j < iNumColumns; j++ )
		{
			if( pColumns[j].type != COLUMN_ITEM )
				continue;

			CMenuBaseItem *item = (CMenuBaseItem*)vRows[i][j];

			item->pos = p;
			item->size = sz;
			item->VidInit();
		}
	}
}

const char *CMenuTable::Key(int key, int down)
{
	const char *sound = NULL;

	switch( key )
	{
	case K_DOWNARROW:
		if( ++iCurItem < vRows.Count() )
		{
			sound = uiSoundMove;
		}
		break;
	case K_UPARROW:
		if( --iCurItem >= 0 )
		{
			sound = uiSoundMove;
		}
		break;
	default:
	{
		if( vRows.IsValidIndex( iCurItem ) )
		{
			for( int j = 0; j < iNumColumns; j++ )
			{
				if( pColumns[j].type != COLUMN_ITEM )
					continue;

				sound = ((CMenuBaseItem*)vRows[iCurItem][j])->Key( key, down );

				if( sound && sound != uiSoundNull )
					break;
			}
		}
	}
	}

	return sound;
}

void CMenuTable::Draw()
{
	UI::Scissor::PushScissor( m_scPos, m_scSize );

	int minVisible = vRows.Count() - iNumRows;
	int start;
	int end;

	if( minVisible < 0 )
	{
		start = flScroll * ( vRows.Count() - iNumRows );
		end = start + iNumRows;

		// it's abnormal, but check anyway
		if( end > vRows.Count() )
		{
			end = vRows.Count();
		}
	}
	else
	{
		start = 0;
		end = vRows.Count();
	}

	Point p = m_scPos;
	Size sz( 0, m_scChSize.h + 4 * uiStatic.scaleY );

	for( int i = start; i < end; i++ )
	{
		for( int j = 0; j < iNumColumns; j++ )
		{
			ETableColumnType type = pColumns[j].type;
			sz.w = pColumns[j].width * m_scSize.w;

			switch( type )
			{
			case COLUMN_TEXT:
			{
				UI_DrawString( font, p, sz, (const char *)vRows[i][j], uiColorHelp, 0, m_scChSize, QM_LEFT, false );
				break;
			}
			case COLUMN_ITEM:
			{
				CMenuBaseItem *item = (CMenuBaseItem*)vRows[i][j];

				item->Draw();
			}
			}

			p.x += sz.w;
		}
		p.x = m_scPos.x;
		p.y += sz.h;
	}

	UI::Scissor::PopScissor();
}

void CMenuTable::SetupColumns(const int num, const column_t columns[])
{
	iNumColumns = num;
	pColumns = new column_t[num];
	memcpy( pColumns, columns, sizeof( column_t ) * num );
}

void CMenuTable::AddRow( const void *first, ... )
{
	row_t row = new void*[iNumColumns];
	row[0] = (void*)first;

	va_list args;
	va_start( args, first );
	for( int i = 1; i < iNumColumns; i++ )
	{
		row[i] = va_arg( args, void* );
	}
	va_end( args );

	vRows.AddToTail( row );
}

void CMenuTable::RemoveRow(int rowNum)
{
	if( vRows.IsValidIndex( rowNum ))
		vRows.FastRemove( rowNum );
}

void CMenuTable::Reset()
{
	for( int i = 0; i < vRows.Count(); i++ )
	{
		delete[] vRows[i];
	}

	vRows.RemoveAll();
}

void *CMenuTable::Cell(int row, int col)
{
	if( vRows.IsValidIndex( row ) && col < iNumColumns && col >= 0 )
	{
		return vRows[row][col];
	}
	return NULL;
}

ETableColumnType CMenuTable::ColumnType(int col)
{
	if( col >= iNumColumns || col < 0 )
		return COLUMN_ERROR;

	return pColumns[col].type;
}

int CMenuTable::SelectedRow()
{
	return iCurItem;
}


