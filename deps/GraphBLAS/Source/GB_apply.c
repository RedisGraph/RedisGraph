//------------------------------------------------------------------------------
// GB_apply: apply a unary operator; optionally transpose a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C<M> = accum (C, op(A)) or accum (C, op(A)')

// GB_apply does the work for GrB_*_apply, including the binary op variants.

#include "GB_apply.h"
#include "GB_binop.h"
#include "GB_transpose.h"
#include "GB_accum_mask.h"

#define GB_FREE_ALL ;

GrB_Info GB_apply                   // C<M> = accum (C, op(A)) or op(A')
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C descriptor
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // M descriptor
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
        const GrB_UnaryOp op1_in,       // unary operator to apply
        const GrB_BinaryOp op2_in,      // binary operator to apply
        const GxB_Scalar scalar,        // scalar to bind to binary operator
        bool binop_bind1st,             // if true, binop(x,A) else binop(A,y)
    const GrB_Matrix A,             // first input:  matrix A
    bool A_transpose,               // A matrix descriptor
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C may be aliased with M and/or A

    struct GB_Matrix_opaque T_header ;
    GrB_Matrix T = GB_clear_static_header (&T_header) ;

    GB_RETURN_IF_FAULTY_OR_POSITIONAL (accum) ;
    ASSERT_MATRIX_OK (C, "C input for GB_apply", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_apply", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GB_apply", GB0) ;
    ASSERT_MATRIX_OK (A, "A input for GB_apply", GB0) ;

    GrB_UnaryOp  op1 = op1_in ;
    GrB_BinaryOp op2 = op2_in ;
    GB_Opcode opcode ;
    GrB_Type T_type ;
    if (op1 != NULL)
    {
        // apply a unary operator
        GB_RETURN_IF_FAULTY (op1) ;
        ASSERT_UNARYOP_OK (op1, "op1 for GB_apply", GB0) ;
        T_type = op1->ztype ;
        opcode = op1->opcode ;
        if (!GB_OPCODE_IS_POSITIONAL (opcode))
        {
            // A must also be compatible with op1->xtype
            if (!GB_Type_compatible (A->type, op1->xtype))
            { 
                GB_ERROR (GrB_DOMAIN_MISMATCH,
                    "Incompatible type for z=%s(x):\n"
                    "input A of type [%s]\n"
                    "cannot be typecast to x input of type [%s]",
                    op1->name, A->type->name, op1->xtype->name) ;
            }
        }
    }
    else if (op2 != NULL)
    {
        // apply a binary operator, with one input bound to a scalar
        GB_RETURN_IF_FAULTY (op2) ;
        ASSERT_BINARYOP_OK (op2, "op2 for GB_apply", GB0) ;
        ASSERT_SCALAR_OK (scalar, "scalar for GB_apply", GB0) ;
        T_type = op2->ztype ;
        opcode = op2->opcode ;
        if (!GB_OPCODE_IS_POSITIONAL (opcode))
        {
            bool op_is_first  = opcode == GB_FIRST_opcode ;
            bool op_is_second = opcode == GB_SECOND_opcode ;
            bool op_is_pair   = opcode == GB_PAIR_opcode ;
            if (binop_bind1st)
            {
                // C = op (scalar,A)
                // A must be compatible with op2->ytype
                if (!(op_is_first || op_is_pair ||
                      GB_Type_compatible (A->type, op2->ytype)))
                { 
                    GB_ERROR (GrB_DOMAIN_MISMATCH,
                        "Incompatible type for z=%s(x,y):\n"
                        "input A of type [%s]\n"
                        "cannot be typecast to y input of type [%s]",
                        op2->name, A->type->name, op2->ytype->name) ;
                }
                // scalar must be compatible with op2->xtype
                if (!(op_is_second || op_is_pair ||
                      GB_Type_compatible (scalar->type, op2->xtype)))
                { 
                    GB_ERROR (GrB_DOMAIN_MISMATCH,
                        "Incompatible type for z=%s(x,y):\n"
                        "input scalar of type [%s]\n"
                        "cannot be typecast to x input of type [%s]",
                        op2->name, scalar->type->name, op2->xtype->name) ;
                }
            }
            else
            {
                // C = op (A,scalar)
                // A must be compatible with op2->xtype
                if (!(op_is_first || op_is_pair ||
                      GB_Type_compatible (A->type, op2->xtype)))
                { 
                    GB_ERROR (GrB_DOMAIN_MISMATCH,
                        "Incompatible type for z=%s(x,y):\n"
                        "input scalar of type [%s]\n"
                        "cannot be typecast to x input of type [%s]",
                        op2->name, A->type->name, op2->xtype->name) ;
                }
                // scalar must be compatible with op2->ytype
                if (!(op_is_second || op_is_pair
                      || GB_Type_compatible (scalar->type, op2->ytype)))
                { 
                    GB_ERROR (GrB_DOMAIN_MISMATCH,
                        "Incompatible type for z=%s(x,y):\n"
                        "input A of type [%s]\n"
                        "cannot be typecast to y input of type [%s]",
                        op2->name, scalar->type->name, op2->ytype->name) ;
                }
            }
        }
    }
    else
    { 
        GB_ERROR (GrB_NULL_POINTER,
            "Required argument is null: [%s]", "op") ;
    }

    // check domains and dimensions for C<M> = accum (C,T)
    GrB_Info info ;
    GB_OK (GB_compatible (C->type, C, M, Mask_struct, accum, T_type, Context)) ;

    // check the dimensions
    int64_t tnrows = (A_transpose) ? GB_NCOLS (A) : GB_NROWS (A) ;
    int64_t tncols = (A_transpose) ? GB_NROWS (A) : GB_NCOLS (A) ;
    if (GB_NROWS (C) != tnrows || GB_NCOLS (C) != tncols)
    { 
        GB_ERROR (GrB_DIMENSION_MISMATCH,
            "Dimensions not compatible:\n"
            "output is " GBd "-by-" GBd "\n"
            "input is " GBd "-by-" GBd "%s",
            GB_NROWS (C), GB_NCOLS (C),
            tnrows, tncols, A_transpose ? " (transposed)" : "") ;
    }

    // quick return if an empty mask is complemented
    GB_RETURN_IF_QUICK_MASK (C, C_replace, M, Mask_comp, Mask_struct) ;

    // delete any lingering zombies and assemble any pending tuples
    GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES (A) ;      // A can be jumbled
    GB_MATRIX_WAIT (scalar) ;

    if (op2 != NULL && GB_nnz ((GrB_Matrix) scalar) != 1)
    { 
        // the scalar entry must be present
        GB_ERROR (GrB_INVALID_VALUE, "%s", "Scalar must contain an entry") ;
    }

    //--------------------------------------------------------------------------
    // rename first, second, any, and pair operators
    //--------------------------------------------------------------------------

    GB_binop_rename (&op1, &op2, binop_bind1st) ;

    //--------------------------------------------------------------------------
    // T = op(A) or op(A')
    //--------------------------------------------------------------------------

    bool T_is_csc = C->is_csc ;
    if (T_is_csc != A->is_csc)
    { 
        // Flip the sense of A_transpose
        A_transpose = !A_transpose ;
    }

    if (!T_is_csc)
    {
        // positional ops must be flipped, with i and j swapped
        if (op1 != NULL)
        { 
            op1 = GB_positional_unop_ijflip (op1) ;
            opcode = op1->opcode ;
        }
        else if (op2 != NULL)
        { 
            op2 = GB_positional_binop_ijflip (op2) ;
            opcode = op2->opcode ;
        }
    }

    if (A_transpose)
    { 
        // T = op (A'), typecasting to op*->ztype
        GBURBLE ("(transpose-op) ") ;
        info = GB_transpose (T, T_type, T_is_csc, A, op1, op2, scalar,
            binop_bind1st, Context) ;
        ASSERT (GB_JUMBLED_OK (T)) ;
        // A positional op is applied to C after the transpose is computed,
        // using the T_is_csc format.  The ijflip is handled above.
    }
    else if (M == NULL && accum == NULL && (C == A) && C->type == T_type
        && GB_nnz (C) > 0)
    {
        GBURBLE ("(in-place-op) ") ;
        // C = op (C), operating on the values in-place, with no typecasting
        // of the output of the operator with the matrix C.
        // No work to do if the op is identity.
        if (opcode != GB_IDENTITY_opcode)
        {
            // the output Cx is aliased with C->x in GB_apply_op.
            GB_iso_code
                C_code_iso = GB_iso_unop_code (C, op1, op2, binop_bind1st) ;
            info = GB_apply_op ((GB_void *) C->x, C->type, C_code_iso,
                op1, op2, scalar, binop_bind1st, C, Context) ;
            if (info == GrB_SUCCESS && C_code_iso != GB_NON_ISO)
            { 
                // compact the iso values of C
                C->iso = true ; // OK
                info = GB_convert_any_to_iso (C, NULL, true, Context) ;
            }
        }
        return (info) ;
    }
    else
    { 
        // T = op (A), pattern is a shallow copy of A, type is op*->ztype.
        GBURBLE ("(shallow-op) ") ;
        info = GB_shallow_op (T, T_is_csc, op1, op2, scalar, binop_bind1st,
            A, Context) ;
    }

    if (info != GrB_SUCCESS)
    { 
        GB_phbix_free (T) ;
        return (info) ;
    }

    ASSERT (T->is_csc == C->is_csc) ;

    //--------------------------------------------------------------------------
    // C<M> = accum (C,T): accumulate the results into C via the M
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, M, NULL, accum, &T, C_replace, Mask_comp,
        Mask_struct, Context)) ;
}

