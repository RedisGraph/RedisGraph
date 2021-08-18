/*
* copyright 2018-2021 redis labs ltd. and contributors
*
* this file is available under the redis labs source available license agreement
*/

#pragma once

#include "../rg_matrix/rg_matrix.h"

typedef struct {
	uint version; // version associated with matrix
	RG_Matrix M;  // delta-matrix
} _VRG_Matrix;

// versioned RG_Matrix
typedef struct {
	_VRG_Matrix *versions;  // array of maintained versions
} __VRG_Matrix, *VRG_Matrix;

VRG_Matrix VRG_Matrix_new        // create a new versioned matrix
(
	RG_Matrix M,                // RG_matrix to version
	uint version                // initial version of matrix
);

RG_Matrix VRG_Matrix_getVersion  // get versioned matrix
(
	const VRG_Matrix A,          // versioned matrix to extract from
	uint version                 // required version
);

void VRG_Matrix_addVersion       // add a new version of matrix
(
	VRG_Matrix A,                // versioned matrix to update
	RG_Matrix M,                 // RG_matrix to version
	uint version                 // matrix new version
);

void VRG_Matrix_delVersion       // delete version
(
	VRG_Matrix A,                // versioned matrix to update
	uint version                 // version to delete
);

void VRG_Matrix_free             // free a versioned matrix
(
	VRG_Matrix *A                // handle of matrix to free
);

