/*
       _________ __                 __
      /   _____//  |_____________ _/  |______     ____  __ __  ______
      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
             \/                  \/          \//_____/            \/
  ______________________                           ______________________
                        T H E   W A R   B E G I N S
         Stratagus - A free fantasy real time strategy game engine

    stratagus-maemo-extract.h - Stratagus Game data extractor for Maemo
    Copyright (C) 2010-2011  Pali Rohár <pali.rohar@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
 * @page MaemoExtract Stratagus Game data extractor for Maemo
 *
 * Stratagus Game Launcher is C code for generating launcher for any Stratagus game.
 * Game launcher for concrete game check if game data exists in default Stratagus
 * location and spawn Stratagus process with correct game data location. If does not
 * exist it show GUI or console error message.
 *
 * Before including this header, you need to define:
 *
 * ::GAME_NAME
 *
 * ::GAME_CD
 *
 * ::GAME_CD_DIR
 *
 * ::GAME_CD_FILE
 *
 * ::GAME
 *
 *
 * ::EXTRACT_BIN
 *
 *
 * And optional:
 *
 * ::EXTRACT_COMMAND
 *
 * ::EXTRACT_INFO
 *
 * Example use of code:
 *
 * @code
 *
 * #define GAME_NAME "My Game Name"
 * #define GAME_CD "Original Game CD Name"
 * #define GAME_CD_DIR "MyDocs/game"
 * #define GAME_CD_FILE "datafile.bin"
 * #define GAME "my_game"
 * #define EXTRACT_BIN "/opt/stratagus/bin/mygametool"
 * #define EXTRACT_COMMAND "/opt/stratagus/bin/mygametool /home/user/MyDocs/game /home/user/stratagus/my_game"
 * #define EXTRACT_INFO ""
 * #include <stratagus-maemo-extract.h>
 *
 * @endcode
 **/

/**
 * \def GAME_NAME
 * Full name of your Game
 **/

/**
 * \def GAME_CD
 * Full name of data CD
 **/

/**
 * \def GAME_CD_DIR
 * Relative path of /home/user where is data CD
 **/

/**
 * \def GAME_CD_FILE
 * Filename of one data file
 **/

/**
 * \def GAME
 * Short name of game (lower ascii chars without space)
 **/

/**
 * \def EXTRACT_BIN
 * Full path to program which extract data
 **/

/**
 * \def EXTRACT_COMMAND
 * Optional: Full patch to extract program with command line arguments for extracting data
 * If not specified 'EXTRACT_BIN DATA_DIR EXTRACT_DIR' is used
 **/

/**
 * \def EXTRACT_INFO
 * Optional: Additional extract data game info
 * Default empty
 **/

/* Fake definitions for Doxygen */
#ifdef DOXYGEN
#define GAME_NAME
#define GAME_CD
#define GAME_CD_DIR
#define GAME_CD_FILE
#define GAME
#define EXTRACT_BIN
#endif

#if ! defined (GAME_NAME) || ! defined (GAME_CD) || ! defined (GAME_CD_DIR) || ! defined (GAME_CD_FILE) || ! defined (GAME) || ! defined (EXTRACT_BIN)
#error You need to define all Game macros, see stratagus-maemo-extract.h
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <gtk/gtk.h>
#include <hildon/hildon.h>

#define DATA_DIR "/home/user/" GAME_CD_DIR
#define EXTRACT_DIR "/home/user/MyDocs/stratagus/" GAME

#ifndef EXTRACT_COMMAND
#define EXTRACT_COMMAND EXTRACT_BIN " " DATA_DIR " " EXTRACT_DIR
#endif

#ifndef EXTRACT_INFO
#define EXTRACT_INFO ""
#endif

#define DATA_NEED_COPY "Note: You need the original " GAME_CD "\nto extract the game data files.\nData files are needed to run " GAME_NAME ".\n\nFirst copy " GAME_CD " to folder\n" GAME_CD_DIR "\nthen press OK. " EXTRACT_INFO
#define DATA_FOUND GAME_CD " data files was found in folder\n" GAME_CD_DIR "\n\nPlease be patient, the data may take\na couple of minutes to extract...\n\nPress OK to start extracting data now."
#define DATA_NOT_FOUND "Error: " GAME_CD " data files was not found.\n\nCheck if you have file\n" GAME_CD_DIR "/" GAME_CD_FILE
#define EXTRACT_OK GAME_CD " data files was successfull extracted."
#define EXTRACT_FAILED "Error: Cannot extract " GAME_CD " data files\nextract program crashed."

static void message(char * title, char * text, int error) {

	GtkWidget * window = NULL;
	GtkWidget * dialog = NULL;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), title);
	gtk_widget_show(window);

	dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, error ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO, GTK_BUTTONS_OK, text, NULL);
	gtk_window_set_title(GTK_WINDOW(dialog), title);
	gtk_window_set_skip_pager_hint(GTK_WINDOW(dialog), 0);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), 0);
	gtk_label_set_selectable(GTK_LABEL(GTK_MESSAGE_DIALOG(dialog)->label), 0);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	gtk_widget_destroy(window);

	if ( error )
		exit(error);

}

int main(int argc, char * argv[]) {

	FILE * file;
	struct stat st;

	hildon_gtk_init(&argc, &argv);

	file = fopen(EXTRACT_DIR "/extracted", "r");

	if ( file ) {

		char act_version[20];
		fgets(act_version, 20, file);
		fclose(file);

		file = popen(EXTRACT_BIN " -V", "r");

		if ( file ) {

			char new_version[20];
			fgets(new_version, 20, file);
			pclose(file);

			if ( strncmp(act_version, new_version, 19) == 0 )
				return 0;

		}

	}

	message(GAME_NAME, DATA_NEED_COPY, 0);

	if ( stat(DATA_DIR "/" GAME_CD_FILE, &st) != 0 ) {

		char * buf = strdup(GAME_CD_FILE);
		char * path = calloc(strlen(DATA_DIR) + strlen(GAME_CD_FILE) + 2, 1);
		char * ptr = buf;

		while ( *ptr ) {

			*ptr = toupper(*ptr);
			++ptr;

		}

		sprintf(path, DATA_DIR "/%s", buf);

		if ( stat(path, &st) != 0 )
			message(GAME_NAME, DATA_NOT_FOUND, 1);

		free(buf);
		free(path);

	}

	if ( stat("/home/user/MyDocs/stratagus", &st) != 0 )
		mkdir("/home/user/MyDocs/stratagus", 0777);

	if ( stat("/home/user/MyDocs/stratagus/" GAME, &st) != 0 )
		mkdir("/home/user/MyDocs/stratagus/" GAME, 0777);

	message(GAME_NAME, DATA_FOUND, 0);

	int ret = system(EXTRACT_COMMAND);

	if ( ret != 0 )
		message(GAME_NAME, EXTRACT_FAILED, ret);

	message(GAME_NAME, EXTRACT_OK, 0);
	return 0;

}
