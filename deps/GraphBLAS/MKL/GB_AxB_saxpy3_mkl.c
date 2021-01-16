//------------------------------------------------------------------------------
// GB_AxB_saxpy3_mkl: compute C=A*B, C<M>=A*B, or C<!M>=A*B via MKL_graph
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Compute C=A*B, C<M>=A*B, or C<!M>=A*B via MKL_graph.

// This function is CSR/CSC agnostic, but the comments are writen as if
// all matrices are in CSR format, to match how MKL_graph views its matrices.

#include "GB_mxm.h"
#include "GB_AxB_saxpy3.h"
#include "GB_mkl.h"

#if GB_HAS_MKL_GRAPH

#define GB_MKL_FREE_WORK                        \
    GB_MKL_GRAPH_MATRIX_DESTROY (C_mkl) ;       \
    GB_MKL_GRAPH_MATRIX_DESTROY (M_mkl) ;       \
    GB_MKL_GRAPH_MATRIX_DESTROY (A_mkl) ;       \
    GB_MKL_GRAPH_MATRIX_DESTROY (B_mkl) ;       \

#define GB_MKL_FREE_ALL                         \
    GB_MKL_FREE_WORK                            \
    GB_Matrix_free (C) ;                        \
    GB_FREE (Cp) ;                              \
    GB_FREE (Ci) ;                              \
    GB_FREE (Cx) ;

GrB_Info GB_AxB_saxpy3_mkl          // C = A*B using MKL
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M,             // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, then mask was applied
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;
    ASSERT (*Chandle == NULL) ;

    ASSERT (!GB_ZOMBIES (M)) ; 
    ASSERT (!GB_JUMBLED (M)) ;
    ASSERT (!GB_PENDING (M)) ; 

    ASSERT (!GB_ZOMBIES (A)) ; 
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ; 

    ASSERT (!GB_ZOMBIES (B)) ; 
    ASSERT (!GB_JUMBLED (B)) ;
    ASSERT (!GB_PENDING (B)) ; 

    GrB_Info info ;

    mkl_graph_matrix_t C_mkl = NULL ;
    mkl_graph_matrix_t M_mkl = NULL ;
    mkl_graph_matrix_t A_mkl = NULL ;
    mkl_graph_matrix_t B_mkl = NULL ;

    GrB_Matrix C = NULL ;
    int64_t *GB_RESTRICT Cp = NULL ;
    int64_t *GB_RESTRICT Ci = NULL ;
    GB_void *GB_RESTRICT Cx = NULL ;

    if (!GB_IS_SPARSE (A) || !GB_IS_SPARSE (B)
        || (M != NULL && !GB_IS_SPARSE (M)))
    {
        // MKL does not handle hypersparsity, bitmap, or full
        return (GrB_NO_VALUE) ;
    }

    if (Mask_comp || Mask_struct)
    {
        // MKL only does C=A*B or C<M>=A*B with M as a valued, non-complemented
        // mask.
        return (GrB_NO_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // get the semiring operators and types
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Monoid add = semiring->add ;
    ASSERT (mult->ztype == add->op->ztype) ;
    bool A_is_pattern, B_is_pattern ;
    GB_AxB_pattern (&A_is_pattern, &B_is_pattern, flipxy, mult->opcode) ;

    GB_Opcode mult_opcode, add_opcode ;
    GB_Type_code xcode, ycode, zcode ;
    bool builtin_semiring = GB_AxB_semiring_builtin (A, A_is_pattern, B,
        B_is_pattern, semiring, flipxy, &mult_opcode, &add_opcode, &xcode,
        &ycode, &zcode) ;
//  bool is_any_pair_semiring = builtin_semiring
//      && (add_opcode == GB_ANY_opcode)
//      && (mult_opcode == GB_PAIR_opcode) ;

    //--------------------------------------------------------------------------
    // determine the MKL_graph semiring
    //--------------------------------------------------------------------------

    if (!builtin_semiring)
    {
        // not a GraphBLAS built-in semiring
        return (GrB_NO_VALUE) ;
    }

    int s = GB_AxB_semiring_mkl (add_opcode, mult_opcode, xcode) ;
    if (s < 0)
    {
        // MKL does not have this semiring in its mkl_graph_semiring_t enum
        return (GrB_NO_VALUE) ;
    }

    mkl_graph_semiring_t mkl_semiring = (mkl_graph_semiring_t) s ;
    #if 1
    if (mkl_semiring != MKL_GRAPH_SEMIRING_PLUS_TIMES_INT32)
    {
        // MKL only supports a single semiring in mkl_graph_mxm
        return (GrB_NO_VALUE) ;
    }
    #endif

    //--------------------------------------------------------------------------
    // determine the # of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // construct shallow copies of M, A, and B as CSR matrices
    //--------------------------------------------------------------------------

    if (M != NULL)
    {
        // note: not actually supported by MKL
        return (GrB_NO_VALUE) ;
        /*
        GB_MKL_OK (mkl_graph_matrix_create (&M_mkl)) ;
        GB_MKL_OK (mkl_graph_matrix_set_csr (M_mkl, M->vdim, M->vlen,
            M->p, MKL_GRAPH_TYPE_INT64,
            M->i, MKL_GRAPH_TYPE_INT64,
            M->x, GB_type_mkl (M->type->code))) ;
        */
    }

    GB_MKL_OK (mkl_graph_matrix_create (&B_mkl)) ;
    GB_MKL_OK (mkl_graph_matrix_set_csr (B_mkl, B->vdim, B->vlen,
        B->p, MKL_GRAPH_TYPE_INT64,
        B->i, MKL_GRAPH_TYPE_INT64,
        B->x, GB_type_mkl (B->type->code))) ;

    GB_MKL_OK (mkl_graph_matrix_create (&A_mkl)) ;
    GB_MKL_OK (mkl_graph_matrix_set_csr (A_mkl, A->vdim, A->vlen,
        A->p, MKL_GRAPH_TYPE_INT64,
        A->i, MKL_GRAPH_TYPE_INT64,
        A->x, GB_type_mkl (A->type->code))) ;

    //--------------------------------------------------------------------------
    // C=A*B or C<M>=A*B via MKL
    //--------------------------------------------------------------------------

// note for MKL: figure out how to call mkl_mxv for both dense and sparse v

    GB_MKL_OK (mkl_graph_matrix_create (&C_mkl)) ;
    GBURBLE ("(MKL start) ") ;
    double t = omp_get_wtime ( ) ;
    GB_MKL_OK (mkl_graph_mxm (C_mkl, M_mkl, MKL_GRAPH_ACCUMULATOR_NONE,
        mkl_semiring, A_mkl, B_mkl, NULL,
        MKL_GRAPH_REQUEST_COMPUTE_ALL, MKL_GRAPH_METHOD_AUTO)) ;
    t = omp_get_wtime ( ) - t ;
    GBURBLE ("(MKL time: %g) ", t) ;

    //--------------------------------------------------------------------------
    // get the contents of C
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT Tp = NULL ;
    int64_t *GB_RESTRICT Ti = NULL ;
    GB_void *GB_RESTRICT Tx = NULL ;
    int64_t cnrows, cncols, cnvals ;
    GrB_Type ctype = mult->ztype ;
    mkl_graph_type_t Tp_type, Ti_type, Tx_type ;

    GB_MKL_OK (mkl_graph_matrix_get_csr (C_mkl, &cnrows, &cncols,
        &Tp, &Tp_type, &Ti, &Ti_type, &Tx, &Tx_type)) ;
    GBURBLE ("(got csr) ") ;

    if (Tp == NULL || Ti == NULL || Tx == NULL)
    {
        // bug in mkl_graph_mxm: returns MKL_GRAPH_STATUS_SUCCESS even if
        // the semiring is not supported
        GB_MKL_FREE_ALL ;
        return (GrB_NO_VALUE) ;
    }

    if (Tp_type != MKL_GRAPH_TYPE_INT64 ||
        Ti_type != MKL_GRAPH_TYPE_INT64 ||
        Tx_type != GB_type_mkl (ctype->code))
    {
        GB_MKL_FREE_ALL ;
        return (GrB_PANIC) ;
    }

    cnvals = Tp [cnrows] ;

#if 1
    // Tp, Ti, and Tx are owned by MKL, so sadly a copy must be made.
    Cp = GB_MALLOC (cnrows+1, int64_t) ;
    Ci = GB_MALLOC (cnvals,   int64_t) ;
    Cx = GB_MALLOC (cnvals * ctype->size, GB_void) ;

    if (Cp == NULL || Ci == NULL || Cx == NULL)
    {
        // out of memory
        GB_MKL_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    GB_memcpy (Cp, Tp, (cnrows+1) * sizeof (int64_t), nthreads_max) ;
    GB_memcpy (Ci, Ti, (cnvals  ) * sizeof (int64_t), nthreads_max) ;
    GB_memcpy (Cx, Tx, (cnvals  ) * ctype->size     , nthreads_max) ;
#else

    // a better solution: take them away from MKL
    GB_MKL_OK (mkl_graph_matrix_set_csr (C_mkl, 0, 0,
            NULL, Tp_type, NULL, Ti_type, NULL, Tx_type)) ;

    // now I own them :-)
    Cp = Tp ;
    Ci = Ti ;
    Cx = Tx ;

#endif

    //--------------------------------------------------------------------------
    // free MKL matrices C, M, A, and B
    //--------------------------------------------------------------------------

    GBURBLE ("(copied) ") ;
    GB_MKL_GRAPH_MATRIX_DESTROY (M_mkl) ; GBURBLE ("(freed M) ") ;
    GB_MKL_GRAPH_MATRIX_DESTROY (A_mkl) ; GBURBLE ("(freed A) ") ;
    GB_MKL_GRAPH_MATRIX_DESTROY (B_mkl) ; GBURBLE ("(freed B) ") ;
    GB_MKL_GRAPH_MATRIX_DESTROY (C_mkl) ; GBURBLE ("(freed C) ") ;

    //--------------------------------------------------------------------------
    // import result in C as a CSR matrix
    //--------------------------------------------------------------------------

    // C may be flagged as a CSC matrix in the caller

    info = GB_new (Chandle, // sparse, new header
        ctype, cncols, cnrows, GB_Ap_null, false,
        GxB_SPARSE, B->hyper_switch, cnrows, Context) ;
    if (info != GrB_SUCCESS)
    {
        // out of memory
        return (info) ;
    }

    if (cnvals == 0)
    {
        // free the MKL input Ci and Cx arrays, if they exist
        GB_FREE (Ci) ;
        GB_FREE (Cx) ;
    }

    // transplant the MKL content into the matrix
    C = (*Chandle) ;
    C->h = NULL ;
    C->p = Cp ;
    C->i = Ci ;
    C->x = Cx ;
    C->nzmax = cnvals ;
    C->plen = cnrows ;
    C->nvec = cnrows ;
    C->magic = GB_MAGIC ;
    C->nvec_nonempty = -1 ;     // not computed; delay until required
    C->jumbled = false ;        // assume MKL returns a non-jumbled matrix

    //--------------------------------------------------------------------------
    // prune empty vectors, free workspace, and return result
    //--------------------------------------------------------------------------

    // MKL never computes C as hypersparse, so skip this step:
    // info = GB_hypermatrix_prune (C, Context) ;
    GBURBLE ("(done) ") ;
    if (info == GrB_SUCCESS) { ASSERT_MATRIX_OK (C, "mkl: output", GB0) ; }
    ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    (*mask_applied) = (M != NULL) ;

    return (info) ;
}

#endif

