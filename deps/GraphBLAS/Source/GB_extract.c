//------------------------------------------------------------------------------
// GB_extract: C<M> = accum(C,A(I,J))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Not user-callable.  Implements the user-callable GrB_*_extract functions.
//
// C<M> = accum (C, A (Rows,Cols)) or
// C<M> = accum (C, AT(Rows,Cols)) where AT = A'
//
// equivalently:
//
// C<M> = accum (C, A(Rows,Cols) )
// C<M> = accum (C, A(Cols,Rows)')

#include "GB.h"

GrB_Info GB_extract                 // C<M> = accum (C, A(I,J))
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C matrix descriptor
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // mask descriptor
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Matrix A,             // input matrix
    const bool A_transpose,         // A matrix descriptor
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows_in,       // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols_in,       // number of column indices
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_ALIAS_OK2 (C, M, A)) ;

    GB_RETURN_IF_NULL (Rows) ;
    GB_RETURN_IF_NULL (Cols) ;
    GB_RETURN_IF_FAULTY (accum) ;

    ASSERT_OK (GB_check (C, "C input for GB_Matrix_extract", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (M, "M for GB_Matrix_extract", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (accum, "accum for GB_Matrix_extract", GB0)) ;
    ASSERT_OK (GB_check (A, "A input for GB_Matrix_extract", GB0)) ;

    // check domains and dimensions for C<M> = accum (C,T)
    GrB_Info info = GB_compatible (C->type, C, M, accum, A->type, Context) ;
    if (info != GrB_SUCCESS)
    { 
        return (info) ;
    }

    // check the dimensions of C
    int64_t cnrows = GB_NROWS (C) ;
    int64_t cncols = GB_NCOLS (C) ;

    int64_t nRows, nCols, RowColon [3], ColColon [3] ;
    int RowsKind, ColsKind ;

    if (!A_transpose)
    { 
        // T = A(Rows,Cols)
        GB_ijlength (Rows, nRows_in, GB_NROWS (A), &nRows, &RowsKind, RowColon);
        GB_ijlength (Cols, nCols_in, GB_NCOLS (A), &nCols, &ColsKind, ColColon);
    }
    else
    { 
        // T = A(Cols,Rows)
        GB_ijlength (Rows, nRows_in, GB_NCOLS (A), &nRows, &RowsKind, RowColon);
        GB_ijlength (Cols, nCols_in, GB_NROWS (A), &nCols, &ColsKind, ColColon);
    }

    if (cnrows != nRows || cncols != nCols)
    { 
        return (GB_ERROR (GrB_DIMENSION_MISMATCH, (GB_LOG,
            "Dimensions not compatible:\n"
            "required size of output is "GBd"-by-"GBd"\n"
            "but actual size output is  "GBd"-by-"GBd"\n",
            nRows, nCols, cnrows, cncols))) ;
    }

    // quick return if an empty mask is complemented
    GB_RETURN_IF_QUICK_MASK (C, C_replace, M, Mask_comp) ;

    // delete any lingering zombies and assemble any pending tuples
    GB_WAIT (C) ;
    GB_WAIT (M) ;
    GB_WAIT (A) ;

    //--------------------------------------------------------------------------
    // handle the CSR/CSC format and transpose; T = A (I,J) or T = A (J,I)
    //--------------------------------------------------------------------------

    const GrB_Index *I, *J ;
    int64_t ni, nj ;
    bool T_is_csc ;

    if (A->is_csc)
    {
        if (!A_transpose)
        { 
            // T = A(Rows,Cols) where both A and T are in CSC format
            I = Rows ; ni = nRows_in ;  // indices into the vectors
            J = Cols ; nj = nCols_in ;  // vectors
            T_is_csc = true ;           // return T in CSC format
        }
        else
        { 
            // T = A(Cols,Rows) where A is CSC and T is returned as CSR format
            I = Cols ; ni = nCols_in ;  // indices into the vectors
            J = Rows ; nj = nRows_in ;  // vectors
            T_is_csc = false ;          // return T in CSR format
        }
    }
    else
    {
        if (!A_transpose)
        { 
            // T = A(Rows,Cols) where both A and T are in CSR format
            I = Cols ; ni = nCols_in ;  // indices into the vectors
            J = Rows ; nj = nRows_in ;  // vectors
            T_is_csc = false ;          // return T in CSR format
        }
        else
        { 
            // T = A(Cols,Rows) where A is CSR but T is returned as CSC format
            I = Rows ; ni = nRows_in ;  // indices into the vectors
            J = Cols ; nj = nCols_in ;  // vectors
            T_is_csc = true ;           // return T in CSC format
        }
    }

    // T has T->vdim = |J|, each vector of length T->vlen = |J|, regardless of
    // its CSR/CSC format.

    // J is a list of length |J| of vectors in the range 0:A->vdim-1
    // I is a list of length |I| of indices in the range 0:A->vlen-1

    // |I| and |J| are either nRows or nCols, depending on the 4 cases above.

    // T has the same hypersparsity as A.

    // If T and C have different CRS/CSC formats, then GB_accum_mask must
    // transpose T, and thus T can be returned from GB_subref_numeric with
    // jumbled indices.  If T and C have the same CSR/CSC formats, then
    // GB_subref_numeric must return T with sorted indices in each vector
    // because GB_accum_mask will not transpose T.

    // If T is a single column or a single row, it must be sorted, because the
    // row/column transpose methods in GB_transpose do not do the sort.

    bool must_sort = (T_is_csc == C->is_csc) || (cnrows == 1) || (cncols == 1) ;

    //--------------------------------------------------------------------------
    // T = A (I,J)
    //--------------------------------------------------------------------------

    GrB_Matrix T ;
    info = GB_subref_numeric (&T, T_is_csc, A, I, ni, J, nj, must_sort,
        Context) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    if (must_sort)
    { 
        ASSERT_OK (GB_check (T, "T extracted", GB0)) ;
    }
    else
    { 
        ASSERT_OK_OR_JUMBLED (GB_check (T, "T extracted (jumbled OK)", GB0)) ;
    }

    //--------------------------------------------------------------------------
    // C<M> = accum (C,T): accumulate the results into C via the mask M
    //--------------------------------------------------------------------------

    return (GB_accum_mask (C, M, NULL, accum, &T, C_replace, Mask_comp,
        Context)) ;
}

