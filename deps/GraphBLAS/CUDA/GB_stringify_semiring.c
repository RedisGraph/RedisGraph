//------------------------------------------------------------------------------
// GB_stringify_semiring: build strings for a semiring
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Construct a string defining all macros for semiring, and its name.
// User-defined types are not handled.

#include "GB.h"
#include "GB_stringify.h"

//------------------------------------------------------------------------------
// GB_stringify_semiring: build strings for a semiring
//------------------------------------------------------------------------------

void GB_stringify_semiring     // build a semiring (name and code)
(
    // input:
    FILE *fp,               // File to write macros, assumed open already
    GrB_Semiring semiring,  // the semiring to stringify
    bool flipxy,            // multiplier is: mult(a,b) or mult(b,a)
    GrB_Type ctype,         // the type of C
    GrB_Type mtype,         // the type of M, or NULL if no mask
    GrB_Type atype,         // the type of A
    GrB_Type btype,         // the type of B
    bool Mask_struct,       // mask is structural
    bool Mask_comp,         // mask is complemented
    int C_sparsity,         // sparsity structure of C
    int M_sparsity,         // sparsity structure of M
    int A_sparsity,         // sparsity structure of A
    int B_sparsity          // sparsity structure of B
)
{

    uint64_t scode ;

    printf("Inside stringify semiring\n");

    GB_enumify_semiring (&scode,
        semiring, flipxy,
        ctype, mtype, atype, btype, Mask_struct, Mask_comp,
        C_sparsity, M_sparsity, A_sparsity, B_sparsity) ;
    printf("done enumify semiring\n");

    GB_macrofy_semiring ( fp, scode) ;

    printf("done macrofy semiring\n");
}

//------------------------------------------------------------------------------
// GB_enumify_semiring: enumerate a semiring
//------------------------------------------------------------------------------

void GB_enumify_semiring   // enumerate a semiring
(
    // output:
    uint64_t *scode,        // unique encoding of the entire semiring
    // input:
    GrB_Semiring semiring,  // the semiring to enumify
    bool flipxy,            // multiplier is: mult(a,b) or mult(b,a)
    GrB_Type ctype,         // the type of C
    GrB_Type mtype,         // the type of M, or NULL if no mask
    GrB_Type atype,         // the type of A
    GrB_Type btype,         // the type of B
    bool Mask_struct,       // mask is structural
    bool Mask_comp,         // mask is complemented
    int C_sparsity,         // sparsity structure of C
    int M_sparsity,         // sparsity structure of M
    int A_sparsity,         // sparsity structure of A
    int B_sparsity          // sparsity structure of B
)
{

    //--------------------------------------------------------------------------
    // get the semiring
    //--------------------------------------------------------------------------
    printf("inside enumify: %lu\n", semiring);

    printf("Getting semiring add\n");
    GrB_Monoid add = semiring->add ;

    printf("Getting semiring mult\n");
    GrB_BinaryOp mult = semiring->multiply ;

    printf("Getting semiring add op\n");
    GrB_BinaryOp addop = add->op ;

    printf("Getting types\n");
    GrB_Type xtype = mult->xtype ;
    GrB_Type ytype = mult->ytype ;
    GrB_Type ztype = mult->ztype ;

    printf("Getting opcodes\n");
    GB_Opcode mult_opcode = mult->opcode ;
    GB_Opcode add_opcode  = addop->opcode ;



    printf("Getting typecodes\n");
    GB_Type_code xcode = xtype->code ;
    GB_Type_code ycode = ytype->code ;
    GB_Type_code zcode = ztype->code ;

    printf("Performing asserts\n");
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

    printf("Invoking boolean rename\n");
    if (zcode == GB_BOOL_code)
    {
        // rename the monoid
        add_opcode = GB_boolean_rename (add_opcode) ;
    }

    printf("Invoking boolean rename\n");

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
    printf("Invoking enumify binop\n");

    int mult_ecode ;
    GB_enumify_binop (&mult_ecode, mult_opcode, xcode, true) ;

    //--------------------------------------------------------------------------
    // enumify the monoid
    //--------------------------------------------------------------------------
    printf("Invoking enumify monoid\n");

    int add_ecode, id_ecode, term_ecode ;
    GB_enumify_monoid (&add_ecode, &id_ecode, &term_ecode, add_opcode, zcode ) ;

    //--------------------------------------------------------------------------
    // enumify the types
    //--------------------------------------------------------------------------

    printf("Done invoking enumify monoid\n");


    printf("atype\n");
    int acode = A_is_pattern ? 0 : atype->code ;   // 0 to 14
    printf("btype\n");
    int bcode = B_is_pattern ? 0 : btype->code ;   // 0 to 14
    printf("ctype\n");
    int ccode = ctype->code ;                      // 1 to 14

    //--------------------------------------------------------------------------
    // enumify the mask
    //--------------------------------------------------------------------------

    printf("Invoking enumify_mask, mtype %p\n", mtype);
    int mtype_code = (mtype == NULL) ? 0 : mtype->code ; // 0 to 14
    int mask_ecode ;
    GB_enumify_mask (&mask_ecode, mtype_code, Mask_struct, Mask_comp) ;
    printf ("got mask_ecode: %d\n", mask_ecode) ;

    //--------------------------------------------------------------------------
    // enumify the sparsity structures of C, M, A, and B
    //--------------------------------------------------------------------------

    int csparsity, msparsity, asparsity, bsparsity ;
    GB_enumify_sparsity (&csparsity, C_sparsity) ;
    GB_enumify_sparsity (&msparsity, M_sparsity) ;
    GB_enumify_sparsity (&asparsity, A_sparsity) ;
    GB_enumify_sparsity (&bsparsity, B_sparsity) ;

    //--------------------------------------------------------------------------
    // construct the semiring scode
    //--------------------------------------------------------------------------

    // total scode bits: 60

    printf("coinstructing semiring scode\n");

#define LSHIFT(x,k) (((uint64_t) x) << k)
    // TODO: We need to
    printf("add_ecode: %d, mult_ecode: %d\n", add_ecode, mult_ecode);

    (*scode) =
                                            // range        bits
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

                printf("serialized mask ecode: %d\n", mask_ecode);

                // types of C, A, and B (bool, int*, uint*, etc)
                LSHIFT (ccode      , 16) |  // 1 to 14      4
                LSHIFT (acode      , 12) |  // 0 to 14      4
                LSHIFT (bcode      ,  8) |  // 0 to 14      4

                // sparsity structures of C, A, and B
                LSHIFT (csparsity  ,  6) |  // 0 to 3       2
                LSHIFT (msparsity  ,  4) |  // 0 to 3       2
                LSHIFT (asparsity  ,  2) |  // 0 to 3       2
                LSHIFT (bsparsity  ,  0) ;  // 0 to 3       2


    printf("done enumify semiring\n");

}

//------------------------------------------------------------------------------
// GB_macrofy_semiring: construct all macros for a semiring
//------------------------------------------------------------------------------

void GB_macrofy_semiring   // construct all macros for a semiring
(
    // input:
    FILE *fp,                   // target file to write, already open
    uint64_t scode
)
{

    printf("scode in macrofy_semiring: %lu\n", scode);

    //--------------------------------------------------------------------------
    // extract the semiring scode
    //--------------------------------------------------------------------------

#define RSHIFT(x,k,b) (x >> k) & ((0x00000001 << b) -1)
//#define RSHIFT(x,k,b) (x >> k) & ((((uint64_t) 1) << b) - 1)

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

    printf("deserialized mask ecode: %d\n", mask_ecode);

    // types of C, A, and B
    int acode       = RSHIFT (scode, 16, 4) ;
    int bcode       = RSHIFT (scode, 12, 4) ;
    int ccode       = RSHIFT (scode,  8, 4) ;

    // formats of C, A, and B
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

    bool A_is_pattern = (acode == 0) ;
    bool B_is_pattern = (bcode == 0) ;

    printf("stringify loaders \n");
    GB_stringify_load ( fp, "GB_GETA", A_is_pattern) ;
    GB_stringify_load ( fp, "GB_GETB", B_is_pattern) ;

    //--------------------------------------------------------------------------
    // construct macros for the multiply
    //--------------------------------------------------------------------------

    printf("stringify mult \n");
    const char *s ;
    printf("mult_ecode: %d\n", mult_ecode);
    GB_charify_binop ( &s, mult_ecode) ;
    GB_macrofy_binop ( fp, "GB_MULT", s, flipxy) ;

    //--------------------------------------------------------------------------
    // construct the monoid macros
    //--------------------------------------------------------------------------

    printf("stringify monoid \n");
    GB_macrofy_monoid ( fp, add_ecode, id_ecode, term_ecode, is_term) ;

    //--------------------------------------------------------------------------
    // macro to typecast the result back into C
    //--------------------------------------------------------------------------

    // for the ANY_PAIR semiring, "c_is_one" will be true, and Cx [0..cnz] will
    // be filled with all 1's later.
    bool c_is_one = false ;
    // TODO:
    // (add_ecode == GB_ANY_binop_code && mult_opcode == GB_PAIR_binop_code) ;
    GB_stringify_load ( fp, "GB_PUTC", c_is_one) ;

    //--------------------------------------------------------------------------
    // construct the macros to access the mask (if any), and its name
    //--------------------------------------------------------------------------

    printf("MACROFY MASK!\n");
    GB_macrofy_mask ( fp, mask_ecode);

    //--------------------------------------------------------------------------
    // determine the sparsity formats of C, M, A, and B
    //--------------------------------------------------------------------------

    printf("stringify sparsity \n");
    GB_macrofy_sparsity (fp, "C", csparsity) ;
    GB_macrofy_sparsity (fp, "M", msparsity) ;
    GB_macrofy_sparsity (fp, "A", asparsity) ;
    GB_macrofy_sparsity (fp, "B", bsparsity) ;

}

