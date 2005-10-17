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
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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
//      $Id$

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "stratagus.h"
#include "util.h"

/*----------------------------------------------------------------------------
--  Random
----------------------------------------------------------------------------*/

unsigned SyncRandSeed;               /// sync random seed value.

/**
**  Inititalize sync rand seed.
*/
void InitSyncRand(void)
{
	SyncRandSeed = 0x87654321;
}

/**
**  Synchronized random number.
**
**  @note This random value must be same on all machines in network game.
**  Very simple random generations, enough for us.
*/
int SyncRand(void)
{
	int val;

	val = SyncRandSeed >> 16;

	SyncRandSeed = SyncRandSeed * (0x12345678 * 4 + 1) + 1;

	return val;
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
	//  is even, and the one bit is as far left as is consistant
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

/*----------------------------------------------------------------------------
--  Strings
----------------------------------------------------------------------------*/

/**
**  String duplicate/concatenate (two arguments)
**
**  @param l  Left string
**  @param r  Right string
**
**  @return   Allocated combined string (must be freed).
*/
char *strdcat(const char *l, const char *r)
{
	char *res;

	res = new char[strlen(l) + strlen(r) + 1];
	if (res) {
		strcpy(res, l);
		strcat(res, r);
	}
	return res;
}

/**
**  String duplicate/concatenate (three arguments)
**
**  @param l  Left string
**  @param m  Middle string
**  @param r  Right string
**
**  @return   Allocated combined string (must be freeded).
*/
char *strdcat3(const char *l, const char *m, const char *r)
{
	char *res;

	res = new char[strlen(l) + strlen(m) + strlen(r) + 1];
	if (res) {
		strcpy(res, l);
		strcat(res, m);
		strcat(res, r);
	}
	return res;
}

#if !defined(HAVE_STRCASESTR)
/**
**  Case insensitive version of strstr
**
**  @param a  String to search in
**  @param b  Substring to search for
**
**  @return   Pointer to first occurence of b or NULL if not found.
*/
char *strcasestr(const char *a, const char *b)
{
	int x;

	if (!a || !*a || !b || !*b || strlen(a) < strlen(b)) {
		return NULL;
	}

	x = 0;
	while (*a) {
		if (a[x] && (tolower(a[x]) == tolower(b[x]))) {
			++x;
		} else if (b[x]) {
			++a;
			x = 0;
		} else {
			return (char*)a;
		}
	}

	return NULL;
}
#endif // !HAVE_STRCASESTR


/*----------------------------------------------------------------------------
--  Getopt
----------------------------------------------------------------------------*/

/**
**  Standard implementation of getopt(3).
**
**  One extension: If the first character of the optionsstring is a ':'
**  the error return for 'argument required' is a ':' not a '?'.
**  This makes it easier to differentiate between an 'illegal option' and
**  an 'argument required' error.
*/

#if defined(_MSC_VER)

#include <io.h>
#include <string.h>

int opterr = 1;
int optind = 1;
int optopt;
char *optarg;

static void getopt_err(char *argv0, char *str, char opt)
{
	if (opterr) {
		char errbuf[2];
		char *x;

		errbuf[0] = opt;
		errbuf[1] = '\n';

		while ((x = strchr(argv0, '/'))) {
			argv0 = x + 1;
		}

		write(2, argv0, strlen(argv0));
		write(2, str, strlen(str));
		write(2, errbuf, 2);
	}
}

int getopt(int argc, char *const *argv, const char *opts)
{
	static int sp = 1;
	register int c;
	register const char *cp;

	optarg = NULL;

	if (sp == 1) {
		if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0') {
			return EOF;
		} else if (!strcmp(argv[optind], "--")) {
			optind++;
			return EOF;
		}
	}
	optopt = c = argv[optind][sp];
	if (c == ':' || (cp = strchr(opts, c)) == NULL) {
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

#endif /* _MSC_VER */
