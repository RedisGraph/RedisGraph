//------------------------------------------------------------------------------
// GB_AxB_flopcount: find the flop count for C=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// compute flop count for C=A*B.  Each multiply-add pair counts just once.

// Returns true if the flop count is computed (and thus below the flimit).  If
// the flop count exceeds the specified flimit, or if exceeds INT64_MAX, the
// the function returns false.

#include "GB.h"

bool GB_AxB_flopcount           // true if count computed, false if hit flimit
(
    const GrB_Matrix A,         // input matrix
    const GrB_Matrix B,         // input matrix
    const int64_t flimit,       // stop counting if flop count hits the flimit,
                                // or if flop count reaches integer overflow
    int64_t *flopcount          // flop count for C=A*B, if limit not reached
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (A, "A for flopcount C=A*B", 0)) ;
    ASSERT_OK (GB_check (B, "B for flopcount C=A*B", 0)) ;
    ASSERT (!PENDING (A)) ; ASSERT (!ZOMBIES (A)) ;
    ASSERT (!PENDING (B)) ; ASSERT (!ZOMBIES (B)) ;
    ASSERT (A->ncols == B->nrows) ;

    int64_t n = B->ncols ;
    int64_t flops = 0 ;

    const int64_t *Bi = B->i ;
    const int64_t *Ap = A->p ;
    int64_t bnz = NNZ (B) ;

    //--------------------------------------------------------------------------
    // compute the flop count
    //--------------------------------------------------------------------------

    for (int64_t p = 0 ; p < bnz ; p++)
    {
        int64_t k = Bi [p] ;
        flops += (Ap [k+1] - Ap [k]) ;
        if (flops < 0 || flops > flimit)
        {
            // integer overflow, or flimit reached
            return (false) ;
        }
    }

    //--------------------------------------------------------------------------
    // return the flop count
    //--------------------------------------------------------------------------

    (*flopcount) = flops ;
    return (true) ;
}

