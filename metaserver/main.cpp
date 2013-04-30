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
/**@name main.cpp - Primary functionality. */
//
//      (c) Copyright 2005-2011 by Edward Haase and Pali Rohár
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "SDL.h"

#include "stratagus.h"
#include "util.h"
#include "netdriver.h"
#include "cmd.h"
#include "db.h"

#ifndef _MSC_VER
#include <unistd.h>
#include <errno.h>
#endif
#ifdef __CYGWIN__
#include <getopt.h>
#endif
#if defined(_MSC_VER) || defined(__MINGW32__)
extern char *optarg;
extern int optopt;
extern int getopt(int argc, char *const *argv, const char *opt);
#endif

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// @todo Needs code cleanup.
/// @todo Needs compatibility checks.
/// @todo Needs error message unification.

#if 1 // from stratagus.cpp, to avoid link issues.

bool EnableDebugPrint;           /// if enabled, print the debug messages
bool EnableAssert;               /// if enabled, halt on assertion failures
bool EnableUnitDebug;            /// if enabled, a unit info dump will be created

void PrintLocation(const char *file, int line, const char *funcName)
{
	fprintf(stdout, "%s:%d: %s: ", file, line, funcName);
}

void AbortAt(const char *file, int line, const char *funcName, const char *conditionStr)
{
	fprintf(stderr, "Assertion failed at %s:%d: %s: %s\n", file, line, funcName, conditionStr);
	abort();
}

void PrintOnStdOut(const char *format, ...)
{
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
}

// from util.cpp

#ifndef HAVE_GETOPT

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

		fprintf(stderr, "%s%s%c\n", argv0, str, opt);
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

#endif
#endif



/**
**  Main loop
*/
static void MainLoop(void)
{
	Uint32 ticks[2];
	int delay;
	int done;

	//
	// Start the transactions.
	//
	done = 0;
	while (!done) {
		ticks[0] = SDL_GetTicks();

		//
		// Update sessions and buffers.
		//
		UpdateSessions();
		UpdateParser();

		ticks[1] = SDL_GetTicks();

		//
		// Idle out the remainder of this loop.
		//
		if ((int)(ticks[1] - ticks[0]) > Server.PollingDelay) {
			delay = 0;
		} else {
			delay = Server.PollingDelay - (ticks[1] - ticks[0]);
		}

		if (delay > 2000) {
			delay = 2000;
		}

		SDL_Delay(delay);
	}

}
/**
**  The main program: initialize, parse options and arguments.
*/
int main(int argc, char **argv)
{
	int status;
	int i;

	Server.Port = DEFAULT_PORT;
	Server.MaxConnections = DEFAULT_MAX_CONN;
	Server.IdleTimeout = DEFAULT_SESSION_TIMEOUT;
	Server.PollingDelay = DEFAULT_POLLING_DELAY;

	//
	// Standard SDL Init.
	//
	if (SDL_Init(0) == -1) {
		printf("SDL_Init: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);

	//
	// Parse the command line.
	//
	while ((i = getopt(argc, argv, ":p:m:i:d:")) != -1) {
		switch (i) {
			case 'p':
				Server.Port = atoi(optarg);
				if (Server.Port <= 0) {
					Server.Port = DEFAULT_PORT;
				}
				break;
			case 'm':
				Server.MaxConnections = atoi(optarg);
				break;
			case 'i':
				Server.IdleTimeout = atoi(optarg);
				break;
			case 'd':
				Server.PollingDelay = atoi(optarg);
				break;
			case ':':
				printf("Missing argument for %c\n", optopt);
				exit(0);
				break;
			case '?':
				printf("Unrecognized option: -%c\n", optopt);
				break;
		}
    }

	// Initialize the database
	if (DBInit()) {
		fprintf(stderr, "DBInit failed\n");
		exit(1);
	}
	atexit(DBQuit);

	//
	// Initialize server.
	//
	// Open the server to connections.
	//
	if ((status = ServerInit(Server.Port)) != 0) {
		if (status > 0) {
			fprintf(stderr, "ERROR %d: %s\n", errno, strerror(errno));		// > 0
		} else {
			fprintf(stderr, "ERROR: %s\n", SDL_GetError());				// < 0
		}
		exit(status);
	}
	atexit(ServerQuit);

	printf("Stratagus Metaserver Initialized on port %d.\n", Server.Port);

	//
	// Uncomment this line for MSVC (or other default)
	// debugging of segmentation violations.
	//
	// signal(SIGSEGV, SIG_DFL);

	MainLoop();

	//
	// Server tasks done.
	//
	// "atexit" will take over from here for cleanup.
	//
	printf("Stratagus Metaserver Done.\n");

	return 0;
}

//@}
