//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name myendian.h	-	The endian-specific headerfile. */
//
//	(c) Copyright 2000,2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __ENDIAN_H__
#define __ENDIAN_H__

//@{

// ============================================================================
#ifdef USE_SDL	// {

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <SDL/SDL_endian.h>

/*----------------------------------------------------------------------------
--	Macros
----------------------------------------------------------------------------*/

/**
**	Convert a 16 bit value in little endian and return it in native format.
*/
#define	ConvertLE16(v)	SDL_SwapLE16((v))

/**
**	Convert a 32 bit value in little endian and return it in native format.
*/
#define	ConvertLE32(v)	SDL_SwapLE32((v))

/**
**	Access a 16 bit value in little endian and return it in native format.
*/
#ifdef __ULTRA_SPARC__

extern unsigned short inline AccessLE16(unsigned char *p) {
        return p[0] + (p[1] << 8);
}

#else

#define	AccessLE16(p)	SDL_SwapLE16(*((unsigned short*)(p)))

#endif

/**
**	Access a 32 bit value in little endian and return it in native format.
*/
#ifdef __ULTRA_SPARC__

extern unsigned inline AccessLE32(unsigned char *p) {
    return  p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] <<24);
}

#else

#define	AccessLE32(p)	SDL_SwapLE32(*((unsigned int*)(p)))

#endif

/**
**	Fetch a 16 bit value in little endian with incrementing pointer
**	and return it in native format.
*/
#ifdef __ULTRA_SPARC__

extern unsigned short inline _FetchLE16(unsigned char **pp) {
    unsigned char *p = *pp;
    unsigned short i = p[0] + (p[1] << 8);
    (*pp) += 2;
    return i;
}
#define FetchLE16(p)    _FetchLE16(&p)

#else

#define	FetchLE16(p)	SDL_SwapLE16(*((unsigned short*)(p))++)

#endif

/**
**	Fetch a 32 bit value in little endian with incrementing pointer
**	and return it in native format.
*/
#ifdef __ULTRA_SPARC__

extern unsigned inline _FetchLE32(unsigned char **pp) {
    unsigned char *p = *pp;
    unsigned int i = p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] <<24);
    (*pp) += 4;
    return i;
}
#define FetchLE32(p)    _FetchLE32(&p)

#else

#define	FetchLE32(p)	SDL_SwapLE32(*((unsigned int*)(p))++)

#endif

// ============================================================================
#else		// }{ SDL
// ============================================================================

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#if !defined(__CYGWIN__) && !defined(__MINGW32__) && !defined(_MSC_VER)
#if defined(__FreeBSD__)
#include <sys/types.h>
#else
#include <endian.h>
#endif /* __FreeBSD__ */

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
#include <byteswap.h>
#endif
#if defined(__APPLE__)
#include <architecture/byte_order.h>
#endif
#endif

/*----------------------------------------------------------------------------
--	Macros
----------------------------------------------------------------------------*/

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN

/**
**	Convert a 16 bit value in little endian and return it in native format.
*/
#define	ConvertLE16(v)	bswap_16((v))

/**
**	Convert a 32 bit value in little endian and return it in native format.
*/
#define	ConvertLE32(v)	bswap_32((v))

#else

#if defined(__APPLE__)

/**
**	Convert a 16 bit value in little endian and return it in native format.
*/
#define	ConvertLE16(v)	NXSwapLittleShortToHost(v)

/**
**	Convert a 32 bit value in little endian and return it in native format.
*/
#define	ConvertLE32(v)	NXSwapLittleIntToHost(v)

#else

/**
**	Convert a 16 bit value in little endian and return it in native format.
*/
#define	ConvertLE16(v)	(v)

/**
**	Convert a 32 bit value in little endian and return it in native format.
*/
#define	ConvertLE32(v)	(v)

#endif  // defined(__APPLE__)

#endif	// ! defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN

/**
**	Access a 16 bit value in little endian and return it in native format.
*/
#define	AccessLE16(p)	ConvertLE16(*((unsigned short*)(p)))

/**
**	Access a 32 bit value in little endian and return it in native format.
*/
#define	AccessLE32(p)	ConvertLE32(*((unsigned int*)(p)))

/**
**	Fetch a 16 bit value in little endian with incrementing pointer
**	and return it in native format.
*/
#define	FetchLE16(p)	ConvertLE16(*((unsigned short*)(p))++)

/**
**	Fetch a 32 bit value in little endian with incrementing pointer
**	and return it in native format.
*/
#define	FetchLE32(p)	ConvertLE32(*((unsigned int*)(p))++)

#endif		// } !SDL
// ============================================================================

/**
**	Fetch a 8 bit value with incrementing pointer
**	and return it in native format.
*/
#define	FetchByte(p)	(*((unsigned char*)(p))++)

//@}

#endif // !__ENDIAN_H__
