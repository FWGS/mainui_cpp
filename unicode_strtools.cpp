/*
*
*    This program is free software; you can redistribute it and/or modify it
*    under the terms of the GNU General Public License as published by the
*    Free Software Foundation; either version 2 of the License, or (at
*    your option) any later version.
*
*    This program is distributed in the hope that it will be useful, but
*    WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*    In addition, as a special exception, the author gives permission to
*    link the code of this program with the Half-Life Game Engine ("HL
*    Engine") and Modified Game Libraries ("MODs") developed by Valve,
*    L.L.C ("Valve").  You must obey the GNU General Public License in all
*    respects for all of the code used other than the HL Engine and MODs
*    from Valve.  If you modify this file, you may extend this exception
*    to your version of the file, but you are not obligated to do so.  If
*    you do not wish to do so, delete this exception statement from your
*    version.
*
*/

#include "unicode_strtools.h"

static int Q_UChar32ToUTF8Len( uchar32 uVal )
{
	if( uVal <= 0x7F )
		return 1;

	if( uVal <= 0x7FF )
		return 2;

	if( uVal <= 0xFFFF )
		return 3;

	return 4;
}

static int Q_UChar32ToUTF8( uchar32 uVal, char * pUTF8Out )
{
	if( uVal <= 0x7F )
	{
		pUTF8Out[0] = uVal;
		return 1;
	}
	else if( uVal <= 0x7FF )
	{
		pUTF8Out[0] = (uVal >> 6) | 0xC0;
		pUTF8Out[1] = (uVal & 0x3F) | 0x80;
		return 2;
	}
	else if( uVal <= 0xFFFF )
	{
		pUTF8Out[0] = ((uVal >> 12)) | 0xE0;
		pUTF8Out[1] = (((uVal >> 6)) & 0x3F) | 0x80;
		pUTF8Out[2] = (uVal & 0x3F) | 0x80;
		return 3;
	}
	else
	{
		pUTF8Out[0] = ((uVal >> 18) & 7) | 0xF0;
		pUTF8Out[1] = ((uVal >> 12) & 0x3F) | 0x80;
		pUTF8Out[2] = ((uVal >> 6) & 0x3F) | 0x80;
		pUTF8Out[3] = (uVal & 0x3F) | 0x80;
		return 4;
	}
}

static bool Q_IsValidUChar32(uchar32 uVal)
{
	return ( uVal < 0x110000u ) && (( uVal - 0x00D800u ) > 0x7FFu ) && (( uVal & 0xFFFFu ) < 0xFFFEu ) && (( uVal - 0x00FDD0u ) > 0x1Fu );
}

static int Q_UTF16ToUChar32( const uchar16 *pUTF16, uchar32 &uValueOut, bool &bErrorOut )
{
	if( Q_IsValidUChar32( pUTF16[0] ))
	{
		uValueOut = pUTF16[0];
		bErrorOut = false;
		return 1;
	}
	else if( pUTF16[0] - 55296 >= 0x400 || ( pUTF16[1] - 56320 ) >= 0x400 )
	{
		uValueOut = 63;
		bErrorOut = true;
		return 1;
	}
	else
	{
		uValueOut = pUTF16[1] + ((uchar32)( pUTF16[0] - 55287 ) << 10 );
		if( Q_IsValidUChar32( uValueOut ))
		{
			bErrorOut = false;
		}
		else
		{
			uValueOut = 63;
			bErrorOut = true;
		}
		return 2;
	}
}

template<
	typename T_IN,
	typename T_OUT,
	int ( *IN_TO_UCHAR32 )( const T_IN *pUTF8, uchar32 &uValueOut, bool &bErrorOut ),
	int ( UCHAR32_TO_OUT_LEN )( uchar32 uVal ),
	int ( UCHAR32_TO_OUT )( uchar32 uVal, T_OUT *pUTF8Out )
>
static int Q_UnicodeConvertT( const T_IN* pIn, T_OUT *pOut, int nOutBytes, enum EStringConvertErrorPolicy ePolicy )
{
	if( nOutBytes == 0 )
		return 0;

	int nOut = 0;
	if( pOut )
	{
		int nMaxOut = nOutBytes / sizeof( T_OUT ) - 1; // print symbols count

		while( *pIn )
		{
			bool bErr;
			uchar32 uVal;
			pIn += IN_TO_UCHAR32( pIn, uVal, bErr );
			int nOutElems = UCHAR32_TO_OUT_LEN( uVal );
			if( nOutElems + nOut > nMaxOut )
				break;
			nOut += UCHAR32_TO_OUT( uVal, &pOut[nOut] );
			if( bErr )
			{
				if( ePolicy & STRINGCONVERT_SKIP )
				{
					nOut -= nOutElems;
				}
				else if( ePolicy & STRINGCONVERT_FAIL )
				{
					pOut[0] = 0;
					return 0;
				}
			}
		}

		pOut[nOut] = 0;
	}
	else
	{
		while( *pIn )
		{
			bool bErr;
			uchar32 uVal;
			pIn += IN_TO_UCHAR32( pIn, uVal, bErr );
			int nOutElems = UCHAR32_TO_OUT_LEN( uVal );
			if( bErr )
			{
				if( ePolicy & STRINGCONVERT_SKIP )
				{
					nOut -= nOutElems;
				}
				else if( ePolicy & STRINGCONVERT_FAIL )
				{
					return 0;
				}
			}
		}
	}

	return ( nOut + 1 ) * sizeof( T_OUT );
}

int Q_UTF16ToUTF8( const uchar16 *pUTF16, char *pUTF8, int cubDestSizeInBytes, enum EStringConvertErrorPolicy ePolicy )
{
	return Q_UnicodeConvertT<uchar16, char, Q_UTF16ToUChar32, Q_UChar32ToUTF8Len, Q_UChar32ToUTF8>( pUTF16, pUTF8, cubDestSizeInBytes, ePolicy );
}
