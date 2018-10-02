#include "./GxB_Delete.h"

GrB_Info GxB_Matrix_Delete
(
    GrB_Matrix M,
    GrB_Index row,
    GrB_Index col
)
{
    GrB_Matrix Z ;  // 1X1 empty matrix.
    GrB_Matrix_new (&Z, GrB_BOOL, 1, 1) ;

    return GxB_Matrix_subassign (M,
                                 GrB_NULL,
                                 GrB_NULL,
                                 Z,
                                 &row,
                                 1,
                                 &col,
                                 1,
                                 GrB_NULL) ;
}
