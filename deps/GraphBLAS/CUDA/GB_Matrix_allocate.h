
#ifndef GB_MATRIX_ALLOCATE_H
#define GB_MATRIX_ALLOCATE_H
#include "matrix.h"
#include "pmr_malloc.h"

#ifdef __cplusplus
extern "C" {
#endif

GrB_Matrix GB_Matrix_allocate
        (
                GrB_Type type,          // NULL on the GPU
                size_t type_size,       // type->size
                int64_t nrows,
                int64_t ncols,
                int sparsity,   //GxB_FULL, ..
                bool is_csc,
                bool iso,
                int64_t anz,    // ignored if sparsity is GxB_FULL or GxB_BITMAP
                int64_t nvec    // hypersparse only
        );

GrB_Vector GB_Vector_allocate
        (
                GrB_Type type,          // NULL on the GPU
                size_t type_size,       // type->size
                int64_t length,
                int sparsity,   // GxB_FULL, GxB_BITMAP, or GxB_SPARSE
                bool iso,
                int64_t anz     // ignored if sparsity is GxB_FULL or GxB_BITMAP
        );

GrB_Scalar GB_Scalar_allocate
        (
                GrB_Type type,          // NULL on the GPU
                size_t type_size,       // type->size
                int sparsity   // GxB_FULL, GxB_BITMAP, or GxB_SPARSE
        );

#ifdef __cplusplus
}
#endif

#endif

