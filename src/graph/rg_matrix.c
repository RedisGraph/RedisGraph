#include "RG.h"
#include "rg_matrix.h"
#include "../util/rmalloc.h"

#define DELTA_MAX_PENDING_CHANGES 100000
#define RG_MATRIX_INTERNAL_MATRIX(rg_matrix) (rg_matrix)->grb_matrix
#define RG_MATRIX_INTERNAL_DELTA_PLUS(rg_matrix) (rg_matrix)->delta_plus
#define RG_MATRIX_INTERNAL_DELTA_MINUS(rg_matrix) (rg_matrix)->delta_minus

extern GrB_BinaryOp _graph_edge_accum;

// Creates a new matrix
GrB_Info RG_Matrix_New
(
	RG_Matrix *A,
    GrB_Type type,
    GrB_Index nrows,
    GrB_Index ncols
) {
	GrB_Info info;
	RG_Matrix matrix = rm_calloc(1, sizeof(_RG_Matrix));

	matrix->dirty = true;
	matrix->allow_multi_edge = true;

	info = GrB_Matrix_new(&matrix->grb_matrix, type, nrows, ncols);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_new(&matrix->delta_plus, type, nrows, ncols);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_new(&matrix->delta_minus, type, nrows, ncols);
	ASSERT(info == GrB_SUCCESS);

	info = GxB_set(matrix->grb_matrix, GxB_SPARSITY_CONTROL, GxB_SPARSE);
	ASSERT(info == GrB_SUCCESS);

	int mutex_res = pthread_mutex_init(&matrix->mutex, NULL);
	ASSERT(mutex_res == 0);

	*A = matrix;
	return info;
}

// Returns underlying GraphBLAS matrix.
GrB_Matrix RG_Matrix_Get_GrB_Matrix
(
	const RG_Matrix matrix
) {
	ASSERT(matrix != NULL);
	return RG_MATRIX_INTERNAL_MATRIX(matrix);
}

// returns underlying delta plus GraphBLAS matrix
GrB_Matrix RG_Matrix_Get_DeltaPlus
(
	RG_Matrix matrix
) {
	ASSERT(matrix != NULL);
	return RG_MATRIX_INTERNAL_DELTA_PLUS(matrix);
}

bool RG_Matrix_IsDirty(const RG_Matrix matrix) {
	ASSERT(matrix);
	return matrix->dirty;
}

void RG_Matrix_SetDirty(RG_Matrix matrix) {
	ASSERT(matrix);
	matrix->dirty = true;
}

void RG_Matrix_SetUnDirty(RG_Matrix matrix) {
	ASSERT(matrix);
	matrix->dirty = false;
}

// Locks the matrix.
void RG_Matrix_Lock(RG_Matrix matrix) {
	ASSERT(matrix);
	pthread_mutex_lock(&matrix->mutex);
}

// Unlocks the matrix.
void RG_Matrix_Unlock(RG_Matrix matrix) {
	ASSERT(matrix);
	pthread_mutex_unlock(&matrix->mutex);
}

bool RG_Matrix_MultiEdgeEnabled(const RG_Matrix matrix) {
	ASSERT(matrix);
	return matrix->allow_multi_edge;
}

GrB_Info RG_Matrix_resize       // change the size of a matrix
(
    RG_Matrix C,                // matrix to modify
    GrB_Index nrows_new,        // new number of rows in matrix
    GrB_Index ncols_new         // new number of columns in matrix
) {
	ASSERT(C != NULL);
	GrB_Info info;

	GrB_Matrix  m            =  RG_MATRIX_INTERNAL_MATRIX(C);
	GrB_Matrix  delta_plus   =  RG_MATRIX_INTERNAL_DELTA_PLUS(C);
	GrB_Matrix  delta_minus  =  RG_MATRIX_INTERNAL_DELTA_MINUS(C);

	info = GrB_Matrix_resize(m, nrows_new, ncols_new);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_resize(delta_plus, nrows_new, ncols_new);
	ASSERT(info == GrB_SUCCESS);
	
	info = GrB_Matrix_resize(delta_minus, nrows_new, ncols_new);
	ASSERT(info == GrB_SUCCESS);
	
	return info;
}

GrB_Info RG_Matrix_setElement_BOOL      // C (i,j) = x
(
    RG_Matrix C,                        // matrix to modify
    bool x,                             // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) {
	ASSERT(C != NULL);

	GrB_Info info;
	GrB_Matrix delta_plus = RG_MATRIX_INTERNAL_DELTA_PLUS(C);
	info = GrB_Matrix_setElement_BOOL(delta_plus, x, i, j);
	if(info == GrB_SUCCESS) RG_Matrix_SetDirty(C);
	return info;
}

GrB_Info RG_Matrix_setElement_UINT64      // C (i,j) = x
(
    RG_Matrix C,                        // matrix to modify
    uint64_t x,                         // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) {
	ASSERT(C != NULL);

	GrB_Info info;
	GrB_Matrix delta_plus = RG_MATRIX_INTERNAL_DELTA_PLUS(C);
	info = GrB_Matrix_setElement_UINT64(delta_plus, x, i, j);
	if(info == GrB_SUCCESS) RG_Matrix_SetDirty(C);
	return info;
}

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
) {
	ASSERT(C != NULL);

	GrB_Info info;
	GrB_Matrix delta_plus = RG_MATRIX_INTERNAL_DELTA_PLUS(C);

	info = GxB_Matrix_subassign_UINT64   // C(I,J)<Mask> = accum (C(I,J),x)
		(
		 delta_plus,           // input/output matrix for results
		 Mask,                 // optional mask for C(I,J), unused if NULL
		 accum,                // optional accum for Z=accum(C(I,J),x)
		 x,                    // scalar to assign to C(I,J)
		 I,                    // row indices
		 ni,                   // number of row indices
		 J,                    // column indices
		 nj,                   // number of column indices
		 desc                  // descriptor for C(I,J) and Mask
		);

	if(info == GrB_SUCCESS) RG_Matrix_SetDirty(C);
	return info;
}

GrB_Info RG_Matrix_sync
(
	RG_Matrix C
) {
	ASSERT(C != NULL);

	GrB_Matrix      m            =  RG_MATRIX_INTERNAL_MATRIX(C);
	GrB_Descriptor  desc         =  GrB_NULL;
	GrB_Matrix      mask         =  GrB_NULL;
	GrB_Matrix      delta_plus   =  RG_MATRIX_INTERNAL_DELTA_PLUS(C);
	GrB_Matrix      delta_minus  =  RG_MATRIX_INTERNAL_DELTA_MINUS(C);

	GrB_Index nvals;
	GrB_Matrix_nvals(&nvals, delta_plus);

	// no changes
	if(nvals == 0) return GrB_SUCCESS;

	GrB_Matrix_nvals(&nvals, delta_minus);
	if(nvals > 0) {
		mask = delta_minus;
		desc = GrB_DESC_SC;
	}

	// add delta-plus to 'm' using delta-minus as a mask
	GrB_Info info = GrB_Matrix_eWiseAdd_Semiring (m, mask, _graph_edge_accum,
			GxB_ANY_PAIR_BOOL, m, delta_plus, desc);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_clear(delta_plus);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_clear(delta_minus);
	ASSERT(info == GrB_SUCCESS);

	return info;
}

GrB_Info RG_Matrix_wait
(
	RG_Matrix A
) {
	ASSERT(A != NULL);
	
	GrB_Info    info         =  GrB_SUCCESS;
	GrB_Matrix  m            =  RG_MATRIX_INTERNAL_MATRIX(A);
	GrB_Matrix  delta_plus   =  RG_MATRIX_INTERNAL_DELTA_PLUS(A);
	GrB_Matrix  delta_minus  =  RG_MATRIX_INTERNAL_DELTA_MINUS(A);

	info = GrB_wait(&m);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_wait(&delta_plus);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_wait(&delta_minus);
	ASSERT(info == GrB_SUCCESS);

	// check if merge is required
	
	GrB_Index delta_plus_nvals;
	GrB_Index delta_minus_nvals;
	GrB_Matrix_nvals(&delta_plus_nvals, delta_plus);
	GrB_Matrix_nvals(&delta_minus_nvals, delta_minus);
	if(delta_plus_nvals + delta_minus_nvals >= DELTA_MAX_PENDING_CHANGES) {
		info = RG_Matrix_sync(A);
	}

	RG_Matrix_SetUnDirty(A);
	return info;
}

void RG_Matrix_Free
(
	RG_Matrix matrix
) {
	ASSERT(matrix != NULL);

	if(matrix->grb_matrix  != NULL) GrB_Matrix_free(&matrix->grb_matrix);
	if(matrix->delta_plus  != NULL) GrB_Matrix_free(&matrix->delta_plus);
	if(matrix->delta_minus != NULL) GrB_Matrix_free(&matrix->delta_minus);
	pthread_mutex_destroy(&matrix->mutex);
	rm_free(matrix);
}

