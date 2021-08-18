/*
* copyright 2018-2021 redis labs ltd. and contributors
*
* this file is available under the redis labs source available license agreement
*/

#include "RG.h"
#include "vrg_matrix.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

#define VRG_MAT_VERSIONS(A)   A->versions
#define VRG_MAT_N_VER(A)      array_len(VRG_MAT_VERSIONS(A))  // number of maintained versions
#define VRG_MAT_MAT_I(A, i)   VRG_MAT_VERSIONS(A)[i].M        // ith verioned matrix
#define VRG_MAT_VER_I(A, i)   VRG_MAT_VERSIONS(A)[i].version  // version of the ith matrix
#define VRG_MAT_LATEST_VER(A) VRG_MAT_VER_I(A, VRG_MAT_N_VER(A)-1) // latest version

VRG_Matrix VRG_Matrix_new  // create a new versioned matrix
(
	RG_Matrix M,          // RG_matrix to version
	uint version          // initial version of matrix
) {
	ASSERT(M != NULL);

	VRG_Matrix A = rm_malloc(sizeof(*A));
	A->versions = array_new(_VRG_Matrix, 1);

	// add M as the latest version
	_VRG_Matrix a = {.M = M, version: version};
	array_append(A->versions, a);

	return A;
}

RG_Matrix VRG_Matrix_getVersion  // get versioned matrix
(
	const VRG_Matrix A,
	uint version
) {
	ASSERT(A != NULL);
	
	// search for required version
	int n = VRG_MAT_N_VER(A) - 1;
	while(n >=0 && VRG_MAT_VER_I(A, n) > version) n--;
	
	if(n < 0) {
		// required version was not found
		return NULL;
	}

	return VRG_MAT_MAT_I(A, n);
}

void VRG_Matrix_addVersion  // add a new version of matrix
(
	VRG_Matrix A,           // versioned matrix to update
	RG_Matrix M,            // RG_matrix to version
	uint version            // matrix new version
) {
	ASSERT(A != NULL);
	ASSERT(M != NULL);

	// version should be greater than latest version
	ASSERT(version > VRG_MAT_LATEST_VER(A));

	// add M as the latest version
	_VRG_Matrix a = {.M = M, version: version};
	array_append(A->versions, a);
}

void VRG_Matrix_delVersion
(
	VRG_Matrix A,
	uint version
) {
	ASSERT(A != NULL);

	uint n = VRG_MAT_N_VER(A);
	// TODO: for large number of versions use binary search
	for(uint i = 0; i < n; i++) {
		if(VRG_MAT_VER_I(A, i) == version) {
			RG_Matrix_free(&VRG_MAT_MAT_I(A, i));
			array_del(A->versions, i);
			break;
		}
	}
}

void VRG_Matrix_free  // free a versioned matrix
(
	VRG_Matrix *A     // handle of matrix to free
) {
	ASSERT(A != NULL);

	VRG_Matrix a = *A;
	uint n = VRG_MAT_N_VER(a);

	// free each individual versions
	for(uint i = 0; i < n; i++) {
		RG_Matrix m = VRG_MAT_MAT_I(a, i);
		RG_Matrix_free(&m);
	}

	// free versioned matrix
	array_free(a->versions);

	*A = NULL;
}

