// SPDX-License-Identifier: Apache-2.0

// Construct a string defining a semiring.
// User-defined types are not handled.

#include "GB.h"
#include "GB_cuda_stringify.h"

void GB_cuda_stringify_semiring     // build a semiring (name and code)
(
    // input:
    GrB_Semiring semiring,  // the semiring to stringify
    bool flipxy,            // multiplier is: mult(a,b) or mult(b,a)
    GrB_Type ctype,         // the type of C
    GrB_Type atype,         // the type of A
    GrB_Type btype,         // the type of B
    GrB_Type mtype,         // the type of M, or NULL if no mask
    bool Mask_struct,       // mask is structural
    bool mask_in_semiring_name, // if true, then the semiring_name includes
                                // the mask_name.  If false, then semiring_name
                                // is independent of the mask_name
    // output: (all of size at least GB_CUDA_LEN+1)
    char *semiring_name,    // name of the semiring
    char *semiring_code,    // List of types and macro defs
    char *mask_name         // definition of mask data load
)
{

    // check inputs
    ASSERT (semiring->object_kind == GB_BUILTIN) ;

    // get the semiring
    GrB_Monoid add = semiring->add ;
    GrB_BinaryOp mult = semiring->multiply ;
    GrB_BinaryOp addop = add->op ;
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

    // for now, this is true for all built-in binops:
    ASSERT (xcode == ycode) ;

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
        // multiplicative operator

        // handle the flip
        mult_opcode = GB_binop_flip (mult_opcode) ;

        // the flip is now handled completely.  This assumes xtype and ytype
        // are the same for all built-in operators.  If this changes, the
        // types will have to be flipped too.
        flipxy = false ;
    }

    //--------------------------------------------------------------------------
    // determine if A and/or B are value-agnostic
    //--------------------------------------------------------------------------

    bool op_is_first  = (mult_opcode == GB_FIRST_opcode ) ;
    bool op_is_second = (mult_opcode == GB_SECOND_opcode) ;
    bool op_is_pair   = false ; // (mult_opcode == GB_PAIR_opcode) ;
    bool A_is_pattern = op_is_second || op_is_pair ;
    bool B_is_pattern = op_is_first  || op_is_pair ;

    //--------------------------------------------------------------------------
    // construct macros to load scalars from A and B (and typecast) them
    //--------------------------------------------------------------------------

    char acast [GB_CUDA_STRLEN+1] ;
    char bcast [GB_CUDA_STRLEN+1] ;
    GB_cuda_stringify_load (acast, "GB_GETA", A_is_pattern) ;
    GB_cuda_stringify_load (bcast, "GB_GETB", B_is_pattern) ;

    //--------------------------------------------------------------------------
    // construct macros for the multiply
    //--------------------------------------------------------------------------

    char mult_function [GB_CUDA_STRLEN+1] ;
    GB_cuda_stringify_binop (mult_function, "GB_MULT", mult_opcode, zcode) ;

    //--------------------------------------------------------------------------
    // construct the monoid macros
    //--------------------------------------------------------------------------

    char add_function [GB_CUDA_STRLEN+1] ;
    GB_cuda_stringify_binop (add_function, "GB_ADD", add_opcode, zcode) ;

    char identity_definition [GB_CUDA_STRLEN+1] ;
    GB_cuda_stringify_identity ( identity_definition, add_opcode, zcode) ;

    bool is_terminal ;
    char terminal_condition [GB_CUDA_STRLEN+1] ;
    char terminal_statement [GB_CUDA_STRLEN+1] ;

    GB_cuda_stringify_terminal (
        &is_terminal, terminal_condition, terminal_statement,
        "GB_TERMINAL_CONDITION", "GB_IF_TERMINAL_BREAK", add_opcode, zcode) ;

    //--------------------------------------------------------------------------
    // macro to typecast the result back into C
    //--------------------------------------------------------------------------

    // for the ANY_PAIR semiring, "c_is_one" will be true, and Cx [0..cnz] will
    // be filled with all 1's later.
    bool c_is_one = false ;
    // TODO:
    // (add_opcode == GB_ANY_opcode && mult_opcode == GB_PAIR_opcode) ;
    char ccast [GB_CUDA_STRLEN+1] ;
    GB_cuda_stringify_load (ccast, "GB_PUTC", c_is_one) ;

    //--------------------------------------------------------------------------
    // construct the macros to access the mask (if any), and its name
    //--------------------------------------------------------------------------

    const char *mask_string = "" ;
    const char *mask_type_name = "" ;
    const char *struct_str = "struct";
    if (mtype != NULL)
    {
        mask_string = GB_cuda_stringify_mask (mtype->code, Mask_struct) ;
        mask_type_name = mtype->name ;
    }
    else
    {
        mask_type_name = struct_str;
    }

    snprintf (mask_name, GB_CUDA_STRLEN, "mask_%s", mask_type_name) ;

    //--------------------------------------------------------------------------
    // build the final semiring code
    //--------------------------------------------------------------------------

    snprintf (semiring_code, GB_CUDA_STRLEN,
        "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
        acast, bcast, mult_function, add_function, identity_definition,
        terminal_condition, terminal_statement, ccast, mask_string) ;

    //--------------------------------------------------------------------------
    // build the final semiring name
    //--------------------------------------------------------------------------

    // the semiring_name depends on:
    // add_opcode
    // mult_opcode
    // ztype->name
    // xtype->name (currently, always == ytype->name, but will change (TODO))
    // ytype->name
    // ctype->name
    // mask_type_name    (but only if mask_in_semiring_name is true)
    // atype->name
    // btype->name

    const char *add_name;
    const char *mult_name;

    add_name  = GB_cuda_stringify_opcode (add_opcode) ;
    mult_name = GB_cuda_stringify_opcode (mult_opcode) ;

//  these are not needed: they are template parameters to the CUDA kernel:
//  ztype->name, xtype->name, ytype->name,
//  ctype->name, atype->name, btype->name

//  ztype->name is required, since the kernel needs it for the identity
//  value.  xtype->name is not strictly required.  However, the GraphBLAS
//  naming scheme is add_mult_xtype, so it is included here.  The ytype
//  and ztype need not be xtype.

    if (mask_in_semiring_name)
    {

        // the format of the semiring name is:
        //
        //  semiring_add_mult_xtype_M_mtype_Z_ztype

        snprintf (semiring_name, GB_CUDA_STRLEN,
            "semiring_%s_%s_%s_M_%s_Z_%s",
            // The first part is akin to GxB_PLUS_TIMES_FP64 (for example),
            // but here this example is semiring_plus_times_double instead:
            add_name, mult_name, xtype->name,
            // these are not in the GrB* or GxB* name, but are needed by CUDA:
            // mask_type_name is (say) 'int64' or 'bool'.
            // ztype is the name of the monoid type.
            mask_type_name, ztype->name) ;

    }
    else
    {

        // the format of the semiring name is:
        //
        //  semiring_add_mult_xtype_Z_ztype

        snprintf (semiring_name, GB_CUDA_STRLEN,
            "semiring_%s_%s_%s_Z_%s",
            // The first part is akin to GxB_PLUS_TIMES_FP64 (for example),
            // but here this example is semiring_plus_times_double instead:
            add_name, mult_name, xtype->name,
            // this is not in the GrB* or GxB* name, but is needed by CUDA:
            // ztype is the name of the monoid type.
            ztype->name) ;

    }

    printf ("semiring_name:\n%s\n", semiring_name) ;
    //printf ("semiring_code:\n%s\n", semiring_code) ;
    //printf ("mask_name:    \n%s\n", mask_name) ;
}

