//------------------------------------------------------------------------------
// GB_mx_mxArray_to_indices
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Get a list of indices from MATLAB

#include "GB_mex.h"

bool GB_mx_mxArray_to_indices       // true if successful, false otherwise
(
    GrB_Index **handle,             // index array returned
    const mxArray *I_matlab,        // MATLAB mxArray to get
    GrB_Index *ni,                  // length of I, or special
    GrB_Index Icolon [3],           // for all but GB_LIST
    bool *I_is_list                 // true if GB_LIST
)
{

    (*handle) = NULL ;

    mxArray *X ;
    GrB_Index *I ;

    if (I_matlab == NULL || mxIsEmpty (I_matlab))
    {
        I = (GrB_Index *) GrB_ALL ;       // like the ":" in C=A(:,j)
        (*ni) = 0 ;
        (*handle) = I ;
        (*I_is_list) = false ;
        // Icolon not used
    }
    else
    {
        if (mxIsStruct (I_matlab))
        {
            // a struct with 3 integers: I.begin, I.inc, I.end
            (*I_is_list) = false ;

            // look for I.begin (required)
            int fieldnumber = mxGetFieldNumber (I_matlab, "begin") ;
            if (fieldnumber < 0)
            {
                mexWarnMsgIdAndTxt ("GB:warn","I.begin missing") ;
                return (false) ;
            }
            X = mxGetFieldByNumber (I_matlab, 0, fieldnumber) ;
            Icolon [GxB_BEGIN] = (int64_t) mxGetScalar (X) ;

            // look for I.end (required)
            fieldnumber = mxGetFieldNumber (I_matlab, "end") ;
            if (fieldnumber < 0)
            {
                mexWarnMsgIdAndTxt ("GB:warn","I.end missing") ;
                return (false) ;
            }
            mxArray *X ;
            X = mxGetFieldByNumber (I_matlab, 0, fieldnumber) ;
            Icolon [GxB_END] = (int64_t) mxGetScalar (X) ;

            // look for I.inc (optional)
            fieldnumber = mxGetFieldNumber (I_matlab, "inc") ;
            if (fieldnumber < 0)
            {
                (*ni) = GxB_RANGE ;
                Icolon [GxB_INC] = 1 ;
            }
            else
            {
                // 
                X = mxGetFieldByNumber (I_matlab, 0, fieldnumber) ;
                int64_t iinc = (int64_t) mxGetScalar (X) ;
                if (iinc == 0)
                {
                    // this can be either a stride, or backwards.  Either 
                    // one works the same, but try a mixture, just for testing.
                    (*ni) = (Icolon [GxB_BEGIN] % 2) ?
                        GxB_STRIDE : GxB_BACKWARDS ;
                    Icolon [GxB_INC] = 0 ;
                }
                else if (iinc > 0)
                {
                    (*ni) = GxB_STRIDE ;
                    Icolon [GxB_INC] = iinc ;
                }
                else
                {
                    // GraphBLAS must be given the magnitude of the stride
                    (*ni) = GxB_BACKWARDS ;
                    Icolon [GxB_INC] = -iinc ;
                }
            }
            (*handle) = Icolon ;

        }
        else
        {
            if (!mxIsClass (I_matlab, "uint64"))
            {
                mexWarnMsgIdAndTxt ("GB:warn","indices must be uint64") ;
                return (false) ;
            }

            (*I_is_list) = true ;
            I = mxGetData (I_matlab) ;
            (*ni) = (uint64_t) mxGetNumberOfElements (I_matlab) ;
            (*handle) = I ;
        }
    }

    return (true) ;
}

