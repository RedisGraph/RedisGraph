/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "rg_matrix.h"

void RG_Matrix_checkBounds
(
	const RG_Matrix C,
	GrB_Index i,
	GrB_Index j
);

void RG_Matrix_checkCompatible
(
	const RG_Matrix M,
	const RG_Matrix N
);

// validate 'C' isn't in an invalid state
void RG_Matrix_validateState
(
	const RG_Matrix C,
	GrB_Index i,
	GrB_Index j
);

