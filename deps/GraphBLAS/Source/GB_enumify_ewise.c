//------------------------------------------------------------------------------
// GB_enumify_ewise: enumerate a GrB_eWise* problem
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Enumify an ewise operation: eWiseAdd, eWiseMult, and eWiseUnion.

#include "GB.h"
#include "GB_stringify.h"

// accum is not present.  Kernels that use it would require accum to be
// the same as the monoid binary operator.

void GB_enumify_ewise         // enumerate a GrB_eWise problem
(
    // output:
    uint64_t *scode,        // unique encoding of the entire operation
    // input:
    // C matrix:
    bool C_iso,             // if true, C is iso and the operator is ignored
    int C_sparsity,         // sparse, hyper, bitmap, or full
    GrB_Type ctype,         // C=((ctype) T) is the final typecast
    // M matrix:
    GrB_Matrix M,           // may be NULL
    bool Mask_struct,       // mask is structural
    bool Mask_comp,         // mask is complemented
    // operator:
    GrB_BinaryOp binaryop,  // the binary operator to enumify (can be NULL)
    bool flipxy,            // multiplier is: op(a,b) or op(b,a)
    // A and B:
    GrB_Matrix A,
    GrB_Matrix B
)
{

    //--------------------------------------------------------------------------
    // get the types of A, B, and M
    //--------------------------------------------------------------------------

    GrB_Type atype = A->type ;
    GrB_Type btype = B->type ;
    GrB_Type mtype = (M == NULL) ? NULL : M->type ;

    //--------------------------------------------------------------------------
    // get the types of X, Y, and Z, and handle the C_iso case, and GB_wait
    //--------------------------------------------------------------------------

    GB_Opcode binaryop_opcode ;
    GrB_Type xtype, ytype, ztype ;
    if (C_iso)
    {
        // values of C are not computed by the kernel
        binaryop_opcode = GB_PAIR_binop_code ;
        xtype = GrB_BOOL ;
        ytype = GrB_BOOL ;
        ztype = GrB_BOOL ;
    }
    else if (binaryop == NULL)
    {
        // GB_wait: A and B are disjoint and the operator is not applied
        binaryop_opcode = GB_NOP_code ;
        ASSERT (atype == btype) ;
        ASSERT (ctype == btype) ;
        xtype = atype ;
        ytype = atype ;
        ztype = atype ;
    }
    else
    {
        // normal case
        binaryop_opcode = binaryop->opcode ;
        xtype = binaryop->xtype ;
        ytype = binaryop->ytype ;
        ztype = binaryop->ztype ;
    }

    GB_Type_code xcode = xtype->code ;
    GB_Type_code ycode = ytype->code ;
    GB_Type_code zcode = ztype->code ;

    //--------------------------------------------------------------------------
    // rename redundant boolean operators
    //--------------------------------------------------------------------------

    // consider z = op(x,y) where both x and y are boolean:
    // DIV becomes FIRST
    // RDIV becomes SECOND
    // MIN and TIMES become LAND
    // MAX and PLUS become LOR
    // NE, ISNE, RMINUS, and MINUS become LXOR
    // ISEQ becomes EQ
    // ISGT becomes GT
    // ISLT becomes LT
    // ISGE becomes GE
    // ISLE becomes LE

    if (xcode == GB_BOOL_code)  // && (ycode == GB_BOOL_code)
    {
        // rename the operator
        binaryop_opcode = GB_boolean_rename (binaryop_opcode) ;
    }

    //--------------------------------------------------------------------------
    // determine if A and/or B are value-agnostic
    //--------------------------------------------------------------------------

    // These 1st, 2nd, and pair operators are all handled by the flip, so if
    // flipxy is still true, all of these booleans will be false.
    bool op_is_first  = (binaryop_opcode == GB_FIRST_binop_code ) ;
    bool op_is_second = (binaryop_opcode == GB_SECOND_binop_code) ;
    bool op_is_pair   = (binaryop_opcode == GB_PAIR_binop_code) ;
    bool A_is_pattern = op_is_second || op_is_pair ;
    bool B_is_pattern = op_is_first  || op_is_pair ;

    //--------------------------------------------------------------------------
    // enumify the binary operator
    //--------------------------------------------------------------------------

    int binop_ecode ;
    GB_enumify_binop (&binop_ecode, binaryop_opcode, xcode, true) ;

    //--------------------------------------------------------------------------
    // enumify the types
    //--------------------------------------------------------------------------

    int acode = A_is_pattern ? 0 : atype->code ;   // 0 to 14
    int bcode = B_is_pattern ? 0 : btype->code ;   // 0 to 14
    int ccode = C_iso ? 0 : ctype->code ;          // 0 to 14

    int A_iso_code = A->iso ? 1 : 0 ;
    int B_iso_code = B->iso ? 1 : 0 ;

    //--------------------------------------------------------------------------
    // enumify the mask
    //--------------------------------------------------------------------------

    int mtype_code = (mtype == NULL) ? 0 : mtype->code ; // 0 to 14
    int mask_ecode ;
    GB_enumify_mask (&mask_ecode, mtype_code, Mask_struct, Mask_comp) ;

    //--------------------------------------------------------------------------
    // enumify the sparsity structures of C, M, A, and B
    //--------------------------------------------------------------------------

    int M_sparsity = GB_sparsity (M) ;
    int A_sparsity = GB_sparsity (A) ;
    int B_sparsity = GB_sparsity (B) ;

    int csparsity, msparsity, asparsity, bsparsity ;
    GB_enumify_sparsity (&csparsity, C_sparsity) ;
    GB_enumify_sparsity (&msparsity, M_sparsity) ;
    GB_enumify_sparsity (&asparsity, A_sparsity) ;
    GB_enumify_sparsity (&bsparsity, B_sparsity) ;

    //--------------------------------------------------------------------------
    // construct the ewise scode
    //--------------------------------------------------------------------------

    // total scode bits: 47 (17 unused bits)

    (*scode) =
                                               // range        bits
                // unused (4 hex digits)
//              GB_LSHIFT (0,         , 48) |  // unused       16

                // A and B iso properites, flipxy (1 hex digit)
//              GB_LSHIFT (0          , 47) |  // unused       1
                GB_LSHIFT (A_iso_code , 46) |  // 0 or 1       1
                GB_LSHIFT (B_iso_code , 45) |  // 0 or 1       1
                GB_LSHIFT (flipxy     , 44) |  // 0 or 1       1

                // binaryop, z = f(x,y) (5 hex digits)
                GB_LSHIFT (binop_ecode, 36) |  // 0 to 140     8
                GB_LSHIFT (zcode      , 32) |  // 0 to 14      4
                GB_LSHIFT (xcode      , 28) |  // 0 to 14      4
                GB_LSHIFT (ycode      , 24) |  // 0 to 14      4

                // mask (one hex digit)
                GB_LSHIFT (mask_ecode , 20) |  // 0 to 13      4

                // types of C, A, and B (3 hex digits)
                GB_LSHIFT (ccode      , 16) |  // 0 to 14      4
                GB_LSHIFT (acode      , 12) |  // 0 to 14      4
                GB_LSHIFT (bcode      ,  8) |  // 0 to 14      4

                // sparsity structures of C, M, A, and B (2 hex digits)
                GB_LSHIFT (csparsity  ,  6) |  // 0 to 3       2
                GB_LSHIFT (msparsity  ,  4) |  // 0 to 3       2
                GB_LSHIFT (asparsity  ,  2) |  // 0 to 3       2
                GB_LSHIFT (bsparsity  ,  0) ;  // 0 to 3       2
}

