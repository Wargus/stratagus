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

void error(const char* title, const char* text);
void mkdir_p(const char* path);
void copy_dir(const char* source_folder, const char* target_folder);

#if __APPLE__
#define USE_MAC
#endif

#if ( defined (_MSC_VER) || defined (_WIN32) || defined (_WIN64) ) && ! defined (WIN32)
#define WIN32 1
#endif

#ifdef WIN32
#include <Shlwapi.h>
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "Shlwapi.lib")
#include <direct.h>
//#define inline __inline
#define chdir _chdir
#define getcwd _getcwd
#define spawnvp _spawnvp
#define stat _stat
#define strdup _strdup
#define mkdir(f, m) _mkdir(f)
// PathRemoveFileSpec on a drive (e.g. when extracting from CD) will leave the trailing \... remove that
#define parentdir(x) PathRemoveFileSpec(x); if (x[strlen(x) - 1] == '\\') x[strlen(x) - 1] = '\0'
#define execvp _execvp
#define unlink(x) _unlink(x)
#else
#if defined(USE_MAC)
#define parentdir(x) strcpy(x, dirname(x))
#else
#define parentdir(x) dirname(x)
#endif
#endif

#ifdef WIN32
#ifndef WINVER
#define WINVER 0x0501
#endif
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

#include "stratagus-tinyfiledialogs.h"

#define BUFF_SIZE 4096
char dst_root[BUFF_SIZE];
char src_root[BUFF_SIZE];

void error(const char* title, const char* text) {
	tinyfd_messageBox(title, text, "ok", "error", 1);
	exit(-1);
}

void mkdir_p(const char* path) {
	int error = 0;	
	printf("mkdir %s\n", path);
	if (mkdir(path, 0777)) {
		error = errno;
		if (error == ENOENT) {	
			char *sep = strrchr((char*)path, '/');
			if (sep == NULL) {
				sep = strrchr((char*)path, SLASH[0]);
			}
			if (sep != NULL) {
				*sep = '\0';
				if (strlen(path) > 0) {
					// will be null if the we reach the first /
					mkdir_p(path);
				}
				*sep = '/';
				mkdir(path, 0777);
			}
		} else if (error != EEXIST) {
			if (mkdir(path, 0777)) {
				printf("Error while trying to create '%s'\n", path);
			}
		}
	}
}

#ifdef WIN32
#include <wchar.h>
#include <string>
void copy_dir(const char* source_folder, const char* target_folder)
{
	wchar_t *wsource_folder = new wchar_t[strlen(source_folder) + 1];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wsource_folder, strlen(source_folder) + 1, source_folder, _TRUNCATE);
	wchar_t *wtarget_folder = new wchar_t[strlen(target_folder) + 1];
	mbstowcs_s(&convertedChars, wtarget_folder, strlen(target_folder) + 1, target_folder, _TRUNCATE);
	WCHAR sf[MAX_PATH + 1];
	WCHAR tf[MAX_PATH + 1];
	wcscpy_s(sf, MAX_PATH, wsource_folder);
	char* ptarget = strdup(target_folder);
	parentdir(ptarget);
	mkdir_p(ptarget);
	wcscpy_s(tf, MAX_PATH, wtarget_folder);
	sf[lstrlenW(sf) + 1] = 0;
	tf[lstrlenW(tf) + 1] = 0;
	SHFILEOPSTRUCTW s = { 0 };
	s.wFunc = FO_COPY;
	s.pTo = tf;
	s.pFrom = sf;
	s.fFlags = FOF_SILENT | FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NO_UI;
	SHFileOperationW(&s);
}
#else
int copy_file(const char* src_path, const struct stat* sb, int typeflag) {
	char dst_path[BUFF_SIZE];
	printf("%s to %s\n", src_path, dst_root);
	strcpy(dst_path, dst_root);
	strcat(dst_path, src_path + strlen(src_root));
	switch(typeflag) {
	case FTW_D:
		mkdir_p(dst_path);
		break;
	case FTW_F:
		mkdir_p(parentdir(strdup(dst_path)));
		FILE* in = fopen(src_path, "rb");
		FILE* out = fopen(dst_path, "wb");
		char buf[4096];
		int c = 0;
		if (!in) {
			error("Extraction error", "Could not open source folder for reading.");
		}
		if (!out) {
			error("Extraction error", "Could not open data folder for writing.");
		}
		while (c = fread(buf, sizeof(char), 4096, in)) {
			fwrite(buf, sizeof(char), c, out);
		}
		fclose(in);
		fclose(out);
		break;
	}
	return 0;
}

void copy_dir(const char* src_path, const char* dst_path) {
	printf("Copying %s to %s\n", src_path, dst_path);
	mkdir_p(parentdir(strdup(dst_path)));
	strcpy(dst_root, dst_path);
	strcpy(src_root, src_path);
	ftw(src_path, copy_file, 20);
}
#endif
