//------------------------------------------------------------------------------
// GB_new: create a new GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This function is not user-callable.

// If Ap_calloc is false, the matrix has not yet been fully initialized, and
// A->magic is set to zero to denote this.  This case only occurs internally in
// GraphBLAS.  The internal function that calls GB_new must then allocate or
// initialize A->p itself, and then set A->magic = MAGIC when it does so.

// Only GrB_SUCCESS and GrB_OUT_OF_MEMORY can be returned by this function.

// The GrB_Matrix object holds both a sparse vector and a sparse matrix.  A
// vector is represented as an nrows-by-1 matrix, but it is sometimes treated
// differently in various methods.  Vectors are never transposed via a
// descriptor, for example, and are typically not transposed internally as well
// since it is costly to do so.

// Two structs with identical content must be defined.  so that the two types
// can be safely typecasted from one to the other.  It would be much cleaner to
// declare a single struct GrB_object_opaque, and then do

// typedef GB_object_opaque *GrB_Vector ;
// typedef GB_object_opaque *GrB_Matrix ;

// However, in this case the compiler complains that _Generic(x) can't
// distinguish between GrB_Vector and GrB_Matrix.  Therefore, GrB_Matrix
// and GrB_Vector are defined separately in GraphBLAS.h.

// Below are details of the content of a GrB_Matrix as defined in GraphBLAS.h:

    //--------------------------------------------------------------------------
    // basic information: magic, type, and size
    //--------------------------------------------------------------------------

//  int64_t magic ;         // for detecting uninitialized objects
//  GrB_Type type ;         // the type of each numerical entry
//  int64_t nrows ;         // number of rows
//  int64_t ncols ;         // number of columns, always 1 for vectors
//  int64_t nzmax ;         // size of i and x arrays

    //--------------------------------------------------------------------------
    // compressed sparse column data structure: CSparse and MATLAB-style
    //--------------------------------------------------------------------------

    // A->p, A->i, and A->x store a sparse matrix in the a very similar style
    // as MATLAB and CSparse.  Ap is an integer array of size ncols+1, with
    // Ap[0]=0 always being true.  Column A(:,j) is held in two parts: the row
    // indices are in Ai [Ap [j]...Ap [j+1]-1], and the numerical values are in
    // the same positions in Ax.  The total number of entries in matrix is
    // thus Ap [ncols].

    // Like MATLAB, the row indices in a GraphBLAS matrix (as implemented here)
    // are "always" kept sorted.  There are two temporary exceptions to this
    // rule.  GB_AxB_symbolic and GB_Matrix_subref can transpose their outputs.
    // In this case, they postpone the sort of each column and do it at the end
    // with a transpose.  Thus GB_transpose_pattern and GB_Matrix_transpose can
    // see as input a matrix with jumbled columns.  This is by design; a
    // transpose also does a bucket sort as a side effect.  No jumbled matrices
    // are returned to the user application.

    // Unlike MATLAB, explicit zeros are never dropped in a GraphBLAS matrix.
    // They cannot be since the semiring "zero" might be something else, like
    // -Infinity for a max-plus semiring.  However, dropping zeros is a minor
    // nuance in the data structure.

    // Like GraphBLAS, CSparse also keeps explicit zeros.  Unlike GraphBLAS,
    // CSparse allows its sparse matrices to be jumbled; its interface to
    // MATLAB always makes sure its matrices are sorted before returning them
    // to MATLAB.  Allowing a matrix to remain jumbled can be faster and
    // simpler, but it means that operations such as GrB_setElement and
    // GrB_*assign are very difficult (CSparse doesn't provide those
    // operations).

    // Finally, MATLAB only allows for boolean ("logical" class) and double
    // precision sparse matrices.  CSparse only supports double.  By contrast,
    // GraphBLAS supports any type , including types defined at run time by the
    // user application.  In the GraphBLAS code, the term "nonzero" is
    // sometimes used in the comments, but this is short-hand for the phrase
    // "an entry A(i,j) whose value is explicity held in the matrix and which
    // appears in the pattern; its value can be anything".  Entries not in the
    // pattern are simply "not there"; see for example GB_extractElement.  The
    // actual numerical value of these implicit entries is dependent upon the
    // identity value of the semiring's monoid operation used on the matrix.
    // The actual semiring is not held in the matrix itself, and there are no
    // restrictions on using a matrix in multiple semirings. 

//  int64_t *p ;            // array of size ncols+1
//  int64_t *i ;            // array of size nzmax
//  void *x ;               // size nzmax; each entry of size A->type->size

    //--------------------------------------------------------------------------
    // shallow matrices: like MATLAB but not in CSparse
    //--------------------------------------------------------------------------

    // Internal matrices in this implementation of GraphBLAS may have "shallow"
    // components.  These are pointers A->p, A->i, and A->x that point to the
    // content of another matrix.  Using shallow components speeds up
    // computations and saves memory, but shallow matrices are never passed
    // back to the user application.  They could be in the future, since the
    // GraphBLAS objects are opaque to the user application.

    // MATLAB allows for shallow matrices (try timing C=A in MATLAB for a large
    // sparse matrix A).  MATLAB breaks the shallow links if A or C are then
    // modified.  This is one reason why a MATLAB mexFunction is not supposed
    // to modify its inputs; it may be unknowingly modifying multiply matrices.
    // CSparse doesn't exploit shallow matrices; any data structure feature
    // here and below does not appear in CSparse.  MATLAB's internal data
    // structure is not published, but GraphBLAS handles shallow matries with
    // the following three boolean flags.

    // If the following are true, then the corresponding component of the
    // object is a pointer into components of another object.  They must not
    // be freed when freeing this object.

//  bool p_shallow ;        // true if p is a shallow copy
//  bool i_shallow ;        // true if i is a shallow copy
//  bool x_shallow ;        // true if x is a shallow copy

    //--------------------------------------------------------------------------
    // pending tuples
    //--------------------------------------------------------------------------

    // The list of pending tuples is a feature that does not appear in MATLAB
    // or CSparse, although something like it appears in CHOLMOD as the
    // "unpacked" matrix format, which allows the pattern of a matrix to be
    // modified when updating/downdating a Cholesky factorization.

    // If an entry A(i,j) does not appear in the data structure, assigning
    // A(i,j)=x requires all entries in columns j to the end of matrix to be
    // shifted down by one, taking up to O(nnz(A)) time to do so.  This very
    // slow, and it is why in both MATLAB and CSparse, the recommendation is to
    // create a list of tuples, and to build a sparse matrix all at once.  This
    // is done by the MATLAB "sparse" function, the CSparse "cs_compress", and
    // the GraphBLAS GrB_Matrix_build and GrB_Vector_build functions.

    // MATLAB does not have a "non-blocking" mode of operation, so A(i,j)=x can
    // be very slow for a single scalar x.  With GraphBLAS' non-blocking mode,
    // tuples from GrB_setElement and GrB_*assign can be held in another format
    // that is easy to update: a conventional list of tuples, held inside the
    // matrix itself.  A list of tuples is easy to update but hard to work with
    // in most operations, so whenever another GraphBLAS method or operation
    // needs to access the matrix, the matrix is "flattened" by applying all
    // the pending tuples.

    // When a new entry is added that doesn't exist in the matrix, it is added
    // to this list of pending tuples.  Only when the matrix is needed in
    // another operation are the pending tuples assembled into the compressed
    // sparse column form, A->p, A->i, and A->x.

    // CSparse has a very limited version of this feature; it can store a
    // sparse matrix in either compressed sparse column form, or as a list of
    // tuples, but never a combination of both as done here.  The CSparse list
    // of tuples can be augmented via cs_entry, which is analogous to
    // GB_setElement.

//  int64_t npending ;      // number of pending tuples to add to the matrix
//  int64_t max_npending ;  // size of ipending, jpending, and xpending arrays
//  bool sorted_pending ;   // true if pending tuples are in sorted order
//  int64_t *ipending ;     // row indices of pending tuples
//  int64_t *jpending ;     // col indices of pending tuples; NULL if ncols <= 1
//  void *xpending ;        // values of pending tuples

//  GrB_BinaryOp operator_pending ; // operator to assemble duplications

    //--------------------------------------------------------------------------
    // zombies
    //--------------------------------------------------------------------------

    // A "zombie" is the opposite of a pending tuple.  It is an entry A(i,j)
    // that has been marked for deletion, but hasn't been deleted yet because
    // it is more efficient to delete all zombies all at once, rather than one
    // (or a few) at a time.  An entry A(i,j) is marked as a zombie by
    // 'flipping' its row index via FLIP(i).  A flipped row index is negative,
    // and the actual row can be obtained by UNFLIP(i).  FLIP(i) is a function
    // that is its own inverse: FLIP(FLIP(x))=x for all x.

    // Using zombies allows entries to be marked for deletion.  Their row index
    // is still important, for two reasons: (1) the row indices in each column
    // of the matrix are kept sorted to enable the use of binary search, (2) a
    // zombie may be restored as a regular entry by a subsequent update, via
    // setElement, subassign, or assign.  In this case its row index is
    // unflipped and its value modified.  Had the zombie not been there, the
    // update would have to be placed in the pending tuple list.  It is more
    // efficient to keep the pending tuple lists as short as possible, so
    // zombies are kept as long as possible to facilitate faster subsequent
    // updates.

    // Unlike pending tuples, no list of zombies is needed since they are
    // already in the right place in the matrix.  However, methods and
    // operations in GraphBLAS that can't tolerate zombies in their input
    // matries can check the condition (A->nzombies > 0), and then delete all
    // of them if they appear, via GB_wait.

//  int64_t nzombies ;      // number of zombines marked for deletion

    //--------------------------------------------------------------------------
    // queue of matrices with work to do
    //--------------------------------------------------------------------------

    // The GrB_wait function tells GraphBLAS to finish all pending computations
    // on all matrices.  The function takes no arguments, so a list must be
    // maintained.  The list is implemented as a simple doubly-linked list.
    // All matrices with either pending tuples, or zombies, or both, appear in
    // this list.  If a matrix has neither pending tuples nor zombies, then it
    // does not appear in this list.

//  void *queue_next ;    // next matrix in the matrix queue
//  void *queue_prev ;    // prev matrix in the matrix queue

//------------------------------------------------------------------------------
// GB_new
//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_new                 // create a new matrix
(
    GrB_Matrix *matrix_handle,  // handle of matrix to create
    const GrB_Type type,        // matrix type
    const GrB_Index nrows,      // number of rows in matrix
    const GrB_Index ncols,      // number of columns in matrix
    const bool Ap_calloc,       // calloc A->p if true
    const bool Ap_malloc        // otherwise, malloc A->p
                                // if both false, return with A->p NULL
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (matrix_handle != NULL) ;
    (*matrix_handle) = NULL ;
    ASSERT_OK (GB_check (type, "type for GB_new", 0)) ;
    ASSERT (nrows <= GB_INDEX_MAX && ncols <= GB_INDEX_MAX) ;

    //--------------------------------------------------------------------------
    // create the matrix
    //--------------------------------------------------------------------------

    // allocate the matrix
    GB_CALLOC_MEMORY (*matrix_handle, 1, sizeof (GB_Matrix_opaque)) ;
    if (*matrix_handle == NULL)
    {
        // out of memory
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG, "out of memory"))) ;
    }

    // initialize the matrix
    GrB_Matrix A = *matrix_handle ;
    A->type = type ;
    A->nrows = (int64_t) nrows ;
    A->ncols = (int64_t) ncols ;
    A->nzmax = 0 ;

    A->p = NULL ;
    A->i = NULL ;
    A->x = NULL ;

    A->p_shallow = false ;
    A->i_shallow = false ;
    A->x_shallow = false ;

    // no pending tuples, no zombies
    A->npending = 0 ;
    A->max_npending = 0 ;
    A->sorted_pending = true ;
    A->operator_pending = NULL ;
    A->ipending = NULL ;
    A->jpending = NULL ;
    A->xpending = NULL ;

    // no pending operations to apply so on in the queue
    A->queue_next = NULL ;
    A->queue_prev = NULL ;
    A->enqueued = false ;

    // no zombies
    A->nzombies = 0 ;

    // allocate A->p if requested
    if (Ap_calloc)
    {
        // Sets the column pointers to zero, which defines all the columns
        // as empty.
        A->magic = MAGIC ;
        GB_CALLOC_MEMORY (A->p, A->ncols+1, sizeof (int64_t)) ;
    }
    else if (Ap_malloc)
    {
        // This is faster but can only be used internally by GraphBLAS since
        // the matrix is allocated but not yet fully initialized.  The
        // caller must set A->p [0..ncols] and then set A->magic to MAGIC,
        // before returning the matrix to the user application.
        A->magic = MAGIC2 ;
        GB_MALLOC_MEMORY (A->p, A->ncols+1, sizeof (int64_t)) ;
    }
    else
    {
        // A is not initialized yet, and A->p is NULL
        A->magic = MAGIC2 ;
        A->p = NULL ;
    }

    if ((Ap_calloc || Ap_malloc) && A->p == NULL)
    {
        // out of memory
        GB_FREE_MEMORY (*matrix_handle, 1, sizeof (GB_Matrix_opaque)) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required",
            GBYTES (A->ncols+1, sizeof (int64_t))))) ;
    }

    // The column pointers A->p are initialized only if Ap_calloc is true
    if (Ap_calloc)
    {
        ASSERT_OK (GB_check (A, "new matrix from GB_new", 0)) ;
    }
    return (REPORT_SUCCESS) ;
}

