#ifndef ETLIB_XMALLOC_H
#define ETLIB_XMALLOC_H
#include <sys/types.h>

extern void *xmalloc(size_t size);
extern void *xcalloc(size_t nelem, size_t elem_size);
extern void *xrealloc(void *ptr, size_t new_size);
extern void *xfree(void *ptr);
extern void *xstrdup(void *str);

#endif /* ETLIB_XMALLOC_H */
