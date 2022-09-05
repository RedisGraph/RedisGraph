//------------------------------------------------------------------------------
// GB_stringify_mxm: build strings for GrB_mxm
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Construct a string defining all macros for semiring, and its name.
// User-defined types are not handled.

#include "GB.h"
#include "GB_stringify.h"

//------------------------------------------------------------------------------
// GB_enumify_mxm: enumerate a GrB_mxm problem
//------------------------------------------------------------------------------

// dot3:  C<M>=A'*B, no accum
// saxpy
// inplace C_in is full/bitmap
//      C_in <M> += A*B     monoid ztype doesn't cast (= accum->ytype)
//      C_in <M>  = A*B     monoid ztype casts to C_in->type
// ...

// accum is not present.  Kernels that use it would require accum to be
// the same as the monoid binary operator.

void GB_enumify_mxm         // enumerate a GrB_mxm problem
(
    // output:    2 x uint64?
    uint64_t *scode,        // unique encoding of the entire semiring
    // input:
    // C matrix:
    bool C_iso,             // if true, semiring is ignored
    int C_sparsity,         // sparse, hyper, bitmap, or full
    GrB_Type ctype,         // C=((ctype) T) is the final typecast
    // M matrix:
    GrB_Matrix M,           // may be NULL
    bool Mask_struct,       // mask is structural
    bool Mask_comp,         // mask is complemented
    // semiring:
    GrB_Semiring semiring,  // the semiring to enumify
    bool flipxy,            // multiplier is: mult(a,b) or mult(b,a)
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
        semiring = GxB_ANY_PAIR_BOOL ;
        flipxy = false ;
    }

    //--------------------------------------------------------------------------
    // get the semiring
    //--------------------------------------------------------------------------

    // GxB_print (semiring, 3) ;
    GrB_Monoid add = semiring->add ;
    GrB_BinaryOp mult = semiring->multiply ;
    GrB_BinaryOp addop = add->op ;

    //--------------------------------------------------------------------------
    // get the types
    //--------------------------------------------------------------------------

    GrB_Type atype = A->type ;
    GrB_Type btype = B->type ;
    GrB_Type mtype = (M == NULL) ? NULL : M->type ;

    GrB_Type xtype = mult->xtype ;
    GrB_Type ytype = mult->ytype ;
    GrB_Type ztype = mult->ztype ;

    GB_Opcode mult_opcode = mult->opcode ;
    GB_Opcode add_opcode  = addop->opcode ;

    GB_Type_code xcode = xtype->code ;
    GB_Type_code ycode = ytype->code ;
    GB_Type_code zcode = ztype->code ;

    // these must always be true for any semiring:
    ASSERT (mult->ztype == addop->ztype) ;
    ASSERT (addop->xtype == addop->ztype && addop->ytype == addop->ztype) ;

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

    if (zcode == GB_BOOL_code)
    {
        // rename the monoid
        add_opcode = GB_boolean_rename (add_opcode) ;
    }

    if (xcode == GB_BOOL_code)  // && (ycode == GB_BOOL_code)
    {
        // rename the multiplicative operator
        mult_opcode = GB_boolean_rename (mult_opcode) ;
    }

    //--------------------------------------------------------------------------
    // handle the flip
    //--------------------------------------------------------------------------

    if (flipxy)
    {
        // z = fmult (b,a) will be computed: handle this by renaming the
        // multiplicative operator, if possible.

        // handle the flip
        bool handled ;
        mult_opcode = GB_flip_binop_code (mult_opcode, &handled) ;

        if (handled)
        {
            // the flip is now handled completely, so discard flipxy
            flipxy = false ;
        }
    }

    // If flipxy is still true, then the multiplier must be used as fmult(b,a)
    // inside the semiring, since it has no flipped equivalent.

    //--------------------------------------------------------------------------
    // determine if A and/or B are value-agnostic
    //--------------------------------------------------------------------------

    // These 1st, 2nd, and pair operators are all handled by the flip, so if
    // flipxy is still true, all of these booleans will be false.
    bool op_is_first  = (mult_opcode == GB_FIRST_binop_code ) ;
    bool op_is_second = (mult_opcode == GB_SECOND_binop_code) ;
    bool op_is_pair   = (mult_opcode == GB_PAIR_binop_code) ;
    bool A_is_pattern = op_is_second || op_is_pair ;
    bool B_is_pattern = op_is_first  || op_is_pair ;

    //--------------------------------------------------------------------------
    // enumify the multiplier
    //--------------------------------------------------------------------------

    int mult_ecode ;
    GB_enumify_binop (&mult_ecode, mult_opcode, xcode, true) ;

    //--------------------------------------------------------------------------
    // enumify the monoid
    //--------------------------------------------------------------------------

    int add_ecode, id_ecode, term_ecode ;
    GB_enumify_monoid (&add_ecode, &id_ecode, &term_ecode, add_opcode, zcode) ;

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

    // GxB_print (mtype, 3) ;
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
    // construct the semiring scode
    //--------------------------------------------------------------------------

    // total scode bits: 62

    (*scode) =
                                            // range        bits

                LSHIFT (A_iso_code , 61) |  // 0 or 1       1
                LSHIFT (B_iso_code , 60) |  // 0 or 1       1

                // monoid
                LSHIFT (add_ecode  , 55) |  // 0 to 22      5
                LSHIFT (id_ecode   , 50) |  // 0 to 31      5
                LSHIFT (term_ecode , 45) |  // 0 to 31      5

                // multiplier, z = f(x,y) or f(y,x)
                LSHIFT (mult_ecode , 37) |  // 0 to 139     8
                LSHIFT (flipxy     , 36) |  // 0 to 1       1
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

//    printf ("serialized_scode: %lu\n", *scode) ;
}

//------------------------------------------------------------------------------
// GB_macrofy_mxm: construct all macros for a semiring
//------------------------------------------------------------------------------

void GB_macrofy_mxm        // construct all macros for GrB_mxm
(
    // input:
    FILE *fp,                   // target file to write, already open
    uint64_t scode
)
{

    //--------------------------------------------------------------------------
    // extract the semiring scode
    //--------------------------------------------------------------------------

    // A and B iso-valued
    int A_iso_code  = RSHIFT (scode, 61, 1) ;
    int B_iso_code  = RSHIFT (scode, 60, 1) ;

    // monoid
    int add_ecode   = RSHIFT (scode, 55, 5) ;
    int id_ecode    = RSHIFT (scode, 50, 5) ;
    int term_ecode  = RSHIFT (scode, 45, 5) ;
    bool is_term    = (term_ecode < 30) ;

    // multiplier
    int mult_ecode  = RSHIFT (scode, 37, 8) ;
    bool flipxy     = RSHIFT (scode, 36, 1) ;
    int zcode       = RSHIFT (scode, 32, 4) ;
    int xcode       = RSHIFT (scode, 28, 4) ;
    int ycode       = RSHIFT (scode, 24, 4) ;

    // mask
    int mask_ecode  = RSHIFT (scode, 20, 4) ;

    // types of C, A, and B
    int ccode       = RSHIFT (scode, 16, 4) ;   // if 0: C is iso
    int acode       = RSHIFT (scode, 12, 4) ;   // if 0: A is pattern
    int bcode       = RSHIFT (scode,  8, 4) ;   // if 0: B is pattern

    // formats of C, M, A, and B
    int csparsity   = RSHIFT (scode,  6, 2) ;
    int msparsity   = RSHIFT (scode,  4, 2) ;
    int asparsity   = RSHIFT (scode,  2, 2) ;
    int bsparsity   = RSHIFT (scode,  0, 2) ;


    //--------------------------------------------------------------------------
    // construct macros to load scalars from A and B (and typecast) them
    //--------------------------------------------------------------------------

    // TODO: these need to be typecasted when loaded.
    // if flipxy false:  A is typecasted to x, and B is typecasted to y.
    // if flipxy true:   A is typecasted to y, and B is typecasted to x.

    int A_is_pattern = (acode == 0) ? 1 : 0 ;
    int B_is_pattern = (bcode == 0) ? 1 : 0 ;

    fprintf (fp, "// GB_mxm_%016" PRIX64 ".h\n", scode) ;
    fprintf (fp, "#define GB_A_IS_PATTERN %d\n", A_is_pattern) ;
    fprintf (fp, "#define GB_A_ISO %d\n", A_iso_code) ;
    fprintf (fp, "#define GB_B_IS_PATTERN %d\n", B_is_pattern) ;
    fprintf (fp, "#define GB_B_ISO %d\n", B_iso_code) ;

    //--------------------------------------------------------------------------
    // construct macros for the multiply
    //--------------------------------------------------------------------------

    const char *s ;
    GB_charify_binop (&s, mult_ecode) ;
    GB_macrofy_binop (fp, "GB_MULT", s, flipxy) ;
    fprintf (fp, "#define GB_FLIPXY %d\n", flipxy ? 1 : 0) ;

    //--------------------------------------------------------------------------
    // construct the monoid macros
    //--------------------------------------------------------------------------

    GB_macrofy_monoid (fp, add_ecode, id_ecode, term_ecode, is_term) ;

    //--------------------------------------------------------------------------
    // special cases
    //--------------------------------------------------------------------------

    // semiring is plus_pair_real
    bool is_plus_pair_real =
        (add_ecode == 11 // plus monoid
        && mult_ecode == 133 // pair multiplicative operator
        && !(zcode == GB_FC32_code || zcode == GB_FC64_code)) ; // real

    fprintf (fp, "#define GB_IS_PLUS_PAIR_REAL_SEMIRING %d\n",
        is_plus_pair_real) ;

    // can ignore overflow in ztype when accumulating the result via the monoid
    bool ztype_ignore_overflow = (
        zcode == GB_INT64_code || zcode == GB_UINT64_code ||
        zcode == GB_FP32_code  || zcode == GB_FP64_code ||
        zcode == GB_FC32_code  || zcode == GB_FC64_code) ;

    // note "CTYPE" is in the name in the CPU kernels (fix them to use ZTYPE)
    fprintf (fp, "#define GB_ZTYPE_IGNORE_OVERFLOW %d\n",
        ztype_ignore_overflow) ;

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

