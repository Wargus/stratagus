//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name rdtsc.h - The rdtsc header file. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer
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
//      $Id$

#ifndef __RDTSC_H__
#define __RDTSC_H__

//@{

#ifndef _MSC_VER
#include <sys/types.h>      // for u_int64_t
#endif

#if defined(__MINGW32__)
typedef unsigned long long u_int64_t;
#endif

#if !defined(i386) && !defined(__i386__) && !defined(__alpha__)
#error "Need an x86 or alpha processor for this"
#endif

/**
**  Read the tsc (time-stamp counter) register of the processor.
**
**  @return the rtc register
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

#endif  // !__RDTSC_H__
