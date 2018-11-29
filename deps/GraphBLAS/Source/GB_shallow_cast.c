//------------------------------------------------------------------------------
// GB_shallow_cast: create a shallow copy of a matrix, optionally typecasted
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C = (ctype) A.

// Create a shallow copy of a matrix, possibly typecasted.

// The CSR/CSC format of C and A can differ, but they have they same vlen and
// vdim.  This function is CSR/CSC agnostic, except that C_is_csc is used to
// set the C->is_csc state in C.

// The values are a shallow copy unless they need to be typecasted.

// The pattern is always a shallow copy.  No errors are checked except for
// out-of-memory conditions.  This function is not user-callable.  Shallow
// matrices are never passed back to the user.

// Compare this function with GB_shallow_op.c


#include "GB.h"

GrB_Info GB_shallow_cast    // create a shallow typecasted matrix
(
    GrB_Matrix *Chandle,    // output matrix C, of type op->ztype
    const GrB_Type ctype,   // type of the output matrix C
    const bool C_is_csc,    // desired CSR/CSC format of C
    const GrB_Matrix A,     // input matrix to typecast
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;
    ASSERT_OK (GB_check (A, "A for shallow cast", GB0)) ;
    ASSERT_OK (GB_check (ctype, "ctype for typecast", GB0)) ;
    ASSERT (GB_Type_compatible (ctype, A->type)) ;
    ASSERT ((A->nzmax == 0) == (A->i == NULL && A->x == NULL)) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;

    (*Chandle) = NULL ;

    //--------------------------------------------------------------------------
    // construct a shallow copy of A for the pattern of C
    //--------------------------------------------------------------------------

    // allocate the struct for C, but do not allocate C->h, C->p, C->i, or C->x.
    // C has the exact same hypersparsity as A.
    GrB_Info info ;
    GrB_Matrix C = NULL ;           // allocate a new header for C
    GB_NEW (&C, ctype, A->vlen, A->vdim, GB_Ap_null, C_is_csc,
        GB_SAME_HYPER_AS (A->is_hyper), A->hyper_ratio, 0) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // make a shallow copy of the vector pointers
    //--------------------------------------------------------------------------

    ASSERT (C->magic == GB_MAGIC2) ;   // [ be careful; C not yet initialized
    C->p_shallow = true ;           // C->p not freed when freeing C
    C->h_shallow = true ;           // C->h not freed when freeing C
    C->p = A->p ;                   // C->p is of size A->plen + 1
    C->h = A->h ;                   // C->h is of size A->plen
    C->plen = A->plen ;             // C and A have the same hyperlist sizes
    C->nvec = A->nvec ;
    C->nvec_nonempty = A->nvec_nonempty ;
    C->magic = GB_MAGIC ;           // C is now initialized ]

    //--------------------------------------------------------------------------
    // check for empty matrix
    //--------------------------------------------------------------------------

    if (A->nzmax == 0)
    { 
        // C->p and C->h are shallow but the rest is empty
        C->nzmax = 0 ;
        C->i = NULL ;
        C->x = NULL ;
        C->i_shallow = false ;
        C->x_shallow = false ;
        ASSERT_OK (GB_check (C, "C = quick copy of empty A", GB0)) ;
        (*Chandle) = C ;
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // make a shallow copy of the pattern
    //--------------------------------------------------------------------------

    C->i = A->i ;               // of size A->nzmax
    C->i_shallow = true ;       // C->i will not be freed when freeing C

    //--------------------------------------------------------------------------
    // make a shallow copy of the values, or allocate new ones
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ (A) ;
    ASSERT (A->nzmax >= GB_IMAX (anz,1)) ;

    if (C->type == A->type)
    { 
        // no work is done at all.  C is a pure shallow copy
        C->nzmax = A->nzmax ;
        C->x = A->x ;
        C->x_shallow = true ;       // C->x will not be freed when freeing C
        ASSERT_OK (GB_check (C, "C = pure shallow (A)", GB0)) ;
        (*Chandle) = C ;
        return (GrB_SUCCESS) ;
    }

    // allocate new space for the numerical values of C
    C->nzmax = GB_IMAX (anz,1) ;
    GB_MALLOC_MEMORY (C->x, C->nzmax, C->type->size) ;
    C->x_shallow = false ;          // free C->x when freeing C
    if (C->x == NULL)
    { 
        // out of memory
        double memory = GBYTES (C->nzmax, C->type->size) ;
        GB_MATRIX_FREE (&C) ;
        return (GB_OUT_OF_MEMORY (memory)) ;
    }

    //--------------------------------------------------------------------------
    // copy the values from A into C and cast from A->type to C->type
    //--------------------------------------------------------------------------

    GB_cast_array (C->x, C->type->code, A->x, A->type->code, anz) ;

    // C->i always shallow, and is of size at least A->nzmax.  The array C->x
    // is either of size A->nzmax if C->x is and not typecasted, or
    // max(anz,1) otherwise.  Thus, the two arrays C->i and C->x can differ
    // in size if C->x is typecasted.  C->nzmax reflects this, and has been set
    // to the smaller of the two sizes.

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    ASSERT_OK (GB_check (C, "C = shallow cast (A)", GB0)) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

