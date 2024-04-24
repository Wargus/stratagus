#ifndef ST_BACKTRACE_H
#define ST_BACKTRACE_H 1

#include <stdio.h>

#ifdef __GLIBC__

#include <execinfo.h>
inline void print_backtrace(int sz = 100) {
    int nptrs;
    void *buffer[sz];
    nptrs = backtrace(buffer, sz);
    fprintf(stderr, "backtrace() returned %d addresses\n", nptrs);
    backtrace_symbols_fd(buffer, sz, 2);
}

#elif defined(USE_WIN32)

#include <windows.h>
#include <winbase.h>
#include <dbghelp.h>
#include <process.h>

#undef DELETE

inline void print_backtrace(int sz = 100) {
    void *stack[100];

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, TRUE);
    unsigned short frames = CaptureStackBackTrace(0, sz, stack, nullptr);
    fprintf(stderr, "backtrace returned %d addresses\n", frames);
    SYMBOL_INFO *symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 1024 * sizeof(char), 1);
    symbol->MaxNameLen = 1024;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    IMAGEHLP_LINE64 *line = (IMAGEHLP_LINE64 *)malloc(sizeof(IMAGEHLP_LINE64));
    line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    for(unsigned int i = 0; i < frames; i++) {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
        const char* name = symbol->Name ? symbol->Name : "<unknown frame>";
        DWORD displacement;
        if (SymGetLineFromAddr64(process, (DWORD64)(stack[i]), &displacement, line)) {
            fprintf(stderr, "%d: %s in %s:%d 0x%llx\n", frames - i - 1, name, line->FileName, line->LineNumber, symbol->Address);
        } else {
            fprintf(stderr, "%d: %s 0x%llx\n", frames - i - 1, name, symbol->Address);
        }
    }
    free(symbol);
}

#else

inline void print_backtrace(int sz = 100) {
}

#endif

#endif
