//------------------------------------------------------------------------------
// GB_AxB_dot4_mkl: compute c+=A*b where c and b are dense vectors
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This function is CSR/CSC agnostic, but the comments are writen as if
// all matrices are in CSR format, to match how MKL_graph views its matrices.

#include "GB_mxm.h"
#include "GB_mkl.h"

#if GB_HAS_MKL_GRAPH

#define GB_MKL_FREE_WORK                            \
    GB_FREE (Zx) ;                                  \
    GB_MKL_GRAPH_DESCRIPTOR_DESTROY (mkl_desc) ;    \
    GB_MKL_GRAPH_VECTOR_DESTROY (z_mkl) ;           \
    GB_MKL_GRAPH_VECTOR_DESTROY (b_mkl) ;           \
    if (!A_preanalyzed)                             \
    {                                               \
        GB_MKL_GRAPH_MATRIX_DESTROY (A_mkl) ;       \
    }

#define GB_MKL_FREE_ALL                             \
    GB_MKL_GRAPH_MATRIX_DESTROY (A->mkl) ;          \
    GB_MKL_FREE_WORK

GrB_Info GB_AxB_dot4_mkl            // c += A*b using MKL
(
    GrB_Vector c,                   // input/output vector (dense)
    const GrB_Matrix A,             // input matrix A
    const GrB_Vector B,             // input vector b (dense)
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (!GB_ZOMBIES (c)) ;
    ASSERT (!GB_JUMBLED (c)) ;
    ASSERT (!GB_PENDING (c)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (!GB_JUMBLED (B)) ;
    ASSERT (!GB_PENDING (B)) ;

    ASSERT (!GB_IS_BITMAP (c)) ;
    ASSERT (!GB_IS_BITMAP (A)) ;
    ASSERT (!GB_IS_BITMAP (B)) ;

    //--------------------------------------------------------------------------
    // declare workspace
    //--------------------------------------------------------------------------

    GrB_Info info ;

    mkl_graph_descriptor_t mkl_desc = NULL ;
    mkl_graph_vector_t z_mkl = NULL ;
    mkl_graph_matrix_t A_mkl = NULL ;
    mkl_graph_vector_t b_mkl = NULL ;

    float *GB_RESTRICT Cx = (float *) c->x ;
    float *GB_RESTRICT Zx = NULL ;

    bool A_preanalyzed = (A->mkl != NULL) ;

    //--------------------------------------------------------------------------
    // get the semiring operators and types
    //--------------------------------------------------------------------------

    GrB_BinaryOp mult = semiring->multiply ;
    GrB_Monoid add = semiring->add ;
    ASSERT (mult->ztype == add->op->ztype) ;
    bool A_is_pattern, B_is_pattern ;
    GB_AxB_pattern (&A_is_pattern, &B_is_pattern, false, mult->opcode) ;

    GB_Opcode mult_opcode, add_opcode ;
    GB_Type_code xcode, ycode, zcode ;
    bool builtin_semiring = GB_AxB_semiring_builtin (A, A_is_pattern, B,
        B_is_pattern, semiring, false, &mult_opcode, &add_opcode, &xcode,
        &ycode, &zcode) ;
    ASSERT (builtin_semiring) ;

    ASSERT (xcode == GB_FP32_code) ;
    ASSERT (ycode == GB_FP32_code) ;
    ASSERT (zcode == GB_FP32_code) ;
    ASSERT (mult_opcode == GB_TIMES_opcode || mult_opcode == GB_SECOND_opcode) ;
    ASSERT (add_opcode == GB_PLUS_opcode) ;
    ASSERT (semiring == GrB_PLUS_TIMES_SEMIRING_FP32 ||
            semiring == GxB_PLUS_SECOND_FP32) ;

    //--------------------------------------------------------------------------
    // determine the MKL_graph semiring
    //--------------------------------------------------------------------------

    // PLUS_SECOND becomes PLUS_TIMES with a non-standard descriptor
    mkl_graph_semiring_t mkl_semiring = MKL_GRAPH_SEMIRING_PLUS_TIMES_FP32 ;

    //--------------------------------------------------------------------------
    // determine the # of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // construct shallow copies of A and b
    //--------------------------------------------------------------------------

    int64_t n = B->vlen ;
    GB_MKL_OK (mkl_graph_vector_create (&b_mkl)) ;
    GB_MKL_OK (mkl_graph_vector_set_dense (b_mkl, n,
        B->x, MKL_GRAPH_TYPE_FP32)) ;

    if (A_preanalyzed)
    {
        // A has already been imported into an MKL matrix and analyzed
        A_mkl = A->mkl ;
    }
    else
    {
        // import A into an MKL version of the matrix, to be destroyed when done
        GB_MKL_OK (mkl_graph_matrix_create (&A_mkl)) ;
        GB_MKL_OK (mkl_graph_matrix_set_csr (A_mkl, A->vdim, A->vlen,
            A->p, MKL_GRAPH_TYPE_INT64,
            A->i, MKL_GRAPH_TYPE_INT64,
            A->x, A_is_pattern ?
            MKL_GRAPH_TYPE_BOOL : GB_type_mkl (A->type->code))) ;
    }

    //--------------------------------------------------------------------------
    // create z workspace
    //--------------------------------------------------------------------------

    Zx = (float *) GB_CALLOC (n, float) ;
    if (Zx == NULL)
    {
        // out of memory
        GB_MKL_FREE_ALL ;
        return (GrB_OUT_OF_MEMORY) ;
    }
    GB_MKL_OK (mkl_graph_vector_create (&z_mkl)) ;
    GB_MKL_OK (mkl_graph_vector_set_dense (z_mkl, n, Zx, MKL_GRAPH_TYPE_FP32)) ;

    //--------------------------------------------------------------------------
    // z=A*b via MKL
    //--------------------------------------------------------------------------

    if (mult_opcode == GB_SECOND_opcode)
    {
        // this fails, "not supported"
        GB_MKL_OK (mkl_graph_descriptor_create (&mkl_desc)) ;
        GB_MKL_OK (mkl_graph_descriptor_set_field (mkl_desc,
            GB_MKL_GRAPH_FIELD_FIRST_INPUT, GB_MKL_GRAPH_MOD_ONLY_STRUCTURE)) ;
    }

    double t = omp_get_wtime ( ) ;
    GB_MKL_OK (mkl_graph_mxv (z_mkl, NULL, MKL_GRAPH_ACCUMULATOR_NONE,
        mkl_semiring, A_mkl, b_mkl, mkl_desc,
        MKL_GRAPH_REQUEST_COMPUTE_ALL, MKL_GRAPH_METHOD_AUTO)) ;
    t = omp_get_wtime ( ) - t ;
    GBURBLE ("(MKL mxv time: %g) ", t) ;

    //--------------------------------------------------------------------------
    // c += z
    //--------------------------------------------------------------------------

    GB_cblas_saxpy (n, 1.0, Zx, Cx, nthreads_max) ;

    //--------------------------------------------------------------------------
    // free MKL matrices z, A, and b
    //--------------------------------------------------------------------------

    GB_MKL_FREE_WORK ;
    ASSERT_VECTOR_OK (c, "mkl mxv result", GB0) ;
    return (info) ;
}

#endif

