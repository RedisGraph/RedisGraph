//------------------------------------------------------------------------------
// GB_convert.h: converting between sparsity structures
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#ifndef GB_CONVERT_H
#define GB_CONVERT_H

// these parameters define the hyper_switch needed to ensure matrix stays
// either always hypersparse, or never hypersparse.
#define GB_ALWAYS_HYPER (1.0)
#define GB_NEVER_HYPER  (-1.0)

// true if A is bitmap
#define GB_IS_BITMAP(A) ((A) != NULL && ((A)->b != NULL))

// true if A is full (but not bitmap)
#define GB_IS_FULL(A) \
    ((A) != NULL && (A)->h == NULL && (A)->p == NULL && (A)->i == NULL \
        && (A)->b == NULL)

// true if A is hypersparse
#define GB_IS_HYPERSPARSE(A) ((A) != NULL && ((A)->h != NULL))

// true if A is sparse (but not hypersparse)
#define GB_IS_SPARSE(A) ((A) != NULL && ((A)->h == NULL) && (A)->p != NULL)

// determine the sparsity control for a matrix
int GB_sparsity_control     // revised sparsity
(
    int sparsity,           // sparsity control
    int64_t vdim            // A->vdim, or -1 to ignore this condition
) ;

// GB_sparsity: determine the current sparsity status of a matrix
static inline int GB_sparsity (GrB_Matrix A)
{
    if (A == NULL)
    {
        // if A is NULL, pretend it is sparse
        return (GxB_SPARSE) ;
    }
    else if (GB_IS_HYPERSPARSE (A))
    { 
        return (GxB_HYPERSPARSE) ;
    }
    else if (GB_IS_FULL (A))
    { 
        return (GxB_FULL) ;
    }
    else if (GB_IS_BITMAP (A))
    { 
        return (GxB_BITMAP) ;
    }
    else
    { 
        return (GxB_SPARSE) ;
    }
}

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_convert_hyper_to_sparse // convert hypersparse to sparse
(
    GrB_Matrix A,           // matrix to convert from hypersparse to sparse
    GB_Context Context
) ;

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_convert_sparse_to_hyper // convert from sparse to hypersparse
(
    GrB_Matrix A,           // matrix to convert to hypersparse
    GB_Context Context
) ;

bool GB_convert_hyper_to_sparse_test    // test for hypersparse to sparse
(
    float hyper_switch,     // A->hyper_switch
    int64_t k,              // # of non-empty vectors of A (an estimate is OK)
    int64_t vdim            // A->vdim
) ;

bool GB_convert_sparse_to_hyper_test  // test sparse to hypersparse conversion
(
    float hyper_switch,     // A->hyper_switch
    int64_t k,              // # of non-empty vectors of A (an estimate is OK)
    int64_t vdim            // A->vdim
) ;

bool GB_convert_bitmap_to_sparse_test    // test for hyper/sparse to bitmap
(
    float bitmap_switch,    // A->bitmap_switch
    int64_t anz,            // # of entries in A = GB_NNZ (A)
    int64_t vlen,           // A->vlen
    int64_t vdim            // A->vdim
) ;

bool GB_convert_sparse_to_bitmap_test    // test for hyper/sparse to bitmap
(
    float bitmap_switch,    // A->bitmap_switch
    int64_t anz,            // # of entries in A = GB_NNZ (A)
    int64_t vlen,           // A->vlen
    int64_t vdim            // A->vdim
) ;

GrB_Info GB_convert_full_to_sparse      // convert matrix from full to sparse
(
    GrB_Matrix A,               // matrix to convert from full to sparse
    GB_Context Context
) ;

GrB_Info GB_convert_full_to_bitmap      // convert matrix from full to bitmap
(
    GrB_Matrix A,               // matrix to convert from full to bitmap
    GB_Context Context
) ;

GrB_Info GB_convert_sparse_to_bitmap    // convert sparse/hypersparse to bitmap
(
    GrB_Matrix A,               // matrix to convert from sparse to bitmap
    GB_Context Context
) ;

GrB_Info GB_convert_bitmap_to_sparse    // convert matrix from bitmap to sparse
(
    GrB_Matrix A,               // matrix to convert from bitmap to sparse
    GB_Context Context
) ;

GrB_Info GB_convert_bitmap_worker   // extract CSC/CSR or triplets from bitmap
(
    // outputs:
    int64_t *GB_RESTRICT Ap,        // vector pointers for CSC/CSR form
    int64_t *GB_RESTRICT Ai,        // indices for CSC/CSR or triplet form
    int64_t *GB_RESTRICT Aj,        // vector indices for triplet form
    GB_void *GB_RESTRICT Ax_new,    // values for CSC/CSR or triplet form
    int64_t *anvec_nonempty,        // # of non-empty vectors
    // inputs: not modified
    const GrB_Matrix A,             // matrix to extract; not modified
    GB_Context Context
) ;

GrB_Info GB_convert_to_full     // convert matrix to full; delete prior values
(
    GrB_Matrix A                // matrix to convert to full
) ;

GrB_Info GB_convert_any_to_bitmap   // convert to bitmap
(
    GrB_Matrix A,           // matrix to convert to bitmap
    GB_Context Context
) ;

GB_PUBLIC                       // used by MATLAB interface
void GB_convert_any_to_full     // convert any matrix to full
(
    GrB_Matrix A                // matrix to convert to full
) ;

GrB_Info GB_convert_any_to_hyper // convert to hypersparse
(
    GrB_Matrix A,           // matrix to convert to hypersparse
    GB_Context Context
) ;

GrB_Info GB_convert_any_to_sparse // convert to sparse
(
    GrB_Matrix A,           // matrix to convert to sparse
    GB_Context Context
) ;

GrB_Info GB_convert_to_nonfull      // ensure a matrix is not full
(
    GrB_Matrix A,
    GB_Context Context
) ;

/* ensure C is sparse or hypersparse */
#define GB_ENSURE_SPARSE(C)                                 \
{                                                           \
    if (GB_IS_BITMAP (C))                                   \
    {                                                       \
        /* convert C from bitmap to sparse */               \
        GB_OK (GB_convert_bitmap_to_sparse (C, Context)) ;  \
    }                                                       \
    else if (GB_IS_FULL (C))                                \
    {                                                       \
        /* convert C from full to sparse */                 \
        GB_OK (GB_convert_full_to_sparse (C, Context)) ;    \
    }                                                       \
}

#define GB_ENSURE_FULL(C)                                       \
{                                                               \
    ASSERT (GB_is_dense (C)) ;                                  \
    if (GB_sparsity_control (C->sparsity, C->vdim) & GxB_FULL)  \
    {                                                           \
        /* convert C from any structure to full, */             \
        /* if permitted by C->sparsity */                       \
        GB_convert_any_to_full (C) ;                            \
    }                                                           \
}

//------------------------------------------------------------------------------
// GB_is_dense
//------------------------------------------------------------------------------

static inline bool GB_is_dense
(
    const GrB_Matrix A
)
{
    // check if A is competely dense:  all entries present.
    // zombies, pending tuples, and jumbled status are not considered.
    // A can have any sparsity structure: hyper, sparse, bitmap, or full.
    // It can be converted to full, if zombies/tuples/jumbled are discarded.
    if (A == NULL)
    {
        return (false) ;
    }
    if (GB_IS_FULL (A))
    { 
        // A is full; the pattern is not present
        return (true) ;
    }
    // A is sparse, hyper, or bitmap: check if all entries present
    GrB_Index anzmax ;
    bool ok = GB_Index_multiply (&anzmax, A->vlen, A->vdim) ;
    return (ok && (anzmax == GB_NNZ (A))) ;
}

//------------------------------------------------------------------------------
// GB_as_if_full
//------------------------------------------------------------------------------

static inline bool GB_as_if_full
(
    const GrB_Matrix A
)
{
    // check if A is competely dense:  all entries present.
    // zombies, pending tuples, and jumbled status are checked.
    // A can have any sparsity structure: hyper, sparse, bitmap, or full.
    // It can be converted to full.
    if (A == NULL)
    {
        return (false) ;
    }
    if (GB_IS_FULL (A))
    { 
        // A is full; the pattern is not present
        return (true) ;
    }
    if (GB_ANY_PENDING_WORK (A))
    {
        // A has pending work and so cannot be treated as if full.
        return (false) ;
    }
    // A is sparse, hyper, or bitmap: check if all entries present
    GrB_Index anzmax ;
    bool ok = GB_Index_multiply (&anzmax, A->vlen, A->vdim) ;
    return (ok && (anzmax == GB_NNZ (A))) ;
}

//------------------------------------------------------------------------------
// GB_is_packed
//------------------------------------------------------------------------------

static inline bool GB_is_packed
(
    const GrB_Matrix A
)
{
    // check if A is a packed matrix.  A is packed if it is bitmap or full.  If
    // A is hypersparse or sparse, it is packed if it is not jumbled, all
    // entries are present, and it has no zombies or pending tuples. 
    // If A is sparse or hypersparse, it can be converted to full via
    // GB_convert_any_to_full, by deleting A->p, A->h, and A->i.  If bitmap,
    // it cannot be converted to full unless GB_is_dense (A) is also true
    // (it must have all entries present).

    return (GB_IS_BITMAP (A) || GB_as_if_full (A)) ;
}

//------------------------------------------------------------------------------

GrB_Info GB_conform     // conform a matrix to its desired sparsity structure
(
    GrB_Matrix A,       // matrix to conform
    GB_Context Context
) ;

static inline char *GB_sparsity_char (int sparsity)
{
    switch (sparsity)
    {
        case GxB_HYPERSPARSE: return ("H") ;
        case GxB_SPARSE:      return ("S") ;
        case GxB_BITMAP:      return ("B") ;
        case GxB_FULL:        return ("F") ;
        default: ASSERT (0) ; return ("?") ;
    }
}

static inline char *GB_sparsity_char_matrix (GrB_Matrix A)
{
    if (A == NULL)             return (".") ;
    if (GB_IS_HYPERSPARSE (A)) return ("H") ;
    if (GB_IS_SPARSE (A))      return ("S") ;
    if (GB_IS_BITMAP (A))      return ("B") ;
    if (GB_IS_FULL (A))        return ("F") ;
    ASSERT (0) ;               return ("?") ;
}

GrB_Matrix GB_hyper_pack            // return C
(
    GrB_Matrix C,                   // output matrix
    const GrB_Matrix A              // input matrix
) ;

#endif

