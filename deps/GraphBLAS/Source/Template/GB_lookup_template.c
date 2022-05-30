//------------------------------------------------------------------------------
// GB_lookup_template: find k so that j == Ah [k]
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// For a sparse, bitmap, or full matrix j == k.
// For a hypersparse matrix, find k so that j == Ah [k], if it
// appears in the list.

// k is not needed by the caller, just pstart, pend, pleft, and found.

// Once k is found, find pstart and pend, the start and end of the vector.
// pstart and pend are defined for all sparsity structures: hypersparse,
// sparse, bitmap, or full.

// This fine is #included' by GB.h, so the #include'ing file does either:
//      #include "GB.h"
// or
//      #define GB_CUDA_KERNEL
//      #include "GB.h"

#ifdef GB_CUDA_KERNEL
__device__
static inline bool GB_lookup_device // FIXME for CUDA: use name "GB_lookup"
#else
static inline bool GB_lookup        // find j = Ah [k] in a hyperlist
#endif
(
    const bool A_is_hyper,          // true if A is hypersparse
    const int64_t *restrict Ah,  // A->h [0..A->nvec-1]: list of vectors
    const int64_t *restrict Ap,  // A->p [0..A->nvec  ]: pointers to vectors
    const int64_t avlen,            // A->vlen
    int64_t *restrict pleft,     // look only in A->h [pleft..pright]
    int64_t pright,                 // normally A->nvec-1, but can be trimmed
//  const int64_t nvec,             // A->nvec: number of vectors
    const int64_t j,                // vector to find, as j = Ah [k]
    int64_t *restrict pstart,    // start of vector: Ap [k]
    int64_t *restrict pend       // end of vector: Ap [k+1]
)
{
    if (A_is_hyper)
    {
        // to search the whole Ah list, use on input:
        // pleft = 0 ; pright = nvec-1 ;
        bool found ;
        GB_BINARY_SEARCH (j, Ah, (*pleft), pright, found) ;
        if (found)
        { 
            // j appears in the hyperlist at Ah [pleft]
            // k = (*pleft)
            (*pstart) = Ap [(*pleft)] ;
            (*pend)   = Ap [(*pleft)+1] ;
        }
        else
        { 
            // j does not appear in the hyperlist Ah
            // k = -1
            (*pstart) = -1 ;
            (*pend)   = -1 ;
        }
        return (found) ;
    }
    else
    { 
        // A is sparse, bitmap, or full; j always appears
        // k = j
        (*pstart) = GBP (Ap, j, avlen) ;
        (*pend)   = GBP (Ap, j+1, avlen) ;
        return (true) ;
    }
}

