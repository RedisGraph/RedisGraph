//------------------------------------------------------------------------------
// GB_AxB__lxor_ne_int8:  hard-coded C=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

// If this filename has a double underscore in its name ("__") then it has
// been automatically constructed from Template/GB*AxB.[ch], via the axb*.m
// scripts, and should not be editted.  Edit the original source file instead.

//------------------------------------------------------------------------------

#include "GB.h"
#ifndef GBCOMPACT
#include "GB_AxB_methods.h"

// The C=A*B semiring is defined by the following types and operators:

// A*B function:  GB_AxB__lxor_ne_int8
// A'*B function: GB_AdotB__lxor_ne_int8
// Z type :  bool (the type of C)
// XY type:  int8_t (the type of A and B)
// Identity: false (where cij = (cij != false) does not change cij)
// Multiply: t = (aik != bkj)
// Add:      cij = (cij != t)

//------------------------------------------------------------------------------
// C<M>=A*B and C=A*B: outer product
//------------------------------------------------------------------------------

void GB_AxB__lxor_ne_int8
(
    GrB_Matrix C,
    const GrB_Matrix Mask,
    const GrB_Matrix A,
    const GrB_Matrix B,
    bool flip                   // if true, A and B have been swapped
)
{

    //--------------------------------------------------------------------------
    // get A, B, and C
    //--------------------------------------------------------------------------

    // w has size C->nrows == A->nrows, each entry size zsize.  uninitialized.
    bool *restrict w = GB_thread_local.Work ;

    bool *restrict Cx = C->x ;
    const int8_t *restrict Ax = A->x ;
    const int8_t *restrict Bx = B->x ;

    const int64_t n = C->ncols ;
    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ai = A->i ;
    const int64_t *restrict Bp = B->p ;
    const int64_t *restrict Bi = B->i ;

    if (Mask != NULL)
    {

        //----------------------------------------------------------------------
        // C<Mask> = A*B where Mask is pattern of C, with zombies
        //----------------------------------------------------------------------

        // get the Flag workspace (already allocated and cleared)
        int8_t *restrict Flag = GB_thread_local.Flag ;

        // get the mask
        const int64_t *restrict Maskp = Mask->p ;
        const int64_t *restrict Maski = Mask->i ;
        const void    *restrict Maskx = Mask->x ;
        GB_cast_function cast_Mask =
            GB_cast_factory (GB_BOOL_code, Mask->type->code) ;
        size_t msize = Mask->type->size ;

        #ifdef WITH_ZOMBIES
        // copy Maskp into C->p
        memcpy (C->p, Maskp, (n+1) * sizeof (int64_t)) ;
        C->magic = MAGIC ;
        #else
        int64_t cnz = 0 ;
        int64_t *restrict Cp = C->p ;
        #endif

        int64_t *restrict Ci = C->i ;

        for (int64_t j = 0 ; j < n ; j++)
        {

            //------------------------------------------------------------------
            // compute C(;,j) = A * B(:,j), both values and pattern
            //------------------------------------------------------------------

            // skip this column j if the Mask is empty
            #ifndef WITH_ZOMBIES
            Cp [j] = cnz ;
            #endif
            int64_t mlo, mhi ;
            if (empty (Maskp, Maski, j, &mlo, &mhi)) continue ;
            bool marked = false ;

            for (int64_t p = Bp [j] ; p < Bp [j+1] ; p++)
            {
                // B(k,j) is present
                int64_t k = Bi [p] ;
                // skip A(:,k) if empty or if entries out of range of Mask
                int64_t alo, ahi ;
                if (empty (Ap, Ai, k, &alo, &ahi)) continue ;
                if (ahi < mlo || alo > mhi) continue ;
                // scatter Mask(:,j) into Flag if not yet done
                scatter_mask (j, Maskp, Maski, Maskx, msize, cast_Mask, Flag,
                    &marked) ;
                int8_t bkj = Bx [p] ;
                for (int64_t pa = Ap [k] ; pa < Ap [k+1] ; pa++)
                {
                    // w [i] += (A(i,k) * B(k,j)) .* Mask(i,j)
                    int64_t i = Ai [pa] ;
                    int8_t flag = Flag [i] ;
                    if (flag == 0) continue ;
                    // Mask(i,j) == 1 so do the work
                    int8_t aik = Ax [pa] ;
                    bool t = aik != bkj ;
                    if (flag > 0)
                    {
                        // first time C(i,j) seen
                        Flag [i] = -1 ;
                        w [i] = t ;
                    }
                    else
                    {
                        // C(i,j) seen before, update it
                        w [i] = (w [i] != t) ;
                    }
                }
            }

            #ifdef WITH_ZOMBIES

                // gather C(:,j), both values and pattern, from the Mask(:,j)
                if (marked)
                {
                    for (int64_t p = Maskp [j] ; p < Maskp [j+1] ; p++)
                    {
                        int64_t i = Maski [p] ;
                        // C(i,j) is present
                        if (Flag [i] < 0)
                        {
                            // C(i,j) is a live entry, gather its row and value
                            Cx [p] = w [i] ;
                            Ci [p] = i ;
                        }
                        else
                        {
                            // C(i,j) is a zombie; in the Mask but not in A*B
                            Cx [p] = false ;
                            Ci [p] = FLIP (i) ;
                            C->nzombies++ ;
                        }
                        Flag [i] = 0 ;
                    }
                }
                else
                {
                    for (int64_t p = Maskp [j] ; p < Maskp [j+1] ; p++)
                    {
                        int64_t i = Maski [p] ;
                        // C(i,j) is a zombie; in the Mask but not in A*B
                        Cx [p] = false ;
                        Ci [p] = FLIP (i) ;
                        C->nzombies++ ;
                    }
                }

            #else

                // gather C(:,j), both values and pattern, from the Mask(:,j)
                if (marked)
                {
                    for (int64_t p = Maskp [j] ; p < Maskp [j+1] ; p++)
                    {
                        int64_t i = Maski [p] ;
                        // C(i,j) is present
                        if (Flag [i] < 0)
                        {
                            // C(i,j) is a live entry, gather its row and value
                            Cx [cnz] = w [i] ;
                            Ci [cnz++] = i ;
                        }
                        Flag [i] = 0 ;
                    }
                }

            #endif

        }

        #ifdef WITH_ZOMBIES
        // place C in the queue if it has zombies
        GB_queue_insert (C) ;
        #else
        Cp [n] = cnz ;
        C->magic = MAGIC ;
        #endif

    }
    else
    {

        //----------------------------------------------------------------------
        // C = A*B with pattern of C computed by GB_AxB_symbolic
        //----------------------------------------------------------------------

        const int64_t *restrict Cp = C->p ;
        const int64_t *restrict Ci = C->i ;

        for (int64_t j = 0 ; j < n ; j++)
        {
            // clear w
            for (int64_t p = Cp [j] ; p < Cp [j+1] ; p++)
            {
                w [Ci [p]] = false ;
            }
            // compute C(;,j)
            for (int64_t p = Bp [j] ; p < Bp [j+1] ; p++)
            {
                // B(k,j) is present
                int64_t k = Bi [p] ;
                int8_t bkj = Bx [p] ;
                for (int64_t pa = Ap [k] ; pa < Ap [k+1] ; pa++)
                {
                    // w [i] += A(i,k) * B(k,j)
                    int64_t i = Ai [pa] ;
                    int8_t aik = Ax [pa] ;
                    bool t = aik != bkj ;
                    w [i] = (w [i] != t) ;
                }
            }
            // gather C(:,j)
            for (int64_t p = Cp [j] ; p < Cp [j+1] ; p++)
            {
                Cx [p] = w [Ci [p]] ;
            }
        }
    }
}


//------------------------------------------------------------------------------
// C<M>=A'*B: dot product
//------------------------------------------------------------------------------

void GB_AdotB__lxor_ne_int8
(
    GrB_Matrix C,
    const GrB_Matrix Mask,
    const GrB_Matrix A,
    const GrB_Matrix B,
    bool flip                   // if true, A and B have been swapped
)
{

    //--------------------------------------------------------------------------
    // get A, B, C, and Mask
    //--------------------------------------------------------------------------

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

    #define MERGE                                           \
    {                                                       \
        int8_t aki = Ax [pa++] ;    /* aki = A(k,i) */      \
        int8_t bkj = Bx [pb++] ;    /* bjk = B(k,j) */      \
        bool t = aki != bkj ;                          \
        if (cij_exists)                                     \
        {                                                   \
            /* cij += A(k,i) * B(k,j) */                    \
            cij = (cij != t) ;                                   \
        }                                                   \
        else                                                \
        {                                                   \
            /* cij = A(k,i) * B(k,j) */                     \
            cij_exists = true ;                             \
            cij = t ;                                       \
        }                                                   \
    }

    bool *Cx = C->x ;
    const int8_t *Ax = A->x ;
    const int8_t *Bx = B->x ;

    for (int64_t j = 0 ; j < n ; j++)
    {

        //----------------------------------------------------------------------
        // C(:,j) = A'*B(:,j)
        //----------------------------------------------------------------------

        int64_t pb_start, pb_end, bjnz, ib_first, ib_last, kk1, kk2 ;
        if (!jinit (Cp, j, cnz, Bp, Bi, Maskp, m, &pb_start, &pb_end,
            &bjnz, &ib_first, &ib_last, &kk1, &kk2)) continue ;

        for (int64_t kk = kk1 ; kk < kk2 ; kk++)
        {

            //------------------------------------------------------------------
            // compute cij = A(:,i)' * B(:,j), using the semiring
            //------------------------------------------------------------------

            bool cij ;
            bool cij_exists = false ;   // C(i,j) not yet in the pattern
            int64_t i, pa, pa_end, pb, ainz ;
            if (!cij_init (kk, Maski, Maskx, cast_Mask, msize,
                Ap, Ai, ib_first, ib_last, pb_start,
                &i, &pa, &pa_end, &pb, &ainz)) continue ;

            // B(:,j) and A(:,i) both have at least one entry

            if (bjnz == nrows && ainz == nrows)
            {

                //--------------------------------------------------------------
                // both A(:,i) and B(:,j) are dense
                //--------------------------------------------------------------

                cij_exists = true ;
                cij = false ;
                for (int64_t k = 0 ; k < nrows ; k++)
                {
                    int8_t aki = Ax [pa + k] ;      // aki = A(k,i)
                    int8_t bkj = Bx [pb + k] ;      // bkj = B(k,j)
                    bool t = aki != bkj ;
                    cij = (cij != t) ;
                }

            }
            else if (ainz == nrows)
            {

                //--------------------------------------------------------------
                // A(:,i) is dense and B(:,j) is sparse
                //--------------------------------------------------------------

                cij_exists = true ;
                cij = false ;
                for ( ; pb < pb_end ; pb++)
                {
                    int64_t k = Bi [pb] ;
                    int8_t aki = Ax [pa + k] ;      // aki = A(k,i)
                    int8_t bkj = Bx [pb] ;          // bkj = B(k,j)
                    bool t = aki != bkj ;
                    cij = (cij != t) ;
                }

            }
            else if (bjnz == nrows)
            {

                //--------------------------------------------------------------
                // A(:,i) is sparse and B(:,j) is dense
                //--------------------------------------------------------------

                cij_exists = true ;
                cij = false ;
                for ( ; pa < pa_end ; pa++)
                {
                    int64_t k = Ai [pa] ;
                    int8_t aki = Ax [pa] ;          // aki = A(k,i)
                    int8_t bkj = Bx [pb + k] ;      // bkj = B(k,j)
                    bool t = aki != bkj ;
                    cij = (cij != t) ;
                }

            }
            else if (ainz > 32 * bjnz)
            {

                //--------------------------------------------------------------
                // B(:,j) is very sparse compared to A(:,i)
                //--------------------------------------------------------------

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

                //--------------------------------------------------------------
                // A(:,i) is very sparse compared to B(:,j)
                //--------------------------------------------------------------

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

                //--------------------------------------------------------------
                // A(:,i) and B(:,j) have about the same sparsity
                //--------------------------------------------------------------

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
                    else // ia == ib == k
                    {
                        // A(k,i) and B(k,j) are the next entries to merge
                        MERGE ;
                    }
                }
            }

            if (cij_exists)
            {
                // C(i,j) = cij
                Cx [cnz] = cij ;
                Ci [cnz++] = i ;
            }
        }
    }
    // log the end of the last column
    Cp [n] = cnz ;
}

#undef MERGE

#endif
