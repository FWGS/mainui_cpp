/*
Color.cpp -- color class
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

#pragma once
#ifndef COLOR_H
#define COLOR_H

class CColor
{
public:
	CColor( ) : rgba( 0 ), init( false ) { }
	CColor( unsigned int rgba ) : rgba( rgba ), init( false ) { }
	CColor( int rgba ) : rgba( (unsigned int)rgba ), init( false ) { }

	unsigned int operator =( unsigned int color )
	{
		rgba = color;
		init = true;
		return rgba;
	}
	int operator =( int color )
	{
		rgba = (unsigned int)color;
		init = true;
		return color;
	}

	operator unsigned int() { return rgba; }
	operator int() { return (int)rgba; }

	unsigned int rgba;

	// get rid of this someday
	bool IsOk() { return init; }
private:
	bool init;
};

#endif // COLOR_H
