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

#include "filesystem.h"

#include <cstdlib>
#include <cstdint>
#include <string>
#include <algorithm>
#include <numeric>

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
using errno_t = int;
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
extern const char *strcasestr(const char *str, const char *substr) noexcept;
#endif // !HAVE_STRCASESTR

#ifndef HAVE_STRNLEN
/// determine length of a fixed-length string
extern size_t strnlen(const char *str, size_t strsize);
#endif // !HAVE_STRNLEN

#if defined(WIN32) && defined(UNICODE)
#define L(LITERAL) L"" LITERAL
#else
#define L(LITERAL) "" LITERAL
#endif


/*----------------------------------------------------------------------------
--  Getopt
----------------------------------------------------------------------------*/

#ifdef HAVE_GETOPT
#include <unistd.h>
#else
extern "C" {
extern char *optarg;
extern int optind, opterr, optopt;
int getopt(int argc, char *const *argv, const char *optstring) noexcept;
}
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
--  Executable path
----------------------------------------------------------------------------*/

fs::path GetExecutablePath();


/*----------------------------------------------------------------------------
--  Ranges
----------------------------------------------------------------------------*/
namespace ranges
{
    template<typename Range, typename UnaryFunction>
    UnaryFunction for_each(Range& range, UnaryFunction fn)
    {
        return std::for_each(begin(range), end(range), fn);
    }
	
	template<typename Range, typename Value>
	void fill(Range& range, const Value& value)
	{
		std::fill(begin(range), end(range), value);
	}

	template<typename Range, typename Value>
	void iota(Range& range, const Value startValue)
	{
		std::iota(begin(range), end(range), startValue);
	}

	template<typename Range, typename Value>
	auto find(Range& range, const Value& value)
	{
		return std::find(begin(range), end(range), value);
	}

	template<typename Range, typename Value>
	bool consist(Range& range, const Value& value)
	{
		return std::find(begin(range), end(range), value) == end(range) ? false : true;
	}

    template<typename Range>
	void reverse(Range& range)
	{
		std::reverse(begin(range), end(range));
	}

    template<typename Range>
    auto min_element(Range& range)
    {
        return std::min_element(begin(range), end(range));
    }

    template<typename Range, typename CmpFunction>
    auto min_element(Range& range, CmpFunction cmp)
    {
        return std::min_element(begin(range), end(range), cmp);
    }
   
    template<typename Range, typename Value>
    auto lower_bound(Range& range, Value value)
    {
        return std::lower_bound(begin(range), end(range), value);
    }

    template<typename Range, typename OutputIt>
    auto copy(Range& range, OutputIt copy_to)
    {
        return std::copy(begin(range), end(range), copy_to);
    }

    template<typename Range, typename OutputIt, typename UnaryPredicate>
    auto copy_if(Range& range, OutputIt copy_to, UnaryPredicate pred )
    {
        return std::copy_if(begin(range), end(range), copy_to, pred);
    }
	
    template<typename Range>
	void rotate_n(Range& range, const int shift)
	{
        if (shift >= 0) {
            for (int i = 0; i < shift; i++) {
                std::rotate(rbegin(range), rbegin(range) + 1, rend(range));
            }
        } else { 
            for (int i = 0; i > shift; i--) {
                std::rotate(begin(range), begin(range) + 1, end(range));
            }
        }
    }	
}

//@}

#endif /* __UTIL_H__ */
