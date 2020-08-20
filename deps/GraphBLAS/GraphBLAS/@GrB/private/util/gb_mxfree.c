//------------------------------------------------------------------------------
// gb_mxfree: mxFree wrapper
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Usage:

//  void *p = mxMalloc ( ... ) ;
//  gb_mxfree (&p) ;                // frees p and sets p to NULL

#include "gb_matlab.h"

void gb_mxfree              // mxFree wrapper
(
    void **p_handle         // handle to pointer to be freed
)
{

    if (p_handle != NULL)
    {
        if (*p_handle != NULL)
        { 
            mxFree (*p_handle) ;
        }
        (*p_handle) = NULL ;
    }
}

