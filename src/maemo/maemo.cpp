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
/**@name maemo.cpp - Maemo platform code. */
//
//      (c) Copyright 2011 by Pali Rohár
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

#ifdef USE_MAEMO

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <libosso.h>
#include <SDL.h>

#include "stratagus.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static osso_context_t *osso = NULL;
static SDL_TimerID timer = NULL;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
 * Callback function for SDL_AddTimer.
 * Tell OSSO to keep display on and prevent suspending device for 60s.
 **/
static Uint32 OssoKeepBacklightAlive(Uint32 interval, void *)
{
	if (!osso) {
		return 0;
	}

	osso_display_state_on(osso);
	osso_display_blanking_pause(osso);

	return interval;
}

/**
 * Initialize OSSO context, needed for calling OSSO functions.
 * Create SDL timer for calling OssoKeepBacklightAlive every 50s.
 **/
static void OssoInitialize(void)
{
	char *application;

	if (FullGameName.empty()) {
		application = strdup("org.stratagus");
	} else {
		application = (char *)calloc(FullGameName.size() + 15, 1);
		strcpy(application, "org.stratagus.");
		for (int i = 0; i < FullGameName.size(); i++) {
			application[i + 14] = tolower(FullGameName[i]);
		}
	}

	osso = osso_initialize(application, VERSION, TRUE, NULL);

	free(application);

	if (!osso) {
		fprintf(stderr, "Couldn't initialize OSSO\n");
		exit(OSSO_ERROR);
	}

	timer = SDL_AddTimer(50000, OssoKeepBacklightAlive, NULL);

	if (!timer) {
		fprintf(stderr, "Couldn't initialize SDL_AddTimer: %s\n", SDL_GetError());
		exit(1);
	}
}

/**
 * Deinitialize OSSO context and remove registred SDL timer
 **/
static void OssoDeinitialize(void)
{
	SDL_RemoveTimer(timer);
	osso_deinitialize(osso);
}

/**
 * Main function which initialize Maemo platform code
 **/
void maemo_init(void)
{
	OssoInitialize();
	atexit(OssoDeinitialize);
}

#endif

//@}
