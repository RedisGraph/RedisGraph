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
	assert(nrows == ncols);

	// Clear input matrix
	GrB_Matrix_clear(M);

	// Calculate number of cells to set
	GrB_Index to_set_count = (density_ratio * nrows) * ncols;
	// printf("Number of cells: %lu\n", to_set_count);

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
	GrB_Index ncols;
	GrB_Matrix_ncols(&ncols, M);

	GrB_Index row = ncols / 2;
	GrB_Index col = ncols / 3;

	GrB_Matrix_setElement_BOOL(M, true, row, col);
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
	else assert(false) ;

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

void delta_multiply(GrB_Matrix Output, GrB_Matrix M, GrB_Matrix M_plus,
					GrB_Matrix M_minus, GrB_Matrix F) {
	GrB_Info res;
	GrB_Descriptor desc = GrB_DESC_RSC;

	// O = F * M
	res = GrB_mxm(Output, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, F, M, GrB_NULL);
	assert(res == GrB_SUCCESS);

	GrB_Index nrows;
	GrB_Index ncols;
	GrB_Matrix_nrows(&nrows, F);
	GrB_Matrix_ncols(&ncols, M);
	// M_plus = F * M_plus
	GrB_Matrix M_plus_output;
	GrB_Matrix_new(&M_plus_output, GrB_BOOL, nrows, ncols);
	res = GrB_mxm(M_plus_output, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, F, M_plus, GrB_NULL);
	assert(res == GrB_SUCCESS);
	GrB_Index plus_count;
	GrB_Matrix_nvals(&plus_count, M_plus_output);

	// M_minus = F * M_minus
	GrB_Matrix M_minus_output;
	GrB_Matrix_new(&M_minus_output, GrB_BOOL, nrows, ncols);
	res = GrB_mxm(M_minus_output, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, F, M_minus, GrB_NULL);
	assert(res == GrB_SUCCESS);
	GrB_Index minus_count;
	GrB_Matrix_nvals(&minus_count, M_minus_output);
	if(minus_count == 0) {
		M_minus = GrB_NULL;
		desc = GrB_NULL;
	}

	// O = (O + F) < M_minus >
	if(plus_count > 0) {
		res = GrB_eWiseAdd(Output, M_minus_output, GrB_NULL, GxB_ANY_PAIR_BOOL,
						   M_plus_output, Output, desc);
		assert(res == GrB_SUCCESS);
	} else if(minus_count > 0) {
		res = GrB_transpose(Output, M_minus, GrB_NULL, Output, GrB_DESC_RSCT0);
		assert(res == GrB_SUCCESS);
	}
}

void standard_multiply(GrB_Matrix Output, GrB_Matrix M, GrB_Matrix F) {
	GrB_Info res;

	// F = F * M
	res = GrB_mxm(Output, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, F, M, GrB_NULL);
	assert(res == GrB_SUCCESS);
}

double Multiply_Standard(GrB_Matrix Output, GrB_Matrix M, GrB_Matrix M_plus,
						 GrB_Matrix M_minus, GrB_Matrix F) {
	GrB_Info res;

	GrB_Matrix M_dup;
	GrB_Matrix_dup(&M_dup, M);

	_DirtyMatrix(M_plus);
	_DirtyMatrix(M_minus);

	res = GrB_eWiseAdd(M_dup, M_minus, GrB_NULL, GxB_ANY_PAIR_BOOL, M_dup, M_plus, GrB_DESC_RSC);
	assert(res == GrB_SUCCESS);

	_DirtyMatrix(M_dup);

	double tic[2];
	// Start timer.
	simple_tic(tic);

	standard_multiply(Output, M_dup, F);

	double time = simple_toc(tic);
	printf("Standard multiplication time: %f\n", time * 1000);

	GrB_Matrix_free(&M_dup);
	return time;
}

double Multiply_Delta(GrB_Matrix Output, GrB_Matrix M, GrB_Matrix M_plus,
					  GrB_Matrix M_minus, GrB_Matrix F) {
	double tic[2];
	// Start timer.
	simple_tic(tic);

	delta_multiply(Output, M, M_plus, M_minus, F);

	double time = simple_toc(tic);
	printf("Delta multiplication time: %f\n", time * 1000);
	return time;
}

double runner(GrB_Index dims, GrB_Index F_nrows, GrB_Matrix M, GrB_Matrix M_plus,
			  GrB_Matrix M_minus, GrB_Matrix F, GrB_Matrix StandardOutput,
			  GrB_Matrix DeltaOutput) {
	float density_ratio = 1.0 / dims;
	float plus_density_ratio = 0.00001 / dims;
	float minus_density_ratio = 0.00001 / dims;

	// Set a seed for the random number generator
	// simple_rand_seed(0);

	_PopulateMatrix(M, density_ratio);

	_PopulateMatrix(M_plus, plus_density_ratio);

	_PopulateMatrix(M_minus, minus_density_ratio);

	_PopulateFMatrix(F, F_nrows);

	GrB_Matrix_clear(StandardOutput);
	GrB_Matrix_clear(DeltaOutput);

	double standard_time = Multiply_Standard(StandardOutput, M, M_plus, M_minus, F);

	// Dirty and synchronize M for parity with standard multiplication matrices
	_DirtyMatrix(M);
	GrB_wait(&M);

	// Dirty and DON'T synchronize M_plus and M_minus
	_DirtyMatrix(M_plus);
	_DirtyMatrix(M_minus);

	double delta_time = Multiply_Delta(DeltaOutput, M, M_plus, M_minus, F);

	bool matrices_are_equal = MatricesAreEqual(DeltaOutput, StandardOutput);
	if(!matrices_are_equal) {
		printf("Matrices are NOT equal\n");
	}

	double percent_change = (standard_time - delta_time) / delta_time;
	return percent_change;
}

int main(int argc, char **argv) {
	// Matrix dimensions
	GrB_Index F_nrows = 16;
	GrB_Index dims = 50000000;

	// Initialize GraphBLAS
	GrB_Info res = GxB_init(GrB_NONBLOCKING, malloc, calloc, realloc, free, true);
	assert(res == GrB_SUCCESS);
	GxB_set(GxB_FORMAT, GxB_BY_ROW); // all matrices in CSR format

	// Initialize matrices
	GrB_Matrix M;
	GrB_Matrix_new(&M, GrB_BOOL, dims, dims);
	GxB_Matrix_Option_set(M, GxB_SPARSITY_CONTROL, GxB_SPARSE);

	GrB_Matrix M_plus;
	GrB_Matrix_new(&M_plus, GrB_BOOL, dims, dims);
	GxB_Matrix_Option_set(M_plus, GxB_SPARSITY_CONTROL, GxB_HYPERSPARSE);

	GrB_Matrix M_minus;
	GrB_Matrix_new(&M_minus, GrB_BOOL, dims, dims);
	GxB_Matrix_Option_set(M_minus, GxB_SPARSITY_CONTROL, GxB_HYPERSPARSE);

	GrB_Matrix F;
	GrB_Matrix_new(&F, GrB_BOOL, F_nrows, dims);

	GrB_Matrix StandardOutput;
	GrB_Matrix_new(&StandardOutput, GrB_BOOL, F_nrows, dims);

	GrB_Matrix DeltaOutput;
	GrB_Matrix_new(&DeltaOutput, GrB_BOOL, F_nrows, dims);

	int run_count = 10;
	double avg_percent_change = 0;
	for(int i = 0; i < run_count; i ++) {
		avg_percent_change += runner(dims, F_nrows, M, M_plus, M_minus, F,
									 StandardOutput, DeltaOutput);
	}
	avg_percent_change /= (double)run_count;
	printf("Average improvement of %f over %d runs\n", avg_percent_change, run_count);

	// cleanup
	GrB_Matrix_free(&M);
	GrB_Matrix_free(&M_plus);
	GrB_Matrix_free(&M_minus);
	GrB_Matrix_free(&F);
	GrB_Matrix_free(&DeltaOutput);
	GrB_Matrix_free(&StandardOutput);

	return 0;
}

