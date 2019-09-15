//------------------------------------------------------------------------------
// GB_msort_3: sort a 3-by-n list of integers, using A[0:2][ ] as the key
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// A parallel mergesort of an array of 3-by-n integers.  Each key consists
// of three integers.

#include "GB_msort_3.h"

//------------------------------------------------------------------------------
// GB_merge_sequential_3: merge two sorted lists via a single thread
//------------------------------------------------------------------------------

// merge Left [0..nleft-1] and Right [0..nright-1] into S [0..nleft+nright-1] */

static void GB_merge_sequential_3
(
    int64_t *restrict S_0,              // output of length nleft + nright
    int64_t *restrict S_1,
    int64_t *restrict S_2,
    const int64_t *restrict Left_0,     // left input of length nleft
    const int64_t *restrict Left_1,
    const int64_t *restrict Left_2,
    const int64_t nleft,
    const int64_t *restrict Right_0,    // right input of length nright
    const int64_t *restrict Right_1,
    const int64_t *restrict Right_2,
    const int64_t nright
)
{
    int64_t p, pleft, pright ;

    // merge the two inputs, Left and Right, while both inputs exist
    for (p = 0, pleft = 0, pright = 0 ; pleft < nleft && pright < nright ; p++)
    {
        if (GB_lt_3 (Left_0,  Left_1,  Left_2,  pleft,
                     Right_0, Right_1, Right_2, pright))
        { 
            // S [p] = Left [pleft++]
            S_0 [p] = Left_0 [pleft] ;
            S_1 [p] = Left_1 [pleft] ;
            S_2 [p] = Left_2 [pleft] ;
            pleft++ ;
        }
        else
        { 
            // S [p] = Right [pright++]
            S_0 [p] = Right_0 [pright] ;
            S_1 [p] = Right_1 [pright] ;
            S_2 [p] = Right_2 [pright] ;
            pright++ ;
        }
    }

    // either input is exhausted; copy the remaining list into S
    if (pleft < nleft)
    { 
        int64_t nremaining = (nleft - pleft) ;
        memcpy (S_0 + p, Left_0 + pleft, nremaining * sizeof (int64_t)) ;
        memcpy (S_1 + p, Left_1 + pleft, nremaining * sizeof (int64_t)) ;
        memcpy (S_2 + p, Left_2 + pleft, nremaining * sizeof (int64_t)) ;
    }
    else if (pright < nright)
    { 
        int64_t nremaining = (nright - pright) ;
        memcpy (S_0 + p, Right_0 + pright, nremaining * sizeof (int64_t)) ;
        memcpy (S_1 + p, Right_1 + pright, nremaining * sizeof (int64_t)) ;
        memcpy (S_2 + p, Right_2 + pright, nremaining * sizeof (int64_t)) ;
    }
}

//------------------------------------------------------------------------------
// GB_merge_parallel_3: parallel merge
//------------------------------------------------------------------------------

// The two input arrays, Bigger [0..nbigger-1] and Smaller [0..nsmaller-1], are
// sorted.  They are merged into the output array S [0..nleft+nright-1], using
// a parallel merge.  nbigger >= nsmaller always holds.

void GB_merge_parallel_3                // parallel merge
(
    int64_t *restrict S_0,              // output of length nbigger + nsmaller
    int64_t *restrict S_1,
    int64_t *restrict S_2,
    const int64_t *restrict Bigger_0,   // Bigger [0..nbigger-1]
    const int64_t *restrict Bigger_1,
    const int64_t *restrict Bigger_2,
    const int64_t nbigger,
    const int64_t *restrict Smaller_0,  // Smaller [0..nsmaller-1]
    const int64_t *restrict Smaller_1,
    const int64_t *restrict Smaller_2,
    const int64_t nsmaller
)
{

    //--------------------------------------------------------------------------
    // split the bigger input in half
    //--------------------------------------------------------------------------

    // The first task will handle Bigger [0..nhalf-1], and the second task
    // will handle Bigger [nhalf..n-1].

    int64_t nhalf = nbigger/2 ;
    int64_t Pivot_0 [1] ; Pivot_0 [0] = Bigger_0 [nhalf] ;
    int64_t Pivot_1 [1] ; Pivot_1 [0] = Bigger_1 [nhalf] ;
    int64_t Pivot_2 [1] ; Pivot_2 [0] = Bigger_2 [nhalf] ;

    //--------------------------------------------------------------------------
    // find where the Pivot appears in the smaller list
    //--------------------------------------------------------------------------

    // This is like GB_BINARY_TRIM_SEARCH, but applied to a 3-by-n array.

    // binary search of Smaller [0..nsmaller-1] for the Pivot

    long pleft = 0, pright = nsmaller-1 ;
    while (pleft < pright)
    {
        long pmiddle = (pleft + pright) / 2 ;
        if (GB_lt_3 (Smaller_0, Smaller_1, Smaller_2, pmiddle,
                     Pivot_0, Pivot_1, Pivot_2, 0))
        { 
            // if in the list, Pivot appears in [pmiddle+1..pright]
            pleft = pmiddle + 1 ;
        }
        else
        { 
            // if in the list, Pivot appears in [pleft..pmiddle]
            pright = pmiddle ;
        }
    }

    // binary search is narrowed down to a single item
    // or it has found the list is empty:
    ASSERT (pleft == pright || pleft == pright + 1) ;

    // If found is true then Smaller [pleft == pright] == Pivot.  If duplicates
    // appear then Smaller [pleft] is any one of the entries equal to the Pivot
    // in the list.  If found is false then
    //    Smaller [original_pleft ... pleft-1] < Pivot and
    //    Smaller [pleft+1 ... original_pright] > Pivot holds.
    //    The value Smaller [pleft] may be either < or > Pivot.
    bool found = (pleft == pright &&
        Smaller_0 [pleft] == Pivot_0 [0] &&
        Smaller_1 [pleft] == Pivot_1 [0] &&
        Smaller_2 [pleft] == Pivot_2 [0]) ;

    // Modify pleft and pright:
    if (!found && (pleft == pright))
    { 
        if (GB_lt_3 (Smaller_0, Smaller_1, Smaller_2, pleft,
                     Pivot_0,   Pivot_1,   Pivot_2,   0))
        {
            pleft++ ;
        }
        else
        {
            pright++ ;
        }
    }

    // Now the following conditions hold:

    // If found is false then
    //    Smaller [original_pleft ... pleft-1] < Pivot and
    //    Smaller [pleft ... original_pright] > Pivot holds,
    //    and pleft-1 == pright

    // If Smaller has no duplicates, then whether or not Pivot is found,
    //    Smaller [original_pleft ... pleft-1] < Pivot and
    //    Smaller [pleft ... original_pright] >= Pivot holds.

    //--------------------------------------------------------------------------
    // merge each part in parallel
    //--------------------------------------------------------------------------

    // The first task merges Bigger [0..nhalf-1] and Smaller [0..pleft-1] into
    // the output S [0..nhalf+pleft-1].  The entries in Bigger [0..nhalf-1] are
    // all < Pivot (if no duplicates appear in Bigger) or <= Pivot otherwise.

    int64_t *restrict S_task0_0 = S_0 ;
    int64_t *restrict S_task0_1 = S_1 ;
    int64_t *restrict S_task0_2 = S_2 ;

    const int64_t *restrict Left_task0_0 = Bigger_0 ;
    const int64_t *restrict Left_task0_1 = Bigger_1 ;
    const int64_t *restrict Left_task0_2 = Bigger_2 ;
    const int64_t nleft_task0 = nhalf ;

    const int64_t *restrict Right_task0_0 = Smaller_0 ;
    const int64_t *restrict Right_task0_1 = Smaller_1 ;
    const int64_t *restrict Right_task0_2 = Smaller_2 ;
    const int64_t nright_task0 = pleft ;

    // The second task merges Bigger [nhalf..nbigger-1] and
    // Smaller [pleft..nsmaller-1] into the output S [nhalf+pleft..n-1].
    // The entries in Bigger [nhalf..nbigger-1] and Smaller [pleft..nsmaller-1]
    // are all >= Pivot.

    int64_t *restrict S_task1_0 = S_0 + nhalf + pleft ;
    int64_t *restrict S_task1_1 = S_1 + nhalf + pleft ;
    int64_t *restrict S_task1_2 = S_2 + nhalf + pleft ;

    const int64_t *restrict Left_task1_0 = Bigger_0 + nhalf ;
    const int64_t *restrict Left_task1_1 = Bigger_1 + nhalf ;
    const int64_t *restrict Left_task1_2 = Bigger_2 + nhalf ;
    const int64_t nleft_task1 = (nbigger - nhalf) ;

    const int64_t *restrict Right_task1_0 = Smaller_0 + pleft ;
    const int64_t *restrict Right_task1_1 = Smaller_1 + pleft ;
    const int64_t *restrict Right_task1_2 = Smaller_2 + pleft ;
    const int64_t nright_task1 = (nsmaller - pleft) ;

    #pragma omp task firstprivate(S_task0_0, S_task0_1,     \
        Left_task0_0,  Left_task0_1,  nleft_task0,          \
        Right_task0_0, Right_task0_1, nright_task0)
    GB_merge_select_3 (S_task0_0, S_task0_1, S_task0_2,
        Left_task0_0,  Left_task0_1,  Left_task0_2,  nleft_task0,
        Right_task0_0, Right_task0_1, Right_task0_2, nright_task0) ;

    #pragma omp task firstprivate(S_task1_0, S_task1_1,     \
        Left_task1_0,  Left_task1_1,  nleft_task1,          \
        Right_task1_0, Right_task1_1, nright_task1)
    GB_merge_select_3 (S_task1_0, S_task1_1, S_task1_2,
        Left_task1_0,  Left_task1_1,  Left_task1_2,  nleft_task1,
        Right_task1_0, Right_task1_1, Right_task1_2, nright_task1) ;

    #pragma omp taskwait
}

//------------------------------------------------------------------------------
// GB_merge_select_3: parallel or sequential merge
//------------------------------------------------------------------------------

// The two input arrays, Left [0..nleft-1] and Right [0..nright-1], are sorted.
// They are merged into the output array S [0..nleft+nright-1], using either
// the sequential merge (for small lists) or the parallel merge (for big
// lists).

void GB_merge_select_3      // parallel or sequential merge of 3-by-n arrays
(
    int64_t *restrict S_0,              // output of length nleft+nright
    int64_t *restrict S_1,
    int64_t *restrict S_2,
    const int64_t *restrict Left_0,     // Left [0..nleft-1]
    const int64_t *restrict Left_1,
    const int64_t *restrict Left_2,
    const int64_t nleft,
    const int64_t *restrict Right_0,    // Right [0..nright01]
    const int64_t *restrict Right_1,
    const int64_t *restrict Right_2,
    const int64_t nright
)
{

    if (nleft + nright < GB_BASECASE)
    { 
        // sequential merge
        GB_merge_sequential_3 (S_0, S_1, S_2,
            Left_0,  Left_1,  Left_2,  nleft,
            Right_0, Right_1, Right_2, nright) ;
    }
    else if (nleft >= nright)
    { 
        // parallel merge, where Left [0..nleft-1] is the bigger of the two.
        GB_merge_parallel_3 (S_0, S_1, S_2,
            Left_0,  Left_1,  Left_2,  nleft,
            Right_0, Right_1, Right_2, nright) ;
    }
    else
    { 
        // parallel merge, where Right [0..nright-1] is the bigger of the two.
        GB_merge_parallel_3 (S_0, S_1, S_2,
            Right_0, Right_1, Right_2, nright,
            Left_0,  Left_1,  Left_2,  nleft) ;
    }
}

//------------------------------------------------------------------------------
// GB_mergesort_3:  parallel merge sort of a 3-by-n array
//------------------------------------------------------------------------------

// GB_mergesort_3 sorts an int64_t array A of size 3-by-n in ascending
// order, using a parallel mergesort.  W is a workspace array of size 3-by-n.
// Small arrays are sorted with a quicksort method.

void GB_mergesort_3 // sort array A of size 3-by-n, using 3 keys (A [0:1][])
(
    int64_t *restrict A_0,      // size n array
    int64_t *restrict A_1,      // size n array
    int64_t *restrict A_2,      // size n array
    int64_t *restrict W_0,      // size n array, workspace
    int64_t *restrict W_1,      // size n array, workspace
    int64_t *restrict W_2,      // size n array, workspace
    const int64_t n
)
{

    if (n <= GB_BASECASE)
    { 

        // ---------------------------------------------------------------------
        // sequential quicksort; no workspace needed
        // ---------------------------------------------------------------------

        GB_qsort_3 (A_0, A_1, A_2, n) ;

    }
    else
    { 

        // ---------------------------------------------------------------------
        // recursive merge sort if A has length greater than GB_BASECASE
        // ---------------------------------------------------------------------

        // ---------------------------------------------------------------------
        // split A into four quarters
        // ---------------------------------------------------------------------

        int64_t n12 = n / 2 ;           // split n into n12 and n34
        int64_t n34 = n - n12 ;

        int64_t n1 = n12 / 2 ;          // split n12 into n1 and n2
        int64_t n2 = n12 - n1 ;

        int64_t n3 = n34 / 2 ;          // split n34 into n3 and n4
        int64_t n4 = n34 - n3 ;

        int64_t n123 = n12 + n3 ;       // start of 4th quarter = n1 + n2 + n3

        // 1st quarter of A and W
        int64_t *restrict A_1st0 = A_0 ;
        int64_t *restrict A_1st1 = A_1 ;
        int64_t *restrict A_1st2 = A_2 ;

        int64_t *restrict W_1st0 = W_0 ;
        int64_t *restrict W_1st1 = W_1 ;
        int64_t *restrict W_1st2 = W_2 ;

        // 2nd quarter of A and W
        int64_t *restrict A_2nd0 = A_0 + n1 ;
        int64_t *restrict A_2nd1 = A_1 + n1 ;
        int64_t *restrict A_2nd2 = A_2 + n1 ;

        int64_t *restrict W_2nd0 = W_0 + n1 ;
        int64_t *restrict W_2nd1 = W_1 + n1 ;
        int64_t *restrict W_2nd2 = W_2 + n1 ;

        // 3rd quarter of A and W
        int64_t *restrict A_3rd0 = A_0 + n12 ;
        int64_t *restrict A_3rd1 = A_1 + n12 ;
        int64_t *restrict A_3rd2 = A_2 + n12 ;

        int64_t *restrict W_3rd0 = W_0 + n12 ;
        int64_t *restrict W_3rd1 = W_1 + n12 ;
        int64_t *restrict W_3rd2 = W_2 + n12 ;

        // 4th quarter of A and W
        int64_t *restrict A_4th0 = A_0 + n123 ;
        int64_t *restrict A_4th1 = A_1 + n123 ;
        int64_t *restrict A_4th2 = A_2 + n123 ;

        int64_t *restrict W_4th0 = W_0 + n123 ;
        int64_t *restrict W_4th1 = W_1 + n123 ;
        int64_t *restrict W_4th2 = W_2 + n123 ;

        // ---------------------------------------------------------------------
        // sort each quarter of A in parallel, using W as workspace
        // ---------------------------------------------------------------------

        #pragma omp task \
           firstprivate(A_1st0, A_1st1, A_1st2, W_1st0, W_1st1, W_1st2, n1)
        GB_mergesort_3 (A_1st0, A_1st1, A_1st2, W_1st0, W_1st1, W_1st2, n1) ;

        #pragma omp task \
           firstprivate(A_2nd0, A_2nd1, A_2nd2, W_2nd0, W_2nd1, W_2nd2, n2)
        GB_mergesort_3 (A_2nd0, A_2nd1, A_2nd2, W_2nd0, W_2nd1, W_2nd2, n2) ;

        #pragma omp task \
           firstprivate(A_3rd0, A_3rd1, A_3rd2, W_3rd0, W_3rd1, W_3rd2, n3)
        GB_mergesort_3 (A_3rd0, A_3rd1, A_3rd2, W_3rd0, W_3rd1, W_3rd2, n3) ;

        #pragma omp task \
           firstprivate(A_4th0, A_4th1, A_4th2, W_4th0, W_4th1, W_4th2, n4)
        GB_mergesort_3 (A_4th0, A_4th1, A_4th2, W_4th0, W_4th1, W_4th2, n4) ;

        #pragma omp taskwait

        // ---------------------------------------------------------------------
        // merge pairs of quarters of A into two halves of W, in parallel
        // ---------------------------------------------------------------------

        #pragma omp task firstprivate(W_1st0, W_1st1, W_1st2, \
            A_1st0, A_1st1, A_1st2, n1, A_2nd0, A_2nd1, A_2nd2, n2)
        GB_merge_select_3 (W_1st0, W_1st1, W_1st2,
            A_1st0, A_1st1, A_1st2, n1, A_2nd0, A_2nd1, A_2nd2, n2) ;

        #pragma omp task firstprivate(W_3rd0, W_3rd1, W_3rd2, \
            A_3rd0, A_3rd1, A_3rd2, n3, A_4th0, A_4th1, A_4th2, n4)
        GB_merge_select_3 (W_3rd0, W_3rd1, W_3rd2,
            A_3rd0, A_3rd1, A_3rd2, n3, A_4th0, A_4th1, A_4th2, n4) ;

        #pragma omp taskwait

        // ---------------------------------------------------------------------
        // merge the two halves of W into A
        // ---------------------------------------------------------------------

        GB_merge_select_3 (A_0, A_1, A_2, W_1st0, W_1st1, W_1st2, n12,
            W_3rd0, W_3rd1, W_3rd2, n34) ;
    }
}

//------------------------------------------------------------------------------
// GB_msort_3: gateway for parallel merge sort
//------------------------------------------------------------------------------

void GB_msort_3     // sort array A of size 2-by-n, using 2 keys (A [0:1][])
(
    int64_t *restrict A_0,      // size n array
    int64_t *restrict A_1,      // size n array
    int64_t *restrict A_2,      // size n array
    int64_t *restrict W_0,      // size n array, workspace
    int64_t *restrict W_1,      // size n array, workspace
    int64_t *restrict W_2,      // size n array, workspace
    const int64_t n,
    const int nthreads          // # of threads to use
)
{

    if (GB_OPENMP_GET_NUM_THREADS > 1)
    {

        // ---------------------------------------------------------------------
        // parallel mergesort: already in parallel region
        // ---------------------------------------------------------------------

        // GB_msort_3 is already in a parallel region in the caller.  This
        // does not occur inside GraphBLAS, but the user application might be
        // calling GraphBLAS inside its own parallel region.

        GB_mergesort_3 (A_0, A_1, A_2, W_0, W_1, W_2, n) ;

    }
    else
    { 

        // ---------------------------------------------------------------------
        // parallel mergesort: start a parallel region
        // ---------------------------------------------------------------------

        #pragma omp parallel num_threads(nthreads)
        #pragma omp master
        GB_mergesort_3 (A_0, A_1, A_2, W_0, W_1, W_2, n) ;

    }
}

