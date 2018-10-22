#include "rmalloc.h"

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
