/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __TUPLES_ITER_H__
#define __TUPLES_ITER_H__

#include <stdint.h>
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

typedef enum
{
    TuplesIter_DEPLETED,
    TuplesIter_OK
} TuplesIter_Info ;

typedef struct
{
    GrB_Matrix A ;
    GrB_Index nvals ;
    GrB_Index nnz_idx ;
    int64_t p ;
    int64_t col_idx ;
} TuplesIter ;

TuplesIter *TuplesIter_new
(
    GrB_Matrix A
) ;

TuplesIter *TuplesIter_iterate_column
(
    TuplesIter *iter,
    GrB_Index colIdx
) ;

TuplesIter_Info TuplesIter_next
(
    TuplesIter *iter,
    GrB_Index *row,
    GrB_Index *col
) ;

void TuplesIter_reset
(
    TuplesIter *iter
) ;

void TuplesIter_reuse
(
    TuplesIter *iter,
    GrB_Matrix A
) ;

void TuplesIter_clear
(
    TuplesIter *iter
) ;

void TuplesIter_free
(
    TuplesIter *iter
) ;

#endif
