
#ifndef PMR_MALLOC_H
#define PMR_MALLOC_H
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
#endif
{
    void *rmm_malloc (size_t size) ;
    void  rmm_free (void *) ;
    void *rmm_calloc (size_t, size_t) ;
    void *rmm_realloc (void *, size_t) ;
}

#endif
