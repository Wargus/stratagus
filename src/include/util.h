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
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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

#ifndef __UTIL_H__
#define __UTIL_H__

#include "SDL.h"


/*----------------------------------------------------------------------------
--  Random
----------------------------------------------------------------------------*/

extern unsigned SyncRandSeed;               /// Sync random seed value

extern void InitSyncRand(void);             /// Initialize the syncron rand
extern int SyncRand(void);                  /// Syncron rand

/*----------------------------------------------------------------------------
--  Math
----------------------------------------------------------------------------*/

	///  rand only used on this computer.
#define MyRand() rand()

	/// Compute a square root using ints
extern long isqrt(long num);

/*----------------------------------------------------------------------------
--  Strings
----------------------------------------------------------------------------*/

	/// strdup + strcat
extern char* strdcat(const char* l, const char* r);
	/// strdup + strcat + strcat
extern char* strdcat3(const char* l, const char* m, const char* r);

#if !HAVE_STRCASESTR
	/// case insensitive strstr
extern char* strcasestr(const char* str, const char* substr);
#endif // !HAVE_STRCASESTR

/*----------------------------------------------------------------------------
--  Hash
----------------------------------------------------------------------------*/

// Basic Defines - Used by Hash
#define NELEM(x)    ((int)(sizeof(x)/sizeof(*(x))))

// Begin hash specific functions
struct hash_st
{
	int nelem;
	int hashsize;
	int maxdepth;
	int middepth;
};

extern void* _hash_get(const Uint8* id, void* table, int size, int usize);
extern const void* _hash_find(const Uint8* id, const void* table, int size, int usize);
extern void  _hash_del(const Uint8* id, void* table, int size, int usize);
extern void  _hash_stat(void* table, int size, struct hash_st* stat_buffer);

#ifdef __GNUC__  // { GNU feature

#define hash_get(tab, id)  (typeof((tab).table[0]->user)*) \
	_hash_get(id, (tab).table, NELEM((tab).table), sizeof((tab).table[0]->user))

#define hash_find(tab, id)  (typeof((tab).table[0]->user)*) \
	_hash_find(id,(tab).table, NELEM((tab).table), sizeof((tab).table[0]->user))

#else // }{ GNU feature

#define hash_get(tab, id)  _hash_get(id, (tab).table, NELEM((tab).table), sizeof((tab).table[0]->user))

#define hash_find(tab, id)  _hash_find(id,(tab).table, NELEM((tab).table), sizeof((tab).table[0]->user))

#endif // } !GNU feature

#define hash_del(tab, id) \
	_hash_del(id, (tab).table, NELEM((tab).table), sizeof((tab).table[0]->user))

#define hash_name(tab, sym) (((Uint8*)sym) + sizeof((tab).table[0]->user) + 1)

#define hash_stat(tab, st) _hash_stat((tab).table, NELEM((tab).table), st)

#define hash_add(tab, id) hash_get(tab, id)

#define hashtable(type, size) struct \
{ \
	struct { \
		void* left; \
		void* right; \
		type user; \
		Uint8 name[2]; \
	} *table[size]; \
}

#endif /* __UTIL_H__ */
