//------------------------------------------------------------------------------
// GB_transpose_ix: transpose the values and pattern of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The input matrix is m-by-n with cnz nonzeros, with column pointers Ap of
// size n+1.  The pattern of column j is in Ai [Ap [j] ... Ap [j+1]] and thus
// cnz is equal to Ap [n].

// The values of the input matrix are in Ax, of type A_type.  They are
// typecasted to R_type, the type of the R matrix.

// The row pointers of the output matrix have already been computed, in Rp.
// Row i will appear in Ri, in the positions Rp [i] .. Rp [i+1], for the
// version of Rp on *input*.  On output, however, Rp has been shifted down
// by one.  Rp [0:m-1] has been over written with Rp [1:m].  They can be
// shifted back, if needed, but GraphBLAS treats this array Rp, on input
// to this function, as a throw-away copy of Rp.

// Compare with GB_transpose_op.c

#include "GB.h"

void GB_transpose_ix        // transpose the pattern and values of a matrix
(
    const int64_t *Ap,      // size n+1, input column pointers
    const int64_t *Ai,      // size cnz, input row indices
    const void *Ax,         // size cnz, input numerical values
    const GrB_Type A_type,  // type of input A
    int64_t *Rp,            // size m+1, input: row pointers, shifted on output
    int64_t *Ri,            // size cnz, output column indices
    void *Rx,               // size cnz, output numerical values, type R_type
    const int64_t n,        // number of columns in input
    const GrB_Type R_type   // type of output R (do typecasting into R)
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (A_type, "A type for transpose_ix", 0)) ;
    ASSERT_OK (GB_check (R_type, "R type for transpose_ix", 0)) ;
    ASSERT (Ap != NULL && Ai != NULL && Ax != NULL) ;
    ASSERT (Rp != NULL && Ri != NULL && Rx != NULL) ;
    ASSERT (n >= 0) ;

    ASSERT (GB_Type_compatible (A_type, R_type)) ;

    // no zombies are tolerated
    #ifndef NDEBUG
    // there is no A->nzombies flag to check, so check the whole pattern
    int64_t anz = Ap [n] ;
    for (int64_t p = 0 ; p < anz ; p++)
    {
        ASSERT (IS_NOT_ZOMBIE (Ai [p])) ;
    }
    #endif

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define WORKER(rtype,atype)                                 \
    {                                                           \
        rtype *rx = (rtype *) Rx ;                              \
        atype *ax = (atype *) Ax ;                              \
        for (int64_t j = 0 ; j < n ; j++)                       \
        {                                                       \
            for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)       \
            {                                                   \
                int64_t q = Rp [Ai [p]]++ ;                     \
                Ri [q] = j ;                                    \
                /* rx [q] = ax [p], type casting */             \
                CAST (rx [q], ax [p]) ;                         \
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
    GB_Type_code code2 = A_type->code ;         // defines atype
    ASSERT (code1 <= GB_UDT_code) ;
    ASSERT (code2 <= GB_UDT_code) ;
    #include "GB_2type_template.c"

    #undef WORKER

    //--------------------------------------------------------------------------
    // generic worker
    //--------------------------------------------------------------------------

    // the switch factory handles all built-in types; user-defined types
    // fall through the switch factory to here, which can never be
    // typecasted.  Because the generic worker does no typecasting, the
    // switch factory cannot be disabled.
    ASSERT (A_type == R_type && A_type->code == GB_UDT_code) ;

    int64_t asize = A_type->size ;
    for (int64_t j = 0 ; j < n ; j++)
    {
        for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
        {
            int64_t q = Rp [Ai [p]]++ ;
            Ri [q] = j ;
            memcpy (Rx +(q*asize), Ax +(p*asize), asize) ;
        }
    }
}

