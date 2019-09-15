//------------------------------------------------------------------------------
// GB_jappend.h: definitions of GB_jstartup, GB_jappend, and GB_jwrapup
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_JAPPEND_H
#define GB_JAPPEND_H
#include "GB.h"

//------------------------------------------------------------------------------
// GB_jstartup:  start the formation of a matrix
//------------------------------------------------------------------------------

// GB_jstartup is used with GB_jappend and GB_jwrapup to create the
// hyperlist and vector pointers of a new matrix, one at a time.

// GB_jstartup logs the start of C(:,0); it also acts as if it logs the end of
// the sentinal vector C(:,-1).

static inline void GB_jstartup
(
    GrB_Matrix C,           // matrix to start creating
    int64_t *jlast,         // last vector appended, set to -1
    int64_t *cnz,           // set to zero
    int64_t *cnz_last       // set to zero
)
{
    C->p [0] = 0 ;          // log the start of C(:,0)
    (*cnz) = 0 ;            //
    (*cnz_last) = 0 ;
    (*jlast) = -1 ;         // last sentinal vector is -1
    if (C->is_hyper)
    { 
        C->nvec = 0 ;       // clear all existing vectors from C
    }
    C->nvec_nonempty = 0 ;  // # of non-empty vectors will be counted
}

//------------------------------------------------------------------------------
// GB_jappend:  append a new vector to the end of a matrix
//------------------------------------------------------------------------------

// Append a new vector to the end of a matrix C.

// If C->is_hyper is true, C is in hypersparse form with
// C->nvec <= C->plen <= C->vdim.  C->h has size C->plen.
// If C->is_hyper is false, C is in non-hypersparse form with
// C->nvec == C->plen == C->vdim.  C->h is NULL.
// In both cases, C->p has size C->plen+1.

// For both hypersparse and non-hypersparse, C->nvec_nonemty <= C->nvec
// is the number of vectors with at least one entry.

static inline GrB_Info GB_jappend
(
    GrB_Matrix C,           // matrix to append vector j to
    int64_t j,              // new vector to append
    int64_t *jlast,         // last vector appended, -1 if none
    int64_t cnz,            // nnz(C) after adding this vector j
    int64_t *cnz_last,      // nnz(C) before adding this vector j
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (C != NULL) ;
    ASSERT (!C->p_shallow) ;
    ASSERT (!C->h_shallow) ;
    ASSERT (C->p != NULL) ;

    if (cnz <= (*cnz_last))
    { 
        // nothing to do
        return (GrB_SUCCESS) ;
    }

    // one more non-empty vector
    C->nvec_nonempty++ ;

    if (C->is_hyper)
    { 

        //----------------------------------------------------------------------
        // C is hypersparse; make sure space exists in the hyperlist
        //----------------------------------------------------------------------

        ASSERT (C->p [C->nvec] == (*cnz_last)) ;
        ASSERT (C->h != NULL) ;

        // check if space exists
        if (C->nvec == C->plen)
        { 
            // double the size of C->h and C->p
            GrB_Info info ;
            info = GB_hyper_realloc (C, GB_IMIN (C->vdim, 2*(C->plen+1)),
                Context) ;
            if (info != GrB_SUCCESS)
            { 
                // out of memory
                return (info) ;
            }
        }

        ASSERT (C->nvec >= 0) ;
        ASSERT (C->nvec < C->plen) ;
        ASSERT (C->plen <= C->vdim) ;
        ASSERT (C->p [C->nvec] == (*cnz_last)) ;

        C->h [C->nvec] = j ;            // add j to the hyperlist
        C->p [C->nvec+1] = cnz ;        // mark the end of C(:,j)
        C->nvec++ ;                     // one more vector in the hyperlist

    }
    else
    {

        //----------------------------------------------------------------------
        // C is non-hypersparse
        //----------------------------------------------------------------------

        int64_t *restrict Cp = C->p ;

        ASSERT (C->nvec == C->plen && C->plen == C->vdim) ;
        ASSERT (C->h == NULL) ;
        ASSERT (Cp [(*jlast)+1] == (*cnz_last)) ;

        // Even if C is non-hypersparse, the iteration that uses this function
        // may iterate over a hypersparse input matrix, so not every vector j
        // will be traversed.  So when j is seen, the end of vectors jlast+1 to
        // j must logged in Cp.

        for (int64_t jprior = (*jlast)+1 ; jprior < j ; jprior++)
        { 
            Cp [jprior+1] = (*cnz_last) ;   // mark the end of C(:,jprior)
        }
        Cp [j+1] = cnz ;                    // mark the end of C(:,j)
    }

    // record the last vector added to C
    (*cnz_last) = cnz ;
    (*jlast) = j ;

    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// GB_jwrapup:  finish contructing a new matrix
//------------------------------------------------------------------------------

// Log the end of any vectors in C that are not yet terminated.  Nothing
// happens if C is hypersparse (except for setting C->magic).

static inline void GB_jwrapup
(
    GrB_Matrix C,           // matrix to finish
    int64_t jlast,          // last vector appended, -1 if none
    int64_t cnz             // final nnz(C)
)
{

    if (!C->is_hyper)
    {

        //----------------------------------------------------------------------
        // C is non-hypersparse
        //----------------------------------------------------------------------

        // log the end of C(:,jlast+1) to C(:,n-1), in case the last vector
        // j=n-1 has not yet been seen, or has been seen but was empty.

        int64_t *restrict Cp = C->p ;
        int64_t j = C->vdim - 1 ;

        for (int64_t jprior = jlast+1 ; jprior <= j ; jprior++)
        { 
            Cp [jprior+1] = cnz ;           // mark the end of C(:,jprior)
        }
    }

    // C->p and C->h are now valid
    C->magic = GB_MAGIC ;
}

#endif

