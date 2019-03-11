//------------------------------------------------------------------------------
// GB_cast_array: typecast an array
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Casts an input array A to an output array C with a different built-in type.
// Does not handle user-defined types.

// PARALLEL: easy.  May want to put the workers in functions, like
// Generated/GB_AxB*, instead of in a macro.

#include "GB.h"

void GB_cast_array              // typecast an array
(
    void *C,                    // output array
    const GB_Type_code code1,   // type code for C
    const void *A,              // input array
    const GB_Type_code code2,   // type code for A
    const int64_t n,            // number of entries in C and A
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (n == 0)
    { 
        // no work to do, and the A and C pointer may be NULL as well
        return ;
    }

    ASSERT (C != NULL) ;
    ASSERT (A != NULL) ;
    ASSERT (n > 0) ;
    ASSERT (code1 <= GB_FP64_code) ;
    ASSERT (code2 <= GB_FP64_code) ;
    ASSERT (GB_code_compatible (code1, code2)) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS (nthreads, Context) ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_WORKER(ctype,atype)          \
    {                                       \
        ctype *c = (ctype *) C ;            \
        atype *a = (atype *) A ;            \
        for (int64_t k = 0 ; k < n ; k++)   \
        {                                   \
            /* c [k] = a [k] ; */           \
            GB_CAST (c [k], a [k]) ;        \
        }                                   \
    }                                       \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    // There is no generic worker so the switch factory cannot be disabled.
    #include "GB_2type_template.c"
}

