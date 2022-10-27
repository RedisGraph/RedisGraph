
// Decide branch direction for GPU use for the dot-product MxM
#include "GB_cuda.h"

bool GB_reduce_to_scalar_cuda_branch 
(
    const GrB_Monoid reduce,        // monoid to do the reduction
    const GrB_Matrix A,             // input matrix
    GB_Context Context
)
{

    // work to do
    double work = GB_nnz (A) ;

    int ngpus_to_use = GB_ngpus_to_use (work) ;
    GBURBLE (" work:%g gpus:%d ", work, ngpus_to_use) ;

    GB_Opcode opcode = reduce->op->opcode ;

    if (ngpus_to_use > 0
        // do it on the CPU if the monoid operator is user-defined:
        // FIXME: handle user-defined operators
        && (opcode != GB_USER_binop_code)
        // the ANY monoid takes O(1) time; do it on the CPU:
        && (opcode != GB_ANY_binop_code)
        // FIXME: handle user-defined types:
        && (A->type->code != GB_UDT_code)
        // A iso takes O(log(nvals(A))) time; do it on the CPU:
        && !A->iso
    )
    {
        return true;
    }
    else
    { 
        return false;
    }
}

