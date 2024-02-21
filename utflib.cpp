/*
utflib.c - small unicode conversion library
Copyright (C) 2024 Alibek Omarov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#include "utflib.h"
#include "xash3d_types.h"

uint32_t Q_DecodeUTF8( utfstate_t *s, int in )
{
	// get character length
	if( s->len == 0 )
	{
		// init state
		s->uc = 0;

		// expect ASCII symbols by default
		if( likely( in <= 0x7f ))
			return in;

		// invalid sequence
		if( unlikely( in >= 0xf8 ))
			return 0;

		s->k = 0;

		if( in >= 0xf0 )
		{
			s->uc = in & 0x07;
			s->len = 3;
		}
		else if( in >= 0xe0 )
		{
			s->uc = in & 0x0f;
			s->len = 2;
		}
		else if( in >= 0xc0 )
		{
			s->uc = in & 0x1f;
			s->len = 1;
		}

		return 0;
	}

	// invalid sequence, reset
	if( unlikely( in > 0xbf ))
	{
		s->len = 0;
		return 0;
	}

	s->uc <<= 6;
	s->uc += in & 0x3f;
	s->k++;

	// sequence complete, reset and return code point
	if( likely( s->k == s->len ))
	{
		s->len = 0;
		return s->uc;
	}

	// feed more characters
	return 0;
}

uint32_t Q_DecodeUTF16( utfstate_t *s, int in )
{
	// get character length
	if( s->len == 0 )
	{
		// init state
		s->uc = 0;

		// expect simple case, after all decoding UTF-16 must be easy
		if( likely( in < 0xd800 || in > 0xdfff ))
			return in;

		s->uc = (( in - 0xd800 ) << 10 ) + 0x10000;
		s->len = 1;
		s->k = 0;

		return 0;
	}

	// invalid sequence, reset
	if( unlikely( in < 0xdc00 || in > 0xdfff ))
	{
		s->len = 0;
		return 0;
	}

	s->uc |= in - 0xdc00;
	s->k++;

	// sequence complete, reset and return code point
	if( likely( s->k == s->len ))
	{
		s->len = 0;
		return s->uc;
	}

	// feed more characters (should never happen with UTF-16)
	return 0;
}

size_t Q_EncodeUTF8( char dst[4], int ch )
{
	if( ch <= 0x7f )
	{
		dst[0] = ch;
		return 1;
	}
	else if( ch <= 0x7ff )
	{
		dst[0] = 0xc0 | (( ch >> 6 ) & 0x1f );
		dst[1] = 0x80 | (( ch ) & 0x3f );
		return 2;
	}
	else if( ch <= 0xffff )
	{
		dst[0] = 0xe0 | (( ch >> 12 ) & 0x0f );
		dst[1] = 0x80 | (( ch >> 6 ) & 0x3f );
		dst[2] = 0x80 | (( ch ) & 0x3f );
		return 3;
	}

	dst[0] = 0xf0 | (( ch >> 18 ) & 0x07 );
	dst[1] = 0x80 | (( ch >> 12 ) & 0x3f );
	dst[2] = 0x80 | (( ch >> 6 ) & 0x3f );
	dst[3] = 0x80 | (( ch ) & 0x3f );
	return 4;
}

size_t Q_UTF8Length( const char *s )
{
	size_t len = 0;
	utfstate_t state = { 0 };

	if( !s )
		return 0;

	for( ; *s; s++ )
	{
		uint32_t ch = Q_DecodeUTF8( &state, (int)*s );

		if( ch == 0 )
			continue;

		len++;
	}

	return len;
}

static size_t Q_CodepointLength( uint32_t ch )
{
	if( ch <= 0x7f )
		return 1;
	else if( ch <= 0x7ff )
		return 2;
	else if( ch <= 0xffff )
		return 3;

	return 4;
}

size_t Q_UTF16ToUTF8( char *dst, size_t dstsize, const uint16_t *src, size_t srcsize )
{
	utfstate_t state = { 0 };
	size_t dsti = 0, srci;

	if( !dst || !src || !dstsize || !srcsize )
		return 0;

	for( srci = 0; srci < srcsize && src[srci]; srci++ )
	{
		uint32_t ch;
		size_t len;

		ch = Q_DecodeUTF16( &state, src[srci] );

		if( ch == 0 )
			continue;

		len = Q_CodepointLength( ch );

		if( dsti + len + 1 > dstsize )
			break;

		dsti += Q_EncodeUTF8( &dst[dsti], ch );
	}

	dst[dsti] = 0;

	return dsti;
}
