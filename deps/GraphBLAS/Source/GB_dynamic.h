//------------------------------------------------------------------------------
// GB_dynamic.h: convert a matrix to/from static/dynamic header
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_do_dynamic_header
(
    // output
    GrB_Matrix *A_dynamic,      // copy of A but with dynamic header
    // input
    GrB_Matrix A,               // input with static or dynamic header
    GB_Context Context
) ;

void GB_undo_dynamic_header
(
    // input
    GrB_Matrix *A_dynamic,      // input matrix with dynamic header
    // output
    GrB_Matrix A,               // output matrix with static or dynamic header
    GB_Context Context
) ;

