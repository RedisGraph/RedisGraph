//------------------------------------------------------------------------------
// GB_new: create a new GraphBLAS matrix, but do not allocate A->{b,i,x}
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Creates a new matrix but does not allocate space for A->b, A->i, and A->x.
// See GB_new_bix instead.

// If the Ap_option is GB_Ap_calloc, the A->p and A->h are allocated and
// initialized, and A->magic is set to GB_MAGIC to denote a valid matrix.
// Otherwise, the matrix has not yet been completelyinitialized, and A->magic
// is set to GB_MAGIC2 to denote this.  This case only occurs internally in
// GraphBLAS.  The internal function that calls GB_new must then allocate or
// initialize A->p itself, and then set A->magic = GB_MAGIC when it does so.

// To allocate a full or bitmap matrix, the sparsity parameter
// is GxB_FULL or GxB_BITMAP.  The Ap_option is ignored.  For a full or
// bitmap matrix, only the header is allocated, if NULL on input.

// Only GrB_SUCCESS and GrB_OUT_OF_MEMORY can be returned by this function.

// The GrB_Matrix object holds both a sparse vector and a sparse matrix.  A
// vector is represented as an vlen-by-1 matrix, but it is sometimes treated
// differently in various methods.  Vectors are never transposed via a
// descriptor, for example.

// The matrix may be created in an existing header, which case *Ahandle is
// non-NULL on input.  If an out-of-memory condition occurs, (*Ahandle) is
// returned as NULL, and the existing header is freed as well, if non-NULL on
// input.

#include "GB.h"

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_new                 // create matrix, except for indices & values
(
    GrB_Matrix *Ahandle,        // handle of matrix to create
    const GrB_Type type,        // matrix type
    const int64_t vlen,         // length of each vector
    const int64_t vdim,         // number of vectors
    const GB_Ap_code Ap_option, // allocate A->p and A->h, or leave NULL
    const bool is_csc,          // true if CSC, false if CSR
    const int sparsity,         // hyper, sparse, bitmap, full, or auto
    const float hyper_switch,   // A->hyper_switch
    const int64_t plen,         // size of A->p and A->h, if A hypersparse.
                                // Ignored if A is not hypersparse.
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Ahandle != NULL) ;
    ASSERT_TYPE_OK (type, "type for GB_new", GB0) ;
    ASSERT (vlen >= 0 && vlen <= GxB_INDEX_MAX)
    ASSERT (vdim >= 0 && vdim <= GxB_INDEX_MAX) ;

    //--------------------------------------------------------------------------
    // allocate the matrix header, if not already allocated on input
    //--------------------------------------------------------------------------

    bool allocated_header = false ;
    if ((*Ahandle) == NULL)
    {
        (*Ahandle) = GB_CALLOC (1, struct GB_Matrix_opaque) ;
        if (*Ahandle == NULL)
        { 
            // out of memory
            return (GrB_OUT_OF_MEMORY) ;
        }
        allocated_header = true ;
    }

    GrB_Matrix A = *Ahandle ;

    //--------------------------------------------------------------------------
    // initialize the matrix header
    //--------------------------------------------------------------------------

    // basic information
    A->magic = GB_MAGIC2 ;                 // object is not yet valid
    A->type = type ;

    // CSR/CSC format
    A->is_csc = is_csc ;

    // initial sparsity format
    bool A_is_hyper ;
    bool A_is_full_or_bitmap = false ;
    A->hyper_switch = hyper_switch ;
    A->bitmap_switch = GB_Global_bitmap_switch_matrix_get (vlen, vdim) ;
    A->sparsity = GxB_AUTO_SPARSITY ;

    if (sparsity == GxB_HYPERSPARSE)
    { 
        A_is_hyper = true ;             // force A to be hypersparse
    }
    else if (sparsity == GxB_SPARSE)
    { 
        A_is_hyper = false ;            // force A to be sparse
    }
    else if (sparsity == GxB_FULL || sparsity == GxB_BITMAP)
    { 
        A_is_full_or_bitmap = true ;    // force A to be full or bitmap
        A_is_hyper = false ;
    }
    else // auto: sparse or hypersparse
    { 
        // auto selection:  sparse if one vector or less or
        // if the global hyper_switch is negative; hypersparse otherwise.
        // Never select A to be full or bitmap for this case.
        A_is_hyper = !(vdim <= 1 || 0 > hyper_switch) ;
    }

    // matrix dimensions
    A->vlen = vlen ;
    A->vdim = vdim ;

    // content that is freed or reset in GB_ph_free
    if (A_is_full_or_bitmap)
    {  
        // A is full or bitmap
        A->plen = -1 ;
        A->nvec = vdim ;
        // all vectors present, unless matrix has a zero dimension 
        A->nvec_nonempty = (vlen > 0) ? vdim : 0 ;
    }
    else if (A_is_hyper)
    { 
        // A is hypersparse
        A->plen = GB_IMIN (plen, vdim) ;
        A->nvec = 0 ;                   // no vectors present
        A->nvec_nonempty = 0 ;
    }
    else
    { 
        // A is sparse
        A->plen = vdim ;
        A->nvec = vdim ;                // all vectors present
        A->nvec_nonempty = 0 ;
    }

    A->p = NULL ;
    A->h = NULL ;
    A->p_shallow = false ;
    A->h_shallow = false ;
    // #include "GB_new_mkl_template.c"

    A->logger = NULL ;          // no error logged yet

    // content that is freed or reset in GB_bix_free
    A->b = NULL ;
    A->i = NULL ;
    A->x = NULL ;
    A->nzmax = 0 ;              // GB_NNZ(A) checks nzmax==0 before Ap[nvec]
    A->nvals = 0 ;              // for bitmapped matrices only
    A->b_shallow = false ;
    A->i_shallow = false ;
    A->x_shallow = false ;
    A->nzombies = 0 ;
    A->jumbled = false ;
    A->Pending = NULL ;

    //--------------------------------------------------------------------------
    // Allocate A->p and A->h if requested
    //--------------------------------------------------------------------------

    bool ok ;
    if (A_is_full_or_bitmap || Ap_option == GB_Ap_null)
    { 
        // A is not initialized yet; A->p and A->h are both NULL.
        // sparse case: GB_NNZ(A) must check A->nzmax == 0 since A->p might not
        // be allocated.
        // full case: A->x not yet allocated.  A->nzmax still zero
        // bitmap case: A->b, A->x not yet allocated.  A->nzmax still zero
        A->magic = GB_MAGIC2 ;
        A->p = NULL ;
        A->h = NULL ;
        ok = true ;
    }
    else if (Ap_option == GB_Ap_calloc)
    {
        // Sets the vector pointers to zero, which defines all vectors as empty
        A->magic = GB_MAGIC ;
        A->p = GB_CALLOC (A->plen+1, int64_t) ;
        ok = (A->p != NULL) ;
        if (A_is_hyper)
        { 
            // since nvec is zero, there is never any need to initialize A->h
            A->h = GB_MALLOC (A->plen, int64_t) ;
            ok = ok && (A->h != NULL) ;
        }
    }
    else // Ap_option == GB_Ap_malloc
    {
        // This is faster but can only be used internally by GraphBLAS since
        // the matrix is allocated but not yet completely initialized.  The
        // caller must set A->p [0..plen] and then set A->magic to GB_MAGIC,
        // before returning the matrix to the user application.  GB_NNZ(A) must
        // check A->nzmax == 0 since A->p [A->nvec] might be undefined.
        A->magic = GB_MAGIC2 ;
        A->p = GB_MALLOC (A->plen+1, int64_t) ;
        ok = (A->p != NULL) ;
        if (A_is_hyper)
        { 
            A->h = GB_MALLOC (A->plen, int64_t) ;
            ok = ok && (A->h != NULL) ;
        }
    }

    if (!ok)
    {
        // out of memory
        if (allocated_header)
        { 
            // only free the header if it was allocated here
            GB_Matrix_free (Ahandle) ;
        }
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    // The vector pointers A->p are initialized only if Ap_calloc is true
    if (A->magic == GB_MAGIC)
    { 
        ASSERT_MATRIX_OK (A, "new matrix from GB_new", GB0) ;
    }
    return (GrB_SUCCESS) ;
}

