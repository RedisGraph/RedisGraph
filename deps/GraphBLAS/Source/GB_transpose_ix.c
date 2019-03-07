//------------------------------------------------------------------------------
// GB_transpose_ix: transpose the values and pattern of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The values of A are typecasted to R_type, the type of the R matrix.
// A can be sparse or hypersparse, but R is not hypersparse.

// The row pointers of the output matrix have already been computed, in Rp.
// Row i will appear in Ri, in the positions Rp [i] .. Rp [i+1], for the
// version of Rp on *input*.  On output, however, Rp has been shifted down
// by one.  Rp [0:m-1] has been over written with Rp [1:m].  They can be
// shifted back, if needed, but GraphBLAS treats this array Rp, on input
// to this function, as a throw-away copy of Rp.

// Compare with GB_transpose_op.c

// PARALLEL: the bucket transpose will not be simple to parallelize.  The qsort
// method of transpose would be more parallel.  This method might remain mostly
// sequential.

#include "GB.h"

void GB_transpose_ix        // transpose the pattern and values of a matrix
(
    int64_t *Rp,            // size m+1, input: row pointers, shifted on output
    int64_t *Ri,            // size cnz, output column indices
    GB_void *Rx,            // size cnz, output numerical values, type R_type
    const GrB_Type R_type,  // type of output R (do typecasting into R)
    const GrB_Matrix A,     // input matrix
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;
    ASSERT (R_type != NULL) ;
    ASSERT (Rp != NULL && Ri != NULL && Rx != NULL) ;
    ASSERT (GB_Type_compatible (A->type, R_type)) ;
    ASSERT (!GB_ZOMBIES (A)) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS (nthreads, Context) ;

    //--------------------------------------------------------------------------
    // get the input matrix
    //--------------------------------------------------------------------------

    const int64_t *Ai = A->i ;
    const GB_void *Ax = A->x ;

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_WORKER(rtype,atype)                              \
    {                                                           \
        rtype *rx = (rtype *) Rx ;                              \
        atype *ax = (atype *) Ax ;                              \
        GBI_for_each_vector (A)                                 \
        {                                                       \
            GBI_for_each_entry (j, p, pend)                     \
            {                                                   \
                int64_t q = Rp [Ai [p]]++ ;                     \
                Ri [q] = j ;                                    \
                /* rx [q] = ax [p], type casting */             \
                GB_CAST (rx [q], ax [p]) ;                      \
            }                                                   \
        }                                                       \
        return ;                                                \
    }

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    // The switch factory cannot be disabled by #ifndef GBCOMPACT
    // because the generic worker does no typecasting.

    // switch factory for two types, controlled by code1 and code2
    GB_Type_code code1 = R_type->code ;         // defines rtype
    GB_Type_code code2 = A->type->code ;        // defines atype
    ASSERT (code1 <= GB_UDT_code) ;
    ASSERT (code2 <= GB_UDT_code) ;
    #include "GB_2type_template.c"

    //--------------------------------------------------------------------------
    // generic worker
    //--------------------------------------------------------------------------

    // the switch factory handles all built-in types; user-defined types
    // fall through the switch factory to here, which can never be
    // typecasted.  Because the generic worker does no typecasting, the
    // switch factory cannot be disabled.
    ASSERT (A->type == R_type && A->type->code > GB_FP64_code) ;

    int64_t asize = A->type->size ;
    GBI_for_each_vector (A)
    {
        GBI_for_each_entry (j, p, pend)
        { 
            int64_t q = Rp [Ai [p]]++ ;
            Ri [q] = j ;
            memcpy (Rx +(q*asize), Ax +(p*asize), asize) ;
        }
    }
}

