//------------------------------------------------------------------------------
// GB_matrix.h: definitions for GrB_Matrix and GrB_Vector
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The GrB_Matrix and GrB_Vector objects are different names for the same
// content.  A GrB_Vector is held as an m-by-1 non-hypersparse CSC matrix.
// This file is #include'd in GB.h to define the GB_Matrix_opaque and
// GB_Vector_opaque structs.  It would be cleaner to define just one opaque
// struct, and then GrB_Matrix and GrB_Vector would be typedef'd as pointers to
// the same struct, but then the compiler gets confused with Generic(x).

// For a GrB_Vector object, as an m-by-1 non-hypersparse CSC matrix:
//      bool is_hyper ;         // always false
//      bool is_csc ;           // always true
//      int64_t plen ;          // always 1, so A->p always has length 2, and
//                              // contains [0 k] if the vector has k entries
//      int64_t vdim ;          // always 1
//      int64_t nvec ;          // always 1
//      int64_t *h ;            // always NULL

//------------------------------------------------------------------------------
// basic information: magic and type
//------------------------------------------------------------------------------

int64_t magic ;         // for detecting uninitialized objects
GrB_Type type ;         // the type of each numerical entry
size_t type_size ;      // type->size, copied here since the type could be
                        // user-defined, and freed before the matrix or vector

//------------------------------------------------------------------------------
// compressed sparse vector data structure
//------------------------------------------------------------------------------

// The matrix can be held in one of 6 formats, each one consisting of a set of
// sparse vectors.  The vector "names" are in the range 0 to A->vdim-1.  Each
// vector has length A->vlen.  These two values define the dimension of the
// matrix, where A is m-by-n.  The m and n dimenions are vlen and vdim for the
// standard CSC and hypersparse-CSC formats, and reversed for the standard CSR
// and hypersparse-CSR formats.

// Ap, Ai, Ax, and Ah are abbreviations for A->p, A->i, A->x, and A->h.

// For all formats Ap is an integer array of size A->plen+1, with Ap [0] always
// zero.  The matrix contains A->nvec sparse vectors, where A->nvec <= A->plen
// <= A->vdim.  The arrays Ai and Ax are both of size A->nzmax, and define the
// indices and values in each sparse vector.  The total number of entries in
// the matrix is Ap [nvec] <= A->nzmax.

// For both hypersparse and non-hypersparse matrices, if A->nvec_nonempty is
// computed, it is the number of vectors that contain at least one entry, where
// 0 <= A->nvec_nonempty <= A->nvec always holds.  If not computed,
// A->nvec_nonempty is equal to -1.

//------------------------------------------------------------------------------
// The Primary 4 formats:  (standard or hypersparse) * (CSR or CSC)
//------------------------------------------------------------------------------

// A->is_slice is false.  These are the only matrices returned to the user.

// --------------------------------------
// A->is_hyper is false: standard format.
// --------------------------------------

    // Ah is NULL
    // A->nvec == A->plen == A->vdim

    // --------------------------------------
    // A->is_csc is true:  standard CSC format
    // --------------------------------------

        // Ap, Ai, and Ax store a sparse matrix in the a very similar style
        // as MATLAB and CSparse, as a collection of sparse column vectors.

        // Column A(:,j) is held in two parts: the row indices are in
        // Ai [Ap [j]...Ap [j+1]-1], and the numerical values are in the
        // same positions in Ax.

        // A is m-by-n: where A->vdim = n, and A->vlen = m

    // --------------------------------------
    // A->is_csc is false:  standard CSR format
    // --------------------------------------

        // Ap, Ai, and Ax store a sparse matrix in CSR format, as a collection
        // of sparse row vectors.

        // Row A(i,:) is held in two parts: the column indices are in
        // Ai [Ap [i]...Ap [i+1]-1], and the numerical values are in the
        // same positions in Ax.

        // A is m-by-n: where A->vdim = m, and A->vlen = n

// --------------------------------------
// A->is_hyper is true: hypersparse format
// --------------------------------------

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
// Internal formats: a slice or hyperslice (either CSR or CSC)
//------------------------------------------------------------------------------

    // A->is_slice is true.  This format is only used inside GraphBLAS, for
    // internal slices or hyperslices of another matrix.

    // It is the same as the hypersparse format, except that Ah may be NULL.
    // All Ah, Ap, Ai, Ax content of the slice is shallow.
    // Ap [0] == 0 only for the leftmost slice; it is normally >= 0.

    // slice: A->is_hyper is false

            // Ah is NULL: Ah [0..A->nvec-1] is implicitly the contiguous list:
            // [A->hfirst ... A->hfirst + A->nvec - 1].  The original matrix is
            // not hypersparse.  A->plen gives the size of Ap, as above.  Ap
            // points into an offset of p of the original matrix.

   // hyperslice: A->is_hyper is true

            // Ah is not-NULL.  The original matrix is hypersparse.  Ah points
            // to an offset inside the h of the original matrix.  A->hfirst is
            // zero, and not used.

//------------------------------------------------------------------------------

// Like MATLAB, the indices in a GraphBLAS matrix (as implemented here) are
// "always" kept sorted.  There is one temporary exception to this rule.
// GB_subref is allowed return a matrix with unsorted vectors, if it will be
// later be transposed by its caller.  The transpose does the sort.

// Unlike MATLAB, explicit zeros are never dropped in a GraphBLAS matrix.  They
// cannot be since the semiring "zero" might be something else, like -Infinity
// for a max-plus semiring.  However, dropping zeros is a minor nuance in the
// data structure.

// Like GraphBLAS, CSparse also keeps explicit zeros.  Unlike GraphBLAS,
// CSparse allows its sparse matrices to be jumbled; its interface to MATLAB
// always makes sure its matrices are sorted before returning them to MATLAB.
// Allowing a matrix to remain jumbled can be faster and simpler, but it means
// that operations such as GrB_setElement and GrB_*assign are very difficult
// (CSparse does not provide those operations).

// Finally, MATLAB only allows for boolean ("logical" class) and double
// precision sparse matrices.  CSparse only supports double.  By contrast,
// GraphBLAS supports any type, including types defined at run time by the user
// application.  In the GraphBLAS code, the term "nonzero" is sometimes used in
// the comments, but this is short-hand for the phrase "an entry A(i,j) whose
// value is explicity held in the matrix and which appears in the pattern; its
// value can be anything".  Entries not in the pattern are simply "not there";
// see for example GB_extractElement.  The actual numerical value of these
// implicit entries is dependent upon the identity value of the semiring's
// monoid operation used on the matrix.  The actual semiring is not held in the
// matrix itself, and there are no restrictions on using a matrix in multiple
// semirings.

// The bool content is placed last, to reduce the size of the struct.

// bool is_hyper ;      // true if the matrix is hypersparse
// bool is_csc ;        // true if stored by column (CSC or hypersparse CSC)
                        // false if by row (CSR or hypersparse CSR)

double hyper_ratio ;    // controls conversion to/from hypersparse

int64_t plen ;          // A->h has size plen, A->p has size plen+1
int64_t vlen ;          // length of each sparse vector
int64_t vdim ;          // number of vectors in the matrix
int64_t nvec ;          // number of non-empty vectors for hypersparse form,
                        // or same as vdim otherwise.  nvec <= plen.
                        // some of these vectors in Ah may actually be empty.
int64_t nvec_nonempty ; // the actual number of non-empty vectors, or -1 if
                        // not known

int64_t *h ;            // list of non-empty vectors of size plen
int64_t *p ;            // array of size plen+1
int64_t *i ;            // array of size nzmax
void *x ;               // size nzmax; each entry of size A->type->size
int64_t nzmax ;         // size of i and x arrays

int64_t hfirst ;        // if A->is_hyper is false but A->is_slice is true,
                        // then A->h is NULL, and the matrix A is a slice
                        // of another standard matrix S.  The vectors in
                        // A are the contiguous list:
                        // [A->hfirst ... A->hfirst+A->nvec-1].
                        // Otherwise, A->hfirst is zero.

// The hyper_ratio determines how the matrix is converted between the
// hypersparse and non-hypersparse formats.  Let n = A->vdim and let k be the
// actual number of non-empty vectors.  If A is hypersparse, k can be less than
// A->nvec since the latter can include vectors that appear in A->h but are
// actually empty.

// If a matrix is currently hypersparse, it can be converted to non-hypersparse
// if the condition (n <= 1 || k > n*hyper_ratio*2) holds.  Otherwise, it stays
// hypersparse.  Note that if n <= 1 the matrix is always stored as
// non-hypersparse.

// If currently non-hypersparse, it can be converted to hypersparse if the
// condition (n > 1 && k <= n*hyper_ratio) holds.  Otherwise, it stays
// non-hypersparse.  Note that if n <= 1 the matrix remains non-hypersparse.

// The default value of hyper_ratio is assigned to be GxB_HYPER_DEFAULT at
// startup by GrB_init, and can then be modified globally with
// GxB_Global_Option_set.  All new matrices are created with the same ratio.
// Once a particular matrix has been constructed, its hypersparsity ratio can
// be modified from the default with GxB_Matrix_Option_set.  GrB_Vectors are
// always stored as non-hypersparse.

// A new matrix created via GrB_Matrix_new starts with k=0 and is created in
// hypersparse form unless (n <= 1 || 0 > hyper_ratio) holds, where hyper_ratio
// is the global default value.  GrB_Vectors are always non-hypersparse.

// To force a matrix to always stay non-hypersparse, use hyper_ratio = -1 (or
// any negative number).  To force a matrix to always stay hypersparse, use
// hyper_ratio = 1 or more.  For code readability, these values are also
// predefined for the user application as the constants GxB_ALWAYS_HYPER and
// GxB_NEVER_HYPER.

//------------------------------------------------------------------------------
// pending tuples
//------------------------------------------------------------------------------

// The list of pending tuples is a feature that does not appear in MATLAB or
// CSparse, although something like it appears in CHOLMOD as the "unpacked"
// matrix format, which allows the pattern of a matrix to be modified when
// updating/downdating a Cholesky factorization.

// If an entry A(i,j) does not appear in the data structure, assigning A(i,j)=x
// requires all entries in vectors j to the end of matrix to be shifted down by
// one, taking up to O(nnz(A)) time to do so.  This very slow, and it is why in
// both MATLAB and CSparse, the recommendation is to create a list of tuples,
// and to build a sparse matrix all at once.  This is done by the MATLAB
// "sparse" function, the CSparse "cs_compress", and the GraphBLAS
// GrB_Matrix_build and GrB_Vector_build functions.

// MATLAB does not have a "non-blocking" mode of operation, so A(i,j)=x can be
// very slow for a single scalar x.  With GraphBLAS' non-blocking mode, tuples
// from GrB_setElement and GrB_*assign can be held in another format that is
// easy to update: a conventional list of tuples, held inside the matrix
// itself.  A list of tuples is easy to update but hard to work with in most
// operations, so whenever another GraphBLAS method or operation needs to
// access the matrix, the matrix is "flattened" by applying all the pending
// tuples.

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

int64_t nzombies ;      // number of zombies marked for deletion

//------------------------------------------------------------------------------
// statistics
//------------------------------------------------------------------------------

GrB_Desc_Value AxB_method_used ;    // last method used for C=A*B (this is C)

//------------------------------------------------------------------------------
// queue of matrices with work to do
//------------------------------------------------------------------------------

// The GrB_wait function tells GraphBLAS to finish all pending computations on
// all matrices.  The function takes no arguments, so a list must be
// maintained.  The list is implemented as a simple doubly-linked list.  All
// matrices with either pending tuples, or zombies, or both, appear in this
// list.  If a matrix has neither pending tuples nor zombies, then it does not
// appear in this list.

void *queue_next ;      // next matrix in the matrix queue
void *queue_prev ;      // prev matrix in the matrix queue
bool enqueued ;         // true if the matrix is in the queue

//------------------------------------------------------------------------------
// shallow matrices: like MATLAB but not in CSparse
//------------------------------------------------------------------------------

// Internal matrices in this implementation of GraphBLAS may have "shallow"
// components.  These are pointers A->p, A->i, and A->x that point to the
// content of another matrix.  Using shallow components speeds up computations
// and saves memory, but shallow matrices are never passed back to the user
// application.  They could be in the future, since the GraphBLAS objects are
// opaque to the user application.

// If the following are true, then the corresponding component of the
// object is a pointer into components of another object.  They must not
// be freed when freeing this object.

bool p_shallow ;        // true if p is a shallow copy
bool h_shallow ;        // true if h is a shallow copy
bool i_shallow ;        // true if i is a shallow copy
bool x_shallow ;        // true if x is a shallow copy

//------------------------------------------------------------------------------
// other bool content
//------------------------------------------------------------------------------

// The boolean content appears last, to reduce the size of the struct

bool is_hyper ;         // true if the matrix is hypersparse
bool is_csc ;           // true if stored by column (CSC or hypersparse CSC)
bool is_slice ;         // true if the matrix is a slice or hyperslice

