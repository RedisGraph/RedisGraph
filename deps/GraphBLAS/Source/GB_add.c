//------------------------------------------------------------------------------
// GB_add: 'add' two matrices using an operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_add (C, A, B, op), 'adds' C = op (A,B), using the given operator
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

// This function does not transpose or reformat its inputs or outputs.  C, A,
// and B must have the same number of vectors and vector lengths.  However,
// suppose A is CSR, and B and C are CSC, but the caller wants to compute C =
// A'+B.  Then no transpose of A is needed; just interpret the CSR of A' as a
// CSC format.  The work is the same with C=A'+B if B and C are CSR and A is
// CSC.  Then the output C is CSR, and the CSC of A' is already effectively in
// CSR format.

// As a result, the input formats of A and B are not relevant, and neither is
// the output format of C.  This function can be completely agnostic as to the
// CSR / CSC formats of A, B, and C.  The format of C is determined by the
// caller and assigned to C->is_csc, but is otherwise unused here.

// The output C is hypersparse if both A and B are hypersparse; otherwise
// C is not hypersparse.

// FUTURE: this could be faster with built-in operators and types.

#include "GB.h"

GrB_Info GB_add             // C = A+B
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
    ASSERT_OK (GB_check (A, "A for C=A+B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for C=A+B", GB0)) ;
    ASSERT_OK (GB_check (op, "op for C=A+B", GB0)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (A->vdim == A->vdim && B->vlen == A->vlen) ;

    ASSERT (GB_Type_compatible (ctype,   op->ztype)) ;
    ASSERT (GB_Type_compatible (ctype,   A->type)) ;
    ASSERT (GB_Type_compatible (ctype,   B->type)) ;
    ASSERT (GB_Type_compatible (A->type, op->xtype)) ;
    ASSERT (GB_Type_compatible (B->type, op->ytype)) ;

    (*Chandle) = NULL ;

    //--------------------------------------------------------------------------
    // allocate the output matrix C
    //--------------------------------------------------------------------------

    // C is hypersparse if both A and B are (contrast with GrB_Matrix_emult).
    // C acquires the same hyperatio as A.

    bool C_is_hyper = (A->is_hyper && B->is_hyper) && (A->vdim > 1) ;

    // [ allocate the result C; C->p is malloc'd
    // worst case nnz (C) is nnz (A) + nnz (B)
    GrB_Info info ;
    GrB_Matrix C = NULL ;           // allocate a new header for C
    GB_CREATE (&C, ctype, A->vlen, A->vdim, GB_Ap_malloc, C_is_csc,
        GB_SAME_HYPER_AS (C_is_hyper), A->hyper_ratio,
        A->nvec_nonempty + B->nvec_nonempty, GB_NNZ (A) + GB_NNZ (B), true) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // two generic workers for C = A '+' B
    //--------------------------------------------------------------------------

    // If types are user-defined, the cast* function is just GB_copy_user_user,
    // which requires the size of the type.  No typecast is done.

    GxB_binary_function fadd = op->function ;

    int64_t *restrict Ci = C->i ;
    GB_void *restrict Cx = C->x ;

    int64_t jlast, cnz, cnz_last ;
    GB_jstartup (C, &jlast, &cnz, &cnz_last) ;

    const int64_t *restrict Ai = A->i ;
    const int64_t *restrict Bi = B->i ;

    const GB_void *restrict Ax = A->x ;
    const GB_void *restrict Bx = B->x ;

    // check if no typecasting is needed for the operator
    bool nocasting =
        (A->type->code == op->xtype->code) &&
        (B->type->code == op->ytype->code) &&
        (ctype->code   == op->ztype->code) ;

    if (nocasting && A->type == ctype && B->type == ctype)
    { 

        //----------------------------------------------------------------------
        // C = A + B, no typecasting at all, all types the same
        //----------------------------------------------------------------------

        size_t s = ctype->size ;

        // for each vector of A and B
        GB_for_each_vector2 (A, B)
        {

            //------------------------------------------------------------------
            // get the next column, A (:,j) and B (:j)
            //------------------------------------------------------------------

            int64_t GBI2_initj (Iter, j, pa, pa_end, pb, pb_end) ;

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

            //------------------------------------------------------------------
            // finalize C(:,j)
            //------------------------------------------------------------------

            // this cannot fail since C->plen is the upper bound: the sum of
            // the non-empty vectors of A and B.
            info = GB_jappend (C, j, &jlast, cnz, &cnz_last, Context) ;
            ASSERT (info == GrB_SUCCESS) ;
            #if 0
            // if it could fail:
            if (info != GrB_SUCCESS) { GB_MATRIX_FREE (&C) ; return (info) ; }
            #endif
        }

    }
    else
    { 

        //----------------------------------------------------------------------
        // C = A + B, with any typecasting
        //----------------------------------------------------------------------

        size_t csize = ctype->size ;
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
        cast_A_to_C = GB_cast_factory (ctype->code,     A->type->code) ;
        cast_B_to_C = GB_cast_factory (ctype->code,     B->type->code) ;
        cast_Z_to_C = GB_cast_factory (ctype->code,     op->ztype->code) ;

        // for each vector of A and B
        GB_for_each_vector2 (A, B)
        {

            //------------------------------------------------------------------
            // get the next column, A (:,j) and B (:j)
            //------------------------------------------------------------------

            int64_t GBI2_initj (Iter, j, pa, pa_end, pb, pb_end) ;

            //------------------------------------------------------------------
            // merge A (:,j) and B (:,j) while both have entries
            //------------------------------------------------------------------

            for ( ; pa < pa_end && pb < pb_end ; cnz++)
            {
                // both A(ia,j) and B (ib,j) are at head of lists to merge
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

            //------------------------------------------------------------------
            // finalize C(:,j)
            //------------------------------------------------------------------

            // this cannot fail since C->plen is the upper bound: the sum of
            // the non-empty vectors of A and B.
            info = GB_jappend (C, j, &jlast, cnz, &cnz_last, Context) ;

            #if 0
            // if it could fail:
            if (info != GrB_SUCCESS) { GB_MATRIX_FREE (&C) ; return (info) ; }
            #endif
        }
    }

    //--------------------------------------------------------------------------
    // finalize C and trim its size: this cannot fail
    //--------------------------------------------------------------------------

    GB_jwrapup (C, jlast, cnz) ;
    info = GB_ix_realloc (C, GB_NNZ (C), true, Context) ;
    ASSERT (info == GrB_SUCCESS) ;
    ASSERT_OK (GB_check (C, "C output for C=A+B", GB0)) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

