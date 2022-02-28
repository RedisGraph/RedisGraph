//------------------------------------------------------------------------------
// GB_Matrix_allocate: allocate space for GrB_Matrix, GrB_Vector, or GrB_Scalar
//------------------------------------------------------------------------------

// A mock of the actual methods in ../Source.  These are just for testing.

// FIXME: We should remove this altogether and use GrB_Matrix_allocate

#include <assert.h>
#include "GB_Matrix_allocate.h"

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

//  TODO after allocating :
//      set A->nvec_nonempty
//      set A->nvals for bitmap
//      fill A->[p,h,b,i,x]

// The GrB_Matrix and GrB_Vector objects are different names for the same
// content.  A GrB_Vector is held as an m-by-1 non-hypersparse CSC matrix.
// This file is #include'd in GB.h to define the GB_Matrix_opaque,
// GB_Vector_opaque, and GB_Scalar_opaque structs.

GrB_Scalar GB_Scalar_allocate
(   
    GrB_Type type,          // NULL on the GPU
    size_t type_size,       // type->size
    int sparsity   // GxB_FULL, GxB_BITMAP, or GxB_SPARSE
)
{
    assert (sparsity != GxB_HYPERSPARSE) ;
    GrB_Scalar s = (GrB_Scalar) GB_Matrix_allocate (type, type_size,
        1, 1, sparsity, true, false, 1, 1) ;
    return (s) ;
}

//------------------------------------------------------------------------------
// GB_Vector_allocate
//------------------------------------------------------------------------------

// For a GrB_Vector object, as an m-by-1 non-hypersparse CSC matrix:
//      bool is_csc ;           // always true
//      int64_t plen ;          // always 1, so A->p always has length 2, and
//                              // contains [0 k] if the vector has k entries;
//                              // A->p is NULL if the GrB_Vector is bitmap.
//      int64_t vdim ;          // always 1
//      int64_t nvec ;          // always 1
//      int64_t *h ;            // always NULL

GrB_Vector GB_Vector_allocate
(   
    GrB_Type type,          // NULL on the GPU
    size_t type_size,       // type->size
    int64_t length,
    int sparsity,   // GxB_FULL, GxB_BITMAP, or GxB_SPARSE
    bool iso,
    int64_t anz     // ignored if sparsity is GxB_FULL or GxB_BITMAP
)
{
    assert (sparsity != GxB_HYPERSPARSE) ;
    GrB_Vector v = (GrB_Vector) GB_Matrix_allocate (type, type_size,
        1, length, sparsity, true, iso, anz, 1) ;
    return (v) ;
}

//------------------------------------------------------------------------------
// GB_Matrix_allocate
//------------------------------------------------------------------------------

GrB_Matrix GB_Matrix_allocate
(
        GrB_Type type,          // NULL on the GPU
        size_t type_size,       // type->size
        int64_t nrows,
        int64_t ncols,
        int sparsity,   //GxB_FULL, ..
        bool is_csc,
        bool iso,
        int64_t anz,    // ignored if sparsity is GxB_FULL or GxB_BITMAP
        int64_t nvec    // hypersparse only
) {

//------------------------------------------------------------------------------
// basic information: magic, error logger, and type
//------------------------------------------------------------------------------

// The first four items exactly match the first four items in the
// GrB_Descriptor struct.

    GrB_Matrix A = rmm_wrap_malloc(sizeof(struct GB_Matrix_opaque));

// int64_t magic ;         // for detecting uninitialized objects

    A->magic = GB_MAGIC;                 // object is valid

// size_t header_size ;    // size of the malloc'd block for this struct, or 0

    A->header_size = sizeof(struct GB_Matrix_opaque);  // or more

// char *logger ;          // error logger string

    A->logger = NULL;

// size_t logger_size ;    // size of the malloc'd block for logger, or 0

    A->logger_size = 0;

// The remaining items are specific the GrB_Matrix, GrB_Vector and GrB_Scalar
// structs, and do not appear in the GrB_Descriptor struct:
// GrB_Type type ;         // the type of each numerical entry

    A->type = type; // GrB_FP32 etc

//------------------------------------------------------------------------------
// compressed sparse vector data structure
//------------------------------------------------------------------------------

// The matrix can be held in one of 8 formats, each one consisting of a set of
// vectors.  The vector "names" are in the range 0 to A->vdim-1.  Each
// vector has length A->vlen.  These two values define the dimension of the
// matrix, where A is m-by-n.  The m and n dimenions are vlen and vdim for the
// CSC formats, and reversed for the CSR formats.

// Ap, Ai, Ax, Ah, and Ab are abbreviations for A->p, A->i, A->x, A->h, and
// A->b, respectively.

// For the sparse and hypersparse formats, Ap is an integer array of size
// A->plen+1, with Ap [0] always zero.  The matrix contains A->nvec sparse
// vectors, where A->nvec <= A->plen <= A->vdim.  The arrays Ai and Ax are
// of size A->(WHATERVER) and define the indices and values in each sparse vector.
// The total number of entries in the matrix is Ap [nvec] <= max # entries.
// For the bitmap and full sparsity structures, Ap and Ai are NULL.

// For both hypersparse and non-hypersparse matrices, if A->nvec_nonempty is
// computed, it is the number of vectors that contain at least one entry, where
// 0 <= A->nvec_nonempty <= A->nvec always holds.  If not computed,
// A->nvec_nonempty is equal to -1.

//------------------------------------------------------------------------------
// The 8 formats:  (hypersparse, sparse, bitmap, full) x (CSR or CSC)
//------------------------------------------------------------------------------

    A->is_csc = is_csc; // true: CSC, false: CSR

    //TODO: This should be enabled in master branch
//    A->iso = iso; // true: A->x holds just one entry, false: normal case

    // set the vector dimension and length
    if (is_csc) {
        A->vlen = nrows;
        A->vdim = ncols;
    } else {
        A->vlen = ncols;
        A->vdim = nrows;
    }

    if (sparsity == GxB_FULL || sparsity == GxB_BITMAP) {
        anz = nrows * ncols;
    }

// create phbix:  A->[p,h,b,i,x]

    A->p_size = 0;
    A->h_size = 0;
    A->b_size = 0;
    A->i_size = 0;
    A->x_size = 0;

    A->p = NULL;
    A->h = NULL;
    A->b = NULL;
    A->i = NULL;
    A->x = NULL;

    // for all matrices:

    if (iso) {
        // DIE if cuda_merge_in_progress
        // OK for master
        A->x_size = type_size;
    } else {
        A->x_size = anz * type_size;
    }
    A->x = rmm_wrap_malloc(A->x_size);

    A->nvals = 0;              // for bitmapped matrices only
    A->nzombies = 0;
    A->jumbled = false;
    A->Pending = NULL;
    A->nvec_nonempty = -1;
    A->hyper_switch = 0.0625;
    A->bitmap_switch = 0.10;
    A->sparsity_control = sparsity;

    switch (sparsity) {
        case GxB_FULL: {

            // --------------------------------------
            // Full structure:
            // --------------------------------------

            // Ah, Ap, Ai, and Ab are all NULL.
            // A->nvec == A->vdim.   A->plen is not needed (set to -1)

            A->plen = -1;
            A->nvec = A->vdim;
            A->nvec_nonempty = (A->vlen > 0) ? A->vdim : 0;

            // --------------------------------------
            // A->is_csc is true:  full CSC format
            // --------------------------------------

            // A is m-by-n: where A->vdim = n, and A->vlen = m

            // Column A(:,j) is held in Ax [p1:p2-1] where p1 = k*m, p2 = (k+1)*m.
            // A(i,j) at position p has row index i = p%m and value Ax [p]

            // --------------------------------------
            // A->is_csc is false:  full CSR format
            // --------------------------------------

            // A is m-by-n: where A->vdim = m, and A->vlen = n

            // Row A(i,:) is held in Ax [p1:p2-1] where p1 = k*n, p2 = (k+1)*n.
            // A(i,j) at position p has column index j = p%n and value Ax [p]
        }
            break;

        case GxB_BITMAP: {

            // --------------------------------------
            // Bitmap structure:
            // --------------------------------------

            // Ah, Ap, and Ai are NULL.  Ab is an int8_t array of size m*n.
            // A->nvec == A->vdim.   A->plen is not needed (set to -1)

            A->plen = -1;
            A->nvec = A->vdim;
            A->nvec_nonempty = (A->vlen > 0) ? A->vdim : 0;
            A->b_size = anz * sizeof(bool);
            A->b = rmm_wrap_malloc(A->b_size);

            // The bitmap structure is identical to the full structure, except for the
            // addition of the bitmap array A->b.

            // --------------------------------------
            // A->is_csc is true:  bitmap CSC format
            // --------------------------------------

            // A is m-by-n: where A->vdim = n, and A->vlen = m

            // Column A(:,j) is held in Ax [p1:p2-1] where p1 = k*m, p2 = (k+1)*m.
            // A(i,j) at position p has row index i = p%m and value Ax [p].
            // The entry A(i,j) is present if Ab [p] == 1, and not present if
            // Ab [p] == 0.

            // --------------------------------------
            // A->is_csc is false:  bitmap CSR format
            // --------------------------------------

            // A is m-by-n: where A->vdim = m, and A->vlen = n

            // Row A(i,:) is held in Ax [p1:p2-1] where p1 = k*n, p2 = (k+1)*n.
            // A(i,j) at position p has column index j = p%n and value Ax [p]
            // The entry A(i,j) is present if Ab [p] == 1, and not present if
            // Ab [p] == 0.
        }
            break;

        case GxB_SPARSE: {

            // --------------------------------------
            // Sparse structure:
            // --------------------------------------

            // Ah and Ab are NULL
            // A->nvec == A->plen == A->vdim

            A->plen = A->vdim;       // size of A->p is plen+1
            A->nvec = A->plen;
            A->p_size = (A->plen + 1) * sizeof(int64_t);
            A->i_size = anz * sizeof(int64_t);
            A->p = rmm_wrap_malloc(A->p_size);
            A->i = rmm_wrap_malloc(A->i_size);

            // --------------------------------------
            // A->is_csc is true:  sparse CSC format
            // --------------------------------------

            // Ap, Ai, and Ax store a sparse matrix in the a very similar style
            // as MATLAB and CSparse, as a collection of sparse column vectors.

            // Column A(:,j) is held in two parts: the row indices are in
            // Ai [Ap [j]...Ap [j+1]-1], and the numerical values are in the
            // same positions in Ax.

            // A is m-by-n: where A->vdim = n, and A->vlen = m

            // --------------------------------------
            // A->is_csc is false:  sparse CSR format
            // --------------------------------------

            // Ap, Ai, and Ax store a sparse matrix in CSR format, as a collection
            // of sparse row vectors.

            // Row A(i,:) is held in two parts: the column indices are in
            // Ai [Ap [i]...Ap [i+1]-1], and the numerical values are in the
            // same positions in Ax.

            // A is m-by-n: where A->vdim = m, and A->vlen = n
        }
            break;

        case GxB_HYPERSPARSE: {
            // --------------------------------------
            // Hypersparse structure:
            // --------------------------------------

            // Ab is NULL
            // Ah is non-NULL and has size A->plen; it is always kept sorted,
            // A->nvec <= A->plen <= A->vdim

            A->plen = nvec;     // size of A->p is plen+1
            A->nvec = nvec;
            A->p_size = (A->plen + 1) * sizeof(int64_t);
            A->h_size = (A->plen) * sizeof(int64_t);
            A->i_size = anz * sizeof(int64_t);
            A->p = rmm_wrap_malloc(A->p_size);
            A->h = rmm_wrap_malloc(A->h_size);
            A->i = rmm_wrap_malloc(A->i_size);

            // --------------------------------------
            // A->is_csc is true: hypersparse CSC format
            // --------------------------------------

            // A is held as a set of A->nvec sparse column vectors, but not all
            // columns 0 to n-1 are present.

            // If column A(:,j) has any entries, then j = Ah [k] for some
            // k in the range 0 to A->nvec-1.

            // Column A(:,j) is held in two parts: the row indices are in Ai [Ap
            // [k]...Ap [k+1]-1], and the numerical values are in the same
            // positions in Ax.

            // A is m-by-n: where A->vdim = n, and A->vlen = m

            // --------------------------------------
            // A->is_csc is false: hypersparse CSR format
            // --------------------------------------

            // A is held as a set of A->nvec sparse row vectors, but not all
            // row 0 to m-1 are present.

            // If row A(i,:) has any entries, then i = Ah [k] for some
            // k in the range 0 to A->nvec-1.

            // Row A(i,:) is held in two parts: the column indices are in Ai
            // [Ap [k]...Ap [k+1]-1], and the numerical values are in the same
            // positions in Ax.

            // A is m-by-n: where A->vdim = n, and A->vlen = m

        }
            break;

        default:;
    }

    A->p_shallow = false ;
    A->h_shallow = false ;
    A->b_shallow = false ;
    A->i_shallow = false ;
    A->x_shallow = false ;
    A->static_header = false ;    // true if this struct is statically allocated

    return (A) ;
}

