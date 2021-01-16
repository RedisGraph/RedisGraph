//------------------------------------------------------------------------------
// GB_AxB_pattern: determine if the values of A and B will be used by C=A*B 
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Given the opcode of a multiplicative operator z = mult (x,y), and a flipxy
// flag, determine if C=A*B will use just the pattern of A and/or B.

#include "GB_mxm.h"

void GB_AxB_pattern
(
    // outputs:
    bool *A_is_pattern,     // true if A is pattern-only, because of the mult
    bool *B_is_pattern,     // true if B is pattern-only, because of the mult
    // inputs:
    const bool flipxy,      // if true,  z = mult (b,a) will be computed
                            // if false, z = mult (a,b) will be computed
    const GB_Opcode mult_opcode // opcode of multiply operator
)
{

    //--------------------------------------------------------------------------
    // determine A_is_pattern and B_is_pattern
    //--------------------------------------------------------------------------

    bool op_is_positional = GB_OPCODE_IS_POSITIONAL (mult_opcode) ;
    bool op_is_first  = (mult_opcode == GB_FIRST_opcode) ;
    bool op_is_second = (mult_opcode == GB_SECOND_opcode) ;
    bool op_is_pair   = (mult_opcode == GB_PAIR_opcode) ;

    if (op_is_positional || op_is_pair)
    { 
        // mult (x,y) does not depend on the values of x or y
        (*A_is_pattern) = true ;
        (*B_is_pattern) = true ;
    }
    else if (flipxy)
    { 
        // z = mult (b,a) will be computed
        (*A_is_pattern) = op_is_first  ;
        (*B_is_pattern) = op_is_second ;
    }
    else
    { 
        // z = mult (a,b) will be computed
        (*A_is_pattern) = op_is_second ;
        (*B_is_pattern) = op_is_first  ;
    }
}

