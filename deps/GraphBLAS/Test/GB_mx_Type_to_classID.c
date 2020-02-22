//------------------------------------------------------------------------------
// GB_mx_Type_to_classID: return the GraphBLAS type of the MATLAB class ID
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

mxClassID GB_mx_Type_to_classID        // returns a MATLAB class ID
(
    const GrB_Type type                 // GraphBLAS type to convert
)
{

    switch (type->code)
    {                                                      // GrB  MATLAB
        case GB_BOOL_code   : return (mxLOGICAL_CLASS) ;   // 0  -> 3
        case GB_INT8_code   : return (mxINT8_CLASS   ) ;   // 1  -> 8
        case GB_UINT8_code  : return (mxUINT8_CLASS  ) ;   // 2  -> 9
        case GB_INT16_code  : return (mxINT16_CLASS  ) ;   // 3  -> 10
        case GB_UINT16_code : return (mxUINT16_CLASS ) ;   // 4  -> 11
        case GB_INT32_code  : return (mxINT32_CLASS  ) ;   // 5  -> 12
        case GB_UINT32_code : return (mxUINT32_CLASS ) ;   // 6  -> 13
        case GB_INT64_code  : return (mxINT64_CLASS  ) ;   // 7  -> 14
        case GB_UINT64_code : return (mxUINT64_CLASS ) ;   // 8  -> 15
        case GB_FP32_code   : return (mxSINGLE_CLASS ) ;   // 9  -> 7
        case GB_FP64_code   : return (mxDOUBLE_CLASS ) ;   // 10 -> 6
        // assume user-defined type is Complex, and MATLAB double complex
        case GB_UDT_code    : return (mxDOUBLE_CLASS ) ;   // 11 -> 6
        default             : ;
    }
    mexWarnMsgIdAndTxt ("GB:warn", "invalid type code") ;
    return (mxUNKNOWN_CLASS) ;
}

