#include "./GxB_Pending.h"

bool GxB_Matrix_Pending
(
    GrB_Matrix M
)
{
    return (M->nzombies > 0) || (M->npending > 0);
}