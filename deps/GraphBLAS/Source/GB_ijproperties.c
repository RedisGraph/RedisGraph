//------------------------------------------------------------------------------
// GB_ijproperties: check I and J and determine properties
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// check row indices I and column indices J and determine their properties

#include "GB.h"

GrB_Info GB_ijproperties    // check I and J and determine properties
(
    const GrB_Index *I,     // size ni, or GrB_ALL
    const int64_t ni,
    const GrB_Index *J,     // size nj, or GrB_ALL
    const int64_t nj,
    const int64_t nrows,    // number of rows of the matrix
    const int64_t ncols,    // number of columns of the matrix
    bool *need_qsort,       // true if I is out of order
    bool *contig,           // true if I is a contiguous list, imin:imax
    int64_t *imin,          // min (I)
    int64_t *imax           // max (I)
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (I != NULL) ;
    ASSERT (J != NULL) ;

    //--------------------------------------------------------------------------
    // scan I and J
    //--------------------------------------------------------------------------

    // scan the list of row indices: check if OK, determine if they are
    // jumbled, or contiguous, and their min and max index
    (*need_qsort) = false ;
    (*contig) = true ;

    if (I == GrB_ALL)
    {
        (*imin) = 0 ;
        (*imax) = ni-1 ;
    }
    else
    {
        (*imin) = nrows ;
        (*imax) = -1 ;
        int64_t ilast = -1 ;
        for (int64_t inew = 0 ; inew < ni ; inew++)
        {
            int64_t i = I [inew] ;
            if (i < 0 || i >= nrows)
            {
                return (ERROR (GrB_INDEX_OUT_OF_BOUNDS, (LOG,
                "index "GBu" out of bounds, must be < "GBd, I [inew], nrows))) ;
            }
            if (i < ilast)
            {
                // The list I of row indices is out of order, and C=A(I,J) will
                // need to use qsort to sort each column.  If C=A(I,J)' is
                // computed, however, this flag will be set back to false,
                // since qsort is not needed if the result is transposed.
                (*need_qsort) = true ;
            }
            if (inew > 0 && i != ilast + 1)
            {
                (*contig) = false ;
            }
            (*imin) = IMIN ((*imin), i) ;
            (*imax) = IMAX ((*imax), i) ;
            ilast = i ;
        }
        if (ni == 1)
        {
            // a single entry does not need to be sorted
            ASSERT (I [0] == (*imin) && I [0] == (*imax) && !(*need_qsort)) ;
        }
        if (ni == 0)
        {
            // the list is empty
            ASSERT ((*imin) == nrows && (*imax) == -1) ;
        }
    }

    ASSERT (IMPLIES ((*contig), !(*need_qsort))) ;
    ASSERT (IMPLIES (I == GrB_ALL, (*contig))) ;

    // contig is true if the list of row indices is a contiguous list,
    // imin:imax in MATLAB notation.  This is an important special case.

    // check the list of column indices
    if (J != GrB_ALL)
    {
        for (int64_t jnew = 0 ; jnew < nj ; jnew++)
        {
            int64_t j = J [jnew] ;
            if (j < 0 || j >= ncols)
            {
                return (ERROR (GrB_INDEX_OUT_OF_BOUNDS, (LOG,
                "index "GBu" out of bounds, must be < "GBd, J [jnew], ncols))) ;
            }
        }
    }

    return (REPORT_SUCCESS) ;
}

