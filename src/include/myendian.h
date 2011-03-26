//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name myendian.h - The endian-specific headerfile. */
//
//      (c) Copyright 2000-2011 by Lutz Sammer and Pali Rohár
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

#ifndef __ENDIAN_H__
#define __ENDIAN_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "SDL_byteorder.h"
#include "SDL_endian.h"

/*----------------------------------------------------------------------------
--  Macros
----------------------------------------------------------------------------*/

/**
**  Convert a 16 bit value in little endian and return it in native format.
*/
#define ConvertLE16(v) SDL_SwapLE16((v))

/**
**  Convert a 32 bit value in little endian and return it in native format.
*/
#define ConvertLE32(v) SDL_SwapLE32((v))

/**
**  Access a 16 bit value in little endian and return it in native format.
*/
#ifdef __ULTRA_SPARC__

extern unsigned short inline AccessLE16(unsigned char *p) {
	return p[0] + (p[1] << 8);
}

#else

#define AccessLE16(p) SDL_SwapLE16(*((unsigned short *)(p)))

#endif

/**
**  Access a 32 bit value in little endian and return it in native format.
*/
#ifdef __ULTRA_SPARC__

extern unsigned inline AccessLE32(unsigned char *p) {
	return  p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] <<24);
}

#else

#define AccessLE32(p) SDL_SwapLE32(*((unsigned int *)(p)))

#endif

/**
**  Fetch a 16 bit value in little endian with incrementing pointer
**  and return it in native format.
*/
#ifdef __ULTRA_SPARC__

extern unsigned short inline _FetchLE16(unsigned char **pp) {
	unsigned char *p = *pp;
	unsigned short i = p[0] + (p[1] << 8);
	(*pp) += 2;
	return i;
}
#define FetchLE16(p) _FetchLE16(&p)

#else

#define FetchLE16(p) SDL_SwapLE16(*((unsigned short *)(p))); p += 2

#endif

/**
**  Fetch a 32 bit value in little endian with incrementing pointer
**  and return it in native format.
*/
#ifdef __ULTRA_SPARC__

extern unsigned inline _FetchLE32(unsigned char **pp) {
	unsigned char *p = *pp;
	unsigned int i = p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] <<24);
	(*pp) += 4;
	return i;
}
#define FetchLE32(p) _FetchLE32(&p)

#else

#define FetchLE32(p) SDL_SwapLE32(*((unsigned int *)(p))); p += 4

#endif

/* byte order defines */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define STRATAGUS_LITTLE_ENDIAN
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
#define STRATAGUS_BIG_ENDIAN
#else
#error Neither SDL_BIG_ENDIAN nor SDL_LIL_ENDIAN is set
#endif

// ============================================================================
#else // }{ SDL
// ============================================================================

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#if !defined(__CYGWIN__) && !defined(__MINGW32__) && !defined(_MSC_VER)
#if defined(USE_BSD)
#include <sys/types.h>
#else
#include <endian.h>
#endif // USE_BSD

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
#include <byteswap.h>
#endif
#if defined(__APPLE__)
#include <architecture/byte_order.h>
#endif
#endif

/*----------------------------------------------------------------------------
--  Macros
----------------------------------------------------------------------------*/

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN

/**
**  Convert a 16 bit value in little endian and return it in native format.
*/
#define ConvertLE16(v) bswap_16((v))

/**
**  Convert a 32 bit value in little endian and return it in native format.
*/
#define ConvertLE32(v) bswap_32((v))

#else

#if defined(__APPLE__)

/**
**  Convert a 16 bit value in little endian and return it in native format.
*/
#define ConvertLE16(v) NXSwapLittleShortToHost(v)

/**
**  Convert a 32 bit value in little endian and return it in native format.
*/
#define ConvertLE32(v) NXSwapLittleIntToHost(v)

#else

/**
**  Convert a 16 bit value in little endian and return it in native format.
*/
#define ConvertLE16(v) (v)

/**
**  Convert a 32 bit value in little endian and return it in native format.
*/
#define ConvertLE32(v) (v)

#endif  // defined(__APPLE__)

#endif // ! defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN

/**
**  Access a 16 bit value in little endian and return it in native format.
*/
#define AccessLE16(p) ConvertLE16(*((unsigned short *)(p)))

/**
**  Access a 32 bit value in little endian and return it in native format.
*/
#define AccessLE32(p) ConvertLE32(*((unsigned int *)(p)))

/**
**  Fetch a 16 bit value in little endian with incrementing pointer
**  and return it in native format.
*/
#define FetchLE16(p) ConvertLE16(*((unsigned short *)(p))++)

/**
**  Fetch a 32 bit value in little endian with incrementing pointer
**  and return it in native format.
*/
#define FetchLE32(p) ConvertLE32(*((unsigned int *)(p))++)

/**
**  Fetch a 8 bit value with incrementing pointer
**  and return it in native format.
*/
#define FetchByte(p) (*((unsigned char *)(p))++)

//@}

#endif // !__ENDIAN_H__
