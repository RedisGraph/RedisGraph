/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../../../deps/GraphBLAS/Include/GraphBLAS.h"

// suitable for integers, and non-NaN floating point:
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define MIN(x,y) (((x) < (y)) ? (x) : (y))

//------------------------------------------------------------------------------
// LAGraph methods
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// LAGRAPH_OK: call LAGraph or GraphBLAS and check the result
//------------------------------------------------------------------------------

// To use LAGRAPH_OK, the #include'ing file must declare a scalar GrB_Info
// info, and must define LAGRAPH_FREE_ALL as a macro that frees all workspace
// if an error occurs.  The method can be a GrB_Info scalar as well, so that
// LAGRAPH_OK(info) works.  The function that uses this macro must return
// GrB_Info, or int.

#define LAGRAPH_ERROR(message, info)                                        \
{                                                                           \
    fprintf (stderr, "Error: %s\n[%d]\n%s\nFile: %s Line: %d\n",            \
        message, info, GrB_error ( ), __FILE__, __LINE__) ;                 \
    LAGRAPH_FREE_ALL ;                                                      \
    return (info) ;                                                         \
}

#define LAGRAPH_OK(method)                                                  \
{                                                                           \
    info = method ;                                                         \
    if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))                    \
    {                                                                       \
        LAGRAPH_ERROR ("", info) ;                                          \
    }                                                                       \
}

typedef struct {
    GrB_Matrix s;           // Block matrix structure, s[i,j] = true if block i,j exists.
    GrB_Matrix *blocks;     // Array of blocks, blocks[i,j] = GrB_NULL for missing blocks.
    GrB_Index block_nrows;  // Number of rows in each block.
    GrB_Index block_ncols;  // Number of columns in each block.
    GrB_Type t;             // Type of blocks.
} GB_BlockMatrix_opaque;

typedef GB_BlockMatrix_opaque* BlockMatrix;

//==============================================================================
//=== Matrix methods ===========================================================
//==============================================================================

// These methods create, free, copy, and clear a matrix.  The nrows, ncols,
// nvals, and type methods return basic information about a matrix.

GrB_Info BlockMatrix_new    // create a new matrix with no entries
(
    BlockMatrix *B,         // handle of matrix to create
    GrB_Type type,          // type of matrix to create
    GrB_Index nrows,        // matrix dimension is nrows-by-ncols
    GrB_Index ncols,
  	int nblocks
) ;

GrB_Info BlockMatrix_dup    // make an exact copy of a matrix
(
    BlockMatrix *C,         // handle of output matrix to create
    const BlockMatrix A     // input matrix to copy
) ;

GrB_Info BlockMatrix_clear  // clear a matrix of all entries;
(                           // type and dimensions remain unchanged
    BlockMatrix A           // matrix to clear
) ;

GrB_Info BlockMatrix_BlocksPerRow   // Return number of blocks per row.
(
  GrB_Index *nblocks_per_row,       // number of blocks in each row
  const BlockMatrix B               // matrix to query
);

GrB_Info BlockMatrix_BlocksPerColumn    // Return number of blocks per column.
(
  GrB_Index *nblocks_per_col,           // number of blocks in each column
  const BlockMatrix B                   // matrix to query
);

GrB_Info BlockMatrix_nblocks    // Returns number of active blocks in matrix.
(
  GrB_Index *nblocks,           // number of active blocks in matrix
  const BlockMatrix B           // matrix to query
);

GrB_Info BlockMatrix_GetBlock   // Extract a single block from a matrix
(
  GrB_Matrix *block,            // extracted block
  const BlockMatrix B,          // matrix to extract block from
  GrB_Index row,                // block row index
  GrB_Index col                 // block column index
);

GrB_Info BlockMatrix_SetBlock   // Set a single block in a matrix
(
  BlockMatrix B,                // matrix to update
  GrB_Matrix block,             // block to set
  GrB_Index row,                // block row index
  GrB_Index col                 // block column index
);

GrB_Info BlockMatrix_nrows  // get the number of rows of a matrix
(
    GrB_Index *nrows,       // matrix has nrows rows
    const BlockMatrix B     // matrix to query
) ;

GrB_Info BlockMatrix_ncols  // get the number of columns of a matrix
(
    GrB_Index *ncols,       // matrix has ncols columns
    const BlockMatrix B     // matrix to query
) ;

GrB_Info BlockMatrix_nvals  // get the number of entries in a matrix
(
    GrB_Index *nvals,       // matrix has nvals entries
    const BlockMatrix B     // matrix to query
) ;

// SPEC: GxB_Matrix_type is an extension to the spec

GrB_Info BlockMatrix_type   // get the type of a matrix
(
    GrB_Type *type,         // returns the type of the matrix
    const BlockMatrix B     // matrix to query
) ;

GrB_Info BlockMatrix_free   // free a matrix
(
    BlockMatrix *B          // handle of matrix to free
) ;

//------------------------------------------------------------------------------
// BlockMatrix_setElement
//------------------------------------------------------------------------------

// Set a single entry in a matrix, C(i,j) = x in MATLAB notation, typecasting
// from the type of x to the type of C, as needed.

GrB_Info BlockMatrix_setElement_BOOL    // C (i,j) = x
(
    BlockMatrix C,                      // matrix to modify
    bool x,                             // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

GrB_Info BlockMatrix_setElement_UINT64  // C (i,j) = x
(
    BlockMatrix C,                      // matrix to modify
    uint64_t x,                         // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) ;

//------------------------------------------------------------------------------
// BlockMatrix_extractElement
//------------------------------------------------------------------------------

// Extract a single entry from a matrix, x = A(i,j), typecasting from the type
// of A to the type of x, as needed.

// Returns GrB_SUCCESS if A(i,j) is present, and sets x to its value.
// Returns GrB_NO_VALUE if A(i,j) is not present, and x is unmodified.

GrB_Info BlockMatrix_extractElement_BOOL    // x = A(i,j)
(
	bool *x,                                // extracted scalar
	const BlockMatrix A,                    // matrix to extract a scalar from
	GrB_Index i,                            // row index
	GrB_Index j                             // column index
) ;

//------------------------------------------------------------------------------
// matrix and vector apply
//------------------------------------------------------------------------------

// Apply a unary operator to the entries in a matrix,
// C<Mask> = accum (C, op (A)).

// The input matrix A may be optionally transposed first, via the descriptor.

GrB_Info BlockMatrix_apply          // C<Mask> = accum (C, op(A)) or op(A')
(
    BlockMatrix C,                  // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_UnaryOp op,           // operator to apply to the entries
    const BlockMatrix A,            // first input:  matrix A
    const GrB_Descriptor desc       // descriptor for C, mask, and A
) ;

//------------------------------------------------------------------------------
// matrix transpose
//------------------------------------------------------------------------------

// T = A' is computed by default, but A can also be transposed via the
// descriptor.  In this case A is not transposed at all, and T = A.  The result
// is then passed through the Mask and accum, like almost all other GraphBLAS
// operations.  This makes GrB_transpose a direct interface to the accum/mask
// operation, C<Mask> = accum (C,A), or C<Mask> = accum (C,A') by default.

GrB_Info BlockMatrix_transpose      // C<Mask> = accum (C, A')
(
    BlockMatrix C,                  // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const BlockMatrix A,            // first input:  matrix A
    const GrB_Descriptor desc       // descriptor for C, Mask, and A
) ;

//------------------------------------------------------------------------------
// matrix and vector multiplication over a semiring
//------------------------------------------------------------------------------

// Each of these methods compute a matrix multiplication over a semiring.  The
// inputs are typecasted into the inputs of the semiring's multiply operator.
// The result T=A*B has the type of the multiplier output, which is also the 3
// types of the 'add' operator.  The 'add' operator is a commutatitive and
// associative monoid.

GrB_Info BlockMatrix_mxm            // C<Mask> = accum (C, A*B)
(
    BlockMatrix C,                  // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '+' and '*' for A*B
    const BlockMatrix A,            // first input:  matrix A
    const BlockMatrix B,            // second input: matrix B
    const GrB_Descriptor desc       // descriptor for C, Mask, A, and B
) ;

//------------------------------------------------------------------------------
// GraphBLAS matrix and Block matrix conversion functions
//------------------------------------------------------------------------------

GrB_Info BlockMatrix_flatten    // populate GraphBLAS matrix with the contents of block matrix
(
    GrB_Matrix *C,              // input/output matrix for results
    const BlockMatrix A         // first input: block matrix A
) ;

//------------------------------------------------------------------------------
// BlockMatrix_print: print the contents of a Block matrix
//------------------------------------------------------------------------------
GrB_Info BlockMatrix_fprint     // print and check a BlockMatrix
(
    const BlockMatrix A,        // object to print and check
    GxB_Print_Level pr          // print level
) ;
