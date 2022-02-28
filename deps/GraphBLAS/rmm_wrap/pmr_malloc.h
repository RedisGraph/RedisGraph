
#ifndef PMR_MALLOC_H
#define PMR_MALLOC_H
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
    void *rmm_wrap_malloc (size_t size) ;
    void  rmm_wrap_free (void *) ;
    void *rmm_wrap_calloc (size_t, size_t) ;
    void *rmm_wrap_realloc (void *, size_t) ;

#ifdef __cplusplus
}
#endif

#endif
