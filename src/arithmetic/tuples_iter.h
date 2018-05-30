#ifndef __TUPLES_ITER_H__
#define __TUPLES_ITER_H__

#include <stdint.h>
#include "GraphBLAS.h"

typedef enum
{
    TuplesIter_OK,
    TuplesIter_DEPLETED
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
    GrB_Matrix M
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
    GrB_Matrix M
) ;

void TuplesIter_free
(
    TuplesIter *iter
) ;

#endif
