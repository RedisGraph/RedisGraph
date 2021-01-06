//------------------------------------------------------------------------------
// GB_shallow_op:  create a shallow copy and apply a unary operator to a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

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

// Compare this function with GB_shallow_copy.c

#include "GB_apply.h"

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_shallow_op      // create shallow matrix and apply operator
(
    GrB_Matrix *Chandle,    // output matrix C, of type op*->ztype
    const bool C_is_csc,    // desired CSR/CSC format of C
        const GrB_UnaryOp op1,          // unary operator to apply
        const GrB_BinaryOp op2,         // binary operator to apply
        const GxB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, binop(x,A) else binop(A,y)
    const GrB_Matrix A,     // input matrix to typecast
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;
    ASSERT_MATRIX_OK (A, "A for shallow_op", GB0) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;

    GrB_Type ztype, op_intype = NULL ;
    GB_Opcode opcode = (op1 != NULL) ? op1->opcode : op2->opcode ;
    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;
    if (op1 != NULL)
    {
        ASSERT_UNARYOP_OK (op1, "unop for shallow_op", GB0) ;
        if (!op_is_positional)
        { 
            ASSERT (GB_Type_compatible (op1->xtype, A->type)) ;
            op_intype = op1->xtype ;
        }
        ztype = op1->ztype ;
    }
    else // op2 != NULL
    {
        ASSERT_BINARYOP_OK (op2, "binop for shallow_op", GB0) ;
        if (!op_is_positional)
        { 
            op_intype = (binop_bind1st) ? op2->xtype : op2->ytype ;
            ASSERT (GB_Type_compatible (op_intype, A->type)) ;
        }
        ztype = op2->ztype ;
    }

    (*Chandle) = NULL ;

    //--------------------------------------------------------------------------
    // construct a shallow copy of A for the pattern of C
    //--------------------------------------------------------------------------

    // allocate the struct for C, but do not allocate C->{p,h,b,i,x}
    // C has the exact same sparsity structure as A.
    GrB_Info info ;
    GrB_Matrix C = NULL ;
    info = GB_new (&C, // full, bitmap, sparse or hyper; new header
        ztype, A->vlen, A->vdim, GB_Ap_null, C_is_csc,
        GB_sparsity (A), A->hyper_switch, 0, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // make a shallow copy of the vector pointers
    //--------------------------------------------------------------------------

    C->p_shallow = (A->p != NULL) ;     // C->p not freed when freeing C
    C->h_shallow = (A->h != NULL) ;     // C->h not freed when freeing C
    C->p = A->p ;                       // C->p is of size A->plen + 1
    C->h = A->h ;                       // C->h is of size A->plen
    C->plen = A->plen ;                 // C and A have the same hyperlist sizes
    C->nvec = A->nvec ;
    C->nvec_nonempty = A->nvec_nonempty ;
    C->nvals = A->nvals ;               // if A bitmap 
    C->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // check for empty matrix
    //--------------------------------------------------------------------------

    if (A->nzmax == 0)
    { 
        // C->p and C->h are shallow but the rest is empty
        C->nzmax = 0 ;
        C->b = NULL ;
        C->i = NULL ;
        C->x = NULL ;
        C->b_shallow = false ;
        C->i_shallow = false ;
        C->x_shallow = false ;
        ASSERT_MATRIX_OK (C, "C = quick copy of empty A", GB0) ;
        (*Chandle) = C ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // make a shallow copy of the pattern
    //--------------------------------------------------------------------------

    C->i = A->i ;               // of size A->nzmax
    C->i_shallow = (A->i != NULL) ; // C->i will not be freed when freeing C

    C->b = A->b ;               // of size A->nzmax
    C->b_shallow = (A->b != NULL) ;  // C->b will not be freed when freeing C

    //--------------------------------------------------------------------------
    // make a shallow copy of the values, if possible
    //--------------------------------------------------------------------------

    // If the identity operator, first(A,y), second(x,A), any(A,y), or any(x,A)
    // are used with no typecasting, C->x becomes a shallow copy of A->x, and
    // no work is done.

    int64_t anz = GB_NNZ_HELD (A) ;
    ASSERT (A->nzmax >= GB_IMAX (anz, 1)) ;

    if (A->type == op_intype &&
        ((opcode == GB_IDENTITY_opcode) || (opcode == GB_ANY_opcode) ||
         (opcode == GB_FIRST_opcode  && !binop_bind1st) ||
         (opcode == GB_SECOND_opcode &&  binop_bind1st)))
    { 
        // no work is done at all.  C is a pure shallow copy
        GBURBLE ("(pure shallow) ") ;
        C->nzmax = A->nzmax ;
        C->x = A->x ;
        C->x_shallow = true ;       // C->x will not be freed when freeing C
        ASSERT_MATRIX_OK (C, "C = pure shallow (A)", GB0) ;
        (*Chandle) = C ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // apply the operator to the numerical values
    //--------------------------------------------------------------------------

    // allocate new space for the numerical values of C
    C->nzmax = GB_IMAX (anz, 1) ;
    C->x = GB_MALLOC (C->nzmax * C->type->size, GB_void) ;
    C->x_shallow = false ;          // free C->x when freeing C
    if (C->x == NULL)
    { 
        // out of memory
        GB_Matrix_free (&C) ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    GB_void *Cx = (GB_void *) C->x ;
    info = GB_apply_op (Cx, op1,    // op1 is never identity of same types
        op2, scalar, binop_bind1st, A, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_Matrix_free (&C) ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C = shallow (op (A))", GB0) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

