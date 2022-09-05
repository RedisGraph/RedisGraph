//------------------------------------------------------------------------------
// GB_stringify_ewise: build strings for GrB_eWise* (Add, Mult, and Union)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Construct a string defining all macros for an ewise operation, for
// eWiseAdd, eWiseMult, and eWiseUnion, and its name.
// User-defined types are not handled.

#include "GB.h"
#include "GB_stringify.h"

//------------------------------------------------------------------------------
// GB_enumify_ewise: enumerate a GrB_eWise* problem
//------------------------------------------------------------------------------

// dot3:  C<M>=A'*B, no accum
// saxpy
// inplace C_in is full/bitmap
//      C_in <M> += A*B     monoid ztype doesn't cast (= accum->ytype)
//      C_in <M>  = A*B     monoid ztype casts to C_in->type
// ...

// accum is not present.  Kernels that use it would require accum to be
// the same as the monoid binary operator.

void GB_enumify_ewise         // enumerate a GrB_eWise problem
(
    // output:    2 x uint64?
    uint64_t *ecode,        // unique encoding of the entire operation
    // input:
    // C matrix:
    bool C_iso,             // if true, operator is ignored
    int C_sparsity,         // sparse, hyper, bitmap, or full
    GrB_Type ctype,         // C=((ctype) T) is the final typecast
    // M matrix:
    GrB_Matrix M,           // may be NULL
    bool Mask_struct,       // mask is structural
    bool Mask_comp,         // mask is complemented
    // operator:
    GrB_BinaryOp binaryop,  // the binary operator to enumify
    // A and B:
    GrB_Matrix A,
    GrB_Matrix B
)
{

    //--------------------------------------------------------------------------
    // handle the C_iso case
    //--------------------------------------------------------------------------

    if (C_iso)
    {
        // values of C are not computed by the kernel
        binaryop = GxB_PAIR_BOOL ;
    }

    //--------------------------------------------------------------------------
    // get the types
    //--------------------------------------------------------------------------

    GrB_Type atype = A->type ;
    GrB_Type btype = B->type ;
    GrB_Type mtype = (M == NULL) ? NULL : M->type ;

    GrB_Type xtype = binaryop->xtype ;
    GrB_Type ytype = binaryop->ytype ;
    GrB_Type ztype = binaryop->ztype ;

    GB_Opcode binaryop_opcode = binaryop->opcode ;

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

    int binaryop_ecode ;
    GB_enumify_binop (&binaryop_ecode, binaryop_opcode, xcode, true) ;

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
    // construct the ewise ecode
    //--------------------------------------------------------------------------

    // total ecode bits: 46

    printf ("before: "
            " binaryop_ecode: %d, zcode: %d, xcode: %d, ycode: %d\n"
            ", mask_ecode: %d, ccode: %d, acode: %d, bcode: %d, \n"
            "csparsity: %d, msparsity: %d, asparsity: %d, bsparsity: %d\n",
            binaryop_ecode, zcode, xcode, ycode, mask_ecode, ccode, acode,
            bcode, csparsity, msparsity, asparsity, bsparsity) ;

    (*ecode) =
                                            // range        bits

                LSHIFT (A_iso_code , 45) |  // 0 or 1       1
                LSHIFT (B_iso_code , 44) |  // 0 or 1       1

                // binaryop, z = f(x,y)
                LSHIFT (binaryop_ecode, 36) |  // 0 to 139     8
                LSHIFT (zcode      , 32) |  // 0 to 14      4
                LSHIFT (xcode      , 28) |  // 0 to 14      4
                LSHIFT (ycode      , 24) |  // 0 to 14      4

                // mask
                LSHIFT (mask_ecode , 20) |  // 0 to 13      4

                // types of C, A, and B (bool, int*, uint*, etc)
                LSHIFT (ccode      , 16) |  // 0 to 14      4
                LSHIFT (acode      , 12) |  // 0 to 14      4
                LSHIFT (bcode      ,  8) |  // 0 to 14      4

                // sparsity structures of C, M, A, and B
                LSHIFT (csparsity  ,  6) |  // 0 to 3       2
                LSHIFT (msparsity  ,  4) |  // 0 to 3       2
                LSHIFT (asparsity  ,  2) |  // 0 to 3       2
                LSHIFT (bsparsity  ,  0) ;  // 0 to 3       2

    printf ("binaryop_ecode: %lu\n", *ecode) ;
}

//------------------------------------------------------------------------------
// GB_macrofy_ewise: construct all macros for a semiring
//------------------------------------------------------------------------------

void GB_macrofy_ewise           // construct all macros for GrB_eWise
(
    // input:
    FILE *fp,                   // target file to write, already open
    uint64_t ecode
)
{

    printf ("ecode in macrofy_ewise: %lu\n", ecode) ;

    //--------------------------------------------------------------------------
    // extract the binaryop ecode
    //--------------------------------------------------------------------------

    // A and B iso-valued
    int A_iso_code  = RSHIFT (ecode, 45, 1) ;
    int B_iso_code  = RSHIFT (ecode, 44, 1) ;

    // binary operator
    int binaryop_ecode  = RSHIFT (ecode, 36, 8) ;
    int zcode       = RSHIFT (ecode, 32, 4) ;
    int xcode       = RSHIFT (ecode, 28, 4) ;
    int ycode       = RSHIFT (ecode, 24, 4) ;

    // mask
    int mask_ecode  = RSHIFT (ecode, 20, 4) ;

    // types of C, A, and B
    int ccode       = RSHIFT (ecode, 16, 4) ;   // if 0: C is iso
    int acode       = RSHIFT (ecode, 12, 4) ;   // if 0: A is pattern
    int bcode       = RSHIFT (ecode,  8, 4) ;   // if 0: B is pattern

    // formats of C, A, and B
    int csparsity   = RSHIFT (ecode,  6, 2) ;
    int msparsity   = RSHIFT (ecode,  4, 2) ;
    int asparsity   = RSHIFT (ecode,  2, 2) ;
    int bsparsity   = RSHIFT (ecode,  0, 2) ;

    printf ("before: "
            " binaryop_ecode: %d, zcode: %d, xcode: %d, ycode: %d\n"
            ", mask_ecode: %d, ccode: %d, acode: %d, bcode: %d, \n"
            "csparsity: %d, msparsity: %d, asparsity: %d, bsparsity: %d\n",
            binaryop_ecode, zcode, xcode, ycode, mask_ecode, ccode, acode,
            bcode, csparsity, msparsity, asparsity, bsparsity) ;

    //--------------------------------------------------------------------------
    // construct macros to load scalars from A and B (and typecast) them
    //--------------------------------------------------------------------------

    int A_is_pattern = (acode == 0) ? 1 : 0 ;
    int B_is_pattern = (bcode == 0) ? 1 : 0 ;

    fprintf (fp, "// GB_ewise_%016" PRIX64 ".h\n", ecode) ;
    fprintf (fp, "#define GB_A_IS_PATTERN %d\n", A_is_pattern) ;
    fprintf (fp, "#define GB_A_ISO %d\n", A_iso_code) ;
    fprintf (fp, "#define GB_B_IS_PATTERN %d\n", B_is_pattern) ;
    fprintf (fp, "#define GB_B_ISO %d\n", B_iso_code) ;

    //--------------------------------------------------------------------------
    // construct macros for the binary operator
    //--------------------------------------------------------------------------

    const char *s ;
    GB_charify_binop (&s, binaryop_ecode) ;
    GB_macrofy_binop (fp, "GB_BINOP", s, false) ;

    //--------------------------------------------------------------------------
    // macro to typecast the result back into C
    //--------------------------------------------------------------------------

    bool C_iso = (ccode == 0) ;
    if (C_iso)
    {
        fprintf (fp, "#define GB_PUTC(blob)\n") ;
        fprintf (fp, "#define GB_C_ISO 1\n") ;
    }
    else
    {
        fprintf (fp, "#define GB_PUTC(blob) blob\n") ;
        fprintf (fp, "#define GB_C_ISO 0\n") ;
    }

    //--------------------------------------------------------------------------
    // construct the macros to access the mask (if any), and its name
    //--------------------------------------------------------------------------

    GB_macrofy_mask (fp, mask_ecode) ;

    //--------------------------------------------------------------------------
    // determine the sparsity formats of C, M, A, and B
    //--------------------------------------------------------------------------

    GB_macrofy_sparsity (fp, "C", csparsity) ;
    GB_macrofy_sparsity (fp, "M", msparsity) ;
    GB_macrofy_sparsity (fp, "A", asparsity) ;
    GB_macrofy_sparsity (fp, "B", bsparsity) ;
}

