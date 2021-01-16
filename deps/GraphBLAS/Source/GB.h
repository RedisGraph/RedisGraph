//------------------------------------------------------------------------------
// GB.h: definitions visible only inside GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_H
#define GB_H

//------------------------------------------------------------------------------
// defintions that modify GraphBLAS.h
//------------------------------------------------------------------------------

#include "GB_warnings.h"
#ifndef MATLAB_MEX_FILE
#define GB_LIBRARY
#endif

//------------------------------------------------------------------------------
// user-visible GraphBLAS.h
//------------------------------------------------------------------------------

#include "GraphBLAS.h"

//------------------------------------------------------------------------------
// internal #include files
//------------------------------------------------------------------------------

#include "GB_dev.h"
#include "GB_defaults.h"
#include "GB_compiler.h"
#include "GB_coverage.h"
#include "GB_index.h"
#include "GB_cplusplus.h"
#include "GB_Global.h"
#include "GB_printf.h"
#include "GB_assert.h"
#include "GB_opaque.h"
#include "GB_casting.h"
#include "GB_math.h"
#include "GB_bitwise.h"
#include "GB_binary_search.h"
#include "GB_check.h"
#include "GB_nnz.h"
#include "GB_zombie.h"
#include "GB_partition.h"
#include "GB_omp.h"
// #include "GB_mkl.h"

//------------------------------------------------------------------------------
// internal definitions
//------------------------------------------------------------------------------

int64_t GB_Pending_n        // return # of pending tuples in A
(
    GrB_Matrix A
) ;

// A is nrows-by-ncols, in either CSR or CSC format
#define GB_NROWS(A) ((A)->is_csc ? (A)->vlen : (A)->vdim)
#define GB_NCOLS(A) ((A)->is_csc ? (A)->vdim : (A)->vlen)

// The internal content of a GrB_Matrix and GrB_Vector are identical, and
// inside SuiteSparse:GraphBLAS, they can be typecasted between each other.
// This typecasting feature should not be done in user code, however, since it
// is not supported in the API.  All GrB_Vector objects can be safely
// typecasted into a GrB_Matrix, but not the other way around.  The GrB_Vector
// object is more restrictive.  The GB_VECTOR_OK(v) macro defines the content
// that all GrB_Vector objects must have.

// GB_VECTOR_OK(v) is used mainly for assertions, but also to determine when it
// is safe to typecast an n-by-1 GrB_Matrix (in standard CSC format) into a
// GrB_Vector.  This is not done in the main SuiteSparse:GraphBLAS library, but
// in the GraphBLAS/Test directory only.  The macro is also used in
// GB_Vector_check, to ensure the content of a GrB_Vector is valid.

#define GB_VECTOR_OK(v)                     \
(                                           \
    ((v) != NULL) &&                        \
    ((v)->is_csc == true) &&                \
    ((v)->plen == 1 || (v)->plen == -1) &&  \
    ((v)->vdim == 1) &&                     \
    ((v)->nvec == 1) &&                     \
    ((v)->h == NULL)                        \
)

// A GxB_Vector is a GrB_Vector of length 1
#define GB_SCALAR_OK(v) (GB_VECTOR_OK(v) && ((v)->vlen == 1))

//------------------------------------------------------------------------------
// aliased and shallow objects
//------------------------------------------------------------------------------

// GraphBLAS allows all inputs to all user-accessible objects to be aliased, as
// in GrB_mxm (C, C, accum, C, C, ...), which is valid.  Internal routines are
// more restrictive.

// GB_aliased also checks the content of A and B
GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
bool GB_aliased             // determine if A and B are aliased
(
    GrB_Matrix A,           // input A matrix
    GrB_Matrix B            // input B matrix
) ;

// matrices returned to the user are never shallow; internal matrices may be
GB_PUBLIC                       // used by the MATLAB interface
bool GB_is_shallow              // true if any component of A is shallow
(
    GrB_Matrix A                // matrix to query
) ;

//------------------------------------------------------------------------------
// internal GraphBLAS type
//------------------------------------------------------------------------------

// predefined type objects
GB_PUBLIC struct GB_Type_opaque
    GB_opaque_GrB_BOOL   ,  // GrB_BOOL is a pointer to this object, etc.
    GB_opaque_GrB_INT8   ,
    GB_opaque_GrB_UINT8  ,
    GB_opaque_GrB_INT16  ,
    GB_opaque_GrB_UINT16 ,
    GB_opaque_GrB_INT32  ,
    GB_opaque_GrB_UINT32 ,
    GB_opaque_GrB_INT64  ,
    GB_opaque_GrB_UINT64 ,
    GB_opaque_GrB_FP32   ,
    GB_opaque_GrB_FP64   ,
    GB_opaque_GxB_FC32   ,
    GB_opaque_GxB_FC64   ;

//------------------------------------------------------------------------------
// monoid structs
//------------------------------------------------------------------------------

GB_PUBLIC struct GB_Monoid_opaque

    // MIN monoids:
    GB_opaque_GxB_MIN_INT8_MONOID,          // identity: INT8_MAX
    GB_opaque_GxB_MIN_UINT8_MONOID,         // identity: UINT8_MAX
    GB_opaque_GxB_MIN_INT16_MONOID,         // identity: INT16_MAX
    GB_opaque_GxB_MIN_UINT16_MONOID,        // identity: UINT16_MAX
    GB_opaque_GxB_MIN_INT32_MONOID,         // identity: INT32_MAX
    GB_opaque_GxB_MIN_UINT32_MONOID,        // identity: UINT32_MAX
    GB_opaque_GxB_MIN_INT64_MONOID,         // identity: INT64_MAX
    GB_opaque_GxB_MIN_UINT64_MONOID,        // identity: UINT64_MAX
    GB_opaque_GxB_MIN_FP32_MONOID,          // identity: INFINITY
    GB_opaque_GxB_MIN_FP64_MONOID,          // identity: INFINITY

    // MAX monoids:
    GB_opaque_GxB_MAX_INT8_MONOID,          // identity: INT8_MIN
    GB_opaque_GxB_MAX_UINT8_MONOID,         // identity: 0
    GB_opaque_GxB_MAX_INT16_MONOID,         // identity: INT16_MIN
    GB_opaque_GxB_MAX_UINT16_MONOID,        // identity: 0
    GB_opaque_GxB_MAX_INT32_MONOID,         // identity: INT32_MIN
    GB_opaque_GxB_MAX_UINT32_MONOID,        // identity: 0
    GB_opaque_GxB_MAX_INT64_MONOID,         // identity: INT64_MIN
    GB_opaque_GxB_MAX_UINT64_MONOID,        // identity: 0
    GB_opaque_GxB_MAX_FP32_MONOID,          // identity: -INFINITY
    GB_opaque_GxB_MAX_FP64_MONOID,          // identity: -INFINITY

    // PLUS monoids:
    GB_opaque_GxB_PLUS_INT8_MONOID,         // identity: 0
    GB_opaque_GxB_PLUS_UINT8_MONOID,        // identity: 0
    GB_opaque_GxB_PLUS_INT16_MONOID,        // identity: 0
    GB_opaque_GxB_PLUS_UINT16_MONOID,       // identity: 0
    GB_opaque_GxB_PLUS_INT32_MONOID,        // identity: 0
    GB_opaque_GxB_PLUS_UINT32_MONOID,       // identity: 0
    GB_opaque_GxB_PLUS_INT64_MONOID,        // identity: 0
    GB_opaque_GxB_PLUS_UINT64_MONOID,       // identity: 0
    GB_opaque_GxB_PLUS_FP32_MONOID,         // identity: 0
    GB_opaque_GxB_PLUS_FP64_MONOID,         // identity: 0
    GB_opaque_GxB_PLUS_FC32_MONOID,         // identity: 0
    GB_opaque_GxB_PLUS_FC64_MONOID,         // identity: 0

    // TIMES monoids:
    GB_opaque_GxB_TIMES_INT8_MONOID,        // identity: 1
    GB_opaque_GxB_TIMES_UINT8_MONOID,       // identity: 1
    GB_opaque_GxB_TIMES_INT16_MONOID,       // identity: 1
    GB_opaque_GxB_TIMES_UINT16_MONOID,      // identity: 1
    GB_opaque_GxB_TIMES_INT32_MONOID,       // identity: 1
    GB_opaque_GxB_TIMES_UINT32_MONOID,      // identity: 1
    GB_opaque_GxB_TIMES_INT64_MONOID,       // identity: 1
    GB_opaque_GxB_TIMES_UINT64_MONOID,      // identity: 1
    GB_opaque_GxB_TIMES_FP32_MONOID,        // identity: 1
    GB_opaque_GxB_TIMES_FP64_MONOID,        // identity: 1
    GB_opaque_GxB_TIMES_FC32_MONOID,        // identity: 1
    GB_opaque_GxB_TIMES_FC64_MONOID,        // identity: 1

    // ANY monoids:
    GB_opaque_GxB_ANY_INT8_MONOID,
    GB_opaque_GxB_ANY_UINT8_MONOID,
    GB_opaque_GxB_ANY_INT16_MONOID,
    GB_opaque_GxB_ANY_UINT16_MONOID,
    GB_opaque_GxB_ANY_INT32_MONOID,
    GB_opaque_GxB_ANY_UINT32_MONOID,
    GB_opaque_GxB_ANY_INT64_MONOID,
    GB_opaque_GxB_ANY_UINT64_MONOID,
    GB_opaque_GxB_ANY_FP32_MONOID,
    GB_opaque_GxB_ANY_FP64_MONOID,
    GB_opaque_GxB_ANY_FC32_MONOID,
    GB_opaque_GxB_ANY_FC64_MONOID,

    // Boolean monoids:
    GB_opaque_GxB_ANY_BOOL_MONOID,
    GB_opaque_GxB_LOR_BOOL_MONOID,          // identity: false
    GB_opaque_GxB_LAND_BOOL_MONOID,         // identity: true
    GB_opaque_GxB_LXOR_BOOL_MONOID,         // identity: false
    GB_opaque_GxB_EQ_BOOL_MONOID,           // identity: true

    // BOR monoids: (bitwise OR)
    GB_opaque_GxB_BOR_UINT8_MONOID,
    GB_opaque_GxB_BOR_UINT16_MONOID,
    GB_opaque_GxB_BOR_UINT32_MONOID,
    GB_opaque_GxB_BOR_UINT64_MONOID,

    // BAND monoids: (bitwise and)
    GB_opaque_GxB_BAND_UINT8_MONOID,
    GB_opaque_GxB_BAND_UINT16_MONOID,
    GB_opaque_GxB_BAND_UINT32_MONOID,
    GB_opaque_GxB_BAND_UINT64_MONOID,

    // BXOR monoids: (bitwise xor)
    GB_opaque_GxB_BXOR_UINT8_MONOID,
    GB_opaque_GxB_BXOR_UINT16_MONOID,
    GB_opaque_GxB_BXOR_UINT32_MONOID,
    GB_opaque_GxB_BXOR_UINT64_MONOID,

    // BXNOR monoids: (bitwise xnor)
    GB_opaque_GxB_BXNOR_UINT8_MONOID,
    GB_opaque_GxB_BXNOR_UINT16_MONOID,
    GB_opaque_GxB_BXNOR_UINT32_MONOID,
    GB_opaque_GxB_BXNOR_UINT64_MONOID ;

//------------------------------------------------------------------------------
// select structs
//------------------------------------------------------------------------------

GB_PUBLIC struct GB_SelectOp_opaque
    GB_opaque_GxB_TRIL,
    GB_opaque_GxB_TRIU,
    GB_opaque_GxB_DIAG,
    GB_opaque_GxB_OFFDIAG,
    GB_opaque_GxB_NONZERO,
    GB_opaque_GxB_EQ_ZERO,
    GB_opaque_GxB_GT_ZERO,
    GB_opaque_GxB_GE_ZERO,
    GB_opaque_GxB_LT_ZERO,
    GB_opaque_GxB_LE_ZERO,
    GB_opaque_GxB_NE_THUNK,
    GB_opaque_GxB_EQ_THUNK,
    GB_opaque_GxB_GT_THUNK,
    GB_opaque_GxB_GE_THUNK,
    GB_opaque_GxB_LT_THUNK,
    GB_opaque_GxB_LE_THUNK ;

//------------------------------------------------------------------------------
// error logging and parallel thread control
//------------------------------------------------------------------------------

// Error messages are logged in Context->logger, on the stack which is handle
// to the input/output matrix/vector (typically C).  If the user-defined data
// types, operators, etc have really long names, the error messages are safely
// truncated (via snprintf).  This is intentional, but gcc with
// -Wformat-truncation will print a warning (see pragmas above).  Ignore the
// warning.

// The Context also contains the number of threads to use in the operation.  It
// is normally determined from the user's descriptor, with a default of
// nthreads_max = GxB_DEFAULT (that is, zero).  The default rule is to let
// GraphBLAS determine the number of threads automatically by selecting a
// number of threads between 1 and nthreads_max.  GrB_init initializes
// nthreads_max to omp_get_max_threads.  Both the global value and the value in
// a descriptor can set/queried by GxB_set / GxB_get.

// Some GrB_Matrix and GrB_Vector methods do not take a descriptor, however
// (GrB_*_dup, _build, _exportTuples, _clear, _nvals, _wait, and GxB_*_resize).
// For those methods the default rule is always used (nthreads_max =
// GxB_DEFAULT), which then relies on the global nthreads_max.

#define GB_RLEN 384
#define GB_DLEN 256

typedef struct
{
    double chunk ;              // chunk size for small problems
    int nthreads_max ;          // max # of threads to use
    const char *where ;         // GraphBLAS function where error occurred
    char **logger ;             // error report
    // #include "GB_Context_struct_mkl_template.h"
}
GB_Context_struct ;

typedef GB_Context_struct *GB_Context ;

// GB_WHERE keeps track of the currently running user-callable function.
// User-callable functions in this implementation are written so that they do
// not call other unrelated user-callable functions (except for GrB_*free).
// Related user-callable functions can call each other since they all report
// the same type-generic name.  Internal functions can be called by many
// different user-callable functions, directly or indirectly.  It would not be
// helpful to report the name of an internal function that flagged an error
// condition.  Thus, each time a user-callable function is entered (except
// GrB_*free), it logs the name of the function with the GB_WHERE macro.
// GrB_*free does not encounter error conditions so it doesn't need to be
// logged by the GB_WHERE macro.

#define GB_CONTEXT(where_string)                                    \
    /* construct the Context */                                     \
    GB_Context_struct Context_struct ;                              \
    GB_Context Context = &Context_struct ;                          \
    /* set Context->where so GrB_error can report it if needed */   \
    Context->where = where_string ;                                 \
    /* get the default max # of threads and default chunk size */   \
    Context->nthreads_max = GB_Global_nthreads_max_get ( ) ;        \
    Context->chunk = GB_Global_chunk_get ( ) ;                      \
    /* get the pointer to where any error will be logged */         \
    Context->logger = NULL ;

// #include "GB_CONTEXT_mkl_template.h"

#define GB_WHERE(C,where_string)                                    \
    if (!GB_Global_GrB_init_called_get ( ))                         \
    {                                                               \
        return (GrB_PANIC) ; /* GrB_init not called */              \
    }                                                               \
    GB_CONTEXT (where_string)                                       \
    if (C != NULL)                                                  \
    {                                                               \
        /* free any prior error logged in the object */             \
        GB_FREE (C->logger) ;                                       \
        Context->logger = &(C->logger) ;                            \
    }

#define GB_WHERE1(where_string)                                     \
    if (!GB_Global_GrB_init_called_get ( ))                         \
    {                                                               \
        return (GrB_PANIC) ; /* GrB_init not called */              \
    }                                                               \
    GB_CONTEXT (where_string)

//------------------------------------------------------------------------------
// GB_GET_NTHREADS_MAX:  determine max # of threads for OpenMP parallelism.
//------------------------------------------------------------------------------

//      GB_GET_NTHREADS_MAX obtains the max # of threads to use and the chunk
//      size from the Context.  If Context is NULL then a single thread *must*
//      be used.  If Context->nthreads_max is <= GxB_DEFAULT, then select
//      automatically: between 1 and nthreads_max, depending on the problem
//      size.  Below is the default rule.  Any function can use its own rule
//      instead, based on Context, chunk, nthreads_max, and the problem size.
//      No rule can exceed nthreads_max.

#define GB_GET_NTHREADS_MAX(nthreads_max,chunk,Context)                     \
    int nthreads_max = (Context == NULL) ? 1 : Context->nthreads_max ;      \
    if (nthreads_max <= GxB_DEFAULT)                                        \
    {                                                                       \
        nthreads_max = GB_Global_nthreads_max_get ( ) ;                     \
    }                                                                       \
    double chunk = (Context == NULL) ? GxB_DEFAULT : Context->chunk ;       \
    if (chunk <= GxB_DEFAULT)                                               \
    {                                                                       \
        chunk = GB_Global_chunk_get ( ) ;                                   \
    }

//------------------------------------------------------------------------------
// GB_nthreads: determine # of threads to use for a parallel loop or region
//------------------------------------------------------------------------------

// If work < 2*chunk, then only one thread is used.
// else if work < 3*chunk, then two threads are used, and so on.

static inline int GB_nthreads   // return # of threads to use
(
    double work,                // total work to do
    double chunk,               // give each thread at least this much work
    int nthreads_max            // max # of threads to use
)
{
    work  = GB_IMAX (work, 1) ;
    chunk = GB_IMAX (chunk, 1) ;
    int64_t nthreads = (int64_t) floor (work / chunk) ;
    nthreads = GB_IMIN (nthreads, nthreads_max) ;
    nthreads = GB_IMAX (nthreads, 1) ;
    return ((int) nthreads) ;
}

//------------------------------------------------------------------------------
// error logging
//------------------------------------------------------------------------------

// The GB_ERROR macro logs an error in the logger error string.
//
//  if (i >= nrows)
//  {
//      GB_ERROR (GrB_INDEX_OUT_OF_BOUNDS,
//          "Row index %d out of bounds; must be < %d", i, nrows) ;
//  }
//
// The user can then do:
//
//  const char *error ;
//  GrB_error (&error, A) ;
//  printf ("%s", error) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
const char *GB_status_code (GrB_Info info) ;

// log an error in the error logger string and return the error
#define GB_ERROR(info,format,...)                                           \
{                                                                           \
    if (Context != NULL)                                                    \
    {                                                                       \
        char **logger = Context->logger ;                                   \
        if (logger != NULL)                                                 \
        {                                                                   \
            (*logger) = GB_MALLOC (GB_RLEN+1, char) ;                       \
            if ((*logger) != NULL)                                          \
            {                                                               \
                snprintf ((*logger), GB_RLEN,                               \
                    "GraphBLAS error: %s\nfunction: %s\n" format,           \
                    GB_status_code (info), Context->where, __VA_ARGS__) ;   \
            }                                                               \
        }                                                                   \
    }                                                                       \
    return (info) ;                                                         \
}

//------------------------------------------------------------------------------
// internal GraphBLAS functions
//------------------------------------------------------------------------------

GrB_Info GB_init            // start up GraphBLAS
(
    const GrB_Mode mode,    // blocking or non-blocking mode

    // pointers to memory management functions.  Must be non-NULL.
    void * (* malloc_function  ) (size_t),
    void * (* calloc_function  ) (size_t, size_t),
    void * (* realloc_function ) (void *, size_t),
    void   (* free_function    ) (void *),
    bool malloc_is_thread_safe,

    bool caller_is_GxB_cuda_init,       // true for GxB_cuda_init only

    GB_Context Context      // from GrB_init or GxB_init
) ;

typedef enum                    // input parameter to GB_new and GB_new_bix
{
    GB_Ap_calloc,               // 0: calloc A->p, malloc A->h if hypersparse
    GB_Ap_malloc,               // 1: malloc A->p, malloc A->h if hypersparse
    GB_Ap_null                  // 2: do not allocate A->p or A->h
}
GB_Ap_code ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_new                 // create matrix, except for indices & values
(
    GrB_Matrix *Ahandle,        // handle of matrix to create
    const GrB_Type type,        // matrix type
    const int64_t vlen,         // length of each vector
    const int64_t vdim,         // number of vectors
    const GB_Ap_code Ap_option, // allocate A->p and A->h, or leave NULL
    const bool is_csc,          // true if CSC, false if CSR
    const int sparsity,         // hyper, sparse, bitmap, full, or
                                // auto (hyper + sparse)
    const float hyper_switch,   // A->hyper_switch, ignored if auto
    const int64_t plen,         // size of A->p and A->h, if A hypersparse.
                                // Ignored if A is not hypersparse.
    GB_Context Context
) ;

GrB_Info GB_new_bix             // create a new matrix, incl. A->b, A->i, A->x
(
    GrB_Matrix *Ahandle,        // output matrix to create
    const GrB_Type type,        // type of output matrix
    const int64_t vlen,         // length of each vector
    const int64_t vdim,         // number of vectors
    const GB_Ap_code Ap_option, // allocate A->p and A->h, or leave NULL
    const bool is_csc,          // true if CSC, false if CSR
    const int sparsity,         // hyper, sparse, bitmap, full, or auto
    const bool bitmap_calloc,   // if true, calloc A->b, otherwise use malloc
    const float hyper_switch,   // A->hyper_switch, unless auto
    const int64_t plen,         // size of A->p and A->h, if hypersparse
    const int64_t anz,          // number of nonzeros the matrix must hold
    const bool numeric,         // if true, allocate A->x, else A->x is NULL
    GB_Context Context
) ;

GrB_Info GB_hyper_realloc
(
    GrB_Matrix A,               // matrix with hyperlist to reallocate
    int64_t plen_new,           // new size of A->p and A->h
    GB_Context Context
) ;

GrB_Info GB_clear           // clear a matrix, type and dimensions unchanged
(
    GrB_Matrix A,           // matrix to clear
    GB_Context Context
) ;

GrB_Info GB_dup             // make an exact copy of a matrix
(
    GrB_Matrix *Chandle,    // handle of output matrix to create
    const GrB_Matrix A,     // input matrix to copy
    const bool numeric,     // if true, duplicate the numeric values
    const GrB_Type ctype,   // type of C, if numeric is false
    GB_Context Context
) ;

GrB_Info GB_dup2            // make an exact copy of a matrix
(
    GrB_Matrix *Chandle,    // handle of output matrix to create
    const GrB_Matrix A,     // input matrix to copy
    const bool numeric,     // if true, duplicate the numeric values
    const GrB_Type ctype,   // type of C, if numeric is false
    GB_Context Context
) ;

void GB_memcpy                  // parallel memcpy
(
    void *dest,                 // destination
    const void *src,            // source
    size_t n,                   // # of bytes to copy
    int nthreads                // # of threads to use
) ;

void GB_memset                  // parallel memset
(
    void *dest,                 // destination
    const int c,                // value to to set
    size_t n,                   // # of bytes to set
    int nthreads                // # of threads to use
) ;

GrB_Info GB_nvals           // get the number of entries in a matrix
(
    GrB_Index *nvals,       // matrix has nvals entries
    const GrB_Matrix A,     // matrix to query
    GB_Context Context
) ;

GrB_Info GB_matvec_type            // get the type of a matrix
(
    GrB_Type *type,         // returns the type of the matrix
    const GrB_Matrix A,     // matrix to query
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_bix_alloc       // allocate A->b, A->i, and A->x space in a matrix
(
    GrB_Matrix A,           // matrix to allocate space for
    const GrB_Index nzmax,  // number of entries the matrix can hold
    const bool is_bitmap,   // if true, allocate A->b, otherwise A->b is NULL
    const bool bitmap_calloc,   // if true, calloc A->b, otherwise use malloc
    const bool is_sparse,   // if true, allocate A->i, otherwise A->i is NULL
    const bool numeric,     // if true, allocate A->x, otherwise A->x is NULL
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_ix_realloc      // reallocate space in a matrix
(
    GrB_Matrix A,               // matrix to allocate space for
    const int64_t nzmax_new,    // new number of entries the matrix can hold
    const bool numeric,         // if true, reallocate A->x, else A->x is NULL
    GB_Context Context
) ;

GrB_Info GB_ix_resize       // resize a matrix
(
    GrB_Matrix A,           // matrix to resize (sparse/hyper, not full/bitmap)
    const int64_t anz_new,  // required new nnz(A)
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_bix_free                // free A->b, A->i, and A->x of a matrix
(
    GrB_Matrix A                // matrix with content to free
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_ph_free                 // free A->p and A->h of a matrix
(
    GrB_Matrix A                // matrix with content to free
) ;

void GB_phbix_free              // free all content of a matrix
(
    GrB_Matrix A                // matrix with content to free
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
bool GB_Type_compatible             // check if two types can be typecast
(
    const GrB_Type atype,
    const GrB_Type btype
) ;

//------------------------------------------------------------------------------
// GB_code_compatible: return true if domains are compatible
//------------------------------------------------------------------------------

// Two domains are compatible for typecasting between them if both are built-in
// types (of any kind) or if both are the same user-defined type.  This
// function does not have the type itself, but just the code.  If the types are
// available, GB_Type_compatible should be called instead.

static inline bool GB_code_compatible       // true if two types can be typecast
(
    const GB_Type_code acode,   // type code of a
    const GB_Type_code bcode    // type code of b
)
{

    bool a_user = (acode == GB_UDT_code) ;
    bool b_user = (bcode == GB_UDT_code) ;

    if (a_user || b_user)
    { 
        // both a and b must be user-defined.  They should be the same
        // user-defined type, but the caller does not have the actual type,
        // just the code.
        return (a_user && b_user) ;
    }
    else
    { 
        // any built-in domain is compatible with any other built-in domain
        return (true) ;
    }
}


//------------------------------------------------------------------------------
// GB_task_struct: parallel task descriptor
//------------------------------------------------------------------------------

// The element-wise computations (GB_add, GB_emult, and GB_mask) compute
// C(:,j)<M(:,j)> = op (A (:,j), B(:,j)).  They are parallelized by slicing the
// work into tasks, described by the GB_task_struct.

// There are two kinds of tasks.  For a coarse task, kfirst <= klast, and the
// task computes all vectors in C(:,kfirst:klast), inclusive.  None of the
// vectors are sliced and computed by other tasks.  For a fine task, klast is
// -1.  The task computes part of the single vector C(:,kfirst).  It starts at
// pA in Ai,Ax, at pB in Bi,Bx, and (if M is present) at pM in Mi,Mx.  It
// computes C(:,kfirst), starting at pC in Ci,Cx.

// GB_subref also uses the TaskList.  It has 12 kinds of fine tasks,
// corresponding to each of the 12 methods used in GB_subref_template.  For
// those fine tasks, method = -TaskList [taskid].klast defines the method to
// use.

// The GB_subassign functions use the TaskList, in many different ways.

typedef struct          // task descriptor
{
    int64_t kfirst ;    // C(:,kfirst) is the first vector in this task.
    int64_t klast  ;    // C(:,klast) is the last vector in this task.
    int64_t pC ;        // fine task starts at Ci, Cx [pC]
    int64_t pC_end ;    // fine task ends at Ci, Cx [pC_end-1]
    int64_t pM ;        // fine task starts at Mi, Mx [pM]
    int64_t pM_end ;    // fine task ends at Mi, Mx [pM_end-1]
    int64_t pA ;        // fine task starts at Ai, Ax [pA]
    int64_t pA_end ;    // fine task ends at Ai, Ax [pA_end-1]
    int64_t pB ;        // fine task starts at Bi, Bx [pB]
    int64_t pB_end ;    // fine task ends at Bi, Bx [pB_end-1]
    int64_t len ;       // fine task handles a subvector of this length
}
GB_task_struct ;

// GB_REALLOC_TASK_LIST: Allocate or reallocate the TaskList so that it can
// hold at least ntasks.  Double the size if it's too small.

#define GB_REALLOC_TASK_LIST(TaskList,ntasks,max_ntasks)                    \
{                                                                           \
    if ((ntasks) >= max_ntasks)                                             \
    {                                                                       \
        bool ok ;                                                           \
        int nold = (max_ntasks == 0) ? 0 : (max_ntasks + 1) ;               \
        int nnew = 2 * (ntasks) + 1 ;                                       \
        GB_REALLOC (TaskList, nnew, nold, GB_task_struct, &ok) ;            \
        if (!ok)                                                            \
        {                                                                   \
            /* out of memory */                                             \
            GB_FREE_ALL ;                                                   \
            return (GrB_OUT_OF_MEMORY) ;                                    \
        }                                                                   \
        for (int t = nold ; t < nnew ; t++)                                 \
        {                                                                   \
            TaskList [t].kfirst = -1 ;                                      \
            TaskList [t].klast  = INT64_MIN ;                               \
            TaskList [t].pA     = INT64_MIN ;                               \
            TaskList [t].pA_end = INT64_MIN ;                               \
            TaskList [t].pB     = INT64_MIN ;                               \
            TaskList [t].pB_end = INT64_MIN ;                               \
            TaskList [t].pC     = INT64_MIN ;                               \
            TaskList [t].pC_end = INT64_MIN ;                               \
            TaskList [t].pM     = INT64_MIN ;                               \
            TaskList [t].pM_end = INT64_MIN ;                               \
            TaskList [t].len    = INT64_MIN ;                               \
        }                                                                   \
        max_ntasks = 2 * (ntasks) ;                                         \
    }                                                                       \
    ASSERT ((ntasks) < max_ntasks) ;                                        \
}

GrB_Info GB_ewise_slice
(
    // output:
    GB_task_struct **p_TaskList,    // array of structs, of size max_ntasks
    int *p_TaskList_size,           // size of TaskList
    int *p_ntasks,                  // # of tasks constructed
    int *p_nthreads,                // # of threads for eWise operation
    // input:
    const int64_t Cnvec,            // # of vectors of C
    const int64_t *GB_RESTRICT Ch,     // vectors of C, if hypersparse
    const int64_t *GB_RESTRICT C_to_M, // mapping of C to M
    const int64_t *GB_RESTRICT C_to_A, // mapping of C to A
    const int64_t *GB_RESTRICT C_to_B, // mapping of C to B
    bool Ch_is_Mh,                  // if true, then Ch == Mh; GB_add only
    const GrB_Matrix M,             // mask matrix to slice (optional)
    const GrB_Matrix A,             // matrix to slice
    const GrB_Matrix B,             // matrix to slice
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_slice_vector
(
    // output: return i, pA, and pB
    int64_t *p_i,                   // work starts at A(i,kA) and B(i,kB)
    int64_t *p_pM,                  // M(i:end,kM) starts at pM
    int64_t *p_pA,                  // A(i:end,kA) starts at pA
    int64_t *p_pB,                  // B(i:end,kB) starts at pB
    // input:
    const int64_t pM_start,         // M(:,kM) starts at pM_start in Mi,Mx
    const int64_t pM_end,           // M(:,kM) ends at pM_end-1 in Mi,Mx
    const int64_t *GB_RESTRICT Mi,     // indices of M (or NULL)
    const int64_t pA_start,         // A(:,kA) starts at pA_start in Ai,Ax
    const int64_t pA_end,           // A(:,kA) ends at pA_end-1 in Ai,Ax
    const int64_t *GB_RESTRICT Ai,     // indices of A
    const int64_t pB_start,         // B(:,kB) starts at pB_start in Bi,Bx
    const int64_t pB_end,           // B(:,kB) ends at pB_end-1 in Bi,Bx
    const int64_t *GB_RESTRICT Bi,     // indices of B
    const int64_t vlen,             // A->vlen and B->vlen
    const double target_work        // target work
) ;

void GB_task_cumsum
(
    int64_t *Cp,                        // size Cnvec+1
    const int64_t Cnvec,
    int64_t *Cnvec_nonempty,            // # of non-empty vectors in C
    GB_task_struct *GB_RESTRICT TaskList,  // array of structs
    const int ntasks,                   // # of tasks
    const int nthreads                  // # of threads
) ;

//------------------------------------------------------------------------------
// GB_GET_VECTOR: get the content of a vector for a coarse/fine task
//------------------------------------------------------------------------------

#define GB_GET_VECTOR(pX_start, pX_fini, pX, pX_end, Xp, kX, Xvlen)         \
    int64_t pX_start, pX_fini ;                                             \
    if (fine_task)                                                          \
    {                                                                       \
        /* A fine task operates on a slice of X(:,k) */                     \
        pX_start = TaskList [taskid].pX ;                                   \
        pX_fini  = TaskList [taskid].pX_end ;                               \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* vectors are never sliced for a coarse task */                    \
        pX_start = GBP (Xp, kX, Xvlen) ;                                    \
        pX_fini  = GBP (Xp, kX+1, Xvlen) ;                                  \
    }

//------------------------------------------------------------------------------

GrB_Info GB_transplant          // transplant one matrix into another
(
    GrB_Matrix C,               // output matrix to overwrite with A
    const GrB_Type ctype,       // new type of C
    GrB_Matrix *Ahandle,        // input matrix to copy from and free
    GB_Context Context
) ;

GrB_Info GB_transplant_conform      // transplant and conform hypersparsity
(
    GrB_Matrix C,                   // destination matrix to transplant into
    GrB_Type ctype,                 // type to cast into
    GrB_Matrix *Thandle,            // source matrix
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
size_t GB_code_size             // return the size of a type, given its code
(
    const GB_Type_code code,    // input code of the type to find the size of
    const size_t usize          // known size of user-defined type
) ;

//------------------------------------------------------------------------------
// memory management
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void *GB_calloc_memory      // pointer to allocated block of memory
(
    size_t nitems,          // number of items to allocate
    size_t size_of_item     // sizeof each item
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void *GB_malloc_memory      // pointer to allocated block of memory
(
    size_t nitems,          // number of items to allocate
    size_t size_of_item     // sizeof each item
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void *GB_realloc_memory     // pointer to reallocated block of memory, or
                            // to original block if the realloc failed.
(
    size_t nitems_new,      // new number of items in the object
    size_t nitems_old,      // old number of items in the object
    size_t size_of_item,    // sizeof each item
    void *p,                // old object to reallocate
    bool *ok                // true if successful, false otherwise
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_free_memory
(
    void *p                 // pointer to allocated block of memory to free
) ;

#define GB_FREE(p)                                          \
{                                                           \
    GB_free_memory ((void *) p) ;                           \
    (p) = NULL ;                                            \
}

#define GB_CALLOC(n,type) (type *) GB_calloc_memory (n, sizeof (type))
#define GB_MALLOC(n,type) (type *) GB_malloc_memory (n, sizeof (type))
#define GB_REALLOC(p,nnew,nold,type,ok) \
    p = (type *) GB_realloc_memory (nnew, nold, sizeof (type), (void *) p, ok)

void GB_Matrix_free             // free a matrix
(
    GrB_Matrix *matrix_handle   // handle of matrix to free
) ;

//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Type GB_code_type           // return the GrB_Type corresponding to the code
(
    const GB_Type_code code,    // type code to convert
    const GrB_Type type         // user type if code is GB_UDT_code
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
bool GB_pslice          // slice Ap; return true if ok, false if out of memory
(
    int64_t *GB_RESTRICT *Slice_handle,    // size ntasks+1
    const int64_t *GB_RESTRICT Ap,         // array of size n+1
    const int64_t n,
    const int ntasks,                       // # of tasks
    const bool perfectly_balanced
) ;

void GB_eslice
(
    // output:
    int64_t *Slice,         // array of size ntasks+1
    // input:
    int64_t e,              // number items to partition amongst the tasks
    const int ntasks        // # of tasks
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_cumsum                      // cumulative sum of an array
(
    int64_t *GB_RESTRICT count,     // size n+1, input/output
    const int64_t n,
    int64_t *GB_RESTRICT kresult,   // return k, if needed by the caller
    int nthreads
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_Descriptor_get      // get the contents of a descriptor
(
    const GrB_Descriptor desc,  // descriptor to query, may be NULL
    bool *C_replace,            // if true replace C before C<M>=Z
    bool *Mask_comp,            // if true use logical negation of M
    bool *Mask_struct,          // if true use the structure of M
    bool *In0_transpose,        // if true transpose first input
    bool *In1_transpose,        // if true transpose second input
    GrB_Desc_Value *AxB_method, // method for C=A*B
    int *do_sort,               // if nonzero, sort in GrB_mxm
    GB_Context Context
) ;

GrB_Info GB_compatible          // SUCCESS if all is OK, *_MISMATCH otherwise
(
    const GrB_Type ctype,       // the type of C (matrix or scalar)
    const GrB_Matrix C,         // the output matrix C; NULL if C is a scalar
    const GrB_Matrix M,         // optional mask, NULL if no mask
    const GrB_BinaryOp accum,   // C<M> = accum(C,T) is computed
    const GrB_Type ttype,       // type of T
    GB_Context Context
) ;

GrB_Info GB_Mask_compatible     // check type and dimensions of mask
(
    const GrB_Matrix M,         // mask to check
    const GrB_Matrix C,         // C<M>= ...
    const GrB_Index nrows,      // size of output if C is NULL (see GB*assign)
    const GrB_Index ncols,
    GB_Context Context
) ;

GrB_Info GB_BinaryOp_compatible     // check for domain mismatch
(
    const GrB_BinaryOp op,          // binary operator to check
    const GrB_Type ctype,           // C must be compatible with op->ztype
    const GrB_Type atype,           // A must be compatible with op->xtype
    const GrB_Type btype,           // B must be compatible with op->ytype
    const GB_Type_code bcode,       // B may not have a type, just a code
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB interface only
bool GB_Index_multiply      // true if ok, false if overflow
(
    GrB_Index *GB_RESTRICT c,  // c = a*b, or zero if overflow occurs
    const int64_t a,
    const int64_t b
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
bool GB_size_t_multiply     // true if ok, false if overflow
(
    size_t *c,              // c = a*b, or zero if overflow occurs
    const size_t a,
    const size_t b
) ;

bool GB_extract_vector_list     // true if successful, false if out of memory
(
    // output:
    int64_t *GB_RESTRICT J,        // size nnz(A) or more
    // input:
    const GrB_Matrix A,
    int nthreads
) ;

GrB_Info GB_extractTuples       // extract all tuples from a matrix
(
    GrB_Index *I_out,           // array for returning row indices of tuples
    GrB_Index *J_out,           // array for returning col indices of tuples
    void *X,                    // array for returning values of tuples
    GrB_Index *p_nvals,         // I,J,X size on input; # tuples on output
    const GB_Type_code xcode,   // type of array X
    const GrB_Matrix A,         // matrix to extract tuples from
    GB_Context Context
) ;

GrB_Info GB_Monoid_new          // create a monoid
(
    GrB_Monoid *monoid,         // handle of monoid to create
    GrB_BinaryOp op,            // binary operator of the monoid
    const void *identity,       // identity value
    const void *terminal,       // terminal value, if any (may be NULL)
    const GB_Type_code idcode,  // identity and terminal type code
    GB_Context Context
) ;

GrB_Info GB_Semiring_new            // create a semiring
(
    GrB_Semiring *semiring,         // handle of semiring to create
    GrB_Monoid add,                 // additive monoid of the semiring
    GrB_BinaryOp multiply           // multiply operator of the semiring
) ;

//------------------------------------------------------------------------------

GrB_Info GB_setElement              // set a single entry, C(row,col) = scalar
(
    GrB_Matrix C,                   // matrix to modify
    void *scalar,                   // scalar to set
    const GrB_Index row,            // row index
    const GrB_Index col,            // column index
    const GB_Type_code scalar_code, // type of the scalar
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_block   // apply all pending computations if blocking mode enabled
(
    GrB_Matrix A,
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
bool GB_op_is_second    // return true if op is SECOND, of the right type
(
    GrB_BinaryOp op,
    GrB_Type type
) ;


//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
char *GB_code_string            // return a static string for a type name
(
    const GB_Type_code code     // code to convert to string
) ;

GrB_Info GB_resize              // change the size of a matrix
(
    GrB_Matrix A,               // matrix to modify
    const GrB_Index nrows_new,  // new number of rows in matrix
    const GrB_Index ncols_new,  // new number of columns in matrix
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
int64_t GB_nvec_nonempty        // return # of non-empty vectors
(
    const GrB_Matrix A,         // input matrix to examine
    GB_Context Context
) ;

GrB_Info GB_conform_hyper       // conform a matrix to sparse/hypersparse
(
    GrB_Matrix A,               // matrix to conform
    GB_Context Context
) ;

GrB_Info GB_hyper_prune
(
    // output, not allocated on input:
    int64_t *GB_RESTRICT *p_Ap,     // size nvec+1
    int64_t *GB_RESTRICT *p_Ah,     // size nvec
    int64_t *p_nvec,                // # of vectors, all nonempty
    // input, not modified
    const int64_t *Ap_old,          // size nvec_old+1
    const int64_t *Ah_old,          // size nvec_old
    const int64_t nvec_old,         // original number of vectors
    GB_Context Context
) ;

GrB_Info GB_hypermatrix_prune
(
    GrB_Matrix A,               // matrix to prune
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
void GB_cast_array              // typecast an array
(
    GB_void *Cx,                // output array
    const GB_Type_code code1,   // type code for Cx
    GB_void *Ax,                // input array
    const GB_Type_code code2,   // type code for Ax
    const int8_t *GB_RESTRICT Ab,   // bitmap for Ax
    const size_t user_size,     // size of Ax and Cx if user-defined
    const int64_t anz,          // number of entries in Cx and Ax
    const int nthreads          // number of threads to use
) ;

//------------------------------------------------------------------------------
// boiler plate macros for checking inputs and returning if an error occurs
//------------------------------------------------------------------------------

// Functions use these macros to check/get their inputs and return an error
// if something has gone wrong.

#define GB_OK(method)                       \
{                                           \
    info = method ;                         \
    if (info != GrB_SUCCESS)                \
    {                                       \
        GB_FREE_ALL ;                       \
        return (info) ;                     \
    }                                       \
}

// check if a required arg is NULL
#define GB_RETURN_IF_NULL(arg)                                          \
    if ((arg) == NULL)                                                  \
    {                                                                   \
        /* the required arg is NULL */                                  \
        return (GrB_NULL_POINTER) ;                                     \
    }

// arg may be NULL, but if non-NULL then it must be initialized
#define GB_RETURN_IF_FAULTY(arg)                                        \
    if ((arg) != NULL && (arg)->magic != GB_MAGIC)                      \
    {                                                                   \
        if ((arg)->magic == GB_MAGIC2)                                  \
        {                                                               \
            /* optional arg is not NULL, but invalid */                 \
            return (GrB_INVALID_OBJECT) ;                               \
        }                                                               \
        else                                                            \
        {                                                               \
            /* optional arg is not NULL, but not initialized */         \
            return (GrB_UNINITIALIZED_OBJECT) ;                         \
        }                                                               \
    }

// arg must not be NULL, and it must be initialized
#define GB_RETURN_IF_NULL_OR_FAULTY(arg)                                \
    GB_RETURN_IF_NULL (arg) ;                                           \
    GB_RETURN_IF_FAULTY (arg) ;

// positional ops not supported for use as accum operators
#define GB_RETURN_IF_FAULTY_OR_POSITIONAL(accum)                        \
{                                                                       \
    GB_RETURN_IF_FAULTY (accum) ;                                       \
    if (GB_OP_IS_POSITIONAL (accum))                                    \
    {                                                                   \
        GB_ERROR (GrB_DOMAIN_MISMATCH,                                  \
            "Positional op z=%s(x,y) not supported as accum\n",         \
                accum->name) ;                                          \
    }                                                                   \
}

// check the descriptor and extract its contents; also copies
// nthreads_max and chunk from the descriptor to the Context
#define GB_GET_DESCRIPTOR(info,desc,dout,dmc,dms,d0,d1,dalgo,dsort)          \
    GrB_Info info ;                                                          \
    bool dout, dmc, dms, d0, d1 ;                                            \
    int dsort ;                                                              \
    GrB_Desc_Value dalgo ;                                                   \
    /* if desc is NULL then defaults are used.  This is OK */                \
    info = GB_Descriptor_get (desc, &dout, &dmc, &dms, &d0, &d1, &dalgo,     \
        &dsort, Context) ;                                                   \
    if (info != GrB_SUCCESS)                                                 \
    {                                                                        \
        /* desc not NULL, but uninitialized or an invalid object */          \
        return (info) ;                                                      \
    }

// C<M>=Z ignores Z if an empty mask is complemented, so return from
// the method without computing anything.  But do apply the mask.
#define GB_RETURN_IF_QUICK_MASK(C, C_replace, M, Mask_comp)                 \
    if (Mask_comp && M == NULL)                                             \
    {                                                                       \
        /* C<!NULL>=NULL since result does not depend on computing Z */     \
        return (C_replace ? GB_clear (C, Context) : GrB_SUCCESS) ;          \
    }

// GB_MASK_VERY_SPARSE is true if C<M>=A+B or C<M>=accum(C,T) is being
// computed, and the mask M is very sparse compared with A and B.
#define GB_MASK_VERY_SPARSE(M,A,B) (8 * GB_NNZ (M) < GB_NNZ (A) + GB_NNZ (B))

//------------------------------------------------------------------------------
// Pending upddate and zombies
//------------------------------------------------------------------------------

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_Matrix_wait         // finish all pending computations
(
    GrB_Matrix A,               // matrix with pending computations
    GB_Context Context
) ;

GrB_Info GB_unjumble        // unjumble a matrix
(
    GrB_Matrix A,           // matrix to unjumble
    GB_Context Context
) ;

// true if a matrix has pending tuples
#define GB_PENDING(A) ((A) != NULL && (A)->Pending != NULL)

// true if a matrix is allowed to have pending tuples
#define GB_PENDING_OK(A) (GB_PENDING (A) || !GB_PENDING (A))

// true if a matrix has zombies
#define GB_ZOMBIES(A) ((A) != NULL && (A)->nzombies > 0)

// true if a matrix is allowed to have zombies
#define GB_ZOMBIES_OK(A) (((A) == NULL) || ((A) != NULL && (A)->nzombies >= 0))

// true if a matrix has pending tuples or zombies
#define GB_PENDING_OR_ZOMBIES(A) (GB_PENDING (A) || GB_ZOMBIES (A))

// true if a matrix is jumbled
#define GB_JUMBLED(A) ((A) != NULL && (A)->jumbled)

// true if a matrix is allowed to be jumbled
#define GB_JUMBLED_OK(A) (GB_JUMBLED (A) || !GB_JUMBLED (A))

// true if a matrix has pending tuples, zombies, or is jumbled
#define GB_ANY_PENDING_WORK(A) \
    (GB_PENDING (A) || GB_ZOMBIES (A) || GB_JUMBLED (A))

// wait if condition holds
#define GB_WAIT_IF(condition,A)                                         \
{                                                                       \
    if (condition)                                                      \
    {                                                                   \
        GrB_Info info ;                                                 \
        GB_OK (GB_Matrix_wait ((GrB_Matrix) A, Context)) ;              \
    }                                                                   \
}

// do all pending work:  zombies, pending tuples, and unjumble
#define GB_MATRIX_WAIT(A) GB_WAIT_IF (GB_ANY_PENDING_WORK (A), A)

// do all pending work if pending tuples; zombies and jumbled are OK
#define GB_MATRIX_WAIT_IF_PENDING(A) GB_WAIT_IF (GB_PENDING (A), A)

// delete zombies and assemble any pending tuples; jumbled is O
#define GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES(A)                         \
    GB_WAIT_IF (GB_PENDING_OR_ZOMBIES (A), A)

// ensure A is not jumbled
#define GB_MATRIX_WAIT_IF_JUMBLED(A) GB_WAIT_IF (GB_JUMBLED (A), A)

// true if a matrix has no entries; zombies OK
#define GB_IS_EMPTY(A) ((GB_NNZ (A) == 0) && !GB_PENDING (A))

//------------------------------------------------------------------------------

#include "GB_convert.h"

//------------------------------------------------------------------------------
// built-in unary and binary operators
//------------------------------------------------------------------------------

#define GB_TYPE             bool
#define GB_REAL
#define GB_BOOLEAN
#define GB(x)               GB_ ## x ## _BOOL
#define GB_BITS             1
#include "GB_ops_template.h"

#define GB_TYPE             int8_t
#define GB_REAL
#define GB_SIGNED_INT
#define GB(x)               GB_ ## x ## _INT8
#define GB_BITS             8
#include "GB_ops_template.h"

#define GB_TYPE             uint8_t
#define GB_REAL
#define GB_UNSIGNED_INT
#define GB(x)               GB_ ## x ## _UINT8
#define GB_BITS             8
#include "GB_ops_template.h"

#define GB_TYPE             int16_t
#define GB_REAL
#define GB_SIGNED_INT
#define GB(x)               GB_ ## x ## _INT16
#define GB_BITS             16
#include "GB_ops_template.h"

#define GB_TYPE             uint16_t
#define GB_REAL
#define GB_UNSIGNED_INT
#define GB(x)               GB_ ## x ## _UINT16
#define GB_BITS             16
#include "GB_ops_template.h"

#define GB_TYPE             int32_t
#define GB_REAL
#define GB_SIGNED_INT
#define GB(x)               GB_ ## x ## _INT32
#define GB_BITS             32
#include "GB_ops_template.h"

#define GB_TYPE             uint32_t
#define GB_REAL
#define GB_UNSIGNED_INT
#define GB(x)               GB_ ## x ## _UINT32
#define GB_BITS             32
#include "GB_ops_template.h"

#define GB_TYPE             int64_t
#define GB_REAL
#define GB_SIGNED_INT
#define GB(x)               GB_ ## x ## _INT64
#define GB_BITS             64
#include "GB_ops_template.h"

#define GB_TYPE             uint64_t
#define GB_REAL
#define GB_UNSIGNED_INT
#define GB(x)               GB_ ## x ## _UINT64
#define GB_BITS             64
#include "GB_ops_template.h"

#define GB_TYPE             float
#define GB_REAL
#define GB_FLOATING_POINT
#define GB_FLOAT
#define GB(x)               GB_ ## x ## _FP32
#define GB_BITS             32
#include "GB_ops_template.h"

#define GB_TYPE             double
#define GB_REAL
#define GB_FLOATING_POINT
#define GB_DOUBLE
#define GB(x)               GB_ ## x ## _FP64
#define GB_BITS             64
#include "GB_ops_template.h"

#define GB_TYPE             GxB_FC32_t
#define GB_COMPLEX
#define GB_FLOATING_POINT
#define GB_FLOAT_COMPLEX
#define GB(x)               GB_ ## x ## _FC32
#define GB_BITS             64
#include "GB_ops_template.h"

#define GB_TYPE             GxB_FC64_t
#define GB_COMPLEX
#define GB_FLOATING_POINT
#define GB_DOUBLE_COMPLEX
#define GB(x)               GB_ ## x ## _FC64
#define GB_BITS             128
#include "GB_ops_template.h"

#define GB_opaque_GrB_LNOT  GB_opaque_GxB_LNOT_BOOL
#define GB_opaque_GrB_LOR   GB_opaque_GxB_LOR_BOOL
#define GB_opaque_GrB_LAND  GB_opaque_GxB_LAND_BOOL
#define GB_opaque_GrB_LXOR  GB_opaque_GxB_LXOR_BOOL
#define GB_opaque_GrB_LXNOR GB_opaque_GxB_LXNOR_BOOL

//------------------------------------------------------------------------------
// CUDA (DRAFT: in progress)
//------------------------------------------------------------------------------

#include "GB_cuda_gateway.h"

#endif

