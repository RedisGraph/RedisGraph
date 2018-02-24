//------------------------------------------------------------------------------
// GB_builder: build a matrix from tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The input arguments &iwork and &jwork are always freed by this function.

#include "GB.h"

GrB_Info GB_builder
(
    GrB_Matrix C,                   // matrix to build
    int64_t **iwork_handle,         // for (i,k) or (j,i,k) tuples
    int64_t **jwork_handle,         // for (j,i,k) tuples
    const bool already_sorted,      // true if tuples already sorted on input
    const void *X,                  // array of values of tuples
    const int64_t len,              // number of tuples
    const int64_t ijlen,            // size of i,j work arrays
    const GrB_BinaryOp dup,         // binary function to assemble duplicates,
                                    // if NULL use the "SECOND" function to 
                                    // keep the most recent duplicate.
    const GB_Type_code X_code       // GB_Type_code of X array
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // X, X_code, and dup are not used here but simply passed down to
    // GB_build_factory, which does the numerical work.

    // C->p may not be initialized and C->magic may be zero, but C is not NULL
    ASSERT (C != NULL && C->p != NULL) ;
    ASSERT (X != NULL) ;
    ASSERT (len >= 0) ;
    ASSERT (X_code <= GB_UDT_code) ;

    ASSERT (iwork_handle != NULL) ;
    ASSERT (jwork_handle != NULL) ;
    int64_t *iwork = *iwork_handle ;
    int64_t *jwork = *jwork_handle ;
    ASSERT (iwork != NULL) ;
    int64_t ncols = C->ncols ;
    if (ncols > 1)
    {
        // If C has more than one column, jwork must be present on input
        ASSERT (jwork != NULL) ;
    }

    // When this function returns, iwork and jwork are freed, and the iwork and
    // jwork pointers (in the caller) are set to NULL by setting their handles
    // to NULL.  Note that jwork may already be NULL on input, if C has 
    // one or zero columns (jwork_handle is always non-NULL however).

    // free the existing content, if any, but leave C->p unchanged
    GB_Matrix_ixfree (C) ;
    ASSERT (C->i == NULL && C->x == NULL) ;
    ASSERT (!PENDING (C)) ; ASSERT (!ZOMBIES (C)) ;

    //--------------------------------------------------------------------------
    // sort the tuples in ascending order (just the pattern, not the values)
    //--------------------------------------------------------------------------

    int64_t *kwork = NULL ;

    if (!already_sorted)
    {

        // create the k part of each tuple
        GB_MALLOC_MEMORY (kwork, len, sizeof (int64_t)) ;
        if (kwork == NULL)
        {
            GB_FREE_MEMORY (*iwork_handle, ijlen, sizeof (int64_t)) ;
            GB_FREE_MEMORY (*jwork_handle, ijlen, sizeof (int64_t)) ;
            GB_Matrix_clear (C) ;
            return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
                "out of memory, %g GBytes required", 
                GBYTES (len, sizeof (int64_t))))) ;
        }

        // The k part of each tuple (i,k) or (j,i,k) records the original
        // position of the tuple in the input list.  This allows sort to be
        // unstable; since k is unique, it forces the result of the sort to be
        // stable regardless of whether or not the sorting algorithm is stable.
        // It also keeps track of where the numerical value of the tuple can be
        // found; it is in X[k] for the tuple (i,k) or (j,i,k), regardless of
        // where the tuple appears in the list after it is sorted.
        for (int64_t k = 0 ; k < len ; k++)
        {
            kwork [k] = k ;
        }

        if (ncols > 1)
        {
            // sort a set of (j,i,k) tuples
            ASSERT (jwork != NULL) ;
            GB_qsort_3 (jwork, iwork, kwork, len) ;
        }
        else
        {
            // sort a set of (i,k) tuples
            GB_qsort_2b (iwork, kwork, len) ;
        }

    }
    else
    {
        // If the tuples were already known to be sorted on input, kwork is
        // NULL.  This implicitly means that kwork [k] = k for all k = 0:len-1.
        ;
    }

    //--------------------------------------------------------------------------
    // find the size of the result, mark duplicates, and build C->p
    //--------------------------------------------------------------------------

    int64_t cnz = 0 ;
    int64_t *Cp = C->p ;

    // log the start of the first column
    Cp [0] = 0 ;

    if (ncols > 1)
    {

        //----------------------------------------------------------------------
        // C has more than one column
        //----------------------------------------------------------------------

        int64_t ilast = -1 ;
        int64_t jlast = -1 ;

        for (int64_t t = 0 ; t < len ; t++)
        {
            // get the t-th tuple.  No need to look up position k in kwork [t]
            int64_t i = iwork [t] ;

            // check if (j,i,k) is a duplicate
            int64_t j = jwork [t] ;
            bool is_duplicate = (i == ilast && j == jlast) ;

            // tuples are now sorted but there may be duplicates
            ASSERT ((jlast < j) || (jlast == j && ilast <= i)) ;

            if (is_duplicate)
            {
                // tuple is a duplicate of tuple just before it in the sorted
                // list.  mark the tuple as a duplicate by setting the row
                // index to -1
                iwork [t] = -1 ;

                #ifndef NDEBUG
                // sort places older tuples (with smaller k) after older ones
                int64_t klast = (t==0) ? -1 : 
                                        ((kwork == NULL) ? t-1 : kwork [t-1]) ;
                int64_t kthis = (kwork == NULL) ? t : kwork [t] ;
                ASSERT (klast < kthis) ;
                #endif
            }
            else
            {
                // this is a new tuple
                if (j > jlast)
                {
                    // this is also the first entry in column j
                    for (int64_t jj = jlast+1 ; jj <= j ; jj++)
                    {
                        // log the start of column jj
                        Cp [jj] = cnz ;
                    }
                }
                cnz++ ;

                // log the last non-duplicate tuple seen
                ilast = i ;
                jlast = j ;
            }
        }

        // log the start of columns not yet seen, if any,
        // and the end of the last column
        for (int64_t jj = jlast+1 ; jj <= ncols ; jj++)
        {
            Cp [jj] = cnz ;
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // C has one column (or even zero columns)
        //----------------------------------------------------------------------

        int64_t ilast = -1 ;

        for (int64_t t = 0 ; t < len ; t++)
        {
            // get the t-th tuple.  No need to look up position k in kwork [t]
            int64_t i = iwork [t] ;

            // check if (i,k) or (j,i,k) is a duplicate
            bool is_duplicate = (i == ilast) ;

            // tuples are now sorted but there may be duplicates
            ASSERT (ilast <= i) ;

            if (is_duplicate)
            {
                // tuple is a duplicate of tuple just before it in the sorted
                // list.  mark the tuple as a duplicate by setting the row
                // index to -1
                iwork [t] = -1 ;

                #ifndef NDEBUG
                // sort places older tuples (with smaller k) after older ones
                int64_t klast = (t==0) ? -1 :
                                        ((kwork == NULL) ? t-1 : kwork [t-1]) ;
                int64_t kthis = (kwork == NULL) ? t : kwork [t] ;
                ASSERT (klast < kthis) ;
                #endif
            }
            else
            {
                // this is a new tuple
                cnz++ ;
                // log the last non-duplicate tuple seen
                ilast = i ;
            }
        }

        // log the end of the last column
        Cp [ncols] = cnz ;
    }

    // Duplicates do not appear in C
    ASSERT (cnz <= len) ;
    // int64_t nduplicates = len - cnz ;    // if needed for reporting

    C->nzmax = IMAX (cnz, 1) ;
    C->magic = MAGIC ;                      // C->p is now valid

    //--------------------------------------------------------------------------
    // free the column indices, jwork
    //--------------------------------------------------------------------------

    // jwork is no longer needed and can be freed.  This is a very important
    // step because for all built-in types, jwork is at least as big as C->x,
    // which has not yet been allocated.  jwork is as big as the number of
    // tuples (len, or nvals), whereas C->x has size cnz = NNZ (C).  Thus, if
    // the matrix is really big the malloc/free memory manager should be able
    // to allocate C->x in place of the freed jwork array as a cheap malloc.
    // This is design, and it helps to speed up the build process.

    // During testing on a Macbook Pro with OS X 10.11.6, and clang 8.0.0
    // (Xcode 8.2.1), it was observed that the jwork and C->x pointers were
    // typically identical for large problems (for C->x double precision, where
    // sizeof (double) == sizeof (int64_t) = 8).  Thus, malloc is detecting
    // this condition and exploiting it.

    // jwork may already be NULL.  It is only required when C has more than
    // one column.  But the jwork_handle itself is always non-NULL.

    ASSERT (jwork_handle != NULL) ;
    GB_FREE_MEMORY (*jwork_handle, ijlen, sizeof (int64_t)) ;

    //--------------------------------------------------------------------------
    // numerical phase of the build via switch factory or generic workers
    //--------------------------------------------------------------------------

    // kwork is freed when GB_worker_build returns.  X has not yet been
    // accessed by this function.  All the work up until now has been symbolic,
    // with no processing of the numerical values.  That step is saved for
    // GB_build_factory.

    // If this method were to be split into user-callable symbolic analysis and
    // numerical phases, the split is at this point.  The symbolic phase must
    // compute the int64_t arrays iwork and kwork, each of size len, from the
    // input tuples in I and J, also of size len (len == nvals), and it must
    // also construct the column pointers C->p.

    return (GB_build_factory (C, iwork_handle, &kwork, X, len, ijlen, dup, X_code)) ;
}

