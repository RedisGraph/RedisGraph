//------------------------------------------------------------------------------
// GB_aliased: determine if two matrices are aliased
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Returns true if A == B (and not NULL), or if any component A and B are
// aliased to each other.  In the latter case, that component of A and B will
// always be shallow, in either A or B, or both.  NULL pointers are not
// aliased.

#include "GB.h"

// true if pointers p1 and p2 are aliased and not NULL
#define GB_POINTER_ALIASED(p1,p2) ((p1) == (p2) && (p1) != NULL)

bool GB_aliased             // determine if A and B are aliased
(
    GrB_Matrix A,           // input A matrix
    GrB_Matrix B            // input B matrix
)
{

    //--------------------------------------------------------------------------
    // check the matrices themselves
    //--------------------------------------------------------------------------

    if (A == NULL || B == NULL)
    { 
        // this is not an error condition; NULL pointers are not an alias
        return (false) ;
    }

    if (A == B)
    { 
        return (true) ;
    }

    //--------------------------------------------------------------------------
    // check their content
    //--------------------------------------------------------------------------

    bool aliased = false ;

    if (GB_POINTER_ALIASED (A->h, B->h))
    { 
        ASSERT (A->h_shallow || B->h_shallow) ;
        aliased = true ;
    }

    if (GB_POINTER_ALIASED (A->p, B->p))
    { 
        ASSERT (A->p_shallow || B->p_shallow) ;
        aliased = true ;
    }

    if (GB_POINTER_ALIASED (A->i, B->i))
    { 
        ASSERT (A->i_shallow || B->i_shallow) ;
        aliased = true ;
    }

    if (GB_POINTER_ALIASED (A->x, B->x))
    { 
        ASSERT (A->x_shallow || B->x_shallow) ;
        aliased = true ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    return (aliased) ;
}

