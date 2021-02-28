//------------------------------------------------------------------------------
// GB_mx_classID_to_string: return a MATLAB string from the class ID
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Given a MATLAB class ID, constructs a MATLAB string with the class name

#include "GB_mex.h"

mxArray *GB_mx_classID_to_string       // returns a MATLAB string
(
    const mxClassID classID             // MATLAB class ID to convert to string
)
{

    switch (classID)
    {
        case mxLOGICAL_CLASS  : return (mxCreateString ("logical")) ;
        case mxINT8_CLASS     : return (mxCreateString ("int8")) ;
        case mxUINT8_CLASS    : return (mxCreateString ("uint8")) ;
        case mxINT16_CLASS    : return (mxCreateString ("int16")) ;
        case mxUINT16_CLASS   : return (mxCreateString ("uint16")) ;
        case mxINT32_CLASS    : return (mxCreateString ("int32")) ;
        case mxUINT32_CLASS   : return (mxCreateString ("uint32")) ;
        case mxINT64_CLASS    : return (mxCreateString ("int64")) ;
        case mxUINT64_CLASS   : return (mxCreateString ("uint64")) ;
        case mxSINGLE_CLASS   : return (mxCreateString ("single")) ;
        case mxDOUBLE_CLASS   : return (mxCreateString ("double")) ;
        case mxCELL_CLASS     : return (mxCreateString ("cell")) ;
        case mxCHAR_CLASS     : return (mxCreateString ("char")) ;
        case mxUNKNOWN_CLASS  : return (mxCreateString ("unknown")) ;
        case mxFUNCTION_CLASS : return (mxCreateString ("function")) ;
        case mxSTRUCT_CLASS   : return (mxCreateString ("struct")) ;
        default               : return (mxCreateString ("other")) ;
    }
}

