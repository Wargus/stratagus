/*
    attachconsole.cpp - WINAPI AttachConsole
    Copyright (C) 2009-2011  Pali Rohár <pali.rohar@gmail.com>

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

#if ( defined(WIN32) || defined(_MSC_VER) ) && ( defined(NO_STDIO_REDIRECT) || ! defined(REDIRECT_OUTPUT) )

#define WINVER 0x0501
#include <windows.h>
#include <wincon.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef __cplusplus
#include <iostream>
#endif

/// Try attach console of parent process for std input/output in Windows NT, 2000, XP or new
static void WINAPI_AttachConsole(void) {

	OSVERSIONINFO osvi;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	int hasVersion = GetVersionEx(&osvi);

	if ( ! hasVersion )
		return;

	int version = 0;
	version |= osvi.dwMinorVersion;
	version |= osvi.dwMajorVersion << 8;

	if ( version < 0x0500 )
		return;

	int attached = AttachConsole(ATTACH_PARENT_PROCESS);

	if ( ! attached )
		return;

	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);

	if ( hIn == NULL || hOut == NULL || hErr == NULL )
		return;

	if ( hIn == INVALID_HANDLE_VALUE || hOut == INVALID_HANDLE_VALUE || hErr == INVALID_HANDLE_VALUE )
		return;

	int osIn = _open_osfhandle((intptr_t) hIn, O_TEXT);
	int osOut = _open_osfhandle((intptr_t) hOut, O_TEXT);
	int osErr = _open_osfhandle((intptr_t) hErr, O_TEXT);

	if ( osIn == -1 || osOut == -1 || osErr == -1 )
		return;

	FILE * fpIn = _fdopen(osIn, "r");
	FILE * fpOut = _fdopen(osOut, "w");
	FILE * fpErr = _fdopen(osErr, "w");

	if ( ! fpIn || ! fpOut || ! fpErr )
		return;

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

	*stdin = *fpIn;
	*stdout = *fpOut;
	*stderr = *fpErr;

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

#ifdef __cplusplus
	std::cin.clear();
	std::cout.clear();
	std::cerr.clear();
	std::ios::sync_with_stdio();
#endif

	printf("\n\n");

}

/// This section set that WINAPI_AttachConsole() will be called at application startup before main()
#ifdef _MSC_VER
#pragma section(".CRT$XCU", long, read)
__declspec(allocate(".CRT$XCU")) void (*initialize)(void) = WINAPI_AttachConsole;
#else
__attribute__((constructor)) static void initialize(void) { WINAPI_AttachConsole(); }
#endif

#endif
