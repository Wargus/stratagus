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
#include "stratagus.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <string>
#include <string_view>

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

	*value = std::clamp(*value, minValue, maxValue);
}

extern uint32_t fletcher32(std::string_view content);

/*----------------------------------------------------------------------------
--  Strings
----------------------------------------------------------------------------*/

#include <cstring>

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

#if defined(WIN32) && defined(UNICODE)
#define LL(LITERAL) L"" LITERAL
#else
#define LL(LITERAL) "" LITERAL
#endif

int to_number(std::string_view s, int base = 10);

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

#include <optional>
#include <string>

std::optional<std::string> GetClipboard();
void SetClipboard(const std::string &str);

/*----------------------------------------------------------------------------
--  UTF8
----------------------------------------------------------------------------*/

int UTF8GetNext(std::string_view text, int curpos);
int UTF8GetPrev(std::string_view text, int curpos);

void append_unicode(std::string &s, std::uint32_t unicode);
std::string to_utf8(std::uint32_t unicode);

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
#include <functional>

struct identity
{
	template <typename T>
	T &&operator()(T &&arg) const
	{
		return std::forward<T>(arg);
	}
};

namespace ranges
{

	template <typename Range, typename Predicate>
	auto all_of(const Range &range, Predicate &&predicate)
	{
		return std::all_of(std::begin(range), std::end(range), std::forward<Predicate>(predicate));
	}

	template <typename Range, typename Predicate>
	auto any_of(const Range &range, Predicate &&predicate)
	{
		return std::any_of(std::begin(range), std::end(range), std::forward<Predicate>(predicate));
	}

	template <typename Range, typename Predicate>
	auto none_of(const Range &range, Predicate &&predicate)
	{
		return std::none_of(std::begin(range), std::end(range), std::forward<Predicate>(predicate));
	}

	template <typename Range>
	bool equal(const Range &range1, const Range &range2)
	{
		return std::equal(std::begin(range1), std::end(range1), std::begin(range2), std::end(range2));
	}


	template <typename Range, typename Predicate>
	std::size_t count_if(const Range &range, Predicate &&predicate)
	{
		return std::count_if(std::begin(range), std::end(range), predicate);
	}

	template <typename Range, typename Value>
	void fill(Range &range, const Value &value)
	{
		std::fill(std::begin(range), std::end(range), value);
	}

	template <typename Range, typename Value>
	void iota(Range &range, const Value startValue)
	{
		std::iota(std::begin(range), std::end(range), startValue);
	}

	template <typename Range, typename Value, typename Proj = identity>
	auto find(Range &range, const Value &value, Proj proj = {})
	{
		return std::find_if(std::begin(range), std::end(range), [&](const auto &elem) {
			return std::invoke(proj, elem) == value;
		});
	}

	template <typename Range, typename Predicate>
	auto find_if(Range &range, Predicate &&predicate)
	{
		return std::find_if(std::begin(range), std::end(range), [&](const auto &elem) {
			return std::invoke(std::forward<Predicate>(predicate), elem);
		});
	}

	template <typename Range, typename Pred>
	auto partition(Range &range, Pred pred)
	{
		return std::partition(std::begin(range), std::end(range), pred);
	}

	template <typename Range, typename Value>
	bool contains(const Range &range, const Value &value)
	{
		return std::find(std::begin(range), std::end(range), value) != std::end(range);
	}

	template <typename Range>
	void reverse(Range &range)
	{
		std::reverse(std::begin(range), std::end(range));
	}

	template <typename T, typename A, typename U>
	void erase(std::vector<T, A> &v, const U &value)
	{
		v.erase(std::remove(std::begin(v), std::end(v), value), std::end(v));
	}

	template <typename T, typename A, typename Pred>
	void erase_if(std::vector<T, A> &v, Pred pred)
	{
		v.erase(std::remove_if(std::begin(v), std::end(v), pred), std::end(v));
	}

	template <typename Range, typename Comparer = std::less<>, typename Proj = identity>
	auto max_element(Range &range, Comparer &&comparer = {}, Proj &&proj = {})
	{
		return std::max_element(
			std::begin(range), std::end(range), [&](const auto &lhs, const auto &rhs) {
				return std::invoke(comparer, std::invoke(proj, lhs), std::invoke(proj, rhs));
			});
	}

	template <typename Range, typename Comparer = std::less<>, typename Proj = identity>
	auto min_element(Range &range, Comparer &&comparer = {}, Proj &&proj = {})
	{
		return std::min_element(
			std::begin(range), std::end(range), [&](const auto &lhs, const auto &rhs) {
				return std::invoke(comparer, std::invoke(proj, lhs), std::invoke(proj, rhs));
			});
	}

	template <typename I, typename S, typename Pred, typename Proj = identity>
	auto partition_point(I first, S last, Pred pred, Proj &&proj = {})
	{
		return std::partition_point(first, last,
			[&](const auto &elem) {
				return std::invoke(pred, std::invoke(proj, elem));
			});
	}

	template <typename I, typename S, typename Value, typename Comparer = std::less<>, typename Proj = identity>
	auto lower_bound(I first, S last, const Value &value, Comparer &&comparer = {}, Proj &&proj = {})
	{
		return std::lower_bound(first, last, value,
			[&](const auto &lhs, const auto &rhs) {
				return std::invoke(comparer, std::invoke(proj, lhs), rhs);
			});
	}

	template <typename Range, typename Value>
	auto lower_bound(Range &range, const Value &value)
	{
		return std::lower_bound(std::begin(range), std::end(range), value);
	}

	template <typename I, typename S, typename Value, typename Comparer = std::less<>, typename Proj = identity>
	auto upper_bound(I first, S last, const Value &value, Comparer &&comparer = {}, Proj &&proj = {})
	{
		return std::upper_bound(first, last, value,
			[&](const auto &lhs, const auto &rhs) {
				return std::invoke(comparer, lhs, std::invoke(proj, rhs));
			});
	}

	template <typename Range, typename Value, typename Comparer = std::less<>>
	auto upper_bound(Range &range, const Value &value, Comparer &&comparer = {})
	{
		return std::upper_bound(
			std::begin(range), std::end(range), value, std::forward<Comparer>(comparer));
	}

	template <typename Range, typename OutputIt>
	auto copy(const Range &range, OutputIt copy_to)
	{
		return std::copy(std::begin(range), std::end(range), copy_to);
	}

	template <typename Range, typename OutputIt, typename UnaryPredicate>
	auto copy_if(const Range &range, OutputIt copy_to, UnaryPredicate pred)
	{
		return std::copy_if(std::begin(range), std::end(range), copy_to, [&](const auto &elem) {
			return std::invoke(std::forward<UnaryPredicate>(pred), elem);
		});
	}

	template <typename Range>
	void rotate_n(Range &range, const int shift)
	{
		if (shift >= 0) {
			for (int i = 0; i < shift; i++) {
				std::rotate(std::rbegin(range), std::rbegin(range) + 1, std::rend(range));
			}
		} else {
			for (int i = 0; i > shift; i--) {
				std::rotate(std::begin(range), std::begin(range) + 1, std::end(range));
			}
		}
	}

	template <typename Range, typename Comparer = std::less<>, typename Proj = identity>
	void sort(Range &range, Comparer &&comparer = {}, Proj &&proj = {})
	{
		std::sort(std::begin(range), std::end(range), [&](const auto &lhs, const auto &rhs) {
			return std::invoke(comparer, std::invoke(proj, lhs), std::invoke(proj, rhs));
		});
	}

	template <typename Range, typename Comparer = std::less<>, typename Proj = identity>
	bool is_sorted(const Range &range, Comparer &&comparer = {}, Proj &&proj = {})
	{
		return std::is_sorted(std::begin(range), std::end(range),
			[&](const auto &lhs, const auto &rhs) {
				return std::invoke(comparer, std::invoke(proj, lhs), std::invoke(proj, rhs));
			});
	}

}

//@}

#endif /* __UTIL_H__ */
