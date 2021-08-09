/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
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