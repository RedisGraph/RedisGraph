//------------------------------------------------------------------------------
// GB_shallow_op:  create a shallow copy and apply a unary operator to a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = op (A)

// Create a shallow copy of a matrix, applying an operator to the entries.

// The CSR/CSC format of C and A can differ, but they have they same vlen and
// vdim.  This function is CSR/CSC agnostic, except that C_is_csc is used to
// set the C->is_csc state in C.

// The values are typically not a shallow copy, unless no typecasting is needed
// and the operator is an identity operator.

// The pattern is always a shallow copy.  No errors are checked except for
// out-of-memory conditions.  This function is not user-callable.  Shallow
// matrices are never passed back to the user.

// Compare this function with GB_shallow_cast.c

#include "GB.h"

GrB_Info GB_shallow_op      // create shallow matrix and apply operator
(
    GrB_Matrix *Chandle,    // output matrix C, of type op->ztype
    const bool C_is_csc,    // desired CSR/CSC format of C
    const GrB_UnaryOp op,   // operator to apply
    const GrB_Matrix A,     // input matrix to typecast
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;
    ASSERT_OK (GB_check (A, "A for shallow_op", GB0)) ;
    ASSERT_OK (GB_check (op, "op for shallow_op", GB0)) ;
    ASSERT (GB_Type_compatible (op->xtype, A->type)) ;
    ASSERT ((A->nzmax == 0) == (A->i == NULL && A->x == NULL)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;

    (*Chandle) = NULL ;

    //--------------------------------------------------------------------------
    // construct a shallow copy of A for the pattern of C
    //--------------------------------------------------------------------------

    // allocate the struct for C, but do not allocate C->h, C->p, C->i, or C->x.
    // C has the exact same hypersparsity as A.
    GrB_Info info ;
    GrB_Matrix C = NULL ;           // allocate a new header for C
    GB_NEW (&C, op->ztype, A->vlen, A->vdim, GB_Ap_null, C_is_csc,
        GB_SAME_HYPER_AS (A->is_hyper), A->hyper_ratio, 0) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // make a shallow copy of the vector pointers
    //--------------------------------------------------------------------------

    ASSERT (C->magic == GB_MAGIC2) ;   // [ be careful; C not yet initialized
    C->p_shallow = true ;           // C->p not freed when freeing C
    C->h_shallow = true ;           // C->h not freed when freeing C
    C->p = A->p ;                   // C->p is of size A->plen + 1
    C->h = A->h ;                   // C->h is of size A->plen
    C->plen = A->plen ;             // C and A have the same hyperlist sizes
    C->nvec = A->nvec ;
    C->nvec_nonempty = A->nvec_nonempty ;
    C->magic = GB_MAGIC ;           // C is now initialized ]

    //--------------------------------------------------------------------------
    // check for empty matrix
    //--------------------------------------------------------------------------

    if (A->nzmax == 0)
    { 
        // C->p and C->h are shallow but the rest is empty
        C->nzmax = 0 ;
        C->i = NULL ;
        C->x = NULL ;
        C->i_shallow = false ;
        C->x_shallow = false ;
        ASSERT_OK (GB_check (C, "C = quick copy of empty A", GB0)) ;
        (*Chandle) = C ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // make a shallow copy of the pattern
    //--------------------------------------------------------------------------

    C->i = A->i ;               // of size A->nzmax
    C->i_shallow = true ;       // C->i will not be freed when freeing C

    //--------------------------------------------------------------------------
    // apply the operator to the numerical values
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ (A) ;
    ASSERT (A->nzmax >= GB_IMAX (anz,1)) ;

    if (op->opcode == GB_IDENTITY_opcode && A->type == op->xtype)
    { 
        // no work is done at all.  C is a pure shallow copy
        ASSERT (op->ztype == op->xtype) ;
        C->nzmax = A->nzmax ;
        C->x = A->x ;
        C->x_shallow = true ;       // C->x will not be freed when freeing C
        ASSERT_OK (GB_check (C, "C = pure shallow (A)", GB0)) ;
        (*Chandle) = C ;
        return (GrB_SUCCESS) ;
    }

    // allocate new space for the numerical values of C
    C->nzmax = GB_IMAX (anz,1) ;
    GB_MALLOC_MEMORY (C->x, C->nzmax, C->type->size) ;
    C->x_shallow = false ;          // free C->x when freeing C
    if (C->x == NULL)
    { 
        // out of memory
        double memory = GBYTES (C->nzmax, C->type->size) ;
        GB_MATRIX_FREE (&C) ;
        return (GB_OUT_OF_MEMORY (memory)) ;
    }

    //--------------------------------------------------------------------------
    // apply the unary operator
    //--------------------------------------------------------------------------

    // C->x = op ((op->xtype) Ax)
    GB_apply_op (C->x, op, A->x, A->type, anz) ;

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (C, "C = shallow (op (A))", GB0)) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

