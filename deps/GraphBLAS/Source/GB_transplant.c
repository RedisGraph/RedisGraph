//------------------------------------------------------------------------------
// GB_transplant: replace contents of one matrix with another
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Transplant A into C, and then free A.  If any part of A is shallow, or if A
// must be typecasted, a deep copy is made into C.  Prior content of C is
// ignored.  Then A is freed, except for any shallow components of A which are
// left untouched (after unlinking them from A).  The resulting matrix C is not
// shallow.  This function is not user-callable.  The new type of C (ctype)
// must be compatible with A->type.

// Only GrB_SUCCESS and GrB_OUT_OF_MEMORY are returned by this function.

#include "GB.h"

GrB_Info GB_transplant          // transplant one matrix into another
(
    GrB_Matrix C,               // output matrix to overwrite with A
    const GrB_Type ctype,       // new type of C
    GrB_Matrix *Ahandle,        // input matrix to copy from and free
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Ahandle != NULL) ;
    GrB_Matrix A = *Ahandle ;
    ASSERT (GB_NOT_ALIASED (C, A)) ;

    ASSERT (C != NULL) ;
    ASSERT_OK (GB_check (A, "A before transplant", GB0)) ;
    ASSERT_OK (GB_check (ctype, "new type for C", GB0)) ;
    ASSERT (!GB_PENDING (A)) ;

    // zombies in A can be safely transplanted into C
    ASSERT (GB_ZOMBIES_OK (A)) ;

    // the ctype and A->type must be compatible.  C->type is ignored
    ASSERT (GB_Type_compatible (ctype, A->type)) ;

    int64_t anz = GB_NNZ (A) ;

    //--------------------------------------------------------------------------
    // clear C and transplant the type, size, and hypersparsity
    //--------------------------------------------------------------------------

    // free all content of C but not the Sauna
    GB_PHIX_FREE (C) ;

    ASSERT (!GB_PENDING (C)) ;
    ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (C->nzmax == 0) ;

    // It is now safe to change the type, dimension, and hypersparsity of C
    C->type = ctype ;
    C->type_size = ctype->size ;
    C->is_csc = A->is_csc ;
    C->is_hyper = A->is_hyper ;
    C->vlen = A->vlen ;
    C->vdim = A->vdim ;
    C->nvec_nonempty = A->nvec_nonempty ;

    // C->hyper_ratio is not modified by the transplant

    // C is not shallow, and has no content
    ASSERT (!C->p_shallow && !C->h_shallow && !C->i_shallow && !C->x_shallow) ;
    ASSERT (C->h == NULL && C->p == NULL && C->i == NULL && C->x == NULL) ;

    //--------------------------------------------------------------------------
    // transplant A->p vector pointers and A->h hyperlist
    //--------------------------------------------------------------------------

    double memory = 0 ;

    if (A->p_shallow || A->h_shallow)
    {

        //----------------------------------------------------------------------
        // A->p or A->h are shallow copies another matrix; make a deep copy
        //----------------------------------------------------------------------

        if (A->is_hyper)
        {
            // A is hypersparse, create new C->p and C->h
            memory += GBYTES (2 * (A->nvec) + 1, sizeof (int64_t)) ;
            C->plen = A->nvec ;
            C->nvec = A->nvec ;
            GB_MALLOC_MEMORY (C->p, C->plen+1, sizeof (int64_t)) ;
            GB_MALLOC_MEMORY (C->h, C->plen,   sizeof (int64_t)) ;
            if (C->p == NULL || C->h == NULL)
            { 
                // out of memory
                GB_CONTENT_FREE (C) ;
                GB_MATRIX_FREE (Ahandle) ;
                return (GB_OUT_OF_MEMORY (memory)) ;
            }

            // copy A->p and A->h into the newly created C->p and C->h
            memcpy (C->p, A->p, (A->nvec+1) * sizeof (int64_t)) ;
            memcpy (C->h, A->h,  A->nvec    * sizeof (int64_t)) ;
        }
        else
        {
            // A is non-hypersparse, create new C->p
            memory += GBYTES ((A->vdim) + 1, sizeof (int64_t)) ;
            C->plen = A->vdim ;
            C->nvec = A->vdim ;
            GB_MALLOC_MEMORY (C->p, C->plen+1, sizeof (int64_t)) ;
            if (C->p == NULL)
            { 
                // out of memory
                GB_CONTENT_FREE (C) ;
                GB_MATRIX_FREE (Ahandle) ;
                return (GB_OUT_OF_MEMORY (memory)) ;
            }

            // copy A->p into the newly created C->p
            memcpy (C->p, A->p, (A->vdim+1) * sizeof (int64_t)) ;
        }

        // free any non-shallow A->p and A->h content of A
        GB_ph_free (A) ;

    }
    else
    { 

        //----------------------------------------------------------------------
        // both A->p and A->h are not shallow: quick transplant into C
        //----------------------------------------------------------------------

        // Quick transplant of A->p and A->h into C.  This works for both
        // standard and hypersparse cases.
        ASSERT (C->p == NULL) ;
        ASSERT (C->h == NULL) ;
        C->p = A->p ;
        C->h = A->h ;
        C->plen = A->plen ;
        C->nvec = A->nvec ;
    }

    // A->p and A->h have been freed or removed from A
    A->p = NULL ;
    A->h = NULL ;
    A->p_shallow = false ;
    A->h_shallow = false ;
    C->p_shallow = false ;
    C->h_shallow = false ;

    C->magic = GB_MAGIC ;          // C is now initialized

    if (anz == 0)
    { 
        // quick return if A has no entries
        ASSERT_OK (GB_check (C, "C empty transplant", GB0)) ;
        GB_MATRIX_FREE (Ahandle) ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // allocate new space for C->i and C->x if A is shallow
    //--------------------------------------------------------------------------

    // get C->nzmax:  if either C->x or C->i must be allocated, then C->nzmax
    // is set to their minimum size.  Otherwise, if both C->x and C->i can
    // be transplanted from A, then they inherit the nzmax of A.

    ASSERT (C->x == NULL && C->i == NULL) ;
    bool allocate_Ci = (A->i_shallow) ;
    bool allocate_Cx = (A->x_shallow || C->type != A->type) ;
    C->nzmax = (allocate_Cx || allocate_Ci) ? anz : A->nzmax ;
    C->nzmax = GB_IMAX (C->nzmax, 1) ;

    // allocate new components if needed
    bool ok = true ;
    if (allocate_Cx)
    { 
        // allocate new C->x component
        GB_MALLOC_MEMORY (C->x, C->nzmax, C->type->size) ;
        ok = ok && (C->x != NULL) ;
        memory += GBYTES (C->nzmax, C->type->size) ;
    }

    if (allocate_Ci)
    { 
        // allocate new C->i component
        GB_MALLOC_MEMORY (C->i, C->nzmax, sizeof (int64_t)) ;
        ok = ok && (C->i != NULL) ;
        memory += GBYTES (C->nzmax, sizeof (int64_t)) ;
    }

    if (!ok)
    { 
        // out of memory
        GB_CONTENT_FREE (C) ;
        GB_MATRIX_FREE (Ahandle) ;
        return (GB_OUT_OF_MEMORY (memory)) ;
    }

    //--------------------------------------------------------------------------
    // transplant or copy A->x numerical values
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (C->type, "target C->type for values", GB0)) ;
    ASSERT_OK (GB_check (A->type, "source A->type for values", GB0)) ;

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
    GB_CRITICAL (GB_queue_insert (C)) ;

    //--------------------------------------------------------------------------
    // free A and return result
    //--------------------------------------------------------------------------

    GB_MATRIX_FREE (Ahandle) ;
    ASSERT_OK (GB_check (C, "C after transplant", GB0)) ;
    return (GrB_SUCCESS) ;
}

