//------------------------------------------------------------------------------
// GB_mx_string_to_classid.c: return the class ID from a class string
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_mex.h"

#define LEN 256

mxClassID GB_mx_string_to_classID      // returns the MATLAB class ID
(
    const mxClassID class_default,      // default if string is NULL
    const mxArray *class_mx             // string with class name
)
{

    // get the class name
    char classname [LEN+2] ;
    int len = GB_mx_mxArray_to_string (classname, LEN, class_mx) ;

    if (len < 0)
    {
        return (mxUNKNOWN_CLASS) ;
    }

    // convert the classname to a MATLAB mxClassID
    mxClassID classID ;

    if (len == 0)
    {
        classID = class_default ;
    }
    else if (MATCH (classname, "logical")) { classID = mxLOGICAL_CLASS ; }
    else if (MATCH (classname, "int8"   )) { classID = mxINT8_CLASS    ; }
    else if (MATCH (classname, "uint8"  )) { classID = mxUINT8_CLASS   ; }
    else if (MATCH (classname, "int16"  )) { classID = mxINT16_CLASS   ; }
    else if (MATCH (classname, "uint16" )) { classID = mxUINT16_CLASS  ; }
    else if (MATCH (classname, "int32"  )) { classID = mxINT32_CLASS   ; }
    else if (MATCH (classname, "uint32" )) { classID = mxUINT32_CLASS  ; }
    else if (MATCH (classname, "int64"  )) { classID = mxINT64_CLASS   ; }
    else if (MATCH (classname, "uint64" )) { classID = mxUINT64_CLASS  ; }
    else if (MATCH (classname, "single" )) { classID = mxSINGLE_CLASS  ; }
    else if (MATCH (classname, "double" )) { classID = mxDOUBLE_CLASS  ; }
    else
    {
        classID = mxUNKNOWN_CLASS ;
    }

    return (classID) ;
}

