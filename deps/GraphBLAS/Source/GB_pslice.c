//------------------------------------------------------------------------------
// GB_pslice: partition Ap for a parallel loop
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Ap [0..n] is an array with monotonically increasing entries.  This function
// slices Ap so that each chunk has the same number of total values of its
// entries.  Ap can be A->p for a matrix and then n = A->nvec.  Or it can be
// the work needed for computing each vector of a matrix (see GB_ewise_slice
// and GB_subref_slice, for example).

#include "GB.h"

void GB_pslice                      // find how to slice Ap
(
    int64_t *Slice,                 // size ntasks+1
    const int64_t *restrict Ap,     // array of size n+1
    const int64_t n,
    const int ntasks                // # of tasks
)
{

    const double work = (Ap == NULL) ? 0 : Ap [n] ;

    Slice [0] = 0 ;
    if (Ap == NULL || n == 0 || ntasks <= 1 || work == 0)
    {
        // matrix is empty, or a single thread is used
        for (int taskid = 1 ; taskid < ntasks ; taskid++)
        { 
            Slice [taskid] = 0 ;
        }
    }
    else
    {
        // slice Ap by # of entries
        int64_t k = 0 ;
        for (int taskid = 1 ; taskid < ntasks ; taskid++)
        { 
            // binary search to find k so that Ap [k] == (taskid * work) /
            // ntasks.  The exact value will not typically not be found;
            // just pick what the binary search comes up with.
            int64_t wtask = ((taskid * work) / (double) ntasks) ;
            int64_t pright = n ;
            GB_BINARY_TRIM_SEARCH (wtask, Ap, k, pright) ;
            Slice [taskid] = k ;
        }
    }
    Slice [ntasks] = n ;
}

