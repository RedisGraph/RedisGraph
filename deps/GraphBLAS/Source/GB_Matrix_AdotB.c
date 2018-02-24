//------------------------------------------------------------------------------
// GB_Matrix_AdotB: compute C = Mask.*(A'*B) without forming A' via dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_Matrix_AdotB (C, Mask, A, B, semiring) computes the matrix multiplication
// Mask.*(A'*B) without forming A' explicitly.  It is useful when A is very
// tall and thin (n-by-1 in particular).  In that case A' is very costly to
// transpose, but A'*B is very easy if B is also tall and thin (say also
// n-by-1).

// If Mask is NULL, the method computes C=A'*B by allocating space as if C were
// DENSE, and this thus suitable only when C is small (such as a scalar,
// 2-by-2, or 1-by-n).  If Mask is present, the upper bound on the number of
// entries in C is the same as nnz(Mask), so that space is allocated for C.
// This function assumes the Mask is not complemented.

// On input, C must be of the right dimensions for C=A'*B.  The values and
// pattern of C on input are cleared.  C->type must be exactly the same as the
// monoid type (semiring->add->op->ztype).  This is not a user-callable
// function.

// Compare this function with GB_AxB_numeric, which computes C=A*B.  That
// function requires C->p and C->i to be constructed first, by GB_AxB_symbolic.
// Otherwise they are very similar.  The dot product in this algorithm is very
// much like the merge-add in GB_Matrix_add, except that the a merge in
// GB_Matrix_add produces a column (a(:,j)+b(:,j)), whereas the merge in this
// function produces a scalar (a(:,j)'*b(:,j)).

// FUTURE: if this were to be done in parallel, the best approach would be to
// ensure C and Mask have the same pattern.  If cij_exists is false, create a
// zombie entry with an arbitrary value (zero would make sense).  Then modifiy
// GB_accum_mask so that it can take as input a matrix with zombies, without
// needed to delete them first.  Alternatively, use a parallel prefix sum
// method to delete zombies in parallel.

#include "GB.h"
#include "GB_AxB_methods.h"

GrB_Info GB_Matrix_AdotB            // C = A'*B using dot product method
(
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix Mask,          // Mask matrix for C<M>=A'*B
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy               // if true, do z=fmult(b,a) vs fmult(a,b)
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // [ C need not be initialized, just the column pointers present
    ASSERT (C != NULL && C->p != NULL && !C->p_shallow) ;
    ASSERT_OK_OR_NULL (GB_check (Mask, "Mask for A'*B", 0)) ;
    ASSERT_OK (GB_check (A, "A for A'*B", 0)) ;
    ASSERT_OK (GB_check (B, "B for A'*B", 0)) ;
    ASSERT (!PENDING (Mask)) ; ASSERT (!ZOMBIES (Mask)) ;
    ASSERT (!PENDING (A)) ; ASSERT (!ZOMBIES (A)) ;
    ASSERT (!PENDING (B)) ; ASSERT (!ZOMBIES (B)) ;
    ASSERT_OK (GB_Semiring_check (semiring, "semiring for numeric A'*B", 0)) ;
    ASSERT (A->nrows == B->nrows) ;
    ASSERT (C->nrows == A->ncols && C->ncols == B->ncols) ;

    if (flipxy)
    {
        // z=fmult(b,a) will be computed
        ASSERT (GB_Type_compatible (A->type, semiring->multiply->ytype)) ;
        ASSERT (GB_Type_compatible (B->type, semiring->multiply->xtype)) ;
    }
    else
    {
        // z=fmult(a,b) will be computed
        ASSERT (GB_Type_compatible (A->type, semiring->multiply->xtype)) ;
        ASSERT (GB_Type_compatible (B->type, semiring->multiply->ytype)) ;
    }
    ASSERT (C->type == semiring->add->op->ztype) ;
    ASSERT (semiring->multiply->ztype == semiring->add->op->ztype) ;

    //--------------------------------------------------------------------------
    // allocate C->x, C->i, and workspace
    //--------------------------------------------------------------------------

    // clear C, keeping only the size and type, and C->p
    GB_Matrix_ixfree (C) ;
    ASSERT (!PENDING (C)) ;
    ASSERT (!ZOMBIES (C)) ;

    GrB_Index nzmax ;
    double memory = 0 ;

    if (Mask == NULL)
    {
        // allocate enough entries to hold C as if it were dense.
        // nzmax = C->nrows * C->ncols, but check for integer overflow
        // this should only be done if the dimensions of C are small!
        if (! (GB_Index_multiply (&nzmax, C->nrows, C->ncols) &&
               GB_Matrix_alloc (C, nzmax, true, &memory)))
        {
            // out of memory: free workspace and all of C
            GB_Matrix_clear (C) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "out of memory, %g GBytes required", memory))) ;
        }
    }
    else
    {
        // allocate enough space in C for each entry in the Mask
        nzmax = NNZ (Mask) ;
        if (!GB_Matrix_alloc (C, nzmax, true, &memory))
        {
            // out of memory: free workspace and all of C
            GB_Matrix_clear (C) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "out of memory, %g GBytes required", memory))) ;
        }
    }

    //--------------------------------------------------------------------------
    // C = A'*B, computing each entry with a dot product, via builtin semiring
    //--------------------------------------------------------------------------

    bool done = false ;

#ifndef GBCOMPACT

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_AdotB(add,mult,xyname) GB_AdotB_ ## add ## mult ## xyname

    #define AxB(add,mult,xyname) \
    { \
        GB_AdotB (add,mult,xyname) (C, Mask, A, B, flipxy) ; \
        done = true ; \
    } \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    GB_Opcode mult_opcode, add_opcode ;
    GB_Type_code xycode, zcode ;

    if (GB_semiring_builtin (A, B, semiring, flipxy,
        &mult_opcode, &add_opcode, &xycode, &zcode))
    {
        #include "GB_AxB_factory.c"
    }

#endif

    //--------------------------------------------------------------------------
    // C = A'*B, computing each entry with a dot product, with typecasting
    //--------------------------------------------------------------------------

    if (!done)
    {

        //----------------------------------------------------------------------
        // get operators, functions, workspace, contents of A, B, C, and Mask
        //----------------------------------------------------------------------

        const int64_t *Ai = A->i ;
        const int64_t *Bi = B->i ;
        const int64_t *Ap = A->p ;
        const int64_t *Bp = B->p ;
        int64_t *Ci = C->i ;
        int64_t *Cp = C->p ;
        int64_t n = B->ncols ;
        int64_t m = A->ncols ;
        int64_t nrows = B->nrows ;
        ASSERT (C->ncols == n) ;
        ASSERT (C->nrows == m) ;

        int64_t cnz = 0 ;

        const int64_t *Maskp = NULL ;
        const int64_t *Maski = NULL ;
        const void    *Maskx = NULL ;
        GB_cast_function cast_Mask = NULL ;
        size_t msize = 0 ;

        if (Mask != NULL)
        {
            Maskp = Mask->p ;
            Maski = Mask->i ;
            Maskx = Mask->x ;
            msize = Mask->type->size ;
            // get the function pointer for casting Mask(i,j) from its current
            // type into boolean
            cast_Mask = GB_cast_factory (GB_BOOL_code, Mask->type->code) ;
        }

        // get the semiring operators
        GrB_BinaryOp multiply = semiring->multiply ;
        GrB_Monoid add = semiring->add ;

        GB_binary_function fmult = multiply->function ;
        GB_binary_function fadd  = add->op->function ;

        size_t csize = C->type->size ;
        size_t asize = A->type->size ;
        size_t bsize = B->type->size ;

        size_t xsize = multiply->xtype->size ;
        size_t ysize = multiply->ytype->size ;

        // scalar workspace
        // flipxy false: aki = (xtype) A(k,i) and bkj = (ytype) B(k,j)
        // flipxy true:  aki = (ytype) A(k,i) and bkj = (xtype) B(k,j)
        char aki [flipxy ? ysize : xsize] ;
        char bkj [flipxy ? xsize : ysize] ;
        char zwork [csize] ;
        char cwork [csize] ;

        const void *Ax = A->x ;
        const void *Bx = B->x ;
        void *Cx = C->x ;
        void *cij = Cx ;        // advances through each entry of C

        void *identity = add->identity ;

        GB_cast_function cast_A, cast_B ;
        if (flipxy)
        {
            // A is typecasted to y, and B is typecasted to x
            cast_A = GB_cast_factory (multiply->ytype->code, A->type->code) ;
            cast_B = GB_cast_factory (multiply->xtype->code, B->type->code) ;
        }
        else
        {
            // A is typecasted to x, and B is typecasted to y
            cast_A = GB_cast_factory (multiply->xtype->code, A->type->code) ;
            cast_B = GB_cast_factory (multiply->ytype->code, B->type->code) ;
        }

        //----------------------------------------------------------------------
        // C = A'*B via dot products, function pointers, and typecasting 
        //----------------------------------------------------------------------

        #define GET(pa,pb)                                              \
        {                                                               \
            /* aki = A(k,i), located in Ax [pa] */                      \
            cast_A (aki, Ax +((pa)*asize), asize) ;                     \
            /* bkj = B(k,j), located in Bx [pb] */                      \
            cast_B (bkj, Bx +((pb)*bsize), bsize) ;                     \
        }

        #define MULT                                                    \
        {                                                               \
            if (flipxy)                                                 \
            {                                                           \
                /* zwork = bkj * aki */                                 \
                fmult (zwork, bkj, aki) ;                               \
            }                                                           \
            else                                                        \
            {                                                           \
                /* zwork = aki * bkj */                                 \
                fmult (zwork, aki, bkj) ;                               \
            }                                                           \
        }

        #define ADD                                                     \
        {                                                               \
                /* cij += A(k,i) * B(k,j), and add to the pattern */    \
                /* cwork = cij */                                       \
                memcpy (cwork, cij, csize) ;                            \
                /* cij = cwork + zwork */                               \
                fadd (cij, cwork, zwork) ;                              \
        }

        #define MERGE                                                   \
        {                                                               \
            GET (pa, pb) ;                                              \
            MULT ;                                                      \
            if (cij_exists)                                             \
            {                                                           \
                ADD ;                                                   \
            }                                                           \
            else                                                        \
            {                                                           \
                /* cij = A(k,i) * B(k,j), and add to the pattern */     \
                cij_exists = true ;                                     \
                /* cij = cwork */                                       \
                memcpy (cij, zwork, csize) ;                            \
            }                                                           \
            pa++ ;                                                      \
            pb++ ;                                                      \
        }

        for (int64_t j = 0 ; j < n ; j++)
        {

            //------------------------------------------------------------------
            // C(:,j) = A'*B(:,j)
            //------------------------------------------------------------------

            int64_t pb_start, pb_end, bjnz, ib_first, ib_last, kk1, kk2 ;
            if (!jinit (Cp, j, cnz, Bp, Bi, Maskp, m, &pb_start, &pb_end,
                &bjnz, &ib_first, &ib_last, &kk1, &kk2)) continue ;

            for (int64_t kk = kk1 ; kk < kk2 ; kk++)
            {

                //--------------------------------------------------------------
                // compute cij = A(:,i)' * B(:,j), using the semiring
                //--------------------------------------------------------------

                bool cij_exists = false ;   // C(i,j) not yet in the pattern
                int64_t i, pa, pa_end, pb, ainz ;
                if (!cij_init (kk, Maski, Maskx, cast_Mask, msize,
                    Ap, Ai, ib_first, ib_last, pb_start,
                    &i, &pa, &pa_end, &pb, &ainz)) continue ;

                // B(:,j) and A(:,i) both have at least one entry

                if (bjnz == nrows && ainz == nrows)
                {

                    //----------------------------------------------------------
                    // both A(:,i) and B(:,j) are dense
                    //----------------------------------------------------------

                    cij_exists = true ;
                    memcpy (cij, identity, csize) ;
                    for (int64_t k = 0 ; k < nrows ; k++)
                    {
                        GET (pa+k, pb+k) ;
                        MULT ;
                        ADD ;
                    }

                }
                else if (ainz == nrows)
                {

                    //----------------------------------------------------------
                    // A(:,i) is dense and B(:,j) is sparse
                    //----------------------------------------------------------

                    cij_exists = true ;
                    memcpy (cij, identity, csize) ;
                    for ( ; pb < pb_end ; pb++)
                    {
                        int64_t k = Bi [pb] ;
                        GET (pa+k, pb) ;
                        MULT ;
                        ADD ;
                    }

                }
                else if (bjnz == nrows)
                {

                    //----------------------------------------------------------
                    // A(:,i) is sparse and B(:,j) is dense
                    //----------------------------------------------------------

                    cij_exists = true ;
                    memcpy (cij, identity, csize) ;
                    for ( ; pa < pa_end ; pa++)
                    {
                        int64_t k = Ai [pa] ;
                        GET (pa, pb+k) ;
                        MULT ;
                        ADD ;
                    }

                }
                else if (ainz > 32 * bjnz)
                {

                    //----------------------------------------------------------
                    // B(:,j) is very sparse compared to A(:,i)
                    //----------------------------------------------------------

                    while (pa < pa_end && pb < pb_end)
                    {
                        int64_t ia = Ai [pa] ;
                        int64_t ib = Bi [pb] ;
                        if (ia < ib)
                        {
                            // A(ia,i) appears before B(ib,j)
                            // discard all entries A(ia:ib-1,i)
                            int64_t pleft = pa + 1 ;
                            int64_t pright = pa_end ;
                            GB_BINARY_TRIM_SEARCH (ib, Ai, pleft, pright) ;
                            ASSERT (pleft > pa) ;
                            pa = pleft ;
                        }
                        else if (ib < ia)
                        {
                            // B(ib,j) appears before A(ia,i)
                            pb++ ;
                        }
                        else // ia == ib == k
                        {
                            // A(k,i) and B(k,j) are the next entries to merge
                            MERGE ;
                        }
                    }

                }
                else if (bjnz > 32 * ainz)
                {

                    //----------------------------------------------------------
                    // A(:,i) is very sparse compared to B(:,j)
                    //----------------------------------------------------------

                    while (pa < pa_end && pb < pb_end)
                    {
                        int64_t ia = Ai [pa] ;
                        int64_t ib = Bi [pb] ;
                        if (ia < ib)
                        {
                            // A(ia,i) appears before B(ib,j)
                            pa++ ;
                        }
                        else if (ib < ia)
                        {
                            // B(ib,j) appears before A(ia,i)
                            // discard all entries B(ib:ia-1,j)
                            int64_t pleft = pb + 1 ;
                            int64_t pright = pb_end ;
                            GB_BINARY_TRIM_SEARCH (ia, Bi, pleft, pright) ;
                            ASSERT (pleft > pb) ;
                            pb = pleft ;
                        }
                        else // ia == ib == k
                        {
                            // A(k,i) and B(k,j) are the next entries to merge
                            MERGE ;
                        }
                    }

                }
                else
                {

                    //----------------------------------------------------------
                    // A(:,i) and B(:,j) have about the same sparsity
                    //----------------------------------------------------------

                    while (pa < pa_end && pb < pb_end)
                    {
                        int64_t ia = Ai [pa] ;
                        int64_t ib = Bi [pb] ;
                        if (ia < ib)
                        {
                            // A(ia,i) appears before B(ib,j)
                            pa++ ;
                        }
                        else if (ib < ia)
                        {
                            // B(ib,j) appears before A(ia,i)
                            pb++ ;
                        }
                        else // ia == ib
                        {
                            // A(k,i) and B(k,j) are the next entries to merge
                            MERGE ;
                        }
                    }
                }

                if (cij_exists)
                {
                    // C(i,j) = cij
                    cij += csize ;      // advance cij pointer to next value
                    Ci [cnz++] = i ;    // add row index i to pattern of C
                }
            }
        }

        // log the end of the last column
        Cp [n] = cnz ;
        ASSERT (cnz <= C->nzmax) ;
    }

    //--------------------------------------------------------------------------
    // trim the size of C: this cannot fail
    //--------------------------------------------------------------------------

    C->magic = MAGIC ;          // C is now initialized ]
    bool ok = GB_Matrix_realloc (C, NNZ (C), true, NULL) ;
    ASSERT (ok) ;
    ASSERT_OK (GB_check (C, "C = A'*B output", 0)) ;
    return (REPORT_SUCCESS) ;
}

#undef AxB
#undef GB_AdotB
#undef MERGE
#undef GET
#undef MULT
#undef ADD

