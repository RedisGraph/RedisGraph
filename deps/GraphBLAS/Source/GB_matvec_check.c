//------------------------------------------------------------------------------
// GB_matvec_check: print a GraphBLAS matrix and check if it is valid
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// for additional diagnostics, use:
// #define GB_DEVELOPER 1

#include "GB_Pending.h"
#include "GB_iterator.h"

GrB_Info GB_matvec_check    // check a GraphBLAS matrix or vector
(
    const GrB_Matrix A,     // GraphBLAS matrix to print and check
    const char *name,       // name of the matrix, optional
    int pr,                 // 0: print nothing, 1: print header and errors,
                            // 2: print brief, 3: print all
                            // if negative, ignore queue and nzombie conditions
                            // and use GB_FLIP(pr) for diagnostic printing.
    FILE *f,                // file for output
    const char *kind,       // "matrix" or "vector"
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // decide what to print
    //--------------------------------------------------------------------------

    bool ignore_queue_and_nzombies = false ;
    if (pr < 0)
    { 
        // -2: print nothing (pr = 0)
        // -3: print header  (pr = 1)
        // -4: print brief   (pr = 2)
        // -5: print all     (pr = 3)
        pr = GB_FLIP (pr) ;
        ignore_queue_and_nzombies = true ;
    }

    GBPR0 ("\nGraphBLAS %s: %s ", kind, GB_NAME) ;

    //--------------------------------------------------------------------------
    // check if null, freed, or uninitialized
    //--------------------------------------------------------------------------

    if (A == NULL)
    { 
        // GrB_error status not modified since this may be an optional argument
        GBPR0 ("NULL\n") ;
        return (GrB_NULL_POINTER) ;
    }

    GB_CHECK_MAGIC (A, kind) ;

    //--------------------------------------------------------------------------
    // print the header
    //--------------------------------------------------------------------------

    if (pr > 0)
    { 
        GBPR ("\nnrows: "GBd" ncols: "GBd" max # entries: "GBd"\n",
            GB_NROWS (A), GB_NCOLS (A), A->nzmax) ;

        GBPR ("format: %s %s",
            A->is_hyper ?
                (A->is_slice ? "hyperslice" : "hypersparse") :
                (A->is_slice ? "slice" : "standard"),
            A->is_csc ?   "CSC" : "CSR") ;

        GBPR (" vlen: "GBd, A->vlen) ;
        if (A->nvec_nonempty != -1)
        { 
            GBPR (" nvec_nonempty: "GBd, A->nvec_nonempty) ;
        }
        GBPR (" nvec: "GBd" plen: "GBd " vdim: "GBd"\n",
            A->nvec, A->plen, A->vdim) ;
        GBPR ("hyper_ratio %g\n", A->hyper_ratio) ;
    }

    //--------------------------------------------------------------------------
    // check the dimensions
    //--------------------------------------------------------------------------

    if (A->vlen < 0 || A->vlen > GB_INDEX_MAX ||
        A->vdim < 0 || A->vdim > GB_INDEX_MAX ||
        A->nzmax < 0 || A->nzmax > GB_INDEX_MAX)
    { 
        GBPR0 ("invalid %s dimensions\n", kind) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s invalid : nrows, ncols, or nzmax out of range: [%s]",
            kind, GB_NAME))) ;
    }

    //--------------------------------------------------------------------------
    // check vector structure
    //--------------------------------------------------------------------------

    if (A->is_slice)
    {
        if (A->is_hyper)
        { 
            // A is a hyperslice of a hypersparse matrix
            GBPR0 ("hyperslice\n") ;
        }
        else
        { 
            // A is a slice of a standard matrix
            GBPR0 ("slice ["GBd":"GBd"]\n",
                A->hfirst, A->hfirst + A->nvec + - 1) ;
        }
        if (! (A->nvec <= A->vdim && A->plen == A->nvec))
        { 
            // invalid slice
            GBPR0 ("invalid slice %s structure\n", kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "invalid slice %s structure [%s]", kind, GB_NAME))) ;
        }
    }
    else
    {
        if (A->is_hyper)
        {
            // A is hypersparse
            if (! (A->nvec >= 0 && A->nvec <= A->plen && A->plen <= A->vdim))
            { 
                GBPR0 ("invalid hypersparse %s structure\n", kind) ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "invalid hypersparse %s structure [%s]", kind, GB_NAME))) ;
            }
        }
        else
        {
            // A is standard
            if (! (A->nvec == A->plen && A->plen == A->vdim))
            { 
                GBPR0 ("invalid standard %s structure\n", kind) ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "invalid %s structure [%s]", kind, GB_NAME))) ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // count the allocated blocks
    //--------------------------------------------------------------------------

    GB_Pending Pending = A->Pending ;

    #ifdef GB_DEVELOPER

    // a matrix contains 1 to 9 different allocated blocks
    int64_t nallocs = 1 +                       // header
        (A->h != NULL && !A->h_shallow) +       // A->h, if not shallow
        (A->p != NULL && !A->p_shallow) +       // A->p, if not shallow
        (A->i != NULL && !A->i_shallow) +       // A->i, if not shallow
        (A->x != NULL && !A->x_shallow) +       // A->x, if not shallow
        (Pending != NULL) +
        (Pending != NULL && Pending->i != NULL) +
        (Pending != NULL && Pending->j != NULL) +
        (Pending != NULL && Pending->x != NULL) ;

    if (pr > 1) GBPR ("A %p number of memory blocks: "GBd"\n", A, nallocs) ;

    #endif

    //--------------------------------------------------------------------------
    // check the type
    //--------------------------------------------------------------------------

    GrB_Info info = GB_Type_check (A->type, "", pr, f, Context) ;
    if (info != GrB_SUCCESS || (A->type->size != A->type_size))
    { 
        GBPR0 ("%s has an invalid type\n", kind) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s has an invalid type: [%s]", kind, GB_NAME))) ;
    }

    //--------------------------------------------------------------------------
    // report last method used for C=A*B
    //--------------------------------------------------------------------------

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

    //--------------------------------------------------------------------------
    // report shallow structure
    //--------------------------------------------------------------------------

    #ifdef GB_DEVELOPER
    if (pr > 1) GBPR ("->h: %p shallow: %d\n", A->h, A->h_shallow) ;
    if (pr > 1) GBPR ("->p: %p shallow: %d\n", A->p, A->p_shallow) ;
    if (pr > 1) GBPR ("->i: %p shallow: %d\n", A->i, A->i_shallow) ;
    if (pr > 1) GBPR ("->x: %p shallow: %d\n", A->x, A->x_shallow) ;
    #endif

    if (A->is_slice)
    {
        // a slice or hyperslice must have shallow i and x content
        if (!A->i_shallow || !A->x_shallow)
        { 
            // bad slice: must have shallow i and x
            GBPR0 ("invalid non-shallow slice %s\n", kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "non-shallow: invalid slice %s [%s\n", kind, GB_NAME))) ;
        }
    }

    //--------------------------------------------------------------------------
    // check p
    //--------------------------------------------------------------------------

    if (A->p == NULL)
    { 
        GBPR0 ("->p is NULL, invalid %s\n", kind) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s contains a NULL A->p pointer: [%s]", kind, GB_NAME))) ;
    }

    //--------------------------------------------------------------------------
    // check h
    //--------------------------------------------------------------------------

    if (A->is_hyper)
    {
        // A is hypersparse
        if (A->h == NULL)
        { 
            GBPR0 ("->h NULL, invalid hypersparse %s\n", kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "hypersparse %s contains a NULL A->h pointer: [%s]",
                kind, GB_NAME))) ;
        }
    }
    else
    {
        // A is standard
        if (A->h != NULL)
        { 
            GBPR0 ("->h not NULL, invalid non-hypersparse %s\n",
                kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "non-hypersparse %s contains a non-NULL A->h pointer: [%s]",
                kind, GB_NAME))) ;
        }
    }

    //--------------------------------------------------------------------------
    // check hfirst
    //--------------------------------------------------------------------------

    if (A->is_slice && !A->is_hyper)
    {
        // hfirst is the first vector in a slice of a standard sparse matrix
        if (A->hfirst < 0 || A->hfirst + A->nvec > A->vdim)
        { 
            // bad slice: hfirst invalid
            GBPR0 ("hfirst: invalid slice %s\n", kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "hfirst: invalid slice %s [%s]\n", kind, GB_NAME))) ;
        }
    }
    else
    {
        // only a standard slice can have a nonzero hfirst
        if (A->hfirst != 0)
        { 
            // bad hyperslice: only a standard slice can have a nonzero hfirst
            GBPR0 ("hfirst: invalid slice %s\n", kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "hfirst: invalid slice %s [%s]\n", kind, GB_NAME))) ;
        }
    }

    //--------------------------------------------------------------------------
    // check an empty matrix
    //--------------------------------------------------------------------------

    bool A_empty = (A->nzmax == 0) ;

    if (A_empty && !(A->is_slice))
    {
        // A->x and A->i pointers must be NULL and shallow must be false

        if (A->i != NULL || A->i_shallow || A->x_shallow)
        { 
            GBPR0 ("invalid empty %s\n", kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s is an invalid empty object: [%s]", kind, GB_NAME))) ;
        }

        // check the vector pointers
        for (int64_t j = 0 ; j <= A->nvec ; j++)
        {
            if (A->p [j] != 0)
            { 
                GBPR0 ("->p ["GBd"] = "GBd" invalid\n", j,A->p[j]);
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s ->p ["GBd"] = "GBd" invalid: [%s]",
                    kind, j, A->p[j], GB_NAME))) ;
            }
        }
        GBPR0 ("empty\n") ;
    }

    //--------------------------------------------------------------------------
    // check a non-empty matrix
    //--------------------------------------------------------------------------

    if (!A_empty && A->i == NULL)
    { 
        GBPR0 ("->i is NULL, invalid %s\n", kind) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s contains a NULL A->i pointer: [%s]", kind, GB_NAME))) ;
    }

    //--------------------------------------------------------------------------
    // check the content of p
    //--------------------------------------------------------------------------

    if (A->is_slice ? (A->p [0] < 0) : (A->p [0] != 0))
    { 
        GBPR0 ("->p [0] = "GBd" invalid\n", A->p [0]) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s A->p [0] = "GBd" invalid: [%s]", kind, A->p [0], GB_NAME))) ;
    }

    for (int64_t j = 0 ; j < A->nvec ; j++)
    {
        if (A->p [j+1] < A->p [j] || A->p [j+1] > A->nzmax)
        { 
            GBPR0 ("->p ["GBd"] = "GBd" invalid\n",
                j+1, A->p [j+1]) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s A->p ["GBd"] = "GBd" invalid: [%s]",
                kind, j+1, A->p [j+1], GB_NAME))) ;
        }
    }

    //--------------------------------------------------------------------------
    // check the content of h
    //--------------------------------------------------------------------------

    if (A->is_hyper)
    {
        int64_t jlast = -1 ;
        for (int64_t k = 0 ; k < A->nvec ; k++)
        {
            int64_t j = A->h [k] ;
            // printf ("Ah ["GBd"] = "GBd"\n", k, j) ;
            if (jlast >= j || j < 0 || j >= A->vdim)
            { 
                GBPR0 ("->h ["GBd"] = "GBd" invalid\n", k, j) ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s A->h ["GBd"] = "GBd" invalid: [%s]",
                    kind, k, j, GB_NAME))) ;
            }
            jlast = j ;
        }
    }

    //--------------------------------------------------------------------------
    // report number of entries
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ (A) ;
    GBPR0 ("number of entries: "GBd" \n", anz) ;

    //--------------------------------------------------------------------------
    // report the number of pending tuples and zombies
    //--------------------------------------------------------------------------

    if (Pending != NULL || A->nzombies != 0)
    { 
        if (A->is_slice)
        { 
            // a slice or hyperslice cannot have pending work
            GBPR0 ("slice %s invalid: unfinished\n", kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "slice %s invalid: unfinished [%s]", kind, GB_NAME))) ;
        }
        GBPR0 ("pending tuples: "GBd" max pending: "GBd
            " zombies: "GBd"\n", GB_Pending_n (A),
            (Pending == NULL) ? 0 : (Pending->nmax),
            A->nzombies) ;
    }

    if (!ignore_queue_and_nzombies && (A->nzombies < 0 || A->nzombies > anz))
    { 
        // zombie count is ignored if pr is flipped
        GBPR0 ("invalid number of zombies: "GBd" "
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

    GBI_for_each_vector (A)
    {
        int64_t ilast = -1 ;
        GBI_for_each_entry (j, p, pend)
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
                GBPR0 ("index ("GBd","GBd") out of range\n", row, col) ;
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
                GBPR0 (" index ("GBd","GBd") jumbled", row, col) ;
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

    if (!ignore_queue_and_nzombies && nzombies != A->nzombies)
    { 
        // zombie count is ignored if pr is flipped
        GBPR0 ("invalid zombie count: "GBd" exist but"
            " A->nzombies = "GBd"\n", nzombies, A->nzombies) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s invalid zombie count: "GBd" exist but A->nzombies = "GBd" "
            "[%s]", kind, nzombies, A->nzombies, GB_NAME))) ;
    }

    //--------------------------------------------------------------------------
    // check and print the pending tuples
    //--------------------------------------------------------------------------

    #ifdef GB_DEVELOPER
    if (pr > 1) GBPR ("Pending %p\n", Pending) ;
    #endif

    if (Pending != NULL)
    {

        //---------------------------------------------------------------------
        // A has pending tuples
        //---------------------------------------------------------------------

        #ifdef GB_DEVELOPER
        if (pr > 1) GBPR ("Pending->i %p\n", Pending->i) ;
        if (pr > 1) GBPR ("Pending->j %p\n", Pending->j) ;
        if (pr > 1) GBPR ("Pending->x %p\n", Pending->x) ;
        #endif

        if (Pending->n < 0 || Pending->n > Pending->nmax ||
            Pending->nmax < 0)
        { 
            GBPR0 ("invalid pending count\n") ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s invalid pending tuple count: pending "GBd" max "GBd": [%s]",
                kind, Pending->n, Pending->nmax, GB_NAME))) ;
        }

        // matrix has tuples, arrays and type must not be NULL
        if (Pending->i == NULL || Pending->x == NULL ||
            (A->vdim > 1 && Pending->j == NULL))
        { 
            GBPR0 ("invalid pending tuples\n") ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s invalid pending tuples: [%s]", kind, GB_NAME))) ;
        }

        GBPR0 ("pending tuples:\n") ;

        info = GB_Type_check (Pending->type, "", pr, f, Context) ;
        if (info != GrB_SUCCESS || (Pending->type->size != Pending->size))
        { 
            GBPR0 ("%s has an invalid Pending->type\n", kind) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s has an invalid Pending->type: [%s]", kind, GB_NAME))) ;
        }

        int64_t ilast = -1 ;
        int64_t jlast = -1 ;
        bool sorted = true ;

        for (int64_t k = 0 ; k < Pending->n ; k++)
        {
            int64_t i = Pending->i [k] ;
            int64_t j = (A->vdim <= 1) ? 0 : (Pending->j [k]) ;
            int64_t row = A->is_csc ? i : j ;
            int64_t col = A->is_csc ? j : i ;

            // print the tuple
            if ((pr > 1 && k < GB_NZBRIEF) || pr > 2)
            { 
                GBPR ("row: "GBd" col: "GBd" ", row, col) ;
                info = GB_entry_check (Pending->type,
                    Pending->x +(k * Pending->type->size), f, Context) ;
                if (info != GrB_SUCCESS) return (info) ;
                GBPR ("\n") ;
            }

            if (i < 0 || i >= A->vlen || j < 0 || j >= A->vdim)
            { 
                GBPR0 ("tuple ("GBd","GBd") out of range\n", row, col) ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s tuple index ("GBd","GBd") out of range: [%s]",
                    kind, row, col, GB_NAME))) ;
            }

            sorted = sorted && ((jlast < j) || (jlast == j && ilast <= i)) ;
            ilast = i ;
            jlast = j ;
        }

        if (sorted != Pending->sorted)
        { 
            GBPR0 ("invalid pending tuples: invalid sort\n") ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s invalid pending tuples: [%s]", kind, GB_NAME))) ;
        }

        if (Pending->op == NULL)
        { 
            GBPR0 ("pending operator: implicit 2nd\n") ;
        }
        else
        {
            info = GB_BinaryOp_check (Pending->op, "pending operator:",
                pr, f, Context) ;
            if (info != GrB_SUCCESS)
            { 
                GBPR0 ("invalid pending operator\n") ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s invalid operator: [%s]", kind, GB_NAME))) ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // check the queue
    //--------------------------------------------------------------------------

    if (!ignore_queue_and_nzombies)
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
            GBPR0 ("queued state inconsistent: [%d] != [%d]\n",
                enqd, GB_IS_IN_QUEUE (A)) ;
            return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s queued state inconsistent: [%s], [%d] != [%d]", kind,
                GB_NAME, enqd, GB_IS_IN_QUEUE (A)))) ;
        }

        if (GB_PENDING (A) || GB_ZOMBIES (A))
        {
            if (!enqd)
            { 
                GBPR0 ("must be in queue but is not there\n") ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                "%s must be in queue but is not there: [%s]", kind, GB_NAME))) ;
            }

            // prev is NULL if and only if A is at the head of the queue
            if ((prev == NULL) != (head == A))
            { 
                GBPR0 ("invalid queue\n") ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s invalid queue: [%s]", kind, GB_NAME))) ;
            }
        }
        else
        {
            if (enqd)
            { 
                GBPR0 ("must not be in queue but is there\n") ;
                return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
                    "%s must not be in queue but present there: [%s]",
                    kind, GB_NAME))) ;
            }
        }
    }

    if (pr == 3) GBPR ("\n") ;

    //--------------------------------------------------------------------------
    // check nvec_nonempty
    //--------------------------------------------------------------------------

    // A->nvec_nonempty == -1 denotes that the value has not been computed.
    // This is valid, and can occur for matrices imported with
    // GxB_Matrix_import*, and in other cases when its computation is postponed
    // or not needed.  If not -1, however, the value must be correct.

    int64_t actual_nvec_nonempty = GB_nvec_nonempty (A, Context) ;

    if (! ((A->nvec_nonempty == actual_nvec_nonempty) ||
           (A->nvec_nonempty == -1)))
    { 
        GBPR0 ("invalid count of non-empty vectors\n"
            "A->nvec_nonempty = "GBd" actual "GBd"\n",
            A->nvec_nonempty, actual_nvec_nonempty) ;
        return (GB_ERROR (GrB_INVALID_OBJECT, (GB_LOG,
            "%s invalid count of nonempty-vectors [%s]", kind, GB_NAME))) ;
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

    return (jumbled ? GrB_INDEX_OUT_OF_BOUNDS : GrB_SUCCESS) ;
}

