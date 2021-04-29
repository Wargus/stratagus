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

#ifndef STRATAGUS_GAME_LAUNCHER_H
#define STRATAGUS_GAME_LAUNCHER_H

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

#ifndef GAME_SHOULD_EXTRACT_AGAIN
#define GAME_SHOULD_EXTRACT_AGAIN false
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

#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

#ifdef _WIN64
#define REGKEY "Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Stratagus (64 bit)"
#elif defined (WIN32)
#define REGKEY "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Stratagus"
#endif

#define TITLE GAME_NAME
#define EXTRACTOR_NOT_FOUND GAME_NAME " could not find its extraction tool.\n" EXTRACTOR_TOOL "!\n"
#define STRATAGUS_NOT_FOUND "Stratagus is not installed.\nYou need Stratagus to run " GAME_NAME "!\nFirst install Stratagus from https://launchpad.net/stratagus"
#define DATA_NOT_EXTRACTED GAME_NAME " data was not extracted, is corrupted, or outdated.\nYou need to extract it from original " GAME_CD "."

#include "stratagus-gameutils.h"

static void SetUserDataPath(char* data_path) {
#if defined(WIN32)
	SHGetFolderPathA(NULL, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, NULL, 0, data_path);
	// strcpy(data_path, getenv("APPDATA"));
#else
	strcpy(data_path, getenv("HOME"));
#endif
	int datalen = strlen(data_path);
#if defined(WIN32)
	strcat(data_path, "\\Stratagus\\");
#elif defined(USE_MAC)
	strcat(data_path, "/Library/Stratagus/");
#else
	strcat(data_path, "/.stratagus/");
#endif
	strcat(data_path, "data." GAME_NAME);
}

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
#ifdef CHECK_EXTRACTED_VERSION
		return 0; // No file means we have a problem
#else
		return 1; // No file means we don't care
#endif
	}
#ifndef WIN32
	sprintf(buf, "%s -V", tool_path);
    FILE *pipe = popen(buf, "r");
    if (f) {
		fgets(toolversion, 20, pipe);
		pclose(pipe);
    }
#else
	sprintf(buf, "%s -V", tool_path); // tool_path is already quoted
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
	struct stat st;
	int canJustReextract;
#ifdef EXTRACTION_FILES
	if (force == 0) {
		canJustReextract = 1;
		char* extraction_files[] = { EXTRACTION_FILES, NULL };
		char* efile = extraction_files[0];
		char efile_path[PATH_MAX] = {'\0'};
		for (int i = 0; efile != NULL; i++) {
			strcpy(efile_path, destination);
			strcat(efile_path, SLASH);
			strcat(efile_path, efile);
			if (stat(efile_path, &st) != 0) {
				// file to extract not found
				canJustReextract = 0;
			}
			efile = extraction_files[i + 1];
		}
	} else {
		canJustReextract = 0;
	}
#else
	canJustReextract = 0;
#endif
	if (canJustReextract) {
		tinyfd_messageBox("", GAME " game data format changed, we can migrate in-place. Please be patient.", "ok", "info", 1);
	} else if (force == 0) {
		tinyfd_messageBox("Missing data",
						  DATA_NOT_EXTRACTED " Please select the " GAME_CD, "ok", "question", 1);
	} else if (force == 1) {
		tinyfd_messageBox("", "Please select the " GAME_CD, "ok", "question", 1);
	} else if (force == 2) {
		// pass
	}
#ifdef USE_MAC
	int patterncount = 0;
	char* filepatterns[] = { NULL };
	// file types as names not working at least on macOS sierra
#else
	char* filepatterns[] = { GAME_CD_FILE_PATTERNS, NULL };
	int patterncount = 0;
	while (filepatterns[patterncount++] != NULL);
#endif
	char srcfolder[1024] = {'\0'};
	if (!canJustReextract) {
		const char* datafile = tinyfd_openFileDialog(GAME_CD " location", "",
													 patterncount - 1, filepatterns, NULL, 0);
		if (datafile == NULL) {
			exit(-1);
		}
		strcpy(srcfolder, datafile);
		parentdir(srcfolder);
	} else {
		strcpy(srcfolder, destination);
	}

#ifdef WIN32
	char* sourcepath = _strdup(scripts_path);
	if (sourcepath[0] == '"') {
		// if scripts_path is quoted, remove the quotes, i.e.,
		// copy all but the first until all but the last char.
		// sourcepath is already large enough because it used to contain the
		// entire scripts_path
		strncpy(sourcepath, scripts_path + 1, strlen(scripts_path) - 2);
		sourcepath[strlen(scripts_path) - 2] = '\0';
	}
#else
	char* sourcepath = strdup(scripts_path);
#endif
	mkdir_p(destination);

	if (stat(sourcepath, &st) != 0) {
		// deployment time path not found, try compile time path
		strcpy(sourcepath, SRC_PATH());
		parentdir(sourcepath);
	}

#ifndef WIN32
	if (stat(sourcepath, &st) != 0) {
		// deployment time path might be same as extractor 
		strcpy(sourcepath, extractor_tool);
		parentdir(sourcepath);
	}
#endif

	if (stat(sourcepath, &st) != 0) {
		// scripts not found, abort!
		tinyfd_messageBox("Error", "There was an error copying the data, could not discover contributed directory path.", "ok", "error", 1);
		return;
	}

	if (force != 2) {
		char contrib_src_path[BUFF_SIZE];
		char contrib_dest_path[BUFF_SIZE];
		int i = 0;
		int optional = 0;
		char* contrib_directories[] = CONTRIB_DIRECTORIES;
		while (contrib_directories[i] != NULL && contrib_directories[i + 1] != NULL) {
			if (!strcmp(contrib_directories[i], ":optional:")) {
				i += 1;
				optional = 1;
			} else {
				if (contrib_directories[i][0] != '/') {
					// absolute Unix paths are not appended to the source path
					strcpy(contrib_src_path, sourcepath);
					strcat(contrib_src_path, SLASH);
					strcat(contrib_src_path, contrib_directories[i]);
				} else {
					strcpy(contrib_src_path, contrib_directories[i]);
				}

				if (stat(contrib_src_path, &st) != 0) {
					// contrib dir not found, abort!
					if (!optional) {
						tinyfd_messageBox(contrib_directories[i], "There was an error copying the data, could not discover contributed directory path.", "ok", "error", 1);
						return;
					}
				} else {
					strcpy(contrib_dest_path, destination);
					strcat(contrib_dest_path, SLASH);
					strcat(contrib_dest_path, contrib_directories[i + 1]);
					copy_dir(contrib_src_path, contrib_dest_path);
				}
				i += 2;
			}
		}
	}

	char cmdbuf[4096] = {'\0'};
#ifdef USE_MAC
	strcat(cmdbuf, "osascript -e \"tell application \\\"Terminal\\\"\n"
                       "    set w to do script \\\"");
#elif defined(WIN32)
	strcat(cmdbuf, "/C \"");
#else
	if (!isatty(1)) {
		strcat(cmdbuf, "xterm -e bash -c ");
		strcat(cmdbuf, " \"");
	}
#endif
	strcat(cmdbuf, extractor_tool);
	strcat(cmdbuf, " " QUOTE);
	strcat(cmdbuf, srcfolder);
	strcat(cmdbuf, QUOTE " " QUOTE);
	strcat(cmdbuf, destination);
	strcat(cmdbuf, QUOTE);
#ifdef USE_MAC
	strcat(cmdbuf, "; exit\\\"\n"
                       "    repeat\n"
                       "        delay 1\n"
                       "        if not busy of w then exit repeat\n"
                       "    end repeat\n"
                       "end tell\"");
#elif defined(WIN32)
	strcat(cmdbuf, "\"");
#else
	if (!isatty(1)) {
	    strcat(cmdbuf, "; echo 'Press RETURN to continue...'; read\"");
	}
#endif
#ifdef WIN32
	DWORD exitcode = 0;
	SHELLEXECUTEINFO ShExecInfo = { 0 };
	char* toolpath = _strdup(extractor_tool);
	if (PathRemoveFileSpec(toolpath)) {
		// remove the leading quote
		if (toolpath[0] == '"') memmove(toolpath, toolpath + 1, strlen(toolpath) + 1);
	} else {
		// nothing was removed, use current dir
		toolpath = NULL;
	}
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
	printf("Running extractor as %s\n", cmdbuf);
	int exitcode = 0;
	exitcode = system(cmdbuf);
#endif
	if (exitcode != 0) {
		char* extractortext = (char*)calloc(sizeof(char), strlen(cmdbuf) + 1024);
		sprintf(extractortext, "The following command was used to extract the data\n%s", cmdbuf);
		tinyfd_messageBox("Extraction failed!", extractortext, "ok", "error", 1);
#ifdef WIN32
		_unlink(destination);
#else
		unlink(destination);
#endif
	} else if (!canJustReextract && GAME_SHOULD_EXTRACT_AGAIN) {
		ExtractData(extractor_tool, destination, scripts_path, 2);
	}
}

int main(int argc, char * argv[]) {
	struct stat st;
	int argccpy = argc;
	char data_path[BUFF_SIZE];
	char scripts_path[BUFF_SIZE];
	char stratagus_bin[BUFF_SIZE];
	char title_path[BUFF_SIZE];
	char extractor_path[BUFF_SIZE] = {'\0'};

	// Try the extractor from the same dir as we are
	if (strchr(argv[0], SLASH[0])) {
		strcpy(extractor_path, argv[0]);
		parentdir(extractor_path);
		strcat(extractor_path, SLASH EXTRACTOR_TOOL);
#ifdef WIN32
		if (!strstr(extractor_path, ".exe")) {
			strcat(extractor_path, ".exe");
		}
#endif
		if (stat(extractor_path, &st) == 0) {
			// Once we have the path, we quote it by moving the memory one byte to the
			// right, and surrounding it with the quote character and finishing null
			// bytes. Then we add the arguments.
			extractor_path[strlen(extractor_path) + 1] = '\0';
			memmove(extractor_path + 1, extractor_path, strlen(extractor_path));
			extractor_path[0] = QUOTE[0];
			extractor_path[strlen(extractor_path) + 1] = '\0';
			extractor_path[strlen(extractor_path)] = QUOTE[0];
		} else {
			extractor_path[0] = '\0';
		}
	}
	if (extractor_path[0] == '\0') {
		// Use extractor from PATH
		strcpy(extractor_path, EXTRACTOR_TOOL);
		if (!detectPresence(extractor_path)) {
			error(TITLE, EXTRACTOR_NOT_FOUND);
		}
	}
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
		_getcwd(data_path, data_path_size);
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

		if (_chdir(stratagus_path) != 0) {
			error(TITLE, STRATAGUS_NOT_FOUND);
		}
		sprintf(stratagus_bin, "%s\\stratagus.exe", stratagus_path);
	}
#ifdef DATA_PATH
	// usually this isn't defined for windows builds. if it is, use it
	strcpy(data_path, DATA_PATH);
#endif
#else
	strcpy(data_path, DATA_PATH);
	strcpy(scripts_path, SCRIPTS_PATH);
	strcpy(stratagus_bin, STRATAGUS_BIN);
#endif

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

	if ( stat(stratagus_bin, &st) != 0 ) {
#ifdef WIN32
		_fullpath(stratagus_bin, argv[0], BUFF_SIZE);
		PathRemoveFileSpec(stratagus_bin);
		strcat(extractor_path, "\\stratagus.exe");
		if (stat(stratagus_bin, &st) != 0) {
			error(TITLE, STRATAGUS_NOT_FOUND);
		}
#else
		if (!detectPresence(stratagus_bin)) {
			realpath(argv[0], stratagus_bin);
			parentdir(stratagus_bin);
			if (strlen(stratagus_bin) > 0) {
				strcat(stratagus_bin, "/stratagus");
			} else {
				strcat(stratagus_bin, "./stratagus");
			}
			if ( stat(stratagus_bin, &st) != 0 ) {
				error(TITLE, STRATAGUS_NOT_FOUND);
			}
		}
#endif
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
	_chdir(data_path);

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

	// Needed to reduce CPU load while idle threads are wating for havn't finished yet ones
	char *const env[] = { (char*)"OMP_WAIT_POLICY=passive", NULL }; 

#ifdef WIN32
	int ret = _spawnvpe(_P_WAIT, stratagus_bin, stratagus_argv, env);
#else
	int ret = 0;
	int childpid = fork();
	if (childpid == 0) {
		execvpe(stratagus_bin, stratagus_argv, env);
		if (strcmp(stratagus_bin, "stratagus") == 0) {
			realpath(argv[0], stratagus_bin);
			parentdir(stratagus_bin);
			strcat(stratagus_bin, "/stratagus");
		}
		execvp(stratagus_bin, stratagus_argv);
		exit(ENOENT);
	} else if (childpid > 0) {
		waitpid(childpid, &ret, 0);
	} else {
		ret = ENOENT;
	}
#endif
	if (ret == ENOENT) {
		error(TITLE, STRATAGUS_NOT_FOUND);
	} else if (ret != 0) {
		char message[8096] = {'\0'};
		snprintf(message, 8096,
				 "Stratagus failed to load game data. "
				 "If you just launched the game without any arguments, this may indicate a bug with the extraction process. "
				 "Please report this on https://github.com/Wargus/stratagus/issues/new, "
				 "and please give details, including: operating system, installation path, username, kind of source CD. "
				 "A possible solution is to remove the hidden folder %s).", data_path);
		error(TITLE, message);
#ifdef WIN32
		_unlink(title_path);
		_unlink(data_path);
#else
		unlink(title_path);
		unlink(data_path);
#endif
	}
	exit(ret);
}

#endif
