//------------------------------------------------------------------------------
// GB_AxB_Gustavson_builtin:  hard-coded C=A*B for built-in types
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This function computes C=A*B with hard-coded versions for all 1040 unique
// built-in semirings that can be constructed with built-in operators.  It also
// handles all non-unique built-in semirings, by renaming operators to
// equivalent ones; if these are included, this function computes C=A*B for all
// possible valid semirings that can be constructed from built-in operators.

#include "GB_mxm.h"
#ifndef GBCOMPACT
#include "GB_AxB__include.h"

GrB_Info GB_AxB_Gustavson_builtin
(
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix M,             // M matrix for C<M> (not complemented)
    const GrB_Matrix A,             // input matrix
    const bool A_is_pattern,        // true if only the pattern of A is used
    const GrB_Matrix B,             // input matrix
    const bool B_is_pattern,        // true if only the pattern of B is used
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Sauna Sauna                  // sparse accumulator
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    #ifdef GB_DEBUG
    GB_Context Context = NULL ;
    #endif
    ASSERT (!GB_aliased (C, M)) ;
    ASSERT (!GB_aliased (C, A)) ;
    ASSERT (!GB_aliased (C, B)) ;

    if (M == NULL)
    {
        // C contains the pattern of C=A*B
        ASSERT_OK (GB_check (C, "C pattern for Gustavson A*B", GB0)) ;
    }
    ASSERT_OK (GB_check (A, "A for Gustavson A*B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for Gustavson A*B", GB0)) ;
    ASSERT (!GB_PENDING (C)) ; ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT_OK (GB_check (semiring, "semiring for Gustavson", GB0)) ;
    ASSERT (C->type == semiring->add->op->ztype) ;

    GB_Opcode mult_opcode, add_opcode ;
    GB_Type_code xycode, zcode ;

    GrB_Info info = GrB_NO_VALUE ;

    // check if the semiring is builtin, and if so, get opcodes and type codes
    if (!GB_AxB_semiring_builtin (A, A_is_pattern, B, B_is_pattern, semiring,
        flipxy, &mult_opcode, &add_opcode, &xycode, &zcode))
    { 
        // no error condition, just not a built-in semiring.
        return (GrB_NO_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // define the worker for the switch factory
    //--------------------------------------------------------------------------

    #define GB_AXB(add,mult,xyname) GB_AgusB_ ## add ## mult ## xyname

    #define GB_AxB_WORKER(add,mult,xyname)                      \
    {                                                           \
        info = GB_AXB (add,mult,xyname) (C, M,                  \
            A, A_is_pattern, B, B_is_pattern, Sauna) ;          \
    }                                                           \
    break ;

    //--------------------------------------------------------------------------
    // launch the switch factory
    //--------------------------------------------------------------------------

    #include "GB_AxB_factory.c"

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (info) ;
}

#endif

