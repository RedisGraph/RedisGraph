//------------------------------------------------------------------------------
// GB_Matrix_transplant: replace contents of one matrix with another
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Transplant A into C, and then free A.  If any part of A is shallow, or if A
// must be typecasted, a deep copy is made into C.  Then A is freed, except for
// any shallow components of A which are left untouched (after unlinking them
// from A).  The resulting matrix C is not shallow.  This function is not
// user-callable.  The matrices C and A must have the same dimensions and
// the types must be compatible.

// Only GrB_SUCCESS and GrB_OUT_OF_MEMORY are returned by this function.

#include "GB.h"

GrB_Info GB_Matrix_transplant   // transplant one matrix into another
(
    GrB_Matrix C,               // matrix to overwrite with A
    const GrB_Type ctype,       // new type of C
    GrB_Matrix *Ahandle         // matrix to copy from and free
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Ahandle != NULL) ;
    GrB_Matrix A = *Ahandle ;

    // C can be just a header; C->p may or may not be present
    // but in either case, if C->p is not null it is not shallow
    ASSERT (C != NULL && !C->p_shallow) ;
    ASSERT_OK (GB_check (A, "A before transplant", 0)) ;
    ASSERT_OK (GB_check (ctype, "new type for C", 0)) ;
    ASSERT (!PENDING (A)) ;

    // zombies in A can be safely transplanted into C
    ASSERT (ZOMBIES_OK (A)) ;

    // C must be the same dimensions as A, and the types must be compatible
    ASSERT (C->nrows == A->nrows && C->ncols == A->ncols) ;
    ASSERT (GB_Type_compatible (ctype, A->type)) ;

    int64_t anz = NNZ (A) ;

    //--------------------------------------------------------------------------
    // clear C and transplant the type
    //--------------------------------------------------------------------------

    // free all content of C, except C->p if present
    GB_Matrix_ixfree (C) ;
    ASSERT (!PENDING (C)) ; ASSERT (!ZOMBIES (C)) ;

    // It is now safe to change the type of C
    C->type = ctype ;

    //--------------------------------------------------------------------------
    // A->p column pointers
    //--------------------------------------------------------------------------

    double memory = 0 ;

    if (A->p_shallow)
    {
        // A->p is a shallow copy of another matrix, so we need a deep copy.
        // In this case C->p is guaranteed to be present.
        ASSERT (C->p != NULL) ;
        memcpy (C->p, A->p, (C->ncols+1) * sizeof (int64_t)) ;
        // A->p is shallow anyway, so remove it from A
        A->p = NULL ;
    }
    else
    {
        // A->p is not shallow, so free the existing C->p if it exists and
        // replace with A->p
        GB_FREE_MEMORY (C->p, C->ncols+1, sizeof (int64_t)) ;
        C->p = A->p ;
        A->p = NULL ;
    }

    ASSERT (A->p == NULL) ;     // has been freed or removed
    A->p_shallow = false ;

    ASSERT (C->p != NULL) ;
    C->p_shallow = false ;

    C->magic = MAGIC ;          // C is now initialized

    if (anz == 0)
    {
        // quick return if A has no entries
        GB_MATRIX_FREE (Ahandle) ;
        ASSERT_OK (GB_check (C, "C empty transplant", 0)) ;
        return (REPORT_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // allocate new space for C if A is shallow
    //--------------------------------------------------------------------------

    // get C->nzmax:  if either C->x or C->i must be allocated, then C->nzmax
    // is set to their minimum size.  Otherwise, if both C->x and C->i can
    // be transplanted from A, then they inherit the nzmax of A.

    ASSERT (C->x == NULL && C->i == NULL) ;
    bool allocate_Ci = (A->i_shallow) ;
    bool allocate_Cx = (A->x_shallow || C->type != A->type) ;
    C->nzmax = (allocate_Cx || allocate_Ci) ? anz : A->nzmax ;
    C->nzmax = IMAX (C->nzmax, 1) ;

    // allocate new components if needed
    bool ok = true ;
    if (allocate_Cx)
    {
        GB_MALLOC_MEMORY (C->x, C->nzmax, C->type->size) ;
        ok = ok && (C->x != NULL) ;
        memory += GBYTES (C->nzmax, C->type->size) ;
    }

    if (allocate_Ci)
    {
        GB_MALLOC_MEMORY (C->i, C->nzmax, sizeof (int64_t)) ;
        ok = ok && (C->i != NULL) ;
        memory += GBYTES (C->nzmax, sizeof (int64_t)) ;
    }

    if (!ok)
    {
        // out of memory
        GB_Matrix_clear (C) ;
        GB_MATRIX_FREE (Ahandle) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "out of memory, %g GBytes required", memory))) ;
    }

    //--------------------------------------------------------------------------
    // transplant or copy A->x numerical values
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (C->type, "target C->type for values", 0)) ;
    ASSERT_OK (GB_check (A->type, "source A->type for values", 0)) ;

    if (C->type == A->type)
    {
        // types match
        if (A->x_shallow)
        {
            // A is shallow so make a deep copy; no typecast needed
            memcpy (C->x, A->x, anz * C->type->size) ;
            A->x = NULL ;
        }
        else
        {
            // OK to move pointers instead
            C->x = A->x ;
            A->x = NULL ;
        }
    }
    else
    {
        // types differ, must typecast from A to C
        GB_cast_array (C->x, C->type->code, A->x, A->type->code, anz) ;
        if (!A->x_shallow)
        {
            GB_FREE_MEMORY (A->x, A->nzmax, A->type->size) ;
        }
        A->x = NULL ;
    }

    ASSERT (A->x == NULL) ;     // has been freed or removed
    A->x_shallow = false ; 

    ASSERT (C->x != NULL) ;
    C->x_shallow = false ;

    //--------------------------------------------------------------------------
    // transplant or copy A->i row indices
    //--------------------------------------------------------------------------

    if (A->i_shallow)
    {
        // A->i is a shallow copy of another matrix, so we need a deep copy
        memcpy (C->i, A->i, anz * sizeof (int64_t)) ;
        A->i = NULL ;
    }
    else
    {
        // A->i is not shallow, so just copy the pointer from A to C
        C->i = A->i ;
        A->i = NULL ;
    }

    ASSERT (A->i == NULL) ;         // has been freed or removed
    A->i_shallow = false ; 

    ASSERT (C->i != NULL) ;
    C->i_shallow = false ;

    C->nzombies = A->nzombies ;     // zombies have been transplanted into C
    GB_queue_insert (C) ;

    //--------------------------------------------------------------------------
    // free A and return result
    //--------------------------------------------------------------------------

    // if A has zombies, it is removed from the queue by GB_Matrix_free
    GB_MATRIX_FREE (Ahandle) ;
    ASSERT_OK (GB_check (C, "C after transplant", 0)) ;
    return (REPORT_SUCCESS) ;
}

