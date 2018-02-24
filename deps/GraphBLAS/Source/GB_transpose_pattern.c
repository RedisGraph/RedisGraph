//------------------------------------------------------------------------------
// GB_transpose_pattern: transpose the pattern of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The input matrix is m-by-n with anz nonzeros, with column pointers Ap of
// size n+1.  The pattern of column j is in Ai [Ap [j] ... Ap [j+1]] and
// thus anz is equal to Ap [n].

// The row pointers of the output matrix have already been computed, in Rp.
// Row i will appear in Ri, in the positions Rp [i] .. Rp [i+1], for the
// version of Rp on *input*.  On output, however, Rp has been shifted down
// by one.  Rp [0:n-1] has been over written with Rp [1:n].  They can be
// shifted back, if needed, but GraphBLAS treats this array Rp, on input
// to this function, as a throw-away copy of Rp.

// The input matrix may have jumbled row indices; this is OK.  See
// GB_Matrix_transpose and GB_AxB_symbolic.  The output matrix will
// always have sorted indices.

#include "GB.h"

void GB_transpose_pattern   // transpose the pattern of a matrix
(
    const int64_t *Ap,      // size n+1, input column pointers
    const int64_t *Ai,      // size anz, input row indices
    int64_t *Rp,            // size m+1, input row pointers, shifted on output
    int64_t *Ri,            // size anz, output column indices
    const int64_t n         // number of columns in input
)
{
    
    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Ap != NULL && Ai != NULL) ;
    ASSERT (Rp != NULL && Ri != NULL) ;
    ASSERT (n >= 0) ;

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
    // transpose the pattern
    //--------------------------------------------------------------------------

    for (int64_t j = 0 ; j < n ; j++)
    {
        for (int64_t p = Ap [j] ; p < Ap [j+1] ; p++)
        {
            Ri [Rp [Ai [p]]++] = j ;
        }
    }
}

