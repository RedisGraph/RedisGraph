//------------------------------------------------------------------------------
// GB_select: apply a select operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C<M> = accum (C, select(A,Thunk)) or select(A,Thunk)').

#define GB_FREE_ALL                         \
{                                           \
    GB_Matrix_free (&T) ;                   \
}

#include "GB_select.h"
#include "GB_accum_mask.h"

GrB_Info GB_select          // C<M> = accum (C, select(A,k)) or select(A',k)
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C descriptor
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // descriptor for M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GxB_SelectOp op,          // operator to select the entries
    const GrB_Matrix A,             // input matrix
    const GxB_Scalar Thunk_in,      // optional input for select operator
    const bool A_transpose,         // A matrix descriptor
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C may be aliased with M and/or A

    GB_RETURN_IF_FAULTY_OR_POSITIONAL (accum) ;
    GB_RETURN_IF_FAULTY (Thunk_in) ;
    GB_RETURN_IF_NULL_OR_FAULTY (op) ;

    ASSERT_MATRIX_OK (C, "C input for GB_select", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_select", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GB_select", GB0) ;
    ASSERT_SELECTOP_OK (op, "selectop for GB_select", GB0) ;
    ASSERT_MATRIX_OK (A, "A input for GB_select", GB0) ;
    ASSERT_SCALAR_OK_OR_NULL (Thunk_in, "Thunk_in for GB_select", GB0) ;

    GrB_Matrix T = NULL ;

    // check domains and dimensions for C<M> = accum (C,T)
    GrB_Info info ;
    GB_OK (GB_compatible (C->type, C, M, accum, A->type, Context)) ;

    GB_Type_code typecode = A->type->code ;
    GB_Select_Opcode opcode = op->opcode ;

    // these opcodes are not availabe to the user
    ASSERT (opcode != GB_RESIZE_opcode) ;
    ASSERT (opcode != GB_NONZOMBIE_opcode) ;

    // check if the op is a GT, GE, LT, or LE comparator
    bool op_is_ordered_comparator =
        opcode == GB_GT_ZERO_opcode || opcode == GB_GT_THUNK_opcode ||
        opcode == GB_GE_ZERO_opcode || opcode == GB_GE_THUNK_opcode ||
        opcode == GB_LT_ZERO_opcode || opcode == GB_LT_THUNK_opcode ||
        opcode == GB_LE_ZERO_opcode || opcode == GB_LE_THUNK_opcode ;

    if (op_is_ordered_comparator)
    {
        // built-in GT, GE, LT, and LE operators cannot be used with
        // user-defined or complex types.
        if (typecode == GB_UDT_code)
        { 
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "Operator %s not defined for user-defined types", op->name) ;
        }
        else if (typecode == GB_FC32_code || typecode == GB_FC64_code)
        { 
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "Operator %s not defined for complex types", op->name) ;
        }
    }

    // C = op (A) must be compatible, already checked in GB_compatible

    // A must also be compatible with op->xtype
    if (!GB_Type_compatible (A->type, op->xtype))
    { 
        GB_ERROR (GrB_DOMAIN_MISMATCH,
            "Incompatible type for C=%s(A,Thunk):\n"
            "input A type [%s]\n"
            "cannot be typecast to operator input of type [%s]",
            op->name, A->type->name, op->xtype->name) ;
    }

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

    // check if op is (NE, EQ, GT, GE, LT, LE)_THUNK
    bool op_is_thunk_comparator =
        (opcode >= GB_NE_THUNK_opcode && opcode <= GB_LE_THUNK_opcode) ;

    // check if op is TRIL, TRIU, DIAG, or OFFDIAG
    bool op_is_positional = GB_SELECTOP_IS_POSITIONAL (opcode) ;

    // check if op is user-defined
    bool op_is_user_defined = (opcode >= GB_USER_SELECT_opcode) ;

    int64_t nz_thunk = 0 ;
    GB_void *GB_RESTRICT xthunk_in = NULL ;

    if (Thunk_in != NULL)
    {

        // finish any pending work on the Thunk
        GB_MATRIX_WAIT (Thunk_in) ;
        nz_thunk = GB_NNZ (Thunk_in) ;

        // if op is TRIL, TRIU, DIAG, or OFFDIAG, Thunk_in must be
        // compatible with GrB_INT64
        if (op_is_positional &&
            !GB_Type_compatible (GrB_INT64, Thunk_in->type))
        { 
            // Thunk not a built-in type, for a built-in select operator
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "Incompatible type for C=%s(A,Thunk):\n"
                "input Thunk type [%s]\n"
                "not compatible with GrB_INT64 input to built-in operator %s",
                op->name, Thunk_in->type->name, op->name) ;
        }

        // if op is (NE, EQ, GT, GE, LT, LE)_THUNK, then Thunk must be
        // compatible with the matrix type
        if (op_is_thunk_comparator &&
           !GB_Type_compatible (A->type, Thunk_in->type))
        { 
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "Incompatible type for C=%s(A,Thunk):\n"
                "input A type [%s] and Thunk type [%s] not compatible",
                op->name, A->type->name, Thunk_in->type->name) ;
        }

        // get the pointer to the value of Thunk_in
        xthunk_in = (GB_void *) Thunk_in->x ;
    }

    // if op is user-defined, Thunk must match the op->ttype exactly
    if (op_is_user_defined)
    {
        if (op->ttype == NULL && Thunk_in != NULL)
        { 
            // select operator does not take a Thunk, but one is present
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "User-defined operator %s(A,Thunk) does not take a Thunk\n"
                "input, but Thunk parameter is non-NULL", op->name) ;
        }
        else if (op->ttype != NULL && Thunk_in == NULL)
        { 
            // select operator takes a Thunk, but Thunk parameter is missing
            GB_ERROR (GrB_NULL_POINTER,
                "Required argument is null: [%s]", "Thunk") ;
        }
        else if (op->ttype != NULL && Thunk_in != NULL)
        {
            // select operator takes a Thunk, and it is present on input.
            // The types must match exactly.
            if (op->ttype != Thunk_in->type)
            { 
                GB_ERROR (GrB_DOMAIN_MISMATCH,
                    "User-defined operator %s(A,Thunk) has a Thunk input\n"
                    "type of [%s], which must exactly match the type of the\n"
                    "Thunk parameter; parameter to GxB_select has type [%s]",
                    op->name, op->ttype->name, Thunk_in->type->name) ;
            }
            if (nz_thunk != 1)
            { 
                GB_ERROR (GrB_INVALID_VALUE,
                    "User-defined operator %s(A,Thunk) has a Thunk input,\n"
                    "which must not be empty", op->name) ;
            }
        }
    }

    // quick return if an empty mask is complemented
    GB_RETURN_IF_QUICK_MASK (C, C_replace, M, Mask_comp) ;

    //--------------------------------------------------------------------------
    // delete any lingering zombies and assemble any pending tuples
    //--------------------------------------------------------------------------

    GB_MATRIX_WAIT (M) ;        // TODO: delay until accum/mask phase
    GB_MATRIX_WAIT (A) ;        // TODO: could tolerate jumbled in some cases

    GB_BURBLE_DENSE (C, "(C %s) ") ;
    GB_BURBLE_DENSE (M, "(M %s) ") ;
    GB_BURBLE_DENSE (A, "(A %s) ") ;

    //--------------------------------------------------------------------------
    // handle the CSR/CSC format and the transposed case
    //--------------------------------------------------------------------------

    // A and C can be in CSR or CSC format (in any combination), and A can be
    // transposed first via A_transpose.  However, A is not explicitly
    // transposed first.  Instead, the selection operation is modified by
    // changing the operator, and the resulting matrix T is transposed, if
    // needed.

    // Instead of explicitly transposing the input matrix A and output T:
    // If A in CSC format and not transposed: treat as if A and T were CSC
    // If A in CSC format and transposed:     treat as if A and T were CSR
    // If A in CSR format and not transposed: treat as if A and T were CSR
    // If A in CSR format and transposed:     treat as if A and T were CSC

    bool A_csc = (A->is_csc == !A_transpose) ;

    // The final transpose, if needed, is accomplished in GB_accum_mask, by
    // tagging T as the same CSR/CSC format as A_csc.  If the format of T and C
    // do not match, GB_accum_mask transposes T, computing C<M>=accum(C,T').

    //--------------------------------------------------------------------------
    // change the opcode if needed
    //--------------------------------------------------------------------------

    bool flipij = !A_csc ;

    ASSERT_SCALAR_OK_OR_NULL (Thunk_in, "Thunk_in now GB_select", GB0) ;

    // if A is boolean, get the value of Thunk typecasted to boolean
    bool bthunk = false ;

    if (typecode == GB_BOOL_code && op_is_thunk_comparator && nz_thunk > 0)
    { 
        // bthunk = (bool) Thunk_in
        GB_cast_array ((GB_void *) (&bthunk), GB_BOOL_code, xthunk_in,
            Thunk_in->type->code, NULL, Thunk_in->type->size, 1, 1) ;
    }

    int64_t ithunk = 0 ;        // ithunk = (int64_t) Thunk (0)
    bool use_dup = false ;
    bool is_empty = false ;

    if (op_is_positional)
    {

        //----------------------------------------------------------------------
        // tril, triu, diag, offdiag: get k and handle the flip
        //----------------------------------------------------------------------

        // The built-in operators are modified so they can always work as if A
        // were in CSC format.  If A is not in CSC, then the operation is
        // flipped.
        // 0: tril(A,k)    becomes triu(A,-k)
        // 1: triu(A,k)    becomes tril(A,-k)
        // 2: diag(A,k)    becomes diag(A,-k)
        // 3: offdiag(A,k) becomes offdiag(A,-k)
        // all others      Thunk is unchanged
        // userop(A)       row/col indices and dimensions are swapped

        // if Thunk is not present, or has no entries, then k defaults to zero
        if (nz_thunk > 0)
        { 
            // ithunk = (int64_t) (Thunk_in (0)) ;
            GB_cast_array ((GB_void *) &ithunk, GB_INT64_code, xthunk_in,
                Thunk_in->type->code, NULL, Thunk_in->type->size, 1, 1) ;
        }

        if (flipij)
        {
            ithunk = -ithunk ;
            if (opcode == GB_TRIL_opcode)
            { 
                opcode = GB_TRIU_opcode ;
            }
            else if (opcode == GB_TRIU_opcode)
            { 
                opcode = GB_TRIL_opcode ;
            }
            flipij = false ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // (NE, EQ, GT, GE, LT, LE) x (0, thunk): handle bool and uint cases
        //----------------------------------------------------------------------

        switch (opcode)
        {

            case GB_GT_ZERO_opcode   :  // A(i,j) > 0

                // bool and uint: rename GxB_GT_ZERO to GxB_NONZERO
                // user type: return error above
                switch (typecode)
                {
                    case GB_BOOL_code   : 
                    case GB_UINT8_code  : 
                    case GB_UINT16_code : 
                    case GB_UINT32_code : 
                    case GB_UINT64_code : opcode = GB_NONZERO_opcode ; break ;
                    default: ;
                }
                break ;

            case GB_GE_ZERO_opcode   :  // A(i,j) >= 0

                // bool and uint: always true; use GB_dup
                // user type: return error above
                switch (typecode)
                {
                    case GB_BOOL_code   : 
                    case GB_UINT8_code  : 
                    case GB_UINT16_code : 
                    case GB_UINT32_code : 
                    case GB_UINT64_code : use_dup = true ; break ;
                    default: ;
                }
                break ;

            case GB_LT_ZERO_opcode   :  // A(i,j) < 0

                // bool and uint: always false; return an empty matrix
                // user type: return error above
                switch (typecode)
                {
                    case GB_BOOL_code   : 
                    case GB_UINT8_code  : 
                    case GB_UINT16_code : 
                    case GB_UINT32_code : 
                    case GB_UINT64_code : is_empty = true ; break ;
                    default: ;
                }
                break ;

            case GB_LE_ZERO_opcode   :  // A(i,j) <= 0

                // bool and uint: rename GxB_LE_ZERO to GxB_EQ_ZERO
                // user type: return error above
                switch (typecode)
                {
                    case GB_BOOL_code   : 
                    case GB_UINT8_code  : 
                    case GB_UINT16_code : 
                    case GB_UINT32_code : 
                    case GB_UINT64_code : opcode = GB_EQ_ZERO_opcode ; break ;
                    default: ;
                }
                break ;

            case GB_NE_THUNK_opcode   : // A(i,j) != thunk

                // bool: if thunk is true,  rename GxB_NE_THUNK to GxB_EQ_ZERO 
                //       if thunk is false, rename GxB_NE_THUNK to GxB_NONZERO 
                if (typecode == GB_BOOL_code)
                { 
                    opcode = (bthunk) ? GB_EQ_ZERO_opcode : GB_NONZERO_opcode ;
                }
                break ;

            case GB_EQ_THUNK_opcode   : // A(i,j) == thunk

                // bool: if thunk is true,  rename GxB_NE_THUNK to GxB_NONZERO 
                //       if thunk is false, rename GxB_NE_THUNK to GxB_EQ_ZERO 
                if (typecode == GB_BOOL_code)
                { 
                    opcode = (bthunk) ? GB_NONZERO_opcode : GB_EQ_ZERO_opcode ;
                }
                break ;

            case GB_GT_THUNK_opcode   : // A(i,j) > thunk

                // bool: if thunk is true,  return an empty matrix
                //       if thunk is false, rename GxB_GT_THUNK to GxB_NONZERO
                // user type: return error above
                if (typecode == GB_BOOL_code)
                {
                    if (bthunk)
                    { 
                        is_empty = true ;
                    }
                    else
                    { 
                        // rename GT_THUNK to NONZERO for boolean
                        opcode = GB_NONZERO_opcode ;
                    }
                }
                break ;

            case GB_GE_THUNK_opcode   : // A(i,j) >= thunk

                // bool: if thunk is true,  rename GxB_GE_THUNK to GxB_NONZERO
                //       if thunk is false, use GB_dup
                // user type: return error above
                if (typecode == GB_BOOL_code)
                {
                    if (bthunk)
                    { 
                        opcode = GB_NONZERO_opcode ;
                    }
                    else
                    { 
                        // use dup for GE_THUNK if thunk is false
                        use_dup = true ;
                    }
                }
                break ;

            case GB_LT_THUNK_opcode   : // A(i,j) < thunk

                // bool: if thunk is true,  rename GxB_LT_THUNK to GxB_EQ_ZERO
                //       if thunk is false, return an empty matrix
                // user type: return error above
                if (typecode == GB_BOOL_code)
                {
                    if (bthunk)
                    { 
                        opcode = GB_EQ_ZERO_opcode ;
                    }
                    else
                    { 
                        // matrix empty for LT_THUNK_BOOL, if thunk false
                        is_empty = true ;
                    }
                }
                break ;

            case GB_LE_THUNK_opcode   : // A(i,j) <= thunk

                // bool: if thunk is true,  use GB_dup
                //       if thunk is false, rename GxB_LE_ZERO to GxB_EQ_ZERO
                // user type: return error
                if (typecode == GB_BOOL_code)
                {
                    if (bthunk)
                    { 
                        // use dup for LE_THUNK if thunk is true
                        use_dup = true ;
                    }
                    else
                    { 
                        opcode = GB_EQ_ZERO_opcode ;
                    }
                }
                break ;

            default : ;     // use the opcode as-is
        }
    }

    //--------------------------------------------------------------------------
    // create T
    //--------------------------------------------------------------------------

    if (use_dup)
    { 
        // selectop is always true, so use GB_dup to do T = A
        GB_OK (GB_dup (&T, A, true, A->type, Context)) ;
    }
    else if (is_empty)
    { 
        // selectop is always false, so T is an empty matrix
        info = GB_new (&T, // auto (sparse or hyper), new header
            A->type, A->vlen, A->vdim, GB_Ap_calloc, A_csc,
            GxB_SPARSE + GxB_HYPERSPARSE, GB_Global_hyper_switch_get ( ),
            1, Context) ;
        GB_OK (info) ;
    }
    else
    { 
        // T = select (A, Thunk)
        GB_OK (GB_selector (&T, opcode, op, flipij, A, ithunk,
            (op_is_thunk_comparator || op_is_user_defined) ? Thunk_in : NULL,
            Context)) ;
    }

    T->is_csc = A_csc ;
    ASSERT_MATRIX_OK (T, "T=select(A,Thunk) output", GB0) ;
    ASSERT_MATRIX_OK (C, "C for accum; T=select(A,Thunk) output", GB0) ;

    //--------------------------------------------------------------------------
    // C<M> = accum (C,T): accumulate the results into C via the mask
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, M, NULL, accum, &T, C_replace, Mask_comp,
        Mask_struct, Context)) ;
}

