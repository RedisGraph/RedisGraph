#include "rmalloc.h"

#if defined(__APPLE__)  // MacOS has different header
  #include <malloc/malloc.h>
	#define malloc_usable_size malloc_size
#else
	#include <malloc.h>
#endif
#include "../errors.h"

#ifdef REDIS_MODULE_TARGET /* Set this when compiling your code as a module */

/* the following are signed becasue malloc_usable_size might be greater than what being requested */
/* in RedisModule_Alloc for more see malloc_usable_size documentation                             */
__thread int64_t n_alloced;        // amount of mem allocated for thread
static int64_t mem_capacity;       // limit on maximal allocated mem for thread
 
// func pointers which stores original addresses of RedisModule functions
static void * (*RedisModule_Alloc_Orig)(size_t bytes);
static void * (*RedisModule_Realloc_Orig)(void *ptr, size_t bytes);
static void (*RedisModule_Free_Orig)(void *ptr);
static void * (*RedisModule_Calloc_Orig)(size_t nmemb, size_t size);
static char * (*RedisModule_Strdup_Orig)(const char *str);

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

void *rm_alloc_with_capacity(size_t bytes) {
	__alloc(bytes, RedisModule_Alloc, bytes);
}

void *rm_realloc_with_capacity(void *ptr, size_t bytes) {
	int64_t diff = (int64_t)bytes - malloc_usable_size(ptr);
	__alloc(diff, RedisModule_Realloc, ptr, bytes);
}

void rm_free_with_capacity(void *ptr) {
	n_alloced -= (int64_t)malloc_usable_size(ptr);
	RedisModule_Free(ptr);
}

void *rm_calloc_with_capacity(size_t nmemb, size_t size) {
	__alloc(nmemb*size, RedisModule_Calloc, nmemb, size);
}

char *rm_strdup_with_capacity(const char *str) {
	__alloc((int64_t)malloc_usable_size(str), RedisModule_Strdup, str);
}

// called when mem_capacity changed
void rm_set_mem_capacity(int64_t cap) {
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

#endif

/* Redefine the allocator functions to use the malloc family.
 * Only to be used when running module code from a non-Redis
 * context, such as unit tests. */
void Alloc_Reset() {
  RedisModule_Alloc = malloc;
  RedisModule_Realloc = realloc;
  RedisModule_Calloc = calloc;
  RedisModule_Free = free;
  RedisModule_Strdup = strdup;
}
