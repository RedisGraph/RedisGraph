#include <stdlib.h>
#include "simple_rand.h"
#include "simple_timer.h"
#include "../deps/GraphBLAS/Include/GraphBLAS.h"
#include "../src/RG.h"

static void _PopulateMatrix(GrB_Matrix M, float density_ratio) {
	GrB_Index nrows;
	GrB_Index ncols;
	GrB_Matrix_nrows(&nrows, M);
	GrB_Matrix_ncols(&ncols, M);
	ASSERT(nrows == ncols);

	// Calculate number of cells to set
	GrB_Index to_set_count = (density_ratio * nrows) * ncols;

	for(GrB_Index k = 0; k < to_set_count; k ++) {
		GrB_Index i = simple_rand_i() % nrows ;
		GrB_Index j = simple_rand_i() % ncols;
		// A (i,j) = x
		GrB_Matrix_setElement_BOOL(M, true, i, j);
	}
	GrB_wait(&M);
}

static void _PopulateFMatrix(GrB_Matrix M, GrB_Index F_nrows) {
	GrB_Index ncols;
	GrB_Matrix_ncols(&ncols, M);

	for(GrB_Index i = 0; i < F_nrows; i ++) {
		GrB_Index j = simple_rand_i() % ncols;
		// A (i,j) = x
		GrB_Matrix_setElement_BOOL(M, true, i, j);
	}
	GrB_wait(&M);
}

static void _DirtyMatrix(GrB_Matrix M) {
	GrB_Matrix_setElement_BOOL(M, true, 0, 3);
}

bool MatricesAreEqual(GrB_Matrix A, GrB_Matrix B) {
	bool _result = true ;
	bool *result = &_result ;
	GrB_BinaryOp compare ;
	GrB_Type atype ;
	GxB_Matrix_type(&atype, A) ;
	if(atype == GrB_BOOL) compare = GrB_EQ_BOOL   ;
	else if(atype == GrB_INT8) compare = GrB_EQ_INT8   ;
	else if(atype == GrB_INT16) compare = GrB_EQ_INT16  ;
	else if(atype == GrB_INT32) compare = GrB_EQ_INT32  ;
	else if(atype == GrB_INT64) compare = GrB_EQ_INT64  ;
	else if(atype == GrB_UINT8) compare = GrB_EQ_UINT8  ;
	else if(atype == GrB_UINT16) compare = GrB_EQ_UINT16 ;
	else if(atype == GrB_UINT32) compare = GrB_EQ_UINT32 ;
	else if(atype == GrB_UINT64) compare = GrB_EQ_UINT64 ;
	else if(atype == GrB_FP32) compare = GrB_EQ_FP32   ;
	else if(atype == GrB_FP64) compare = GrB_EQ_FP64   ;
	else if(atype == GxB_FC32) compare = GxB_EQ_FC32   ;
	else if(atype == GxB_FC64) compare = GxB_EQ_FC64   ;
	else ASSERT(false) ;

	GrB_Index nrows1, ncols1, nrows2, ncols2 ;
	GrB_Matrix_nrows(&nrows1, A) ;
	GrB_Matrix_nrows(&nrows2, B) ;
	if(nrows1 != nrows2) {
		// # of rows differ
		return false ;
	}

	GrB_Matrix_ncols(&ncols1, A) ;
	GrB_Matrix_ncols(&ncols2, B) ;
	if(ncols1 != ncols2) {
		// # of cols differ
		return false ;
	}

	//--------------------------------------------------------------------------
	// compare the # entries in A and B
	//--------------------------------------------------------------------------

	GrB_Index nvals1, nvals2 ;
	GrB_Matrix_nvals(&nvals1, A) ;
	GrB_Matrix_nvals(&nvals2, B) ;
	if(nvals1 != nvals2) {
		// # of entries differ
		return false ;
	}

	//--------------------------------------------------------------------------
	// C = A .* B, where the pattern of C is the intersection of A and B
	//--------------------------------------------------------------------------

	GrB_Matrix C = NULL ;
	GrB_Matrix_new(&C, GrB_BOOL, nrows1, ncols1) ;
	GrB_eWiseMult(C, NULL, NULL, compare, A, B, NULL) ;

	//--------------------------------------------------------------------------
	// ensure C has the same number of entries as A and B
	//--------------------------------------------------------------------------

	GrB_Index nvals ;
	GrB_Matrix_nvals(&nvals, C) ;
	if(nvals != nvals1) {
		// pattern of A and B are different
		return false ;
	}

	//--------------------------------------------------------------------------
	// result = and (C)
	//--------------------------------------------------------------------------

	GrB_reduce(result, NULL, GrB_LAND_MONOID_BOOL, C, NULL) ;

	return *result;
}

GrB_Matrix delta_multiply(GrB_Matrix Output, GrB_Matrix M, GrB_Matrix M_plus,
						  GrB_Matrix M_minus, GrB_Matrix F, GrB_Matrix N) {
	GrB_Info res;

	GrB_Descriptor desc;
	GrB_Descriptor_new(&desc);

	// O = F * M
	res = GrB_mxm(Output, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, F, M, GrB_NULL);
	ASSERT(res == GrB_SUCCESS);

	// F = F * M_plus
	res = GrB_mxm(F, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, F, M, GrB_NULL);
	ASSERT(res == GrB_SUCCESS);
	// O = (O + F) < M_minus >
	GrB_Descriptor_set(desc, GrB_MASK, GrB_COMP);
	// res = GrB_eWiseAdd(Output, M_minus, GrB_NULL, GxB_ANY_PAIR_BOOL, F, M_plus, desc);
	res = GrB_eWiseAdd(F, M_minus, GrB_NULL, GxB_ANY_PAIR_BOOL, F, M_plus, desc);
	ASSERT(res == GrB_SUCCESS);

	// Output = F * N
	// res = GrB_mxm(Output, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, F, M, GrB_NULL);
	// ASSERT(res == GrB_SUCCESS);

	GrB_Descriptor_free(&desc);

	return F;
}

void standard_multiply(GrB_Matrix Output, GrB_Matrix M, GrB_Matrix F, GrB_Matrix N) {
	GrB_Info res;

	// F = F * M
	res = GrB_mxm(F, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, F, M, GrB_NULL);
	ASSERT(res == GrB_SUCCESS);

	// Output = F * N
	res = GrB_mxm(Output, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, F, M, GrB_NULL);
	ASSERT(res == GrB_SUCCESS);
}

GrB_Matrix Multiply_Standard(GrB_Matrix M, GrB_Matrix M_plus, GrB_Matrix M_minus,
							 GrB_Matrix F, GrB_Matrix N) {
	GrB_Info res;

	GrB_Matrix M_dup;
	GrB_Matrix_dup(&M_dup, M);

	GrB_Matrix F_dup;
	GrB_Matrix_dup(&F_dup, F);

	GrB_Descriptor desc;
	GrB_Descriptor_new(&desc);
	GrB_Descriptor_set(desc, GrB_MASK, GrB_COMP);

	res = GrB_eWiseAdd(M_dup, M_minus, GrB_NULL, GxB_ANY_PAIR_BOOL, M_dup, M_plus, desc);
	ASSERT(res == GrB_SUCCESS);

	_DirtyMatrix(M_dup);

	standard_multiply(F_dup, M_dup, F_dup, N);

	GrB_Descriptor_free(&desc);
	GrB_Matrix_free(&M_dup);

	return F_dup;
}

GrB_Matrix Multiply_Delta(GrB_Matrix M, GrB_Matrix M_plus, GrB_Matrix M_minus,
						  GrB_Matrix F, GrB_Matrix N) {
	GrB_Info res;

	GrB_Matrix delta_output = delta_multiply(Output, M, M_plus, M_minus, F, N);
}

bool runner(GrB_Matrix M, GrB_Matrix M_plus, GrB_Matrix M_minus,
			GrB_Matrix F, GrB_Matrix N) {
}

int main(int argc, char **argv) {
	GrB_Index F_nrows = 16;
	GrB_Index dims = 50000000;
	float density_ratio = 1 / dims;
	float plus_density_ratio = 0.0001 / dims;
	float minus_density_ratio = 0.0001 / dims;
	float multiply_against_ratio = 0.01 / dims;

	// Initialize GraphBLAS
	GrB_Info res = GxB_init(GrB_NONBLOCKING, malloc, calloc, realloc, free, true);
	ASSERT(res == GrB_SUCCESS);
	GxB_set(GxB_FORMAT, GxB_BY_ROW); // all matrices in CSR format

	GrB_Matrix M;
	GrB_Matrix_new(&M, GrB_BOOL, dims, dims);
	_PopulateMatrix(M, density_ratio);

	GrB_Matrix M_plus;
	GrB_Matrix_new(&M_plus, GrB_BOOL, dims, dims);
	_PopulateMatrix(M_plus, plus_density_ratio);

	GrB_Matrix M_minus;
	GrB_Matrix_new(&M_minus, GrB_BOOL, dims, dims);
	_PopulateMatrix(M_minus, minus_density_ratio);

	GrB_Matrix F;
	GrB_Matrix_new(&F, GrB_BOOL, F_nrows, dims);
	_PopulateFMatrix(F, F_nrows);

	GrB_Matrix N = GrB_NULL;
	GrB_Matrix_new(&N, GrB_BOOL, dims, dims);
	_PopulateMatrix(N, multiply_against_ratio);

	GrB_Matrix Output;
	GrB_Matrix_new(&Output, GrB_BOOL, dims, dims);

	GrB_Matrix compare_against = Multiply_Standard(M, M_plus, M_minus, F, N);

	_DirtyMatrix(M);
	GrB_wait(&M);
	GrB_Matrix delta_output = Multiply_Delta(Output, M, M_plus, M_minus, F, N);

	bool matrices_are_equal = MatricesAreEqual(delta_output, compare_against);
	if(matrices_are_equal) {
		printf("Matrices are equal\n");
	} else {
		printf("Matrices are NOT equal\n");
	}

	// cleanup
	GrB_Matrix_free(&M);
	GrB_Matrix_free(&M_plus);
	GrB_Matrix_free(&M_minus);
	GrB_Matrix_free(&F);
	GrB_Matrix_free(&N);
	GrB_Matrix_free(&Output);

	return 0;
}

