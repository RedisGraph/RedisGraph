//------------------------------------------------------------------------------
// gb_norm_kind: determine the kind of norm to compute
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "gb_matlab.h"

// 'fro':       Frobenius norm
// 1:           1-norm
// 2:           2-norm
// INFINITY:    inf-norm
// -INFINITY:   (-inf)-norm

int64_t gb_norm_kind (const mxArray *arg)
{
    if (mxIsChar (arg))
    {
        char string [65] ;
        gb_mxstring_to_string (string, 64, arg, "kind") ;
        if (MATCH (string, "fro"))
        {
            return (0) ;
        }
        else
        {
            // unknown string
            ERROR ("unknown norm") ;
        }
    }
    else if (mxIsScalar (arg))
    {
        double x = mxGetScalar (arg) ;
        if (x == INFINITY)
        {
            return (INT64_MAX) ;
        }
        else if (x == -INFINITY)
        {
            return (INT64_MIN) ;
        }
        else if (x == 1 || x == 2)
        {
            return ((int64_t) x) ;
        }
        else
        {
            ERROR ("unknown norm") ;
        }
    }
    else
    {
        // arg must be a scalar
        ERROR ("unknown norm") ;
    }
}

