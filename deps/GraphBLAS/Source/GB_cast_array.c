//------------------------------------------------------------------------------
// GB_cast_array: typecast or copy an array
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Casts an input array Ax to an output array Cx with a different built-in
// type.  Does not handle user-defined types.

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_unop__include.h"
#endif

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_cast_array              // typecast an array
(
    GB_void *Cx,                // output array
    const GB_Type_code code1,   // type code for Cx
    GB_void *Ax,                // input array
    const GB_Type_code code2,   // type code for Ax
    const int8_t *GB_RESTRICT Ab,   // bitmap for Ax
    const size_t user_size,     // size of Ax and Cx if user-defined
    const int64_t anz,          // number of entries in Cx and Ax
    const int nthreads          // number of threads to use
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (anz == 0 || Cx == Ax)
    { 
        // if anz is zero: no work to do, and the Ax and Cx pointer may be NULL
        // as well.  If Cx and Ax are aliased, then no copy is needed.
        return ;
    }

    ASSERT (Cx != NULL) ;
    ASSERT (Ax != NULL) ;
    ASSERT (anz > 0) ;
    ASSERT (GB_code_compatible (code1, code2)) ;

    //--------------------------------------------------------------------------
    // typecast the array
    //--------------------------------------------------------------------------

    #ifndef GBCOMPACT

        //----------------------------------------------------------------------
        // define the worker for the switch factory
        //----------------------------------------------------------------------

        #define GB_unop_apply(zname,xname)                          \
            GB_unop_apply__identity ## zname ## xname

        #define GB_WORKER(ignore1,zname,ztype,xname,xtype)          \
        {                                                           \
            GrB_Info info = GB_unop_apply (zname,xname)             \
                ((ztype *) Cx, (xtype *) Ax, Ab, anz, nthreads) ;   \
            if (info == GrB_SUCCESS) return ;                       \
        }                                                           \
        break ;

        //----------------------------------------------------------------------
        // launch the switch factory
        //----------------------------------------------------------------------

        #include "GB_2type_factory.c"

    #endif

    //--------------------------------------------------------------------------
    // generic worker
    //--------------------------------------------------------------------------

    int64_t csize = GB_code_size (code1, user_size) ;
    int64_t asize = GB_code_size (code2, user_size) ;
    GB_cast_function cast_A_to_C = GB_cast_factory (code1, code2) ;

    int64_t p ;
    #pragma omp parallel for num_threads(nthreads) schedule(static)
    for (p = 0 ; p < anz ; p++)
    { 
        if (!GBB (Ab, p)) continue ;
        // Cx [p] = Ax [p]
        cast_A_to_C (Cx +(p*csize), Ax +(p*asize), asize) ;
    }
}

