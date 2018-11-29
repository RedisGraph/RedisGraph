//------------------------------------------------------------------------------
// GraphBLAS/Demo/bfs_level.c:  unary operator for bfs6.c
//------------------------------------------------------------------------------

// Modified from the GraphBLAS C API Specification, by Aydin Buluc, Timothy
// Mattson, Scott McMillan, Jose' Moreira, Carl Yang.  Based on "GraphBLAS
// Mathematics" by Jeremy Kepner.

#include "demos.h"

//------------------------------------------------------------------------------
// bfs_level: for unary operator
//------------------------------------------------------------------------------

// level = depth in BFS traversal, roots=1, unvisited=0.

// Note the operator accesses a global variable outside the control of
// GraphBLAS.  This is safe, but care must be taken not to change the global
// variable "level" while pending operations have yet to be completed.
// See the User Guide on GrB_wait, which forces completion of pending work
// on all matrices, and also methods that force completion on individual
// matries (GrB_Matrix_nvals in particular).

#pragma omp threadprivate(level)
int32_t level = 0 ;

void bfs_level (int32_t *result, bool *element)
{
    // Note this function does not depend on its input.  It returns the value
    // of the global variable level for all inputs.  It is applied to the
    // vector q via GrB_apply, which only applies the unary operator to entries
    // in the pattern.  Entries not in the pattern remain implicit (zero in
    // this case), and then are not added by the GrB_PLUS_INT32 accum function.
    (*result) = level ;
}

