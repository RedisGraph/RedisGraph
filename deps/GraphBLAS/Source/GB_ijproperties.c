//------------------------------------------------------------------------------
// GB_ijproperties: check I and determine its properties
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// check a list of indices I and determine its properties

#include "GB.h"

#define GB_ICHECK(i,limit)                                              \
{                                                                       \
    if ((i) < 0 || (i) >= (limit))                                      \
    {                                                                   \
        return (GB_ERROR (GrB_INDEX_OUT_OF_BOUNDS, (GB_LOG,             \
        "index "GBd" out of bounds, must be < "GBd, (i), (limit)))) ;   \
    }                                                                   \
}

GrB_Info GB_ijproperties        // check I and determine its properties
(
    const GrB_Index *I,         // list of indices, or special
    const int64_t ni,           // length I, or special
    const int64_t nI,           // actual length from GB_ijlength
    const int64_t limit,        // I must be in the range 0 to limit-1
    const int64_t Ikind,        // kind of I, from GB_ijlength
    const int64_t Icolon [3],   // begin:inc:end from GB_ijlength
    bool *I_is_unsorted,        // true if I is out of order
    bool *I_is_contig,          // true if I is a contiguous list, imin:imax
    int64_t *imin_result,       // min (I)
    int64_t *imax_result,       // max (I)
    bool is_I,                  // true if I, false if J (debug only)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // limit: the matrix dimension (# of rows or # of columns)
    // ni: only used if Ikind is GB_LIST: the length of the array I
    // nI: the length of the list I, either actual or implicit
    // Ikind: GB_ALL, GB_RANGE, GB_STRIDE (both +/- inc), or GB_LIST
    // Icolon: begin:inc:end for all but GB_LIST

    // outputs:
    // I_is_unsorted: true if Ikind == GB_LIST and not in ascending order
    // I_is_contig: true if I has the form I = begin:end
    // imin, imax: min (I) and max (I), but only including actual indices
    //      in the sequence.  The end value of I=begin:inc:end may not be
    //      reached.  For example if I=1:2:10 then max(I)=9, not 10.

    ASSERT (I != NULL) ;
    ASSERT (limit >= 0) ;
    ASSERT (limit <= GB_INDEX_MAX) ;
    int64_t imin, imax ;

    //--------------------------------------------------------------------------
    // scan I
    //--------------------------------------------------------------------------

    // scan the list of indices: check if OK, determine if they are
    // jumbled, or contiguous, their min and max index, and actual length
    bool I_unsorted = false ;
    bool I_contig = true ;

    if (Ikind == GB_ALL)
    { 

        //----------------------------------------------------------------------
        // I = 0:limit-1
        //----------------------------------------------------------------------

        imin = 0 ;
        imax = limit-1 ;

    }
    else if (Ikind == GB_RANGE)
    {

        //----------------------------------------------------------------------
        // I = imin:imax
        //----------------------------------------------------------------------

        imin = Icolon [GxB_BEGIN] ;
        imax = Icolon [GxB_END  ] ;

        if (imin > imax)
        { 
            // imin > imax: list is empty
            imin = limit ;
            imax = -1 ;
        }
        else
        { 
            // check the limits
            GB_ICHECK (imin, limit) ;
            GB_ICHECK (imax, limit) ;
        }

    }
    else if (Ikind == GB_STRIDE)
    {

        //----------------------------------------------------------------------
        // I = imin:iinc:imax
        //----------------------------------------------------------------------

        int64_t ibegin = Icolon [GxB_BEGIN] ;
        int64_t iinc   = Icolon [GxB_INC  ] ;
        int64_t iend   = Icolon [GxB_END  ] ;

        // if iinc == 1 on input, the kind has been changed to GB_RANGE
        ASSERT (iinc != 1) ;

        if (iinc == 0)
        { 
            // stride is zero: list is empty, contiguous, and sorted
            imin = limit ;
            imax = -1 ;
        }
        else if (iinc > 0)
        { 
            // stride is positive, get the first and last indices
            imin = GB_ijlist (I, 0,    GB_STRIDE, Icolon) ;
            imax = GB_ijlist (I, nI-1, GB_STRIDE, Icolon) ;
        }
        else
        { 
            // stride is negative, get the first and last indices
            imin = GB_ijlist (I, nI-1, GB_STRIDE, Icolon) ;
            imax = GB_ijlist (I, 0,    GB_STRIDE, Icolon) ;
        }

        if (imin > imax)
        { 
            // list is empty: so it is contiguous and sorted
            imin = limit ;
            imax = -1 ;
        }
        else
        { 
            // list is contiguous if the stride is 1, not contiguous otherwise
            I_contig = false ;

            // check the limits
            GB_ICHECK (imin, limit) ;
            GB_ICHECK (imax, limit) ;
        }

    }
    else // Ikind == GB_LIST
    {

        //----------------------------------------------------------------------
        // I is an array of indices
        //----------------------------------------------------------------------

        // scan I to find imin and imax, and validate the list. Also determine
        // if it is sorted or not.

        imin = limit ;
        imax = -1 ;
        int64_t ilast = -1 ;
        for (int64_t inew = 0 ; inew < ni ; inew++)
        {
            int64_t i = I [inew] ;
            GB_ICHECK (i, limit) ;
            if (i < ilast)
            { 
                // The list I of row indices is out of order, and C=A(I,J) will
                // need to use qsort to sort each column.  If C=A(I,J)' is
                // computed, however, this flag will be set back to false,
                // since qsort is not needed if the result is transposed.
                I_unsorted = true ;
            }
            if (inew > 0 && i != ilast + 1)
            { 
                I_contig = false ;
            }
            imin = GB_IMIN (imin, i) ;
            imax = GB_IMAX (imax, i) ;
            ilast = i ;
        }
        if (ni == 1)
        {
            // a single entry does not need to be sorted
            ASSERT (I [0] == imin && I [0] == imax && !I_unsorted) ;
        }
        if (ni == 0)
        {
            // the list is empty
            ASSERT (imin == limit && imax == -1) ;
        }
    }

    ASSERT (GB_IMPLIES (I_contig, !I_unsorted)) ;
    ASSERT (GB_IMPLIES (Ikind == GB_ALL, I_contig)) ;
    ASSERT (GB_IMPLIES (Ikind == GB_RANGE, I_contig)) ;

    // I_is_contig is true if the list of row indices is a contiguous list,
    // imin:imax in MATLAB notation.  This is an important special case.

    // I_is_unsorted is true if I is an explicit list, the list is non-empty,
    // and the indices are not sorted in ascending order.

    (*I_is_contig) = I_contig ;
    (*I_is_unsorted) = I_unsorted ;
    (*imin_result) = imin ;
    (*imax_result) = imax ;
    return (GrB_SUCCESS) ;
}

