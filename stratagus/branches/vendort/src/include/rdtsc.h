/*
**	A clone of a famous game.
*/
/**@name rdtsc.h	-	The rdtsc header file. */
/*
**	(c) Copyright 1999 by Lutz Sammer
**
**	$Id$
*/

#ifndef __RDTSC_H__
#define __RDTSC_H__

//@{

#include <sys/types.h>			// for u_int64_t
#if defined(__MINGW32__)
typedef unsigned long long u_int64_t;
#endif

#if !defined(i386) && !defined(__i386__) && !defined(__alpha__)
#error "Need an x86 or alpha processor for this"
#endif

/**
**      Read the tsc (time-stamp counter) register of the processor.
**
**      @return the rtc register
*/
static __inline u_int64_t rdtsc(void)
{
#ifdef __i386__
    u_int64_t rv;

    __asm __volatile(".byte 0x0f, 0x31" : "=A" (rv));
#endif
#ifdef __alpha__
    int rv;

    __asm __volatile("rpcc %0" : "=r" (rv));
#endif

    return rv;
}

//@}

#endif	// !__RDTSC_H__
