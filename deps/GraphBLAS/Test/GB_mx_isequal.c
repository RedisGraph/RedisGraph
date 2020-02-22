//------------------------------------------------------------------------------
// GB_mx_isequal: check if two matrices are equal
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

bool GB_mx_isequal     // true if A and B are exactly the same
(
    GrB_Matrix A,
    GrB_Matrix B,
    double eps      // if A and B are both FP32 or FP64, and if eps > 0,
                    // then the values are considered equal if their relative
                    // difference is less than or equal to eps.
)
{
    // printf ("mx_isequal\n") ;

    if (A == B) return (true) ;
    if (A == NULL) return (false) ;
    if (B == NULL) return (false) ;

    GB_Pending AP = A->Pending ;
    GB_Pending BP = B->Pending ;

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
    // if (AP->nmax != BP->nmax) return (false) ;
    // queue_next and queue_prev are expected to differ
    // if (A->enqueued != B->enqueued) return (false) ;

    if (A->p_shallow        != B->p_shallow        ) return (false) ;
    if (A->h_shallow        != B->h_shallow        ) return (false) ;
    if (A->i_shallow        != B->i_shallow        ) return (false) ;
    if (A->x_shallow        != B->i_shallow        ) return (false) ;
    if (A->nzombies         != B->nzombies         ) return (false) ;

    if ((AP != NULL) != (BP != NULL)) return (false) ;

    if (AP != NULL)
    {
        if (AP->n      != BP->n     ) return (false) ;
        if (AP->sorted != BP->sorted) return (false) ;
        if (AP->op     != BP->op    ) return (false) ;
        if (AP->type   != BP->type  ) return (false) ;
        if (AP->size   != BP->size  ) return (false) ;
    }

    int64_t n = A->nvec ;
    int64_t nnz = GB_NNZ (A) ;
    size_t s = sizeof (int64_t) ;
    size_t asize = A->type->size ;

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

        if (A->type == GrB_FP32 && eps > 0)
        {
            if (!GB_mx_xsame32 (A->x, B->x, nnz, A->i, eps))
                return (false) ;
        }
        else if (A->type == GrB_FP64 && eps > 0)
        {
            if (!GB_mx_xsame64 (A->x, B->x, nnz, A->i, eps))
                return (false) ;
        }
        else
        {
            if (!GB_mx_xsame (A->x, B->x, nnz, asize, A->i))
                return (false) ;
        }
        // printf ("x same\n") ;
    }

    if (AP != NULL)
    {
        size_t psize = AP->size ;
        int64_t np = AP->n ;
        if (!GB_mx_same ((char *) AP->i, (char *) BP->i, np*s)) return (false) ;
        if (!GB_mx_same ((char *) AP->j, (char *) BP->j, np*s)) return (false) ;
        if (!GB_mx_same ((char *) AP->x, (char *) BP->x, np*psize))
        {
            return (false) ;
        }
        // printf ("xp same\n") ;
    }

    return (true) ;
}

