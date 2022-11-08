//------------------------------------------------------------------------------
// GB_select: apply a select operator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C<M> = accum (C, select(A,Thunk)) or select(A,Thunk)')

#define GB_FREE_ALL         \
{                           \
    GB_Matrix_free (&T) ;   \
}

#include "GB_select.h"
#include "GB_accum_mask.h"
#include "GB_transpose.h"

GrB_Info GB_select          // C<M> = accum (C, select(A,k)) or select(A',k)
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C descriptor
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // descriptor for M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GB_Operator op,           // GxB_SelectOp or GrB_IndexUnaryOp
    const GrB_Matrix A,             // input matrix
    const GrB_Scalar Thunk,         // optional input for select operator
    const bool A_transpose,         // A matrix descriptor
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // C may be aliased with M and/or A

    GB_RETURN_IF_FAULTY_OR_POSITIONAL (accum) ;
    GB_RETURN_IF_FAULTY (Thunk) ;
    GB_RETURN_IF_NULL_OR_FAULTY (op) ;

    ASSERT_MATRIX_OK (C, "C input for GB_select", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for GB_select", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (accum, "accum for GB_select", GB0) ;
    ASSERT_OP_OK (op, "selectop/indexunaryop for GB_select", GB0) ;
    ASSERT_MATRIX_OK (A, "A input for GB_select", GB0) ;
    ASSERT_SCALAR_OK_OR_NULL (Thunk, "Thunk for GB_select", GB0) ;

    struct GB_Matrix_opaque T_header ;
    GrB_Matrix T = NULL ;

    // check domains and dimensions for C<M> = accum (C,T)
    GrB_Info info ;
    GB_OK (GB_compatible (C->type, C, M, Mask_struct, accum, A->type, Context));

    GB_Type_code acode = A->type->code ;
    GB_Type_code xcode = (op->xtype == NULL) ? GB_ignore_code : op->xtype->code;
    GB_Type_code tcode = GB_ignore_code ;
    GB_Opcode opcode = op->opcode ;
    bool op_is_selectop = GB_IS_SELECTOP_CODE (opcode) ;
    bool op_is_idxunop  = GB_IS_INDEXUNARYOP_CODE (opcode) ;
    ASSERT (op_is_selectop || op_is_idxunop) ;
    ASSERT (opcode != GB_FLIPDIAGINDEX_idxunop_code) ;

    if (op_is_idxunop)
    { 
        // Thunk is optional for GxB_Selectop, required for GrB_IndexUnaryOp
        GB_RETURN_IF_NULL (Thunk) ;
    }

    // this opcodes are not available to the user
    ASSERT (opcode != GB_NONZOMBIE_selop_code) ;

    // check if the op is a GT, GE, LT, or LE comparator
    bool op_is_ordered_comparator =
        opcode == GB_GT_ZERO_selop_code || opcode == GB_GT_THUNK_selop_code ||
        opcode == GB_GE_ZERO_selop_code || opcode == GB_GE_THUNK_selop_code ||
        opcode == GB_LT_ZERO_selop_code || opcode == GB_LT_THUNK_selop_code ||
        opcode == GB_LE_ZERO_selop_code || opcode == GB_LE_THUNK_selop_code ||
        opcode == GB_VALUEGT_idxunop_code ||
        opcode == GB_VALUEGE_idxunop_code ||
        opcode == GB_VALUELT_idxunop_code ||
        opcode == GB_VALUELE_idxunop_code ;

    if (op_is_ordered_comparator)
    {
        // built-in GT, GE, LT, and LE operators cannot be used with
        // user-defined or complex types.
        if (acode == GB_UDT_code)
        { 
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "Operator %s not defined for user-defined types", op->name) ;
        }
        else if (acode == GB_FC32_code || acode == GB_FC64_code)
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

    // check if op is (NE, EQ, GT, GE, LT, LE)_THUNK or VALUE*
    bool op_is_thunk_comparator =
        (opcode >= GB_VALUENE_idxunop_code && opcode <= GB_VALUELE_idxunop_code)
     || (opcode >= GB_NE_THUNK_selop_code && opcode <= GB_LE_THUNK_selop_code) ;

    // check if op is TRIL, TRIU, DIAG, OFFDIAG, ROW/COL/DIAGINDEX
    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (opcode) ;

    // check if op is user-defined
    bool op_is_user_defined =
        (opcode == GB_USER_selop_code) ||
        (opcode == GB_USER_idxunop_code) ;

    int64_t nz_thunk = 0 ;
    GrB_Type ttype = NULL ;

    if (Thunk != NULL)
    {
        // finish any pending work on the Thunk
        ttype = Thunk->type ;
        GB_MATRIX_WAIT (Thunk) ;
        nz_thunk = GB_nnz ((GrB_Matrix) Thunk) ;
        ASSERT (nz_thunk == 0 || nz_thunk == 1) ;
        tcode = ttype->code ;

        // if op is positional, Thunk must be compatible with GrB_INT64 (both
        // GxB_SelectOp and GrB_IndexUnaryOp)
        if (op_is_positional && !GB_Type_compatible (GrB_INT64, ttype))
        { 
            // Thunk not a built-in type, for a built-in select operator
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "Incompatible type for C=%s(A,Thunk):\n"
                "input Thunk type [%s]\n"
                "not compatible with GrB_INT64 input to built-in operator %s",
                op->name, ttype->name, op->name) ;
        }

        // if op is (NE, EQ, GT, GE, LT, LE)_THUNK, then Thunk must be
        // compatible with the matrix type
        if (op_is_thunk_comparator)
        {
            if (op_is_selectop && !GB_Type_compatible (A->type, ttype))
            { 
                GB_ERROR (GrB_DOMAIN_MISMATCH,
                    "Incompatible type for C=%s(A,Thunk):\n"
                    "input A type [%s] and Thunk type [%s] not compatible",
                    op->name, A->type->name, ttype->name) ;
            }
        }
    }

    if (op_is_idxunop)
    {
        // check the GrB_IndexUnaryOp
        if (nz_thunk == 0)
        { 
            // Thunk cannot be empty for GrB_select
            GB_ERROR (GrB_EMPTY_OBJECT, "Thunk for C=%s(A,Thunk)"
                " cannot be an empty scalar\n", op->name) ;
        }
        if (!GB_Type_compatible (GrB_BOOL, op->ztype))
        { 
            // GrB_IndexUnaryOp ztype must be compatible with GrB_BOOL
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "Output of user-defined IndexUnaryOp %s is %s\n"
                "which cannot be typecasted to bool\n",
                op->name, op->ztype->name) ;
        }
        if (!GB_Type_compatible (ttype, op->ytype))
        { 
            // Thunk must be typecasted to the op->ytype
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "Incompatible type for C=%s(A,Thunk):\n"
                "input Thunk type [%s] and op thunk type [%s]"
                " not compatible",
                op->name, ttype->name, op->ytype->name) ;
        }
    }
    else if (op_is_user_defined)
    {
        // for a user-defined selectop, Thunk must match the op->ytype exactly
        if (op->ytype == NULL && Thunk != NULL)
        { 
            // select operator does not take a Thunk, but one is present
            GB_ERROR (GrB_DOMAIN_MISMATCH,
                "User-defined operator %s(A,Thunk) does not take a Thunk\n"
                "input, but Thunk parameter is non-NULL", op->name) ;
        }
        else if (op->ytype != NULL && Thunk == NULL)
        { 
            // select operator takes a Thunk, but Thunk parameter is missing
            GB_ERROR (GrB_NULL_POINTER,
                "Required argument is null: [%s]", "Thunk") ;
        }
        else if (op->ytype != NULL && Thunk != NULL)
        {
            // select operator takes a Thunk, and it is present on input.
            // The types must match exactly.
            if (op->ytype != ttype)
            { 
                GB_ERROR (GrB_DOMAIN_MISMATCH,
                    "User-defined operator %s(A,Thunk) has a Thunk input\n"
                    "type of [%s], which must exactly match the type of the\n"
                    "Thunk parameter; parameter has type [%s]",
                    op->name, op->ytype->name, ttype->name) ;
            }
            if (nz_thunk != 1)
            { 
                GB_ERROR (GrB_EMPTY_OBJECT,
                    "User-defined operator %s(A,Thunk) has a Thunk input,\n"
                    "which must not be empty", op->name) ;
            }
        }
    }

    // quick return if an empty mask is complemented
    GB_RETURN_IF_QUICK_MASK (C, C_replace, M, Mask_comp, Mask_struct) ;

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

    ASSERT_SCALAR_OK_OR_NULL (Thunk, "Thunk now GB_select", GB0) ;

    // if A is boolean, get the value of Thunk typecasted to boolean
    bool bthunk = false ;

    // if Thunk is not present, or has no entries, then k defaults to zero
    int64_t ithunk = 0 ;        // ithunk = (int64_t) Thunk (0)

    if (nz_thunk > 0 && GB_Type_compatible (GrB_INT64, ttype))
    { 
        // ithunk = (int64_t) Thunk
        GB_cast_scalar (&ithunk, GB_INT64_code, Thunk->x, tcode,
            sizeof (int64_t)) ;
        // bthunk = (bool) Thunk
        bthunk = (ithunk != 0) ;
    }

    bool thunk_is_zero = false ;
    if (nz_thunk > 0)
    { 
        thunk_is_zero = !GB_is_nonzero ((GB_void *) Thunk->x, ttype->size) ;
    }

    bool make_copy = false ;
    bool is_empty = false ;

    if (op_is_positional)
    { 

        //----------------------------------------------------------------------
        // replace idxunop with their corresponding selectop positional ops
        //----------------------------------------------------------------------

        switch (opcode)
        {
            case GB_TRIL_idxunop_code : 
                opcode = GB_TRIL_selop_code ;
                break ;
            case GB_TRIU_idxunop_code : 
                opcode = GB_TRIU_selop_code ;
                break ;
            case GB_DIAG_idxunop_code : 
                opcode = GB_DIAG_selop_code ;
                break ;
            case GB_DIAGINDEX_idxunop_code : 
            case GB_OFFDIAG_idxunop_code : 
                opcode = GB_OFFDIAG_selop_code ;
                break ;
            default:;
        }

        // The only remaining positional GrB_IndexUnaryOps are:
        // ROWINDEX, COLINDEX, COLLE, COLGT, ROWLE, and ROWGT.

        // These GrB_IndexUnaryOps are distinct from any GxB_SelectOps, but act
        // just like all positional GxB_SelectOps (TRIL, TRIU, etc), so the
        // same code base can handle both kinds of positional ops.  No
        // typecasting needs to be performed.

        //----------------------------------------------------------------------
        // tril, triu, diag, offdiag, ...: get k and handle the flip
        //----------------------------------------------------------------------

        // The built-in operators are modified so they can always work as if A
        // were in CSC format.  If A is not in CSC, then the operation is
        // flipped.

        if (flipij)
        { 
            switch (opcode)
            {
                // TRIL becomes TRIU with thunk negated
                case GB_TRIL_selop_code : 
                    ithunk = -ithunk ;
                    opcode = GB_TRIU_selop_code ;
                    break ;

                // TRIU becomes TRIL with thunk negated
                case GB_TRIU_selop_code : 
                    ithunk = -ithunk ;
                    opcode = GB_TRIL_selop_code ;
                    break ;

                // DIAG and OFFDIAG: same opcode, but negate the thunk
                case GB_DIAG_selop_code : 
                case GB_OFFDIAG_selop_code : 
                    ithunk = -ithunk ;
                    break ;

                // ROWINDEX becomes COLINDEX
                case GB_ROWINDEX_idxunop_code  : 
                    // i+thunk becomes j+thunk: no change to thunk
                    opcode = GB_COLINDEX_idxunop_code ;
                    break ;

                // COLINDEX becomes ROWINDEX
                case GB_COLINDEX_idxunop_code  : 
                    // j+thunk becomes i+thunk: no change to thunk
                    opcode = GB_ROWINDEX_idxunop_code ;
                    break ;

                // COLLE becomes ROWLE
                case GB_COLLE_idxunop_code : 
                    // j <= thunk becomes i <= thunk: no change to thunk
                    opcode = GB_ROWLE_idxunop_code ;
                    break ;

                // COLGT becomes ROWGT
                case GB_COLGT_idxunop_code : 
                    // j > thunk becomes i > thunk: no change to thunk
                    opcode = GB_ROWGT_idxunop_code ;
                    break ;

                // ROWLE becomes COLLE
                case GB_ROWLE_idxunop_code : 
                    // i <= thunk becomes j <= thunk: no change to thunk
                    opcode = GB_COLLE_idxunop_code ;
                    break ;

                // ROWGT becomes COLGT
                case GB_ROWGT_idxunop_code : 
                    // i > thunk becomes j > thunk: no change to thunk
                    opcode = GB_COLGT_idxunop_code ;
                    break ;

                default:;
            }

            flipij = false ;
        }

        // flipij is now false for any positional operator

    }
    else
    {

        //----------------------------------------------------------------------
        // replace GrB_IndexUnaryOp with GxB_SelectOp if no typecasting
        //----------------------------------------------------------------------

        if (op_is_idxunop && A->type == op->xtype)
        { 
            // If there is no typecasting of the A matrix, the VALUE* operators
            // are identical to their selectop counterparts.  The GxB_SelectOps
            // never typecast the input matrix A, but do their tests on the
            // original type of A, after typecasting the thunk scalar to the
            // type of A if needed.  If A->type and op->xtype are the same for
            // a GrB_IndexUnaryOp, then no typecasting occurs at all, neither
            // for A nor the thunk scalar.  After this conversion, the VALUE*
            // operators are only applied in the generic select method.
            switch (opcode)
            {
                case GB_VALUENE_idxunop_code : 
                    opcode = GB_NE_THUNK_selop_code ;
                    break ;
                case GB_VALUEEQ_idxunop_code : 
                    opcode = GB_EQ_THUNK_selop_code ;
                    break ;
                case GB_VALUEGT_idxunop_code : 
                    opcode = GB_GT_THUNK_selop_code ;
                    break ;
                case GB_VALUEGE_idxunop_code : 
                    opcode = GB_GE_THUNK_selop_code ;
                    break ;
                case GB_VALUELT_idxunop_code : 
                    opcode = GB_LT_THUNK_selop_code ;
                    break ;
                case GB_VALUELE_idxunop_code : 
                    opcode = GB_LE_THUNK_selop_code ;
                    break ;
                default:;
            }
        }

        //----------------------------------------------------------------------
        // rename THUNK comparators if thunk is zero
        //----------------------------------------------------------------------

        if (thunk_is_zero)
        { 
            switch (opcode)
            {
                case GB_NE_THUNK_selop_code : 
                    opcode = GB_NONZERO_selop_code ;
                    break ;
                case GB_EQ_THUNK_selop_code : 
                    opcode = GB_EQ_ZERO_selop_code ;
                    break ;
                case GB_GT_THUNK_selop_code : 
                    opcode = GB_GT_ZERO_selop_code ;
                    break ;
                case GB_GE_THUNK_selop_code : 
                    opcode = GB_GE_ZERO_selop_code ;
                    break ;
                case GB_LT_THUNK_selop_code : 
                    opcode = GB_LT_ZERO_selop_code ;
                    break ;
                case GB_LE_THUNK_selop_code : 
                    opcode = GB_LE_ZERO_selop_code ;
                    break ;
                default:;
            }
        }

        //----------------------------------------------------------------------
        // (NE, EQ, GT, GE, LT, LE) x (0, thunk): handle bool and uint cases
        //----------------------------------------------------------------------

        switch (opcode)
        {

            case GB_GT_ZERO_selop_code   :  // A(i,j) > 0

                // bool and uint: rename GxB_GT_ZERO to GxB_NONZERO
                switch (acode)
                {
                    case GB_BOOL_code   :   // C is iso, if boolean
                    case GB_UINT8_code  :   // C is not iso if uint*
                    case GB_UINT16_code : 
                    case GB_UINT32_code : 
                    case GB_UINT64_code : 
                        opcode = GB_NONZERO_selop_code ; break ;
                    default: ;
                }
                break ;

            case GB_GE_ZERO_selop_code   :  // A(i,j) >= 0

                // bool and uint: always true; use GB_dup_worker
                switch (acode)
                {
                    case GB_BOOL_code   : 
                    case GB_UINT8_code  : 
                    case GB_UINT16_code : 
                    case GB_UINT32_code : 
                    case GB_UINT64_code : 
                        make_copy = true ; break ;
                    default: ;
                }
                break ;

            case GB_LT_ZERO_selop_code   :  // A(i,j) < 0

                // bool and uint: always false; return an empty matrix
                switch (acode)
                {
                    case GB_BOOL_code   : 
                    case GB_UINT8_code  : 
                    case GB_UINT16_code : 
                    case GB_UINT32_code : 
                    case GB_UINT64_code : 
                        is_empty = true ; break ;
                    default: ;
                }
                break ;

            case GB_LE_ZERO_selop_code   :  // A(i,j) <= 0

                // bool and uint: rename GxB_LE_ZERO to GxB_EQ_ZERO
                switch (acode)
                {
                    case GB_BOOL_code   : 
                    case GB_UINT8_code  : 
                    case GB_UINT16_code : 
                    case GB_UINT32_code : 
                    case GB_UINT64_code : 
                        // C is iso for boolean and uint* cases
                        opcode = GB_EQ_ZERO_selop_code ; break ;
                    default: ;
                }
                break ;

            case GB_NE_THUNK_selop_code   : // A(i,j) != thunk

                // bool: if thunk is true,  rename GxB_NE_THUNK to GxB_EQ_ZERO 
                //       if thunk is false, rename GxB_NE_THUNK to GxB_NONZERO 
                if (acode == GB_BOOL_code)
                { 
                    // C is iso boolean, in both cases
                    opcode = (bthunk) ?
                        GB_EQ_ZERO_selop_code : GB_NONZERO_selop_code ;
                }
                break ;

            case GB_EQ_THUNK_selop_code   : // A(i,j) == thunk

                // bool: if thunk is true,  rename GxB_NE_THUNK to GxB_NONZERO 
                //       if thunk is false, rename GxB_NE_THUNK to GxB_EQ_ZERO 
                if (acode == GB_BOOL_code)
                { 
                    // C is iso boolean, in both cases
                    opcode = (bthunk) ?
                        GB_NONZERO_selop_code : GB_EQ_ZERO_selop_code ;
                }
                break ;

            case GB_GT_THUNK_selop_code   : // A(i,j) > thunk

                // bool: if thunk is true,  return an empty matrix
                //       if thunk is false, rename GxB_GT_THUNK to GxB_NONZERO
                // user type: return error above
                if (acode == GB_BOOL_code)
                {
                    if (bthunk)
                    { 
                        is_empty = true ;
                    }
                    else
                    { 
                        // C is iso boolean
                        opcode = GB_NONZERO_selop_code ;
                    }
                }
                break ;

            case GB_GE_THUNK_selop_code   : // A(i,j) >= thunk

                // bool: if thunk is true,  rename GxB_GE_THUNK to GxB_NONZERO
                //       if thunk is false, use GB_dup_worker
                // user type: return error above
                if (acode == GB_BOOL_code)
                {
                    if (bthunk)
                    { 
                        // C is iso boolean
                        opcode = GB_NONZERO_selop_code ;
                    }
                    else
                    { 
                        // use dup for GE_THUNK if thunk is false
                        make_copy = true ;
                    }
                }
                break ;

            case GB_LT_THUNK_selop_code   : // A(i,j) < thunk

                // bool: if thunk is true,  rename GxB_LT_THUNK to GxB_EQ_ZERO
                //       if thunk is false, return an empty matrix
                // user type: return error above
                if (acode == GB_BOOL_code)
                {
                    if (bthunk)
                    { 
                        // C is iso boolean
                        opcode = GB_EQ_ZERO_selop_code ;
                    }
                    else
                    { 
                        // matrix empty for LT_THUNK_BOOL, if thunk false
                        is_empty = true ;
                    }
                }
                break ;

            case GB_LE_THUNK_selop_code   : // A(i,j) <= thunk

                // bool: if thunk is true,  use GB_dup_worker
                //       if thunk is false, rename GxB_LE_ZERO to GxB_EQ_ZERO
                // user type: return error
                if (acode == GB_BOOL_code)
                {
                    if (bthunk)
                    { 
                        // use dup for LE_THUNK if thunk is true
                        make_copy = true ;
                    }
                    else
                    { 
                        // C is iso boolean
                        opcode = GB_EQ_ZERO_selop_code ;
                    }
                }
                break ;

            default : ;     // use the opcode as-is
        }
    }

    if (!op_is_user_defined)
    { 
        // flipij can still be true but is only needed for if the op
        // (GrB_IndexUnaryOp or GxB_SelectOp) is user-defined.  So set here it
        // to false for all but user-defined op.
        flipij = false ;
    }

    //--------------------------------------------------------------------------
    // create T
    //--------------------------------------------------------------------------

    GB_CLEAR_STATIC_HEADER (T, &T_header) ;

    if (make_copy)
    { 
        // selectop is always true, so T = A
        // set T->iso = A->iso  OK
        GB_OK (GB_shallow_copy (T, A_csc, A, Context)) ;
    }
    else if (is_empty)
    { 
        // selectop is always false, so T is an empty non-iso matrix
        GB_OK (GB_new (&T, // auto (sparse or hyper), existing header
            A->type, A->vlen, A->vdim, GB_Ap_calloc, A_csc,
            GxB_SPARSE + GxB_HYPERSPARSE, GB_Global_hyper_switch_get ( ),
            1, Context)) ;
    }
    else
    { 
        // T = select (A, Thunk)
        GrB_Scalar Thunk2 = NULL ;
        if (nz_thunk > 0 && (op_is_thunk_comparator || op_is_user_defined))
        {
            // the GrB_Scalar Thunk is passed to GB_selector only if the
            // operator is a thunk comparator (EQ, NE, GT, GE, LT, LE),
            // or if the operator is user-defined.
            Thunk2 = Thunk ;
        }
        GB_OK (GB_selector (
            T,          // output matrix
            opcode,     // opcode of the operator
            op,         // the GB_Operator itself
            flipij,     // if true, flip i and j for user-defined operator
            A,          // input matrix
            ithunk,     // thunk typecasted to int64_t
            Thunk2,     // NULL, or the GrB_Scalar Thunk
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

