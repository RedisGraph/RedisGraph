//------------------------------------------------------------------------------
// GxB_Matrix_Iterator: seek to a specific entry for a matrix iterator 
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "GB.h"
#include "GB_search_for_vector_template.c"

//------------------------------------------------------------------------------
// GB_full_position: find the vector containing p for a full/bitmap matrix
//------------------------------------------------------------------------------

static inline GrB_Info GB_full_position (GxB_Iterator iterator)
{ 
    iterator->k = iterator->p / iterator->avlen ;
    iterator->pstart = iterator->k * iterator->avlen ;
    iterator->pend = iterator->pstart + iterator->avlen ;
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// GB_check_for_end_of_vector: move to next vector if current vector is done
//------------------------------------------------------------------------------

static inline GrB_Info GB_check_for_end_of_vector (GxB_Iterator iterator)
{
    // move to the next vector if p has reached the end of the current vector 
    switch (iterator->A_sparsity)
    {
        default: 
        case GxB_SPARSE : 
        case GxB_HYPERSPARSE : 
        {
            if (iterator->p >= iterator->pend)
            {
                // the kth vector is done; move to the next non-empty vector
                iterator->pstart = iterator->pend ;
                iterator->k++ ;
                while (iterator->Ap [iterator->k+1] == iterator->pend)
                { 
                    // iterator->k is an empty vector; move to the next one
                    iterator->k++ ;
                    ASSERT (iterator->k < iterator->anvec) ;
                }
                // iterator->k is now the next non-empty vector
                iterator->pend = iterator->Ap [iterator->k+1] ;
                return (GrB_SUCCESS) ;
            }
        }
        break ;

        case GxB_BITMAP : 
        {
            for ( ; iterator->p < iterator->pmax ; iterator->p++)
            {
                if (iterator->Ab [iterator->p])
                {
                    // found the next entry; check if it is past the kth vector
                    if (iterator->p >= iterator->pend)
                    { 
                        // find the vector of this entry
                        return (GB_full_position (iterator)) ;
                    }
                    return (GrB_SUCCESS) ;
                }
            }
            return (GxB_EXHAUSTED) ;
        }
        break ;

        case GxB_FULL : 
        {
            if (iterator->p >= iterator->pend)
            { 
                // kth vector is done; move to the next vector
                iterator->k++ ;
                iterator->pstart += iterator->avlen ;
                iterator->pend += iterator->avlen ;
            }
        }
        break ;
    }

    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// GxB_Matrix_Iterator_next: move an entry iterator to the next entry
//------------------------------------------------------------------------------

GrB_Info GxB_Matrix_Iterator_next (GxB_Iterator iterator)
{
    // move to the next entry
    if (++iterator->p >= iterator->pmax)
    { 
        // the iterator is exhausted
        iterator->p = iterator->pmax ;
        return (GxB_EXHAUSTED) ;
    }
    else
    { 
        // move to next vector if iterator has reached the end of a vector
        return (GB_check_for_end_of_vector (iterator)) ;
    }
}

//------------------------------------------------------------------------------
// GxB_Matrix_Iterator_seek: seek an entry iterator to any entry
//------------------------------------------------------------------------------

GrB_Info GxB_Matrix_Iterator_seek
(
    GxB_Iterator iterator,
    GrB_Index p
)
{
    if (p >= iterator->pmax)
    { 
        // the iterator is exhausted
        iterator->p = iterator->pmax ;
        return (GxB_EXHAUSTED) ;
    }
    else if (p == 0)
    { 
        // seek to the first entry of the first vector A(:,0)
        iterator->pstart = 0 ;
        iterator->pend = (iterator->Ap != NULL) ?
            iterator->Ap [1] : iterator->avlen ;
        iterator->p = 0 ;
        iterator->k = 0 ;
        // move to the next non-empty vector if A(:,0) is empty
        return (GB_check_for_end_of_vector (iterator)) ;
    }
    else
    {
        // seek to an arbitrary position in the matrix
        iterator->p = p ;
        switch (iterator->A_sparsity)
        {
            default: 
            case GxB_SPARSE : 
            case GxB_HYPERSPARSE : 
            { 
                // find the vector k that contains position p
                iterator->k = GB_search_for_vector (p, iterator->Ap,
                    0, iterator->anvec, iterator->avlen) ;
                iterator->pstart = iterator->Ap [iterator->k] ;
                iterator->pend = iterator->Ap [iterator->k+1] ;
            }
            break ;
            case GxB_BITMAP : 
            {
                for ( ; iterator->p < iterator->pmax ; iterator->p++)
                {
                    if (iterator->Ab [iterator->p])
                    { 
                        // found next entry; find the vector that contains it
                        return (GB_full_position (iterator)) ;
                    }
                }
                return (GxB_EXHAUSTED) ;
            }
            break ;
            case GxB_FULL : 
            { 
                // find the vector k that contains position p
                return (GB_full_position (iterator)) ;
            }
            break ;
        }
    }
    return (GrB_SUCCESS) ;
}

