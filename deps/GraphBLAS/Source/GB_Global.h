//------------------------------------------------------------------------------
// GB_Global.h: definitions for global variables
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// These defintions are not visible to the user.  They are used only inside
// GraphBLAS itself.  Note that the GB_Global struct does not appear here.
// It is accessible only by the functions in GB_Global.c.

#ifndef GB_GLOBAL_H
#define GB_GLOBAL_H

GB_PUBLIC void     GB_Global_mode_set (GrB_Mode mode) ;
          GrB_Mode GB_Global_mode_get (void) ;

          void     GB_Global_sort_set (int sort) ;
          int      GB_Global_sort_get (void) ;

GB_PUBLIC void     GB_Global_GrB_init_called_set (bool GrB_init_called) ;
GB_PUBLIC bool     GB_Global_GrB_init_called_get (void) ;

GB_PUBLIC void     GB_Global_nthreads_max_set (int nthreads_max) ;
GB_PUBLIC int      GB_Global_nthreads_max_get (void) ;

GB_PUBLIC int      GB_Global_omp_get_max_threads (void) ;

GB_PUBLIC void     GB_Global_chunk_set (double chunk) ;
GB_PUBLIC double   GB_Global_chunk_get (void) ;

GB_PUBLIC void     GB_Global_hyper_switch_set (float hyper_switch) ;
GB_PUBLIC float    GB_Global_hyper_switch_get (void) ;

GB_PUBLIC void     GB_Global_bitmap_switch_set (int k, float b) ;
GB_PUBLIC float    GB_Global_bitmap_switch_get (int k) ;
GB_PUBLIC float    GB_Global_bitmap_switch_matrix_get
                        (int64_t vlen, int64_t vdim) ;
GB_PUBLIC void     GB_Global_bitmap_switch_default (void) ;

          void     GB_Global_is_csc_set (bool is_csc) ;
          bool     GB_Global_is_csc_get (void) ;

GB_PUBLIC void     GB_Global_abort_function_set
                        (void (* abort_function) (void)) ;
GB_PUBLIC void     GB_Global_abort_function (void) ;

          void     GB_Global_malloc_function_set
                        (void * (* malloc_function) (size_t)) ;
          void  *  GB_Global_malloc_function (size_t size) ;
          void     GB_Global_calloc_function_set
                        (void * (* calloc_function) (size_t, size_t)) ;
          void  *  GB_Global_calloc_function (size_t count, size_t size) ;
          void     GB_Global_realloc_function_set
                        (void * (* realloc_function) (void *, size_t)) ;
          void  *  GB_Global_realloc_function (void *p, size_t size) ;
          bool     GB_Global_have_realloc_function (void) ;
          void     GB_Global_free_function_set
                        (void (* free_function) (void *)) ;
          void     GB_Global_free_function (void *p) ;

GB_PUBLIC void     GB_Global_malloc_is_thread_safe_set
                        (bool malloc_is_thread_safe) ;
GB_PUBLIC bool     GB_Global_malloc_is_thread_safe_get (void) ;

GB_PUBLIC void     GB_Global_malloc_tracking_set (bool malloc_tracking) ;
          bool     GB_Global_malloc_tracking_get (void) ;

          void     GB_Global_nmalloc_clear (void) ;
GB_PUBLIC int64_t  GB_Global_nmalloc_get (void) ;
          void     GB_Global_nmalloc_increment (void) ;
GB_PUBLIC void     GB_Global_nmalloc_decrement (void) ;

GB_PUBLIC void     GB_Global_malloc_debug_set (bool malloc_debug) ;
GB_PUBLIC bool     GB_Global_malloc_debug_get (void) ;

GB_PUBLIC void     GB_Global_malloc_debug_count_set
                        (int64_t malloc_debug_count) ;
          bool     GB_Global_malloc_debug_count_decrement (void) ;

GB_PUBLIC void     GB_Global_hack_set (int64_t hack) ;
GB_PUBLIC int64_t  GB_Global_hack_get (void) ;

          void     GB_Global_burble_set (bool burble) ;
GB_PUBLIC bool     GB_Global_burble_get (void) ;

GB_PUBLIC void     GB_Global_print_one_based_set (bool onebased) ;
GB_PUBLIC bool     GB_Global_print_one_based_get (void) ;

          void     GB_Global_gpu_control_set (GrB_Desc_Value value) ;
          GrB_Desc_Value GB_Global_gpu_control_get (void);
          void     GB_Global_gpu_chunk_set (double gpu_chunk) ;
          double   GB_Global_gpu_chunk_get (void) ;
          bool     GB_Global_gpu_count_set (bool enable_cuda) ;
          int      GB_Global_gpu_count_get (void) ;
          size_t   GB_Global_gpu_memorysize_get (int device) ;
          int      GB_Global_gpu_sm_get (int device) ;
          bool     GB_Global_gpu_device_pool_size_set
                        (int device, size_t size) ;
          bool     GB_Global_gpu_device_max_pool_size_set
                        (int device, size_t size) ;
          bool     GB_Global_gpu_device_memory_resource_set
                        (int device, void *resource) ;
          void*    GB_Global_gpu_device_memory_resource_get
                        (int device) ;
          bool     GB_Global_gpu_device_properties_get (int device) ;

GB_PUBLIC void     GB_Global_timing_clear_all (void) ;
GB_PUBLIC void     GB_Global_timing_clear (int k) ;
GB_PUBLIC void     GB_Global_timing_set (int k, double t) ;
GB_PUBLIC void     GB_Global_timing_add (int k, double t) ;
GB_PUBLIC double   GB_Global_timing_get (int k) ;

// #include "GB_Global_mkl_template.h"
#endif

