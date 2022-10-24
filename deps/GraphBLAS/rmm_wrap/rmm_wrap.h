//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#ifndef RMM_WRAP_H
#define RMM_WRAP_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO describe the modes
typedef enum { rmm_wrap_host=0, rmm_wrap_host_pinned=1, rmm_wrap_device=2, rmm_wrap_managed=3 } RMM_MODE ;

void rmm_wrap_finalize (void) ;
int rmm_wrap_initialize (RMM_MODE mode, size_t init_pool_size, size_t max_pool_size) ;

// example usage:
    //  rmm_wrap_initialize (rmm_wrap_managed, INT32_MAX, INT64_MAX) ;
    //  GxB_init (GrB_NONBLOCKING, rmm_wrap_malloc, rmm_wrap_calloc, rmm_wrap_realloc, rmm_wrap_free) ;
    //  use GraphBLAS ...
    //  GrB_finalize ( ) ;
    //  rmm_wrap_finalize ( ) ;

// The two PMR-based allocate/deallocate signatures (C-style):
void *rmm_wrap_allocate (size_t *size) ;
void  rmm_wrap_deallocate (void *p, size_t size) ;

// The four malloc/calloc/realloc/free signatures:
void *rmm_wrap_malloc (size_t size) ;
void *rmm_wrap_calloc (size_t n, size_t size) ;
void *rmm_wrap_realloc (void *p, size_t newsize) ;
void  rmm_wrap_free (void *p) ;

#ifdef __cplusplus
}
#endif
#endif

