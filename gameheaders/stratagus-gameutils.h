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

void error(const char *title, const char *text);
void mkdir_p(const char *path);
void copy_dir(const char *source_folder, const char *target_folder);

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
#include <vector>
#include <iostream>
#include <sstream>
#include <istream>
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
error "Missing the <filesystem> header."
#endif

#include "stratagus-tinyfiledialogs.h"

#ifdef WIN32
#define BUFF_SIZE MAX_PATH
#else
#define BUFF_SIZE 4096
#endif

void error(const char *title, const char *text)
{
	tinyfd_messageBox(title, text, "ok", "error", 1);
	exit(-1);
}

void mkdir_p(const char *path)
{
	fs::create_directories(path);
}
void mkdir_p(const wchar_t *path)
{
    fs::create_directories(path);
}

void copy_dir(fs::path source_folder, fs::path target_folder)
{
	if (fs::exists(target_folder)) {
		if (fs::equivalent(source_folder, target_folder)) {
			return;
		}
		// first delete the target_folder, if it exists, to ensure clean slate
		fs::remove_all(target_folder);
	} else {
		// make the parentdir of the target folder
		fs::create_directories(target_folder.parent_path());
	}
	// now copy the new folder in its place
	fs::copy(source_folder, target_folder, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
}

#ifdef WIN32
char *GetExtractionLogPath(const char *game_name, char *data_path)
{
    static char *marker = (char *)calloc(MAX_PATH, sizeof(char));
    if (marker[0] != '\0') {
        return marker;
    }
    char logname[MAX_PATH];
    strcpy(logname, game_name);
    strcat(logname, "-extraction.log");
    if (PathCombineA(marker, data_path, "portable-install")) {
        if (PathFileExistsA(marker)) {
            PathCombineA(marker, data_path, logname);
            return marker;
        }
    }
    SHGetFolderPathA(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, marker);
    PathAppendA(marker, "Stratagus");
    mkdir_p(marker);
    PathAppendA(marker, logname);
    return marker;
}

wchar_t *GetExtractionLogPath(const wchar_t *game_name, const wchar_t *data_path)
{
    static wchar_t *marker = (wchar_t *)calloc(MAX_PATH, sizeof(wchar_t));
    if (marker[0] != '\0') {
        return marker;
    }
    wchar_t logname[MAX_PATH];
    wcscpy(logname, game_name);
    wcscat(logname, L"-extraction.log");
    if (PathCombineW(marker, data_path, L"portable-install")) {
        if (PathFileExistsW(marker)) {
            PathCombineW(marker, data_path, logname);
            return marker;
        }
    }
    SHGetFolderPathW(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, marker);
    PathAppendW(marker, L"Stratagus");
    mkdir_p(marker);
    PathAppendW(marker, logname);
    return marker;
}
#endif

#ifdef WIN32
// quoting logic taken from "Everyone quotes command line arguments the wrong way" by Daniel Colascione
/*++

Routine Description:

    This routine appends the given argument to a command line such
    that CommandLineToArgvW will return the argument string unchanged.
    Arguments in a command line should be separated by spaces; this
    function does not add these spaces.

Arguments:

    Argument - Supplies the argument to encode.

    CommandLine - Supplies the command line to which we append the encoded argument string.

    Force - Supplies an indication of whether we should quote
            the argument even if it does not contain any characters that would
            ordinarily require quoting.

Return Value:

    None.

Environment:

    Arbitrary.

--*/
void ArgvQuote(const std::wstring &Argument, std::wstring &CommandLine, bool Force)
{
    //
    // Unless we're told otherwise, don't quote unless we actually
    // need to do so --- hopefully avoid problems if programs won't
    // parse quotes properly
    //
    if (Force == false && Argument.empty() == false && Argument.find_first_of(L" \t\n\v\"") == Argument.npos) {
        CommandLine.append(Argument);
    } else {
        CommandLine.push_back(L'"');

        for (auto It = Argument.begin(); ; ++It) {
            unsigned NumberBackslashes = 0;

            while (It != Argument.end() && *It == L'\\') {
                ++It;
                ++NumberBackslashes;
            }

            if (It == Argument.end()) {
                //
                // Escape all backslashes, but let the terminating
                // double quotation mark we add below be interpreted
                // as a metacharacter.
                //
                CommandLine.append(NumberBackslashes * 2, L'\\');
                break;
            } else if (*It == L'"') {
                //
                // Escape all backslashes and the following
                // double quotation mark.
                //
                CommandLine.append(NumberBackslashes * 2 + 1, L'\\');
                CommandLine.push_back(*It);
            } else {
                //
                // Backslashes aren't special here.
                //
                CommandLine.append(NumberBackslashes, L'\\');
                CommandLine.push_back(*It);
            }
        }
        CommandLine.push_back(L'"');
    }
}

int runCommand(std::wstring &file, std::vector<std::wstring> argv, bool echo = false, std::wstring *outputCommandline = NULL)
{
	std::wstring cmdline;
	std::wstring executable;

	ArgvQuote(file, executable, false);

	for (size_t i = 0; i < argv.size(); i++) {
		std::wstring arg = argv[i];
		ArgvQuote(arg, cmdline, false);
		if (i + 1 < argv.size()) {
			cmdline.push_back(L' ');
		}
	}
	std::wstring cmdcmdline;
	for (auto c : cmdline) {
		if (c == L'(' || c == L')' || c == L'%' || c == L'!' || c == L'^' || c == L'"' || c == L'<' || c == L'>' || c == L'&' || c == L'|') {
			cmdcmdline.push_back(L'^');
		}
		cmdcmdline.push_back(c);
	}

	if (argv.size() > 0) {
		cmdcmdline = std::wstring(L"@") + executable + std::wstring(L" ") + cmdcmdline;
	} else {
		cmdcmdline = std::wstring(L"@") + executable;
	}

	if (outputCommandline != NULL) {
		outputCommandline->append(cmdcmdline);
	}
	if (echo) {
		std::wcout << executable << L' ' << cmdline << L'\n';
		std::wcout << cmdcmdline << L'\n';
	}
	_flushall();
	int code = _wsystem(cmdcmdline.c_str());
	if (code == -1) {
		std::wcout << _wcserror(errno) << L'\n';
	}
	return code;
}
#else
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int runCommand(const char *file, char *const argv[], bool echo = false, std::string *outputCommandline = NULL)
{
	pid_t pid = fork();

	if (echo || outputCommandline) {
		std::string commandline = file;
		for (int i = 0; ; i++) {
			if (argv[i] == NULL) {
				break;
			}
			commandline += " ";
			commandline += argv[i];
		}
		if (echo) {
			std::cout << commandline << std::endl;
		}
		if (outputCommandline) {
			outputCommandline->append(commandline);
		}
	}

	if (pid == 0) {
		// child
		exit(execvp(file, argv));
	} else {
		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			return WEXITSTATUS(status);
		} else {
			return -1;
		}
	}
}
#endif

#endif
