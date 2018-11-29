//------------------------------------------------------------------------------
// GB_emult: element-wise "multiplication" of two matrices
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_emult (C, A, B, op), applies an operator C = op (A,B)
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

// FUTURE: this could be faster with built-in operators and types.

// C (i,j) = fmult (A (i,j), B (i,j))
#define GB_EMULT                                                    \
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

#include "GB.h"

GrB_Info GB_emult           // C = A.*B
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    const GrB_Type ctype,   // type of output matrix C
    const bool C_is_csc,    // format of output matrix C
    const GrB_Matrix A,     // input A matrix
    const GrB_Matrix B,     // input B matrix
    const GrB_BinaryOp op,  // op to perform C = op (A,B)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;
    ASSERT_OK (GB_check (A, "A for C=A.*B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for C=A.*B", GB0)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT_OK (GB_check (op, "op for C=A.*B", GB0)) ;
    ASSERT (A->vdim == A->vdim && B->vlen == A->vlen) ;

    ASSERT (GB_Type_compatible (ctype,   op->ztype)) ;
    ASSERT (GB_Type_compatible (A->type, op->xtype)) ;
    ASSERT (GB_Type_compatible (B->type, op->ytype)) ;

    (*Chandle) = NULL ;

    //--------------------------------------------------------------------------
    // allocate the output matrix C
    //--------------------------------------------------------------------------

    // C is hypersparse if A or B are hypersparse (contrast with GB_add)
    bool C_is_hyper = (A->is_hyper || B->is_hyper) && (A->vdim > 1) ;

    // [ allocate the result C; C->p is malloc'd
    // worst case nnz (C) is min (nnz (A), nnz (B))
    GrB_Info info ;
    GrB_Matrix C = NULL ;           // allocate a new header for C
    GB_CREATE (&C, ctype, A->vlen, A->vdim, GB_Ap_malloc, C_is_csc,
        GB_SAME_HYPER_AS (C_is_hyper), B->hyper_ratio,
        GB_IMIN (A->nvec_nonempty, B->nvec_nonempty),
        GB_IMIN (GB_NNZ (A), GB_NNZ (B)), true) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

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

    GxB_binary_function fmult = op->function ;

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

    int64_t *Ci = C->i ;
    GB_void *Cx = C->x ;

    int64_t jlast, cnz, cnz_last ;
    GB_jstartup (C, &jlast, &cnz, &cnz_last) ;

    const int64_t *Ai = A->i, *Bi = B->i ;
    const GB_void *Ax = A->x, *Bx = B->x ;

    GB_for_each_vector2 (A, B)
    {

        //----------------------------------------------------------------------
        // get the next column, A (:,j) and B (:j)
        //----------------------------------------------------------------------

        int64_t GBI2_initj (Iter, j, pa, pa_end, pb, pb_end) ;
        int64_t ajnz = pa_end - pa ;
        int64_t bjnz = pb_end - pb ;

        //----------------------------------------------------------------------
        // compute C (:,j): pattern is the set intersection
        //----------------------------------------------------------------------

        if (ajnz == 0 || bjnz == 0)
        { 

            // one or both columns are empty; set intersection is empty
            ;

        }
        else if (Ai [pa_end-1] < Bi [pb])
        { 

            // all entries in A are in lower row indices than all the
            // entries in B; set intersection is empty
            ;

        }
        else if (Bi [pb_end-1] < Ai [pa])
        { 
            // all entries in B are in lower row indices than all the
            // entries in A; set intersection is empty
            ;

        }
        else if (ajnz > 256 * bjnz)
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
                    GB_EMULT ;
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
                    GB_EMULT ;
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
                    GB_EMULT ;
                }
            }
        }

        //----------------------------------------------------------------------
        // finalize C(:,j)
        //----------------------------------------------------------------------

        // this cannot fail since C->plen is the upper bound: min of the
        // non-empty vectors of A and B
        info = GB_jappend (C, j, &jlast, cnz, &cnz_last, Context) ;
        ASSERT (info == GrB_SUCCESS) ;

        #if 0
        // if it could fail, do this:
        if (info != GrB_SUCCESS) { GB_MATRIX_FREE (&C) ; return (info) ; }
        #endif
    }

    GB_jwrapup (C, jlast, cnz) ;        // C->p now initialized ]

    //--------------------------------------------------------------------------
    // trim the size of C: this cannot fail
    //--------------------------------------------------------------------------

    ASSERT (cnz <= C->nzmax) ;
    info = GB_ix_realloc (C, cnz, true, Context) ;
    ASSERT (info == GrB_SUCCESS) ;
    ASSERT_OK (GB_check (C, "C output for C=A.*B", GB0)) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

