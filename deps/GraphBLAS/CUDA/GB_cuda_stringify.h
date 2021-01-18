// SPDX-License-Identifier: Apache-2.0
//------------------------------------------------------------------------------
// GB_cuda_stringify.h: prototype definitions for using C helpers 
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is #include'd only in the GraphBLAS/CUDA/GB_cuda*.cu source files.

#ifndef GB_CUDA_STRINGIFY_H
#define GB_CUDA_STRINGIFY_H

// length of strings for building semiring code and names
#define GB_CUDA_STRLEN 2048

void GB_cuda_stringify_terminal // return strings to check terminal
(
    // outputs:
    bool *is_monoid_terminal,
    char *terminal_condition,
    char *terminal_statement,
    // inputs:
    const char *macro_condition_name,
    const char *macro_statement_name,
    GB_Opcode opcode,    // must be a built-in binary operator from a monoid
    GB_Type_code zcode   // op->ztype->code
) ;

const char *GB_cuda_stringify_mask
(
    const GB_Type_code M_type_code,
    bool mask_is_structural
) ;

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
) ;

void GB_cuda_stringify_binop
(
    // output:
    char *code_string,  // string with the #define macro
    // input:
    const char *macro_name,   // name of macro to construct
    GB_Opcode opcode,   // opcode of GraphBLAS operator to convert into a macro
    GB_Type_code zcode  // op->ztype->code of the operator
) ;

void GB_cuda_stringify_load    // return a string to load/typecast macro
(
    // output:
    char *result,
    // input:
    const char *macro_name,       // name of macro to construct
    bool is_pattern         // if true, load/cast does nothing
) ;

void GB_cuda_stringify_identity        // return string for identity value
(
    // output:
    char *code_string,  // string with the #define macro
    // input:
    GB_Opcode opcode,     // must be a built-in binary operator from a monoid
    GB_Type_code zcode
) ;

const char *GB_cuda_stringify_opcode
(
    GB_Opcode opcode    // opcode of GraphBLAS operator
) ;

GB_Opcode GB_binop_flip     // flipped opcode, or same opcode if not flipped
(
    GB_Opcode opcode        // opcode to flip
) ;

#endif
