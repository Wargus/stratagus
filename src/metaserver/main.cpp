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
