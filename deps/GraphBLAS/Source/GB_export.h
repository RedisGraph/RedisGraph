//------------------------------------------------------------------------------
// GB_export.h: definitions for import/export
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_EXPORT_H
#define GB_EXPORT_H
#include "GB_transpose.h"

GrB_Info GB_import      // import a matrix in any format
(
    GrB_Matrix *A,      // handle of matrix to create
    GrB_Type type,      // type of matrix to create
    GrB_Index vlen,     // vector length
    GrB_Index vdim,     // vector dimension

    // the 5 arrays:
    GrB_Index **Ap,     // pointers, for sparse and hypersparse formats.
                        // Ap_size >= nvec+1 for hyper, Ap_size >= vdim+1 for
                        // sparse.  Ignored for bitmap and full formats.
    GrB_Index Ap_size,  // size of Ap; ignored if Ap is ignored.
    GrB_Index **Ah,     // vector indices, Ah_size >= nvec for hyper.
                        // Ignored for sparse, bitmap, and full formats.
    GrB_Index Ah_size,  // size of Ah; ignored if Ah is ignored.
    int8_t **Ab,        // bitmap, for bitmap format only, Ab_size >= vlen*vdim.
                        // Ignored for hyper, sparse, and full formats.  
    GrB_Index Ab_size,  // size of Ab; ignored if Ab is ignored.
    GrB_Index **Ai,     // indices, size Ai_size >= nvals(A) for hyper and
                        // sparse formats.  Ignored for bitmap and full.
    GrB_Index Ai_size,  // size of Ai; ignored if Ai is ignored.
    void **Ax,          // values, Ax_size is either 1, or >= nvals(A) for
                        // hyper or sparse formats.  Ax_size >= vlen*vdim for
                        // bitmap or full formats.
    GrB_Index Ax_size,  // size of Ax; never ignored.

    // additional information for specific formats:
    GrB_Index nvals,    // # of entries for bitmap format.
    bool jumbled,       // if true, sparse/hypersparse may be jumbled.
    GrB_Index nvec,     // size of Ah for hypersparse format.

    // information for all formats:
    int sparsity,       // hypersparse, sparse, bitmap, or full
    bool is_csc,        // if true then matrix is by-column, else by-row
    GB_Context Context
) ;

GrB_Info GB_export      // export a matrix in any format
(
    GrB_Matrix *A,      // handle of matrix to export and free
    GrB_Type *type,     // type of matrix to export
    GrB_Index *vlen,    // vector length
    GrB_Index *vdim,    // vector dimension

    // the 5 arrays:
    GrB_Index **Ap,     // pointers, size nvec+1 for hyper, vdim+1 for sparse
    GrB_Index *Ap_size, // size of Ap

    GrB_Index **Ah,     // vector indices, size nvec for hyper
    GrB_Index *Ah_size, // size of Ah

    int8_t **Ab,        // bitmap, size nzmax
    GrB_Index *Ab_size, // size of Ab

    GrB_Index **Ai,     // indices, size nzmax
    GrB_Index *Ai_size, // size of Ai

    void **Ax,          // values, size nzmax
    GrB_Index *Ax_size, // size of Ax (# of entries)

    // additional information for specific formats:
    GrB_Index *nvals,   // # of entries for bitmap format.
    bool *jumbled,      // if true, sparse/hypersparse may be jumbled.
    GrB_Index *nvec,    // size of Ah for hypersparse format.

    // information for all formats:
    int *sparsity,      // hypersparse, sparse, bitmap, or full
    bool *is_csc,       // if true then matrix is by-column, else by-row
    GB_Context Context
) ;

#endif

