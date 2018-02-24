//------------------------------------------------------------------------------
// GB_object_check: print a GraphBLAS matrix and check if it is valid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GB_object_check    // check a GraphBLAS matrix
(
    const GrB_Matrix A,     // GraphBLAS matrix to print and check
    const char *name,       // name of the matrix, optional
    const GB_diagnostic pr, // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
    const char *kind
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (pr > 0) printf ("\nGraphBLAS %s: %s ", kind, NAME) ;

    if (A == NULL)
    {
        // GrB_error status not modified since this may be an optional argument
        if (pr > 0) printf ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check the object
    //--------------------------------------------------------------------------

    CHECK_MAGIC (A, kind) ;

    if (pr > 0) printf ("nrows: "GBd" ncols: "GBd" max # entries: "GBd"\n",
        A->nrows, A->ncols, A->nzmax) ;

    if (A->nrows < 0 || A->nrows > GB_INDEX_MAX ||
        A->ncols < 0 || A->ncols > GB_INDEX_MAX ||
        A->nzmax < 0 || A->nzmax > GB_INDEX_MAX)
    {
        if (pr > 0) printf ("invalid %s dimensions\n", kind) ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "%s invalid : nrows, ncols, or nzmax out of range: [%s]",
            kind, NAME))) ;
    }

    // a matrix contains 1 to 7 different malloc'd blocks
    int64_t nallocs = 1 +                       // header
        (A->p != NULL && !A->p_shallow) +       // A->p, if not shallow
        (A->i != NULL && !A->i_shallow) +       // A->i, if not shallow
        (A->x != NULL && !A->x_shallow) +       // A->x, if not shallow
        (A->ipending != NULL) +                 // A->ipending if tuples
        (A->jpending != NULL) +                 // A->jpending if tuples
        (A->xpending != NULL) ;                 // A->xpending if tuples

    #ifdef DEVELOPER
    if (pr > 0) printf ("number of memory blocks: "GBd"\n", nallocs) ;
    #endif

    GrB_Info info = GB_check (A->type, "", pr) ;
    if (info != GrB_SUCCESS)
    {
        if (pr > 0) printf ("%s has an invalid type\n", kind) ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "%s has an invalid type: [%s]", kind, NAME))) ;
    }

    #ifdef DEVELOPER
    if (pr > 0) printf ("->p: %p shallow: %d\n", A->p, A->p_shallow) ;
    if (pr > 0) printf ("->i: %p shallow: %d\n", A->i, A->i_shallow) ;
    if (pr > 0) printf ("->x: %p shallow: %d\n", A->x, A->x_shallow) ;
    #endif

    if (A->p == NULL)
    {
        if (pr > 0) printf ("->p is NULL, invalid %s\n", kind) ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "%s contains a NULL A->p pointer: [%s]", kind, NAME))) ;
    }

    bool is_empty = (A->nzmax == 0) ;

    if (is_empty)
    {
        // A->x and A->i pointers must be NULL and shallow must be false
        if (A->i != NULL || A->i_shallow || A->x_shallow)
        {
            if (pr > 0) printf ("invalid empty %s\n", kind) ;
            return (ERROR (GrB_INVALID_OBJECT, (LOG,
                "%s is an invalid empty object: [%s]", kind, NAME))) ;
        }

        // check the column pointers
        for (int64_t j = 0 ; j < A->ncols ; j++)
        {
            if (A->p [j] != 0)
            {
                if (pr > 0) printf ("->p ["GBd"] = "GBd" invalid\n", j,A->p[j]);
                return (ERROR (GrB_INVALID_OBJECT, (LOG,
                    "%s ->p ["GBd"] = "GBd" invalid: [%s]",
                    kind, j, A->p[j], NAME))) ;
            }
        }
        if (pr > 0) printf ("empty\n") ;
    }

    if (!is_empty && A->i == NULL)
    {
        if (pr > 0) printf ("->i is NULL, invalid %s\n", kind) ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "%s contains a NULL A->i pointer: [%s]", kind, NAME))) ;
    }

    //--------------------------------------------------------------------------
    // check the column pointers
    //--------------------------------------------------------------------------

    if (A->p [0] != 0)
    {
        if (pr > 0) printf ("->p [0] = "GBd" invalid\n", A->p [0]) ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "%s A->p [0] = "GBd" invalid: [%s]", kind, A->p [0], NAME))) ;
    }

    for (int64_t j = 0 ; j < A->ncols ; j++)
    {
        if (A->p [j+1] < A->p [j] || A->p [j+1] > A->nzmax)
        {
            if (pr > 0) printf ("->p ["GBd"] = "GBd" invalid\n",
                j+1, A->p [j+1]) ;
            return (ERROR (GrB_INVALID_OBJECT, (LOG,
                "%s A->p ["GBd"] = "GBd" invalid: [%s]",
                kind, j, A->p [0], NAME))) ;
        }
    }

    int64_t anz = A->p [A->ncols] ;
    if (pr > 0) printf ("number of entries: "GBd" ", anz) ;

    if (pr > 0) printf ("\n") ;

    //--------------------------------------------------------------------------
    // report the number of pending tuples and number of zombies
    //--------------------------------------------------------------------------

    if (A->npending != 0 || A->nzombies != 0)
    {
        if (pr > 0) printf ("pending tuples: "GBd" max pending: "GBd
            " zombies: "GBd"\n", A->npending, A->max_npending, A->nzombies) ;
    }

    if (A->nzombies < 0 || A->nzombies > anz)
    {
        if (pr > 0) printf ("invalid number of zombies: "GBd" "
            "must be >= 0 and <= # entries ("GBd")\n", A->nzombies, anz) ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "%s invalid number of zombies: "GBd"\n"
            "must be >= 0 and <= # entries ("GBd") [%s]",
            kind, A->nzombies, anz, NAME))) ;
        
    }

    //--------------------------------------------------------------------------
    // check and print the row indices and numerical values
    //--------------------------------------------------------------------------

    #define NBRIEF 10
    #define NZBRIEF 30

    bool jumbled = false ;
    int64_t nzombies = 0 ;

    for (int64_t j = 0 ; j < A->ncols ; j++)
    {
        bool prcol = ((pr > 1 && j < NBRIEF) || pr > 2) ;
        if (prcol)
        {
            printf ("column: "GBd" : "GBd" entries\n",
                j, A->p [j+1] - A->p [j]) ;
        }
        else if (pr == 2 && j == NBRIEF)
        {
            printf ("...\n") ;
        }
        int64_t ilast = -1 ;
        for (int64_t p = A->p [j] ; p < A->p [j+1] ; p++)
        {
            int64_t i = A->i [p] ;
            bool is_zombie = IS_ZOMBIE (i) ;
            i = UNFLIP (i) ;
            if (is_zombie) nzombies++ ;
            if (prcol)
            {
                if ((pr > 1 && p < NZBRIEF) || pr > 2)
                {
                    // #if defined (PRINT_MALLOC) || !defined (NDEBUG)
                    // printf ("    p: "GBd" row "GBd": ", p, i) ;
                    // #else
                    printf ("    row "GBd": ", i) ;
                    // #endif
                }
                else if (pr == 2 && (p == A->p [j] || p == NZBRIEF))
                {
                    printf ("    ...\n") ;
                }
            }
            if (i < 0 || i >= A->nrows)
            {
                if (pr > 0) printf ("index ("GBd","GBd") out of range\n", i, j);
                return (ERROR (GrB_INVALID_OBJECT, (LOG,
                    "%s index ("GBd","GBd") out of range: [%s]",
                    kind, i, j, NAME))) ;
            }
            if (i <= ilast)
            {
                // row indices unsorted, or duplicates present
                if (pr > 0) printf ("index ("GBd","GBd") jumbled\n", i, j) ;
                jumbled = true ;
            }
            // print the value
            if (prcol && ((pr > 1 && p < NZBRIEF) || pr > 2))
            {
                if (is_zombie)
                {
                    printf ("zombie") ;
                }
                else if (A->x != NULL)
                {
                    GB_Entry_print (A->type, A->x +(p * A->type->size)) ;
                }
                printf ("\n") ;
            }
            ilast = i ;
        }
    }

    //--------------------------------------------------------------------------
    // check the zombie count
    //--------------------------------------------------------------------------

    if (nzombies != A->nzombies)
    {
        if (pr > 0) printf ("invalid zombie count: "GBd" exist but"
            " A->nzombies = "GBd"\n", nzombies, A->nzombies) ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "%s invalid zombie count: "GBd" exist but A->nzombies = "GBd" "
            "[%s]", kind, nzombies, A->nzombies, NAME))) ;
    }

    //--------------------------------------------------------------------------
    // check and print the pending tuples
    //--------------------------------------------------------------------------

    if (A->npending < 0 || A->npending > A->max_npending || A->max_npending < 0)
    {
        if (pr > 0) printf ("invalid pending count\n") ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "%s invalid pending tuple count: pending "GBd" max "GBd": [%s]",
            kind, A->npending, A->max_npending, NAME))) ;
    }

    #ifdef DEVELOPER
    if (pr > 0) printf ("A %p\n", A) ;
    if (pr > 0) printf ("->ipending %p\n", A->ipending) ;
    if (pr > 0) printf ("->jpending %p\n", A->jpending) ;
    if (pr > 0) printf ("->xpending %p\n", A->xpending) ;
    #endif

    if (A->npending == 0)
    {
        // no tuples; arrays must be NULL
        if (A->ipending != NULL || A->xpending != NULL ||
            A->jpending != NULL || A->max_npending != 0)
        {
            if (pr > 0) printf ("invalid pending tuples\n") ;
            return (ERROR (GrB_INVALID_OBJECT, (LOG,
                "%s invalid pending tuples: [%s]", kind, NAME))) ;
        }

    }
    else
    {
        // matrix has tuples, arrays must not be NULL
        if (A->ipending == NULL || A->xpending == NULL ||
            (A->ncols > 1 && A->jpending == NULL))
        {
            if (pr > 0) printf ("invalid pending tuples\n") ;
            return (ERROR (GrB_INVALID_OBJECT, (LOG,
                "%s invalid pending tuples: [%s]", kind, NAME))) ;
        }

        if (pr > 0) printf ("pending tuples:\n") ;

        int64_t ilast = -1 ;
        int64_t jlast = -1 ;
        bool sorted = true ;

        for (int64_t k = 0 ; k < A->npending ; k++)
        {
            int64_t i = A->ipending [k] ;
            int64_t j = (A->ncols <= 1) ? 0 : (A->jpending [k]) ;

            // print the tuple
            if ((pr > 1 && k < NZBRIEF) || pr > 2)
            {
                printf ("row: "GBd" col: "GBd" ", i, j) ;
                GB_Entry_print (A->type, A->xpending +(k * A->type->size)) ;
                printf ("\n") ;
            }

            if (i < 0 || i >= A->nrows || j < 0 || j >= A->ncols)
            {
                if (pr > 0) printf ("tuple ("GBd","GBd") out of range\n", i, j);
                return (ERROR (GrB_INVALID_OBJECT, (LOG,
                    "%s tuple index ("GBd","GBd") out of range: [%s]",
                    kind, i, j, NAME))) ;
            }

            sorted = sorted && ((jlast < j) || (jlast == j && ilast <= i)) ;
            ilast = i ;
            jlast = j ;
        }

        if (sorted != A->sorted_pending)
        {
            printf ("sorted %d sorted_pending %d\n", sorted, A->sorted_pending);
            if (pr > 0) printf ("invalid pending tuples: invalid sort\n") ;
            return (ERROR (GrB_INVALID_OBJECT, (LOG,
                "%s invalid pending tuples: [%s]", kind, NAME))) ;
        }

        if (A->operator_pending == NULL)
        {
            if (pr > 0) printf ("pending operator: implicit 2nd\n") ;
        }
        else
        {
            info = GB_check (A->operator_pending, "pending operator:", pr) ;
            if (info != GrB_SUCCESS)
            {
                if (pr > 0) printf ("invalid pending operator\n") ;
                return (ERROR (GrB_INVALID_OBJECT, (LOG,
                    "%s invalid operator: [%s]", kind, NAME))) ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // check the queue
    //--------------------------------------------------------------------------

    GrB_Matrix head, prev, next ;
    bool enqd ;

    GB_queue_check (A, &head, &prev, &next, &enqd) ;

    #ifdef DEVELOPER
    if (pr > 0) printf ("queue head  %p\n", head) ;
    if (pr > 0) printf ("queue prev  %p\n", prev) ;
    if (pr > 0) printf ("queue next  %p\n", next) ;
    if (pr > 0) printf ("is in queue %d\n", enqd) ;
    #endif

    #define IS_NOT_IN_QUEUE(A) (prev == NULL && head != A)
    #define IS_IN_QUEUE(A) (! IS_NOT_IN_QUEUE(A))
    if (enqd != IS_IN_QUEUE (A))
    {
        if (pr > 0) printf ("queued state inconsistent: [%d] != [%d]\n",
            enqd, IS_IN_QUEUE (A)) ;
        return (ERROR (GrB_INVALID_OBJECT, (LOG,
            "%s queued state inconsistent: [%s], [%d] != [%d]", kind, NAME,
            enqd, IS_IN_QUEUE (A)))) ;
    }
    #undef IS_NOT_IN_QUEUE
    #undef IS_IN_QUEUE

    if (PENDING (A) || ZOMBIES (A))
    {
        if (!enqd)
        {
            if (pr > 0) printf ("must be in queue but is not there\n") ;
            return (ERROR (GrB_INVALID_OBJECT, (LOG,
                "%s must be in queue but is not there: [%s]", kind, NAME))) ;
        }

        // prev is NULL if and only if A is at the head of the queue
        if ((prev == NULL) != (head == A))
        {
            if (pr > 0) printf ("invalid queue\n") ;
            return (ERROR (GrB_INVALID_OBJECT, (LOG,
                "%s invalid queue: [%s]", kind, NAME))) ;
        }
    }
    else
    {
        if (enqd)
        {
            if (pr > 0) printf ("must not be in queue but present there\n") ;
            return (ERROR (GrB_INVALID_OBJECT, (LOG,
                "%s must not be in queue but present there: [%s]",
                kind, NAME))) ;
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    // Returns GrB_INVALID_OBJECT if a row or column index is out of bounds,
    // since this indicates the object is corrupted.  No valid matrix is ever
    // built with indices out of bounds since the indices are checked when the
    // matrix is built.

    // Returns GrB_INDEX_OUT_OF_BOUNDS if a column has unsorted indices, and
    // perhaps duplicates as well.  For matrices passed back to the user, or
    // obtained from the user, this is an error.  For some matrices internally,
    // the row indices may be jumbled.  These are about to be sorted via qsort
    // or transpose.  In this case, a jumbled matrix is OK.  Duplicates are
    // still an error but this function does not distinguish between the two
    // cases (it would require workspace to do so).  See the
    // ASSERT_OK_OR_JUMBLED macro.

    // do not log error with REPORT_SUCCESS; it may mask an error in the caller
    return (jumbled ? GrB_INDEX_OUT_OF_BOUNDS : GrB_SUCCESS) ;
}

#undef NBRIEF
#undef NZBRIEF

