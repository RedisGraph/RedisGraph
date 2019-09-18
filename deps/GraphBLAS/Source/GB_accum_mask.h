//------------------------------------------------------------------------------
// GB_accum_mask.h: definitions for GB_accum_mask
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The sole purpose of the GB_ACCUM_MASK macro is to simplify the doxygen call
// graph drawings, so that the details of the GB_accum_mask function do not
// appear in the call graphs of its callers:

//      GB_mxm
//      GB_extract
//      GrB_transpose
//      GB_ewise
//      GB_reduce_to_vector
//      GB_kron
//      GB_select
//      GB_apply

// The call graph of GB_accum_mask does appear.  It is complex, since it call
// GB_subassigner, which can then call the entire body of code for the many
// subassign methods.  It also calls GB_add, GB_transpose, and GB_mask, and
// all their dependent functions.

#ifndef GB_ACCUM_MASK_H
#define GB_ACCUM_MASK_H

#include "GB.h"

GrB_Info GB_accum_mask          // C<M> = accum (C,T)
(
    GrB_Matrix C,               // input/output matrix for results
    const GrB_Matrix M_in,      // optional mask for C, unused if NULL
    const GrB_Matrix MT_in,     // MT=M' if computed already in the caller
    const GrB_BinaryOp accum,   // optional accum for Z=accum(C,results)
    GrB_Matrix *Thandle,        // results of computation, freed when done
    const bool C_replace,       // if true, clear C first
    const bool Mask_comp,       // if true, complement the mask
    GB_Context Context
) ;

#define GB_ACCUM_MASK(C, M, MT, accum, Thandle, C_replace, Mask_comp) \
        GB_accum_mask(C, M, MT, accum, Thandle, C_replace, Mask_comp, Context)
#endif

