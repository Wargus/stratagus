/*
    Copyright (C) 2015  Pali Rohár <pali.rohar@gmail.com>

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

/* _WIN32_WINNT must be set to at least 0x0501 */
#ifdef _WIN32_WINNT
#if _WIN32_WINNT < 0x0501
#undef _WIN32_WINNT
#endif
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <errno.h>
#include <fcntl.h>

#ifdef __cplusplus
#include <iostream>
#endif

#include "SetupConsole_win32.h"

/*
 * Microsoft systems were and always are one big mess where even standard output does not work :-(
 * There are at least 4 different I/O API levels and each WINAPI function initilize correctly just one...
 * For example standard output information is stored in: DWORD STD_OUTPUT_HANDLE, std::istream std::cout, FILE* stdout, int fd (value 1)
 * This code tries to use console window of parent process and properly initialize all I/O API levels and make sure they are not out of sync
 */

bool IsHandleConsole(HANDLE handle) {

    if (!handle || handle == INVALID_HANDLE_VALUE)
        return false;

    /* GetConsoleMode() returns false on non console handlers */
    DWORD mode;
    return GetConsoleMode(handle, &mode);

}

bool IsStdoutConsole() {

    return IsHandleConsole(GetStdHandle(STD_OUTPUT_HANDLE));

}

bool IsStderrConsole() {

    return IsHandleConsole(GetStdHandle(STD_ERROR_HANDLE));

}

bool IsHandleValid(HANDLE handle) {

    if (!handle || handle == INVALID_HANDLE_VALUE)
        return false;

    DWORD type = GetFileType(handle);
    DWORD error = GetLastError();

    /* When error occure GetFileType() returns FILE_TYPE_UNKNOWN and GetLastError() returns error code */
    return (type != FILE_TYPE_UNKNOWN || error == NO_ERROR);

}

bool IsStreamValid(FILE *stream) {

    return _fileno(stream) >= 0;

}

bool IsFdValid(int fd) {

    /* Function _isatty returns non zero value when parameter is a terminal, console, printer, or serial port */
    /* When parameter is bad file descriptor, then zero is returned and errno is set to EBADF */
    errno = 0;
    return (_isatty(fd) != 0 || errno != EBADF);

}

bool SetStreamAndFdFromStdConsole(FILE *stream, int fd, DWORD std, bool input) {

    /* Retrieves a handle to the standard device of active console screen */
    HANDLE handle = GetStdHandle(std);
    if (!handle || handle == INVALID_HANDLE_VALUE)
        return false;

    /* Allocates new file descriptor and associates it with handle */
    int fd_new = _open_osfhandle((intptr_t)handle, _O_TEXT | (input ? _O_RDONLY : _O_APPEND));
    if (fd_new < 0)
        return false;

    /* Retrieves existing file descriptor from FILE* stream */
    int fd_old = _fileno(stream);
    if (fd_old >= 0 && fd_old != fd_new) {
        /* Reassigns new file descriptor to existing one from FILE* stream */
        if (_dup2(fd_new, fd_old) < 0) {
            _close(fd_new);
            return false;
        }
    }

    if (fd_new != fd) {
        /* Reassigns new file descriptor to one provided by function arg */
        if (_dup2(fd_new, fd) < 0) {
            _close(fd_new);
            return false;
        }
    }

    /* Clean up */
    if (fd_old < 0 || fd_old != fd_new)
        _close(fd_new);

    /* Allocates new FILE* stream and associates it with file descriptor */
    FILE *stream_new = _fdopen(fd, (input ? "r" : "a"));
    if (!stream_new) {
        _close(fd);
        return false;
    }

    /* Close old FILE* stream */
    fclose(stream);

    /* Set new FILE* stream */
    *stream = *stream_new;

    /* Disable bufferring */
    setvbuf(stream, NULL, _IONBF, 0);

    return true;

}

void SetupConsole() {

    /* Get current handlers */
    HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE errorHandle = GetStdHandle(STD_ERROR_HANDLE);

    bool stdin_ok = IsHandleValid(inputHandle);
    bool stdout_ok = IsHandleValid(outputHandle);
    bool stderr_ok = IsHandleValid(errorHandle);

    /* If stdin, stdout and stderr is valid we do not need to do anything */
    if (stdin_ok && stdout_ok && stderr_ok)
        return;

    /* Attaches the calling process to the console of parent process */
    /* After successfull call STD_INPUT_HANDLE, STD_OUTPUT_HANDLE and STD_ERROR_HANDLE are changed to new console screen */
    if (!AttachConsole(ATTACH_PARENT_PROCESS))
        return;

    /* If console handlers were valid before AttachConsole() call, set handlers back (we do not want to overwrite them) */
    if (stdin_ok)
        SetStdHandle(STD_INPUT_HANDLE, inputHandle);
    if (stdout_ok)
        SetStdHandle(STD_OUTPUT_HANDLE, outputHandle);
    if (stderr_ok)
        SetStdHandle(STD_ERROR_HANDLE, errorHandle);

    /* And now STD_INPUT_HANDLE, STD_OUTPUT_HANDLE and STD_ERROR_HANDLE are valid and set correctly */

    stdin_ok = (IsStreamValid(stdin) && IsFdValid(0));
    stdout_ok = (IsStreamValid(stdout) && IsFdValid(1));
    stderr_ok = (IsStreamValid(stderr) && IsFdValid(2));

    bool stdin_console = false;
    bool stdout_console = false;
    bool stderr_console = false;

    /* Try to set standard C streams (stdin/stdout/stderr) and fds (0/1/2) from active console screen (STD_INPUT_HANDLE/STD_OUTPUT_HANDLE/STD_ERROR_HANDLE) */
    if (!stdin_ok)
        stdin_console = SetStreamAndFdFromStdConsole(stdin, 0, STD_INPUT_HANDLE, true);
    if (!stdout_ok)
        stdout_console = SetStreamAndFdFromStdConsole(stdout, 1, STD_OUTPUT_HANDLE, false);
    if (!stderr_ok)
        stderr_console = SetStreamAndFdFromStdConsole(stderr, 2, STD_ERROR_HANDLE, false);

    /* If everything failed then detach console and try to restore original console handlers... we cannot do more */
    if (!stdin_console && !stdout_console && !stderr_console) {
        FreeConsole();
        SetStdHandle(STD_INPUT_HANDLE, inputHandle);
        SetStdHandle(STD_OUTPUT_HANDLE, outputHandle);
        SetStdHandle(STD_ERROR_HANDLE, errorHandle);
        return;
    }

#ifdef __cplusplus
    /* Clear C++ std buffers */
    if (stdin_console)
        std::cin.clear();
    if (stdout_console)
        std::cout.clear();
    if (stderr_console)
        std::cerr.clear();

    /* Synchronise C streams with C++ iostream */
    std::ios::sync_with_stdio();
#endif

    /* Write two empty lines to prevent overwriting command prompt in cmd.exe */
    if (stdout_console) {
        fprintf(stdout, "\n\n");
        fflush(stdout);
    } else if (stderr_console) {
        fprintf(stderr, "\n\n");
        fflush(stderr);
    }

}
