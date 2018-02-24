//------------------------------------------------------------------------------
// GB_AxB_numeric: compute the values of C = A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = A*B on a semiring

// On input, C must be of the right dimensions for C=A*B.  If Mask is NULL, it
// must contain the exact pattern of C=A*B, computed by GB_AxB_symbolic.  On
// input, the C->x array is freed if present.  C->type must be exactly the same
// as the semiring monoid (semiring->add->op->ztype ==
// semiring->multipy->ztype).  This is not a user-callable function.

// flipxy: if true, then compute z = fmult (B(k,j), A(i,k)) instead of the
// usual z = fmult (A(i,k), B(k,j)), since A and B have been swapped on input.
// This option is used when C=A'*B' is computed via C=(B*A)' by GrB_mxm, and
// for all uses of GrB_vxm.

// If the Mask is not NULL, then the pattern of C has not been computed.  C->p
// and C->i are only allocated.  This function constucts the pattern of C as
// the same as the Mask, but with zombies if an entry appears in the Mask but
// not in A*B.

// FUTURE: this can be done in parallel.  The computation of each column C(:,j)
// is an independent task.  Each thread would need its own Flag and Work array.

#include "GB.h"
#include "GB_AxB_methods.h"

GrB_Info GB_AxB_numeric             // compute the values of C = A*B
(
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix Mask,          // Mask matrix for C<M> (not complemented)
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const bool flops_are_low        // true if flop count is very low
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (Mask == NULL)
    {
        // C contains the pattern of C=A*B
        ASSERT_OK (GB_check (C, "C input for numeric C=A*B", 0)) ;
    }
    else
    {
        // Mask is present.  C->p and C->i are allocated but not initialized
        ASSERT (C != NULL && C->p != NULL && C->i != NULL) ;
        ASSERT (!C->p_shallow && !C->i_shallow) ;
        ASSERT_OK (GB_check (Mask, "Mask for numeric C<M>=A*B", 0)) ;
    }
    ASSERT_OK (GB_check (A, "A for numeric A*B", 0)) ;
    ASSERT_OK (GB_check (B, "B for numeric A*B", 0)) ;
    ASSERT (!PENDING (Mask)) ; ASSERT (!ZOMBIES (Mask)) ;
    ASSERT (!PENDING (C)) ; ASSERT (!ZOMBIES (C)) ;
    ASSERT (!PENDING (A)) ; ASSERT (!ZOMBIES (A)) ;
    ASSERT (!PENDING (B)) ; ASSERT (!ZOMBIES (B)) ;
    ASSERT_OK (GB_check (semiring, "semiring for numeric A*B", 0)) ;
    ASSERT (A->ncols == B->nrows) ;
    ASSERT (C->nrows == A->nrows && C->ncols == B->ncols) ;

    if (flops_are_low)
    {
        // If the flop count is low, the Mask was not used as a proxy for the
        // symbolic analysis, because it could be dense compared with C.
        // Instead, the entire symbolic analsys was done, and the Mask was not
        // passed to GB_AxB_symbolic and thus must not passed to this function.
        ASSERT (Mask == NULL) ;
    }

    //--------------------------------------------------------------------------
    // get functions and type sizes
    //--------------------------------------------------------------------------

    // get the semiring operators
    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Monoid add = semiring->add ;

    // these conditions have already been checked in the caller
    if (flipxy)
    {
        // z = fmult (b,a) will be computed
        ASSERT (GB_Type_compatible (A->type, mult->ytype)) ;
        ASSERT (GB_Type_compatible (B->type, mult->xtype)) ;
    }
    else
    {
        // z = fmult (a,b) will be computed
        ASSERT (GB_Type_compatible (A->type, mult->xtype)) ;
        ASSERT (GB_Type_compatible (B->type, mult->ytype)) ;
    }
    ASSERT (C->type == mult->ztype && mult->ztype == add->op->ztype) ;

    //--------------------------------------------------------------------------
    // allocate C->x and ensure workspace is large enough
    //--------------------------------------------------------------------------

    size_t zsize = mult->ztype->size ;

    char zwork [zsize] ;
    char cwork [zsize] ;

    double memory = GBYTES (C->nzmax + A->nrows+1, zsize) ;

    // free C->x unless it is shallow, then reallocate it at the right size
    if (!C->x_shallow)
    {
        // C->x is normally NULL already so this should do nothing
        GB_FREE_MEMORY (C->x, C->nzmax, zsize) ;
    }
    GB_MALLOC_MEMORY (C->x, C->nzmax, zsize) ;
    C->x_shallow = false ;

    bool ok = GB_Work_alloc (A->nrows, zsize) ;

    int8_t *Flag = NULL ;
    if (Mask != NULL)
    {
        // allocate Flag
        ok = ok && GB_Flag_alloc (A->nrows) ;
    }

    if (C->x == NULL || !ok)
    {
        GB_Matrix_clear (C) ;
        GB_Flag_free ( ) ;
        GB_Work_free ( ) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory))) ;
    }

    // w has size A->nrows+1, each entry of size zsize.  It is not initialized.
    void *w = GB_thread_local.Work ;

    int64_t n = C->ncols ;

    // get the Flag array and ensure that is cleared
    if (Mask != NULL)
    {
        ASSERT_FLAG_IS_CLEAR ;
        Flag = GB_thread_local.Flag ;
        ASSERT (Mask->nrows == C->nrows && Mask->ncols == C->ncols) ;
    }

    //--------------------------------------------------------------------------
    // determine the type of A2 and B2
    //--------------------------------------------------------------------------

    GrB_Matrix A2 = NULL ;
    GrB_Matrix B2 = NULL ;
    GrB_Type atype_required, btype_required ;

    if (flipxy)
    {
        // A is passed as y, and B as x, in z = mult(x,y)
        atype_required = mult->ytype ;
        btype_required = mult->xtype ;
    }
    else
    {
        // A is passed as x, and B as y, in z = mult(x,y)
        atype_required = mult->xtype ;
        btype_required = mult->ytype ;
    }

    //--------------------------------------------------------------------------
    // generic C = A*B for any valid semiring, with typecasting on the fly
    //--------------------------------------------------------------------------

    // If the flop count is low, then it is likely not all of A needs to be
    // typecasted.  In this case, only typecast the part of A that must be
    // typecasted, and do so on the fly.  This method is slower than
    // GB_AxB_builtin if the flop count is high (typically about 5 or 6 times
    // slower) but it is much faster than typecasting all of A when the flop
    // count is very low (such as A*b when b is a very sparse column).

    // This method does not exploit the mask, for the sake of efficiency.  If
    // the flop count is low, then it is possible that the pattern of C=A*B is
    // much sparser than the Mask itself, even asymptotically smaller.  So
    // trying to exploit the Mask here would defeat the purpose of this method.
    // With a low flop count, the full symbolic analysis becomes very cheap.

    // As a result, if the flop count is low, Mask must be NULL both here and
    // in the symbolic analysis.

    bool a_cast = atype_required->code != A->type->code ;
    bool b_cast = btype_required->code != B->type->code ;

    if (flops_are_low && (a_cast || b_cast))
    {
        ASSERT (Mask == NULL) ;

        GB_cast_function cast_A = NULL, cast_B = NULL ;

        GB_binary_function fmult = mult->function ;
        GB_binary_function fadd  = add->op->function ;

        const int64_t *Ap = A->p ;
        const int64_t *Ai = A->i ;
        const int64_t *Bp = B->p ;
        const int64_t *Bi = B->i ;
        const int64_t *Cp = C->p ;
        const int64_t *Ci = C->i ;

        void *Cx = C->x ;
        const void *Ax = A->x ;
        const void *Bx = B->x ;

        void *identity = add->identity ;

        // a2size is the size of x, or y if flipxy is true, for z=mult(x,y)
        // b2size is the size of y, or x if flipxy is true, for z=mult(x,y)
        size_t a2size = atype_required->size ;
        size_t b2size = btype_required->size ;
        size_t a1size = A->type->size ;
        size_t b1size = B->type->size ;

        char bkj [b2size] ;
        char aik [a2size] ;

        cast_A = GB_cast_factory (atype_required->code, A->type->code) ;

        if (b_cast)
        {
            cast_B = GB_cast_factory (btype_required->code, B->type->code) ;
        }

        for (int64_t j = 0 ; j < n ; j++)
        {
            // clear w
            for (int64_t p = Cp [j] ; p < Cp [j+1] ; p++)
            {
                // w [Ci [p]] = identity ;
                memcpy (w +((Ci [p])*zsize), identity, zsize) ;
            }
            // compute C(:,j) = A * B(:,j)
            for (int64_t pb = Bp [j] ; pb < Bp [j+1] ; pb++)
            {
                // B(k,j) is present
                int64_t k = Bi [pb] ;
                int64_t pa1 = Ap [k] ;
                int64_t pa2 = Ap [k+1] ;
                if (pa2 == pa1) continue ;
                // bkj = B(k,j)
                if (b_cast)
                {
                    cast_B (bkj, Bx +(pb*b1size), b1size) ;
                }
                else
                {
                    memcpy (bkj, Bx +(pb*b1size), b1size) ;
                }
                for (int64_t pa = pa1 ; pa < pa2 ; pa++)
                {
                    // w [i] += A(i,k) * B(k,j)
                    int64_t i = Ai [pa] ;
                    cast_A (aik, Ax +(pa*a1size), a1size) ;
                    if (flipxy)
                    {
                        // zwork = bkj * A(i,k)
                        fmult (zwork, bkj, aik) ;
                    }
                    else
                    {
                        // zwork = A(i,k) * bkj
                        fmult (zwork, aik, bkj) ;
                    }
                    // cwork = w [i]
                    memcpy (cwork, w +(i*zsize), zsize) ;
                    // w [i] = cwork + zwork
                    fadd (w +(i*zsize), cwork, zwork) ;
                }
            }
            // gather C(:,j)
            for (int64_t p = Cp [j] ; p < Cp [j+1] ; p++)
            {
                // Cx [p] = w [Ci [p]] ;
                memcpy (Cx +(p*zsize), w +((Ci [p])*zsize), zsize) ;
            }
        }
        ASSERT_OK (GB_check (C, "C output for numeric C=A*B", 0)) ;
        return (REPORT_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // cast A and B to x and y for z=mult(x,y), if needed
    //--------------------------------------------------------------------------

    // Shallow casting creates a new matrix A2 that has shallow pointers to
    // A->p and A->i, but it constructs a new array A2->x for the numerical
    // values.  The types A->type and A2->type differ, as does A->x and A2->x,
    // but all other content is the same.  If the types of A and A2 are the
    // same, then A2->x is a shallow copy of A->x and no data is moved.

    // shallow copy A2 = A.  Shallow copy of A2->p and A2->i, and create A2->x
    // if needed.  If A is already of the right type, then no work is done
    // except for allocating the tiny struct of A2.  Then A2->x is a pointer
    // copy of A->x.
    GrB_Info info = GB_shallow_cast (&A2, atype_required, A) ;
    if (info != GrB_SUCCESS)
    {
        GB_Matrix_clear (C) ;
        GB_Flag_free ( ) ;
        GB_Work_free ( ) ;
        return (info) ;
    }

    // also shallow copy B2 = B
    info = GB_shallow_cast (&B2, btype_required, B) ;
    if (info != GrB_SUCCESS)
    {
        GB_Matrix_clear (C) ;
        GB_MATRIX_FREE (&A2) ;
        GB_Flag_free ( ) ;
        GB_Work_free ( ) ;
        return (info) ;
    }

    // A2 and B2 are now the right types for the multiply operator.
    // no further typecasting is needed.
    ASSERT (A2->type == atype_required) ;
    ASSERT (B2->type == btype_required) ;

    //--------------------------------------------------------------------------
    // compute C = A2*B2 for built-in types and operators
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (A2->type, "A2 type for builtin", 0)) ;
    ASSERT_OK (GB_check (B2->type, "B2 type for builtin", 0)) ;
    ASSERT_OK (GB_check (C->type, "C type for builtin", 0)) ;
    ASSERT_OK (GB_check (semiring, "semiring for builtin", 0)) ;

#ifndef GBCOMPACT

    // If the GB_AxB_builtin function has a worker for the particular semiring,
    // then it does the computation and returns true.  Otherwise, it returns
    // false, and the generic worker below does the work.

    // If GBCOMPACT is enabled at compile-time, then no built-in workers are
    // created, and this function is not used.  All C=A*B computations are done
    // with the generic worker below.

    if (GB_AxB_builtin (C, Mask, A2, B2, semiring, flipxy))
    {
        // C = A2*B2 has been done via a hard-coded case; free memory and return
        GB_MATRIX_FREE (&A2) ;
        GB_MATRIX_FREE (&B2) ;
        if (Mask) ASSERT_FLAG_IS_CLEAR ;
        return (REPORT_SUCCESS) ;
    }

#endif

    //--------------------------------------------------------------------------
    // generic C = A2*B2 for any valid semiring, built-in or user-defined
    //--------------------------------------------------------------------------

    // compare this code with the AxB macro in GB_AxB_builtin

    GB_binary_function fmult = mult->function ;
    GB_binary_function fadd  = add->op->function ;

    const int64_t *restrict Ap = A2->p ;
    const int64_t *restrict Ai = A2->i ;
    const int64_t *restrict Bp = B2->p ;
    const int64_t *restrict Bi = B2->i ;

    void *restrict Cx = C->x ;
    const void *restrict Ax = A2->x ;
    const void *restrict Bx = B2->x ;

    void *restrict identity = add->identity ;

    // asize is the size of x, or y if flipxy is true, for z=mult(x,y)
    // bsize is the size of y, or x if flipxy is true, for z=mult(x,y)
    size_t asize = atype_required->size ;
    size_t bsize = btype_required->size ;

    if (Mask != NULL)
    {

        //----------------------------------------------------------------------
        // C<M> = A*B, using Mask as a superset of the symbolic pattern of C
        //----------------------------------------------------------------------

        // The pattern of C and Mask are the same, except that the Mask has no
        // zombies but C may have them.  Entries in the Mask but not in A*B
        // become zombies in C.

        // get cast function for casting Mask(i,j) from current type to boolean
        GB_cast_function cast_Mask =
            GB_cast_factory (GB_BOOL_code, Mask->type->code) ;

        const int64_t *restrict Maski = Mask->i ;
        const int64_t *restrict Maskp = Mask->p ;
        const void    *restrict Maskx = Mask->x ;
        size_t msize = Mask->type->size ;

        char bkj [bsize] ;

        int64_t *restrict Ci = C->i ;

        // copy Maskp into C->p
        memcpy (C->p, Maskp, (n+1) * sizeof (int64_t)) ;
        C->magic = MAGIC ;

        for (int64_t j = 0 ; j < n ; j++)
        {
            // scatter Mask(:,j) into Flag
            bool marked = false ;
            scatter_mask (j, Maskp, Maski, Maskx, msize, cast_Mask, Flag,
                &marked) ;
            // compute C(:,j) = A * B(:,j), both values and pattern
            for (int64_t p = Bp [j] ; p < Bp [j+1] ; p++)
            {
                // B(k,j) is present
                int64_t k = Bi [p] ;
                memcpy (bkj, Bx +(p*bsize), bsize) ;
                for (int64_t pa = Ap [k] ; pa < Ap [k+1] ; pa++)
                {
                    // w [i] += (A(i,k) * B(k,j)) .* Mask(i,j)
                    int64_t i = Ai [pa] ;
                    int8_t flag = Flag [i] ;
                    if (flag == 0) continue ;
                    // Mask(i,j) == 1 so do the work
                    if (flipxy)
                    {
                        // zwork = bkj * A(i,k)
                        fmult (zwork, bkj, Ax +(pa*asize)) ;
                    }
                    else
                    {
                        // zwork = A(i,k) * bkj
                        fmult (zwork, Ax +(pa*asize), bkj) ;
                    }
                    if (flag > 0)
                    {
                        // first time C(i,j) seen
                        Flag [i] = -1 ;
                        // w [i] = zwork
                        memcpy (w +(i*zsize), zwork, zsize) ;
                    }
                    else
                    {
                        // C(i,j) seen before, update it
                        // cwork = w [i]
                        memcpy (cwork, w +(i*zsize), zsize) ;
                        // w [i] = cwork + zwork
                        fadd (w +(i*zsize), cwork, zwork) ;
                    }
                }
            }
            // gather C(:,j), both values and pattern, from the Mask(:,j)
            for (int64_t p = Maskp [j] ; p < Maskp [j+1] ; p++)
            {
                int64_t i = Maski [p] ;
                // C(i,j) is present
                if (Flag [i] < 0)
                {
                    // C(i,j) is a live entry, gather its row and value
                    // Cx [p] = w [i] ;
                    memcpy (Cx +(p*zsize), w +(i*zsize), zsize) ;
                    Ci [p] = i ;
                }
                else
                {
                    // C(i,j) is a zombie; in the Mask but not in A*B
                    // Cx [p] left uninitialized, or this could be done:
                    // memcpy (Cx +(p*zsize), identity, zsize) ;
                    Ci [p] = FLIP (i) ;
                    C->nzombies++ ;
                }
                Flag [i] = 0 ;
            }
        }
        ASSERT_FLAG_IS_CLEAR ;
        ASSERT (ZOMBIES_OK (C)) ;
        GB_queue_insert (C) ;
        ASSERT_OK (GB_check (C, "C<M> = A*B, with built-in mask", 0)) ;

    }
    else
    {

        //----------------------------------------------------------------------
        // C = A*B, using pattern of C computed by GB_AxB_symbolic
        //----------------------------------------------------------------------

        char bkj [bsize] ;

        const int64_t *restrict Cp = C->p ;
        const int64_t *restrict Ci = C->i ;

        for (int64_t j = 0 ; j < n ; j++)
        {
            // clear w
            for (int64_t p = Cp [j] ; p < Cp [j+1] ; p++)
            {
                // w [Ci [p]] = identity ;
                memcpy (w +((Ci [p])*zsize), identity, zsize) ;
            }
            // compute C(:,j) = A * B(:,j)
            for (int64_t p = Bp [j] ; p < Bp [j+1] ; p++)
            {
                // B(k,j) is present
                int64_t k = Bi [p] ;
                memcpy (bkj, Bx +(p*bsize), bsize) ;
                for (int64_t pa = Ap [k] ; pa < Ap [k+1] ; pa++)
                {
                    // w [i] += A(i,k) * B(k,j)
                    int64_t i = Ai [pa] ;
                    if (flipxy)
                    {
                        // zwork = bkj * A(i,k)
                        fmult (zwork, bkj, Ax +(pa*asize)) ;
                    }
                    else
                    {
                        // zwork = A(i,k) * bkj
                        fmult (zwork, Ax +(pa*asize), bkj) ;
                    }
                    // cwork = w [i]
                    memcpy (cwork, w +(i*zsize), zsize) ;
                    // w [i] = cwork + zwork
                    fadd (w +(i*zsize), cwork, zwork) ;
                }
            }
            // gather C(:,j)
            for (int64_t p = Cp [j] ; p < Cp [j+1] ; p++)
            {
                // Cx [p] = w [Ci [p]] ;
                memcpy (Cx +(p*zsize), w +((Ci [p])*zsize), zsize) ;
            }
        }
        ASSERT (!ZOMBIES (C)) ;
    }

    //--------------------------------------------------------------------------
    // free workspace
    //--------------------------------------------------------------------------

    GB_MATRIX_FREE (&A2) ;
    GB_MATRIX_FREE (&B2) ;

    ASSERT_OK (GB_check (C, "C output for numeric C=A*B", 0)) ;
    return (REPORT_SUCCESS) ;
}

