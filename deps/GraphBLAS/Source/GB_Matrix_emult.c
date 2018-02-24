//------------------------------------------------------------------------------
// GB_Matrix_emult: element-wise "multiplication" of two matrices
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_Matrix_emult (C, A, B, op), applies an operator C = op (A,B)
// element-wise on the matrices A and B.  The result is typecasted as needed.
//
// Let the op be z=f(x,y) where x, y, and z have type xtype, ytype, and ztype.
// If both A(i,j) and B(i,j) are present, then:
//
//      C(i,j) = (ctype) op ((xtype) A(i,j), (ytype) B(i,j))
//
// If just A(i,j) is present but not B(i,j), then:
//
//      C(i,j) is not present, and is implicitly 'zero'
//
// If just B(i,j) is present but not A(i,j), then:
//
//      C(i,j) is not present, and is implicitly 'zero'
//
// ctype is the type of matrix C.  Its pattern is the intersection of A and B.
//
// This function should not be called by the end user.  It is a helper function
// for user-callable routines.  No error checking is performed except for
// out-of-memory conditions.

#include "GB.h"

GrB_Info GB_Matrix_emult    // C = A.*B
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

    // [ [ C need not be initialized, just the column pointers present
    ASSERT (C != NULL && C->p != NULL && !C->p_shallow) ;
    ASSERT_OK (GB_check (A, "A for C=A.*B", 0)) ;
    ASSERT_OK (GB_check (B, "B for C=A.*B", 0)) ;
    ASSERT (!PENDING (A)) ; ASSERT (!ZOMBIES (A)) ;
    ASSERT (!PENDING (B)) ; ASSERT (!ZOMBIES (B)) ;
    ASSERT_OK (GB_check (op, "op for C=A.*B", 0)) ;
    ASSERT (C->ncols == A->ncols && C->nrows == A->nrows) ;
    ASSERT (C->ncols == B->ncols && C->nrows == B->nrows) ;

    ASSERT (GB_Type_compatible (C->type, op->ztype)) ;
    ASSERT (GB_Type_compatible (A->type, op->xtype)) ;
    ASSERT (GB_Type_compatible (B->type, op->ytype)) ;

    //--------------------------------------------------------------------------
    // allocate the output matrix C
    //--------------------------------------------------------------------------

    double memory = 0 ;
    if (!GB_Matrix_alloc (C, IMIN (NNZ (A), NNZ (B)), true, &memory))
    {
        // out of memory
        GB_Matrix_clear (C) ;
        // C->p is now initialized ]
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory))) ;
    }

    ASSERT (!PENDING (C)) ;

    //--------------------------------------------------------------------------
    // get functions and type sizes
    //--------------------------------------------------------------------------

    GB_cast_function cast_A_to_X, cast_B_to_Y, cast_Z_to_C ;

    cast_A_to_X = GB_cast_factory (op->xtype->code, A->type->code) ;
    cast_B_to_Y = GB_cast_factory (op->ytype->code, B->type->code) ;
    cast_Z_to_C = GB_cast_factory (C->type->code,   op->ztype->code) ;

    // If types are user-defined, the cast* function is just
    // GB_copy_user_user, which requires the size of the type.  No typecast is
    // done.

    GB_binary_function fmult = op->function ;

    size_t xsize = op->xtype->size ;
    size_t ysize = op->ytype->size ;
    size_t zsize = op->ztype->size ;
    size_t asize = A->type->size ;
    size_t bsize = B->type->size ;
    size_t csize = C->type->size ;

    // no typecasting needed if all the types match the operator
    bool nocasting =
        (A->type->code == op->xtype->code) &&
        (B->type->code == op->ytype->code) &&
        (C->type->code == op->ztype->code) ;

    // scalar workspace
    char xwork [nocasting ? 1 : xsize] ;
    char ywork [nocasting ? 1 : ysize] ;
    char zwork [nocasting ? 1 : zsize] ;

    //--------------------------------------------------------------------------
    // C = A .* B, where .*+ is defined by z=fmult(x,y)
    //--------------------------------------------------------------------------

    const int64_t *Ap = A->p ;
    const int64_t *Ai = A->i ;
    const int64_t *Bp = B->p ;
    const int64_t *Bi = B->i ;
    int64_t *Cp = C->p ;
    int64_t *Ci = C->i ;
    int64_t cnz = 0 ;

    const void *Ax = A->x ;
    const void *Bx = B->x ;
    void *Cx = C->x ;

    for (int64_t j = 0 ; j < A->ncols ; j++)
    {

        //----------------------------------------------------------------------
        // construct C (:,j)
        //----------------------------------------------------------------------

        Cp [j] = cnz ;
        int64_t pa = Ap [j] ;
        int64_t pb = Bp [j] ;
        int64_t pa_end = Ap [j+1] ;
        int64_t pb_end = Bp [j+1] ;

        int64_t ajnz = pa_end - pa ;
        int64_t bjnz = pb_end - pb ;

        //----------------------------------------------------------------------
        // quick check to see if A (:,j) and B (:,j) do not overlap
        //----------------------------------------------------------------------

        if (ajnz == 0 || bjnz == 0)
        {
            // one or both columns are empty; set intersection is empty
            continue ;
        }

        if (Ai [pa_end-1] < Bi [pb])
        {
            // all entries in A are in lower row indices than all the
            // entries in B; set intersection is empty
            continue ;
        }

        if (Bi [pb_end-1] < Ai [pa])
        {
            // all entries in B are in lower row indices than all the
            // entries in A; set intersection is empty
            continue ;
        }

        //----------------------------------------------------------------------
        // range of row indices in A (:,j) and B (:,j) overlap
        //----------------------------------------------------------------------

        // C (i,j) = fmult (A (i,j), B (i,j))
        #define MULTIPLY                                                    \
        {                                                                   \
            Ci [cnz] = ib ;                                                 \
            if (nocasting)                                                  \
            {                                                               \
                fmult (Cx +(cnz*csize), Ax +(pa*asize), Bx +(pb*bsize)) ;   \
            }                                                               \
            else                                                            \
            {                                                               \
                /* xwork = (xtype) Ax [pa] */                               \
                cast_A_to_X (xwork, Ax +(pa*asize), asize) ;                \
                /* ywork = (ytype) Bx [pa] */                               \
                cast_B_to_Y (ywork, Bx +(pb*bsize), bsize) ;                \
                /* zwork = fmult (xwork, ywork), result is ztype */         \
                fmult (zwork, xwork, ywork) ;                               \
                /* Cx [cnz] = (ctype) zwork */                              \
                cast_Z_to_C (Cx +(cnz*csize), zwork, csize) ;               \
            }                                                               \
            pa++ ;                                                          \
            pb++ ;                                                          \
            cnz++ ;                                                         \
        }

        if (ajnz > 256 * bjnz)
        {

            //------------------------------------------------------------------
            // A (:,j) has many more nonzeros than B (:,j)
            //------------------------------------------------------------------

            for ( ; pa < pa_end && pb < pb_end ; )
            {
                int64_t ia = Ai [pa] ;
                int64_t ib = Bi [pb] ;
                if (ia < ib)
                {
                    // A (ia,j) appears before B (ib,j)
                    // discard all entries A (ia:ib-1,j)
                    int64_t pleft = pa + 1 ;
                    int64_t pright = pa_end ;
                    GB_BINARY_TRIM_SEARCH (ib, Ai, pleft, pright) ;
                    ASSERT (pleft > pa) ;
                    pa = pleft ;
                }
                else if (ia > ib)
                {
                    // B (ib,j) appears before A (ia,j)
                    pb++ ;
                }
                else // ia == ib
                {
                    // A (i,j) and B (i,j) match
                    MULTIPLY ;
                }
            }

        }
        else if (bjnz > 256 * ajnz)
        {

            //------------------------------------------------------------------
            // B (:,j) has many more nonzeros than A (:,j)
            //------------------------------------------------------------------

            for ( ; pa < pa_end && pb < pb_end ; )
            {
                int64_t ia = Ai [pa] ;
                int64_t ib = Bi [pb] ;
                if (ia < ib)
                {
                    // A (ia,j) appears before B (ib,j)
                    pa++ ;
                }
                else if (ia > ib)
                {
                    // B (ib,j) appears before A (ia,j)
                    // discard all entries B (ib:ia-1,j)
                    int64_t pleft = pb + 1 ;
                    int64_t pright = pb_end ;
                    GB_BINARY_TRIM_SEARCH (ia, Bi, pleft, pright) ;
                    ASSERT (pleft > pb) ;
                    pb = pleft ;
                }
                else // ia == ib
                {
                    // A (i,j) and B (i,j) match
                    MULTIPLY ;
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // A (:,j) and B (:,j) have about the same number of entries
            //------------------------------------------------------------------

            for ( ; pa < pa_end && pb < pb_end ; )
            {
                int64_t ia = Ai [pa] ;
                int64_t ib = Bi [pb] ;
                if (ia < ib)
                {
                    // A (ia,j) appears before B (ib,j)
                    pa++ ;
                }
                else if (ia > ib)
                {
                    // B (ib,j) appears before A (ia,j)
                    pb++ ;
                }
                else // ia == ib
                {
                    // A (i,j) and B (i,j) match
                    MULTIPLY ;
                }
            }
        }
    }

    Cp [C->ncols] = cnz ;
    C->magic = MAGIC ;      // C->p now initialized ]

    //--------------------------------------------------------------------------
    // trim the size of C: this cannot fail
    //--------------------------------------------------------------------------

    ASSERT (cnz <= C->nzmax) ;
    bool ok = GB_Matrix_realloc (C, cnz, true, NULL) ;
    ASSERT (ok) ;
    ASSERT_OK (GB_check (C, "C output for C=A.*B", 0)) ;
    return (REPORT_SUCCESS) ;
}

#undef MULTIPLY

