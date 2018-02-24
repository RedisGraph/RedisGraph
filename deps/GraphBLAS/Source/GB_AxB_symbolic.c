//------------------------------------------------------------------------------
// GB_AxB_symbolic: find the pattern of C = A*B or variations
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = nonzero pattern of A*B in column or row oriented form.  The inputs
// A and B can optionally be transposed, as can the result C.   The row indices
// of the output are always returned sorted.

// The row indices of the input matrices A and B need not be sorted, but this
// holds for 'all' GraphBLAS matrices anyway (allowing A and B to be unsorted
// on input might be useful for internal computations in GraphBLAS).  The C->x
// and C->i content of C on input is ignored and will be cleared.  Any pending
// tuples or zombies in C are deleted.  C->p must be already allocated on
// input, but need not be initialized.  This algorithm is type-agnostic; it
// ignores the type of A, B, and C, and does not consider their numerical
// values.

// On output, the C->i will have been allocated to hold the pattern of C, and
// C->p is valid.  The C->x array, however, has NOT been allocated and is NULL.
// If present on input, C->x has been freed.

// If A_transpose is true, then A is transposed first.
// If B_transpose is true, then B is transposed first.

// If C_transpose is true, C' is computed first and then transposed when done.
// Computing the transpose can be faster since C=A*B relies on qsort to sort
// each column.  Computing C=(A*B)' can skip the qsort since the C_transpose
// acts as a bucket sort.

// If the method fails, C is returned as a valid empty matrix.

// If Mask is present, then C->i is allocated for NNZ(Mask) entries, but not
// computed.  The symbolic analysis is not performed. All content of C (C->p,
// C->i, and C->x) is computed in GB_AxB_numeric.  The content of the Mask
// is not used, just its size.

// FUTURE: If this were to be done in parallel, it would start with a first
// pass that just computed the column counts of A*B.  Next, a parallel prefix
// sum would construct Cp.  The next step would compute the pattern of each
// column of C, fully in parallel.  Each thread would need its own workspace.

#include "GB.h"

GrB_Info GB_AxB_symbolic        // pattern of C = A*B, A'*B, A*B', or A'*B'
(
    GrB_Matrix C,               // output matrix
    const GrB_Matrix Mask,      // if present, only NNZ(Mask) is used
    const GrB_Matrix Ainput,    // input matrix
    const GrB_Matrix Binput,    // input matrix
    const bool A_transpose,     // if true, A is transposed first
    const bool B_transpose,     // if true, B is transposed first
    const bool C_transpose      // if true, C is transposed when done
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // [ [ C need not be initialized, just the column pointers present
    ASSERT (C != NULL && C->p != NULL && !C->p_shallow) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for symbolic C=A*B", 0)) ;
    ASSERT_OK (GB_check (Ainput, "A for symbolic C=A*B", 0)) ;
    ASSERT_OK (GB_check (Binput, "B for symbolic C=A*B", 0)) ;
    ASSERT (!PENDING (C)) ;      ASSERT (!ZOMBIES (C)) ;
    ASSERT (!PENDING (Mask)) ;   ASSERT (!ZOMBIES (Mask)) ;
    ASSERT (!PENDING (Ainput)) ; ASSERT (!ZOMBIES (Ainput)) ;
    ASSERT (!PENDING (Binput)) ; ASSERT (!ZOMBIES (Binput)) ;

    int64_t anrows = (A_transpose) ? Ainput->ncols : Ainput->nrows ;
    int64_t ancols = (A_transpose) ? Ainput->nrows : Ainput->ncols ;

    int64_t bnrows = (B_transpose) ? Binput->ncols : Binput->nrows ;
    int64_t bncols = (B_transpose) ? Binput->nrows : Binput->ncols ;

    ASSERT (ancols == bnrows) ;
    ASSERT (C->nrows == ((C_transpose) ? bncols : anrows)) ;
    ASSERT (C->ncols == ((C_transpose) ? anrows : bncols)) ;

    //--------------------------------------------------------------------------
    // if Mask is present, just allocate space for C->i and return
    //--------------------------------------------------------------------------

    if (Mask != NULL)
    {
        double memory = 0 ;
        if (!GB_Matrix_alloc (C, NNZ (Mask), false, &memory))
        {
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "out of memory, %g GBytes required", memory))) ;
        }
        return (REPORT_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // transpose A and/or B if requested
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GrB_Matrix AT = NULL ;
    GrB_Matrix BT = NULL ;

    double imemory = 0, AT_memory = 0, BT_memory = 0 ;

    if (A_transpose)
    {
        // AT = pattern of Ainput'; boolean type but AT->x not allocated
        // [ AT-> malloc'ed and not initialized
        GB_NEW (&AT, GrB_BOOL, anrows, ancols, false, true) ;
        if (info != GrB_SUCCESS)
        {
            GB_Matrix_clear (C) ;
            return (info) ;
        }
        // no typecasting since the numerical values are not touched
        info = GB_Matrix_transpose (AT, Ainput, NULL, false) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&AT) ;
            GB_Matrix_clear (C) ;
            return (info) ;
        }
        AT_memory = GBYTES (AT->ncols + NNZ (AT), sizeof (int64_t)) ;
        // AT->p initialized ]
    }

    if (B_transpose)
    {
        // BT = pattern of Binput'; boolean type but AT->x not allocated
        // [ BT-> malloc'ed and not initialized
        GB_NEW (&BT, GrB_BOOL, bnrows, bncols, false, true) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&AT) ;
            GB_Matrix_clear (C) ;
            return (info) ;
        }
        // no typecasting since the numerical values are not touched
        info = GB_Matrix_transpose (BT, Binput, NULL, false) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&AT) ;
            GB_MATRIX_FREE (&BT) ;
            GB_Matrix_clear (C) ;
            return (info) ;
        }
        BT_memory = GBYTES (BT->ncols + NNZ (BT), sizeof (int64_t)) ;
        // BT->p initialized ]
    }

    GrB_Matrix A = (A_transpose) ? AT : Ainput ;
    GrB_Matrix B = (B_transpose) ? BT : Binput ;

    const int64_t *Bp = B->p ;
    const int64_t *Bi = B->i ;
    const int64_t *Ap = A->p ;
    const int64_t *Ai = A->i ;

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    GB_Matrix_ixfree (C) ;

    // A is now anrows-by-ancols, B is bnrows-by-bncols
    ASSERT_OK (GB_check (A, "A for symbolic A*B", 0)) ;
    ASSERT_OK (GB_check (B, "B for symbolic A*B", 0)) ;
    ASSERT (A->nrows == anrows && A->ncols == ancols) ;
    ASSERT (B->nrows == bnrows && B->ncols == bncols) ;

    // ensure Mark is at least of size anrows+1
    bool ok = GB_Mark_alloc (anrows+1) ;
    int64_t *Mark = GB_thread_local.Mark ;

    // memory for Mark, rowcount, and the temporary Cp
    imemory = GBYTES (anrows+1, sizeof (bool)) ;

    int64_t *rowcount = NULL ;          // size anrows + 1, or NULL
    int64_t *Cp = NULL ;                // points to either Work or C->p

    if (C_transpose)
    {
        // rowcount [i] will be nnz in the ith row of (A*B).
        // the pattern of C=(A*B)' will be returned, so Cp is temporary.
        // rowcount has size anrows+1 and Cp has size bncols+1, so ensure Work
        // is at least of size (anrows + 1) + (bncols + 1) integers.
        ok = ok && GB_Work_alloc (anrows + bncols + 2, sizeof (int64_t)) ;
        rowcount = (int64_t *) GB_thread_local.Work ;
        Cp = rowcount + (anrows+1) ;
        imemory += GBYTES (anrows + bncols + 2, sizeof (int64_t)) ;
    }
    else
    {
        // the column pattern of C=A*B is returned, so Cp will be kept.
        // no additional workspace needed.
        ASSERT (C->ncols == bncols) ;
        Cp = C->p ;
    }

    //--------------------------------------------------------------------------
    // estimate nnz(C)
    //--------------------------------------------------------------------------

    // Ci has size min (anrows + nnz(A) + nnz (B), anrows*bncols) + 1.  It
    // is allocated last of all, so that sometimes the system can realloc
    // it for free without moving it.

    // safe against integer overflow since all 3 values are <= GB_INDEX_MAX
    C->nzmax = anrows + NNZ (A) + NNZ (B) ;

    // abnzmax = anrows * bncols, but check for overflow
    GrB_Index abnzmax ;
    if (GB_Index_multiply (&abnzmax, anrows, bncols))
    {
        // only do this if anrows * bncols does not overflow
        C->nzmax = IMIN (C->nzmax, abnzmax) ;
    }

    C->nzmax++ ;

    //--------------------------------------------------------------------------
    // allocate Ci
    //--------------------------------------------------------------------------

    GB_MALLOC_MEMORY (int64_t *Ci, C->nzmax, sizeof (int64_t)) ;
    double Ci_memory = GBYTES (C->nzmax, sizeof (int64_t)) ;

    if (Ci == NULL || !ok)
    {
        // out of memory
        GB_MATRIX_FREE (&AT) ;
        GB_MATRIX_FREE (&BT) ;
        GB_Matrix_clear (C) ;
        GB_Mark_free ( ) ;
        GB_Work_free ( ) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required",
            imemory + Ci_memory + AT_memory + BT_memory))) ;
    }

    if (C_transpose)
    {
        // clear the rowcount
        for (int64_t i = 0 ; i <= anrows ; i++)
        {
            rowcount [i] = 0 ;
        }
    }

    //--------------------------------------------------------------------------
    // compute each column of the pattern of A*B
    //--------------------------------------------------------------------------

    Cp [0] = 0 ;
    int64_t cnz = 0 ;
    int64_t flag ;

    for (int64_t j = 0 ; j < bncols ; j++)
    {

        int64_t cmax ;

        //----------------------------------------------------------------------
        // reallocate C if necessary
        //----------------------------------------------------------------------

        cmax = cnz + anrows ;
        if (cmax > C->nzmax)
        {
            int64_t cold = C->nzmax ;
            int64_t cnew = 4*(C->nzmax + anrows) ;
            GB_REALLOC_MEMORY (Ci, cnew, C->nzmax, sizeof (int64_t), &ok) ;
            Ci_memory = GBYTES (C->nzmax, sizeof (int64_t)) ;
            if (!ok)
            {
                // out of memory
                GB_MATRIX_FREE (&AT) ;
                GB_MATRIX_FREE (&BT) ;
                GB_FREE_MEMORY (Ci, cold, sizeof (int64_t)) ;
                GB_Matrix_clear (C) ;
                GB_Mark_free ( ) ;
                GB_Work_free ( ) ;
                return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                    "out of memory, %g GBytes required",
                    imemory + Ci_memory + AT_memory + BT_memory))) ;
            }
            C->nzmax = cnew ;
        }

        // flag++ 
        flag = GB_Mark_reset (1, 0) ;

        //----------------------------------------------------------------------
        // C(:,j) = set union of all A(:,k) for each nonzero B(k,j) ;
        //----------------------------------------------------------------------

        bool needs_sorting = false ;
        ASSERT (Cp [j] == cnz) ;

        int64_t pb_start = Bp [j] ;
        int64_t pb_end   = Bp [j+1] ;
        int64_t bnz = pb_end - pb_start ;

        if (bnz == 0)
        {

            // B (:,k) is empty; nothing to do

        }
        else if (bnz == 1)
        {

            //------------------------------------------------------------------
            // C (:,j) = A (:,k) for a single nonzero B(k,j)
            //------------------------------------------------------------------

            // C(:,j) = A(:,k)
            int64_t k = Bi [pb_start] ;
            for (int64_t pa = Ap [k] ; pa < Ap [k+1] ; pa++)
            {
                int64_t i = Ai [pa] ;
                // C(i,j) is nonzero
                // add to the column pattern of A*B
                Ci [cnz++] = i ;
                if (C_transpose)
                {
                    rowcount [i]++ ;
                }
            }

        }
        else if (bnz == 2)
        {

            //------------------------------------------------------------------
            // 2-way merge of A (:,k1) and A (:,k2)
            //------------------------------------------------------------------

            int64_t k1 = Bi [pb_start] ;
            int64_t k2 = Bi [pb_start+1] ;
            int64_t p1 = Ap [k1] ;
            int64_t p2 = Ap [k2] ;
            int64_t p1_end = Ap [k1+1] ;
            int64_t p2_end = Ap [k2+1] ;

            while (p1 < p1_end || p2 < p2_end)
            {
                int64_t i1 = (p1 < p1_end) ? Ai [p1] : anrows ;
                int64_t i2 = (p2 < p2_end) ? Ai [p2] : anrows ;
                int64_t i ;
                if (i1 < i2)
                {
                    i = i1 ;
                    p1++ ;
                }
                else if (i1 > i2)
                {
                    i = i2 ;
                    p2++ ;
                }
                else // i1 == i2
                {
                    i = i1 ;
                    p1++ ;
                    p2++ ;
                }
                // C(i,j) is nonzero
                if (Mark [i] < flag)
                {
                    // C(i,j) is nonzero, and this is the 1st time row i
                    // has been added to the pattern in C(:,j).  Mark it so
                    // row i is not added again.
                    Mark [i] = flag ;
                    // add to the column pattern of A*B
                    Ci [cnz++] = i ;
                    if (C_transpose)
                    {
                        rowcount [i]++ ;
                    }
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // general case, nnz (B (:,j)) > 2
            //------------------------------------------------------------------

            for (int64_t pb = pb_start ; pb < pb_end ; pb++)
            {
                if (cnz == cmax)
                {
                    // C(:,j) is now completely full, no need to continue
                    break ;
                }
                // symbolic saxpy C(:,j) += A(:,k)*B(k,j)
                int64_t k = Bi [pb] ;
                for (int64_t pa = Ap [k] ; pa < Ap [k+1] ; pa++)
                {
                    int64_t i = Ai [pa] ;
                    // C(i,j) is nonzero
                    if (Mark [i] < flag)
                    {
                        // C(i,j) is nonzero, and this is the 1st time row i
                        // has been added to the pattern in C(:,j).  Mark it so
                        // row i is not added again.
                        Mark [i] = flag ;
                        // add to the column pattern of A*B
                        Ci [cnz++] = i ;
                        if (C_transpose)
                        {
                            rowcount [i]++ ;
                        }
                        else if (pb > pb_start) 
                        {
                            // more precisely, the column might still be in
                            // sorted order if this row i is > Ci [cnz-1].
                            needs_sorting = true ;
                        }
                    }
                }
            }
        }

        // log the end of the jth column of C and the start of column j+1
        Cp [j+1] = cnz ;

        //----------------------------------------------------------------------
        // sort the pattern of C(:,j)
        //----------------------------------------------------------------------

        if (needs_sorting)
        {
            // Sorting is not needed if C_transpose is true, since the tranpose
            // of C will also sort it.

            // This sort-based method is useful when A and B are very large and
            // C is small.  For example, a very tall and thin n-by-tiny sparse
            // matrix C with s << n nonzeros should not be transposed since it
            // takes O(n+s) time and space, whereas sorting the row indices of
            // the takes O(s log s).

            int64_t len = Cp [j+1] - Cp [j] ;
            if (len == anrows)
            {
                // no need to sort C(:,j) if it is entirely nonzero; recreate it
                for (int64_t pc = Cp [j], i = 0 ; pc < Cp [j+1] ; pc++, i++)
                {
                    Ci [pc] = i ;
                }
            }
            else
            {
                // sort the nonzero indices in C(:,j)
                GB_qsort_1 (Ci + Cp [j], len) ;
            }
        }
        ASSERT (cnz <= C->nzmax) ;
    }

    // clear the Mark array
    GB_Mark_reset (1, 0) ;

    // free workspace no longer needed
    GB_MATRIX_FREE (&AT) ;
    GB_MATRIX_FREE (&BT) ;
    AT_memory = BT_memory = 0 ;

    // If C_transpose is true, the matrix C may have jumbled row indices.
    // This is fixed below when C is transposed.

    //--------------------------------------------------------------------------
    // reduce the size of Ci to hold just the required space
    //--------------------------------------------------------------------------

    // this cannot fail since the size is shrinking
    int64_t cnew = IMAX (cnz, 1) ;
    ASSERT (cnew <= C->nzmax) ;
    GB_REALLOC_MEMORY (Ci, cnew, C->nzmax, sizeof (int64_t), &ok) ;
    ASSERT (ok) ;
    Ci_memory = GBYTES (cnew, sizeof (int64_t)) ;
    C->nzmax = cnew ;

    //--------------------------------------------------------------------------
    // column pattern of C=A*B is now computed, and is sorted if needed
    //--------------------------------------------------------------------------

    if (!C_transpose)
    {
        // C = A*B has been computed; keep Ci and Cp in the matrix C
        C->i = Ci ;                     // Ci becomes permanent
        C->magic = MAGIC ;              // C->p is now initialized ]
        ASSERT_OK (GB_check (C, "C output for symbolic C=A*B", 0)) ;
        return (REPORT_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // transpose the column-oriented form into the row-oriented form
    //--------------------------------------------------------------------------

    // allocate the permanent space for the row-form of (A*B)
    GB_MALLOC_MEMORY (C->i, C->nzmax, sizeof (int64_t)) ;
    Ci_memory += GBYTES (cnew, sizeof (int64_t)) ;

    if (C->i == NULL)
    {
        // out of memory
        GB_FREE_MEMORY (Ci, cnew, sizeof (int64_t)) ;
        GB_Matrix_clear (C) ;
        GB_Mark_free ( ) ;
        GB_Work_free ( ) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", imemory + Ci_memory))) ;
    }

    // C->p = cumsum ([0 rowcount]) ; rowcount = C->p ;
    GB_cumsum (C->p, rowcount, anrows) ;
    C->magic = MAGIC ;                  // C->p is now initialized ]

    ASSERT (anrows == C->ncols) ;
    ASSERT (cnz == C->p [anrows]) ;
    // rowcount [i] now points to the first empty slot in each row
    GB_transpose_pattern (Cp, Ci, rowcount, C->i, bncols) ;

    // free workspace and the column-oriented form
    ASSERT_OK (GB_check (C, "C output for symbolic C=(A*B)'", 0)) ;
    GB_FREE_MEMORY (Ci, cnew, sizeof (int64_t)) ; // Ci no longer needed
    return (REPORT_SUCCESS) ;
}

