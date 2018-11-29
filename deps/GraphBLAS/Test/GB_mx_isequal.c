//------------------------------------------------------------------------------
// GB_mx_isequal: check if two matrices are equal
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

bool GB_mx_isequal     // true if A and B are exactly the same
(
    GrB_Matrix A,
    GrB_Matrix B
)
{
    // printf ("mx_isequal\n") ;

    if (A == B) return (true) ;
    if (A == NULL) return (false) ;
    if (B == NULL) return (false) ;

    if (A->magic != B->magic) return (false) ;
    if (A->type  != B->type ) return (false) ;
    if (A->vlen  != B->vlen ) return (false) ;
    if (A->vdim  != B->vdim ) return (false) ;
    if (A->nvec  != B->nvec ) return (false) ;

    if (GB_NNZ (A)  != GB_NNZ (B) ) return (false) ;

    if (A->is_hyper != B->is_hyper) return (false) ;
    if (A->is_csc   != B->is_csc  ) return (false) ;

    // these differences are OK:
    // if (A->plen  != B->plen ) return (false) ;
    // if (A->nzmax != B->nzmax) return (false) ;
    // if (A->max_n_pending != B->max_n_pending) return (false) ;
    // queue_next and queue_prev are expected to differ
    // if (A->enqueued != B->enqueued) return (false) ;

    if (A->p_shallow        != B->p_shallow        ) return (false) ;
    if (A->h_shallow        != B->h_shallow        ) return (false) ;
    if (A->i_shallow        != B->i_shallow        ) return (false) ;
    if (A->x_shallow        != B->i_shallow        ) return (false) ;
    if (A->n_pending        != B->n_pending        ) return (false) ;
    if (A->sorted_pending   != B->sorted_pending   ) return (false) ;
    if (A->operator_pending != B->operator_pending ) return (false) ;
    if (A->type_pending     != B->type_pending     ) return (false) ;
    if (A->type_pending_size!= B->type_pending_size) return (false) ;
    if (A->nzombies         != B->nzombies         ) return (false) ;

    int64_t n = A->nvec ;
    int64_t nnz = GB_NNZ (A) ;
    size_t s = sizeof (int64_t) ;
    size_t asize = A->type->size ;
    size_t psize = A->type_pending_size ;

    int64_t np = A->n_pending ;

    ASSERT (n >= 0 && n <= A->vdim) ;
    // printf ("mx_isequal: nvec "GBd" nnz "GBd", np "GBd"\n", n, nnz, np) ;

    if (!GB_mx_same  ((char *) A->p, (char *) B->p, (n+1) * s)) return (false) ;
    // printf ("p same\n") ;
    if (A->is_hyper)
    {
        if (!GB_mx_same ((char *) A->h, (char *) B->h, n * s)) return (false) ;
    }
    // printf ("h same\n") ;

    if (A->nzmax > 0 && B->nzmax > 0)
    {
        if (!GB_mx_same  ((char *) A->i, (char *) B->i, nnz * s))
            return (false) ;
        // printf ("i same\n") ;
        if (!GB_mx_xsame (A->x, B->x, nnz, asize, A->i))
            return (false) ;
        // printf ("x same\n") ;
    }

    if (!GB_mx_same  ((char *) A->i_pending, (char *) B->i_pending, np * s))
        return (false) ;
    // printf ("ip same\n") ;
    if (!GB_mx_same  ((char *) A->j_pending, (char *) B->j_pending, np * s))
        return (false) ;
    // printf ("jp same\n") ;
    if (!GB_mx_same (A->s_pending, B->s_pending, np*psize))
        return (false) ;
    // printf ("xp same\n") ;

    return (true) ;
}

