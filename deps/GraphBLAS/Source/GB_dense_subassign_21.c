//------------------------------------------------------------------------------
// GB_dense_subassign_21: C(:,:) = x where x is a scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C(:,:) = x where C is a matrix and x is a scalar

#include "GB_dense.h"
#include "GB_select.h"
#include "GB_Pending.h"

GrB_Info GB_dense_subassign_21      // C(:,:) = x; C is a matrix and x a scalar
(
    GrB_Matrix C,                   // input/output matrix
    const GB_void *scalar,          // input scalar
    const GrB_Type atype,           // type of the input scalar
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT_MATRIX_OK (C, "C for C(:,:)=x", GB0) ;
    ASSERT (scalar != NULL) ;
    // any prior pending tuples are discarded, and all zombies will be killed
    ASSERT (GB_PENDING_OK (C)) ; ASSERT (GB_ZOMBIES_OK (C)) ;
    ASSERT_TYPE_OK (atype, "atype for C(:,:)=x", GB0) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    int64_t cvdim = C->vdim ;
    int64_t cvlen = C->vlen ;
    GrB_Index cnzmax ;
    bool ok = GB_Index_multiply (&cnzmax, cvlen, cvdim) ;
    if (!ok)
    { 
        // problem too large
        return (GB_OUT_OF_MEMORY) ;
    }

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (cnzmax, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // typecast the scalar into the same type as C
    //--------------------------------------------------------------------------

    int64_t csize = C->type->size ;
    GB_cast_function
        cast_A_to_C = GB_cast_factory (C->type->code, atype->code) ;
    GB_void cwork [GB_VLA(csize)] ;
    cast_A_to_C (cwork, scalar, atype->size) ;

    //--------------------------------------------------------------------------
    // create the pattern, and allocate space for values, if needed
    //--------------------------------------------------------------------------

    // discard any prior pending tuples
    GB_Pending_free (&(C->Pending)) ;

    int64_t pC ;

    if (GB_NNZ (C) < cnzmax || C->x_shallow || C->i_shallow || C->is_hyper
        || GB_ZOMBIES (C))
    {

        //----------------------------------------------------------------------
        // C is not yet dense: create pattern and allocate values
        //----------------------------------------------------------------------

        // clear all prior content and recreate it; use exising header for C.
        // do not malloc C->x if the scalar is zero; calloc it later.
        bool scalar_is_nonzero = GB_is_nonzero (cwork, csize) ;
        GB_PHIX_FREE (C) ;
        GB_CREATE (&C, C->type, cvlen, cvdim, GB_Ap_malloc, C->is_csc,
            GB_FORCE_NONHYPER, C->hyper_ratio, C->vdim, cnzmax,
            scalar_is_nonzero, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            return (GB_OUT_OF_MEMORY) ;
        }

        int64_t *GB_RESTRICT Cp = C->p ;
        int64_t *GB_RESTRICT Ci = C->i ;
        int nth = GB_nthreads (cvdim, chunk, nthreads_max) ;

        // FUTURE:: dense data structure, where Cp and Ci will be implicit

        int64_t k ;
        #pragma omp parallel for num_threads(nth) schedule(static)
        for (k = 0 ; k <= cvdim ; k++)
        { 
            Cp [k] = k * cvlen ;
        }

        C->magic = GB_MAGIC ;
        C->nvec_nonempty = (cvlen == 0) ? 0 : cvdim ;

        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (pC = 0 ; pC < cnzmax ; pC++)
        { 
            Ci [pC] = pC % cvlen ;
        }

        if (!scalar_is_nonzero)
        { 
            GBBURBLE ("calloc ") ;
            GB_CALLOC_MEMORY (C->x, cnzmax, csize) ;
        }

        if (C->x == NULL)
        { 
            // out of memory
            GB_PHIX_FREE (C) ;
            return (GB_OUT_OF_MEMORY) ;
        }

        if (!scalar_is_nonzero)
        { 
            // quick return if the scalar is zero
            ASSERT_MATRIX_OK (C, "C(:,:)=0 output", GB0) ;
            return (GrB_SUCCESS) ;
        }
    }

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    // worker for built-in types
    #define GB_WORKER(ctype)                                                \
    {                                                                       \
        ctype *GB_RESTRICT Cx = C->x ;                                      \
        ctype x = (*(ctype *) cwork) ;                                      \
        GB_PRAGMA (omp parallel for num_threads(nthreads) schedule(static)) \
        for (pC = 0 ; pC < cnzmax ; pC++)                                   \
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
                GB_BURBLE_N (cnzmax, "generic ") ;
                GB_void *GB_RESTRICT Cx = C->x ;
                #pragma omp parallel for num_threads(nthreads) schedule(static)
                for (pC = 0 ; pC < cnzmax ; pC++)
                { 
                    memcpy (Cx +((pC)*csize), cwork, csize) ;
                }
            }
            break ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C(:,:)=x output", GB0) ;
    return (GrB_SUCCESS) ;
}

