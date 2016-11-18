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
 * ::GAME_CD_FILE_PATTERNS
 *
 * ::GAME
 *
 * ::EXTRACTOR_TOOL
 *
 * ::EXTRACTOR_ARGS
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
 * #define GAME_CD_FILE_PATTERNS "*.WAR", "*.war"
 * #define EXTRACTOR_TOOL "gametool"
 * #define EXTRACTOR_ARGS "-v"
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
 * \def GAME_CD_FILE_PATTERNS
 * Comma-separated file patterns for the extraction wizard to help users select
 * the right folder.
 **/

/**
 * \def GAME
 * Short name of game (lower ascii chars without space)
 **/

/**
 * \def EXTRACTOR_TOOL
 * The name of the game data extractor tool. This code will append the
 * arguments, src, and destionation directories.
 **/

/**
 * \def EXTRACTOR_ARGS
 * The default arguments of the game data extractor tool.
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

#if ! defined (GAME_NAME) || ! defined (GAME_CD) || ! defined (GAME) || ! defined(EXTRACTOR_TOOL)
#error You need to define all Game macros, see stratagus-game-launcher.h
#endif

#if ( defined (_MSC_VER) || defined (_WIN32) || defined (_WIN64) ) && ! defined (WIN32)
#define WIN32
#endif

#ifdef WIN32
#include <Shlwapi.h>
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "Shlwapi.lib")
#endif

/**
 * \def TITLE_PNG
 * OPTIONAL: Path to title screen (for testing if data was extracted)
 **/
#ifndef TITLE_PNG
#ifdef WIN32
#define TITLE_PNG "%s\\graphics\\ui\\title.png"
#else
#define TITLE_PNG "%s/graphics/ui/title.png"
#endif
#endif

#ifndef WIN32
#if ! defined (DATA_PATH) || ! defined (SCRIPTS_PATH) || ! defined (STRATAGUS_BIN)
#error You need to define paths, see stratagus-game-launcher.h
#endif
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#ifdef WIN32
#ifndef WINVER
#define WINVER 0x0501
#endif
#include <windows.h>
#include <wincon.h>
#include <process.h>
#else
#include <ftw.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <direct.h>
//#define inline __inline
#define chdir _chdir
#define getcwd _getcwd
#define spawnvp _spawnvp
#define stat _stat
#define strdup _strdup
#define mkdir(f, m) _mkdir(f)
#define dirname(x) PathRemoveFileSpec(x)
#define execvp _execvp
#define unlink _unlink
#endif

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

#ifndef WIN32
#include <unistd.h>
#include <X11/Xlib.h>
#include <libgen.h>
#endif

#ifdef _WIN64
#define REGKEY "Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Stratagus (64 bit)"
#elif defined (WIN32)
#define REGKEY "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Stratagus"
#endif

#define TITLE GAME_NAME
#define STRATAGUS_NOT_FOUND "Stratagus is not installed.\nYou need Stratagus to run " GAME_NAME "!\nFirst install Stratagus from https://launchpad.net/stratagus"
#define DATA_NOT_EXTRACTED GAME_NAME " data was not extracted, is corrupted, or outdated.\nYou need to extract it from original " GAME_CD "."

#define BUFF_SIZE 4096

#ifndef WIN32
int ConsoleMode = 0;
#endif

#include "tinyfiledialogs.h"
#include "tinyfiledialogs.c"

static void SetUserDataPath(char* data_path) {
#if WIN32
	strcpy(data_path, getenv("APPDATA"));
#else
	strcpy(data_path, getenv("HOME"));
#endif
	int datalen = strlen(data_path);
#if WIN32
	strcat(data_path, "\\Stratagus\\");
#elif defined(USE_MAC)
	strcat(data_path, "/Library/Stratagus/");
#else
	strcat(data_path, "/.stratagus/");
#endif
	strcat(data_path, "data." GAME_NAME);
}

#ifdef WIN32
#define QUOTE "\""
#define SLASH "\\"
#else
#define QUOTE "'"
#define SLASH "/"
#endif

static void error(char* title, char* text) {
	tinyfd_messageBox(title, text, "ok", "error", 1);
	exit(-1);
}

char dst_root[BUFF_SIZE];
char src_root[BUFF_SIZE];

void mkdir_p(const char* path)
{
	char *cp, *s, *s2;

	if (*path && path[0] == '.') {  // relative don't work
		return;
	}
	cp = strdup(path);
	s = strrchr(cp, '/');
	if (!s) s = strrchr(cp, SLASH[0]);
	if (s) {
		*s = '\0';  // remove file
		s = cp;
		for (;;) {  // make each path element
			s2 = strchr(s, '/');
			if (!s2) s = strchr(s, SLASH[0]);
			s = s2;
			if (s) {
				*s = '\0';
			}
			mkdir(cp, 0777);
			if (s) {
				*s++ = SLASH[0];
			} else {
				break;
			}
		}
	} else {
		mkdir(cp, 0777);
	}
	free(cp);
}

#ifdef WIN32
#include <wchar.h>
#include <string>
static void copy_dir(const char* source_folder, const char* target_folder)
{
	wchar_t *wsource_folder = new wchar_t[strlen(source_folder) + 1];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wsource_folder, strlen(source_folder) + 1, source_folder, _TRUNCATE);
	wchar_t *wtarget_folder = new wchar_t[strlen(target_folder) + 1];
	mbstowcs_s(&convertedChars, wtarget_folder, strlen(target_folder) + 1, target_folder, _TRUNCATE);
	WCHAR sf[MAX_PATH + 1];
	WCHAR tf[MAX_PATH + 1];
	wcscpy_s(sf, MAX_PATH, wsource_folder);
	mkdir_p(target_folder);
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
static int copy_file(const char* src_path, const struct stat* sb, int typeflag) {
	char dst_path[BUFF_SIZE];
	strcpy(dst_path, dst_root);
	strcat(dst_path, src_path + strlen(src_root));
	switch(typeflag) {
	case FTW_D:
		mkdir(dst_path, sb->st_mode);
		break;
	case FTW_F:
		FILE* in = fopen(src_path, "rb");
		FILE* out = fopen(dst_path, "wb");
		char buf[4096];
		int c = 0;
		while (c = fread(buf, sizeof(char), 4096, in)) {
			fwrite(buf, sizeof(char), c, out);
		}
		fclose(in);
		fclose(out);
		break;
	}
	return 0;
}

static void copy_dir(const char* src_path, const char* dst_path) {
	printf("Copying %s to %s\n", src_path, dst_path);
	mkdir_p(dst_path);
	strcpy(dst_root, dst_path);
	strcpy(src_root, src_path);
	ftw(src_path, copy_file, 20);
}
#endif

int check_version(char* tool_path, char* data_path) {
    char buf[4096] = {'\0'};
    sprintf(buf, "%s/extracted" , data_path);
    FILE *f = fopen(buf, "r");
    char dataversion[20] = {'\0'};
    char toolversion[20] = {'\0'};
    if (f) {
		fgets(dataversion, 20, f);
		fclose(f);
    } else {
		return 1; // No file means we don't care
	}
    sprintf(buf, "%s -V", tool_path);
#ifndef WIN32
    FILE *pipe = popen(buf, "r");
    if (f) {
		fgets(toolversion, 20, pipe);
		pclose(pipe);
    }
#else
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		return 1;
	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		return 1;
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	if (!CreateProcess(NULL, buf, NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo))
		return 1;
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);
	ReadFile(g_hChildStd_OUT_Rd, toolversion, 20, NULL, NULL);
#endif
    // strip whitespace
    for (size_t i=0, j=0; toolversion[j]=toolversion[i]; j+=!isspace(toolversion[i++]));
    for (size_t i=0, j=0; dataversion[j]=dataversion[i]; j+=!isspace(dataversion[i++]));
	if (strcmp(dataversion, toolversion) == 0) {
		return 1;
	}
    return 0;
}

static void ExtractData(char* extractor_tool, char* destination, char* scripts_path, int force=0) {
	if (!force) {
		tinyfd_messageBox("Missing data",
						  DATA_NOT_EXTRACTED " Please select the " GAME_CD, "ok", "error", 1);
	} else {
		tinyfd_messageBox("", "Please select the " GAME_CD, "ok", "error", 1);
	}
	char* filepatterns[] = { GAME_CD_FILE_PATTERNS, NULL };
	int patterncount = 0;
	while (filepatterns[patterncount++] != NULL);
	const char* datafile = tinyfd_openFileDialog(GAME_CD " location", "",
												  patterncount - 1, filepatterns, NULL, 0);
	if (datafile == NULL) {
		exit(-1);
	}
	char srcfolder[1024] = {'\0'};
	strcpy(srcfolder, datafile);

	dirname(srcfolder);

	struct stat st;
	if (stat(scripts_path, &st) != 0) {
		// deployment time path not found, try compile time path
		strcpy(scripts_path, SRC_PATH());
		dirname(scripts_path);
	}

	char contrib_src_path[BUFF_SIZE];
	char contrib_dest_path[BUFF_SIZE];
	int i = 0;
	char* contrib_directories[] = CONTRIB_DIRECTORIES;
	while (contrib_directories[i] != NULL && contrib_directories[i + 1] != NULL) {
		strcpy(contrib_src_path, scripts_path);
		strcat(contrib_src_path, SLASH);
		strcat(contrib_src_path, contrib_directories[i]);
		strcpy(contrib_dest_path, destination);
		strcat(contrib_dest_path, SLASH);
		strcat(contrib_dest_path, contrib_directories[i + 1]);
		copy_dir(contrib_src_path, contrib_dest_path);
		i += 2;
	}

	char cmdbuf[4096] = {'\0'};
#ifdef USE_MAC
	strcat(cmdbuf, "osascript -e \"tell application \\\"Terminal\\\" to do script \\\"'");
#elseif !defined(WIN32)
	if (!ConsoleMode) {
		strcat(cmdbuf, "x-terminal-emulator -e \"");
	}
#else
	strcat(cmdbuf, "/C \"");
#endif
	strcat(cmdbuf, extractor_tool);
	strcat(cmdbuf, " " QUOTE);
	strcat(cmdbuf, srcfolder);
	strcat(cmdbuf, QUOTE " " QUOTE);
	strcat(cmdbuf, destination);
	strcat(cmdbuf, QUOTE);
#ifdef USE_MAC
	strcat(cmdbuf, "\\\"\"");
#elseif !defined(WIN32)
	if (!ConsoleMode) {
	strcat(cmdbuf, "\"");
}
#else
	strcat(cmdbuf, "\"");
#endif
#ifdef WIN32
	DWORD exitcode = 0;
	SHELLEXECUTEINFO ShExecInfo = { 0 };
	char* toolpath = strdup(extractor_tool);
	PathRemoveFileSpec(toolpath);
	// remove the leading quote
	if (toolpath[0] == '"') memmove(toolpath, toolpath + 1, strlen(toolpath) + 1);
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = "cmd";
	ShExecInfo.lpParameters = cmdbuf;
	ShExecInfo.lpDirectory = toolpath;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	GetExitCodeProcess(ShExecInfo.hProcess, &exitcode);
#else
	int exitcode = 0;
    printf("Running %s\n", cmdbuf);
	exitcode = system(cmdbuf);
#endif
	if (exitcode != 0) {
		tinyfd_messageBox("Missing data", "Data extraction failed", "ok", "error", 1);
		unlink(destination);
	};
}

int main(int argc, char * argv[]) {

#if !defined(WIN32) && !defined(USE_MAC)
	if ( ! XOpenDisplay(NULL) ) {
		ConsoleMode = 1;
	}
	if (!ConsoleMode) {
	}
#endif

	struct stat st;
	char data_path[BUFF_SIZE];
	char scripts_path[BUFF_SIZE];
	char stratagus_bin[BUFF_SIZE];
	char title_path[BUFF_SIZE];
	char extractor_path[BUFF_SIZE];

	// The extractor is in the same dir as we are
	strcpy(extractor_path, argv[0]);
	dirname(extractor_path);
	strcat(extractor_path, SLASH EXTRACTOR_TOOL);
	// Once we have the path, we quote it by moving the memory one byte to the
	// right, and surrounding it with the quote character and finishing null
	// bytes. Then we add the arguments.
	extractor_path[strlen(extractor_path) + 1] = '\0';
	memmove(extractor_path + 1, extractor_path, strlen(extractor_path));
	extractor_path[0] = QUOTE[0];
	extractor_path[strlen(extractor_path) + 1] = '\0';
	extractor_path[strlen(extractor_path)] = QUOTE[0];
	strcat(extractor_path, " " EXTRACTOR_ARGS);

#ifdef WIN32
	char executable_path[BUFF_SIZE];
	memset(executable_path, 0, sizeof(executable_path));
	GetModuleFileName(NULL, executable_path, sizeof(executable_path)-1);

	char executable_drive[_MAX_DRIVE];
	char executable_dir[_MAX_DIR];
	memset(executable_drive, 0, sizeof(executable_drive));
	memset(executable_dir, 0, sizeof(executable_dir));
	_splitpath(executable_path, executable_drive, executable_dir, NULL, NULL);

	size_t data_path_size = sizeof(data_path);
	memset(data_path, 0, data_path_size);

	if (executable_path[0] && executable_drive[0] && executable_dir[0]) {
		strcpy(data_path, executable_drive);
		strcpy(data_path+strlen(executable_drive), executable_dir);
	} else {
		getcwd(data_path, data_path_size);
	}
	const size_t data_path_length = strlen(data_path);
	if (data_path_length != 0 && data_path[data_path_length - 1] == '\\') {
		data_path[data_path_length - 1] = '\0';
	}
	sprintf(scripts_path, "\"%s\"", data_path);

	char stratagus_path[BUFF_SIZE];

	// Try to use stratagus.exe from data (install) directory first
	sprintf(stratagus_bin, "%s\\stratagus.exe", data_path);
	if (stat(stratagus_bin, &st) != 0) {
		// If no local stratagus.exe is present, look for a globally installed version
		DWORD stratagus_path_size = sizeof(stratagus_path);
		memset(stratagus_path, 0, stratagus_path_size);
		HKEY key;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY, 0, KEY_QUERY_VALUE, &key) == ERROR_SUCCESS) {
			if (RegQueryValueEx(key, "InstallLocation", NULL, NULL, (LPBYTE)stratagus_path, &stratagus_path_size) == ERROR_SUCCESS) {
				if (stratagus_path_size == 0 || strlen(stratagus_path) == 0) {
					error(TITLE, STRATAGUS_NOT_FOUND);
				}
			}
			RegCloseKey(key);
		}

		if (chdir(stratagus_path) != 0) {
			error(TITLE, STRATAGUS_NOT_FOUND);
		}
		sprintf(stratagus_bin, "%s\\stratagus.exe", stratagus_path);
	}
#else
	strcpy(data_path, DATA_PATH);
	strcpy(scripts_path, SCRIPTS_PATH);
	strcpy(stratagus_bin, STRATAGUS_BIN);
#endif

	if ( stat(stratagus_bin, &st) != 0 ) {
#ifdef WIN32
		_fullpath(stratagus_bin, argv[0], BUFF_SIZE);
		PathRemoveFileSpec(stratagus_bin);
		strcat(extractor_path, "\\stratagus.exe");
#else
		realpath(argv[0], stratagus_bin);
		dirname(stratagus_bin);
		if (strlen(stratagus_bin) > 0) {
			strcat(stratagus_bin, "/stratagus");
		} else {
			strcat(stratagus_bin, "./stratagus");
		}
#endif
		if ( stat(stratagus_bin, &st) != 0 ) {
			error(TITLE, STRATAGUS_NOT_FOUND);
		}
	}

	if (argc > 1) {
		if (!strcmp(argv[1], "--extract")) {
			// Force extraction and exit
			SetUserDataPath(data_path);
			ExtractData(extractor_path, data_path, scripts_path, 1);
			return 0;
		}

		if (!strcmp(argv[1], "--extract-no-gui")) {
			// Force extraction without ui and exit
			tinyfd_forceConsole = 1;
			SetUserDataPath(data_path);
			ExtractData(extractor_path, data_path, scripts_path, 1);
			return 0;
		}
	}

	sprintf(title_path, TITLE_PNG, data_path);
	if ( stat(title_path, &st) != 0 ) {
		SetUserDataPath(data_path);
		sprintf(title_path, TITLE_PNG, data_path);
		if ( stat(title_path, &st) != 0 ) {
			ExtractData(extractor_path, data_path, scripts_path);
		}
		if ( stat(title_path, &st) != 0 ) {
			error(TITLE, DATA_NOT_EXTRACTED);
		}
	}

	if (!check_version(extractor_path, data_path)) {
		ExtractData(extractor_path, data_path, scripts_path);
	}

#ifdef WIN32
	int data_path_len = strlen(data_path);

	for (int i = data_path_len - 1; i >= 0; --i) {
		data_path[i + 1] = data_path[i];
	}
	data_path[0] = '"';
	data_path[data_path_len + 1] = '"';
	data_path[data_path_len + 2] = 0;
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
	stratagus_argv[2] = data_path;

	for (int i = 3; i < argc + 2; ++i ) {
		stratagus_argv[i] = argv[i - 2];
	}
	stratagus_argv[argc + 2] = NULL;

	execvp(stratagus_bin, stratagus_argv);

#ifndef WIN32
	if (strcmp(stratagus_bin, "stratagus") == 0) {
		realpath(argv[0], stratagus_bin);
		dirname(stratagus_bin);
		strcat(stratagus_bin, "/stratagus");
	}
	execvp(stratagus_bin, stratagus_argv);
#endif
	error(TITLE, STRATAGUS_NOT_FOUND);
	return 1;
}
