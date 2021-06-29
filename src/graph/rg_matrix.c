#include "RG.h"
#include "rg_matrix.h"
#include "../util/rmalloc.h"

#define DELTA_MAX_PENDING_CHANGES 1000
#define RG_MATRIX_MATRIX(rg_matrix) (rg_matrix)->grb_matrix
#define RG_MATRIX_DELTA_PLUS(rg_matrix) (rg_matrix)->delta_plus
#define RG_MATRIX_DELTA_MINUS(rg_matrix) (rg_matrix)->delta_minus

extern GrB_BinaryOp _graph_edge_accum;

// Creates a new matrix
GrB_Info RG_Matrix_new
(
	RG_Matrix *A,
	GrB_Type type,
	GrB_Index nrows,
	GrB_Index ncols,
	bool multi_edge
) {
	GrB_Info info;
	RG_Matrix matrix = rm_calloc(1, sizeof(_RG_Matrix));

	matrix->dirty = true;
	matrix->multi_edge = multi_edge;

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
	const RG_Matrix C
) {
	ASSERT(C != NULL);
	return RG_MATRIX_MATRIX(C);
}

// returns underlying delta plus GraphBLAS matrix
GrB_Matrix RG_Matrix_Get_DeltaPlus
(
	RG_Matrix C
) {
	ASSERT(C != NULL);
	return RG_MATRIX_DELTA_PLUS(C);
}

bool RG_Matrix_IsDirty
(
	const RG_Matrix C
) {
	ASSERT(C);
	return C->dirty;
}

void RG_Matrix_SetDirty
(
	RG_Matrix C
) {
	ASSERT(C);
	C->dirty = true;
}

void RG_Matrix_SetUnDirty
(
	RG_Matrix C
) {
	ASSERT(C);
	C->dirty = false;
}

// locks the matrix
void RG_Matrix_Lock
(
	RG_Matrix C
) {
	ASSERT(C);
	pthread_mutex_lock(&C->mutex);
}

// unlocks the matrix
void RG_Matrix_Unlock
(
	RG_Matrix C
) {
	ASSERT(C);
	pthread_mutex_unlock(&C->mutex);
}

void RG_Matrix_setMultiEdge
(
	RG_Matrix C,
	bool multi_edge
) {
	ASSERT(C);
	C->multi_edge = multi_edge;
}

bool RG_Matrix_getMultiEdge
(
	const RG_Matrix C
) {
	ASSERT(C);
	return C->multi_edge;
}

GrB_Info RG_Matrix_resize       // change the size of a matrix
(
    RG_Matrix C,                // matrix to modify
    GrB_Index nrows_new,        // new number of rows in matrix
    GrB_Index ncols_new         // new number of columns in matrix
) {
	ASSERT(C != NULL);
	GrB_Info info;

	GrB_Matrix  m            =  RG_MATRIX_MATRIX(C);
	GrB_Matrix  delta_plus   =  RG_MATRIX_DELTA_PLUS(C);
	GrB_Matrix  delta_minus  =  RG_MATRIX_DELTA_MINUS(C);

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
	GrB_Matrix delta_plus = RG_MATRIX_DELTA_PLUS(C);
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
	GrB_Matrix delta_plus = RG_MATRIX_DELTA_PLUS(C);
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
	GrB_Matrix delta_plus = RG_MATRIX_DELTA_PLUS(C);

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

	GrB_Matrix      m            =  RG_MATRIX_MATRIX(C);
	GrB_Descriptor  desc         =  GrB_NULL;
	GrB_Matrix      mask         =  GrB_NULL;
	GrB_Matrix      delta_plus   =  RG_MATRIX_DELTA_PLUS(C);
	GrB_Matrix      delta_minus  =  RG_MATRIX_DELTA_MINUS(C);

	GrB_Info info;
	GrB_Semiring semiring;
	GrB_Index delta_plus_nvals;
	GrB_Matrix_nvals(&delta_plus_nvals, delta_plus);

	// no changes
	if(delta_plus_nvals == 0) return GrB_SUCCESS;

	GrB_Index delta_minus_nvals;
	GrB_Matrix_nvals(&delta_minus_nvals, delta_minus);
	if(delta_minus_nvals > 0) {
		mask = delta_minus;
		desc = GrB_DESC_SC;
	}

	GxB_print(m, GxB_COMPLETE);
	GxB_print(delta_plus, GxB_COMPLETE);
	GxB_print(delta_minus, GxB_COMPLETE);

	GrB_Type t;
	info = GxB_Matrix_type(&t, m);

	if(t == GrB_BOOL) {
		semiring = GxB_ANY_PAIR_BOOL;
	} else {
		// TODO: figure out which semiring to use
		semiring = GxB_ANY_SECOND_UINT64;
	}

	if(C->multi_edge) {
		// add delta-plus to 'm' using delta-minus as a mask
		info = GrB_Matrix_eWiseAdd_Semiring(m, mask, _graph_edge_accum,
				semiring, m, delta_plus, desc);
		ASSERT(info == GrB_SUCCESS);
	} else {
		info = GrB_Matrix_eWiseAdd_Semiring(m, mask, GrB_NULL,
				semiring, m, delta_plus, desc);
		ASSERT(info == GrB_SUCCESS);
	}

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
	GrB_Matrix  m            =  RG_MATRIX_MATRIX(A);
	GrB_Matrix  delta_plus   =  RG_MATRIX_DELTA_PLUS(A);
	GrB_Matrix  delta_minus  =  RG_MATRIX_DELTA_MINUS(A);

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
	RG_Matrix C
) {
	ASSERT(C != NULL);

	if(C->grb_matrix  != NULL) GrB_Matrix_free(&C->grb_matrix);
	if(C->delta_plus  != NULL) GrB_Matrix_free(&C->delta_plus);
	if(C->delta_minus != NULL) GrB_Matrix_free(&C->delta_minus);
	pthread_mutex_destroy(&C->mutex);
	rm_free(C);
}

