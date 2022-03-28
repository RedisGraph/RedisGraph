//------------------------------------------------------------------------------
// GxB_Global_Option_set: set a global default option for all future matrices
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Global_Option_set      // set a global default option
(
    GxB_Option_Field field,         // option to change
    ...                             // value to change it to
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Global_Option_set (field, value)") ;

    //--------------------------------------------------------------------------
    // set the global option
    //--------------------------------------------------------------------------

    va_list ap ;

    switch (field)
    {

        //----------------------------------------------------------------------
        // matrix format
        //----------------------------------------------------------------------

        case GxB_HYPER_SWITCH : 

            {
                va_start (ap, field) ;
                double hyper_switch = va_arg (ap, double) ;
                va_end (ap) ;
                GB_Global_hyper_switch_set ((float) hyper_switch) ;
            }
            break ;

        case GxB_BITMAP_SWITCH : 

            {
                va_start (ap, field) ;
                double *bitmap_switch = va_arg (ap, double *) ;
                va_end (ap) ;
                if (bitmap_switch == NULL)
                { 
                    // set all switches to their default
                    GB_Global_bitmap_switch_default ( ) ;
                }
                else
                {
                    for (int k = 0 ; k < GxB_NBITMAP_SWITCH ; k++)
                    { 
                        float b = (float) (bitmap_switch [k]) ;
                        GB_Global_bitmap_switch_set (k, b) ;
                    }
                }
            }
            break ;

        case GxB_FORMAT : 

            {
                va_start (ap, field) ;
                int format = va_arg (ap, int) ;
                va_end (ap) ;
                if (! (format == GxB_BY_ROW || format == GxB_BY_COL))
                { 
                    return (GrB_INVALID_VALUE) ;
                }
                GB_Global_is_csc_set (format != (int) GxB_BY_ROW) ; 
            }
            break ;

        //----------------------------------------------------------------------
        // OpenMP control
        //----------------------------------------------------------------------

        case GxB_GLOBAL_NTHREADS :      // same as GxB_NTHREADS

            {
                va_start (ap, field) ;
                int nthreads_max_new = va_arg (ap, int) ;
                va_end (ap) ;
                // if < 1, then treat it as if nthreads_max = 1
                nthreads_max_new = GB_IMAX (1, nthreads_max_new) ;
                GB_Global_nthreads_max_set (nthreads_max_new) ;
            }
            break ;

        case GxB_GLOBAL_CHUNK :         // same as GxB_CHUNK

            {
                va_start (ap, field) ;
                double chunk = va_arg (ap, double) ;
                va_end (ap) ;
                GB_Global_chunk_set (chunk) ;
            }
            break ;

        //----------------------------------------------------------------------
        // memory pool control
        //----------------------------------------------------------------------

        case GxB_MEMORY_POOL : 

            {
                va_start (ap, field) ;
                int64_t *free_pool_limit = va_arg (ap, int64_t *) ;
                va_end (ap) ;
                if (free_pool_limit == NULL)
                { 
                    // set all limits to their default
                    GB_Global_free_pool_init (false) ;
                }
                else
                {
                    for (int k = 3 ; k < 64 ; k++)
                    { 
                        GB_Global_free_pool_limit_set (k, free_pool_limit [k]) ;
                    }
                }
            }
            break ;

        //----------------------------------------------------------------------
        // diagnostics
        //----------------------------------------------------------------------

        case GxB_BURBLE : 

            {
                va_start (ap, field) ;
                int burble = va_arg (ap, int) ;
                va_end (ap) ;
                GB_Global_burble_set ((bool) burble) ;
            }
            break ;

        case GxB_PRINTF : 

            {
                va_start (ap, field) ;
                void *printf_func = va_arg (ap, void *) ;
                va_end (ap) ;
                GB_Global_printf_set ((GB_printf_function_t) printf_func) ;
            }
            break ;

        case GxB_FLUSH : 

            {
                va_start (ap, field) ;
                void *flush_func = va_arg (ap, void *) ;
                va_end (ap) ;
                GB_Global_flush_set ((GB_flush_function_t) flush_func) ;
            }
            break ;

        case GxB_PRINT_1BASED : 

            {
                va_start (ap, field) ;
                int onebased = va_arg (ap, int) ;
                va_end (ap) ;
                GB_Global_print_one_based_set ((bool) onebased) ;
            }
            break ;

        //----------------------------------------------------------------------
        // CUDA (DRAFT: in progress, do not use)
        //----------------------------------------------------------------------

        case GxB_GLOBAL_GPU_CONTROL :       // same as GxB_GPU_CONTROL

            {
                va_start (ap, field) ;
                GrB_Desc_Value gpu_control = (GrB_Desc_Value) va_arg (ap, int) ;
                va_end (ap) ;
                GB_Global_gpu_control_set (gpu_control) ;
            }
            break ;

        case GxB_GLOBAL_GPU_CHUNK :         // same as GxB_GPU_CHUNK

            {
                va_start (ap, field) ;
                double gpu_chunk = va_arg (ap, double) ;
                va_end (ap) ;
                GB_Global_gpu_chunk_set (gpu_chunk) ;
            }
            break ;

        default : 

            return (GrB_INVALID_VALUE) ;
    }

    return (GrB_SUCCESS) ;
}

