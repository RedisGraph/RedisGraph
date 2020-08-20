//------------------------------------------------------------------------------
// gblogextract: logical extraction: C = A(M)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gblogextract computes the MATLAB logical indexing expression C = A(M).  The
// matrices A and M must be the same size.  M is normally logical but it can be
// of any type in this mexFunction.  M should not have any explicit zeros.  C
// has the same type as A, and is a sparse vector of size nnz(M)-by-1.

// Usage:

//      C = gblogextract (A, M)

//  This function is the C equivalent of the following m-function:

/*

    function C = gblogextract (A, M_input)
    % Computing the MATLAB logical indexing expression C = A(M) in GraphBLAS.
    % C is a sparse vector of size nnz(M)-by-1.  M is normally a sparse logical
    % matrix, either GraphBLAS or MATLAB, but it can be of any type.
    % A and M have the same size.

    [m n] = size (A) ;

    % make sure all input, internal, and output matrices are all stored by
    % column 
    save = GrB.format ;
    GrB.format ('by col') ;
    M = GrB (m, n, 'logical') ;
    M = GrB.select (M, '2nd', 'nonzero', M_input) ;
    if (isequal (GrB.format (A), 'by row'))
        A = GrB (A) ;
    end
    mnz = nnz (M) ;         % C will be mnz-by-1

    % G<M> = A
    % G has the same type and size as A, but G is always stored by column
    G = GrB (m, n, GrB.type (A)) ;
    G = GrB.subassign (G, M, A) ;

    % extract gx = the entries of G
    [~, ~, gx] = GrB.extracttuples (G) ;

    % convert G to logical
    G = spones (G, 'logical') ;

    % K = symbolic structure of M, where the kth entry in K(:) is equal to k.
    desc.base = 'zero-based' ;
    [mi, mj] = GrB.extracttuples (M, desc) ;
    K = GrB.build (mi, mj, int64 (0:mnz-1), m, n, desc) ;

    % T<G> = K
    T = GrB (m, n, 'uint64') ;
    T = GrB.subassign (T, G, K) ;

    % extract the values from T
    [~, ~, tx] = GrB.extracttuples (T) ;

    % construct the result C (always a column vector)
    C = GrB.build (tx, zeros(length(gx),1,'uint64'), gx, mnz, 1) ;

    % restore the format to its original state
    GrB.format (save) ;

*/

// This C mexFunction is faster than the above m-function, since it avoids the
// use of GrB.extracttuples and GrB.build.  Instead, it accesses the internal
// structure of the GrB_Matrix objects, and creates shallow copies.  The
// m-file above is useful for understanding that this C mexFunction does.

#include "gb_matlab.h"
#include "GB_transpose.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    gb_usage (nargin == 2 && nargout <= 1, "usage: C = gblogextract (A, M)") ;
    GB_CONTEXT ("gblogextract") ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    // make sure A is stored by column
    GrB_Matrix A_input = gb_get_shallow (pargin [0]) ;
    GrB_Matrix A, A_copy ;
    A = gb_by_col (&A_copy, A_input) ;

    GrB_Index nrows, ncols ;
    OK (GrB_Matrix_nrows (&nrows, A)) ;
    OK (GrB_Matrix_ncols (&ncols, A)) ;

    //--------------------------------------------------------------------------
    // get M
    //--------------------------------------------------------------------------

    // make M boolean, stored by column, and drop explicit zeros
    GrB_Matrix M_input = gb_get_shallow (pargin [1]) ;
    GrB_Matrix M ;
    OK (GrB_Matrix_new (&M, GrB_BOOL, nrows, ncols)) ;
    OK (GxB_Matrix_Option_set (M, GxB_FORMAT, GxB_BY_COL)) ;
    OK (GxB_Matrix_select (M, NULL, NULL, GxB_NONZERO, M_input, NULL, NULL)) ;
    OK (GrB_Matrix_free (&M_input)) ;

    GrB_Index mnz ;
    OK (GrB_Matrix_nvals (&mnz, M)) ;

    //--------------------------------------------------------------------------
    // G<M> = A
    //--------------------------------------------------------------------------

    // G has the same type and size as A, but it is always stored by column
    GrB_Type type ;
    OK (GxB_Matrix_type (&type, A)) ;
    GrB_Matrix G ;
    OK (GrB_Matrix_new (&G, type, nrows, ncols)) ;
    OK (GxB_Matrix_Option_set (G, GxB_FORMAT, GxB_BY_COL)) ;

    OK (GxB_Matrix_subassign (G, M, NULL,
        A, GrB_ALL, nrows, GrB_ALL, ncols, NULL)) ;

    OK (GrB_Matrix_free (&A_copy)) ;
    OK (GrB_Matrix_free (&A_input)) ;

    //--------------------------------------------------------------------------
    // extract Gx, the values of G
    //--------------------------------------------------------------------------

    GrB_Index gnvals ;
    OK (GrB_Matrix_nvals (&gnvals, G)) ;
    void *Gx = G->x ;

    //--------------------------------------------------------------------------
    // change G to boolean
    //--------------------------------------------------------------------------

    // This does not affect the extracted values Gx
    G->type = GrB_BOOL ;
    G->type_size = sizeof (bool) ;
    if (G->nzmax > 0)
    { 
        G->x = mxMalloc (G->nzmax * sizeof (bool)) ;
        bool *Gbool = G->x ;
        GB_matlab_helper6 (Gbool, gnvals) ;
    }

    //--------------------------------------------------------------------------
    // K = structure of M, where the kth entry in K is equal to k
    //--------------------------------------------------------------------------

    // K is a shallow copy of M, except for its numerical values
    GrB_Matrix K ;
    OK (GB_shallow_copy (&K, GxB_BY_COL, M, Context)) ;

    // Kx = uint64 (0:mnz-1)
    uint64_t *Kx = mxMalloc (MAX (mnz, 1) * sizeof (uint64_t)) ;
    GB_matlab_helper7 (Kx, mnz) ;

    K->x = Kx ;
    K->x_shallow = false ;
    K->type = GrB_UINT64 ;
    K->type_size = sizeof (uint64_t) ;

    //--------------------------------------------------------------------------
    // T<G> = K
    //--------------------------------------------------------------------------

    GrB_Matrix T ;
    OK (GrB_Matrix_new (&T, GrB_UINT64, nrows, ncols)) ;
    OK (GxB_Matrix_Option_set (T, GxB_FORMAT, GxB_BY_COL)) ;
    OK (GxB_Matrix_subassign (T, G, NULL,
        K, GrB_ALL, nrows, GrB_ALL, ncols, NULL)) ;

    //--------------------------------------------------------------------------
    // extract Tx, the values of T
    //--------------------------------------------------------------------------

    GrB_Index tnvals ;
    OK (GrB_Matrix_nvals (&tnvals, T)) ;
    uint64_t *Tx = T->x ;

    // gnvals and tnvals are identical, by construction
    CHECK_ERROR (gnvals != tnvals, "internal error 1") ;

    //--------------------------------------------------------------------------
    // construct the result C
    //--------------------------------------------------------------------------

    // Vectors are always stored by column, and are never hypersparse.  This
    // step takes constant time, using a transplant of the row indices Tx from
    // T and the values Gx from G.

    GrB_Vector V ;
    OK (GrB_Vector_new (&V, type, mnz)) ;
    gb_mxfree (&V->i) ;
    gb_mxfree (&V->x) ;
    V->i = Tx ;         // transplant the values of T as the row indices of V
    T->x = NULL ;
    V->x = Gx ;         // transplant the values of G as the values of V
    V->nzmax = tnvals ;
    int64_t *Vp = V->p ;
    Vp [0] = 0 ;
    Vp [1] = tnvals ;
    V->magic = GB_MAGIC ;
    V->nvec_nonempty = (tnvals > 0) ? 1 : 0 ;

    // typecast V to a matrix C, for export back to MATLAB
    GrB_Matrix C = (GrB_Matrix) V ;
    V = NULL ;

    //--------------------------------------------------------------------------
    // free shallow copies and temporary matrices
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_free (&G)) ;
    OK (GrB_Matrix_free (&K)) ;
    OK (GrB_Matrix_free (&T)) ;

    //--------------------------------------------------------------------------
    // export the output matrix C back to MATLAB
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&C, KIND_GRB) ;
    GB_WRAPUP ;
}

