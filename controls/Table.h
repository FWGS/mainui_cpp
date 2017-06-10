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
#ifndef TABLE_H
#define TABLE_H

#include <BaseItem.h>
#include "utlvector.h"

enum ETableColumnType
{
	COLUMN_ERROR = -1,
	COLUMN_TEXT = 0, // char *
	COLUMN_ITEM      // CMenuBaseItem
};

enum
{
	COLFLAG_NONE = 0,
	// for future expansion
};

struct column_t
{
	ETableColumnType type;
	const char *name;  // can be NULL for no name
	float width; // sum of widths of all columns should be == 1
	int flags;
};

class CMenuTable : public CMenuBaseItem
{
public:
	CMenuTable();
	virtual ~CMenuTable();

	virtual void VidInit( void );
	virtual const char *Key( int key, int down );
	virtual void Draw( void );

	// Declare types in vararg, should be == num
	void SetupColumns(const int num, const column_t columns[] );

	// Pass pointers in vararg. Types should match column type, otherwise behaviour is undefined
	void AddRow( const void *first, ... );

	void *Cell( int row, int col );

	void RemoveRow( int rowNum );

	// Reset everything, drop column types and rows
	void Reset();

	ETableColumnType ColumnType( int col );

	int SelectedRow();

	void SetUpArrowPicture( const char *upArrow, const char *upArrowFocus, const char *upArrowPressed )
	{
		szUpArrow = upArrow;
		szUpArrowFocus = upArrowFocus;
		szUpArrowPressed = upArrowPressed;
	}

	void SetDownArrowPicture( const char *downArrow, const char *downArrowFocus, const char *downArrowPressed )
	{
		szDownArrow = downArrow;
		szDownArrowFocus = downArrowFocus;
		szDownArrowPressed = downArrowPressed;
	}

	const char *szBackground;

	CEventCallback onDeleteEntry;
	CEventCallback onActivateEntry;

// TODO: Move this out to ScrollView, if it there will be a need in ScrollView ;)
// scrollbar stuff // ADAMIX
	int		iScrollBarX;
	int		iScrollBarY;
	int		iScrollBarWidth;
	int		iScrollBarHeight;
	int		iScrollBarSliding;
// highlight
	int		iHighlight;

	bool    bFramedHintText;

	float   flScrollSpeed;

private:
	const char *szUpArrow, *szUpArrowFocus, *szUpArrowPressed;
	const char *szDownArrow, *szDownArrowFocus, *szDownArrowPressed;

	column_t *pColumns;
	int       iNumColumns;

	typedef void** row_t;

	CUtlVector<row_t> vRows;

	int iCurItem;
	int iNumRows; // visible row count

	double flScroll;
};

#endif // TABLE_H
