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

    stratagus-game-launcher.h - Stratagus Game Launcher
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
 * @page GameLauncher Stratagus Game Launcher
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
 * ::GAME
 *
 * On Non Windows system you need to specify also paths:
 *
 * ::DATA_PATH
 *
 * ::SCRIPTS_PATH
 *
 * ::STRATAGUS_BIN
 *
 * On Windows paths are reading from InstallLocation key in Uninstall section:
 * Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Stratagus
 *
 * Example use of code:
 *
 * @code
 *
 * #define GAME_NAME "My Game Name"
 * #define GAME_CD "Original Game CD Name"
 * #define GAME "my_game"
 *
 * #ifndef WIN32
 * #define DATA_PATH "/usr/share/games/stratagus/my_game"
 * #define SCRIPTS_PATH "/usr/share/games/stratagus/my_game"
 * #define STRATAGUS_BIN "/usr/games/stratagus"
 * #endif
 *
 * #include <stratagus-game-launcher.h>
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
 * \def GAME
 * Short name of game (lower ascii chars without space)
 **/

/**
 * \def DATA_PATH
 * Path to game data directory
 **/

/**
 * \def SCRIPTS_PATH
 * Path to game scripts directory
 **/

/**
 * \def STRATAGUS_BIN
 * Path to stratagus executable binary
 **/

/* Fake definitions for Doxygen */
#ifdef DOXYGEN
#define GAME_NAME
#define GAME_CD
#define GAME
#define DATA_PATH
#define SCRIPTS_PATH
#define STRATAGUS_BIN
#endif

#if ! defined (GAME_NAME) || ! defined (GAME_CD) || ! defined (GAME)
#error You need to define all Game macros, see stratagus-game-launcher.h
#endif

#if ( defined (_MSC_VER) || defined (_WIN32) || defined (_WIN64) ) && ! defined (WIN32)
#define WIN32
#endif

#ifndef WIN32
#if ! defined (DATA_PATH) || ! defined (SCRIPTS_PATH) || ! defined (STRATAGUS_BIN)
#error You need to define paths, see stratagus-game-launcher.h
#endif
#endif

#if ( defined (MAEMO_GTK) || defined (MAEMO_CHANGES) ) && ! defined (MAEMO)
#define MAEMO
#endif

#ifdef WIN32
#define WINVER 0x0501
#include <windows.h>
#include <wincon.h>
#include <process.h>
#include <errno.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <direct.h>
#define inline __inline
#define chdir _chdir
#define getcwd _getcwd
#define spawnvp _spawnvp
#define stat _stat
#endif

#ifndef WIN32
#include <unistd.h>
#include <X11/Xlib.h>
#include <gtk/gtk.h>
#endif

#ifdef MAEMO
#include <hildon/hildon.h>
#endif

#ifdef _WIN64
#define REGKEY "Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Stratagus (64 bit)"
#elif defined (WIN32)
#define REGKEY "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Stratagus"
#endif

#define TITLE GAME_NAME
#define STRATAGUS_NOT_FOUND "Stratagus is not installed.\nYou need Stratagus to run " GAME_NAME "!\nFirst install Stratagus from https://launchpad.net/stratagus"
#define DATA_NOT_EXTRACTED GAME_NAME " data was not extracted yet.\nYou need extract data from original " GAME_CD " first!"
#define NO_X_DISPLAY "Cannot open X Display"
#define CONSOLE_MODE_NOT_ROOT "You must be root to run " GAME_NAME " in console framebuffer mode"

#define BUFF_SIZE 1024

#ifndef WIN32
int ConsoleMode = 0;
#endif

static void error(char * title, char * text) {

#ifdef WIN32
	MessageBox(NULL, text, title, MB_OK | MB_ICONERROR);
#else
	if ( ! ConsoleMode ) {
		GtkWidget * window = NULL;
		GtkWidget * dialog = NULL;

#ifdef MAEMO
		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(window), title);
		gtk_widget_show(window);
#endif

		dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, text, NULL);
		gtk_window_set_title(GTK_WINDOW(dialog), title);
		gtk_window_set_skip_pager_hint(GTK_WINDOW(dialog), 0);
		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(dialog), 0);
		gtk_label_set_selectable(GTK_LABEL(GTK_MESSAGE_DIALOG(dialog)->label), 0);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

#ifdef MAEMO
		gtk_widget_destroy(window);
#endif
	} else {
		fprintf(stderr, "%s -- Error: %s\n", title, text);
	}
#endif
	exit(1);
}

int main(int argc, char * argv[]) {

#ifndef WIN32
	if ( ! XOpenDisplay(NULL) ) {
		ConsoleMode = 1;
	}
	if ( ConsoleMode ) {
#ifdef MAEMO
		error(TITLE, NO_X_DISPLAY);
#else
		if ( getuid() != 0 ) {
			error(TITLE, CONSOLE_MODE_NOT_ROOT);
		}
#endif
	} else {
		gtk_init(&argc, &argv);
#ifdef MAEMO
		hildon_init();
#endif
	}
#endif

	struct stat st;
	char data_path[BUFF_SIZE];
	char scripts_path[BUFF_SIZE];
	char stratagus_bin[BUFF_SIZE];
	char title_path[BUFF_SIZE];

#ifdef WIN32
	size_t data_path_size = sizeof(data_path);
	memset(data_path, 0, data_path_size);
	getcwd(data_path, data_path_size);

	char stratagus_path[BUFF_SIZE];
	DWORD stratagus_path_size = sizeof(stratagus_path);
	memset(stratagus_path, 0, stratagus_path_size);
	HKEY key;

	if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY, 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS ) {
		if ( RegQueryValueEx(key, "InstallLocation", NULL, NULL, (LPBYTE) stratagus_path, &stratagus_path_size) == ERROR_SUCCESS ) {
			if ( stratagus_path_size == 0 || strlen(stratagus_path) == 0 ) {
				error(TITLE, STRATAGUS_NOT_FOUND);
			}
		}
		RegCloseKey(key);
	}

	if ( chdir(stratagus_path) != 0 ) {
		error(TITLE, STRATAGUS_NOT_FOUND);
	}
	strcpy(scripts_path, data_path);
	sprintf(stratagus_bin, "%s\\stratagus.exe", stratagus_path);
#else
	strcpy(data_path, DATA_PATH);
	strcpy(scripts_path, SCRIPTS_PATH);
	strcpy(stratagus_bin, STRATAGUS_BIN);
#endif

	if ( stat(stratagus_bin, &st) != 0 ) {
		error(TITLE, STRATAGUS_NOT_FOUND);
	}
	if ( stat(data_path, &st) != 0 ) {
		error(TITLE, DATA_NOT_EXTRACTED);
	}
#ifdef WIN32
	sprintf(title_path, "%s\\graphics\\ui\\title.png", data_path);

	int data_path_len = strlen(data_path);

	for (int i = data_path_len - 1; i >= 0; --i) {
		data_path[i + 1] = data_path[i];
	}
	data_path[0] = '"';
	data_path[data_path_len + 1] = '"';
	data_path[data_path_len + 2] = 0;
#else
	sprintf(title_path, "%s/graphics/ui/title.png", data_path);
#endif

	if ( stat(title_path, &st) != 0 ) {
		error(TITLE, DATA_NOT_EXTRACTED);
	}
#ifndef WIN32
	if ( strcmp(data_path, scripts_path) != 0 ) {
		if ( chdir(data_path) != 0 ) {
			error(TITLE, DATA_NOT_EXTRACTED);
		}
	}
#endif

#ifdef _MSC_VER
	char** stratagus_argv;
	stratagus_argv = (char**) malloc((argc + 3) * sizeof (*stratagus_argv));
#else
	char * stratagus_argv[argc + 3];
#endif

#ifdef WIN32
	char stratagus_argv0_esc[BUFF_SIZE];
	memset(stratagus_argv0_esc, 0, sizeof(stratagus_argv0_esc));
	strcpy(stratagus_argv0_esc + 1, argv[0]);
	stratagus_argv0_esc[0] = '"';
	stratagus_argv0_esc[strlen(argv[0]) + 1] = '"';
	stratagus_argv0_esc[strlen(argv[0]) + 2] = 0;
	stratagus_argv[0] = stratagus_argv0_esc;
#else
	stratagus_argv[0] = argv[0];
#endif

	stratagus_argv[1] = "-d";
	stratagus_argv[2] = scripts_path;

	for (int i = 3; i < argc + 2; ++i ) {
		stratagus_argv[i] = argv[i - 2];
	}
	stratagus_argv[argc + 2] = NULL;

#ifdef WIN32
	AttachConsole(ATTACH_PARENT_PROCESS);

	errno = 0;
	int ret = spawnvp(_P_WAIT, stratagus_bin, stratagus_argv);
#ifdef _MSC_VER
	free (stratagus_argv);
#endif
	if ( errno == 0 ) {
		return ret;
	}
#else
	execvp(stratagus_bin, stratagus_argv);
#endif

	error(TITLE, STRATAGUS_NOT_FOUND);
	return 1;
}
