//------------------------------------------------------------------------------
// adjacency_matrix.h: definition of an adjacency matrix
// where
//------------------------------------------------------------------------------

// An adjacency matrix is a sparse boolean matrix where row i column j is 1 if
// node i is connected to node j, otherwise the entry [i,j] is 0.

#ifndef ADJACENCY_MATRIX_H
#define ADJACENCY_MATRIX_H

#include "GraphBLAS.h"

//==============================================================================
//=== Matrix methods ===========================================================
//==============================================================================

GrB_Matrix Adj_Matrix_new
(
    const GrB_Index nrows,  // matrix dimension is nrows-by-ncols
    const GrB_Index ncols
);


GrB_Info Adj_Matrix_free    // free a matrix
(
    GrB_Matrix *A           // handle of matrix to free
) ;

#endif