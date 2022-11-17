//------------------------------------------------------------------------------
// GB_debugify_mxm: dump the definitions for mxm to /tmp/GB_mxm_*.h file
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB.h"
#include "GB_stringify.h"

void GB_debugify_mxm
(
    // C matrix:
    bool C_iso,             // if true, operator is ignored
    int C_sparsity,         // sparse, hyper, bitmap, or full
    GrB_Type ctype,         // C=((ctype) T) is the final typecast
    // M matrix:
    GrB_Matrix M,
    bool Mask_struct,
    bool Mask_comp,
    // semiring:
    GrB_Semiring semiring,
    bool flipxy,
    // A and B matrices:
    GrB_Matrix A,
    GrB_Matrix B
)
{

    uint64_t scode ;

    GrB_Type atype = A->type ;
    GrB_Type btype = B->type ;

    // enumify the mxm problem
    GB_enumify_mxm (&scode, C_iso, C_sparsity, ctype,
        M, Mask_struct, Mask_comp, semiring, flipxy, A, B) ;

    // namify the mxm problem
    char mxm_name [256 + 8*GxB_MAX_NAME_LEN] ;
    GB_namify_problem (mxm_name, scode,
        semiring->add->op->name,
        semiring->multiply->name,
        semiring->multiply->xtype->name,
        semiring->multiply->ytype->name,
        semiring->multiply->ztype->name,
        atype->name,
        btype->name,
        ctype->name) ;

    // construct the filename and create the file
    char filename [512 + 8*GxB_MAX_NAME_LEN] ;
    sprintf (filename, "/tmp/GB_mxm_%s.h", mxm_name);
    FILE *fp = fopen (filename, "w") ;

    // macrofy the mxm problem
    GB_macrofy_mxm (fp, scode, semiring, ctype, atype, btype) ;
    fclose (fp) ;
}

