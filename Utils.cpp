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


// ui_qmenu.c -- Quake menu framework


#include "extdll.h"
#include "BaseMenu.h"
#include "Utils.h"
#include "keydefs.h"
#include "BtnsBMPTable.h"

#ifdef _DEBUG
void DBG_AssertFunction( BOOL fExpr, const char* szExpr, const char* szFile, int szLine, const char* szMessage )
{
	if( fExpr ) return;

	char szOut[512];
	if( szMessage != NULL )
		sprintf( szOut, "ASSERT FAILED:\n %s \n(%s@%d)\n%s", szExpr, szFile, szLine, szMessage );
	else sprintf( szOut, "ASSERT FAILED:\n %s \n(%s@%d)", szExpr, szFile, szLine );
	HOST_ERROR( szOut );
}
#endif	// DEBUG

void AddSpaces(char *s, int size, int buffersize)
{
	int len = strlen(s);

	size += len - ColorStrlen(s);
	if( size > buffersize )
		size = buffersize;

	while( len < size - 1 )
	{	
		s[len] = ' ';
		len++;
	}
	s[len] = '\0';
}

int ColorStrlen( const char *str )
{
	const char *p;

	if( !str )
		return 0;

	int len = 0;
	p = str;
	EngFuncs::UtfProcessChar( 0 );
	while( *p )
	{
		if( IsColorString( p ))
		{
			p += 2;
			continue;
		}

		p++;
		if( EngFuncs::UtfProcessChar( (unsigned char) *p ) )
			len++;
	}

	return len;
}

int ColorPrexfixCount( const char *str )
{
	const char *p;

	if( !str )
		return 0;

	int len = 0;
	p = str;

	while( *p )
	{
		if( IsColorString( p ))
		{
			len += 2;
			p += 2;
			continue;
		}
		p++;
	}

	return len;
}

void StringConcat( char *dst, const char *src, size_t size )
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = size;
	size_t dlen;

	if( !dst || !src || !size )
		return;

#if 0
	// find the end of dst and adjust bytes left but don't go past end
	while(n-- != 0 && *d != '\0') d++;
	dlen = d - dst;
#else
	// VERY UNSAFE. SURE THAT DST IS BIG
	dlen = ColorStrlen( dst );
	d += strlen( dst );
#endif

	n = size - dlen;

	if ( n == 0 ) return;
	while ( *s != '\0' )
	{
		if ( n != 1 )
		{
			*d++ = *s;
			n--;
		}
		s++;
	}

	*d = '\0';
	return;
}

char *StringCopy( const char *input )
{
	if( !input ) return NULL;

	char *out = (char *)MALLOC( strlen( input ) + 1 );
	strcpy( out, input );

	return out;
}

/*
============
COM_CompareSaves
============
*/
int COM_CompareSaves( const void **a, const void **b )
{
	char *file1, *file2;

	file1 = (char *)*a;
	file2 = (char *)*b;

	int bResult;

	EngFuncs::CompareFileTime( file2, file1, &bResult );

	return bResult;
}

/*
============
COM_FileBase
============
*/
// Extracts the base name of a file (no path, no extension, assumes '/' as path separator)
void COM_FileBase ( const char *in, char *out )
{
	int len, start, end;

	len = strlen( in );
	
	// scan backward for '.'
	end = len - 1;
	while ( end && in[end] != '.' && in[end] != '/' && in[end] != '\\' )
		end--;
	
	if ( in[end] != '.' )		// no '.', copy to end
		end = len-1;
	else 
		end--;			// Found ',', copy to left of '.'


	// Scan backward for '/'
	start = len-1;
	while ( start >= 0 && in[start] != '/' && in[start] != '\\' )
		start--;

	if ( in[start] != '/' && in[start] != '\\' )
		start = 0;
	else 
		start++;

	// Length of new sting
	len = end - start + 1;

	// Copy partial string
	strncpy( out, &in[start], len );
	// Terminate it
	out[len] = 0;
}

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
const char *Info_ValueForKey( const char *s, const char *key )
{
	char	pkey[MAX_INFO_STRING];
	static	char value[2][MAX_INFO_STRING]; // use two buffers so compares work without stomping on each other
	static	int valueindex;
	char	*o;
	
	valueindex ^= 1;
	if( *s == '\\' ) s++;
	printf("I_VFK '%s' '%s'\n", s, key );

	while( 1 )
	{
		o = pkey;
		while( *s != '\\' && *s != '\n' )
		{
			if( !*s ) return "";
			*o++ = *s++;
		}

		*o = 0;
		s++;

		o = value[valueindex];

		while( *s != '\\' && *s != '\n' && *s )
		{
			if( !*s ) return "";
			*o++ = *s++;
		}
		*o = 0;

		if( !strcmp( key, pkey ))
			return value[valueindex];
		if( !*s ) return "";
		s++;
	}
}


/* 
===================
Key_GetKey
===================
*/
int KEY_GetKey( const char *binding )
{
	const char *b;

	if ( !binding )
		return -1;

	for ( int i = 0; i < 256; i++ )
	{
		b = EngFuncs::KEY_GetBinding( i );
		if( !b ) continue;

		if( !stricmp( binding, b ))
			return i;
	}
	return -1;
}

/*
================
UI_FadeAlpha
================
*/
int UI_FadeAlpha( int starttime, int endtime )
{
	int	time, fade_time;

	if( starttime == 0 )
	{
		return 0xFFFFFFFF;
	}

	time = ( gpGlobals->time * 1000 ) - starttime;

	if( time >= endtime )
	{
		return 0x00FFFFFF;
	}

	// fade time is 1/4 of endtime
	fade_time = endtime / 4;
	fade_time = bound( 300, fade_time, 10000 );

	int alpha;

	// fade out
	if(( endtime - time ) < fade_time )
		alpha = bound( 0, (( endtime - time ) * 1.0f / fade_time ) * 255, 255 );
	else alpha = 255;

	return PackRGBA( 255, 255, 255, alpha );
}

void UI_EnableTextInput( bool enable )
{
	uiStatic.textInput = enable;
	EngFuncs::EnableTextInput( enable );
}

void *operator new( size_t a )
{
	return MALLOC( a );
}

void *operator new[]( size_t a )
{
	return MALLOC( a );
}

void operator delete( void *ptr )
{
	if( ptr ) FREE( ptr );
}

void operator delete[]( void *ptr )
{
	if( ptr ) FREE( ptr );
}

#define BI_SIZE	40 //size of bitmap info header.

typedef unsigned short       word;

typedef struct
{
	//char	id[2];		// bmfh.bfType
	uint	fileSize;		// bmfh.bfSize
	uint	reserved0;	// bmfh.bfReserved1 + bmfh.bfReserved2
	uint	bitmapDataOffset;	// bmfh.bfOffBits
	uint	bitmapHeaderSize;	// bmih.biSize
	uint	width;		// bmih.biWidth
	int	height;		// bmih.biHeight
	word	planes;		// bmih.biPlanes
	word	bitsPerPixel;	// bmih.biBitCount
	uint	compression;	// bmih.biCompression
	uint	bitmapDataSize;	// bmih.biSizeImage
	uint	hRes;		// bmih.biXPelsPerMeter
	uint	vRes;		// bmih.biYPelsPerMeter
	uint	colors;		// bmih.biClrUsed
	uint	importantColors;	// bmih.biClrImportant
} bmp_t;

byte *UI::Graphics::MakeBMP( unsigned int w, unsigned int h, byte **ptr, int *size, int *texOffset )
{
	bmp_t bhdr;

	if( !ptr || !size )
		return NULL;

	int biTrueWidth = (w + 3) & ~3;
	const char magic[2] = { 'B', 'M' };
	const int pixel_size = 4; // create 32 bit image everytime
	const size_t cbPalBytes = 0; // TODO: calculate it, when it would be needed to create BMP with palette
	size_t cbBmpBits = biTrueWidth * h * pixel_size;

	bhdr.fileSize = sizeof( magic ) + sizeof( bmp_t ) + cbBmpBits + cbPalBytes;
	bhdr.reserved0 = 0;
	bhdr.bitmapDataOffset = sizeof( magic ) + sizeof( bmp_t ) + cbPalBytes;
	bhdr.bitmapHeaderSize = BI_SIZE;
	bhdr.width = biTrueWidth;
	bhdr.height = h;
	bhdr.planes = 1;
	bhdr.bitsPerPixel = pixel_size * 8;
	bhdr.compression = 0;
	bhdr.bitmapDataSize = cbBmpBits;
	bhdr.hRes = bhdr.vRes = 0;
	bhdr.colors = ( pixel_size == 1 ) ? 256 : 0;
	bhdr.importantColors = 0;

	*ptr = new byte[bhdr.fileSize];
	*size = bhdr.fileSize;

	if( texOffset )
		*texOffset = bhdr.bitmapDataOffset;

	memcpy( *ptr, magic, sizeof( magic ) );
	memcpy( *ptr + sizeof( magic ), &bhdr, sizeof( bhdr ) );

	byte *rgbdata = *ptr + bhdr.bitmapDataOffset;

	memset( rgbdata, 0, cbBmpBits );

	return rgbdata;
}


wchar_t *UI::String::ConvertToWChar( const char *text, int *length )
{
	int len = strlen( text );
	wchar_t *wText = new wchar_t[len+1];
	mbstowcs( wText, text, len );
	wText[len] = 0;

	if( length ) *length = len;

	return wText;
}
