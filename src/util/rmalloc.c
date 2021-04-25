#include "rmalloc.h"

/* the following are signed becasue malloc_usable_size might be greater than what being requested */
/* in RedisModule_Alloc for more see malloc_usable_size documentation                             */
__thread int64_t n_alloced; // amount of mem allocated for thread
int64_t mem_capacity;           // limit on maximal allocated mem for thread

// func pointers which stores original addresses of RedisModule functions
void * (*RedisModule_Alloc_Orig)(size_t bytes);
void * (*RedisModule_Realloc_Orig)(void *ptr, size_t bytes);
void (*RedisModule_Free_Orig)(void *ptr);
void * (*RedisModule_Calloc_Orig)(size_t nmemb, size_t size);
char * (*RedisModule_Strdup_Orig)(const char *str);

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
