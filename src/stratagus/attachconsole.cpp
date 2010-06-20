/*
    attachconsole.cpp - WINAPI AttachConsole
    Copyright (C) 2009-2010  Pali Rohár <pali.rohar@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifdef WIN32

#define WINVER 0x0501
#include <windows.h>
#include <wincon.h>
#include <io.h>
#include <fcntl.h>
#include <cstdio>
#include <iostream>

#include "attachconsole.h"

/// Try attach console of parent process for std input/output in Windows NT, 2000, XP or new
bool WINAPI_AttachConsole() {

	OSVERSIONINFO osvi;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	bool hasVersion = GetVersionEx(&osvi);

	if ( ! hasVersion )
		return false;

	short version = 0;
	version |= (char) osvi.dwMinorVersion;
	version |= (char) osvi.dwMajorVersion << 8;

	if ( version < 0x0500 )
		return false;

	bool attached = AttachConsole(ATTACH_PARENT_PROCESS);

	if ( ! attached )
		return false;

	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);

	if ( hIn == NULL || hOut == NULL || hErr == NULL )
		return false;

	if ( hIn == INVALID_HANDLE_VALUE || hOut == INVALID_HANDLE_VALUE || hErr == INVALID_HANDLE_VALUE )
		return false;

	int osIn = _open_osfhandle((long) hIn, O_TEXT);
	int osOut = _open_osfhandle((long) hOut, O_TEXT);
	int osErr = _open_osfhandle((long) hErr, O_TEXT);
				
	if ( osIn == -1 || osOut == -1 || osErr == -1 )
		return false;

	FILE * fpIn = _fdopen(osIn, "r");
	FILE * fpOut = _fdopen(osOut, "w");
	FILE * fpErr = _fdopen(osErr, "w");

	if ( ! fpIn || ! fpOut || ! fpErr )
		return false;

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

	*stdin = *fpIn;
	*stdout = *fpOut;
	*stderr = *fpErr;

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	std::cin.clear();
	std::cout.clear();
	std::cerr.clear();

	std::ios::sync_with_stdio();

	printf("\n\n");

	return true;

}

#endif //WIN32
