//------------------------------------------------------------------------------
// GB_I_inverse: invert an index list
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// I is a large list relative to the vector length, avlen, and it is not
// contiguous.  Scatter I into the I inverse buckets (Mark and Inext) for quick
// lookup.

// FUTURE:: this code is sequential.  Constructing the I inverse buckets in
// parallel would require synchronization (a critical section for each bucket,
// or atomics).  A more parallel approach might use qsort first, to find
// duplicates in I, and then construct the buckets in parallel after the qsort.
// But the time complexity would be higher.

#include "GB_subref.h"

GrB_Info GB_I_inverse           // invert the I list for C=A(I,:)
(
    const GrB_Index *I,         // list of indices, duplicates OK
    int64_t nI,                 // length of I
    int64_t avlen,              // length of the vectors of A
    // outputs:
    int64_t *GB_RESTRICT *p_Mark,  // head pointers for buckets, size avlen
    int64_t *GB_RESTRICT *p_Inext, // next pointers for buckets, size nI
    int64_t *p_ndupl,           // number of duplicate entries in I
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    int64_t *Mark = NULL ;
    int64_t *Inext = NULL ;
    int64_t ndupl = 0 ;

    (*p_Mark ) = NULL ;
    (*p_Inext) = NULL ;
    (*p_ndupl) = 0 ;

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    GB_CALLOC_MEMORY (Mark,  avlen, sizeof (int64_t)) ;
    GB_MALLOC_MEMORY (Inext, nI,    sizeof (int64_t)) ;
    if (Inext == NULL || Mark == NULL)
    { 
        // out of memory
        GB_FREE_MEMORY (Mark,  avlen, sizeof (int64_t)) ;
        GB_FREE_MEMORY (Inext, nI,    sizeof (int64_t)) ;
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // scatter the I indices into buckets
    //--------------------------------------------------------------------------

    // at this point, Mark is all zero, so Mark [i] < 1 for all i in
    // the range 0 to avlen-1.

    // O(nI) time; not parallel
    for (int64_t inew = nI-1 ; inew >= 0 ; inew--)
    {
        int64_t i = I [inew] ;
        ASSERT (i >= 0 && i < avlen) ;
        int64_t ihead = (Mark [i] - 1) ;
        if (ihead < 0)
        { 
            // first time i has been seen in the list I
            ihead = -1 ;
        }
        else
        { 
            // i has already been seen in the list I
            ndupl++ ;
        }
        Mark [i] = inew + 1 ;       // (Mark [i] - 1) = inew
        Inext [inew] = ihead ;
    }

    // indices in I are now in buckets.  An index i might appear more than once
    // in the list I.  inew = (Mark [i] - 1) is the first position of i in I (i
    // will be I [inew]), (Mark [i] - 1) is the head of a link list of all
    // places where i appears in I.  inew = Inext [inew] traverses this list,
    // until inew is -1.

    // to traverse all entries in bucket i, do:
    // GB_for_each_index_in_bucket (inew,i)) { ... }

    #define GB_for_each_index_in_bucket(inew,i) \
        for (int64_t inew = Mark[i]-1 ; inew >= 0 ; inew = Inext [inew])

    // If Mark [i] < 1, then the ith bucket is empty and i is not in I.
    // Otherise, the first index in bucket i is (Mark [i] - 1).

    #ifdef GB_DEBUG
    for (int64_t i = 0 ; i < avlen ; i++)
    {
        GB_for_each_index_in_bucket (inew, i)
        {
            ASSERT (inew >= 0 && inew < nI) ;
            ASSERT (i == I [inew]) ;
        }
    }
    #endif

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    (*p_Mark ) = Mark ;
    (*p_Inext) = Inext ;
    (*p_ndupl) = ndupl ;
    return (GrB_SUCCESS) ;
}

