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
/**@name util.cpp - General utilites. */
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

#include "util.h"

#include "stratagus.h"

#include <SDL.h>
#include <cctype>
#include <cerrno>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#ifndef __APPLE__
#include <malloc.h>
#endif
#ifdef WIN32
#include <windows.h>
#include <intrin.h>
#endif

#ifdef USE_STACKTRACE
#include <stdexcept>
#include <stacktrace/call_stack.hpp>
#include <stacktrace/stack_exception.hpp>
#else
#include "st_backtrace.h"
#endif

#if __has_include(<charconv>)
#include <charconv> // from_chars
#endif

/*----------------------------------------------------------------------------
--  Random
----------------------------------------------------------------------------*/

uint32_t SyncRandSeed;               /// sync random seed value.
uint32_t FileChecksums = 0;              /// checksums of all loaded lua files

/**
**  Inititalize sync rand seed.
*/
void InitSyncRand()
{
	SyncRandSeed = 0x87654321;
	if (EnableDebugPrint) {
		DebugPrint("GameCycle: %lud, init seed: %x\n", GameCycle, SyncRandSeed);
		//print_backtrace();
		//fflush(stderr);
	}
}

/**
**  Synchronized random number.
**
**  @note This random value must be same on all machines in network game.
**  Very simple random generations, enough for us.
*/
int SyncRand()
{
	int val;

	val = SyncRandSeed >> 16;

	SyncRandSeed = SyncRandSeed * (0x12345678 * 4 + 1) + 1;

	if (EnableDebugPrint) {
		DebugPrint("GameCycle: %lud, seed: %x, Sync rand: %d\n", GameCycle, SyncRandSeed, val);
		//print_backtrace(8);
		//fflush(stderr);
	}
	return val;
}

/**
**  Synchronized random number.
**
**  @param max  Max value of random number to return
*/
int SyncRand(int max)
{
	return SyncRand() % max;
}

static std::random_device dev;
static std::mt19937 rng_engine(dev());
static std::uniform_int_distribution<std::mt19937::result_type> rng_dist(0,RAND_MAX);

int MyRand()
{
	return rng_dist(rng_engine);
}

/*----------------------------------------------------------------------------
--  Math
----------------------------------------------------------------------------*/

/**
**  Compute a square root using ints
**
**  Uses John Halleck's method, see
**  http://www.cc.utah.edu/~nahaj/factoring/isqrt.legalize.c.html
**
**  @param num  Calculate the square root of this number
**
**  @return     The integer square root.
*/
long isqrt(long num)
{
	long squaredbit;
	long remainder;
	long root;

	if (num < 1) {
		return 0;
	}

	//
	//  Load the binary constant 01 00 00 ... 00, where the number
	//  of zero bits to the right of the single one bit
	//  is even, and the one bit is as far left as is consistent
	//  with that condition.)
	//
	//  This portable load replaces the loop that used to be
	//  here, and was donated by  legalize@xmission.com
	//
	squaredbit  = (long)((((unsigned long)~0L) >> 1) & ~(((unsigned long)~0L) >> 2));

	// Form bits of the answer.
	remainder = num;
	root = 0;
	while (squaredbit > 0) {
		if (remainder >= (squaredbit | root)) {
			remainder -= (squaredbit | root);
			root >>= 1;
			root |= squaredbit;
		} else {
			root >>= 1;
		}
		squaredbit >>= 2;
	}

	return root;
}

// from wikipedia, simple checksumming of our lua files. only considers a subset of 7-bit
// ascii chars to hopefully avoid issues with filesystem encodings
uint32_t fletcher32(std::string_view content)
{
	std::vector<uint16_t> alphas;
	size_t consideredChars = 0;
	for (size_t i = 0; i < content.size(); i++) {
		uint16_t c = content[i];
		if (c >= 'A' && c <= 'z') {
			if ((consideredChars++ % 2) == 0) {
				alphas.push_back(c);
			} else {
				alphas.back() |= c << 8;
			}
		}
	}

	const uint16_t *data = alphas.data();
	uint32_t sum1 = 0xffff, sum2 = 0xffff;

	auto shorts = alphas.size();
	while (shorts) {
		size_t tlen = ((shorts >= 359) ? 359 : shorts);
		shorts -= tlen;
		do {
			sum2 += sum1 += *data++;
			tlen--;
		} while (tlen);
		sum1 = (sum1 & 0xffff) + (sum1 >> 16);
		sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	}
	/* Second reduction step to reduce sums to 16 bits */
	sum1 = (sum1 & 0xffff) + (sum1 >> 16);
	sum2 = (sum2 & 0xffff) + (sum2 >> 16);
	return (sum2 << 16) | sum1;
}

/*----------------------------------------------------------------------------
--  Strings
----------------------------------------------------------------------------*/

#ifndef HAVE_STRCPYS
errno_t strcpy_s(char *dst, size_t dstsize, const char *src)
{
	if (dst == nullptr || src == nullptr) {
		return EINVAL;
	}
	if (strlen(src) >= dstsize) {
		return ERANGE;
	}
	strcpy(dst, src);
	return 0;
}
#endif

#ifndef HAVE_STRNLEN
static size_t strnlen(const char *str, size_t strsize) noexcept
{
	size_t len = 0;
	while (len < strsize) {
		if (*str == '\0') {
			break;
		}
		++str;
		++len;
	}
	return len;
}
#endif

#ifndef HAVE_STRNCPYS
errno_t strncpy_s(char *dst, size_t dstsize, const char *src, size_t count)
{
	if (dst == nullptr || src == nullptr || dstsize == 0) {
		return EINVAL;
	}

	size_t mincount;
	if (count == _TRUNCATE) {
		mincount = strnlen(src, dstsize);
	} else {
		mincount = strnlen(src, count);
	}
	if (mincount >= dstsize) {
		if (count != _TRUNCATE) {
			dst[0] = '\0';
			return EINVAL;
		} else {
			mincount = dstsize - 1;
		}
	}
	for (size_t i = 0; i < mincount; ++i) {
		*dst++ = *src++;
	}
	*dst = '\0';
	return 0;
}
#endif

#ifndef HAVE_STRCATS
errno_t strcat_s(char *dst, size_t dstsize, const char *src)
{
	if (dst == nullptr || src == nullptr) {
		return EINVAL;
	}
	char *enddst = dst;
	size_t count = dstsize;
	while (count > 0 && *enddst != '\0') {
		++enddst;
		count--;
	}
	if (count == 0) {
		return EINVAL;
	}
	if (strlen(src) >= count) {
		return ERANGE;
	}
	strcpy(enddst, src);
	return 0;
}
#endif

int to_number(std::string_view s, int base)
{
#if __has_include(<charconv>)
	int res{};
	auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), res, base);
	if (ec != std::errc()) {
		if (ec == std::errc::invalid_argument) {
			ErrorPrint("That isn't a number '%s'.\n", s.data());
		} else if (ec == std::errc::result_out_of_range) {
			ErrorPrint("This number '%s' is larger than an int.\n", s.data());
		}
		Exit(1);
	} else if (ptr != s.data() + s.size()) {
		ErrorPrint("That isn't a number '%s'.\n", s.data());
		Exit(1);
	}
	return res;
#else
	const std::string buf(s); // handle non nul-terminated string
	char *end = nullptr;

	int res = std::strtol(buf.data(), &end, base);
	if (end != buf.data() + buf.size()) {
		ErrorPrint("That isn't a number '%s'.\n", buf.c_str());
		Exit(1);
	}
	return res;
#endif
}


/*----------------------------------------------------------------------------
--  Getopt
----------------------------------------------------------------------------*/

#ifndef HAVE_GETOPT

/**
**  Standard implementation of getopt(3).
**
**  One extension: If the first character of the optionsstring is a ':'
**  the error return for 'argument required' is a ':' not a '?'.
**  This makes it easier to differentiate between an 'illegal option' and
**  an 'argument required' error.
*/

#include <cstring>
#include <string_view>

int opterr = 1;
int optind = 1;
int optopt;
char *optarg;

static void getopt_err(const char *argv0, const char *str, char opt)
{
	if (opterr) {
		const char *x;

		while ((x = strchr(argv0, '/'))) {
			argv0 = x + 1;
		}

		ErrorPrint("%s%s%c\n", argv0, str, opt);
	}
}

int getopt(int argc, char *const *argv, const char *opts) noexcept
{
	static int sp = 1;
	int c;
	const char *cp;

	optarg = nullptr;

	if (sp == 1) {
		if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0') {
			return EOF;
		} else if (argv[optind] == std::string_view("--")) {
			optind++;
			return EOF;
		}
	}
	optopt = c = argv[optind][sp];
	if (c == ':' || (cp = strchr(opts, c)) == nullptr) {
		getopt_err(argv[0], ": illegal option -", (char)c);
		cp = "xx"; /* make the next if false */
		c = '?';
	}
	if (*++cp == ':') {
		if (argv[optind][++sp] != '\0') {
			optarg = &argv[optind++][sp];
		} else if (++optind < argc) {
			optarg = argv[optind++];
		} else {
			getopt_err(argv[0], ": option requires an argument -", (char)c);
			c = (*opts == ':') ? ':' : '?';
		}
		sp = 1;
	} else if (argv[optind][++sp] == '\0') {
		optind++;
		sp = 1;
	}
	return c;
}

#endif


/*----------------------------------------------------------------------------
--  Clipboard
----------------------------------------------------------------------------*/

/**
** Paste text from the clipboard
*/
std::optional<std::string> GetClipboard()
{
	char* txt = SDL_GetClipboardText();
	if (txt) {
		std::string str = txt;
		SDL_free(txt);
		return str;
	} else {
		return std::nullopt;
	}
}

void SetClipboard(const std::string &str) {
	SDL_SetClipboardText(str.c_str());
}

/*----------------------------------------------------------------------------
--  UTF8
----------------------------------------------------------------------------*/

int UTF8GetPrev(std::string_view text, int curpos)
{
	--curpos;
	if (curpos < 0) {
		return curpos;
	}
	while (curpos >= 0) {
		if (static_cast<unsigned int>(curpos) < text.size() && (text[curpos] & 0xC0) != 0x80) {
			return curpos;
		}
		--curpos;
	}
	if (curpos < 0) {
		ErrorPrint("Invalid UTF8.\n");
	}
	return 0;
}

int UTF8GetNext(std::string_view text, int curpos)
{
	if (curpos >= (int)text.size()) {
		return curpos + 1;
	}
	char c = text[curpos];
	if (!(c & 0x80)) {
		return curpos + 1;
	}
	if ((c & 0xE0) == 0xC0) {
		return curpos + 2;
	}
	if ((c & 0xF0) == 0xE0) {
		return curpos + 3;
	}
	ErrorPrint("Invalid UTF8.\n");
	return text.size();
}

void append_unicode(std::string &s, std::uint32_t unicode)
{
	if (unicode < 0x80) {
		s.push_back(unicode & 0x7F);
	} else if (unicode < 0x08'00) {
		s.push_back(((unicode >> 6) & 0x1F) | 0xC0);
		s.push_back((unicode & 0x3F) | 0x80);
	} else if (unicode < 0x00'01'00'00) {
		s.push_back(((unicode >> 12) & 0x0F) | 0xE0);
		s.push_back(((unicode >> 6) & 0x3F) | 0x80);
		s.push_back((unicode & 0x3F) | 0x80);
	} else {
		s.push_back(((unicode >> 18) & 0x07) | 0xF0);
		s.push_back(((unicode >> 12) & 0x3F) | 0x80);
		s.push_back(((unicode >> 6) & 0x3F) | 0x80);
		s.push_back((unicode & 0x3F) | 0x80);
	}
}

//--------------------------------------------------------------------------
std::string to_utf8(std::uint32_t unicode)
{
	std::string res;
	append_unicode(res, unicode);
	return res;
}

/*----------------------------------------------------------------------------
--  others
----------------------------------------------------------------------------*/

void PrintLocation(const char *file, int line, const char *funcName)
{
	fprintf(stdout, "%s:%d: %s: ", file, line, funcName);
}

void AbortAt(const char *file, int line, const char *funcName, const char *conditionStr)
{
	char buf[1024];
	snprintf(buf, 1024, "Assertion failed at %s:%d: %s: %s\n", file, line, funcName, conditionStr);
#ifdef USE_STACKTRACE
	throw stacktrace::stack_runtime_error((const char*)buf);
#else
	fprintf(stderr, "%s\n", buf);
	print_backtrace();
#endif
	fflush(stdout);
	fflush(stderr);
	abort();
}

void PrintOnStdOut(const char *format, ...)
{
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
	fflush(stdout);
}

/*----------------------------------------------------------------------------
	Check SSE/AVX support.
	This can detect the instruction support of
	SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2, SSE4a, SSE5, and AVX.
  ----------------------------------------------------------------------------*/

#ifdef __x86_64__

#ifdef __GNUC__

static void __cpuid(unsigned int* cpuinfo, int info)
{
	__asm__ __volatile__(
		"xchg %%ebx, %%edi;"
		"cpuid;"
		"xchg %%ebx, %%edi;"
		:"=a" (cpuinfo[0]), "=D" (cpuinfo[1]), "=c" (cpuinfo[2]), "=d" (cpuinfo[3])
		:"0" (info)
	);
}

static unsigned long long _my_xgetbv(unsigned int index)
{
	unsigned int eax, edx;
	__asm__ __volatile__(
		"xgetbv;"
		: "=a" (eax), "=d"(edx)
		: "c" (index)
	);
	return ((unsigned long long)edx << 32) | eax;
}

#else // __GNUC__

#define _my_xgetbv(index) _xgetbv(index)

#endif // __GNUC__

struct SIMDSupport {
	bool sseSupportted = false;
	bool sse2Supportted = false;
	bool sse3Supportted = false;
	bool ssse3Supportted = false;
	bool sse4_1Supportted = false;
	bool sse4_2Supportted = false;
	bool sse4aSupportted = false;
	bool sse5Supportted = false;
	bool avxSupportted = false;
};

static struct SIMDSupport checkSIMDSupport() {
	struct SIMDSupport s;

	unsigned int cpuinfo[4];
	__cpuid(cpuinfo, 1);

	// Check SSE, SSE2, SSE3, SSSE3, SSE4.1, and SSE4.2 support
	s.sseSupportted		= cpuinfo[3] & (1 << 25) || false;
	s.sse2Supportted	= cpuinfo[3] & (1 << 26) || false;
	s.sse3Supportted	= cpuinfo[2] & (1 << 0) || false;
	s.ssse3Supportted	= cpuinfo[2] & (1 << 9) || false;
	s.sse4_1Supportted	= cpuinfo[2] & (1 << 19) || false;
	s.sse4_2Supportted	= cpuinfo[2] & (1 << 20) || false;

	// ----------------------------------------------------------------------

	// Check AVX support
	// References
	// http://software.intel.com/en-us/blogs/2011/04/14/is-avx-enabled/
	// http://insufficientlycomplicated.wordpress.com/2011/11/07/detecting-intel-advanced-vector-extensions-avx-in-visual-studio/

	s.avxSupportted = cpuinfo[2] & (1 << 28) || false;
	bool osxsaveSupported = cpuinfo[2] & (1 << 27) || false;
	if (osxsaveSupported && s.avxSupportted)
	{
		// _XCR_XFEATURE_ENABLED_MASK = 0
		unsigned long long xcrFeatureMask = _my_xgetbv(0);
		s.avxSupportted = (xcrFeatureMask & 0x6) == 0x6;
	}

	// ----------------------------------------------------------------------

	// Check SSE4a and SSE5 support

	// Get the number of valid extended IDs
	__cpuid(cpuinfo, 0x80000000);
	unsigned int numExtendedIds = cpuinfo[0];
	if (numExtendedIds >= 0x80000001)
	{
		__cpuid(cpuinfo, 0x80000001);
		s.sse4aSupportted = cpuinfo[2] & (1 << 6) || false;
		s.sse5Supportted = cpuinfo[2] & (1 << 11) || false;
	}

	// ----------------------------------------------------------------------

	return s;
}

bool supportsSSE2()
{
	static struct SIMDSupport s = checkSIMDSupport();
	return s.sse2Supportted;
}

bool supportsAVX()
{
	static struct SIMDSupport s = checkSIMDSupport();
	return s.avxSupportted;
}

#else // __x86_64__

bool supportsSSE2()
{
	return false;
}

bool supportsAVX()
{
	return false;
}

#endif // __x86_64__

void *aligned_malloc(size_t alignment, size_t size)
{
#ifdef WIN32
	return _aligned_malloc(size, alignment);
#elif _ISOC11_SOURCE
	return aligned_alloc(alignment, size);
#elif __APPLE__
	return malloc(size);
#else
	return memalign(alignment, size);
#endif
}

void aligned_free(void *block)
{
#ifdef WIN32
	_aligned_free(block);
#else
	free(block);
#endif
}

fs::path GetExecutablePath()
{
#ifdef WIN32
	TCHAR executable_path[MAX_PATH]{};
	GetModuleFileName(nullptr, executable_path, sizeof(executable_path)-1);
#else
	const auto& executable_path = OriginalArgv[0];
#endif
	return executable_path;
}
