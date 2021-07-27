//------------------------------------------------------------------------------
// GB_AxB_dot: C<M>=A'*B using dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Parallel matrix-matrix multiply, A'*B, with optional mask M.  This
// method is used by GrB_mxm, GrB_vxm, and GrB_mxv.  For both of the latter two
// methods, B on input will be an nrows-by-1 column vxector.

// This function, and the matrices C, M, A, and B are all CSR/CSC agnostic.
// For this discussion, suppose they are CSC, with vlen = # of rows, and vdim =
// # of columns.

// C=A'*B, C<M>=A'*B or C<!M>=A'*B is being computed.  A has not been
// transposed yet (and will not be).  A and B must have the same vector length,
// vlen (as if both A and B are CSC matrices with the same number of rows, for
// example).  GB_AxB_dot2 and GB_AxB_dot3 operate on A' without forming it.
// GB_AxB_dot2 computes C=A'*B and C<!M>=A'*B, and it takes Omega(m*n) time,
// if C is m-by-n.  It is thus only suitable for cases when A and B are large,
// and C is small.  GB_AxB_dot3 computes C<M>=A'*B, and it only needs to
// examine entries in M, taking Omega(nnz(M)) time.  It can thus be used for
// very large matrices C.  GB_AxB_dot4 computes C+=A'*B when C is dense.

// The output matrix C has not been allocated.  It is an uninitialzed static
// header on input.  The mask M is optional.

// If the result is computed in-place, then the C parameter is ignored, and the
// result is computed in C_in instead.  This case requires the accum operator
// to match the monoid of the semiring.

// The semiring defines C=A*B.  flipxy modifies how the semiring multiply
// operator is applied.  If false, then fmult(aik,bkj) is computed.  If true,
// then the operands are swapped, and fmult(bkj,aij) is done instead.

// Context: the GB_Context containing the # of threads to use, a string of the
// user-callable function that is calling this function (GrB_mxm, GrB_mxv, or
// GxB_vxm) and detailed error reports.

#include "GB_mxm.h"
#define GB_FREE_ALL ;

GrB_Info GB_AxB_dot                 // dot product (multiple methods)
(
    GrB_Matrix C,                   // output matrix, static header
    GrB_Matrix C_in,                // input/output matrix, if done in-place
    GrB_Matrix M,                   // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    bool *done_in_place,            // if true, C_in was computed in-place
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (C != NULL && C->static_header) ;

    ASSERT_MATRIX_OK_OR_NULL (M, "M for dot A'*B", GB0) ;
    ASSERT (!GB_PENDING (M)) ;
    ASSERT (GB_JUMBLED_OK (M)) ;
    ASSERT (!GB_ZOMBIES (M)) ;

    ASSERT_MATRIX_OK (A, "A for dot A'*B", GB0) ;
    GB_MATRIX_WAIT (A) ;
    ASSERT (!GB_PENDING (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;

    ASSERT_MATRIX_OK (B, "B for dot A'*B", GB0) ;
    GB_MATRIX_WAIT (B) ;
    ASSERT (!GB_PENDING (B)) ;
    ASSERT (!GB_JUMBLED (B)) ;
    ASSERT (!GB_ZOMBIES (B)) ;

    ASSERT_SEMIRING_OK (semiring, "semiring for dot A'*B", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (C_in, "C_in for dot A'*B", GB0) ;

    //--------------------------------------------------------------------------
    // determine if C is iso
    //--------------------------------------------------------------------------

    GrB_Type ztype = semiring->add->op->ztype ;
    size_t zsize = ztype->size ;
    GB_void cscalar [GB_VLA(zsize)] ;
    bool C_iso = GB_iso_AxB (cscalar, A, B, A->vlen, semiring, flipxy, false) ;
    if (C_iso)
    {
        // revise the method if A and B are both iso and full
        if (A->iso && GB_as_if_full (A) && B->iso && GB_as_if_full (B))
        {

            //------------------------------------------------------------------
            // C is iso and full; do not apply the mask
            //------------------------------------------------------------------

            GBURBLE ("(iso full dot) ") ;
            (*done_in_place) = false ;
            (*mask_applied) = false ;
            // set C->iso = true    OK
            GrB_Info info = GB_new_bix (&C, true,    // static header
                ztype, A->vdim, B->vdim, GB_Ap_null, true, GxB_FULL, false,
                GB_HYPER_SWITCH_DEFAULT, -1, 1, true, true, Context) ;
            if (info == GrB_SUCCESS)
            { 
                C->magic = GB_MAGIC ;
                memcpy (C->x, cscalar, zsize) ;
            }
            return (info) ;
        }
    }

    const char *iso_kind = (C_iso) ? "iso " : "" ;

    //--------------------------------------------------------------------------
    // in-place C+=A'*B.  mask is not present (and not applied)
    //--------------------------------------------------------------------------

    if (GB_AxB_dot4_control (C_iso, C_in, M, Mask_comp))
    { 
        // C_in must be as-if-full on input.  M must be NULL and not
        // complemented.  the C iso case is not handled (where C is iso on
        // output), but C_in might be iso on input.
        (*done_in_place) = true ;
        (*mask_applied) = false ;    // no mask to apply
        GBURBLE ("(dot4) ") ;
        return (GB_AxB_dot4 (C_in, A, B, semiring, flipxy, Context)) ;
    }

    //--------------------------------------------------------------------------
    // check the empty case
    //--------------------------------------------------------------------------

    if (A->vlen == 0)
    { 
        // no work to do; C is an empty matrix, normally hypersparse
        GBURBLE ("(empty dot) ") ;
        if (C_in != NULL) return (GrB_SUCCESS) ;
        return (GB_new (&C, true, // auto sparsity, static header
            ztype, A->vdim, B->vdim, GB_Ap_calloc, true, GxB_AUTO_SPARSITY,
            GB_Global_hyper_switch_get ( ), 1, Context)) ;
    }

    //--------------------------------------------------------------------------
    // C<M>=A'*B: general case
    //--------------------------------------------------------------------------

    if (GB_AxB_dot3_control (M, Mask_comp))
    { 

        // use dot3 if M is present and not complemented, and either sparse or
        // hypersparse
        GBURBLE ("(%sdot3) ", iso_kind) ;
        (*mask_applied) = true ;    // mask is always applied
        (*done_in_place) = false ;

        #if defined ( GBCUDA )

// [ replace this with:
// if (GB_AxB_dot3_cuda_branch (M, Mask_struct, A, B, semiring, flipxy, Context)

        // very rough estimate of the work to do
        int64_t anz = GB_nnz (A) ;
        int64_t bnz = GB_nnz (B) ;
        int64_t mnz = GB_nnz (M) ;

        double adeg = ((double) anz) / ((double) GB_IMAX (1, A->nvec)) ;
        double bdeg = ((double) bnz) / ((double) GB_IMAX (1, B->nvec)) ;
        double work = mnz * GB_IMIN (adeg, bdeg) ;

        // TODO for GPU: if A or B are not accessed (first, 2nd, or pair
        // ops) then the type of A can be user-defined here, for CUDA.

        int ngpus_to_use = GB_ngpus_to_use (work) ;
        GBURBLE (" work:%g gpus:%d ", work, ngpus_to_use) ;
        if (ngpus_to_use > 0
            && (semiring->header_size == 0)     // semiring is built-in
            && (A->type->code != GB_UDT_code)
            && (B->type->code != GB_UDT_code)
            && !GB_IS_BITMAP (A) && !GB_IS_BITMAP (B)
            && !C_iso)
// to here ... ]
        {
            // use "the" GPU (TODO for GPU: could use multiple GPUs too)
            return (GB_AxB_dot3_cuda (C, M, Mask_struct, A, B, semiring,
                flipxy, Context)) ;
        }
        else
        #endif
        { 
            // use the CPU
            return (GB_AxB_dot3 (C, C_iso, cscalar, M, Mask_struct, A, B,
                semiring, flipxy, Context)) ;
        }
    }

    //--------------------------------------------------------------------------
    // general case: C<M>=A'*B, C<!M>=A'B*, or C=A'*B, not in-place
    //--------------------------------------------------------------------------

    GBURBLE ("(%sdot2) ", iso_kind) ;
    (*mask_applied) = (M != NULL) ; // mask applied if present
    (*done_in_place) = false ;      // TODO: allow dot2 to work in-place
    return (GB_AxB_dot2 (C, C_iso, cscalar, M, Mask_comp, Mask_struct, A, B,
        semiring, flipxy, Context)) ;
}

