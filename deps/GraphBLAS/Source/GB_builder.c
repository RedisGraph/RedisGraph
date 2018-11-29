//------------------------------------------------------------------------------
// GB_builder: build a matrix from tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The input arguments &iwork and &jwork are always freed by this function.
// This function is agnostic regarding the CSR/CSC format.  It decides whether
// T is standard sparse or hypersparse, as determined by the default rules
// for GrB_Matrix_new.

// This function is called by GB_build to build a matrix T for GrB_Matrix_build
// or GrB_Vector_build, and by GB_wait to build a matrix T from the list of
// pending tuples.

#include "GB.h"

GrB_Info GB_builder
(
    GrB_Matrix *Thandle,            // matrix T to build
    const GrB_Type ttype,           // type of output matrix T
    const int64_t vlen,             // length of each vector of T
    const int64_t vdim,             // number of vectors in T
    const bool is_csc,              // true if T is CSC, false if CSR
    int64_t **iwork_handle,         // for (i,k) or (j,i,k) tuples
    int64_t **jwork_handle,         // for (j,i,k) tuples
    const bool already_sorted,      // true if tuples already sorted on input
    const void *S,                  // array of values of tuples
    const int64_t len,              // number of tuples, and size of kwork
    const int64_t ijlen,            // size of iwork and jwork arrays
    const GrB_BinaryOp dup,         // binary function to assemble duplicates,
                                    // if NULL use the "SECOND" function to
                                    // keep the most recent duplicate.
    const GB_Type_code scode,       // GB_Type_code of S array
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // S, scode, and dup are not used here but simply passed down to
    // GB_build_factory, which does the numerical work.

    ASSERT (Thandle != NULL) ;
    (*Thandle) = NULL ;
    ASSERT (GB_IMPLIES (len > 0, S != NULL)) ;
    ASSERT (len >= 0) ;
    ASSERT (scode <= GB_UDT_code) ;

    ASSERT (iwork_handle != NULL) ;
    ASSERT (jwork_handle != NULL) ;
    int64_t *restrict iwork = *iwork_handle ;
    int64_t *restrict jwork = *jwork_handle ;
    ASSERT (iwork != NULL) ;

    // If T has more than one vector, jwork must be present on input
    ASSERT (GB_IMPLIES (vdim > 1, jwork != NULL)) ;

    ASSERT_OK (GB_check (ttype, "ttype for builder", GB0)) ;
    ASSERT_OK_OR_NULL (GB_check (dup, "dup for builder", GB0)) ;

    // When this function returns, iwork and jwork are freed, and the iwork and
    // jwork pointers (in the caller) are set to NULL by setting their handles
    // to NULL.  Note that jwork may already be NULL on input, if T has
    // one or zero vectors (jwork_handle is always non-NULL however).

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
            // out of memory
            GB_FREE_MEMORY (*iwork_handle, ijlen, sizeof (int64_t)) ;
            GB_FREE_MEMORY (*jwork_handle, ijlen, sizeof (int64_t)) ;
            return (GB_OUT_OF_MEMORY (GBYTES (len, sizeof (int64_t)))) ;
        }

        // The k part of each tuple (i,k) or (j,i,k) records the original
        // position of the tuple in the input list.  This allows sort to be
        // unstable; since k is unique, it forces the result of the sort to be
        // stable regardless of whether or not the sorting algorithm is stable.
        // It also keeps track of where the numerical value of the tuple can be
        // found; it is in S[k] for the tuple (i,k) or (j,i,k), regardless of
        // where the tuple appears in the list after it is sorted.
        for (int64_t k = 0 ; k < len ; k++)
        { 
            kwork [k] = k ;
        }

        // This work takes O(t log t) time if t=len is the number of tuples.
        if (jwork != NULL)
        { 
            // sort a set of (j,i,k) tuples
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
        // kwork is not allocated.
        ;
    }

    //--------------------------------------------------------------------------
    // find the size of the result, mark duplicates, and count non-empty vectors
    //--------------------------------------------------------------------------

    int64_t tnz = 0 ;
    int64_t tnvec = 0 ;
    int64_t ilast = -1 ;
    int64_t jlast = -1 ;

    for (int64_t t = 0 ; t < len ; t++)
    {
        // get the t-th tuple.  No need to look up position k in kwork [t]
        int64_t i = iwork [t] ;
        int64_t j = (jwork == NULL) ? 0 : jwork [t] ;

        // check if (j,i,k) is a duplicate
        bool is_duplicate = (i == ilast && j == jlast) ;

        // tuples are now sorted but there may be duplicates
        ASSERT ((jlast < j) || (jlast == j && ilast <= i)) ;

        if (is_duplicate)
        { 
            // tuple is a duplicate of tuple just before it in the sorted list.
            // mark the tuple as a duplicate by setting the index to -1
            iwork [t] = -1 ;

            #ifndef NDEBUG
            // sort places older tuples (with smaller k) after older ones
            int64_t klast = (t==0) ?
                                -1 : ((kwork == NULL) ? t-1 : kwork [t-1]) ;
            int64_t kthis = (kwork == NULL) ? t : kwork [t] ;
            ASSERT (klast < kthis) ;
            #endif
        }
        else
        {
            // this is a new tuple
            if (j > jlast)
            { 
                tnvec++ ;       // this is also the first entry in vector j
                jlast = j ;     // log the new unique vector j
            }
            tnz++ ;
            ilast = i ;         // log the last non-duplicate tuple seen
        }
    }

    //--------------------------------------------------------------------------
    // allocate T; always hypersparse
    //--------------------------------------------------------------------------

    // [ allocate T; malloc T->p and T->h but do not initialize them
    // T is always hypersparse.
    GrB_Info info ;
    GrB_Matrix T = NULL ;           // allocate a new header for T
    GB_NEW (&T, ttype, vlen, vdim, GB_Ap_malloc, is_csc, GB_FORCE_HYPER,
        GB_ALWAYS_HYPER, tnvec) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE_MEMORY (*iwork_handle, ijlen, sizeof (int64_t)) ;
        GB_FREE_MEMORY (*jwork_handle, ijlen, sizeof (int64_t)) ;
        GB_FREE_MEMORY (kwork,         len,   sizeof (int64_t)) ;
        return (info) ;
    }

    // bool T_is_hyper = T->is_hyper ;
    ASSERT (T->is_hyper) ;

    (*Thandle) = T ;
    ASSERT (T->nzmax == 0) ;        // T->i and T->x not yet allocated

    //--------------------------------------------------------------------------
    // construct the vector pointers and hyperlist for T
    //--------------------------------------------------------------------------

    // This phase takes O(t) time if T is hypersparse, or O(n+t) if T is
    // non-hypersparse (but the non-hypersparse code is commented out).

    int64_t *restrict Th = T->h ;
    int64_t *restrict Tp = T->p ;

    // like GB_jstart, except Tp [0] = 0 is done later, and tnz_last not needed
    tnz = 0 ;
    tnvec = 0 ;
    jlast = -1 ;

    for (int64_t t = 0 ; t < len ; t++)
    {
        // get the t-th tuple.  No need to look up position k in kwork [t]
        int64_t i = iwork [t] ;
        int64_t j = (jwork == NULL) ? 0 : jwork [t] ;

        // check if (j,i,k) is a duplicate
        bool is_duplicate = (i == -1) ;

        if (!is_duplicate)
        {
            // this is a new tuple
            if (j > jlast)
            { 
                // First entry in vector j.  Log the start of T(:,j).
                // GB_jappend is not used since that logs the end of T(:,j).
                // if (T_is_hyper)
                {
                    // T is hypersparse; just log this vector j.
                    ASSERT (tnvec < T->plen) ;
                    Th [tnvec] = j ;
                    Tp [tnvec] = tnz ;
                }
                #if 0
                else
                {
                    // T is non-hypersparse.  Finish vectors jlast+1 to j.
                    // This adds O(n) time to the work required to build T.
                    for (int64_t jprior = jlast+1 ; jprior <= j ; jprior++)
                    {
                        Tp [jprior] = tnz ;
                    }
                }
                #endif
                tnvec++ ;       // one more non-empty vector in T
                jlast = j ;
            }
            tnz++ ;
        }
    }

    T->nvec_nonempty = tnvec ;

    // log the end of the last column, like GB_jwrapup
    // if (T_is_hyper)
    { 
        Tp [tnvec] = tnz ;
        T->nvec = tnvec ;
        ASSERT (T->nvec == T->plen) ;
    }
    #if 0
    else
    {
        // T is non-hypersparse.  Finish vectors jlast+1 to n-1.
        // This adds O(n) time to the work required to build T.
        for (int64_t jprior = jlast+1 ; jprior <= vdim ; jprior++)
        {
            Tp [jprior] = tnz ;
        }
    }
    #endif

    T->magic = GB_MAGIC ;                      // T->p and T->h are now valid ]

    // all duplicates have been removed from T
    // int64_t nduplicates = len - tnz ;    // if needed for reporting
    ASSERT (tnz <= len && tnz >= 0) ;

    //--------------------------------------------------------------------------
    // free the vector indices, jwork
    //--------------------------------------------------------------------------

    // jwork is no longer needed and can be freed.  This is a very important
    // step because for all built-in types, jwork is at least as big as T->x,
    // which has not yet been allocated.  jwork is as big as the number of
    // tuples (len, or nvals), whereas T->x will have size tnz.  Thus, if the
    // matrix is really big the malloc/free memory manager should be able to
    // allocate T->x in place of the freed jwork array as a cheap malloc.  This
    // is design, and it helps to speed up the build process.

    // During testing on a Macbook Pro and clang 8.0.0 it was observed that the
    // jwork and T->x pointers were typically identical for large problems (for
    // T->x double precision, where sizeof (double) == sizeof (int64_t) = 8).
    // Thus, malloc is detecting this condition and exploiting it.

    // jwork may already be NULL.  It is only required when T has more than
    // one vector.  But the jwork_handle itself is always non-NULL.

    ASSERT (jwork_handle != NULL) ;
    GB_FREE_MEMORY (*jwork_handle, ijlen, sizeof (int64_t)) ;

    //--------------------------------------------------------------------------
    // numerical phase of the build via switch factory or generic workers
    //--------------------------------------------------------------------------

    // kwork is freed when GB_build_factory returns.  S has not yet been
    // accessed by this function.  All the work up until now has been symbolic,
    // with no processing of the numerical values.  That step is saved for
    // GB_build_factory.

    return (GB_build_factory (Thandle, tnz, iwork_handle, &kwork,
        S, len, ijlen, dup, scode, Context)) ;
}

