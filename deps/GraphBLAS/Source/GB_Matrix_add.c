//------------------------------------------------------------------------------
// GB_Matrix_add: 'add' two matrices using an operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_Matrix_add (C, A, B, op), 'adds' C = op (A,B), using the given operator
// element-wise on the matrices A and B.  The result is typecasted as needed.
// The pattern of C is the union of the pattern of A and B.
//
// Let the op be z=f(x,y) where x, y, and z have type xtype, ytype, and ztype.
// If both A(i,j) and B(i,j) are present, then:
//
//      C(i,j) = (ctype) op ((xtype) A(i,j), (ytype) B(i,j))
//
// If just A(i,j) is present but not B(i,j), then:
//
//      C(i,j) = (ctype) A (i,j)
//
// If just B(i,j) is present but not A(i,j), then:
//
//      C(i,j) = (ctype) B (i,j)
//
// ctype is the type of matrix C.  The pattern of C is the union of A and B.
//
// This function should not be called by the end user.  It is a helper function
// for user-callable routines.  No error checking is performed except for
// out-of-memory conditions.

#include "GB.h"

GrB_Info GB_Matrix_add      // C = A+B
(
    GrB_Matrix C,           // output matrix
    const GrB_Matrix A,     // input A matrix
    const GrB_Matrix B,     // input B matrix
    const GrB_BinaryOp op   // op to perform C = op (A,B)
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // [ C need not be initialized, just the column pointers present
    ASSERT (C != NULL && C->p != NULL && !C->p_shallow) ;
    ASSERT_OK (GB_check (A, "A for C=A+B", 0)) ;
    ASSERT_OK (GB_check (B, "B for C=A+B", 0)) ;
    ASSERT_OK (GB_check (op, "op for C=A+B", 0)) ;
    ASSERT (!PENDING (A)) ; ASSERT (!ZOMBIES (A)) ;
    ASSERT (!PENDING (B)) ; ASSERT (!ZOMBIES (B)) ;
    ASSERT (C->ncols == A->ncols && C->nrows == A->nrows) ;
    ASSERT (C->ncols == B->ncols && C->nrows == B->nrows) ;

    ASSERT (GB_Type_compatible (C->type, op->ztype)) ;
    ASSERT (GB_Type_compatible (C->type, A->type)) ;
    ASSERT (GB_Type_compatible (C->type, B->type)) ;
    ASSERT (GB_Type_compatible (A->type, op->xtype)) ;
    ASSERT (GB_Type_compatible (B->type, op->ytype)) ;

    //--------------------------------------------------------------------------
    // allocate the output matrix C
    //--------------------------------------------------------------------------

    // safe against integer overflow since both values are <= GB_INDEX_MAX
    double memory = 0 ;
    if (!GB_Matrix_alloc (C, NNZ (A) + NNZ (B), true, &memory))
    {
        // out of memory
        GB_Matrix_clear (C) ;           // C is now initialized, just empty
        ASSERT_OK (GB_check (C, "C cleared", 0)) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory))) ;
    }

    ASSERT (!PENDING (C)) ;

    //--------------------------------------------------------------------------
    // two generic workers for C = A '+' B
    //--------------------------------------------------------------------------

    // If types are user-defined, the cast* function is just GB_copy_user_user,
    // which requires the size of the type.  No typecast is done.

    GB_binary_function fadd = op->function ;

    const int64_t *Ap = A->p ;
    const int64_t *Ai = A->i ;
    const void    *Ax = A->x ;

    const int64_t *Bp = B->p ;
    const int64_t *Bi = B->i ;
    const void    *Bx = B->x ;

    int64_t *Cp = C->p ;
    int64_t *Ci = C->i ;
    void    *Cx = C->x ;

    int64_t cnz = 0 ;
    int64_t ncols = A->ncols ;

    // check if no typecasting is needed for the operator
    bool nocasting =
        (A->type->code == op->xtype->code) &&
        (B->type->code == op->ytype->code) &&
        (C->type->code == op->ztype->code) ;

    if (nocasting && A->type == C->type && B->type == C->type)
    {

        //----------------------------------------------------------------------
        // C = A + B, no typecasting at all, all types the same
        //----------------------------------------------------------------------

        size_t s = C->type->size ;

        for (int64_t j = 0 ; j < ncols ; j++)
        {

            //------------------------------------------------------------------
            // log the start of column j
            //------------------------------------------------------------------

            Cp [j] = cnz ;
            int64_t pa = Ap [j] ;
            int64_t pb = Bp [j] ;
            int64_t pa_end = Ap [j+1] ;
            int64_t pb_end = Bp [j+1] ;

            //------------------------------------------------------------------
            // merge A (:,j) and B (:,j) while both have entries
            //------------------------------------------------------------------

            while (pa < pa_end && pb < pb_end)
            {
                // both A(ia,j) and B (ib,j) are at head of lists to merge
                int64_t ia = Ai [pa] ;
                int64_t ib = Bi [pb] ;
                if (ia < ib)
                {
                    // C (ia:ib-1,j) = A (ia:ib-1,j)
                    int64_t pa2 = pa ;
                    do
                    {
                        pa2++ ;
                    }
                    while (pa2 < pa_end && Ai [pa2] < ib) ;
                    int64_t alen = pa2 - pa ;
                    memcpy (&Ci [cnz  ], &Ai [pa  ], alen * sizeof (int64_t)) ;
                    memcpy (Cx +(cnz*s), Ax +(pa*s), alen * s) ;
                    pa  += alen ;
                    cnz += alen ;
                }
                else if (ib < ia)
                {
                    // C (ib:ia-1,j) = B (ib:ia-1,j)
                    int64_t pb2 = pb ;
                    do
                    {
                        pb2++ ;
                    }
                    while (pb2 < pb_end && Bi [pb2] < ia) ;
                    int64_t blen = pb2 - pb ;
                    memcpy (&Ci [cnz  ], &Bi [pb  ], blen * sizeof (int64_t)) ;
                    memcpy (Cx +(cnz*s), Bx +(pb*s), blen * s) ;
                    pb  += blen ;
                    cnz += blen ;
                }
                else // ia == ib == i
                {
                    // C (i,j) = fadd (A (i,j), B (i,j))
                    Ci [cnz] = ib ;
                    fadd (Cx +(cnz*s), Ax +(pa*s), Bx +(pb*s)) ;
                    pa++ ;
                    pb++ ;
                    cnz++ ;
                }
            }

            //------------------------------------------------------------------
            // A (:,j) or B (:,j) have entries left; not both
            //------------------------------------------------------------------

            if (pa < pa_end)
            {
                int64_t alen = pa_end - pa ;
                memcpy (&Ci [cnz  ], &Ai [pa  ], alen * sizeof (int64_t)) ;
                memcpy (Cx +(cnz*s), Ax +(pa*s), alen * s) ;
                cnz += alen ;
            }
            else if (pb < pb_end)
            {
                int64_t blen = pb_end - pb ;
                memcpy (&Ci [cnz  ], &Bi [pb  ], blen * sizeof (int64_t)) ;
                memcpy (Cx +(cnz*s), Bx +(pb*s), blen * s) ;
                cnz += blen ;
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // C = A + B, with any typecasting
        //----------------------------------------------------------------------

        size_t csize = C->type->size ;
        size_t asize = A->type->size ;
        size_t bsize = B->type->size ;

        // scalar workspace
        char xwork [nocasting ? 1 : op->xtype->size] ;
        char ywork [nocasting ? 1 : op->ytype->size] ;
        char zwork [nocasting ? 1 : op->ztype->size] ;

        GB_cast_function
            cast_A_to_X, cast_B_to_Y, cast_A_to_C, cast_B_to_C, cast_Z_to_C ;
        cast_A_to_X = GB_cast_factory (op->xtype->code, A->type->code) ;
        cast_B_to_Y = GB_cast_factory (op->ytype->code, B->type->code) ;
        cast_A_to_C = GB_cast_factory (C->type->code,   A->type->code) ;
        cast_B_to_C = GB_cast_factory (C->type->code,   B->type->code) ;
        cast_Z_to_C = GB_cast_factory (C->type->code,   op->ztype->code) ;

        for (int64_t j = 0 ; j < ncols ; j++)
        {

            //------------------------------------------------------------------
            // log the start of column j
            //------------------------------------------------------------------

            Cp [j] = cnz ;
            int64_t pa = Ap [j] ;
            int64_t pb = Bp [j] ;
            int64_t pa_end = Ap [j+1] ;
            int64_t pb_end = Bp [j+1] ;

            //------------------------------------------------------------------
            // merge A (:,j) and B (:,j) while both have entries
            //------------------------------------------------------------------

            for ( ; pa < pa_end && pb < pb_end ; cnz++)
            {
                int64_t ia = Ai [pa] ;
                int64_t ib = Bi [pb] ;
                if (ia < ib)
                {
                    // C (ia,j) = A (ia,j)
                    Ci [cnz] = ia ;
                    // Cx [cnz] = Ax [pa]
                    cast_A_to_C (Cx +(cnz*csize), Ax +(pa*asize), csize) ;
                    pa++ ;
                }
                else if (ia > ib)
                {
                    // C (ib,j) = B (ib,j)
                    Ci [cnz] = ib ;
                    // Cx [cnz] = Bx [pb]
                    cast_B_to_C (Cx +(cnz*csize), Bx +(pb*bsize), csize) ;
                    pb++ ;
                }
                else
                {
                    // C (i,j) = fadd (A (i,j), B (i,j))
                    Ci [cnz] = ib ;
                    if (nocasting)
                    {
                        // operator requires no typecasting
                        fadd (Cx +(cnz*csize), Ax +(pa*asize), Bx +(pb*bsize)) ;
                    }
                    else
                    {
                        // xwork = (xtype) Ax [pa]
                        cast_A_to_X (xwork, Ax +(pa*asize), asize) ;
                        // ywork = (ytype) Bx [pa]
                        cast_B_to_Y (ywork, Bx +(pb*bsize), bsize) ;
                        // zwork = fadd (xwork, ywork), result is ztype
                        fadd (zwork, xwork, ywork) ;
                        // Cx [cnz] = (ctype) zwork
                        cast_Z_to_C (Cx +(cnz*csize), zwork, csize) ;
                    }
                    pa++ ;
                    pb++ ;
                }
            }

            //------------------------------------------------------------------
            // A (:,j) or B (:,j) have entries left; not both
            //------------------------------------------------------------------

            for ( ; pa < pa_end ; pa++, cnz++)
            {
                // C (i,j) = A (i,j)
                Ci [cnz] = Ai [pa] ;
                // Cx [cnz] = (ctype) Ax [pa]
                cast_A_to_C (Cx +(cnz*csize), Ax +(pa*asize), csize) ;
            }
            for ( ; pb < pb_end ; pb++, cnz++)
            {
                // C (i,j) = B (i,j)
                Ci [cnz] = Bi [pb] ;
                // Cx [cnz] = (ctype) Bx [pb]
                cast_B_to_C (Cx +(cnz*csize), Bx +(pb*bsize), csize) ;
            }
        }
    }

    Cp [ncols] = cnz ;
    C->magic = MAGIC ;      // ] C is now initialized

    //--------------------------------------------------------------------------
    // finalize C and trim its size: this cannot fail
    //--------------------------------------------------------------------------

    bool ok = GB_Matrix_realloc (C, NNZ (C), true, NULL) ;
    ASSERT (ok) ;
    ASSERT_OK (GB_check (C, "C output for C=A+B", 0)) ;
    return (REPORT_SUCCESS) ;
}

