#ifndef ETLIB_GENERIC_H
#define ETLIB_GENERIC_H

#define NELEM(x) ((int)(sizeof(x)/sizeof(*(x))))
#define NORETURN(x) void x __attribute__((__noreturn__))
#define DEFINE(x) typeof(x) x
#define OFFSET_OF(type, elem) ((u8 *)&((type *)0)->elem - (u8 *)0)
#define BASE_OF(type, elem, p) ((type *)((u8 *)(p) - OFFSET_OF(type, elem)))

#define not !
#define elif else if
#define Case break; case
#define Default break; default
#define streq(a, b) (strcmp((a), (b)) == 0)

#ifdef _MSC_VER
#ifndef inline
#define inline _inline
#endif
#endif

#ifndef min
#ifdef __GNUC__
#define min(a,b) ({ typeof(a) _a = a; typeof(b) _b = b; _a < _b ? _a : _b; })
#else
#define min min
static inline min(int a,int b) { return a<b ? a : b; }
#endif
#endif

#ifndef max
#ifdef __GNUC__
#define max(a,b) ({ typeof(a) _a = a; typeof(b) _b = b; _a > _b ? _a : _b; })
#else
#define max max
static inline max(int a,int b) { return a>b ? a : b; }
#endif
#endif

#ifdef __GNUC__
#define bound(a,b,c) ({ typeof(a) _a = a; typeof(b) _b = b; typeof(c) _c = c; \
			_b < _a ? _a : _b > _c ? _c : _b; })
#else
//#warning "// FIXME: bound not written!"
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef signed char s8;
typedef signed short s16;
typedef signed long s32;

#endif /* ETLIB_GENERIC_H */
