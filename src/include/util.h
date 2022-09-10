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
/**@name util.h - General utilities. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer and Jimmy Salmon
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

#ifndef __UTIL_H__
#define __UTIL_H__

//@{

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include "ghc_filesystem.hpp"
namespace fs = ghc::filesystem;
#endif

#include <cstdlib>
#include <cstdint>
#include <string>

/*----------------------------------------------------------------------------
--  Random
----------------------------------------------------------------------------*/

#include <cmath>

extern unsigned SyncRandSeed;           /// Sync random seed value
extern uint32_t FileChecksums;          /// checksums of all loaded lua files

extern void InitSyncRand();             /// Initialize the syncron rand
extern int SyncRand();                  /// Syncron rand
extern int SyncRand(int max);           /// Syncron rand

///  rand only used on this computer.
extern int MyRand();

/*----------------------------------------------------------------------------
--  Math
----------------------------------------------------------------------------*/

/// Compute a square root using ints
extern long isqrt(long num);

inline int square(int v)
{
	return v * v;
}

template <typename T>
void clamp(T *value, T minValue, T maxValue)
{
	Assert(minValue <= maxValue);

	if (*value < minValue) {
		*value = minValue;
	} else if (maxValue < *value) {
		*value = maxValue;
	}
}

extern uint32_t fletcher32(const std::string &content);

/*----------------------------------------------------------------------------
--  Strings
----------------------------------------------------------------------------*/

#include <string.h>

#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
#endif

#ifndef HAVE_ERRNOT
typedef int errno_t;
#endif

#ifndef HAVE_STRCPYS
extern errno_t strcpy_s(char *dst, size_t dstsize, const char *src);
#endif

#ifndef HAVE_STRNCPYS
extern errno_t strncpy_s(char *dst, size_t dstsize, const char *src, size_t count);
#endif

#ifndef HAVE_STRCATS
extern errno_t strcat_s(char *dst, size_t dstsize, const char *src);
#endif

#ifndef HAVE_STRCASESTR
/// case insensitive strstr
extern char *strcasestr(const char *str, const char *substr);
#endif // !HAVE_STRCASESTR

#ifndef HAVE_STRNLEN
/// determine length of a fixed-length string
extern size_t strnlen(const char *str, size_t strsize);
#endif // !HAVE_STRNLEN

/*----------------------------------------------------------------------------
--  Getopt
----------------------------------------------------------------------------*/

#ifdef HAVE_GETOPT
#include <unistd.h>
#else
extern char *optarg;
extern int optind, opterr, optopt;
int getopt(int argc, char *const argv[], const char *optstring);
#endif

/*----------------------------------------------------------------------------
--  Clipboard
----------------------------------------------------------------------------*/

#include <string>

int GetClipboard(std::string &str);
void SetClipboard(std::string &str);

/*----------------------------------------------------------------------------
--  UTF8
----------------------------------------------------------------------------*/

int UTF8GetNext(const std::string &text, int curpos);
int UTF8GetPrev(const std::string &text, int curpos);

/*----------------------------------------------------------------------------
--  SIMD support
----------------------------------------------------------------------------*/
bool supportsSSE2();
bool supportsAVX();
void *aligned_malloc(size_t alignment, size_t size);
void aligned_free(void *block);

/*----------------------------------------------------------------------------
--  SIMD support
----------------------------------------------------------------------------*/

std::string GetExecutablePath();

//@}

#endif /* __UTIL_H__ */
