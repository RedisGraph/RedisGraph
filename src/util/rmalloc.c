#include "rmalloc.h"
#include "branch_pred.h" 

#if defined(__APPLE__)  // MacOS has different header
  #include <malloc/malloc.h>
	#define malloc_usable_size malloc_size
#else
	#include <malloc.h>
#endif

#include "../errors.h"

#ifdef REDIS_MODULE_TARGET /* Set this when compiling your code as a module */

/* the following are signed becasue malloc_usable_size might be greater than what being requested        */
/* in RedisModule_Alloc so we might end up with memory_count<0 or even a query might allocate a bit more */
/* memory than the limit and there will be no exception, so we only ensure that the allocation will      */
/* not exceed the limit by large amount.                                                                 */
/* For instance a query calls rm_alloc_with_capacity to allocate 1 byte                                  */
/* and RedisModule_Alloc might allocate 4 bytes so on free,  malloc_usable_size will return 4.           */
/* thus at the end of the free func n_alloced will be -3 (1-4), we tolerate this skew in the counter     */
/* because it's better performance wise, the alternative would be to call malloc_usable_size on alloc    */
/* and to inc by it's result n_alloced.                                                                  */
static __thread int64_t n_alloced; // amount of memory allocated for currently executed query (thread_local counter)
static int64_t mem_capacity;       // maximum memory consumption for thread
 
// function pointers which hold the original address of RedisModule_Alloc* functions
static void (*RedisModule_Free_Orig)(void *ptr);
static void * (*RedisModule_Alloc_Orig)(size_t bytes);
static char * (*RedisModule_Strdup_Orig)(const char *str);
static void * (*RedisModule_Realloc_Orig)(void *ptr, size_t bytes);
static void * (*RedisModule_Calloc_Orig)(size_t nmemb, size_t size);

void rm_reset_n_alloced() {
	n_alloced = 0;
}

/* n_bytes: number of bytes to alloc or dealloc (might be negative) */
static inline void _throw_on_memory_limit_exceeded(int64_t n_bytes) {
	n_alloced += n_bytes;
	if(unlikely(n_alloced > mem_capacity)) { // Check if capacity exceeded
		// set n_alloced to MIN casue we would like to avoid more mem exceptions
		n_alloced = INT64_MIN;
		
		// throw exception cause memory limit exceeded
    	ErrorCtx_SetError("Querie's mem consumption exceeded capacity");
	}
	return;     
}

void *rm_alloc_with_capacity(size_t bytes) {
	_throw_on_memory_limit_exceeded(bytes);
	return RedisModule_Alloc_Orig(bytes);
}

void *rm_realloc_with_capacity(void *ptr, size_t bytes) {
	int64_t diff = (int64_t)bytes - (int64_t)malloc_usable_size(ptr);
	_throw_on_memory_limit_exceeded(diff);
	return RedisModule_Realloc_Orig(ptr, bytes);
}

void rm_free_with_capacity(void *ptr) {
	n_alloced -= (int64_t)malloc_usable_size(ptr);
	return RedisModule_Free_Orig(ptr);
}

void *rm_calloc_with_capacity(size_t nmemb, size_t size) {
	_throw_on_memory_limit_exceeded((int64_t)nmemb*size);
	return RedisModule_Calloc_Orig(nmemb, size);
}

char *rm_strdup_with_capacity(const char *str) {
	char *str_copy = RedisModule_Strdup_Orig(str);
	// It's ok to call _throw_on_memory_limit_exceeded after the allocation because query is single threaded.
	_throw_on_memory_limit_exceeded((int64_t)malloc_usable_size(str_copy));
	return str_copy;
}

void rm_set_mem_capacity(int64_t cap) {
	bool is_capped = (mem_capacity > 0); // current allocator enforces memory capacity
	bool should_cap = (cap > 0); // should we use a memory capped allocator
	
	/* The local capacity be setted before changing the function pointers  */
	/* for instance if we switched to capped function we like the cap      */
	/* to be valid when they will be called.                               */
	mem_capacity = cap; 
	if(should_cap && !is_capped) {
		// store the function pointer's original values and change them to the capped version
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
	} else if(!should_cap && is_capped) {
		// restore all function pointers to their original values
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
