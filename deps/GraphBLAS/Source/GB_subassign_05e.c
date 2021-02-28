//------------------------------------------------------------------------------
// GB_subassign_05e: C(:,:)<M,struct> = scalar ; no S, C empty, M structural
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Method 05e: C(:,:)<M,struct> = scalar ; no S
// compare with Methods 21, 25, and 05d

// M:           present
// Mask_comp:   false
// Mask_struct: true
// C_replace:   false
// accum:       NULL
// A:           scalar
// S:           none

#include "GB_subassign_methods.h"

#undef  GB_FREE_ALL
#define GB_FREE_ALL

GrB_Info GB_subassign_05e
(
    GrB_Matrix C,
    // input:
    const GrB_Matrix M,
    const void *scalar,
    const GrB_Type atype,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (C, "C for subassign method_05e", GB0) ;
    ASSERT_MATRIX_OK (M, "M for subassign method_05e", GB0) ;
    ASSERT (GB_NNZ (C) == 0) ;
    ASSERT (!GB_PENDING (C)) ; ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
    const GB_Type_code ccode = C->type->code ;
    const size_t csize = C->type->size ;
    GB_GET_SCALAR ;

    int64_t mnz = GB_NNZ (M) ;

    //--------------------------------------------------------------------------
    // Method 05e: C(:,:)<M> = x ; C is empty, x is a scalar, M is structural
    //--------------------------------------------------------------------------

    // Time: Optimal:  the method must iterate over all entries in M,
    // and the time is O(nnz(M)).  This is also the size of C.

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (mnz, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // allocate C and create its pattern
    //--------------------------------------------------------------------------

    // clear prior content and then create a copy of the pattern of M.  Keep
    // the same type and CSR/CSC for C.  Allocate the values of C but do not
    // initialize them.

    bool C_is_csc = C->is_csc ;
    GB_PHIX_FREE (C) ;
    GB_OK (GB_dup2 (&C, M, false, C->type, Context)) ;
    C->is_csc = C_is_csc ;
    int64_t pC ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    // worker for built-in types
    #define GB_WORKER(ctype)                                                \
    {                                                                       \
        ctype *GB_RESTRICT Cx = C->x ;                                      \
        ctype x = (*(ctype *) cwork) ;                                      \
        GB_PRAGMA (omp parallel for num_threads(nthreads) schedule(static)) \
        for (pC = 0 ; pC < mnz ; pC++)                                      \
        {                                                                   \
            Cx [pC] = x ;                                                   \
        }                                                                   \
    }                                                                       \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    switch (C->type->code)
    {
        case GB_BOOL_code   : GB_WORKER (bool) ;
        case GB_INT8_code   : GB_WORKER (int8_t) ;
        case GB_INT16_code  : GB_WORKER (int16_t) ;
        case GB_INT32_code  : GB_WORKER (int32_t) ;
        case GB_INT64_code  : GB_WORKER (int64_t) ;
        case GB_UINT8_code  : GB_WORKER (uint8_t) ;
        case GB_UINT16_code : GB_WORKER (uint16_t) ;
        case GB_UINT32_code : GB_WORKER (uint32_t) ;
        case GB_UINT64_code : GB_WORKER (uint64_t) ;
        case GB_FP32_code   : GB_WORKER (float) ;
        case GB_FP64_code   : GB_WORKER (double) ;
        default:
            {
                // worker for all user-defined types
                GB_BURBLE_N (mnz, "generic ") ;
                GB_void *GB_RESTRICT Cx = C->x ;
                #pragma omp parallel for num_threads(nthreads) schedule(static)
                for (pC = 0 ; pC < mnz ; pC++)
                { 
                    memcpy (Cx +((pC)*csize), cwork, csize) ;
                }
            }
            break ;
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    ASSERT_MATRIX_OK (C, "C output for subassign method_05e", GB0) ;
    return (GrB_SUCCESS) ;
}

