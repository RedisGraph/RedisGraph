//------------------------------------------------------------------------------
// GxB_mxv_optimize: optimize a matrix for matrix-vector multiply
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_mxm.h"
#include "GB_mkl.h"

#define GB_FREE_ALL ;

#undef  GB_MKL_OK
#define GB_MKL_OK(status)                               \
    if (status != MKL_GRAPH_STATUS_SUCCESS)             \
    {                                                   \
        /* if the analysis fails, return GrB_SUCCESS */ \
        /* anyway, since the analysis is optional */    \
        GB_MKL_GRAPH_MATRIX_DESTROY (C->mkl) ;          \
        return (GrB_SUCCESS) ;                          \
    }

GrB_Info GxB_mxv_optimize           // analyze C for subsequent use in mxv
(
    GrB_Matrix C,                   // input/output matrix
    int64_t ncalls,                 // estimate # of future calls to GrB_mxv
    const GrB_Descriptor desc       // currently unused
)
{
#if GB_HAS_MKL_GRAPH

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE (C, "GxB_mxv_optimize (C, ncols, desc)") ;
    GB_BURBLE_START ("GxB_mxv_optimize") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;

    // get the use_mkl flag from the descriptor
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5, xx6, xx7) ;

    // delete any lingering zombies and assemble any pending tuples
    GB_MATRIX_WAIT (C) ;

    //--------------------------------------------------------------------------
    // optimize the matrix for mkl_graph_mxv in MKL
    //--------------------------------------------------------------------------

    if (use_mkl && !GB_IS_BITMAP (C))
    {

        //----------------------------------------------------------------------
        // free any existing MKL version of the matrix C and its optimization
        //----------------------------------------------------------------------

        GB_MKL_GRAPH_MATRIX_DESTROY (C->mkl) ;

        //----------------------------------------------------------------------
        // create the MKL version of the matrix C, and analyze it
        //----------------------------------------------------------------------

        // note for MKL: doesn't the analysis depend on C'*x or C*x?

        int A_mkl_type = GB_type_mkl (C->type->code) ;
        if (!GB_IS_HYPERSPARSE (C) && A_mkl_type >= 0)
        {

            // create the MKL version of the matrix
            mkl_graph_matrix_t A_mkl = NULL ;
            GB_MKL_OK (mkl_graph_matrix_create (&A_mkl)) ;
            C->mkl = A_mkl ;

            // import the data as shallow arrays into the MKL matrix
            GB_MKL_OK (mkl_graph_matrix_set_csr (A_mkl, C->vdim, C->vlen,
                C->p, MKL_GRAPH_TYPE_INT64,
                C->i, MKL_GRAPH_TYPE_INT64,
                C->x, A_mkl_type)) ;

            // analyze the matrix for future calls to GrB_mxv
            GB_MKL_OK (mkl_graph_optimize_mxv (A_mkl, ncalls)) ;

            // save the analysis inside the GrB_Matrix
            C->mkl = (void *) A_mkl ;
        }

        // note for MKL: if C is modified, C->mkl needs to be freed.

    }

    GB_BURBLE_END ;
#endif
    return (GrB_SUCCESS) ;
}

