//------------------------------------------------------------------------------
// gb_mxarray_to_list: convert a MATLAB array to a list of integers
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The MATLAB list may be double, int64, or uint64.  If double or 1-based
// int64, a new integer list is created, and the 1-based input list is
// converted to the 0-based integer list.

#include "gb_matlab.h"

int64_t *gb_mxarray_to_list     // return List of integers
(
    const mxArray *mxList,      // list to extract
    base_enum_t base,           // input is zero-based or one-based
    bool *allocated,            // true if output list was allocated
    int64_t *len,               // length of list
    int64_t *List_max           // max entry in the list, if computed
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    CHECK_ERROR (!mxIsNumeric (mxList), "index list must be numeric") ;
    CHECK_ERROR (mxIsSparse (mxList), "index list cannot be sparse") ;

    //--------------------------------------------------------------------------
    // get the length and class of the MATLAB list
    //--------------------------------------------------------------------------

    (*len) = mxGetNumberOfElements (mxList) ;
    mxClassID class = mxGetClassID (mxList) ;

    //--------------------------------------------------------------------------
    // extract the contents and convert to int64_t
    //--------------------------------------------------------------------------

    (*List_max) = -1 ;
    bool zerobased = (base == BASE_0_INT64) ;

    if (*len == 0)
    { 
        (*allocated) = true ;
        return ((int64_t *) mxCalloc (1, sizeof (int64_t))) ;
    }
    else if (class == mxINT64_CLASS && zerobased)
    { 
        // input list is int64; just return a shallow pointer
        (*allocated) = false ;
        return ((int64_t *) mxGetInt64s (mxList)) ;
    }
    else if (class == mxUINT64_CLASS && zerobased)
    { 
        // input list is uint64; just return a shallow pointer
        (*allocated) = false ;
        return ((int64_t *) mxGetUint64s (mxList)) ;
    }
    else if (class == mxINT64_CLASS || class == mxUINT64_CLASS ||
             class == mxDOUBLE_CLASS)
    {
        // input list 1-based: decrement to convert to 0-based
        (*allocated) = true ;
        int64_t *List = mxMalloc ((*len) * sizeof (int64_t)) ;
        if (class == mxDOUBLE_CLASS)
        {
            // input list is 1-based double
            double *List_double = mxGetDoubles (mxList) ;
            bool ok = GB_matlab_helper3 (List, List_double, (*len), List_max) ;
            CHECK_ERROR (!ok, "index must be integer") ;
        }
        else if (class == mxINT64_CLASS)
        {
            // input list is 1-based int64
            int64_t *List_int64 = (int64_t *) mxGetInt64s (mxList) ;
            GB_matlab_helper3i (List, List_int64, (*len), List_max) ;
        }
        else // if (class == mxUINT64_CLASS)
        {
            // input list is 1-based uint64
            int64_t *List_int64 = (int64_t *) mxGetUint64s (mxList) ;
            GB_matlab_helper3i (List, List_int64, (*len), List_max) ;
        }
        return (List) ;
    }
    else
    { 
        ERROR ("integer array must be double, int64, or uint64") ;
    }
}

