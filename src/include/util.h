//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name util.h - General utilities. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

#ifndef __UTIL_H__
#define __UTIL_H__

//@{

/*----------------------------------------------------------------------------
--  Threads
----------------------------------------------------------------------------*/

#ifndef __unix
#undef NOUSER
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#include <winsock2.h>
#include <windows.h>
#elif defined(__hpux)
#include <sys/mpctl.h>
#ifndef MPC_GETNUMSPUS_SYS
#define MPC_GETNUMSPUS_SYS MPC_GETNUMSPUS
#endif
#else
#include <unistd.h>
#include <pthread.h>
#endif

inline int get_cpu_count()
{
#ifndef __unix
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;
#elif defined(__linux) || defined (__sun)
	return sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(__hpux)
	return mpctl(MPC_GETNUMSPUS_SYS, 0, 0);
#else
	return 1;
#endif
}


class CMutex
{
	CMutex(const CMutex &); // prohibited
	CMutex &operator= (const CMutex &); // prohibited

#if !defined (__unix)
	CRITICAL_SECTION _mut;
#else
	pthread_mutexattr_t _attr;
	pthread_mutex_t _mut;
#endif
public:
	CMutex();
	~CMutex();

	void Lock() {
#if !defined (__unix)
		EnterCriticalSection(&_mut);
#else
		pthread_mutex_lock(&_mut);
#endif
	}

	void UnLock() {
#if !defined (__unix)
		LeaveCriticalSection(&_mut);
#else
		pthread_mutex_unlock(&_mut);
#endif
	}

	bool TryLock() {
#if !defined (__unix)
		return 0 != TryEnterCriticalSection(&_mut);
#else
		return 0 == pthread_mutex_trylock(&_mut);
#endif
	}
};


class CThread
{
	CThread(const CThread &); // prohibited
	CThread &operator= (const CThread &); // prohibited

	bool m_bRunning;
	int m_dExitFlag;

#if !defined (__unix)
	unsigned long m_dThreadID;
	static unsigned long WINAPI threadFun(void *pThread);
	HANDLE m_hndlThread;
#else
	pthread_t m_dThreadID;
	static void *threadFun(void *pThread);
#endif

public:
	CThread() : m_bRunning(false), m_dExitFlag(0) {};
	virtual ~CThread();

	virtual void Run() = 0;
	int Start();
	int Wait();
	void Exit();
	void Terminate();

	bool IsRunning() const { return m_bRunning; }

	void SetExitFlag(int dExitCode = 1) { m_dExitFlag = dExitCode; }

	int GetExitFlag() const { return m_dExitFlag; }

#if !defined (__unix)
	static unsigned long GetThreadID() { return GetCurrentThreadId(); }
#else
	static pthread_t GetThreadID() { return pthread_self(); }
#endif

};



/*----------------------------------------------------------------------------
--  Random
----------------------------------------------------------------------------*/

extern unsigned SyncRandSeed;               /// Sync random seed value

extern void InitSyncRand();             /// Initialize the syncron rand
extern int SyncRand();                  /// Syncron rand
extern int SyncRand(int max);               /// Syncron rand

/*----------------------------------------------------------------------------
--  Math
----------------------------------------------------------------------------*/

///  rand only used on this computer.
#define MyRand() rand()

//for 32 bit signed int
extern inline int MyAbs(int x) { return (x ^ (x >> 31)) - (x >> 31); }

/// Compute a square root using ints
extern long isqrt(long num);


template <typename T>
void clamp(T *value, T minValue, T maxValue)
{
	Assert(minValue <= maxValue);

	if (*value < minValue) {
		*value = minValue;
	} else if (maxValue < *value) {
		*value = maxValue;
	}
}


/*----------------------------------------------------------------------------
--  Strings
----------------------------------------------------------------------------*/

#if !defined(_MSC_VER) || _MSC_VER < 1400
#define _TRUNCATE ((size_t)-1)
extern unsigned int strcpy_s(char *dst, size_t dstsize, const char *src);
extern unsigned int strncpy_s(char *dst, size_t dstsize, const char *src, size_t count);
extern unsigned int strcat_s(char *dst, size_t dstsize, const char *src);
#endif

#ifndef HAVE_STRCASESTR
/// case insensitive strstr
extern char *strcasestr(const char *str, const char *substr);
#endif // !HAVE_STRCASESTR

#ifndef HAVE_STRNLEN
/// determine length of a fixed-length string
extern size_t strnlen(const char *str, size_t strsize);
#endif // !HAVE_STRNLEN

/*----------------------------------------------------------------------------
--  Clipboard
----------------------------------------------------------------------------*/

#include <string>

int GetClipboard(std::string &str);

/*----------------------------------------------------------------------------
--  UTF8
----------------------------------------------------------------------------*/

int UTF8GetNext(const std::string &text, int curpos);
int UTF8GetPrev(const std::string &text, int curpos);

//@}

#endif /* __UTIL_H__ */
