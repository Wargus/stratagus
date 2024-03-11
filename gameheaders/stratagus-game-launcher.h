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
 * #define EXTRACTOR_ARGS {"-v", nullptr}
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
#include <sys/types.h>
#ifdef DOXYGEN
# define GAME_NAME
# define GAME_CD
# define GAME
# define DATA_PATH
# define SCRIPTS_PATH
# define STRATAGUS_BIN
#endif

#if !defined(GAME_NAME) || !defined(GAME_CD) || !defined(GAME) || !defined(EXTRACTOR_TOOL)
# error You need to define all Game macros, see stratagus-game-launcher.h
#endif

#ifndef GAME_SHOULD_EXTRACT_AGAIN
# define GAME_SHOULD_EXTRACT_AGAIN false
#endif

/**
 * \def TITLE_PNG
 * OPTIONAL: Path to title screen (for testing if data was extracted)
 **/
#ifndef TITLE_PNG
# ifdef WIN32
#  define TITLE_PNG "%s\\graphics\\ui\\title.png"
# else
#  define TITLE_PNG "%s/graphics/ui/title.png"
# endif
#endif

#ifndef WIN32
# if !defined(DATA_PATH) || !defined(SCRIPTS_PATH) || !defined(STRATAGUS_BIN)
#  error You need to define paths, see stratagus-game-launcher.h
# endif
# pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#ifdef _MSC_VER
# pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif

#ifdef _WIN64
# define REGKEY \
	 "Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Stratagus (64 bit)"
#elif defined(WIN32)
# define REGKEY "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Stratagus"
#endif

#define TITLE GAME_NAME
#define EXTRACTOR_NOT_FOUND GAME_NAME " could not find its extraction tool.\n" EXTRACTOR_TOOL "!\n"
#define STRATAGUS_NOT_FOUND \
	"Stratagus is not installed.\nYou need Stratagus to run " GAME_NAME "!\n"
#define DATA_NOT_EXTRACTED \
	GAME_NAME " data was not extracted, is corrupted, or outdated.\nYou need to extract it from " \
	          "original " GAME_CD "."

#include "stratagus-gameutils.h"

#include <algorithm>

const char *argv0;

static void SetUserDataPath(char *data_path)
{
#if defined(WIN32)
	if (fs::exists(fs::path(data_path) / "portable-install")) {
		return;
	}
	SHGetFolderPathA(nullptr, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, nullptr, 0, data_path);
	if (!data_path[0]) {
		strcpy(data_path, getenv("APPDATA"));
	}
	strcat(data_path, "\\Stratagus\\");
#else
	char *appimage_ptr = getenv("APPIMAGE");
	std::string appimage;
	if (appimage_ptr != nullptr) {
		appimage = std::string(appimage_ptr);
	}
	if (!appimage.empty() && fs::exists(fs::path(appimage))) {
		if (fs::exists(fs::path(appimage + ".data"))) {
			strcpy(data_path, (appimage + ".data/stratagus").c_str());
			strcat(data_path, "data." GAME_NAME);
			return;
		}
	}
	char *dataDir = getenv("XDG_DATA_HOME");
	if (dataDir) {
		strcpy(data_path, dataDir);
		strcat(data_path, "/stratagus/");
	} else {
		dataDir = getenv("HOME");
		if (dataDir) {
			strcpy(data_path, dataDir);
# ifdef USE_MAC
			strcat(data_path, "/Library/Application Support/Stratagus/");
# else
			strcat(data_path, "/.local/share/stratagus/");
# endif
		}
	}
#endif
	strcat(data_path, "data." GAME_NAME);
}

static int check_version(char *tool_path, const fs::path &data_path)
{
	char dataversion[20] = {'\0'};
	if (FILE *f = fopen((data_path / "extracted").string().c_str(), "r")) {
		fgets(dataversion, 20, f);
		fclose(f);
	} else {
#ifdef CHECK_EXTRACTED_VERSION
		return 0; // No file means we have a problem
#else
		return 1; // No file means we don't care
#endif
	}
	char toolversion[20] = {'\0'};
	char buf[4096] = {'\0'};
	sprintf(buf, "%s -V", tool_path); // tool_path is already quoted
#ifndef WIN32
	if (FILE *pipe = popen(buf, "r")) {
		fgets(toolversion, 20, pipe);
		pclose(pipe);
	}
#else
	HANDLE g_hChildStd_OUT_Rd = nullptr;
	HANDLE g_hChildStd_OUT_Wr = nullptr;
	DWORD nbByteRead;
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = nullptr;
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) return 1;
	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) return 1;
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFOA siStartInfo;
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
	siStartInfo.cb = sizeof(STARTUPINFOA);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	if (!CreateProcessA(
			nullptr, buf, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &siStartInfo, &piProcInfo))
		return 1;
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);
	ReadFile(g_hChildStd_OUT_Rd, toolversion, 20, &nbByteRead, nullptr);
#endif
	// strip whitespace
	for (size_t i = 0, j = 0; toolversion[j] = toolversion[i]; j += !isspace(toolversion[i++]))
		;
	for (size_t i = 0, j = 0; dataversion[j] = dataversion[i]; j += !isspace(dataversion[i++]))
		;
	if (strcmp(dataversion, toolversion) == 0) {
		return 1;
	}
	return 0;
}

static void ExtractData(char *extractor_tool,
                        const char *const extractor_args[],
                        char *destination,
                        char *scripts_path,
                        int force = 0,
                        char *datafileCstr = nullptr)
{
	bool canJustReextract = false;
#ifdef EXTRACTION_FILES
	if (force == 0) {
		const char *extraction_files[] = {EXTRACTION_FILES};

		canJustReextract =
			std::all_of(std::begin(extraction_files),
		                std::end(extraction_files),
		                [&](const auto *file) { return fs::exists(fs::path(destination) / file); });
	}
#endif
	if (canJustReextract) {
		tinyfd_messageBox("",
		                  GAME
		                  " game data format changed, we can migrate in-place. Please be patient.",
		                  "ok",
		                  "info",
		                  1);
	} else if (force == 0) {
		tinyfd_messageBox(
			"Missing data", DATA_NOT_EXTRACTED " Please select the " GAME_CD, "ok", "question", 1);
	} else if (force == 1) {
		tinyfd_messageBox("", "Please select the " GAME_CD, "ok", "question", 1);
	} else if (force == 2) {
		// pass
	}
#ifdef USE_MAC
	int patterncount = 0;
	char *filepatterns[] = {nullptr};
	// file types as names not working at least on macOS sierra
#else
	const char *filepatterns[] = {GAME_CD_FILE_PATTERNS, nullptr};
	int patterncount = 0;
	while (filepatterns[patterncount++] != nullptr)
		;
#endif
	fs::path srcfolder;
	if (!canJustReextract || datafileCstr != nullptr) {
		if (datafileCstr == nullptr) {
			datafileCstr = tinyfd_openFileDialog(
				GAME_CD " location", "", patterncount - 1, filepatterns, nullptr, 0);
		}
		if (datafileCstr == nullptr) {
			exit(-1);
		}
		std::string datafile = datafileCstr;
		if (datafile.compare(datafile.length() - 4, 4, ".exe") == 0) {
			// test if this is an innoextract installer and if so, extract it to a tempdir and pass that
#ifdef WIN32
			char moduleFileName[BUFF_SIZE];
			memset(moduleFileName, 0, sizeof(moduleFileName));
			GetModuleFileNameA(nullptr, moduleFileName, sizeof(moduleFileName) - 1);
			fs::path innoextractPath = fs::path(moduleFileName).parent_path() / "innoextract.exe";
			std::wstring file = innoextractPath.wstring();
			std::vector<std::wstring> argv = {L"-i", fs::path(datafile).wstring()};
#else
			const char *file = "innoextract";
			char *argv[] = {"-i", (char *) datafile.c_str(), nullptr, nullptr, nullptr};
#endif
			if (runCommand(file, argv) == 0) {
				// innoextract exists and this exe file is an innosetup file
				const fs::path tmpp = fs::temp_directory_path() / GAME;
				fs::create_directories(tmpp);
				const fs::path curdir = fs::current_path();
				fs::current_path(tmpp);
#ifdef WIN32
				argv[0] = L"-m";
#else
				argv[0] = "-m";
				argv[1] = "-d";
				argv[2] = (char *) tmpp.string().c_str();
				argv[3] = (char *) datafile.c_str();
#endif
				if (runCommand(file, argv) != 0) {
					error("Problem with installer",
					      "You selected an innosetup installer, and we could not extract it. "
					      "Please extract it manually and point the extraction tool there.");
				} else {
					srcfolder = tmpp;
					fs::current_path(curdir);
				}
			} else {
				// we cannot test if this is an innoextract installer, assume not but maybe warn
				if (datafile.compare("INSTALL.EXE") == 0 || datafile.compare("install.exe") == 0
				    || datafile.compare("INSTALL.exe") == 0 || datafile.compare("SETUP.EXE") == 0
				    || datafile.compare("setup.exe") == 0 || datafile.compare("SETUP.exe") == 0) {
					// probably not a packaged installer
				} else {
					// warn
					tinyfd_messageBox(
						"",
						"You selected an exe file, but I cannot run innoextract "
						"to check if its a single-file installer. If it is, please extract/install "
						"manually first and then run " GAME " again.",
						"ok",
						"question",
						1);
				}
				srcfolder = fs::path(datafile).parent_path();
			}
		} else {
			srcfolder = fs::path(datafile).parent_path();
		}
	} else {
		srcfolder = fs::path(destination);
	}

#ifdef WIN32
	fs::path sourcepath;
	if (scripts_path[0] == '"') {
		// if scripts_path is quoted, remove the quotes
		sourcepath = std::string(scripts_path).substr(1, strlen(scripts_path) - 2);
	} else {
		sourcepath = scripts_path;
	}
#else
	fs::path sourcepath;
	if (scripts_path[0] != '/') {
		fs::path normalized_path(argv0);
		fs::path relative_path(scripts_path);
		for (auto it = relative_path.begin(); it != relative_path.end(); ++it) {
			if (*it == fs::path("..")) {
				normalized_path = normalized_path.parent_path();
			} else {
				normalized_path = normalized_path / *it;
			}
		}
		sourcepath = normalized_path;
	} else {
		sourcepath = scripts_path;
	}
#endif

	fs::create_directories(fs::path(destination));

	if (!fs::exists(sourcepath)) {
		// deployment time path not found, try compile time path
		sourcepath = fs::path(SRC_PATH()).parent_path();
	}

#ifndef WIN32
	if (!fs::exists(sourcepath)) {
		// deployment time path might be same as extractor
		sourcepath = fs::path(extractor_tool).parent_path();
	}
#endif

	if (!fs::exists(sourcepath)) {
		// scripts not found, abort!
		std::string msg("There was an error copying the data, could not discover scripts path: ");
		msg += sourcepath.u8string();
		tinyfd_messageBox("Error", msg.c_str(), "ok", "error", 1);
		return;
	}

	if (force != 2) {
		fs::path contrib_dest_path(destination);
		int i = 0;
		int optional = 0;
		const char *contrib_directories[] = CONTRIB_DIRECTORIES;
		while (contrib_directories[i] != nullptr && contrib_directories[i + 1] != nullptr) {
			if (!strcmp(contrib_directories[i], ":optional:")) {
				i += 1;
				optional = 1;
			} else {
				fs::path contrib_src_path = sourcepath / contrib_directories[i];

				if (!fs::exists(contrib_src_path)) {
					// contrib dir not found, abort!
					if (!optional) {
						std::string msg("There was an error copying the data, could not discover "
						                "contributed directory path: ");
						msg += contrib_src_path.string();
						error("Error", msg.c_str());
						return;
					}
				} else {
					copy_dir(contrib_src_path, contrib_dest_path / contrib_directories[i + 1]);
				}
				i += 2;
			}
		}
	}

	int exitcode = 0;

	char cmdbuf[4096] = {'\0'};
#ifdef WIN32
	std::vector<std::wstring> args;
	std::wstring file = fs::path(extractor_tool).wstring();

	for (int i = 0;; i++) {
		const char *earg = extractor_args[i];
		if (earg == nullptr) {
			break;
		} else {
			const size_t WCHARBUF = 100;
			wchar_t wszDest[WCHARBUF];
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, earg, -1, wszDest, WCHARBUF);
			args.push_back(wszDest);
		}
	}
	args.push_back(srcfolder.wstring());
	args.push_back(fs::path(destination).wstring());
	std::wstring combinedCommandline;
	exitcode = runCommand(file, args, true, &combinedCommandline);
#else

# ifdef USE_MAC
	strcat(cmdbuf,
	       "osascript -e \"tell application \\\"Terminal\\\"\n"
	       "    set w to do script \\\"");
# else
	bool hasXterm = false;
	if (!isatty(1)) {
		hasXterm = detectPresence("xterm");
		if (hasXterm) {
			strcat(cmdbuf, "xterm -e bash -c ");
			strcat(cmdbuf, " \"");
		} else {
			tinyfd_messageBox("",
			                  "Extracting data, cannot find xterm to display output.\n"
			                  "Please be patient. If something fails, re-run from terminal.",
			                  "ok",
			                  "info",
			                  1);
		}
	}
	if (getenv("APPIMAGE") && !detectPresence("ffmpeg")) {
		tinyfd_messageBox(
			"",
			"Could not find ffmpeg on PATH, video\nand/or audio conversion may not work...",
			"ok",
			"info",
			1);
	}
# endif

# ifdef USE_MAC
	strcat(cmdbuf, fs::absolute(fs::path(extractor_tool)).c_str());
# else
	strcat(cmdbuf, extractor_tool);
# endif
	for (int i = 0;; i++) {
		const char *earg = extractor_args[i];
		if (earg == nullptr) {
			break;
		} else {
			strcat(cmdbuf, " ");
			strcat(cmdbuf, earg);
		}
	}
	strcat(cmdbuf, " " QUOTE);
	strcat(cmdbuf, srcfolder.string().c_str());
	strcat(cmdbuf, QUOTE " " QUOTE);
	strcat(cmdbuf, destination);
	strcat(cmdbuf, QUOTE);

# ifdef USE_MAC
	strcat(cmdbuf,
	       "; exit\\\"\n"
	       "    repeat\n"
	       "        delay 1\n"
	       "        if not busy of w then exit repeat\n"
	       "    end repeat\n"
	       "end tell\"");
# else
	if (!isatty(1)) {
		if (hasXterm) {
			strcat(cmdbuf, "; echo 'Press RETURN to continue...'; read\"");
		}
	}
# endif

	printf("Running extractor as %s\n", cmdbuf);
	exitcode = system(cmdbuf);
#endif

	if (exitcode != 0) {
#ifdef WIN32
		WideCharToMultiByte(CP_ACP,
		                    0,
		                    combinedCommandline.c_str(),
		                    combinedCommandline.size(),
		                    cmdbuf,
		                    sizeof(cmdbuf) - 1,
		                    nullptr,
		                    nullptr);
#endif
		char *extractortext = (char *) calloc(sizeof(char), strlen(cmdbuf) + 1024);
		for (int i = 0; i < strlen(cmdbuf); i++) {
			if (cmdbuf[i] == '"' || cmdbuf[i] == '\'') {
				cmdbuf[i] = ' ';
			}
		}
		sprintf(extractortext,
		        "The following command was used to extract the data (you can run it manually in a "
		        "console to find out more):\n%s",
		        cmdbuf);
		tinyfd_messageBox("Extraction failed!", extractortext, "ok", "error", 1);
	} else if (!canJustReextract && GAME_SHOULD_EXTRACT_AGAIN) {
		ExtractData(extractor_tool, extractor_args, destination, scripts_path, 2);
	}
}

int main(int argc, char *argv[])
{
	int argccpy = argc;
	char data_path[BUFF_SIZE];
	char scripts_path[BUFF_SIZE];
	char stratagus_bin[BUFF_SIZE];
	char title_path[BUFF_SIZE];

	// set global variable to this executable
#ifndef WIN32
	// we accept a special argument to get our own location, if we see that,
	// shift the other arguments down. This is not documented, and right now
	// mainly used for AppImages
	const char argv0prefix[] = "--argv0=";
	if (argv[1] && strstr(argv[1], argv0prefix) == argv[1]) {
		argv0 = realpath(argv[1] + strlen(argv0prefix), nullptr);
		for (int i = 1; i < argc - 1; i++) {
			argv[i] = argv[i + 1];
		}
		argc--;
	} else {
		argv0 = realpath(argv[0], nullptr);
	}
#endif
	argv0 = argv[0];

	// Try the extractor from the same dir as we are
	char extractor_path[BUFF_SIZE] = {'\0'};
	if (strchr(argv0, SLASH[0])) {
		strcpy(extractor_path, argv0);
		parentdir(extractor_path);
		strcat(extractor_path, SLASH EXTRACTOR_TOOL);
#ifdef WIN32
		if (!strstr(extractor_path, ".exe")) {
			strcat(extractor_path, ".exe");
		}
#endif
		if (fs::exists(extractor_path)) {
#ifndef WIN32
			// Once we have the path, we quote it by moving the memory one byte to the
			// right, and surrounding it with the quote character and finishing null
			// bytes. Then we add the arguments.
			extractor_path[strlen(extractor_path) + 1] = '\0';
			memmove(extractor_path + 1, extractor_path, strlen(extractor_path));
			extractor_path[0] = QUOTE[0];
			extractor_path[strlen(extractor_path) + 1] = '\0';
			extractor_path[strlen(extractor_path)] = QUOTE[0];
#endif
		} else {
			extractor_path[0] = '\0';
		}
	}
	if (extractor_path[0] == '\0') {
		// Use extractor from PATH
		strcpy(extractor_path, EXTRACTOR_TOOL);
		if (!detectPresence(extractor_path)) {
			error(TITLE,
			      (std::string(EXTRACTOR_NOT_FOUND) + " (expected at " + extractor_path + ")")
			          .c_str());
		}
	}

#ifdef WIN32
	char executable_path[BUFF_SIZE]{};
	GetModuleFileNameA(nullptr, executable_path, sizeof(executable_path) - 1);

	char executable_drive[_MAX_DRIVE]{};
	char executable_dir[_MAX_DIR]{};
	_splitpath(executable_path, executable_drive, executable_dir, nullptr, nullptr);

	const size_t data_path_size = sizeof(data_path);
	memset(data_path, 0, data_path_size);

	if (executable_path[0] && executable_drive[0] && executable_dir[0]) {
		PathCombineA(data_path, executable_drive, executable_dir);
	} else {
		_getcwd(data_path, data_path_size);
	}
	PathRemoveBackslashA(data_path);
	sprintf(scripts_path, "\"%s\"", data_path);

	char stratagus_path[BUFF_SIZE]{};

	// Try to use stratagus.exe from data (install) directory first
	sprintf(stratagus_bin, "%s\\stratagus.exe", data_path);
	if (!fs::exists(stratagus_bin)) {
		// If no local stratagus.exe is present, search PATH
		if (!SearchPathA(nullptr, "stratagus", ".exe", MAX_PATH, stratagus_bin, nullptr)
		    && !SearchPathA(nullptr, "stratagus-dbg", ".exe", MAX_PATH, stratagus_bin, nullptr)) {
			// If no local or PATH stratagus.exe is present, look for a globally installed version
			DWORD stratagus_path_size = sizeof(stratagus_path);
			HKEY key;

			if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, REGKEY, 0, KEY_QUERY_VALUE, &key)
			    == ERROR_SUCCESS) {
				if (RegQueryValueExA(key,
				                     "InstallLocation",
				                     nullptr,
				                     nullptr,
				                     (LPBYTE) stratagus_path,
				                     &stratagus_path_size)
				    == ERROR_SUCCESS) {
					if (stratagus_path_size == 0 || strlen(stratagus_path) == 0) {
						error(TITLE,
						      (std::string(STRATAGUS_NOT_FOUND)
						       + " (expected globally installed or in " + stratagus_bin + ")")
						          .c_str());
					}
				}
				RegCloseKey(key);
			}

			if (_chdir(stratagus_path) != 0) {
				error(TITLE,
				      (std::string(STRATAGUS_NOT_FOUND) + " (registry key found, but directory "
				       + stratagus_path + " cannot be opened)")
				          .c_str());
			}
			sprintf(stratagus_bin, "%s\\stratagus.exe", stratagus_path);
		}
	}

# ifdef DATA_PATH
	// usually this isn't defined for windows builds. if it is, use it
	strcpy(data_path, DATA_PATH);
# endif
#else
	strcpy(data_path, DATA_PATH);
	strcpy(scripts_path, SCRIPTS_PATH);
	strcpy(stratagus_bin, STRATAGUS_BIN);
#endif

	const char *const extractor_args[] = EXTRACTOR_ARGS;

	if (argc == 2) {
		if (fs::exists(argv[1])) {
			// extraction file given as argument and it is accessible => force extraction and exit
			tinyfd_forceConsole = 1;
			SetUserDataPath(data_path);
			ExtractData(extractor_path, extractor_args, data_path, scripts_path, 2, argv[1]);
			return 0;
		} else if (!strcmp(argv[1], "--extract")) {
			// Force extraction and exit
			SetUserDataPath(data_path);
			ExtractData(extractor_path, extractor_args, data_path, scripts_path, 1);
			return 0;
		} else if (!strcmp(argv[1], "--extract-no-gui")) {
			// Force extraction without ui and exit
			tinyfd_forceConsole = 1;
			SetUserDataPath(data_path);
			ExtractData(extractor_path, extractor_args, data_path, scripts_path, 1);
			return 0;
		} else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
			printf("Usage: %s [path to extraction file|--extract|--extract-no-gui]\n"
			       "\tpath to extraction file - will be used as file to start the extraction "
			       "process on\n"
			       "\t--extract - force extraction even if data is already extracted\n"
			       "\t--extract-no-gui - force extraction even if data is already extracted, using "
			       "the console only for prompts\n\n",
			       argv[0]);
		}
	}

	if (!fs::exists(stratagus_bin)) {
#ifdef WIN32
		_fullpath(stratagus_bin, argv0, BUFF_SIZE);
		PathRemoveFileSpecA(stratagus_bin);
		strcat(stratagus_bin, "\\stratagus.exe");
		if (!fs::exists(stratagus_bin)) {
			error(TITLE,
			      (std::string(STRATAGUS_NOT_FOUND) + " (expected in " + stratagus_bin + ")")
			          .c_str());
		}
#else
		if (!detectPresence(stratagus_bin)) {
			realpath(argv0, stratagus_bin);
			parentdir(stratagus_bin);
			if (strlen(stratagus_bin) > 0) {
				strcat(stratagus_bin, "/stratagus");
			} else {
				strcat(stratagus_bin, "./stratagus");
			}
			if (!fs::exists(stratagus_bin)) {
				error(TITLE,
				      (std::string(STRATAGUS_NOT_FOUND) + " (expected in " + stratagus_bin + ")")
				          .c_str());
			}
		}
#endif
	}

	sprintf(title_path, TITLE_PNG, data_path);
	if (!fs::exists(title_path)) {
		SetUserDataPath(data_path);
		sprintf(title_path, TITLE_PNG, data_path);
		if (!fs::exists(title_path)) {
			ExtractData(extractor_path, extractor_args, data_path, scripts_path);
		}
		if (!fs::exists(title_path)) {
			std::string msg(DATA_NOT_EXTRACTED);
			msg += " (extraction was attempted, but it seems an error occurred)";
			error(TITLE, msg.c_str());
		}
	}

	if (!check_version(extractor_path, data_path)) {
		ExtractData(extractor_path, extractor_args, data_path, scripts_path);
	}

#ifdef WIN32
	int data_path_len = strlen(data_path);
	fs::current_path(data_path);

	for (int i = data_path_len - 1; i >= 0; --i) {
		data_path[i + 1] = data_path[i];
	}
	data_path[0] = '"';
	data_path[data_path_len + 1] = '"';
	data_path[data_path_len + 2] = '\0';
#endif

#ifdef _MSC_VER
	char **stratagus_argv;
	stratagus_argv = (char **) malloc((argc + 3) * sizeof(*stratagus_argv));
#else
	char *stratagus_argv[argc + 3];
#endif

#ifdef WIN32
	char stratagus_argv0_esc[BUFF_SIZE];
	memset(stratagus_argv0_esc, 0, sizeof(stratagus_argv0_esc));
	strcpy(stratagus_argv0_esc + 1, argv0);
	stratagus_argv0_esc[0] = '"';
	stratagus_argv0_esc[strlen(argv0) + 1] = '"';
	stratagus_argv0_esc[strlen(argv0) + 2] = 0;
	stratagus_argv[0] = stratagus_argv0_esc;
#else
	stratagus_argv[0] = strdup(argv0);
#endif

	stratagus_argv[1] = (char *) "-d";
	stratagus_argv[2] = data_path;

	for (int i = 3; i < argc + 2; ++i) {
		stratagus_argv[i] = argv[i - 2];
	}
	stratagus_argv[argc + 2] = nullptr;

	// Needed to reduce CPU load while idle threads are waiting for not finished yet ones
	extern char **environ;
	int i = 0;
	while (environ[i]) {
		i++;
	}
	environ[i] = (char *) "OMP_WAIT_POLICY=passive";
	environ[i + 1] = nullptr;
#ifdef WIN32
	int ret = _spawnvpe(_P_WAIT, stratagus_bin, stratagus_argv, environ);
#else
	int ret = 0;
	int childpid = fork();
	if (childpid == 0) {
		execvp(stratagus_bin, stratagus_argv);
		if (strcmp(stratagus_bin, "stratagus") == 0) {
			realpath(argv0, stratagus_bin);
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
		std::string msg = "Execution failed for: ";
		msg += stratagus_bin;
		msg += " ";
		for (int i = 1; stratagus_argv[i] != nullptr; i++) {
			msg += stratagus_argv[i];
			msg += " ";
		}
		error(TITLE, msg.c_str());
	} else if (ret != 0) {
		char message[8096 * 2] = {'\0'};
		snprintf(
			message,
			8096 * 2,
			"Stratagus failed to load game data.\n"
			"If you just launched the game without any arguments, this may indicate a bug with the "
		    "extraction process.\n"
			"Please report this on https://github.com/Wargus/stratagus/issues/new,\n"
			"and please give details, including: operating system, installation path, username, "
		    "kind of source CD.\n"
			"If you got an error message about the extraction command failing, please try to run "
		    "it in a console\n"
			"and post the output to the issue. A common problem is symbols in the path for the "
		    "installation, the game data path,\n"
			"or the username (like an ampersand or exclamation mark). Try changing these.\n"
#ifndef WIN32
# ifdef WIN32
			"Also check if the file '%s' exists and check for errors or post it to the issue.\n"
# endif
			"Try also to remove the folder %s and try the extraction again.\n",
# ifdef WIN32
			GetExtractionLogPath(GAME_NAME, data_path),
# endif
			data_path);
#else
			"If not already done, please try using the portable version and check for stdout.txt, "
		    "stderr.txt, and an extraction.log in the folder.\n");
#endif
		error(TITLE, message);
		fs::remove(title_path);
		fs::remove(data_path);
	}
	exit(ret);
}

#endif
