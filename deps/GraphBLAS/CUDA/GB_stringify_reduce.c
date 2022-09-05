//------------------------------------------------------------------------------
// GB_stringify_reduce: build strings for GrB_reduce to scalar
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Construct a string defining all macros for reduction to scalar, and its name.
// User-defined types are not handled.

#include "GB.h"
#include "GB_stringify.h"

//------------------------------------------------------------------------------
// GB_enumify_reduce: enumerate a GrB_reduce problem
//------------------------------------------------------------------------------

void GB_enumify_reduce      // enumerate a GrB_reduce problem
(
    // output:
    uint64_t *rcode,        // unique encoding of the entire problem
    // input:
    GrB_Monoid reduce,      // the monoid to enumify
    GrB_Matrix A
)
{

    //--------------------------------------------------------------------------
    // get the monoid and type of A
    //--------------------------------------------------------------------------

    GrB_BinaryOp reduceop = reduce->op ;
    GrB_Type atype = A->type ;
    GrB_Type ztype = reduceop->ztype ;
    GB_Opcode reduce_opcode  = reduceop->opcode ;
    // these must always be true for any monoid:
    ASSERT (reduceop->xtype == reduceop->ztype) ;
    ASSERT (reduceop->ytype == reduceop->ztype) ;

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

    GB_Type_code zcode = ztype->code ;
    if (zcode == GB_BOOL_code)
    {
        // rename the monoid
        reduce_opcode = GB_boolean_rename (reduce_opcode) ;
    }

    //--------------------------------------------------------------------------
    // enumify the monoid
    //--------------------------------------------------------------------------

    int red_ecode, id_ecode, term_ecode ;
    GB_enumify_monoid (&red_ecode, &id_ecode, &term_ecode, reduce_opcode,
        zcode) ;

    //--------------------------------------------------------------------------
    // enumify the type and sparsity structure of A
    //--------------------------------------------------------------------------

    int acode = atype->code ;   // 0 to 14
    int A_sparsity = GB_sparsity (A) ;
    int asparsity ;
    GB_enumify_sparsity (&asparsity, A_sparsity) ;

    //--------------------------------------------------------------------------
    // construct the reduction rcode
    //--------------------------------------------------------------------------

    // total rcode bits: 25

    (*rcode) =
                                            // range        bits
                // monoid
                LSHIFT (red_ecode  , 20) |  // 0 to 22      5
                LSHIFT (id_ecode   , 15) |  // 0 to 31      5
                LSHIFT (term_ecode , 10) |  // 0 to 31      5

                // type of the monoid
                LSHIFT (zcode      ,  6) |  // 0 to 14      4

                // type of A
                LSHIFT (acode      ,  2) |  // 0 to 14      4

                // sparsity structure of A
                LSHIFT (asparsity  ,  0);  // 0 to 3       2
}

//------------------------------------------------------------------------------
// GB_macrofy_reduce: construct all macros for a reduction to scalar
//------------------------------------------------------------------------------

void GB_macrofy_reduce      // construct all macros for GrB_reduce to scalar
(
    // input:
    FILE *fp,               // target file to write, already open
    uint64_t rcode
)
{

    //--------------------------------------------------------------------------
    // extract the reduction rcode
    //--------------------------------------------------------------------------

    // monoid
    int red_ecode   = RSHIFT (rcode, 20, 5) ;
    int id_ecode    = RSHIFT (rcode, 15, 5) ;
    int term_ecode  = RSHIFT (rcode, 10, 5) ;
    bool is_term    = (term_ecode < 30) ;

    // type of the monoid
    int zcode       = RSHIFT (rcode, 6, 4) ;

    // type of A
    int acode       = RSHIFT (rcode, 2, 4) ;

    // format of A
    int asparsity   = RSHIFT (rcode, 0, 2) ;

    //--------------------------------------------------------------------------
    // construct macros to load scalars from A (and typecast) them
    //--------------------------------------------------------------------------

    fprintf (fp, "// GB_reduce_%016" PRIX64 ".h\n", rcode) ;
    fprintf (fp, "#define GB_A_IS_PATTERN 0\n") ;
    fprintf (fp, "#define GB_A_ISO 0\n") ;
    fprintf (fp, "#define GB_B_IS_PATTERN 1\n") ;
    fprintf (fp, "#define GB_B_ISO 1\n") ;
    fprintf (fp, "#define GB_FLIPXY 0\n") ;
    fprintf (fp, "#define T_Y T_Z\n") ;
    fprintf (fp, "#define T_X T_Z\n") ;

    //--------------------------------------------------------------------------
    // construct the monoid macros
    //--------------------------------------------------------------------------

    GB_macrofy_monoid (fp, red_ecode, id_ecode, term_ecode, is_term) ;

    //--------------------------------------------------------------------------
    // macro to typecast the result back into C
    //--------------------------------------------------------------------------

    fprintf (fp, "#define GB_PUTC(blob) blob\n") ;
    fprintf (fp, "#define GB_C_ISO 0\n") ;

    //--------------------------------------------------------------------------
    // determine the sparsity format of A
    //--------------------------------------------------------------------------

    GB_macrofy_sparsity (fp, "A", asparsity) ;
}

