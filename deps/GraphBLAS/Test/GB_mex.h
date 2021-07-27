//------------------------------------------------------------------------------
// GB_mex.h: definitions for the Test interface to GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// to turn on memory usage debug printing, uncomment this line:
// #define GB_PRINT_MALLOC 1

#ifndef GB_MEXH
#define GB_MEXH

#include "GB_mxm.h"
#include "GB_Pending.h"
#include "GB_add.h"
#include "GB_subref.h"
#include "GB_transpose.h"
#include "GB_sort.h"
#include "GB_apply.h"
#include "GB_mex_generic.h"
#undef OK
#include "GB_mx_usercomplex.h"
#include "mex.h"
#include "matrix.h"

#define SIMPLE_RAND_MAX 32767
uint64_t simple_rand (void) ;
void simple_rand_seed (uint64_t seed) ;
uint64_t simple_rand_getseed (void) ;
double simple_rand_x (void) ;
uint64_t simple_rand_i (void) ;

#define PARGIN(k) ((nargin > (k)) ? pargin [k] : NULL)

#define GET_SCALAR(arg,type,n,n_default)    \
    n = n_default ;                    \
    if (nargin > arg) n = (type) mxGetScalar (pargin [arg]) ;

// MATCH(s,t) compares two strings and returns true if equal
#define MATCH(s,t) (strcmp(s,t) == 0)

void GB_mx_abort (void) ;               // assertion failure

bool GB_mx_mxArray_to_BinaryOp          // true if successful, false otherwise
(
    GrB_BinaryOp *op_handle,            // the binary op
    const mxArray *op_builtin,          // built-in version of op
    const char *name,                   // name of the argument
    const GrB_Type default_optype,      // default operator type
    const bool user_complex             // if true, use user-defined Complex
) ;

bool GB_mx_mxArray_to_UnaryOp           // true if successful
(
    GrB_UnaryOp *op_handle,             // the unary op
    const mxArray *op_builtin,          // built-in version of op
    const char *name,                   // name of the argument
    const GrB_Type default_optype,      // default operator type
    const bool user_complex             // if true, use user-defined Complex
) ;

bool GB_mx_mxArray_to_SelectOp          // true if successful
(
    GxB_SelectOp *handle,               // returns GraphBLAS version of op
    const mxArray *op_builtin,          // built-in version of op
    const char *name                    // name of the argument
) ;

bool GB_mx_string_to_BinaryOp       // true if successful, false otherwise
(
    GrB_BinaryOp *op_handle,        // the binary op
    const GrB_Type default_optype,  // default operator type
    const mxArray *opname_mx,       // built-in string with operator name
    const mxArray *optype_mx,       // built-in string with operator type
    const bool user_complex         // if true, use user-defined Complex op
) ;

bool GB_mx_string_to_UnaryOp            // true if successful, false otherwise
(
    GrB_UnaryOp *op_handle,             // the unary op
    const GrB_Type default_optype,      // default operator type
    const mxArray *opname_mx,           // built-in string with operator name
    const mxArray *optype_mx,           // built-in string with operator type
    const bool user_complex             // true if X is complex
) ;

mxArray *GB_mx_Vector_to_mxArray    // returns the built-in mxArray
(
    GrB_Vector *handle,             // handle of GraphBLAS matrix to convert
    const char *name,               // name for error reporting
    const bool create_struct        // if true, then return a struct
) ;

mxArray *GB_mx_Matrix_to_mxArray    // returns the built-in mxArray
(
    GrB_Matrix *handle,             // handle of GraphBLAS matrix to convert
    const char *name,
    const bool create_struct        // if true, then return a struct
) ;

mxArray *GB_mx_object_to_mxArray    // returns the built-in mxArray
(
    GrB_Matrix *handle,             // handle of GraphBLAS matrix to convert
    const char *name,
    const bool create_struct        // if true, then return a struct
) ;

GrB_Matrix GB_mx_mxArray_to_Matrix     // returns GraphBLAS version of A
(
    const mxArray *A_builtin,           // built-in version of A
    const char *name,                   // name of the argument
    bool deep_copy,                     // if true, return a deep copy
    const bool empty    // if false, 0-by-0 matrices are returned as NULL.
                        // if true, a 0-by-0 matrix is returned.
) ;

GrB_Vector GB_mx_mxArray_to_Vector     // returns GraphBLAS version of V
(
    const mxArray *V_builtin,           // built-in version of V
    const char *name,                   // name of the argument
    const bool deep_copy,               // if true, return a deep copy
    const bool empty    // if false, 0-by-0 matrices are returned as NULL.
                        // if true, a 0-by-0 matrix is returned.
) ;

GrB_Type GB_mx_Type                    // returns a GraphBLAS type
(
    const mxArray *X                   // built-in matrix to query
) ;

void GB_mx_mxArray_to_array    // convert mxArray to array
(
    const mxArray *Xbuiltin,    // input built-in array
    // output:
    GB_void **X,                // pointer to numerical values (shallow)
    int64_t *nrows,             // number of rows of X
    int64_t *ncols,             // number of columns of X
    GrB_Type *xtype             // GraphBLAS type of X, NULL if error
) ;

int GB_mx_mxArray_to_string // returns length of string, or -1 if S not a string
(
    char *string,           // size maxlen
    const size_t maxlen,    // length of string
    const mxArray *S        // built-in mxArray containing a string
) ;

bool GB_mx_mxArray_to_Descriptor    // true if successful, false otherwise
(
    GrB_Descriptor *handle,         // descriptor to return
    const mxArray *D_builtin,       // built-in struct
    const char *name                // name of the descriptor
) ;

bool GB_mx_mxArray_to_Semiring         // true if successful
(
    GrB_Semiring *handle,               // the semiring
    const mxArray *semiring_builtin,    // built-in version of semiring
    const char *name,                   // name of the argument
    const GrB_Type default_optype,      // default operator type
    const bool user_complex         // if true, use user-defined Complex op
) ;

GrB_Semiring GB_mx_semiring         // semiring, or NULL if error
(
    const GrB_Monoid add_monoid,    // input monoid
    const GrB_BinaryOp mult         // input multiply operator
) ;

GrB_Monoid GB_mx_BinaryOp_to_Monoid // monoid, or NULL if error
(
    const GrB_BinaryOp add          // monoid operator
) ;

bool GB_mx_mxArray_to_indices       // true if successful, false otherwise
(
    GrB_Index **handle,             // index array returned
    const mxArray *I_builtin,       // built-in mxArray to get
    GrB_Index *ni,                  // length of I, or special
    GrB_Index Icolon [3],           // for all but GB_LIST
    bool *I_is_list                 // true if I is an explicit list
) ;

bool GB_mx_Monoid               // true if successful, false otherwise
(
    GrB_Monoid *handle,         // monoid to construct
    const GrB_BinaryOp add,     // monoid operator
    const bool malloc_debug     // true if malloc debug should be done
) ;

bool GB_mx_get_global       // true if doing malloc_debug
(
    bool cover              // true if doing statement coverage
) ;

void GB_mx_put_global
(   
    bool cover
) ;

bool GB_mx_same     // true if arrays X and Y are the same
(
    char *X,
    char *Y,
    int64_t len     // length of X and Y
) ;

bool GB_mx_xsame    // true if arrays X and Y are the same (ignoring zombies)
(
    char *X,    bool X_iso,
    char *Y,    bool Y_iso,
    int8_t *Xb,     // bitmap of X and Y (NULL if no bitmap)
    int64_t len,    // length of X and Y
    size_t s,       // size of each entry of X and Y
    int64_t *I      // row indices (for zombies), same length as X and Y
) ;

bool GB_mx_xsame32  // true if arrays X and Y are the same (ignoring zombies)
(
    float *X,   bool X_iso,
    float *Y,   bool Y_iso,
    int8_t *Xb,     // bitmap of X and Y (NULL if no bitmap)
    int64_t len,    // length of X and Y
    int64_t *I,     // row indices (for zombies), same length as X and Y
    float eps       // error tolerance allowed (eps > 0)
) ;

bool GB_mx_xsame64  // true if arrays X and Y are the same (ignoring zombies)
(
    double *X,  bool X_iso,
    double *Y,  bool Y_iso,
    int8_t *Xb,     // bitmap of X and Y (NULL if no bitmap)
    int64_t len,    // length of X and Y
    int64_t *I,     // row indices (for zombies), same length as X and Y
    double eps      // error tolerance allowed (eps > 0)
) ;

bool GB_mx_isequal  // true if A and B are exactly the same
(
    GrB_Matrix A,
    GrB_Matrix B,
    double eps      // if A and B are both FP32 or FP64, and if eps > 0,
                    // then the values are considered equal if their relative
                    // difference is less than or equal to eps.
) ;

GrB_Matrix GB_mx_alias      // output matrix (NULL if no match found)
(
    char *arg_name,         // name of the output matrix
    const mxArray *arg,     // string to select the alias
    char *arg1_name,        // name of first possible alias
    GrB_Matrix arg1,        // first possible alias
    char *arg2_name,        // name of 2nd possible alias
    GrB_Matrix arg2         // second possible alias
) ;

mxArray *GB_mx_create_full      // return new built-in full matrix
(
    const GrB_Index nrows,
    const GrB_Index ncols,
    GrB_Type type               // type of the matrix to create
) ;

mxArray *GB_mx_Type_to_mxstring        // returns a built-in string
(
    const GrB_Type type
) ;

GrB_Type GB_mx_string_to_Type       // GrB_Type from the string
(
    const mxArray *type_mx,         // string with type name
    const GrB_Type default_type     // default type if string empty
) ;

GrB_Info GB_mx_random_matrix      // create a random double-precision matrix
(
    GrB_Matrix *A_output,   // handle of matrix to create
    bool make_symmetric,    // if true, return A as symmetric
    bool no_self_edges,     // if true, then do not create self edges
    int64_t nrows,          // number of rows
    int64_t ncols,          // number of columns
    int64_t nedges,         // number of edges
    int method,             // method to use: 0:setElement, 1:build,
    bool A_complex          // if true, create a Complex matrix
) ;

//------------------------------------------------------------------------------

// remove a block that had been allocated from within GraphBLAS and then
// exported.
#define REMOVE(p)                                       \
{                                                       \
    if ((p) != NULL)                                    \
    {                                                   \
        GB_Global_nmalloc_decrement ( ) ;               \
        if (GB_Global_memtable_find (p))                \
        {                                               \
            GB_Global_memtable_remove (p) ;             \
        }                                               \
    }                                                   \
}

#define GB_AS_IF_FREE(p)                \
{                                       \
    GB_Global_nmalloc_decrement ( ) ;   \
    GB_Global_memtable_remove (p) ;     \
    (p) = NULL ;                        \
}

#ifdef GB_PRINT_MALLOC

#define METHOD_START(OP) \
    printf ("\n================================================================================\n") ; \
    printf ("method: [%s] start: "GBd" "GBd"\n", #OP, \
        GB_Global_nmalloc_get ( ), GB_Global_free_pool_nblocks_total ( )) ; \
    printf ("================================================================================\n") ;

#define METHOD_TRY \
    printf ("\n--------------------------------------------------------------------- try %d\n", tries) ;

#define METHOD_FINAL(OP) \
    printf ("\n================================================================================\n") ; \
    printf ("method: [%s] # tries before success: %d\n", #OP, tries) ;  \
    printf ("================================================================================\n") ;

#else

#define METHOD_START(OP) ;
#define METHOD_TRY ;
#define METHOD_FINAL(OP) ;

#endif

// test a GraphBLAS operation with malloc debuging
#define METHOD(GRAPHBLAS_OPERATION)                                         \
    METHOD_START (GRAPHBLAS_OPERATION) ;                                    \
    if (!malloc_debug)                                                      \
    {                                                                       \
        /* no malloc debugging; just call the method */                     \
        GrB_Info info = GRAPHBLAS_OPERATION ;                               \
        if (info == GrB_PANIC) mexErrMsgTxt ("panic!") ;                    \
        if (! (info == GrB_SUCCESS || info == GrB_NO_VALUE))                \
        {                                                                   \
            FREE_ALL ;                                                      \
            printf ("info: %d\n", info) ;                                   \
            mexErrMsgTxt ("method failed") ;                                \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* brutal malloc debug */                                           \
        int nmalloc_start = (int) GB_Global_nmalloc_get ( ) ;               \
        int nfree_pool_start = (int) GB_Global_free_pool_nblocks_total ( ) ;\
        for (int tries = 0 ; ; tries++)                                     \
        {                                                                   \
            /* give GraphBLAS the ability to do a # of mallocs, */          \
            /* callocs, and reallocs of larger size, equal to tries */      \
            GB_Global_malloc_debug_count_set (tries) ;                      \
            METHOD_TRY ;                                                    \
            /* call the method with malloc debug enabled */                 \
            GB_Global_malloc_debug_set (true) ;                             \
            GrB_Info info = GRAPHBLAS_OPERATION ;                           \
            /* do not finish the work */                                    \
            GB_Global_malloc_debug_set (false) ;                            \
            if (tries > 1000000) mexErrMsgTxt ("infinite loop!") ;          \
            if (info == GrB_SUCCESS || info == GrB_NO_VALUE)                \
            {                                                               \
                /* finally gave GraphBLAS enough malloc's to do the work */ \
                METHOD_FINAL (GRAPHBLAS_OPERATION) ;                        \
                break ;                                                     \
            }                                                               \
            else if (info == GrB_OUT_OF_MEMORY)                             \
            {                                                               \
                /* out of memory; check for leaks */                        \
                /* output matrix may have changed; recopy for next test */  \
                /* but turn off malloc debugging to get the copy */         \
                FREE_DEEP_COPY ;                                            \
                GET_DEEP_COPY ;                                             \
                int nmalloc_end = (int) GB_Global_nmalloc_get ( ) ;         \
                int nfree_pool_end =                                        \
                    (int) GB_Global_free_pool_nblocks_total ( ) ;           \
                int nleak = nmalloc_end - nmalloc_start ;                   \
                int nfree_delta = nfree_pool_end - nfree_pool_start ;       \
                if (nleak > nfree_delta)                                    \
                {                                                           \
                    /* memory leak */                                       \
                    printf ("Leak! tries %d : nleak %d\n"                   \
                        "nmalloc_end:        %d\n"                          \
                        "nmalloc_start:      %d\n"                          \
                        "nfree_pool start:   %d\n"                          \
                        "nfree_pool end:     %d\n"                          \
                        "method [%s]\n",                                    \
                        tries, nleak, nmalloc_end, nmalloc_start,           \
                        nfree_pool_start, nfree_pool_end,                   \
                        GB_STR (GRAPHBLAS_OPERATION)) ;                     \
                    mexWarnMsgIdAndTxt ("GB:leak", "memory leak") ;         \
                    FREE_ALL ;                                              \
                    mexErrMsgTxt ("Leak!") ;                                \
                }                                                           \
            }                                                               \
            else                                                            \
            {                                                               \
                /* another error has occurred */                            \
                FREE_ALL ;                                                  \
                printf ("info: %d\n", info) ; \
                mexErrMsgTxt ("unexpected error in mex brutal malloc debug") ; \
            }                                                               \
        }                                                                   \
    }

//------------------------------------------------------------------------------
// statement coverage
//------------------------------------------------------------------------------

// GB_cover_get copies GraphBLAS_grbcov from the built-in global workspace into
// the internal GB_cov array.  The built-in array is created if it doesn't
// exist.  Thus, to clear the counts simply clear GraphBLAS_grbcov from the
// built-in global workpace.
void GB_cover_get (void) ;

// GB_cover_put copies the internal GB_cov array back into the GraphBLAS_grbcov
// array, for analysis and for subsequent statement counting.  This way,
// multiple tests in built-in can be accumulated into a single array of
// counters.
void GB_cover_put (void) ;

#endif

