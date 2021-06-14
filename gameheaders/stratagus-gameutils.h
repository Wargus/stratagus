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
    Copyright (C) 2015-2016  The Stratagus Developers

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

#ifndef STRATAGUS_GAMEUTILS_H
#define STRATAGUS_GAMEUTILS_H

void error(const char* title, const char* text);
void mkdir_p(const char* path);
void copy_dir(const char* source_folder, const char* target_folder);

#if __APPLE__
#define USE_MAC
#endif

#if ( defined (_MSC_VER) || defined (_WIN32) || defined (_WIN64) ) && ! defined (WIN32)
#define WIN32 1
#endif

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef WIN32

// set everything to winxp sp2 compatiblity
#define NTDDI_VERSION 0x05010200
#define _WIN32_WINNT 0x0501
#define WINVER 0x0501

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#include <Shlwapi.h>
#include <Shlobj.h>
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "Shlwapi.lib")
#include <direct.h>
#define mkdir(f, m) _mkdir(f)
// PathRemoveFileSpec on a drive (e.g. when extracting from CD) will leave the trailing \... remove that
#define parentdir(x) PathRemoveFileSpec(x); if (x[strlen(x) - 1] == '\\') x[strlen(x) - 1] = '\0'
#else
#if defined(USE_MAC)
#define parentdir(x) strcpy(x, dirname(x))
#else
#define parentdir(x) dirname(x)
#endif
#endif

#ifdef WIN32
#include <windows.h>
#include <wincon.h>
#include <process.h>
#define QUOTE "\""
#else
#include <ftw.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/wait.h>
#define QUOTE "'"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string>
#include <filesystem>

#include "stratagus-tinyfiledialogs.h"

#ifdef WIN32
#define BUFF_SIZE MAX_PATH
#else
#define BUFF_SIZE 4096
#endif

void error(const char* title, const char* text) {
	tinyfd_messageBox(title, text, "ok", "error", 1);
	exit(-1);
}

void mkdir_p(const char* path) {
	std::filesystem::create_directories(path);
}

void copy_dir(std::filesystem::path source_folder, std::filesystem::path target_folder) {
	if (std::filesystem::exists(target_folder)) {
		if (std::filesystem::equivalent(source_folder, target_folder)) {
			return;
		}
		// first delete the target_folder, if it exists, to ensure clean slate
		std::filesystem::remove_all(target_folder);
	} else {
		// make the parentdir of the target folder
		std::filesystem::create_directories(target_folder.parent_path());
	}
	// now copy the new folder in its place
	std::filesystem::copy(source_folder, target_folder, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
}

#ifdef WIN32
char* GetExtractionLogPath(const char* game_name, char* data_path) {
	static char *marker = (char*)calloc(MAX_PATH, sizeof(char));
	if (marker[0] != '\0') {
		return marker;
	}
	char logname[MAX_PATH];
	strcpy(logname, game_name);
	strcat(logname, "-extraction.log");
	if (PathCombine(marker, data_path, "portable-install")) {
		if (PathFileExists(marker)) {
			PathCombine(marker, data_path, logname);
			return marker;
		}
	}
	SHGetFolderPathA(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, marker);
	PathAppend(marker, "Stratagus");
	mkdir_p(marker);
	PathAppend(marker, logname);
	return marker;
}
#endif

#endif
