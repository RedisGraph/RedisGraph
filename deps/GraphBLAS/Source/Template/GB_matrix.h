//------------------------------------------------------------------------------
// GB_matrix.h: definitions for GrB_Matrix and GrB_Vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The GrB_Matrix and GrB_Vector objects are different names for the same
// content.  A GrB_Vector is held as an m-by-1 non-hypersparse CSC matrix.
// This file is #include'd in GB.h to define the GB_Matrix_opaque and
// GB_Vector_opaque structs.  It would be cleaner to define just one opaque
// struct, and then GrB_Matrix and GrB_Vector would be typedef'd as pointers to
// the same struct, but then the compiler gets confused with Generic(x).

// For a GrB_Vector object, as an m-by-1 non-hypersparse CSC matrix:
//      bool is_csc ;           // always true
//      int64_t plen ;          // always 1, so A->p always has length 2, and
//                              // contains [0 k] if the vector has k entries;
//                              // A->p is NULL if the GrB_Vector is bitmap.
//      int64_t vdim ;          // always 1
//      int64_t nvec ;          // always 1
//      int64_t *h ;            // always NULL

//------------------------------------------------------------------------------
// basic information: magic, error logger, and type
//------------------------------------------------------------------------------

// The first four items exactly match the first four items in the
// GrB_Descriptor struct.

int64_t magic ;         // for detecting uninitialized objects
size_t header_size ;    // size of the malloc'd block for this struct, or 0
char *logger ;          // error logger string
size_t logger_size ;    // size of the malloc'd block for logger, or 0

// The remaining items are specific the GrB_Matrix, GrB_Vector and GxB_Scalar
// structs, and do not appear in the GrB_Descriptor struct:
GrB_Type type ;         // the type of each numerical entry

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
// vectors, where A->nvec <= A->plen <= A->vdim.  The arrays Ai and Ax define
// the indices and values in each sparse vector.  The total number of entries
// in the matrix is Ap [nvec] <= GB_nnz_max (A).
// For the bitmap and full sparsity structures, Ap and Ai are NULL.

// For both hypersparse and non-hypersparse matrices, if A->nvec_nonempty is
// computed, it is the number of vectors that contain at least one entry, where
// 0 <= A->nvec_nonempty <= A->nvec always holds.  If not computed,
// A->nvec_nonempty is equal to -1.

//------------------------------------------------------------------------------
// The 8 formats:  (hypersparse, sparse, bitmap, full) x (CSR or CSC)
//------------------------------------------------------------------------------

// --------------------------------------
// Full structure:
// --------------------------------------

    // Ah, Ap, Ai, and Ab are all NULL.
    // A->nvec == A->vdim.   A->plen is not needed (set to -1)

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

// --------------------------------------
// Bitmap structure:
// --------------------------------------

    // Ah, Ap, and Ai are NULL.  Ab is an int8_t array of size m*n.
    // A->nvec == A->vdim.   A->plen is not needed (set to -1)

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

// --------------------------------------
// Sparse structure:
// --------------------------------------

    // Ah and Ab are NULL
    // A->nvec == A->plen == A->vdim

    // --------------------------------------
    // A->is_csc is true:  sparse CSC format
    // --------------------------------------

        // Ap, Ai, and Ax store a sparse matrix in the a very similar style
        // as CSparse, as a collection of sparse column vectors.

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

// --------------------------------------
// Hypersparse structure:
// --------------------------------------

    // Ab is NULL
    // Ah is non-NULL and has size A->plen; it is always kept sorted,
    // A->nvec <= A->plen <= A->vdim

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

//------------------------------------------------------------------------------
// primary matrix content
//------------------------------------------------------------------------------

int64_t plen ;          // size of A->p and A->h
int64_t vlen ;          // length of each sparse vector
int64_t vdim ;          // number of vectors in the matrix
int64_t nvec ;          // number of non-empty vectors for hypersparse form,
                        // or same as vdim otherwise.  nvec <= plen.
                        // some of these vectors in Ah may actually be empty.
int64_t nvec_nonempty ; // the actual number of non-empty vectors, or -1 if
                        // not known

int64_t *h ;            // list of non-empty vectors: h_size >= 8*max(plen,1)
int64_t *p ;            // pointers: p_size >= 8*(plen+1)
int64_t *i ;            // indices:  i_size >= 8*max(anz,1)
void *x ;               // values:   x_size >= max(anz*A->type->size,1),
                        //           or x_size >= 1 if A is iso
int8_t *b ;             // bitmap:   b_size >= max(anz,1)
int64_t nvals ;         // nvals(A) if A is bitmap

size_t p_size ;         // exact size of A->p in bytes, zero if A->p is NULL
size_t h_size ;         // exact size of A->h in bytes, zero if A->h is NULL
size_t b_size ;         // exact size of A->b in bytes, zero if A->b is NULL
size_t i_size ;         // exact size of A->i in bytes, zero if A->i is NULL
size_t x_size ;         // exact size of A->x in bytes, zero if A->x is NULL

//------------------------------------------------------------------------------
// pending tuples
//------------------------------------------------------------------------------

// If an entry A(i,j) does not appear in the data structure, assigning A(i,j)=x
// requires all entries in vectors j to the end of matrix to be shifted down by
// one, taking up to O(nnz(A)) time to do so.  This very slow.

// Without its "non-blocking" mode of operation, A(i,j)=x would be very slow
// for a single scalar x.  With GraphBLAS' non-blocking mode, tuples from
// GrB_setElement and GrB_*assign can be held in another format that is easy to
// update: a conventional list of tuples, held inside the matrix itself.  A
// list of tuples is easy to update but hard to work with in most operations,
// so whenever another GraphBLAS method or operation needs to access the
// matrix, the matrix is finalized by applying all the pending updates.

// When a new entry is added that does not exist in the matrix, it is added to
// this list of pending tuples.  Only when the matrix is needed in another
// operation are the pending tuples assembled into the compressed sparse vector
// form, A->h, A->p, A->i, and A->x.

// The type of the list of pending tuples (Pending->type) need not be the same
// as the type of the matrix.  The GrB_assign and GxB_subassign operations can
// leave pending tuples in the matrix.  The accum operator, if not NULL,
// becomes the pending operator for assembling the pending tuples and adding
// them to the matrix.  For typecasting z=accum(x,y), the pending tuples are
// typecasted to the type of y.
//
// Let aij by the value of the pending tuple of a matrix C.  There are up to 5
// different types to consider: Pending->type (the type of aij), ztype, xtype,
// ytype, and ctype = C->type, (the type of the matrix C with pending tuples).
//
// If this is the first update to C(i,j), or if there is no accum operator,
// for for GrB_setElement:
//
//      aij of Pending->type
//      cij = (ctype) aij
//
// For subsequent tuples with GrB_assign or GxB_subassign, when accum is
// present:
//
//      y = (ytype) aij
//      x = (xtype) cij
//      z = accum (x,y), result of ztype
//      cij = (ctype) z ;
//
// Since the pending tuple must be typecasted to either ctype or ytype,
// depending on where it appears, it must be stored in its original type.

GB_Pending Pending ;        // list of pending tuples

//-----------------------------------------------------------------------------
// zombies
//-----------------------------------------------------------------------------

// A "zombie" is the opposite of a pending tuple.  It is an entry A(i,j) that
// has been marked for deletion, but has not been deleted yet because it is more
// efficient to delete all zombies all at once, rather than one (or a few) at a
// time.  An entry A(i,j) is marked as a zombie by 'flipping' its index via
// GB_FLIP(i).  A flipped index is negative, and the actual index can be
// obtained by GB_UNFLIP(i).  GB_FLIP(i) is a function that is its own inverse:
// GB_FLIP(GB_FLIP(x))=x for all x.

// Using zombies allows entries to be marked for deletion.  Their index is
// still important, for two reasons: (1) the indices in each vector of the
// matrix are kept sorted to enable the use of binary search, (2) a zombie may
// be restored as a regular entry by a subsequent update, via setElement,
// subassign, or assign.  In this case its index is unflipped and its value
// modified.  Had the zombie not been there, the update would have to be placed
// in the pending tuple list.  It is more efficient to keep the pending tuple
// lists as short as possible, so zombies are kept as long as possible to
// facilitate faster subsequent updates.

// Unlike pending tuples, no list of zombies is needed since they are already
// in the right place in the matrix.  However, methods and operations in
// GraphBLAS that cannot tolerate zombies in their input matries can check the
// condition (A->nzombies > 0), and then delete all of them if they appear, via
// GB_wait.

uint64_t nzombies ;     // number of zombies marked for deletion

//------------------------------------------------------------------------------
// sparsity control
//------------------------------------------------------------------------------

// The hyper_switch determines how the matrix is converted between the
// hypersparse and non-hypersparse formats.  Let n = A->vdim and let k be the
// actual number of non-empty vectors.  If A is hypersparse, k can be less than
// A->nvec since the latter can include vectors that appear in A->h but are
// actually empty.

// If a matrix is currently hypersparse, it can be converted to non-hypersparse
// if the condition (n <= 1 || k > n*hyper_switch*2) holds.  Otherwise, it
// stays hypersparse.  Note that if n <= 1 the matrix is always stored as
// non-hypersparse.

// If currently non-hypersparse, it can be converted to hypersparse if the
// condition (n > 1 && k <= n*hyper_switch) holds.  Otherwise, it stays
// non-hypersparse.  Note that if n <= 1 the matrix remains non-hypersparse.

// The default value of hyper_switch is assigned to be GxB_HYPER_DEFAULT at
// startup by GrB_init, and can then be modified globally with
// GxB_Global_Option_set.  All new matrices are created with the same
// hyper_switch.  Once a particular matrix has been constructed, its
// hyper_switch can be modified from the default with GxB_Matrix_Option_set.
// GrB_Vectors are never stored as hypersparse.

// A new matrix created via GrB_Matrix_new starts with k=0 and is created in
// hypersparse form unless (n <= 1 || 0 > hyper_switch) holds, where
// hyper_switch is the global default value.  GrB_Vectors are always
// non-hypersparse.

// To force a matrix to always stay non-hypersparse, use hyper_switch = -1 (or
// any negative number).  To force a matrix to always stay hypersparse, use
// hyper_switch = 1 or more.  For code readability, these values are also
// predefined for the user application as GxB_ALWAYS_HYPER and GxB_NEVER_HYPER.

// Summary for switching between formats:

// (1) by-row and by-column: there is no automatic switch between CSR/CSC.
//      By default, all GrB_Matrices are held in CSR form, unless they are
//      n-by-1 (then they are CSC).  The GrB_vector is always CSC.

// (2) If A->sparsity_control is GxB_AUTO_SPARSITY (15), then the following
//      rules are used to control the sparsity structure:
//
//      (a) When a matrix is created, it is empty and starts as hypersparse,
//          except that a GrB_Vector is never hypersparse.
//
//      (b) A hypersparse matrix with k non-empty vectors and
//          k > 2*n*A->hyper_switch is changed to sparse, and a sparse matrix
//          with k <= 1*n*A->hyper_switch is changed to hypersparse.
//          A->hyper_switch = (1/16) by default.  See GB_convert*test.
//
//      (c) A matrix with all entries present is converted to full (anz =
//          GB_nnz (A) = anz_dense = (A->vlen)*(A->vdim)).
//
//      (d) A matrix with anz = GB_nnz (A) entries and dimension A->vlen by
//          A->vdim can have at most anz_dense = (A->vlen)*(A->vdim) entries.
//          If A is sparse/hypersparse with anz > A->bitmap_switch * anz_dense,
//          then it switches to bitmap.  If A is bitmap and anz =
//          (A->bitmap_switch / 2) * anz_dense, it switches to sparse.  In
//          between those two regions, the sparsity structure is unchanged.

float hyper_switch ;    // controls conversion hyper to/from sparse
float bitmap_switch ;   // controls conversion sparse to/from bitmap
int sparsity_control ;  // controls sparsity structure: hypersparse,
                        // sparse, bitmap, or full, or any combination.

//------------------------------------------------------------------------------
// shallow matrices
//------------------------------------------------------------------------------

// Internal matrices in this implementation of GraphBLAS may have "shallow"
// components.  These are pointers A->p, A->h, A->i, A->b, and A->x that point
// to the content of another matrix.  Using shallow components speeds up
// computations and saves memory, but shallow matrices are never passed back to
// the user application.

// If the following are true, then the corresponding component of the
// object is a pointer into components of another object.  They must not
// be freed when freeing this object.

bool p_shallow ;        // true if p is a shallow copy
bool h_shallow ;        // true if h is a shallow copy
bool b_shallow ;        // true if b is a shallow copy
bool i_shallow ;        // true if i is a shallow copy
bool x_shallow ;        // true if x is a shallow copy
bool static_header ;    // true if this struct is statically allocated

//------------------------------------------------------------------------------
// other bool content
//------------------------------------------------------------------------------

bool is_csc ;           // true if stored by column, false if by row
bool jumbled ;          // true if the matrix may be jumbled.  bitmap and full
                        // matrices are never jumbled.

//------------------------------------------------------------------------------
// iso matrices
//------------------------------------------------------------------------------

// Entries that are present in a GraphBLAS matrix, vector, or scalar always
// have a value, and thus the C API of GraphBLAS does not have a structure-only
// data type, where the matrix, vector, or scalar consists only of its pattern,
// with no values assign to the entries in the matrix.  Such an object might be
// useful for representing unweighted graphs, but it would result in a
// mathematical mismatch with all other objects.  Operations between valued
// matrices and structure-only matrices would not be defined.

// Instead, the common practice is to assign all entries present in the matrix
// to be equal to a single value, typically 1 or true.  SuiteSparse:GraphBLAS
// exploits this typical practice by allowing for iso matrices, where all
// entries present have the same value, held as A->x [0].  The sparsity
// structure is kept, so in an iso matrix, A(i,j) is either equal to A->x [0],
// or not present in the sparsity pattern of A.

// If A is full, A->x is the only component present, and thus a full iso matrix
// takes only O(1) memory, regardless of its dimension.

bool iso ;              // true if all entries have the same value

//------------------------------------------------------------------------------
// iterating through a matrix
//------------------------------------------------------------------------------

// The matrix can be held in 8 formats: (hypersparse, sparse, bitmap, full) x
// (CSR, CSC).  Each of these can also be iso.  The comments below
// assume A is in CSC format but the code works for both CSR and CSC.
// The type is assumed to be double, just for illustration.

#ifdef for_comments_only    // only so vim will add color to the code below:

// for reference:
#define GBI(Ai,p,avlen) ((Ai == NULL) ? ((p) % (avlen)) : Ai [p])
#define GBB(Ab,p)       ((Ab == NULL) ? 1 : Ab [p])
#define GBP(Ap,k,avlen) ((Ap == NULL) ? ((k) * (avlen)) : Ap [k])
#define GBH(Ah,k)       ((Ah == NULL) ? (k) : Ah [k])
#define GBX(Ax,p,A_iso) (Ax [(A_iso) ? 0 : (p)])

    // A->vdim: the vector dimension of A (ncols(A))
    // A->nvec: # of vectors that appear in A.  For the hypersparse case,
    //          these are the number of column indices in Ah [0..nvec-1], since
    //          A is CSC.  For all cases, Ap [0...nvec] are the pointers.

    //--------------------
    // (1) full      // A->h, A->p, A->i, A->b are NULL, A->nvec == A->vdim

        int64_t vlen = A->vlen ;
        for (k = 0 ; k < A->nvec ; k++)
        {
            j = k ;
            // operate on column A(:,j)
            int64_t pA_start = k * vlen ;
            int64_t pA_end   = (k+1) * vlen ;
            for (p = pA_start ; p < pA_end ; p++)
            {
                // entry A(i,j) with row index i and value aij
                int64_t i = (p % vlen) ;
                double aij = GBX (Ax, p, A->iso) ;
            }
        }

    //--------------------
    // (2) bitmap    // A->h, A->p, A->i are NULL, A->nvec == A->vdim

        int64_t vlen = A->vlen ;
        for (k = 0 ; k < A->nvec ; k++)
        {
            j = k ;
            // operate on column A(:,j)
            int64_t pA_start = k * vlen ;
            int64_t pA_end   = (k+1) * vlen ;
            for (p = pA_start ; p < pA_end ; p++)
            {
                if (Ab [p] != 0)
                {
                    // entry A(i,j) with row index i and value aij
                    int64_t i = (p % vlen) ;
                    double aij = GBX (Ax, p, A->iso) ;
                }
                else
                {
                    // A(i,j) is not present
                }
            }
        }

    //--------------------
    // (3) sparse     // A->h is NULL, A->nvec == A->vdim

        for (k = 0 ; k < A->nvec ; k++)
        {
            j = k ;
            // operate on column A(:,j)
            for (p = Ap [k] ; p < Ap [k+1] ; p++)
            {
                // entry A(i,j) with row index i and value aij
                int64_t i = Ai [p] ;
                double aij = GBX (Ax, p, A->iso) ;
            }
        }

    //--------------------
    // (4) hypersparse  // A->h is non-NULL, A->nvec <= A->dim

        for (k = 0 ; k < A->nvec ; k++)
        {
            j = A->h [k]
            // operate on column A(:,j)
            for (p = Ap [k] ; p < Ap [k+1] ; p++)
            {
                // entry A(i,j) with row index i and value aij
                int64_t i = Ai [p] ;
                double aij = GBX (Ax, p, A->iso) ;
            }
        }

    //--------------------
    // generic: for any matrix

        int64_t vlen = A->vlen ;
        for (k = 0 ; k < A->nvec ; k++)
        {
            j = GBH (Ah, k) ;
            // operate on column A(:,j)
            int64_t pA_start = GBP (Ap, k, vlen) ;
            int64_t pA_end   = GBP (Ap, k+1, vlen) ;
            for (p = pA_start ; p < pA_end ; p++)
            {
                if (!GBB (Ab, p)) continue ;
                // entry A(i,j) with row index i and value aij
                int64_t i = GBI (Ai, p, vlen) ;
                double aij = GBX (Ax, p, A->iso) ;
            }
        }

#endif

