#ifndef UTIL_H
#define UTIL_H

#include "SDL.h"

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

#endif /* UTIL_H */
