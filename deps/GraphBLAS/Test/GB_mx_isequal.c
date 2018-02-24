//------------------------------------------------------------------------------
// GB_mx_isequal: check if two matrices are equal
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

bool GB_mx_isequal     // true if A and B are exactly the same
(
    GrB_Matrix A,
    GrB_Matrix B
)
{

    if (A == B) return (true) ;
    if (A == NULL) return (false) ;
    if (B == NULL) return (false) ;

    if (A->magic != B->magic) return (false) ;
    if (A->type  != B->type ) return (false) ;
    if (A->nrows != B->nrows) return (false) ;
    if (A->ncols != B->ncols) return (false) ;
    if (NNZ (A)  != NNZ (B) ) return (false) ;

    // these differences are OK:
    // if (A->nzmax != B->nzmax) return (false) ;
    // if (A->max_npending != B->max_npending) return (false) ;
    // queue_next and queue_prev are expected to differ

    if (A->p_shallow        != B->p_shallow       ) return (false) ;
    if (A->i_shallow        != B->i_shallow       ) return (false) ;
    if (A->x_shallow        != B->i_shallow       ) return (false) ;
    if (A->npending         != B->npending        ) return (false) ;
    if (A->sorted_pending   != B->sorted_pending  ) return (false) ;
    if (A->operator_pending != B->operator_pending) return (false) ;
    if (A->nzombies         != B->nzombies        ) return (false) ;
    if (A->enqueued         != B->enqueued        ) return (false) ;

    int64_t n = A->ncols ;
    int64_t nnz = NNZ (A) ;
    size_t s = sizeof (int64_t) ;
    size_t a = A->type->size ;
    if (!GB_mx_same  ((char *) A->p, (char *) B->p, (n+1) * s)) return (false) ;
    if (!GB_mx_same  ((char *) A->i, (char *) B->i, nnz * s)) return (false) ;
    if (!GB_mx_xsame (A->x, B->x, nnz, a, A->i)) return (false) ;

    int64_t np = A->npending ;
    if (!GB_mx_same  ((char *) A->ipending, (char *) B->ipending, np * s))
        return (false) ;
    if (!GB_mx_same  ((char *) A->jpending, (char *) B->jpending, np * s))
        return (false) ;
    if (!GB_mx_xsame (A->xpending, B->xpending, np, a, A->i)) return (false) ;

    return (true) ;
}

