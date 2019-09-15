//------------------------------------------------------------------------------
// GB_mxm.h: definitions for C=A*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_MXM_H
#define GB_MXM_H
#include "GB.h"

GrB_Info GB_mxm                     // C<M> = A*B
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, use !M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_Semiring semiring,    // defines '+' and '*' for C=A*B
    const GrB_Matrix A,             // input matrix
    const bool A_transpose,         // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    const bool B_transpose,         // if true, use B' instead of B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
    GB_Context Context
) ;

GrB_Info GB_AxB_saxpy_parallel      // parallel C=A*B multiply
(
    GrB_Matrix *Chandle,            // output matrix, NULL on input
    GrB_Matrix M,                   // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
    GrB_Desc_Value *AxB_method_used,// method selected by thread zero
    bool *mask_applied,             // if true, mask was applied
    GB_Context Context
) ;

GrB_Info GB_AxB_dot_parallel        // parallel C=A'*B
(
    GrB_Matrix *Chandle,            // output matrix, NULL on input
    GrB_Matrix M,                   // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    GB_Context Context
) ;

void GB_AxB_select                  // select method for A*B
(
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
    // output
    GrB_Desc_Value *AxB_method_used,        // method to use
    int64_t *bjnz_max                       // # entries in densest col of B
) ;

GrB_Info GB_AxB_saxpy_sequential    // single-threaded C<M>=A*B
(
    GrB_Matrix *Chandle,            // output matrix, NULL on input
    GrB_Matrix M,                   // optional mask matrix
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    const GrB_Desc_Value AxB_method,// already chosen
    const int64_t bjnz_max,         // for heap method only
    const bool check_for_dense_mask,// if true, check floplimit for mask 
    bool *mask_applied,             // if true, mask was applied
    const int Sauna_id              // Sauna to use, for Gustavson method only
) ;

bool GB_AxB_flopcount           // compute flops for C<M>=A*B or C=A*B
(
    int64_t *Bflops,            // size B->nvec+1 and all zero, if present
    int64_t *Bflops_per_entry,  // size nnz(B)+1 and all zero, if present
    const GrB_Matrix M,         // optional mask matrix
    const GrB_Matrix A,
    const GrB_Matrix B,
    int64_t floplimit,          // maximum flops to compute if Bflops NULL
    GB_Context Context
) ;

GrB_Info GB_AxB_heap                // C<M>=A*B or C=A*B using a heap
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M_in,          // mask matrix for C<M>=A*B
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    const int64_t bjnz_max          // max # entries in any vector of B
) ;

GrB_Info GB_AxB_Gustavson           // C=A*B or C<M>=A*B, Gustavson's method
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M_in,          // optional matrix
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A,             // input matrix A
    const GrB_Matrix B,             // input matrix B
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    const int Sauna_id              // Sauna to use
) ;

GrB_Info GB_AxB_meta                // C<M>=A*B meta algorithm
(
    GrB_Matrix *Chandle,            // output matrix C
    const bool C_is_csc,            // desired CSR/CSC format of C
    GrB_Matrix *MT_handle,          // return MT = M' to caller, if computed
    const GrB_Matrix M_in,          // mask for C<M> (not complemented)
    const bool Mask_comp,           // if true, use !M
    const GrB_Matrix A_in,          // input matrix
    const GrB_Matrix B_in,          // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    bool A_transpose,               // if true, use A', else A
    bool B_transpose,               // if true, use B', else B
    bool flipxy,                    // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    const GrB_Desc_Value AxB_method,// for auto vs user selection of methods
    GrB_Desc_Value *AxB_method_used,// method selected
    GB_Context Context
) ;

GrB_Info GB_AxB_rowscale            // C = D*B, row scale with diagonal D
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix D,             // diagonal input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=D*A
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
) ;

GrB_Info GB_AxB_colscale            // C = A*D, column scale with diagonal D
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix D,             // diagonal input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*D
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
) ;

GrB_Info GB_AxB_alloc           // estimate nnz(C) and allocate C for C=A*B
(
    GrB_Matrix *Chandle,        // output matrix
    const GrB_Type ctype,       // type of C
    const GrB_Index cvlen,      // vector length of C
    const GrB_Index cvdim,      // # of vectors of C
    const GrB_Matrix M,         // optional mask
    const GrB_Matrix A,         // input matrix A
    const GrB_Matrix B,         // input matrix B
    const bool numeric,         // if true, allocate A->x, else A->x is NULL
    const int64_t cnz_extra     // added to the rough estimate (if M NULL)
) ;

bool GB_AxB_semiring_builtin        // true if semiring is builtin
(
    // inputs:
    const GrB_Matrix A,
    const bool A_is_pattern,        // true if only the pattern of A is used
    const GrB_Matrix B,
    const bool B_is_pattern,        // true if only the pattern of B is used
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // true if z=fmult(y,x), flipping x and y
    // outputs, unused by caller if this function returns false
    GB_Opcode *mult_opcode,         // multiply opcode
    GB_Opcode *add_opcode,          // add opcode
    GB_Type_code *xycode,           // type code for x and y inputs
    GB_Type_code *zcode             // type code for z output
) ;

GrB_Info GB_AxB_Gustavson_builtin
(
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix M,             // M matrix for C<M> (not complemented)
    const GrB_Matrix A,             // input matrix
    const bool A_is_pattern,        // true if only the pattern of A is used
    const GrB_Matrix B,             // input matrix
    const bool B_is_pattern,        // true if only the pattern of B is used
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Sauna Sauna                  // sparse accumulator
) ;

GrB_Info GB_AxB_dot2                // C = A'*B using dot product method
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M,             // mask matrix for C<!M>=A'*B
#if 0
    // for dot2, if the mask M is present, this is now always true.
    // dot3 is used for C<M>=A'*B
    const bool Mask_comp,           // if true, use !M
#endif
    const GrB_Matrix *Aslice,       // input matrices (already sliced)
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    bool *mask_applied,             // if true, mask was applied
    int nthreads,
    int naslice,
    int nbslice,
    GB_Context Context
) ;

bool GB_is_diagonal             // true if A is diagonal
(
    const GrB_Matrix A,         // input matrix to examine
    GB_Context Context
) ;

GrB_Info GB_hcat_fine_slice // horizontal concatenation and sum of slices of C
(
    GrB_Matrix *Chandle,    // output matrix C to create
    int nthreads,           // # of slices to concatenate
    GrB_Matrix *Cslice,     // array of slices of size nthreads
    GrB_Monoid add,         // monoid to use to sum up the entries
    int *Sauna_ids,         // size nthreads, Sauna id's of each thread
    GB_Context Context
) ;

GrB_Info GB_hcat_slice      // horizontal concatenation of the slices of C
(
    GrB_Matrix *Chandle,    // output matrix C to create
    int nthreads,           // # of slices to concatenate
    GrB_Matrix *Cslice,     // array of slices of size nthreads
    GB_Context Context
) ;

GrB_Info GB_fine_slice  // slice B into nthreads fine hyperslices
(
    GrB_Matrix B,       // matrix to slice
    int nthreads,       // # of slices to create
    int64_t *Slice,     // array of size nthreads+1 that defines the slice
    GrB_Matrix *Bslice, // array of output slices, of size nthreads
    GB_Context Context
) ;

GrB_Info GB_AxB_user
(
    const GrB_Desc_Value GB_AxB_method,
    const GrB_Semiring GB_s,

    GrB_Matrix *GB_Chandle,
    const GrB_Matrix GB_M,
    const GrB_Matrix GB_A,          // not used for dot2 method
    const GrB_Matrix GB_B,
    bool GB_flipxy,

    // for heap method only:
    int64_t *restrict GB_List,
    GB_pointer_pair *restrict GB_pA_pair,
    GB_Element *restrict GB_Heap,
    const int64_t GB_bjnz_max,

    // for Gustavson's method only:
    GB_Sauna GB_C_Sauna,

    // for dot method only:
    const GrB_Matrix *GB_Aslice,    // for dot2 only
    const int GB_dot_nthreads,      // for dot2 and dot3
    const int GB_naslice,           // for dot2 only
    const int GB_nbslice,           // for dot2 only
    int64_t **GB_C_counts,          // for dot2 only

    // for dot3 method only:
    const GB_task_struct *restrict GB_TaskList,
    const int GB_ntasks
) ;

GrB_Info GB_AxB_dot3                // C<M> = A'*B using dot product method
(
    GrB_Matrix *Chandle,            // output matrix
    const GrB_Matrix M,             // mask matrix for C<M>=A'*B or C<!M>=A'*B
    const GrB_Matrix A,             // input matrix
    const GrB_Matrix B,             // input matrix
    const GrB_Semiring semiring,    // semiring that defines C=A*B
    const bool flipxy,              // if true, do z=fmult(b,a) vs fmult(a,b)
    GB_Context Context
) ;

GrB_Info GB_AxB_dot3_slice
(
    // output:
    GB_task_struct **p_TaskList,    // array of structs, of size max_ntasks
    int *p_max_ntasks,              // size of TaskList
    int *p_ntasks,                  // # of tasks constructed
    int *p_nthreads,                // # of threads to use
    // input:
    const GrB_Matrix C,             // matrix to slice
    GB_Context Context
) ;

GrB_Info GB_AxB_dot3_one_slice
(
    // output:
    GB_task_struct **p_TaskList,    // array of structs, of size max_ntasks
    int *p_max_ntasks,              // size of TaskList
    int *p_ntasks,                  // # of tasks constructed
    int *p_nthreads,                // # of threads to use
    // input:
    const GrB_Matrix M,             // matrix to slice
    GB_Context Context
) ;

#endif

