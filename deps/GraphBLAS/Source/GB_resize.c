//------------------------------------------------------------------------------
// GB_resize: change the size of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_resize              // change the size of a matrix
(
    GrB_Matrix A,               // matrix to modify
    const GrB_Index nrows_new,  // new number of rows in matrix
    const GrB_Index ncols_new   // new number of columns in matrix
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (A, "A to resize", 0)) ;

    // delete any lingering zombies and assemble any pending tuples
    APPLY_PENDING_UPDATES (A) ;

    //--------------------------------------------------------------------------
    // resize the matrix
    //--------------------------------------------------------------------------

    // change the size of A->p
    if (ncols_new != A->ncols)
    {
        bool ok = true ;
        GB_REALLOC_MEMORY (A->p, ncols_new+1, A->ncols+1, sizeof (int64_t),
            &ok) ;
        if (!ok)
        {
            // out of memory
            double memory = GBYTES (ncols_new + 1, sizeof (int64_t)) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "out of memory, %g GBytes required", memory))) ;
        }
    }

    int64_t *restrict Ap = A->p ;
    int64_t *restrict Ai = A->i ;
    void    *restrict Ax = A->x ;

    // if # of columns is increasing, extend the column pointers
    if (ncols_new > A->ncols)
    {
        int64_t anz = Ap [A->ncols] ;
        for (int64_t j = A->ncols + 1 ; j <= ncols_new ; j++)
        {
            Ap [j] = anz ;
        }
    }

    A->ncols = ncols_new ;
    ASSERT_OK (GB_check (A, "A col resized", 0)) ;

    // if # of rows is shrinking, delete entries outside the new matrix
    if (nrows_new < A->nrows)
    {
        int64_t anz = 0 ;
        int64_t asize = A->type->size ;
        for (int64_t j = 0 ; j < ncols_new ; j++)
        {
            int64_t p = Ap [j] ;
            Ap [j] = anz ;
            for ( ; p < Ap [j+1] ; p++)
            {
                int64_t i = Ai [p] ;
                if (i < nrows_new)
                {
                    // keep this entry
                    Ai [anz] = i ;
                    memcpy (Ax +(anz*asize), Ax +(p*asize), asize) ;
                    anz++ ;
                }
            }
        }
        Ap [ncols_new] = anz ;
    }

    // matrix has been resized
    A->nrows = nrows_new ;

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (A, "A resized", 0)) ;
    return (REPORT_SUCCESS) ;
}

