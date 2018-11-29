#include "spok.h"

/* check the validity of a MATLAB sparse matrix */

SPOK_INT spok
(
    /* inputs, not modified */
    SPOK_INT m,             /* number of rows */
    SPOK_INT n,             /* number of columns */
    SPOK_INT nzmax,         /* max # of entries */
    SPOK_INT *Ap,           /* size n+1, column pointers */
    SPOK_INT *Ai,           /* size nz = Ap [n], row indices */
    double *Ax,             /* double matrices always have Ax */
    double *Az,             /* imaginary matrices always have Az */
    char *As,               /* logical matrices always have As */

    /* outputs, not defined on input */
    SPOK_INT *p_njumbled,   /* # of jumbled row indices (-1 if not computed) */
    SPOK_INT *p_nzeros      /* number of explicit zeros (-1 if not computed) */
)
{
    double x, z ;
    SPOK_INT i, j, p, pend, njumbled, nzeros, ilast ;
    char s ;

    /* ---------------------------------------------------------------------- */
    /* in case of early return */
    /* ---------------------------------------------------------------------- */

    if (p_njumbled != NULL)
    {
        *p_njumbled = -1 ;
    }
    if (p_nzeros != NULL)
    {
        *p_nzeros = -1 ;
    }

    /* ---------------------------------------------------------------------- */
    /* check the dimensions */
    /* ---------------------------------------------------------------------- */

    if (m < 0)
    {
        return (SPOK_FATAL_M) ;
    }
    if (n < 0)
    {
        return (SPOK_FATAL_N) ;
    }
    if (nzmax < 1) 
    {
        /* note that nzmax cannot be zero */
        return (SPOK_FATAL_NZMAX) ;
    }

    /* ---------------------------------------------------------------------- */
    /* check the column pointers */
    /* ---------------------------------------------------------------------- */

    if (Ap == NULL || Ap [0] != 0)
    {
        /* column pointers invalid */
        return (SPOK_FATAL_P) ;
    }
    for (j = 0 ; j < n ; j++)
    {
        p = Ap [j] ;
        pend = Ap [j+1] ;
        if (pend < p || pend > nzmax)
        {
            /* column pointers not monotonically non-decreasing */
            return (SPOK_FATAL_P) ;
        }
    }

    /* ---------------------------------------------------------------------- */
    /* check the row indices and numerical values */
    /* ---------------------------------------------------------------------- */

    if (Ai == NULL)
    {
        /* row indices not present */
        return (SPOK_FATAL_I) ;
    }

    njumbled = 0 ;
    nzeros = 0 ;

    for (j = 0 ; j < n ; j++)
    {
        ilast = -1 ;
        // printf ("column %lld [%lld : %lld]\n", j, Ap [j], Ap [j+1]-1) ;
        for (p = Ap [j] ; p < Ap [j+1] ; p++)
        {
            i = Ai [p] ;
            // printf ("row %lld value: ", i) ;
            if (i < 0 || i >= m)
            {
                /* row indices out of range */
                return (SPOK_FATAL_I) ;
            }
            if (i <= ilast)
            {
                /* row indices unsorted, or duplicates present */
                njumbled++ ;
            }
            s = (As == NULL) ? 0 : As [p] ;
            x = (Ax == NULL) ? 0 : Ax [p] ;
            z = (Az == NULL) ? 0 : Az [p] ;
            // printf (" %d %g %g\n", s, x, z) ;
            if (s == 0 && x == 0 && z == 0)
            {
                /* an explicit zero is present */
                nzeros++ ;
            }
            ilast = i ;
        }
    }

    /* ---------------------------------------------------------------------- */
    /* return results */
    /* ---------------------------------------------------------------------- */

    if (p_njumbled != NULL)
    {
        *p_njumbled = njumbled ;
    }
    if (p_nzeros != NULL)
    {
        *p_nzeros = nzeros ;
    }
    return ((njumbled > 0 || nzeros > 0) ? SPOK_WARNING : SPOK_OK) ;
}
