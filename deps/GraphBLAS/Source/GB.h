//------------------------------------------------------------------------------
// GB.h: definitions visible only inside GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_H
#define GB_H

//------------------------------------------------------------------------------
// definitions that modify GraphBLAS.h
//------------------------------------------------------------------------------

#include "GB_warnings.h"
#define GB_LIBRARY

//------------------------------------------------------------------------------
// user-visible GraphBLAS.h
//------------------------------------------------------------------------------

#include "GraphBLAS.h"

//------------------------------------------------------------------------------
// internal #include files
//------------------------------------------------------------------------------

#include "GB_prefix.h"
#include "GB_dev.h"
#include "GB_bytes.h"
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
#include "GB_context.h"
#include "GB_memory.h"
#include "GB_werk.h"
#include "GB_log2.h"
#include "GB_iso.h"

#if defined ( GB_HAVE_IPP )
// Intel(R) IPP, not yet supported
#include <ipp.h>
#endif

//------------------------------------------------------------------------------
// more internal definitions
//------------------------------------------------------------------------------

int64_t GB_Pending_n        // return # of pending tuples in A
(
    GrB_Matrix A
) ;

// A is nrows-by-ncols, in either CSR or CSC format
#define GB_NROWS(A) ((A)->is_csc ? (A)->vlen : (A)->vdim)
#define GB_NCOLS(A) ((A)->is_csc ? (A)->vdim : (A)->vlen)

//------------------------------------------------------------------------------
// aliased and shallow objects
//------------------------------------------------------------------------------

// GraphBLAS allows all inputs to all user-accessible objects to be aliased, as
// in GrB_mxm (C, C, accum, C, C, ...), which is valid.  Internal routines are
// more restrictive.

// GB_aliased also checks the content of A and B
GB_PUBLIC
bool GB_aliased             // determine if A and B are aliased
(
    GrB_Matrix A,           // input A matrix
    GrB_Matrix B            // input B matrix
) ;

// matrices returned to the user are never shallow; internal matrices may be
GB_PUBLIC
bool GB_is_shallow              // true if any component of A is shallow
(
    GrB_Matrix A                // matrix to query
) ;

//------------------------------------------------------------------------------
// internal GraphBLAS functions
//------------------------------------------------------------------------------

GrB_Info GB_init            // start up GraphBLAS
(
    const GrB_Mode mode,    // blocking or non-blocking mode

    // pointers to memory management functions
    void * (* malloc_function  ) (size_t),
    void * (* realloc_function ) (void *, size_t),
    void   (* free_function    ) (void *),

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

GB_PUBLIC
GrB_Info GB_new                 // create matrix, except for indices & values
(
    GrB_Matrix *Ahandle,        // handle of matrix to create
    const bool A_static_header, // true if Ahandle is statically allocated
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
    const bool A_static_header, // true if Ahandle is statically allocated.
    const GrB_Type type,        // type of output matrix
    const int64_t vlen,         // length of each vector
    const int64_t vdim,         // number of vectors
    const GB_Ap_code Ap_option, // allocate A->p and A->h, or leave NULL
    const bool is_csc,          // true if CSC, false if CSR
    const int sparsity,         // hyper, sparse, bitmap, full, or auto
    const bool bitmap_calloc,   // if true, calloc A->b, otherwise use malloc
    const float hyper_switch,   // A->hyper_switch, unless auto
    const int64_t plen,         // size of A->p and A->h, if hypersparse
    const int64_t nzmax,        // number of nonzeros the matrix must hold;
                                // ignored if A is iso and full
    const bool numeric,         // if true, allocate A->x, else A->x is NULL
    const bool iso,             // if true, allocate A as iso
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
    GB_Context Context
) ;

GrB_Info GB_dup_worker      // make an exact copy of a matrix
(
    GrB_Matrix *Chandle,    // output matrix, NULL or existing static/dynamic
    const bool C_iso,       // if true, construct C as iso
    const GrB_Matrix A,     // input matrix to copy
    const bool numeric,     // if true, duplicate the numeric values; if A is
                            // iso, only the first entry is copied, regardless
                            // of C_iso on input
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

GrB_Info GB_matvec_type_name  // return the name of the type of a matrix
(
    char *type_name,        // name of the type (char array of size at least
                            // GxB_MAX_NAME_LEN, owned by the user application).
    const GrB_Matrix A,     // matrix to query
    GB_Context Context
) ;

GB_PUBLIC
GrB_Info GB_bix_alloc       // allocate A->b, A->i, and A->x space in a matrix
(
    GrB_Matrix A,           // matrix to allocate space for
    const GrB_Index nzmax,  // number of entries the matrix can hold;
                            // ignored if A is iso and full
    const int sparsity,     // sparse (=hyper/auto) / bitmap / full
    const bool bitmap_calloc,   // if true, calloc A->b, otherwise use malloc
    const bool numeric,     // if true, allocate A->x, otherwise A->x is NULL
    const bool iso,         // if true, allocate A as iso
    GB_Context Context
) ;

GB_PUBLIC
GrB_Info GB_ix_realloc      // reallocate space in a matrix
(
    GrB_Matrix A,               // matrix to allocate space for
    const int64_t nzmax_new,    // new number of entries the matrix can hold
    GB_Context Context
) ;

GB_PUBLIC
void GB_bix_free                // free A->b, A->i, and A->x of a matrix
(
    GrB_Matrix A                // matrix with content to free
) ;

GB_PUBLIC
void GB_ph_free                 // free A->p and A->h of a matrix
(
    GrB_Matrix A                // matrix with content to free
) ;

void GB_phbix_free              // free all content of a matrix
(
    GrB_Matrix A                // matrix with content to free
) ;

GB_PUBLIC
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

#include "GB_task_struct.h"

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

GB_PUBLIC
size_t GB_code_size             // return the size of a type, given its code
(
    const GB_Type_code code,    // input code of the type to find the size of
    const size_t usize          // known size of user-defined type
) ;

void GB_Matrix_free             // free a matrix
(
    GrB_Matrix *Ahandle         // handle of matrix to free
) ;

GrB_Info GB_Op_free             // free a user-created op
(
    GB_Operator *op_handle      // handle of operator to free
) ;

//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Type GB_code_type           // return the GrB_Type corresponding to the code
(
    const GB_Type_code code,    // type code to convert
    const GrB_Type type         // user type if code is GB_UDT_code
) ;

GB_PUBLIC
void GB_pslice                      // slice Ap
(
    int64_t *restrict Slice,     // size ntasks+1
    const int64_t *restrict Ap,  // array size n+1 (NULL if full or bitmap)
    const int64_t n,
    const int ntasks,               // # of tasks
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

GB_PUBLIC
void GB_cumsum                      // cumulative sum of an array
(
    int64_t *restrict count,     // size n+1, input/output
    const int64_t n,
    int64_t *restrict kresult,   // return k, if needed by the caller
    int nthreads,
    GB_Context Context
) ;

GB_PUBLIC
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
    const bool Mask_struct,     // true if M is structural
    const GrB_BinaryOp accum,   // C<M> = accum(C,T) is computed
    const GrB_Type ttype,       // type of T
    GB_Context Context
) ;

GrB_Info GB_Mask_compatible     // check type and dimensions of mask
(
    const GrB_Matrix M,         // mask to check
    const bool Mask_struct,     // true if M is structural
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

GB_PUBLIC
bool GB_int64_multiply      // true if ok, false if overflow
(
    GrB_Index *restrict c,  // c = a*b, or zero if overflow occurs
    const int64_t a,
    const int64_t b
) ;

GB_PUBLIC
bool GB_size_t_multiply     // true if ok, false if overflow
(
    size_t *c,              // c = a*b, or zero if overflow occurs
    const size_t a,
    const size_t b
) ;

GrB_Info GB_extract_vector_list     // extract vector list from a matrix
(
    // output:
    int64_t *restrict J,         // size nnz(A) or more
    // input:
    const GrB_Matrix A,
    GB_Context Context
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
    GrB_Semiring semiring,          // semiring to create
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

GrB_Info GB_Vector_removeElement
(
    GrB_Vector V,               // vector to remove entry from
    GrB_Index i,                // index
    GB_Context Context
) ;

GrB_Info GB_Matrix_removeElement
(
    GrB_Matrix C,               // matrix to remove entry from
    GrB_Index row,              // row index
    GrB_Index col,              // column index
    GB_Context Context
) ;

GB_PUBLIC
GrB_Info GB_block   // apply all pending computations if blocking mode enabled
(
    GrB_Matrix A,
    GB_Context Context
) ;

GB_PUBLIC
bool GB_op_is_second    // return true if op is SECOND, of the right type
(
    GrB_BinaryOp op,
    GrB_Type type
) ;

void GB_op_name_and_defn
(
    // output
    char *operator_name,        // op->name of the GrB operator struct
    char **operator_defn,       // op->defn of the GrB operator struct
    // input
    const char *input_name,     // user-provided name, may be NULL
    const char *input_defn,     // user-provided name, may be NULL
    const char *typecast_name,  // typecast name for function pointer
    size_t typecast_name_len    // length of typecast_name
) ;

//------------------------------------------------------------------------------

GB_PUBLIC
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

GB_PUBLIC
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
    int64_t *restrict *p_Ap, size_t *p_Ap_size,      // size nvec+1
    int64_t *restrict *p_Ah, size_t *p_Ah_size,      // size nvec
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

GrB_UnaryOp GB_unop_one (GB_Type_code xcode) ;

//------------------------------------------------------------------------------

#include "GB_ok.h"
#include "GB_cast.h"
#include "GB_wait.h"
#include "GB_convert.h"
#include "GB_ops.h"

//------------------------------------------------------------------------------
// CUDA (DRAFT: in progress)
//------------------------------------------------------------------------------

#include "GB_cuda_gateway.h"

#endif

