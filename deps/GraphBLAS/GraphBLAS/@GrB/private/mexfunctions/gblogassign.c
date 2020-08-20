//------------------------------------------------------------------------------
// gblogassign: logical assignment: C(M) = A
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gblogassign computes the MATLAB logical indexing expression C(M) = A.  The
// matrices C and M must be the same size.  M is normally logical but it can be
// of any type in this mexFunction.  M should not have any explicit zeros.  A
// is a sparse vector of size nnz(M)-by-1.  Scalar expansion is not handled.
// Use GrB.subassign (C, M, scalar) for that case.

// Usage:

//      C = gblogassign (C, M, A)

//  This function is the C equivalent of the following m-function:

/*

    function C = gblogassign (C, M_input, A)
    % Computing the MATLAB logical indexing expression C(M) = A in GraphBLAS.
    % A is a sparse vector of size nnz(M)-by-1 (scalar expansion is not
    % handled). M is normally a sparse logical matrix, either GraphBLAS or
    % MATLAB, but it can be of any type.  C and M have the same size.

    % make sure all matrices are stored by column
    save = GrB.format ;
    GrB.format ('by col') ;
    M = GrB (m, n, 'logical') ;
    M = GrB.select (M, '2nd', 'nonzero', M_input) ;
    if (isequal (GrB.format (A), 'by row'))
        A = GrB (A) ;
    end

    [m n] = size (C) ;
    mnz = nnz (M) ;         % A must be mnz-by-1
    if (~isequal (size (A), [mnz 1]))
        error ('A must be nnz(M)-by-1')
    end

    [ai,  ~, ax] = GrB.extracttuples (A) ;
    [mi, mj,  ~] = GrB.extracttuples (M) ;

    % construct a subset of the entries of the mask M corresponding to the
    % entries in A
    si = mi (ai) ;
    sj = mj (ai) ;
    S = GrB.build (si, sj, ax, m, n) ;

    GrB.format (save) ;

    % C<M> = S
    C = GrB.subassign (C, M, S) ;

*/


// This C mexFunction is faster than the above m-function, since it avoids the
// use of GrB.extracttuples.  Instead, it accesses the internal structure of the
// GrB_Matrix objects.  The m-file above is useful for understanding that this
// C mexFunction does.

#include "gb_matlab.h"

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

    gb_usage (nargin == 3 && nargout <= 1, "usage: C = gblogassign (C, M, A)") ;

    //--------------------------------------------------------------------------
    // get a deep copy of C
    //--------------------------------------------------------------------------

    GrB_Matrix C = gb_get_deep (pargin [0]) ;

    GrB_Index nrows, ncols ;
    OK (GrB_Matrix_nrows (&nrows, C)) ;
    OK (GrB_Matrix_ncols (&ncols, C)) ;

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
    // get A
    //--------------------------------------------------------------------------

    // make sure A is stored by column, and is nnz(M)-by-1
    GrB_Matrix A_input = gb_get_shallow (pargin [2]) ;
    GrB_Matrix A, A_copy ;
    A = gb_by_col (&A_copy, A_input) ;

    GrB_Index anrows, ancols ;
    OK (GrB_Matrix_nrows (&anrows, A)) ;
    OK (GrB_Matrix_ncols (&ancols, A)) ;
    if (mnz > 0)
    { 
        CHECK_ERROR (anrows != mnz || ancols != 1, "A must be nnz(M)-by-1") ;
    }

    GrB_Index anz ;
    OK (GrB_Matrix_nvals (&anz, A)) ;

    GrB_Type atype ;
    OK (GxB_Matrix_type (&atype, A)) ;

    int64_t *Ai = A->i ;
    void *Ax = A->x ;
    double empty ;
    if (Ax == NULL) Ax = &empty ;

    //--------------------------------------------------------------------------
    // extract the pattern of M
    //--------------------------------------------------------------------------

    GrB_Index *Mi = (GrB_Index *) (M->i) ;
    GrB_Index *Mj = mxMalloc (MAX (mnz, 1) * sizeof (GrB_Index)) ;
    OK (GrB_Matrix_extractTuples_BOOL (NULL, Mj, NULL, &mnz, M)) ;

    //--------------------------------------------------------------------------
    // construct a subset of the pattern of M corresponding to the entries of A
    //--------------------------------------------------------------------------

    GrB_Index *Si = mxMalloc (MAX (anz, 1) * sizeof (GrB_Index)) ;
    GrB_Index *Sj = mxMalloc (MAX (anz, 1) * sizeof (GrB_Index)) ;

    GB_matlab_helper5 (Si, Sj, Mi, Mj, Ai, anz) ;

    GrB_Matrix S ;
    OK (GrB_Matrix_new (&S, atype, nrows, ncols)) ;
    OK (GxB_Matrix_Option_set (S, GxB_FORMAT, GxB_BY_COL)) ;

    if (atype == GrB_BOOL)
    { 
        OK (GrB_Matrix_build_BOOL (S, Si, Sj, Ax, anz, GrB_LOR)) ;
    }
    else if (atype == GrB_INT8)
    { 
        OK (GrB_Matrix_build_INT8 (S, Si, Sj, Ax, anz, GrB_PLUS_INT8)) ;
    }
    else if (atype == GrB_INT16)
    { 
        OK (GrB_Matrix_build_INT16 (S, Si, Sj, Ax, anz, GrB_PLUS_INT16)) ;
    }
    else if (atype == GrB_INT32)
    { 
        OK (GrB_Matrix_build_INT32 (S, Si, Sj, Ax, anz, GrB_PLUS_INT32)) ;
    }
    else if (atype == GrB_INT64)
    { 
        OK (GrB_Matrix_build_INT64 (S, Si, Sj, Ax, anz, GrB_PLUS_INT64)) ;
    }
    else if (atype == GrB_UINT8)
    { 
        OK (GrB_Matrix_build_UINT8 (S, Si, Sj, Ax, anz, GrB_PLUS_UINT8)) ;
    }
    else if (atype == GrB_UINT16)
    { 
        OK (GrB_Matrix_build_UINT16 (S, Si, Sj, Ax, anz, GrB_PLUS_UINT16)) ;
    }
    else if (atype == GrB_UINT32)
    { 
        OK (GrB_Matrix_build_UINT32 (S, Si, Sj, Ax, anz, GrB_PLUS_UINT32)) ;
    }
    else if (atype == GrB_UINT64)
    { 
        OK (GrB_Matrix_build_UINT64 (S, Si, Sj, Ax, anz, GrB_PLUS_UINT64)) ;
    }
    else if (atype == GrB_FP32)
    { 
        OK (GrB_Matrix_build_FP32 (S, Si, Sj, Ax, anz, GrB_PLUS_FP32)) ;
    }
    else if (atype == GrB_FP64)
    { 
        OK (GrB_Matrix_build_FP64 (S, Si, Sj, Ax, anz, GrB_PLUS_FP64)) ;
    }
    #ifdef GB_COMPLEX_TYPE
    else if (atype == gb_complex_type)
    {
        OK (GrB_Matrix_build_UDT (S, Si, Sj, Ax, anz, ...)) ;
    }
    #endif
    else
    {
        ERROR ("unsupported type") ;
    }

    //--------------------------------------------------------------------------
    // C<M> = S
    //--------------------------------------------------------------------------

    OK (GxB_Matrix_subassign (C, M, NULL,
        S, GrB_ALL, nrows, GrB_ALL, ncols, NULL)) ;

    //--------------------------------------------------------------------------
    // free shallow copies and temporary matrices
    //--------------------------------------------------------------------------

    gb_mxfree (&Si) ;
    gb_mxfree (&Sj) ;
    gb_mxfree (&Mj) ;
    OK (GrB_Matrix_free (&M)) ;
    OK (GrB_Matrix_free (&M_input)) ;

    //--------------------------------------------------------------------------
    // export the output matrix C back to MATLAB
    //--------------------------------------------------------------------------

    pargout [0] = gb_export (&C, KIND_GRB) ;
    GB_WRAPUP ;
}

