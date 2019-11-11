/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdint.h>
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

/* Clones given matrix. */
GrB_Info GxB_MatrixClone
(
    const GrB_Matrix A,
    GrB_Matrix *C
) ;
