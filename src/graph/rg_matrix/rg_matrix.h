/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <pthread.h>
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

// forward declaration of RG_Matrix type
typedef struct _RG_Matrix _RG_Matrix;
typedef _RG_Matrix *RG_Matrix;

#define RG_MATRIX_MATRIX(C) (C)->matrix
#define RG_MATRIX_DELTA_PLUS(C) (C)->delta_plus
#define RG_MATRIX_DELTA_MINUS(C) (C)->delta_minus

struct _RG_Matrix {
	bool dirty;                         // Indicates if matrix requires sync
	bool multi_edge;                    // Entry i,j can contain multiple edges
	bool maintain_transpose;            // Maintain transpose matrix
	GrB_Matrix matrix;                  // Underlying GrB_Matrix
	GrB_Matrix delta_plus;              // Pending additions
	GrB_Matrix delta_minus;             // Pending deletions
	RG_Matrix transposed;               // Transposed matrix
	pthread_mutex_t mutex;              // Lock
};


GrB_Info RG_Matrix_new
(
    RG_Matrix *A,            // handle of matrix to create
    GrB_Type type,           // type of matrix to create
    GrB_Index nrows,         // matrix dimension is nrows-by-ncols
    GrB_Index ncols,
    bool multi_edge,         // alow multi edge
	bool maintain_transpose  // maintain transpose matrix
);

// returns transposed matrix of C
RG_Matrix RG_Matrix_getTranspose
(
	const RG_Matrix C
);

// returns underlying GraphBLAS matrix
GrB_Matrix RG_Matrix_getGrB_Matrix
(
	RG_Matrix C
);

// returns underlying delta plus GraphBLAS matrix
GrB_Matrix RG_Matrix_getDeltaPlus
(
	RG_Matrix C
);

// mark matrix as dirty
void RG_Matrix_setDirty
(
	RG_Matrix C
);

bool RG_Matrix_isDirty
(
	const RG_Matrix C
);

// locks the matrix
void RG_Matrix_Lock
(
	RG_Matrix C
);

// unlocks the matrix
void RG_Matrix_Unlock
(
	RG_Matrix C
);

void RG_Matrix_setMultiEdge
(
	RG_Matrix C,
	bool multi_edge
);

bool RG_Matrix_getMultiEdge
(
	const RG_Matrix C
);

GrB_Info RG_Matrix_nvals    // get the number of entries in a matrix
(
    GrB_Index *nvals,       // matrix has nvals entries
    const RG_Matrix A       // matrix to query
);

GrB_Info RG_Matrix_resize      // change the size of a matrix
(
    RG_Matrix C,                // matrix to modify
    GrB_Index nrows_new,        // new number of rows in matrix
    GrB_Index ncols_new         // new number of columns in matrix
);

GrB_Info RG_Matrix_setElement_BOOL      // C (i,j) = x
(
    RG_Matrix C,                        // matrix to modify
    bool x,                             // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
);

GrB_Info RG_Matrix_setElement_UINT64      // C (i,j) = x
(
    RG_Matrix C,                        // matrix to modify
    uint64_t x,                         // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
);

GrB_Info RG_Matrix_extractElement_BOOL     // x = A(i,j)
(
    bool *x,                               // extracted scalar
    const RG_Matrix A,                     // matrix to extract a scalar from
    GrB_Index i,                           // row index
    GrB_Index j                            // column index
) ;

GrB_Info RG_Matrix_extractElement_UINT64   // x = A(i,j)
(
    uint64_t *x,                           // extracted scalar
    const RG_Matrix A,                     // matrix to extract a scalar from
    GrB_Index i,                           // row index
    GrB_Index j                            // column index
) ;

GrB_Info RG_Matrix_removeElement
(
    RG_Matrix C,                    // matrix to remove entry from
    GrB_Index i,                    // row index
    GrB_Index j                     // column index
);

GrB_Info RG_Matrix_subassign_UINT64 // C(I,J)<Mask> = accum (C(I,J),x)
(
    RG_Matrix C,                    // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C(I,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),x)
    uint64_t x,                     // scalar to assign to C(I,J)
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Index *J,             // column indices
    GrB_Index nj,                   // number of column indices
    const GrB_Descriptor desc       // descriptor for C(I,J) and Mask
);

GrB_Info RG_mxm                     // C = A * B
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Semiring semiring,    // defines '+' and '*' for A*B
    const GrB_Matrix A,             // first input:  matrix A
    const RG_Matrix B               // second input: matrix B
);

GrB_Info RG_Matrix_wait
(
	RG_Matrix C,
	bool force_sync
);

void RG_Matrix_free
(
	RG_Matrix *C
);

