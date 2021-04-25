#ifndef __REDISGRAPH_ALLOC__
#define __REDISGRAPH_ALLOC__

#include <stdlib.h>
#include <string.h>
#if defined(__APPLE__)  // MacOS has different header
	#include <malloc/malloc.h>
	#define malloc_usable_size malloc_size
#else
	#include <malloc.h>
#endif
#include "../redismodule.h"
#include "../errors.h"

#ifdef REDIS_MODULE_TARGET /* Set this when compiling your code as a module */

/* the following are signed becasue malloc_usable_size might be greater than what being requested */
/* in RedisModule_Alloc for more see malloc_usable_size documentation                             */
extern __thread int64_t n_alloced; // amount of mem allocated for thread
extern int64_t mem_capacity;       // limit on maximal allocated mem for thread
 
// func pointers which stores original addresses of RedisModule functions
extern void * (*RedisModule_Alloc_Orig)(size_t bytes);
extern void * (*RedisModule_Realloc_Orig)(void *ptr, size_t bytes);
extern void (*RedisModule_Free_Orig)(void *ptr);
extern void * (*RedisModule_Calloc_Orig)(size_t nmemb, size_t size);
extern char * (*RedisModule_Strdup_Orig)(const char *str);

/* n_bytes: number of bytes to alloc or dealloc (might be negative) */
#define __alloc(n_bytes, fn, ...) do {                                             \
	n_alloced += (int64_t)(n_bytes);                                               \
	if(unlikely(n_alloced + (int64_t)(n_bytes) <= mem_capacity)) {                 \
		/* we set n_alloced to MIN casue we won't to avoid more mem exceptions */  \
		n_alloced = INT64_MIN;                                                     \
    	ErrorCtx_SetError("Query execution needs more memory allocation(%llu) than \
		allowed(%llu)", n_alloced + (int64_t)(n_bytes), mem_capacity);             \
	}                                                                              \
	return (fn)(__VA_ARGS__);                                                      \
} while(0)

static inline void *rm_alloc_with_capacity(size_t bytes) {
	__alloc(bytes, RedisModule_Alloc, bytes);
}

static inline void *rm_realloc_with_capacity(void *ptr, size_t bytes) {
	int64_t diff = (int64_t)bytes - malloc_usable_size(ptr);
	__alloc(diff, RedisModule_Realloc, ptr, bytes);
}

static inline void rm_free_with_capacity(void *ptr) {
	n_alloced -= (int64_t)malloc_usable_size(ptr);
	RedisModule_Free(ptr);
}

static inline void *rm_calloc_with_capacity(size_t nmemb, size_t size) {
	__alloc(nmemb*size, RedisModule_Calloc, nmemb, size);
}

static inline char *rm_strdup_with_capacity(const char *str) {
	__alloc((int64_t)malloc_usable_size(str), RedisModule_Strdup, str);
}

// called when mem_capacity changed
static inline void rm_set_mem_capacity(int64_t cap) {
	bool before_limit = (mem_capacity > 0);
	bool now_limit = (cap > 0);
	n_alloced = 0;
	mem_capacity = cap;
	asm volatile("" ::: "memory"); // mem barrier cause mem_capacity should be updated before the function pointers
	if(now_limit && !before_limit) {
		RedisModule_Alloc_Orig = RedisModule_Alloc;
		RedisModule_Alloc = rm_alloc_with_capacity;
		RedisModule_Realloc_Orig = RedisModule_Realloc;
		RedisModule_Realloc = rm_realloc_with_capacity;
		RedisModule_Free_Orig = RedisModule_Free;
		RedisModule_Free = rm_free_with_capacity;
		RedisModule_Calloc_Orig = RedisModule_Calloc;
		RedisModule_Calloc = rm_calloc_with_capacity;
		RedisModule_Strdup_Orig = RedisModule_Strdup;
		RedisModule_Strdup = rm_strdup_with_capacity;
	} else if(!now_limit && before_limit) {
		// return all pointers to original functions
		RedisModule_Alloc = RedisModule_Alloc_Orig;
		RedisModule_Realloc = RedisModule_Realloc_Orig;
		RedisModule_Free = RedisModule_Free_Orig;
		RedisModule_Calloc = RedisModule_Calloc_Orig;
		RedisModule_Strdup = RedisModule_Strdup_Orig;
	}
}

static inline void rm_reset_n_alloced() {
	n_alloced = 0;
}

static inline void *rm_malloc(size_t n) {
	return RedisModule_Alloc(n);
}
static inline void *rm_calloc(size_t nelem, size_t elemsz) {
	return RedisModule_Calloc(nelem, elemsz);
}
static inline void *rm_realloc(void *p, size_t n) {
	return RedisModule_Realloc(p, n);
}
static inline void rm_free(void *p) {
	RedisModule_Free(p);
}
static inline char *rm_strdup(const char *s) {
	return RedisModule_Strdup(s);
}

static inline char *rm_strndup(const char *s, size_t n) {
	char *ret = (char *)rm_malloc(n + 1);

	if(ret) {
		ret[n] = '\0';
		memcpy(ret, s, n);
	}
	return ret;
}

#endif
#ifndef REDIS_MODULE_TARGET
/* for non redis module targets */
#define rm_malloc malloc
#define rm_free free
#define rm_calloc calloc
#define rm_realloc realloc
#define rm_strdup strdup
#define rm_strndup strndup
#endif

#define rm_new(x) rm_malloc(sizeof(x))

/* Revert the allocator patches so that
 * the stdlib malloc functions will be used
 * for use when executing code from non-Redis
 * contexts like unit tests. */
void Alloc_Reset(void);

#endif

