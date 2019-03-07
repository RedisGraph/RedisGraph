//------------------------------------------------------------------------------
// GB_AxB_parallel: C<M>=A*B, C<M>=A'*B, C=A*B, or C=A'*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Parallel matrix-matrix multiply, A*B or A'*B, with optional mask M.  This
// method is used by GrB_mxm, GrB_vxm, and GrB_mxv.  For both of the latter two
// methods, B on input will be an nrows-by-1 column vxector.
//
// If do_adotb is true, then A'*B is being computed.  In this case, A has
// not been transposed yet (and will not be).  A and B must have the same
// vector length, vlen (as if both A and B are CSC matrices with the same
// number of rows, for example).  The GB_AxB_sequential method when applied
// to A and B (or a slice of B) will operate on A' without forming it.
//
// If do_adotb is false, then A*B is being computed, and the vector dimension
// of A must be identical to the vector length of B (as if both A and B are
// CSC matrices, and the number of columns of A is the same as the number of
// rows of B).
//
// The output matrix C = *Chandle has not been allocated, so C is NULL on
// input.  The mask M is optional.  If present, it is not complemented.
//
// The semiring defines C=A*B, and must be valid.  flipxy modifies how the
// semiring multiply operator is applied.  If false, then fmult(aik,bkj)
// is computed.  If true, then the operands are swapped, and fmult(bkj,aij)
// is done instead.
//
// AxB_method selects the method to use:
//      GxB_DEFAULT:        the method is selected automatically
//      GxB_AxB_GUSTAVSON:  Gustavson's method for A*B
//      GxB_AxB_HEAP:       heap method for A*B
//      GxB_AxB_DOT:        dot method for A'*B
//      GxB_AxB_HASH:       hash method for A*B (FUTURE)
//
// AxB_method_used reports the method actually chosen.  This is for
// informational purposes only, so if a parallel C=A*B splits the work into
// multiple submatrix multiplications, and uses different methods on each
// submatrix, then AxB_method_used is the method chosen by thread zero.
//
// Context: the GB_Context containing the # of threads to use, a string of the
// user-callable function that is calling this function (GrB_mxm, GrB_mxv, or
// GxB_vxm) and detailed error reports.
//
// C, M, A, and B are CSR/CSC agnostic.  For this discussion, suppose they
// are CSC, with vlen = # of rows, and vdim = # of columns.

// PARALLEL: A' or B is sliced, and the slices of C are concatenated.

// FUTURE: The result of this function is the T matrix in GB_mxm.  It may be
// transplanted directly into the user's matrix, or it may be passed through
// GB_accum_mask.  See GB_mxm.  For the latter case, it would be better to
// delay the contatenation of the output matrix T = [C0 ... C(t-t)].
// GB_accum_mask could do the accum/mask using the sliced T matrix, to
// update the user's C matrix (which is not sliced), and then T is freed.

#include "GB.h"

GrB_Info GB_AxB_parallel            // parallel matrix-matrix multiply
(
    GrB_Matrix *Chandle,            // output matrix, NULL on input
    GrB_Matrix M,                   // optional mask matrix
    const bool Mask_comp,           // if true, use ~M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const bool do_adotb,            // if true, do A'*B via dot products
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
//  const GrB_Desc_Value AxB_slice, // how to slice B or A'
    GrB_Desc_Value *AxB_method_used,// method selected
    bool *mask_applied,             // if true, mask was applied
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;          // C = (*Chandle) is NULL
    ASSERT (*Chandle == NULL) ;
    ASSERT_OK_OR_NULL (GB_check (M, "M for parallel A*B", GB0)) ;
    ASSERT_OK (GB_check (A, "A for parallel A*B", GB0)) ;
    ASSERT_OK (GB_check (B, "B for parallel A*B", GB0)) ;
    ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT_OK (GB_check (semiring, "semiring for parallel A*B", GB0)) ;
    ASSERT (AxB_method_used != NULL) ;

    GrB_Info info ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS (nthreads, Context) ;
    nthreads = 1 ;      // FUTURE:: add parallel phase

    //==========================================================================
    // sequential C<M>=A*B, C<M>=A'*B, C=A*B, or C=A'*B
    //==========================================================================

    #define GB_FREE_ALL ;

    // if (nthreads == 1)
    {

        // do the entire computation with a single thread

        // select the method
        // printf ("select the method %d\n", AxB_method) ;
        int64_t bjnz_max ;
        GB_AxB_select (A, B, semiring, do_adotb, AxB_method,
            AxB_method_used, &bjnz_max) ;

        // printf ("selected method %d\n", *AxB_method_used) ;

        // acquire a Sauna if Gustavson's method is being used
        int Sauna_id = -2 ;
        if (*AxB_method_used == GxB_AxB_GUSTAVSON)
        {
            // printf ("get sauna\n") ;
            GB_OK (GB_Sauna_acquire (1, &Sauna_id, AxB_method_used, Context)) ;
            // printf ("got sauna %d\n", Sauna_id) ;
        }

        // C<M>=A*B or A'*B
        GrB_Info thread_info = GB_AxB_sequential (Chandle, M, Mask_comp,
            A, B, semiring, flipxy, *AxB_method_used, bjnz_max, mask_applied,
            Sauna_id) ;

        // printf ("info from GB_AxB_sequential is %d\n", thread_info) ;

        // release the Sauna for Gustavson's method
        if (*AxB_method_used == GxB_AxB_GUSTAVSON)
        {
            // printf ("release sauna\n") ;
            GB_OK (GB_Sauna_release (1, &Sauna_id)) ;
            // printf ("released sauna\n") ;
        }

        info = thread_info ;
        // printf ("thread_info now is %d\n", info) ;

        return ((info == GrB_OUT_OF_MEMORY) ? GB_OUT_OF_MEMORY : info) ;
    }

}

