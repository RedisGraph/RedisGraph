//------------------------------------------------------------------------------
// GB_matvec_check: print a GraphBLAS matrix and check if it is valid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// uncomment this line to add extra diagnostics (for the developer only)
// #define GB_DEVELOPER

#include "GB.h"

GrB_Info GB_matvec_check    // check a GraphBLAS matrix or vector
(
    const GrB_Matrix A,     // GraphBLAS matrix to print and check
    const char *name,       // name of the matrix, optional
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
                            // if negative, ignore queue conditions
                            // and use GB_FLIP(pr) for diagnostic printing.
    FILE *f,                // file for output
    const char *kind,       // "matrix" or "vector"
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    bool ignore_queue = false ;
    if (pr < 0)
    { 
        // -2: print nothing (pr = 0)
        // -3: print header  (pr = 1)
        // -4: print brief   (pr = 2)
        // -5: print all     (pr = 3)
        pr = GB_FLIP (pr) ;
        ignore_queue = true ;
    }

    if (pr > 0) GBPR ("\nGraphBLAS %s: %s ", kind, GB_NAME) ;

    if (A == NULL)
    { 
        // GrB_error status not modified since this may be an optional argument
        if (pr > 0) GBPR ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // check the object
    //--------------------------------------------------------------------------

    GB_CHECK_MAGIC (A, kind) ;
    ASSERT (A->magic == GB_MAGIC) ;    // A is now a valid initialized object

    if (pr > 0)
    { 
        GBPR ("\nnrows: "GBd" ncols: "GBd" max # entries: "GBd"\n",
            GB_NROWS (A), GB_NCOLS (A), A->nzmax) ;
        GBPR ("format: %s %s",
            A->is_hyper ? "hypersparse" : "standard",
            A->is_csc ?   "CSC" : "CSR") ;
        GBPR (" vlen: "GBd" nvec_nonempty: "GBd" nvec: "GBd" plen: "
            GBd " vdim: "GBd"\n",
            A->vlen, A->nvec_nonempty, A->nvec, A->plen, A->vdim) ;
        GBPR ("hyper_ratio %g\n", A->hyper_ratio) ;
    }

    if (A->vlen < 0 || A->vlen > GB_INDEX_MAX ||
        A->vdim < 0 || A->vdim > GB_INDEX_MAX ||
        A->nzmax < 0 || A->nzmax > GB_INDEX_MAX)
    { 
        if (pr > 0) GBPR ("invalid %s dimensions\n", kind) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s invalid : nrows, ncols, or nzmax out of range: [%s]",
            kind, GB_NAME))) ;
    }

    if (A->is_hyper)
    {
        if (! (A->nvec >= 0 && A->nvec <= A->plen && A->plen <= A->vdim &&
               A->nvec == A->nvec_nonempty))
        { 
            if (pr > 0) GBPR ("invalid %s hypersparse structure\n", kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s invalid hypersparse structure [%s]", kind, GB_NAME))) ;
        }
    }
    else
    {
        if (! (A->nvec == A->plen && A->plen == A->vdim))
        { 
            if (pr > 0) GBPR ("invalid %s standard structure\n", kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s invalid structure [%s]", kind, GB_NAME))) ;
        }
    }

    // a matrix contains 1 to 8 different malloc'd blocks
    int64_t nallocs = 1 +                       // header
        (A->h != NULL && !A->h_shallow) +       // A->h, if not shallow
        (A->p != NULL && !A->p_shallow) +       // A->p, if not shallow
        (A->i != NULL && !A->i_shallow) +       // A->i, if not shallow
        (A->x != NULL && !A->x_shallow) +       // A->x, if not shallow
        (A->i_pending != NULL) +                // A->i_pending if tuples
        (A->j_pending != NULL) +                // A->j_pending if tuples
        (A->s_pending != NULL) ;                // A->s_pending if tuples

    #ifdef GB_DEVELOPER
    if (pr > 1) GBPR ("A %p magic "GBd"\n", A, A->magic) ;
    if (pr > 1) GBPR ("number of memory blocks: "GBd"\n", nallocs) ;
    #endif

    GrB_Info info = GB_Type_check (A->type, "", pr, f, Context) ;
    if (info != GrB_SUCCESS || (A->type->size != A->type_size))
    { 
        if (pr > 0) GBPR ("%s has an invalid type\n", kind) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s has an invalid type: [%s]", kind, GB_NAME))) ;
    }

    if (A->Sauna != NULL)
    {
        if (pr > 1) GBPR ("Sauna: n: "GBd" entry size: %zu\n",
            A->Sauna->Sauna_n, A->Sauna->Sauna_size) ;
    }

    if (pr > 1 && A->AxB_method_used != GxB_DEFAULT)
    {
        GBPR ("last method used for GrB_mxm, vxm, or mxv: ") ;
        switch (A->AxB_method_used)
        {
            case GxB_AxB_GUSTAVSON : GBPR ("Gustavson") ; break ;
            case GxB_AxB_HEAP      : GBPR ("heap")      ; break ;
            case GxB_AxB_DOT       : GBPR ("dot")       ; break ;
            default: ;
        }
        GBPR ("\n") ;
    }

    #ifdef GB_DEVELOPER
    if (pr > 1) GBPR ("->h: %p shallow: %d\n", A->h, A->h_shallow) ;
    if (pr > 1) GBPR ("->p: %p shallow: %d\n", A->p, A->p_shallow) ;
    if (pr > 1) GBPR ("->i: %p shallow: %d\n", A->i, A->i_shallow) ;
    if (pr > 1) GBPR ("->x: %p shallow: %d\n", A->x, A->x_shallow) ;
    #endif

    if (A->p == NULL)
    { 
        if (pr > 0) GBPR ("->p is NULL, invalid %s\n", kind) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s contains a NULL A->p pointer: [%s]", kind, GB_NAME))) ;
    }

    if (A->is_hyper)
    {
        if (A->h == NULL)
        { 
            if (pr > 0) GBPR ("->h is NULL, invalid hypersparse %s\n",
                kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "hypersparse %s contains a NULL A->h pointer: [%s]",
                kind, GB_NAME))) ;
        }
    }
    else
    {
        if (A->h != NULL)
        { 
            if (pr > 0) GBPR ("->h is not NULL, invalid non-hypersparse %s\n",
                kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "non-hypersparse %s contains a non-NULL A->h pointer: [%s]",
                kind, GB_NAME))) ;
        }
    }

    bool A_empty = (A->nzmax == 0) ;

    if (A_empty)
    {
        // A->x and A->i pointers must be NULL and shallow must be false
        if (A->i != NULL || A->i_shallow || A->x_shallow)
        { 
            if (pr > 0) GBPR ("invalid empty %s\n", kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s is an invalid empty object: [%s]", kind, GB_NAME))) ;
        }

        // check the vector pointers
        for (int64_t j = 0 ; j <= A->nvec ; j++)
        {
            if (A->p [j] != 0)
            { 
                if (pr > 0) GBPR ("->p ["GBd"] = "GBd" invalid\n", j,A->p[j]);
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s ->p ["GBd"] = "GBd" invalid: [%s]",
                    kind, j, A->p[j], GB_NAME))) ;
            }
        }
        if (pr > 0) GBPR ("empty\n") ;
    }

    if (!A_empty && A->i == NULL)
    { 
        if (pr > 0) GBPR ("->i is NULL, invalid %s\n", kind) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s contains a NULL A->i pointer: [%s]", kind, GB_NAME))) ;
    }

    //--------------------------------------------------------------------------
    // check the vector pointers
    //--------------------------------------------------------------------------

    if (A->p [0] != 0)
    { 
        if (pr > 0) GBPR ("->p [0] = "GBd" invalid\n", A->p [0]) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s A->p [0] = "GBd" invalid: [%s]", kind, A->p [0], GB_NAME))) ;
    }

    for (int64_t j = 0 ; j < A->nvec ; j++)
    {
        if (A->p [j+1] < A->p [j] || A->p [j+1] > A->nzmax)
        { 
            if (pr > 0) GBPR ("->p ["GBd"] = "GBd" invalid\n",
                j+1, A->p [j+1]) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s A->p ["GBd"] = "GBd" invalid: [%s]",
                kind, j+1, A->p [j+1], GB_NAME))) ;
        }
    }

    if (A->is_hyper)
    {
        int64_t jlast = -1 ;
        for (int64_t k = 0 ; k < A->nvec ; k++)
        {
            int64_t j = A->h [k] ;
            if (jlast >= j || j < 0 || j >= A->vdim)
            { 
                if (pr > 0) GBPR ("->h ["GBd"] = "GBd" invalid\n",
                    k, A->h [k]) ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s A->h ["GBd"] = "GBd" invalid: [%s]",
                    kind, k, A->h [k], GB_NAME))) ;
            }
            jlast = j ;
        }
    }

    int64_t anz = GB_NNZ (A) ;
    if (pr > 0) GBPR ("number of entries: "GBd" ", anz) ;

    if (pr > 0) GBPR ("\n") ;

    //--------------------------------------------------------------------------
    // report the number of pending tuples and number of zombies
    //--------------------------------------------------------------------------

    if (A->n_pending != 0 || A->nzombies != 0)
    { 
        if (pr > 0) GBPR ("pending tuples: "GBd" max pending: "GBd
            " zombies: "GBd"\n", A->n_pending, A->max_n_pending, A->nzombies) ;
    }

    if (A->nzombies < 0 || A->nzombies > anz)
    { 
        if (pr > 0) GBPR ("invalid number of zombies: "GBd" "
            "must be >= 0 and <= # entries ("GBd")\n", A->nzombies, anz) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s invalid number of zombies: "GBd"\n"
            "must be >= 0 and <= # entries ("GBd") [%s]",
            kind, A->nzombies, anz, GB_NAME))) ;
    }

    //--------------------------------------------------------------------------
    // check and print the row indices and numerical values
    //--------------------------------------------------------------------------

    #define GB_NBRIEF 10
    #define GB_NZBRIEF 30

    bool jumbled = false ;
    int64_t nzombies = 0 ;
    int64_t jcount = 0 ;

    GB_for_each_vector (A)
    {
        int64_t ilast = -1 ;
        GB_for_each_entry (j, p, pend)
        {
            bool prcol = ((pr > 1 && jcount < GB_NBRIEF) || pr > 2) ;
            if (ilast == -1)
            {
                // print the header for vector j
                if (prcol)
                { 
                    GBPR ("%s: "GBd" : "GBd" entries ["GBd":"GBd"]\n",
                        A->is_csc ? "column" : "row", j, pend - p, p, pend-1) ;
                }
                else if (pr == 2 && jcount == GB_NBRIEF)
                { 
                    GBPR ("...\n") ;
                }
                jcount++ ;      // count # of vectors printed so far
            }
            int64_t i = A->i [p] ;
            bool is_zombie = GB_IS_ZOMBIE (i) ;
            i = GB_UNFLIP (i) ;
            if (is_zombie) nzombies++ ;
            if (prcol)
            { 
                if ((pr > 1 && p < GB_NZBRIEF) || pr > 2)
                { 
                    GBPR ("    %s "GBd": ", A->is_csc ? "row" : "column", i) ;
                }
                else if (pr == 2 && (ilast == -1 || p == GB_NZBRIEF))
                { 
                    GBPR ("    ...\n") ;
                }
            }
            int64_t row = A->is_csc ? i : j ;
            int64_t col = A->is_csc ? j : i ;
            if (i < 0 || i >= A->vlen)
            { 
                if (pr > 0) GBPR ("index ("GBd","GBd") out of range\n",
                    row, col) ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s index ("GBd","GBd") out of range: [%s]",
                    kind, row, col, GB_NAME))) ;
            }

            // print the value
            bool print_value = prcol && ((pr > 1 && p < GB_NZBRIEF) || pr > 2) ;
            if (print_value)
            { 
                if (is_zombie)
                { 
                    GBPR ("zombie") ;
                }
                else if (A->x != NULL)
                { 
                    GB_void *Ax = A->x ;
                    info = GB_entry_check (A->type,
                        Ax +(p * (A->type->size)), f, Context) ;
                    if (info != GrB_SUCCESS) return (info) ;
                }
            }

            if (i <= ilast)
            { 
                // indices unsorted, or duplicates present
                if (pr > 0) GBPR (" index ("GBd","GBd") jumbled", row, col) ;
                jumbled = true ;
                print_value = (pr > 0) ;
            }

            if (print_value)
            { 
                GBPR ("\n") ;
            }
            ilast = i ;
        }
    }

    //--------------------------------------------------------------------------
    // check the zombie count
    //--------------------------------------------------------------------------

    if (nzombies != A->nzombies)
    { 
        if (pr > 0) GBPR ("invalid zombie count: "GBd" exist but"
            " A->nzombies = "GBd"\n", nzombies, A->nzombies) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s invalid zombie count: "GBd" exist but A->nzombies = "GBd" "
            "[%s]", kind, nzombies, A->nzombies, GB_NAME))) ;
    }

    //--------------------------------------------------------------------------
    // check and print the pending tuples
    //--------------------------------------------------------------------------

    if (A->n_pending < 0 || A->n_pending > A->max_n_pending ||
        A->max_n_pending < 0)
    { 
        if (pr > 0) GBPR ("invalid pending count\n") ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s invalid pending tuple count: pending "GBd" max "GBd": [%s]",
            kind, A->n_pending, A->max_n_pending, GB_NAME))) ;
    }

    #ifdef GB_DEVELOPER
    if (pr > 1) GBPR ("->i_pending %p\n", A->i_pending) ;
    if (pr > 1) GBPR ("->j_pending %p\n", A->j_pending) ;
    if (pr > 1) GBPR ("->s_pending %p\n", A->s_pending) ;
    #endif

    if (A->n_pending == 0)
    {

        //---------------------------------------------------------------------
        // A has no pending tuples
        //---------------------------------------------------------------------

        // no tuples; arrays must be NULL
        if (A->i_pending != NULL || A->s_pending != NULL ||
            A->j_pending != NULL || A->max_n_pending != 0)
        { 
            if (pr > 0) GBPR ("invalid pending tuples\n") ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s invalid pending tuples: [%s]", kind, GB_NAME))) ;
        }

    }
    else
    {

        //---------------------------------------------------------------------
        // A has pending tuples
        //---------------------------------------------------------------------

        // matrix has tuples, arrays and type must not be NULL
        if (A->i_pending == NULL || A->s_pending == NULL ||
            (A->vdim > 1 && A->j_pending == NULL))
        { 
            if (pr > 0) GBPR ("invalid pending tuples\n") ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s invalid pending tuples: [%s]", kind, GB_NAME))) ;
        }

        if (pr > 0) GBPR ("pending tuples:\n") ;

        info = GB_Type_check (A->type_pending, "", pr, f, Context) ;
        if (info != GrB_SUCCESS ||
            (A->type_pending->size != A->type_pending_size))
        { 
            if (pr > 0) GBPR ("%s has an invalid type_pending\n", kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s has an invalid type_pending: [%s]", kind, GB_NAME))) ;
        }

        int64_t ilast = -1 ;
        int64_t jlast = -1 ;
        bool sorted = true ;

        for (int64_t k = 0 ; k < A->n_pending ; k++)
        {
            int64_t i = A->i_pending [k] ;
            int64_t j = (A->vdim <= 1) ? 0 : (A->j_pending [k]) ;
            int64_t row = A->is_csc ? i : j ;
            int64_t col = A->is_csc ? j : i ;

            // print the tuple
            if ((pr > 1 && k < GB_NZBRIEF) || pr > 2)
            { 
                GBPR ("row: "GBd" col: "GBd" ", row, col) ;
                GB_void *As = A->s_pending ;
                info = GB_entry_check (A->type_pending,
                    As +(k * A->type_pending->size), f, Context) ;
                if (info != GrB_SUCCESS) return (info) ;
                GBPR ("\n") ;
            }

            if (i < 0 || i >= A->vlen || j < 0 || j >= A->vdim)
            { 
                if (pr > 0) GBPR ("tuple ("GBd","GBd") out of range\n",
                    row, col) ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s tuple index ("GBd","GBd") out of range: [%s]",
                    kind, row, col, GB_NAME))) ;
            }

            sorted = sorted && ((jlast < j) || (jlast == j && ilast <= i)) ;
            ilast = i ;
            jlast = j ;
        }

        if (sorted != A->sorted_pending)
        { 
            GBPR ("sorted %d sorted_pending %d\n", sorted, A->sorted_pending);
            if (pr > 0) GBPR ("invalid pending tuples: invalid sort\n") ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s invalid pending tuples: [%s]", kind, GB_NAME))) ;
        }

        if (A->operator_pending == NULL)
        { 
            if (pr > 0) GBPR ("pending operator: implicit 2nd\n") ;
        }
        else
        {
            info = GB_BinaryOp_check (A->operator_pending, "pending operator:",
                pr, f, Context) ;
            if (info != GrB_SUCCESS)
            { 
                if (pr > 0) GBPR ("invalid pending operator\n") ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s invalid operator: [%s]", kind, GB_NAME))) ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // check the queue
    //--------------------------------------------------------------------------

    if (!ignore_queue)
    {
        GrB_Matrix head, prev, next ;
        bool enqd ;

        GB_CRITICAL (GB_queue_status (A, &head, &prev, &next, &enqd)) ;

        #ifdef GB_DEVELOPER
        if (pr > 1) GBPR ("queue head  %p\n", head) ;
        if (pr > 1) GBPR ("queue prev  %p\n", prev) ;
        if (pr > 1) GBPR ("queue next  %p\n", next) ;
        if (pr > 1) GBPR ("is in queue %d\n", enqd) ;
        #endif

        #define GB_IS_NOT_IN_QUEUE(A) (prev == NULL && head != A)
        #define GB_IS_IN_QUEUE(A) (! GB_IS_NOT_IN_QUEUE(A))
        if (enqd != GB_IS_IN_QUEUE (A))
        { 
            if (pr > 0) GBPR ("queued state inconsistent: [%d] != [%d]\n",
                enqd, GB_IS_IN_QUEUE (A)) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s queued state inconsistent: [%s], [%d] != [%d]", kind,
                GB_NAME, enqd, GB_IS_IN_QUEUE (A)))) ;
        }

        if (GB_PENDING (A) || GB_ZOMBIES (A))
        {
            if (!enqd)
            { 
                if (pr > 0) GBPR ("must be in queue but is not there\n") ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s must be in queue but is not there: [%s]", kind, GB_NAME))) ;
            }

            // prev is NULL if and only if A is at the head of the queue
            if ((prev == NULL) != (head == A))
            { 
                if (pr > 0) GBPR ("invalid queue\n") ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s invalid queue: [%s]", kind, GB_NAME))) ;
            }
        }
        else
        {
            if (enqd)
            { 
                if (pr > 0) GBPR ("must not be in queue but is there\n") ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s must not be in queue but present there: [%s]",
                    kind, GB_NAME))) ;
            }
        }
    }

    if (pr == 3) GBPR ("\n") ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    int64_t actual = GB_nvec_nonempty (A) ;
    if (A->nvec_nonempty != actual)
    { 
        if (pr > 0) GBPR ("invalid count of non-empty vectors"
            "A->nvec_nonempty = "GBd" actual "GBd"\n",
            A->nvec_nonempty, actual) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s invalid count of nonempty-vectors [%s]", kind, GB_NAME))) ;
    }

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

    return (jumbled ? GrB_INDEX_OUT_OF_BOUNDS : GrB_SUCCESS) ;
}

