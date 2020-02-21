//------------------------------------------------------------------------------
// GB_build.h: definitions for GB_build
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_BUILD_H
#define GB_BUILD_H
#include "GB.h"

GrB_Info GB_matvec_build        // check inputs then build matrix or vector
(
    GrB_Matrix C,               // matrix or vector to build
    const GrB_Index *I,         // row indices of tuples
    const GrB_Index *J,         // col indices of tuples (NULL for vector)
    const void *S,              // array of values of tuples
    const GrB_Index nvals,      // number of tuples
    const GrB_BinaryOp dup,     // binary function to assemble duplicates
    const GB_Type_code scode,   // GB_Type_code of S array
    const bool is_matrix,       // true if C is a matrix, false if GrB_Vector
    GB_Context Context
) ;

GrB_Info GB_build               // build matrix
(
    GrB_Matrix C,               // matrix to build
    const GrB_Index *I_input,   // "row" indices of tuples (as if CSC)
    const GrB_Index *J_input,   // "col" indices of tuples (as if CSC) NULL for
                                // GrB_Vector_build or GB_reduce_to_vector
    const void *S_input,        // values
    const GrB_Index nvals,      // number of tuples
    const GrB_BinaryOp dup,     // binary function to assemble duplicates
    const GB_Type_code scode,   // GB_Type_code of S_input array
    const bool is_matrix,       // true if C is a matrix, false if GrB_Vector
    const bool ijcheck,         // true if I and J are to be checked
    GB_Context Context
) ;

GrB_Info GB_builder                 // build a matrix from tuples
(
    GrB_Matrix *Thandle,            // matrix T to build
    const GrB_Type ttype,           // type of output matrix T
    const int64_t vlen,             // length of each vector of T
    const int64_t vdim,             // number of vectors in T
    const bool is_csc,              // true if T is CSC, false if CSR
    int64_t **I_work_handle,        // for (i,k) or (j,i,k) tuples
    int64_t **J_work_handle,        // for (j,i,k) tuples
    GB_void **S_work_handle,        // array of values of tuples, size ijslen
    bool known_sorted,              // true if tuples known to be sorted
    bool known_no_duplicates,       // true if tuples known to not have dupl
    int64_t ijslen,                 // size of I_work and J_work arrays
    const bool is_matrix,           // true if T a GrB_Matrix, false if vector
    const bool ijcheck,             // true if I_input,J_input must be checked
    const int64_t *GB_RESTRICT I_input,// original indices, size nvals
    const int64_t *GB_RESTRICT J_input,// original indices, size nvals
    const GB_void *GB_RESTRICT S_input,// array of values of tuples, size nvals
    const int64_t nvals,            // number of tuples, and size of K_work
    const GrB_BinaryOp dup,         // binary function to assemble duplicates,
                                    // if NULL use the SECOND operator to
                                    // keep the most recent duplicate.
    const GB_Type_code scode,       // GB_Type_code of S_work or S_input array
    GB_Context Context
) ;

#endif
